/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#include <config.h>

#include <sys/param.h>

#include <pthread.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/thread.h>

#include <fusion/build.h>
#include <fusion/object.h>
#include <fusion/hash.h>
#include <fusion/shmalloc.h>

#include "fusion_internal.h"

D_DEBUG_DOMAIN( Fusion_Object, "Fusion/Object", "Fusion Objects and Pools" );

struct __Fusion_FusionObjectPool {
     int                     magic;

     FusionWorldShared      *shared;

     FusionSkirmish          lock;
     FusionHash             *objects;
     FusionObjectID          id_pool;

     char                   *name;
     int                     object_size;
     int                     message_size;
     FusionObjectDestructor  destructor;
     void                   *ctx;

     FusionCall              call;
};

static FusionCallHandlerResult
object_reference_watcher( int caller, int call_arg, void *call_ptr, void *ctx, unsigned int serial, int *ret_val )
{
     FusionObject     *object;
     FusionObjectPool *pool = ctx;

     D_DEBUG_AT( Fusion_Object, "%s( %d, %d, %p, %p, %u, %p )\n",
                 __FUNCTION__, caller, call_arg, call_ptr, ctx, serial, ret_val );

#if FUSION_BUILD_KERNEL
     if (caller) {
          D_BUG( "Call not from Fusion/Kernel (caller %d)", caller );
          return FCHR_RETURN;
     }
#endif

     D_MAGIC_ASSERT( pool, FusionObjectPool );

     /* Lock the pool. */
     if (fusion_skirmish_prevail( &pool->lock ))
          return FCHR_RETURN;

     /* Lookup the object. */
     object = fusion_hash_lookup( pool->objects, (void*)(long) call_arg );
     if (object) {
          D_MAGIC_ASSERT( object, FusionObject );

          switch (fusion_ref_zero_trylock( &object->ref )) {
               case DR_OK:
                    break;

               case DR_DESTROYED:
                    D_BUG( "already destroyed %p [%u] in '%s'", object, object->id, pool->name );

                    fusion_hash_remove( pool->objects, (void*)(long) object->id, NULL, NULL );
                    fusion_skirmish_dismiss( &pool->lock );
                    return FCHR_RETURN;


               default:
                    D_ERROR( "Fusion/ObjectPool: Error locking ref of %p [%u] in '%s'\n",
                             object, object->id, pool->name );
                    /* fall through */

               case DR_BUSY:
                    fusion_skirmish_dismiss( &pool->lock );
                    return FCHR_RETURN;
          }

          D_DEBUG_AT( Fusion_Object, "== %s ==\n", pool->name );
          D_DEBUG_AT( Fusion_Object, "  -> dead object %p [%u] (ref %d)\n", object, object->id, object->ref.multi.id );

          if (object->state == FOS_INIT) {
               D_BUG( "== %s == incomplete object: %d (%p)", pool->name, call_arg, object );
               D_WARN( "won't destroy incomplete object, leaking some memory" );
               fusion_hash_remove( pool->objects, (void*)(long) object->id, NULL, NULL );
               fusion_skirmish_dismiss( &pool->lock );
               return FCHR_RETURN;
          }

          /* Set "deinitializing" state. */
          object->state = FOS_DEINIT;

          /* Remove the object from the pool. */
          object->pool = NULL;
          fusion_hash_remove( pool->objects, (void*)(long) object->id, NULL, NULL );

          /* Unlock the pool. */
          fusion_skirmish_dismiss( &pool->lock );


          D_DEBUG_AT( Fusion_Object, "  -> calling destructor...\n" );

          /* Call the destructor. */
          pool->destructor( object, false, pool->ctx );

          D_DEBUG_AT( Fusion_Object, "  -> destructor done.\n" );

          return FCHR_RETURN;
     }

     D_BUG( "unknown object [%d] in '%s'", call_arg, pool->name );

     /* Unlock the pool. */
     fusion_skirmish_dismiss( &pool->lock );

     return FCHR_RETURN;
}

FusionObjectPool *
fusion_object_pool_create( const char             *name,
                           int                     object_size,
                           int                     message_size,
                           FusionObjectDestructor  destructor,
                           void                   *ctx,
                           const FusionWorld      *world )
{
     FusionObjectPool  *pool;
     FusionWorldShared *shared;

     D_ASSERT( name != NULL );
     D_ASSERT( object_size >= sizeof(FusionObject) );
     D_ASSERT( message_size > 0 );
     D_ASSERT( destructor != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     /* Allocate shared memory for the pool. */
     pool = SHCALLOC( shared->main_pool, 1, sizeof(FusionObjectPool) );
     if (!pool) {
          D_OOSHM();
          return NULL;
     }

     /* Initialize the pool lock. */
     fusion_skirmish_init( &pool->lock, name, world );

     fusion_skirmish_add_permissions( &pool->lock, 0, FUSION_SKIRMISH_PERMIT_PREVAIL | FUSION_SKIRMISH_PERMIT_DISMISS );

     /* Fill information. */
     pool->shared       = shared;
     pool->name         = SHSTRDUP( shared->main_pool, name );
     pool->object_size  = object_size;
     pool->message_size = message_size;
     pool->destructor   = destructor;
     pool->ctx          = ctx;

     fusion_hash_create( shared->main_pool, HASH_INT, HASH_PTR, 17, &pool->objects );

     /* Destruction call from Fusion. */
     fusion_call_init( &pool->call, object_reference_watcher, pool, world );

     D_MAGIC_SET( pool, FusionObjectPool );

     return pool;
}

DirectResult
fusion_object_pool_destroy( FusionObjectPool  *pool,
                            const FusionWorld *world )
{
     DirectResult        ret;
     FusionObject       *object;
     FusionWorldShared  *shared;
     FusionHashIterator  it;

     D_ASSERT( pool != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );
     D_ASSERT( shared == pool->shared );

     D_DEBUG_AT( Fusion_Object, "== %s ==\n", pool->name );
     D_DEBUG_AT( Fusion_Object, "  -> destroying pool...\n" );

     D_DEBUG_AT( Fusion_Object, "  -> syncing...\n" );

     /* Wait for processing of pending messages. */
     if (pool->objects)
          fusion_sync( world );

     D_DEBUG_AT( Fusion_Object, "  -> locking...\n" );

     /* Lock the pool. */
     ret = fusion_skirmish_prevail( &pool->lock );
     if (ret)
          return ret;

     /* Destroy the call. */
     fusion_call_destroy( &pool->call );

     if (pool->objects)
          D_WARN( "still objects in '%s'", pool->name );

     /* Destroy zombies */
     fusion_hash_foreach (object, it, pool->objects) {
          int refs;

          fusion_ref_stat( &object->ref, &refs );

          D_DEBUG_AT( Fusion_Object, "== %s ==\n", pool->name );
          D_DEBUG_AT( Fusion_Object, "  -> zombie %p [%u], refs %d\n", object, object->id, refs );

          /* Set "deinitializing" state. */
          object->state = FOS_DEINIT;

          /* Remove the object from the pool. */
          //direct_list_remove( &pool->objects, &object->link );
          //object->pool = NULL;

          D_DEBUG_AT( Fusion_Object, "  -> calling destructor...\n" );

          /* Call the destructor. */
          pool->destructor( object, refs > 0, pool->ctx );

          D_DEBUG_AT( Fusion_Object, "  -> destructor done.\n" );
     }

     fusion_hash_destroy( pool->objects );

     /* Destroy the pool lock. */
     fusion_skirmish_destroy( &pool->lock );

     D_DEBUG_AT( Fusion_Object, "  -> pool destroyed (%s)\n", pool->name );

     D_MAGIC_CLEAR( pool );

     /* Deallocate shared memory. */
     SHFREE( shared->main_pool, pool->name );
     SHFREE( shared->main_pool, pool );

     return DR_OK;
}

typedef struct {
     FusionObjectPool     *pool;
     FusionObjectCallback  callback;
     void                 *ctx;
} ObjectIteratorContext;

static bool
object_iterator( FusionHash *hash,
                 void       *key,
                 void       *value,
                 void       *ctx )
{
     ObjectIteratorContext *context = ctx;
     FusionObject          *object  = value;

     D_MAGIC_ASSERT( object, FusionObject );

     return !context->callback( context->pool, object, context->ctx );
}

DirectResult
fusion_object_pool_enum( FusionObjectPool     *pool,
                         FusionObjectCallback  callback,
                         void                 *ctx )
{
     ObjectIteratorContext iterator_context;

     D_MAGIC_ASSERT( pool, FusionObjectPool );
     D_ASSERT( callback != NULL );

     /* Lock the pool. */
     if (fusion_skirmish_prevail( &pool->lock ))
          return DR_FUSION;

     iterator_context.pool     = pool;
     iterator_context.callback = callback;
     iterator_context.ctx      = ctx;

     fusion_hash_iterate( pool->objects, object_iterator, &iterator_context );

     /* Unlock the pool. */
     fusion_skirmish_dismiss( &pool->lock );

     return DR_OK;
}

FusionObject *
fusion_object_create( FusionObjectPool  *pool,
                      const FusionWorld *world,
                      FusionID           identity )
{
     FusionObject      *object;
     FusionWorldShared *shared;

     D_MAGIC_ASSERT( pool, FusionObjectPool );
     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );
     D_ASSERT( shared == pool->shared );

     /* Lock the pool. */
     if (fusion_skirmish_prevail( &pool->lock ))
          return NULL;

     /* Allocate shared memory for the object. */
     object = SHCALLOC( shared->main_pool, 1, pool->object_size );
     if (!object) {
          D_OOSHM();
          fusion_skirmish_dismiss( &pool->lock );
          return NULL;
     }

     /* Set "initializing" state. */
     object->state = FOS_INIT;

     /* Set object id. */
     object->id = ++pool->id_pool;

     object->identity = identity;

     /* Initialize the reference counter. */
     if (fusion_ref_init( &object->ref, pool->name, world )) {
          SHFREE( shared->main_pool, object );
          fusion_skirmish_dismiss( &pool->lock );
          return NULL;
     }

     /* Increase the object's reference counter. */
     fusion_ref_up( &object->ref, false );

     /* Install handler for automatic destruction. */
     if (fusion_ref_watch( &object->ref, &pool->call, object->id )) {
          fusion_ref_destroy( &object->ref );
          SHFREE( shared->main_pool, object );
          fusion_skirmish_dismiss( &pool->lock );
          return NULL;
     }

     /* Create a reactor for message dispatching. */
     object->reactor = fusion_reactor_new( pool->message_size, pool->name, world );
     if (!object->reactor) {
          fusion_ref_destroy( &object->ref );
          SHFREE( shared->main_pool, object );
          fusion_skirmish_dismiss( &pool->lock );
          return NULL;
     }

     fusion_reactor_set_lock( object->reactor, &pool->lock );

     /* Set pool/world back pointer. */
     object->pool   = pool;
     object->shared = shared;

     /* Add the object to the pool. */
     fusion_hash_insert( pool->objects, (void*)(long) object->id, object );

     D_DEBUG_AT( Fusion_Object, "== %s ==\n", pool->name );

#if FUSION_BUILD_MULTI
     D_DEBUG_AT( Fusion_Object, "  -> added %p with ref [0x%x]\n", object, object->ref.multi.id );
#else
     D_DEBUG_AT( Fusion_Object, "  -> added %p\n", object );
#endif

     D_MAGIC_SET( object, FusionObject );

     /* Unlock the pool. */
     fusion_skirmish_dismiss( &pool->lock );

     return object;
}

DirectResult
fusion_object_get( FusionObjectPool  *pool,
                   FusionObjectID     object_id,
                   FusionObject     **ret_object )
{
     DirectResult  ret = DR_IDNOTFOUND;
     FusionObject *object;

     D_MAGIC_ASSERT( pool, FusionObjectPool );
     D_ASSERT( ret_object != NULL );

     /* Lock the pool. */
     if (fusion_skirmish_prevail( &pool->lock ))
          return DR_FUSION;

     object = fusion_hash_lookup( pool->objects, (void*)(long) object_id );
     if (object) {
          ret = fusion_object_ref( object );
          if (ret == DR_OK)
               *ret_object = object;
     }

     /* Unlock the pool. */
     fusion_skirmish_dismiss( &pool->lock );

     return ret;
}

DirectResult
fusion_object_set_lock( FusionObject   *object,
                        FusionSkirmish *lock )
{
     D_MAGIC_ASSERT( object, FusionObject );

     D_ASSERT( lock != NULL );

     D_ASSUME( object->state == FOS_INIT );

     return fusion_reactor_set_lock_only( object->reactor, lock );
}

DirectResult
fusion_object_activate( FusionObject *object )
{
     D_MAGIC_ASSERT( object, FusionObject );

     /* Set "active" state. */
     object->state = FOS_ACTIVE;

     return DR_OK;
}

DirectResult
fusion_object_destroy( FusionObject *object )
{
     FusionObjectPool  *pool;
     FusionWorldShared *shared;

     D_MAGIC_ASSERT( object, FusionObject );
     D_ASSERT( object->state != FOS_ACTIVE );

     shared = object->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     pool = object->pool;

//     D_ASSUME( pool != NULL );

     /* Set "deinitializing" state. */
     object->state = FOS_DEINIT;

     /* Remove the object from the pool. */
     if (pool) {
          D_MAGIC_ASSERT( pool, FusionObjectPool );

          /* Lock the pool. */
          if (fusion_skirmish_prevail( &pool->lock ))
               return DR_FAILURE;

          D_MAGIC_ASSERT( pool, FusionObjectPool );

          D_ASSUME( object->pool != NULL );

          /* Remove the object from the pool. */
          if (object->pool) {
               D_ASSERT( object->pool == pool );

               object->pool = NULL;

               fusion_hash_remove( pool->objects, (void*)(long) object->id, NULL, NULL );
          }

          /* Unlock the pool. */
          fusion_skirmish_dismiss( &pool->lock );
     }

     fusion_ref_destroy( &object->ref );

     fusion_reactor_free( object->reactor );

     if ( object->properties )
          fusion_hash_destroy(object->properties);

     D_MAGIC_CLEAR( object );
     SHFREE( shared->main_pool, object );
     return DR_OK;
}

/*
 * Sets a value for a key.
 * If the key currently has a value the old value is returned
 * in old_value.
 * If old_value is null the object is freed with SHFREE.
 * If this is not the correct semantics for your data, if for example
 * its reference counted  you must pass in a old_value.
 */
DirectResult
fusion_object_set_property( FusionObject  *object,
                            const char    *key,
                            void          *value,
                            void         **old_value )
{
     DirectResult  ret;
     char         *sharedkey;

     D_MAGIC_ASSERT( object, FusionObject );
     D_ASSERT( object->shared != NULL );
     D_ASSERT( key != NULL );
     D_ASSERT( value != NULL );

     /* Create property hash on demand. */
     if (!object->properties) {
          ret = fusion_hash_create( object->shared->main_pool,
                                    HASH_STRING, HASH_PTR,
                                    FUSION_HASH_MIN_SIZE,
                                    &object->properties );
          if (ret)
               return ret;
     }

     /* Create a shared copy of the key. */
     sharedkey = SHSTRDUP( object->shared->main_pool, key );
     if (!sharedkey)
          return D_OOSHM();

     /* Put it into the hash. */
     ret = fusion_hash_replace( object->properties, sharedkey,
                                value, NULL, old_value );
     if (ret)
          SHFREE( object->shared->main_pool, sharedkey );

     return ret;
}

/*
 * Helper function for int values
 */
DirectResult
fusion_object_set_int_property( FusionObject *object,
                                const char   *key,
                                int           value )
{
     DirectResult  ret;
     int          *iptr;

     D_MAGIC_ASSERT( object, FusionObject );
     D_ASSERT( key != NULL );

     iptr = SHMALLOC( object->shared->main_pool, sizeof(int) );
     if (!iptr)
          return D_OOSHM();

     *iptr = value;

     ret = fusion_object_set_property( object, key, iptr, NULL );
     if (ret)
          SHFREE( object->shared->main_pool, iptr );

     return ret;
}

/*
 * Helper function for char* values use if the string 
 * is not in shared memory
 * Assumes that the old value was a string and frees it.
 */
DirectResult
fusion_object_set_string_property( FusionObject *object,
                                   const char   *key,
                                   char         *value )
{
     DirectResult  ret;
     char         *copy;

     D_MAGIC_ASSERT( object, FusionObject );
     D_ASSERT( key != NULL );
     D_ASSERT( value != NULL );

     copy = SHSTRDUP( object->shared->main_pool, value );
     if (!copy)
          return D_OOSHM();

     ret = fusion_object_set_property( object, key, copy, NULL );
     if (ret)
          SHFREE( object->shared->main_pool, copy );

     return ret;
}

void *
fusion_object_get_property( FusionObject *object, const char *key )
{
     D_MAGIC_ASSERT( object, FusionObject );
     D_ASSERT( key != NULL );

     if (!object->properties)
          return NULL;

     return fusion_hash_lookup( object->properties, key );
}

void 
fusion_object_remove_property( FusionObject  *object,
                               const char    *key,
                               void         **old_value)
{
     D_MAGIC_ASSERT( object, FusionObject );
     D_ASSERT( key != NULL );

     if (!object->properties)
          return;

     fusion_hash_remove( object->properties, key, NULL, old_value );

     if (fusion_hash_should_resize( object->properties ))
          fusion_hash_resize( object->properties );
}

