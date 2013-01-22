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
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include <sys/param.h>
#include <sys/types.h>

#include <fusion/build.h>

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <fusion/types.h>
#include <fusion/property.h>

#include "fusion_internal.h"


#if FUSION_BUILD_MULTI

#if FUSION_BUILD_KERNEL

DirectResult
fusion_property_init (FusionProperty *property, const FusionWorld *world)
{
     D_ASSERT( property != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );

     while (ioctl (world->fusion_fd, FUSION_PROPERTY_NEW, &property->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;
               default:
                    break;
          }

          D_PERROR ("FUSION_PROPERTY_NEW");

          return DR_FAILURE;
     }

     /* Keep back pointer to shared world data. */
     property->multi.shared = world->shared;

     return DR_OK;
}

DirectResult
fusion_property_lease (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     while (ioctl (_fusion_fd( property->multi.shared ), FUSION_PROPERTY_LEASE, &property->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;
               case EAGAIN:
                    return DR_BUSY;
               case EINVAL:
                    D_ERROR ("Fusion/Property: invalid property\n");
                    return DR_DESTROYED;
               default:
                    break;
          }

          D_PERROR ("FUSION_PROPERTY_LEASE");

          return DR_FAILURE;
     }

     return DR_OK;
}

DirectResult
fusion_property_purchase (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     while (ioctl (_fusion_fd( property->multi.shared ), FUSION_PROPERTY_PURCHASE, &property->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;
               case EAGAIN:
                    return DR_BUSY;
               case EINVAL:
                    D_ERROR ("Fusion/Property: invalid property\n");
                    return DR_DESTROYED;
               default:
                    break;
          }

          D_PERROR ("FUSION_PROPERTY_PURCHASE");

          return DR_FAILURE;
     }

     return DR_OK;
}

DirectResult
fusion_property_cede (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     while (ioctl (_fusion_fd( property->multi.shared ), FUSION_PROPERTY_CEDE, &property->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;
               case EINVAL:
                    D_ERROR ("Fusion/Property: invalid property\n");
                    return DR_DESTROYED;
               default:
                    break;
          }

          D_PERROR ("FUSION_PROPERTY_CEDE");

          return DR_FAILURE;
     }

     return DR_OK;
}

DirectResult
fusion_property_holdup (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     while (ioctl (_fusion_fd( property->multi.shared ), FUSION_PROPERTY_HOLDUP, &property->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;
               case EINVAL:
                    D_ERROR ("Fusion/Property: invalid property\n");
                    return DR_DESTROYED;
               default:
                    break;
          }

          D_PERROR ("FUSION_PROPERTY_HOLDUP");

          return DR_FAILURE;
     }

     return DR_OK;
}

DirectResult
fusion_property_destroy (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     while (ioctl (_fusion_fd( property->multi.shared ), FUSION_PROPERTY_DESTROY, &property->multi.id)) {
          switch (errno) {
               case EINTR:
                    continue;
               case EINVAL:
                    D_ERROR ("Fusion/Property: invalid property\n");
                    return DR_DESTROYED;
               default:
                    break;
          }

          D_PERROR ("FUSION_PROPERTY_DESTROY");

          return DR_FAILURE;
     }

     return DR_OK;
}

#else /* FUSION_BUILD_KERNEL */

#include <direct/system.h>

DirectResult
fusion_property_init (FusionProperty *property, const FusionWorld *world)
{
     D_ASSERT( property != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );

     /* Set state to available. */
     property->multi.builtin.state = FUSION_PROPERTY_AVAILABLE;
     property->multi.builtin.owner = 0;

     property->multi.builtin.requested = false;
     property->multi.builtin.destroyed = false;

     /* Keep back pointer to shared world data. */
     property->multi.shared = world->shared;

     return DR_OK;
}

DirectResult
fusion_property_lease (FusionProperty *property)
{
     int count = 0;
     
     D_ASSERT( property != NULL );

     if (property->multi.builtin.destroyed)
          return DR_DESTROYED;

     D_ASSUME( property->multi.builtin.owner != direct_gettid() );
     
     asm( "" ::: "memory" );
     
     while (property->multi.builtin.state == FUSION_PROPERTY_LEASED) {
          /* Check whether owner exited without releasing. */
          if (kill( property->multi.builtin.owner, 0 ) < 0 && errno == ESRCH) {
               property->multi.builtin.state = FUSION_PROPERTY_AVAILABLE;
               property->multi.builtin.requested = false;
               break;
          }
          
          property->multi.builtin.requested = true;
          
          asm( "" ::: "memory" );
          
          if (++count > 1000) {
               usleep( 10000 );
               count = 0;
          }
          else {          
               direct_sched_yield();
          }
               
          if (property->multi.builtin.destroyed)
               return DR_DESTROYED;
     }

     if (property->multi.builtin.state == FUSION_PROPERTY_PURCHASED) {
          /* Check whether owner exited without releasing. */
          if (!(kill( property->multi.builtin.owner, 0 ) < 0 && errno == ESRCH))
               return DR_BUSY;
     }
     
     property->multi.builtin.state = FUSION_PROPERTY_LEASED;
     property->multi.builtin.owner = direct_gettid();
     
     asm( "" ::: "memory" );

     return DR_OK;
}

DirectResult
fusion_property_purchase (FusionProperty *property)
{
     int count = 0;
     
     D_ASSERT( property != NULL );

     if (property->multi.builtin.destroyed)
          return DR_DESTROYED;

     D_ASSUME( property->multi.builtin.owner != direct_gettid() );
     
     asm( "" ::: "memory" );
          
     while (property->multi.builtin.state == FUSION_PROPERTY_LEASED) {
          /* Check whether owner exited without releasing. */
          if (kill( property->multi.builtin.owner, 0 ) < 0 && errno == ESRCH) {
               property->multi.builtin.state = FUSION_PROPERTY_AVAILABLE;
               property->multi.builtin.requested = false;
               break;
          }
          
          property->multi.builtin.requested = true;
          
          asm( "" ::: "memory" );
          
          if (++count > 1000) {
               usleep( 10000 );
               count = 0;
          }
          else {          
               direct_sched_yield();
          }
               
          if (property->multi.builtin.destroyed)
               return DR_DESTROYED;
     }
     
     if (property->multi.builtin.state == FUSION_PROPERTY_PURCHASED) {
          /* Check whether owner exited without releasing. */
          if (!(kill( property->multi.builtin.owner, 0 ) < 0 && errno == ESRCH))
               return DR_BUSY;
     }
     
     property->multi.builtin.state = FUSION_PROPERTY_PURCHASED;
     property->multi.builtin.owner = direct_gettid();
     
     asm( "" ::: "memory" );

     return DR_OK;
}

DirectResult
fusion_property_cede (FusionProperty *property)
{
     D_ASSERT( property != NULL );
     
     if (property->multi.builtin.destroyed)
          return DR_DESTROYED;

     D_ASSUME( property->multi.builtin.state != FUSION_PROPERTY_AVAILABLE );
     D_ASSUME( property->multi.builtin.owner == direct_gettid() );

     property->multi.builtin.state = FUSION_PROPERTY_AVAILABLE;
     property->multi.builtin.owner = 0;
     
     asm( "" ::: "memory" );

     if (property->multi.builtin.requested) {
          property->multi.builtin.requested = false;
          asm( "" ::: "memory" );
          direct_sched_yield();
     }

     return DR_OK;
}

DirectResult
fusion_property_holdup (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     if (property->multi.builtin.destroyed)
          return DR_DESTROYED;

     if (property->multi.builtin.state == FUSION_PROPERTY_PURCHASED &&
         property->multi.builtin.owner != direct_gettid()) {
          pid_t pid = property->multi.builtin.owner;
          
          if (kill( pid, SIGKILL ) < 0 && errno != ESRCH)
               return DR_UNSUPPORTED;

          /* Wait process termination. */
          while (kill( pid, 0 ) == 0) {
               if (property->multi.builtin.destroyed)
                    return DR_DESTROYED;
               
               direct_sched_yield();
          }
          
          property->multi.builtin.state = FUSION_PROPERTY_AVAILABLE;
          property->multi.builtin.owner = 0;
          property->multi.builtin.requested = false;
     } 

     return DR_OK;
}

DirectResult
fusion_property_destroy (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     if (property->multi.builtin.destroyed)
          return DR_DESTROYED;
     
     property->multi.builtin.destroyed = true;

     return DR_OK;
}

#endif /* FUSION_BUILD_KERNEL */

#else /* FUSION_BUILD_MULTI */

#include <pthread.h>

/*
 * Initializes the property
 */
DirectResult
fusion_property_init (FusionProperty *property, const FusionWorld *world)
{
     D_ASSERT( property != NULL );

     direct_util_recursive_pthread_mutex_init (&property->single.lock);
     pthread_cond_init (&property->single.cond, NULL);

     property->single.state = FUSION_PROPERTY_AVAILABLE;

     return DR_OK;
}

/*
 * Lease the property causing others to wait before leasing or purchasing.
 */
DirectResult
fusion_property_lease (FusionProperty *property)
{
     DirectResult ret = DR_OK;

     D_ASSERT( property != NULL );

     pthread_mutex_lock (&property->single.lock);

     /* Wait as long as the property is leased by another party. */
     while (property->single.state == FUSION_PROPERTY_LEASED)
          pthread_cond_wait (&property->single.cond, &property->single.lock);

     /* Fail if purchased by another party, otherwise succeed. */
     if (property->single.state == FUSION_PROPERTY_PURCHASED)
          ret = DR_BUSY;
     else
          property->single.state = FUSION_PROPERTY_LEASED;

     pthread_mutex_unlock (&property->single.lock);

     return ret;
}

/*
 * Purchase the property disallowing others to lease or purchase it.
 */
DirectResult
fusion_property_purchase (FusionProperty *property)
{
     DirectResult ret = DR_OK;

     D_ASSERT( property != NULL );

     pthread_mutex_lock (&property->single.lock);

     /* Wait as long as the property is leased by another party. */
     while (property->single.state == FUSION_PROPERTY_LEASED)
          pthread_cond_wait (&property->single.cond, &property->single.lock);

     /* Fail if purchased by another party, otherwise succeed. */
     if (property->single.state == FUSION_PROPERTY_PURCHASED)
          ret = DR_BUSY;
     else {
          property->single.state = FUSION_PROPERTY_PURCHASED;

          /* Wake up any other waiting party. */
          pthread_cond_broadcast (&property->single.cond);
     }

     pthread_mutex_unlock (&property->single.lock);

     return ret;
}

/*
 * Cede the property allowing others to lease or purchase it.
 */
DirectResult
fusion_property_cede (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     pthread_mutex_lock (&property->single.lock);

     /* Simple error checking, maybe we should also check the owner. */
     D_ASSERT( property->single.state != FUSION_PROPERTY_AVAILABLE );

     /* Put back into 'available' state. */
     property->single.state = FUSION_PROPERTY_AVAILABLE;

     /* Wake up one waiting party if there are any. */
     pthread_cond_signal (&property->single.cond);

     pthread_mutex_unlock (&property->single.lock);

     return DR_OK;
}

/*
 * Does nothing to avoid killing ourself.
 */
DirectResult
fusion_property_holdup (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     return DR_OK;
}

/*
 * Destroys the property
 */
DirectResult
fusion_property_destroy (FusionProperty *property)
{
     D_ASSERT( property != NULL );

     pthread_cond_destroy (&property->single.cond);
     pthread_mutex_destroy (&property->single.lock);

     return DR_OK;
}

#endif

