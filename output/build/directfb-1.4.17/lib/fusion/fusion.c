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
#include <signal.h>
#include <errno.h>
#include <time.h>
#include <fcntl.h>

#include <sys/param.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/utsname.h>

#include <direct/clock.h>
#include <direct/debug.h>
#include <direct/direct.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/signals.h>
#include <direct/thread.h>
#include <direct/trace.h>
#include <direct/util.h>

#include <fusion/build.h>
#include <fusion/conf.h>
#include <fusion/types.h>

#include "fusion_internal.h"

#include <fusion/shmalloc.h>

#include <fusion/shm/shm.h>


#if FUSION_BUILD_MULTI

D_DEBUG_DOMAIN( Fusion_Main,          "Fusion/Main",          "Fusion - High level IPC" );
D_DEBUG_DOMAIN( Fusion_Main_Dispatch, "Fusion/Main/Dispatch", "Fusion - High level IPC Dispatch" );

/**********************************************************************************************************************/

static void                      *fusion_dispatch_loop ( DirectThread *thread,
                                                         void         *arg );

/**********************************************************************************************************************/

static void                       fusion_fork_handler_prepare( void );
static void                       fusion_fork_handler_parent( void );
static void                       fusion_fork_handler_child( void );

/**********************************************************************************************************************/

static FusionWorld     *fusion_worlds[FUSION_MAX_WORLDS];
static pthread_mutex_t  fusion_worlds_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t   fusion_init_once   = PTHREAD_ONCE_INIT;

/**********************************************************************************************************************/

int
_fusion_fd( const FusionWorldShared *shared )
{
     int          index;
     FusionWorld *world;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     index = shared->world_index;

     D_ASSERT( index >= 0 );
     D_ASSERT( index < FUSION_MAX_WORLDS );

     world = fusion_worlds[index];

     D_MAGIC_ASSERT( world, FusionWorld );

     return world->fusion_fd;
}

FusionID
_fusion_id( const FusionWorldShared *shared )
{
     int          index;
     FusionWorld *world;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     index = shared->world_index;

     D_ASSERT( index >= 0 );
     D_ASSERT( index < FUSION_MAX_WORLDS );

     world = fusion_worlds[index];

     D_MAGIC_ASSERT( world, FusionWorld );

     return world->fusion_id;
}

FusionWorld *
_fusion_world( const FusionWorldShared *shared )
{
     int          index;
     FusionWorld *world;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     index = shared->world_index;

     D_ASSERT( index >= 0 );
     D_ASSERT( index < FUSION_MAX_WORLDS );

     world = fusion_worlds[index];

     D_MAGIC_ASSERT( world, FusionWorld );

     return world;
}

/**********************************************************************************************************************/

static void
init_once( void )
{
     struct utsname uts;
     int            i, j, k, l;

     pthread_atfork( fusion_fork_handler_prepare, fusion_fork_handler_parent, fusion_fork_handler_child );

     if (uname( &uts ) < 0) {
          D_PERROR( "Fusion/Init: uname() failed!\n" );
          return;
     }
     
#if !FUSION_BUILD_KERNEL
     D_INFO( "Fusion/Init: "
             "Builtin Implementation is still experimental! Crash/Deadlocks might occur!\n" );
#endif

     if (fusion_config->madv_remove_force) {
          if (fusion_config->madv_remove)
               D_INFO( "Fusion/SHM: Using MADV_REMOVE (forced)\n" );
          else
               D_INFO( "Fusion/SHM: Not using MADV_REMOVE (forced)!\n" );
     }
     else {
          switch (sscanf( uts.release, "%d.%d.%d.%d", &i, &j, &k, &l )) {
               case 3:
                    l = 0;
               case 4:
                    if (((i << 24) | (j << 16) | (k << 8) | l) >= 0x02061302)
                         fusion_config->madv_remove = true;
                    break;

               default:
                    D_WARN( "could not parse kernel version '%s'", uts.release );
          }

          if (fusion_config->madv_remove)
               D_INFO( "Fusion/SHM: Using MADV_REMOVE (%d.%d.%d.%d >= 2.6.19.2)\n", i, j, k, l );
          else
               D_INFO( "Fusion/SHM: NOT using MADV_REMOVE (%d.%d.%d.%d < 2.6.19.2)! [0x%08x]\n",
                       i, j, k, l, (i << 24) | (j << 16) | (k << 8) | l );
     }
}

/**********************************************************************************************************************/

#if FUSION_BUILD_KERNEL

static void
fusion_world_fork( FusionWorld *world )
{
     int                fd;
     char               buf1[20];
     char               buf2[20];
     FusionEnter        enter;
     FusionFork         fork;
     FusionWorldShared *shared;

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     snprintf( buf1, sizeof(buf1), "/dev/fusion%d", shared->world_index );
     snprintf( buf2, sizeof(buf2), "/dev/fusion/%d", shared->world_index );

     /* Open Fusion Kernel Device. */
     fd = direct_try_open( buf1, buf2, O_RDWR | O_NONBLOCK, true );
     if (fd < 0) {
          D_PERROR( "Fusion/Main: Reopening fusion device (world %d) failed!\n", shared->world_index );
          raise(5);
     }

     /* Drop "identity" when running another program. */
     if (fcntl( fd, F_SETFD, FD_CLOEXEC ) < 0)
          D_PERROR( "Fusion/Init: Setting FD_CLOEXEC flag failed!\n" );

     /* Fill enter information. */
     enter.api.major = FUSION_API_MAJOR_REQUIRED;
     enter.api.minor = FUSION_API_MINOR_REQUIRED;
     enter.fusion_id = 0;     /* Clear for check below. */

     /* Enter the fusion world. */
     while (ioctl( fd, FUSION_ENTER, &enter )) {
          if (errno != EINTR) {
               D_PERROR( "Fusion/Init: Could not reenter world '%d'!\n", shared->world_index );
               raise(5);
          }
     }

     /* Check for valid Fusion ID. */
     if (!enter.fusion_id) {
          D_ERROR( "Fusion/Init: Got no ID from FUSION_ENTER! Kernel module might be too old.\n" );
          raise(5);
     }

     D_DEBUG_AT( Fusion_Main, "  -> Fusion ID 0x%08lx\n", enter.fusion_id );


     /* Fill fork information. */
     fork.fusion_id = world->fusion_id;

     fusion_world_flush_calls( world, 1 );

     /* Fork within the fusion world. */
     while (ioctl( fd, FUSION_FORK, &fork )) {
          if (errno != EINTR) {
               D_PERROR( "Fusion/Main: Could not fork in world '%d'!\n", shared->world_index );
               raise(5);
          }
     }

     D_DEBUG_AT( Fusion_Main, "  -> Fusion ID 0x%08lx\n", fork.fusion_id );

     /* Get new fusion id back. */
     world->fusion_id = fork.fusion_id;

     /* Close old file descriptor. */
     close( world->fusion_fd );

     /* Write back new file descriptor. */
     world->fusion_fd = fd;


     D_DEBUG_AT( Fusion_Main, "  -> restarting dispatcher loop...\n" );

     /* Restart the dispatcher thread. FIXME: free old struct */
     world->dispatch_loop = direct_thread_create( DTT_MESSAGING,
                                                  fusion_dispatch_loop,
                                                  world, "Fusion Dispatch" );
     if (!world->dispatch_loop)
          raise(5);
}

static void
fusion_fork_handler_prepare( void )
{
     int i;
     
     D_DEBUG_AT( Fusion_Main, "%s()\n", __FUNCTION__ );
     
     for (i=0; i<FUSION_MAX_WORLDS; i++) {
          FusionWorld *world = fusion_worlds[i];

          if (!world)
               continue;
               
          D_MAGIC_ASSERT( world, FusionWorld );
          
          if (world->fork_callback)
               world->fork_callback( world->fork_action, FFS_PREPARE );
     }
}     

static void
fusion_fork_handler_parent( void )
{
     int i;

     D_DEBUG_AT( Fusion_Main, "%s()\n", __FUNCTION__ );
    
     for (i=0; i<FUSION_MAX_WORLDS; i++) {
          FusionWorld       *world = fusion_worlds[i];
          FusionWorldShared *shared;

          if (!world)
               continue;
               
          D_MAGIC_ASSERT( world, FusionWorld );
          
          shared = world->shared;
          
          D_MAGIC_ASSERT( shared, FusionWorldShared );
          
          if (world->fork_callback)
               world->fork_callback( world->fork_action, FFS_PARENT );
               
          if (world->fork_action == FFA_FORK) {
               /* Increase the shared reference counter. */
               if (fusion_master( world ))
                    shared->refs++;
          }
     }
}

static void
fusion_fork_handler_child( void )
{
     int i;

     D_DEBUG_AT( Fusion_Main, "%s()\n", __FUNCTION__ );

     for (i=0; i<FUSION_MAX_WORLDS; i++) {
          FusionWorld *world = fusion_worlds[i];

          if (!world)
               continue;

          D_MAGIC_ASSERT( world, FusionWorld );
          
          if (world->fork_callback)
               world->fork_callback( world->fork_action, FFS_CHILD );

          switch (world->fork_action) {
               default:
                    D_BUG( "unknown fork action %d", world->fork_action );

               case FFA_CLOSE:
                    D_DEBUG_AT( Fusion_Main, "  -> closing world %d\n", i );

                    /* Remove world from global list. */
                    fusion_worlds[i] = NULL;

                    /* Unmap shared area. */
                    munmap( world->shared, sizeof(FusionWorldShared) );

                    /* Close Fusion Kernel Device. */
                    close( world->fusion_fd );

                    /* Free local world data. */
                    D_MAGIC_CLEAR( world );
                    D_FREE( world );

                    break;

               case FFA_FORK:
                    D_DEBUG_AT( Fusion_Main, "  -> forking in world %d\n", i );

                    fusion_world_fork( world );

                    break;
          }
     }
}

/**********************************************************************************************************************/

static DirectResult
map_shared_root( void               *shm_base,
                 int                 world_index,
                 bool                master,
                 FusionWorldShared **ret_shared )
{
     DirectResult ret = DR_OK;
     int          fd;
     void        *map;
     char         tmpfs[FUSION_SHM_TMPFS_PATH_NAME_LEN];
     char         root_file[FUSION_SHM_TMPFS_PATH_NAME_LEN+32];
     int          flags = O_RDONLY;
     int          prot  = PROT_READ;

     if (master || !fusion_config->secure_fusion) {
          prot  |= PROT_WRITE;
          flags  = O_RDWR;
     }

     if (master)
          flags |= O_CREAT | O_TRUNC;

     if (fusion_config->tmpfs) {
          direct_snputs( tmpfs, fusion_config->tmpfs, FUSION_SHM_TMPFS_PATH_NAME_LEN );
     }
     else if (!fusion_find_tmpfs( tmpfs, FUSION_SHM_TMPFS_PATH_NAME_LEN )) {
          D_ERROR( "Fusion/SHM: Could not find tmpfs mount point, falling back to /dev/shm!\n" );
          direct_snputs( tmpfs, "/dev/shm", FUSION_SHM_TMPFS_PATH_NAME_LEN );
     }

     snprintf( root_file, sizeof(root_file), "%s/fusion.%d", tmpfs, world_index );

     /* open the virtual file */
     fd = open( root_file, flags, 0660 );
     if (fd < 0) {
          ret = errno2result(errno);
          D_PERROR( "Fusion/SHM: Could not open shared memory file '%s'!\n", root_file );
          return ret;
     }

     if (fusion_config->shmfile_gid != (gid_t)-1) {
          /* chgrp the SH_FILE dev entry */
          if (fchown( fd, -1, fusion_config->shmfile_gid ) != 0)
               D_WARN( "Fusion/SHM: Changing owner on %s failed... continuing on.", root_file );
     }

     if (master) {
          fchmod( fd, fusion_config->secure_fusion ? 0640 : 0660 );
          ftruncate( fd, sizeof(FusionWorldShared) );
     }

     D_DEBUG_AT( Fusion_Main, "  -> mmaping shared memory file... (%zu bytes)\n", sizeof(FusionWorldShared) );



     /* Map shared area. */
     map = mmap( shm_base + 0x10000 * world_index, sizeof(FusionWorldShared),
                 prot, MAP_FIXED | MAP_SHARED, fd, 0 );
     if (map == MAP_FAILED) {
          ret = errno2result(errno);
          D_PERROR( "Fusion/Init: Mapping shared area failed!\n" );
          goto out;
     }

     *ret_shared = map;


out:
     close( fd );

     return ret;
}

/**********************************************************************************************************************/

/*
 * Enters a fusion world by joining or creating it.
 *
 * If <b>world</b> is negative, the next free index is used to create a new world.
 * Otherwise the world with the specified index is joined or created.
 */
DirectResult
fusion_enter( int               world_index,
              int               abi_version,
              FusionEnterRole   role,
              FusionWorld     **ret_world )
{
     DirectResult       ret    = DR_OK;
     int                fd     = -1;
     FusionWorld       *world  = NULL;
     FusionWorldShared *shared = NULL;
     FusionEnter        enter;
     char               buf1[20];
     char               buf2[20];

     D_DEBUG_AT( Fusion_Main, "%s( %d, %d, %p )\n", __FUNCTION__, world_index, abi_version, ret_world );

     D_ASSERT( ret_world != NULL );

     if (world_index >= FUSION_MAX_WORLDS) {
          D_ERROR( "Fusion/Init: World index %d exceeds maximum index %d!\n", world_index, FUSION_MAX_WORLDS - 1 );
          return DR_INVARG;
     }

     pthread_once( &fusion_init_once, init_once );


     if (fusion_config->force_slave)
          role = FER_SLAVE;

     direct_initialize();

     pthread_mutex_lock( &fusion_worlds_lock );


     if (world_index < 0) {
          if (role == FER_SLAVE) {
               D_ERROR( "Fusion/Init: Slave role and a new world (index -1) was requested!\n" );
               pthread_mutex_unlock( &fusion_worlds_lock );
               return DR_INVARG;
          }

          for (world_index=0; world_index<FUSION_MAX_WORLDS; world_index++) {
               world = fusion_worlds[world_index];
               if (world)
                    break;

               snprintf( buf1, sizeof(buf1), "/dev/fusion%d", world_index );
               snprintf( buf2, sizeof(buf2), "/dev/fusion/%d", world_index );

               /* Open Fusion Kernel Device. */
               fd = direct_try_open( buf1, buf2, O_RDWR | O_NONBLOCK | O_EXCL, false );
               if (fd < 0) {
                    if (errno != EBUSY)
                         D_PERROR( "Fusion/Init: Error opening '%s' and/or '%s'!\n", buf1, buf2 );
               }
               else
                    break;
          }
     }
     else {
          world = fusion_worlds[world_index];
          if (!world) {
               int flags = O_RDWR | O_NONBLOCK;

               snprintf( buf1, sizeof(buf1), "/dev/fusion%d", world_index );
               snprintf( buf2, sizeof(buf2), "/dev/fusion/%d", world_index );

               if (role == FER_MASTER)
                    flags |= O_EXCL;
               else if (role == FER_SLAVE)
                    flags |= O_APPEND;

               /* Open Fusion Kernel Device. */
               fd = direct_try_open( buf1, buf2, flags, true );
          }
     }

     /* Enter a world again? */
     if (world) {
          D_MAGIC_ASSERT( world, FusionWorld );
          D_ASSERT( world->refs > 0 );

          /* Check the role again. */
          switch (role) {
               case FER_MASTER:
                    if (world->fusion_id != FUSION_ID_MASTER) {
                         D_ERROR( "Fusion/Init: Master role requested for a world (%d) "
                                  "we're already slave in!\n", world_index );
                         ret = DR_UNSUPPORTED;
                         goto error;
                    }
                    break;

               case FER_SLAVE:
                    if (world->fusion_id == FUSION_ID_MASTER) {
                         D_ERROR( "Fusion/Init: Slave role requested for a world (%d) "
                                  "we're already master in!\n", world_index );
                         ret = DR_UNSUPPORTED;
                         goto error;
                    }
                    break;

               case FER_ANY:
                    break;
          }

          shared = world->shared;

          D_MAGIC_ASSERT( shared, FusionWorldShared );

          if (shared->world_abi != abi_version) {
               D_ERROR( "Fusion/Init: World ABI (%d) of world '%d' doesn't match own (%d)!\n",
                        shared->world_abi, world_index, abi_version );
               ret = DR_VERSIONMISMATCH;
               goto error;
          }

          world->refs++;

          pthread_mutex_unlock( &fusion_worlds_lock );

          D_DEBUG_AT( Fusion_Main, "  -> using existing world %p [%d]\n", world, world_index );

          /* Return the world. */
          *ret_world = world;

          return DR_OK;
     }

     if (fd < 0) {
          D_PERROR( "Fusion/Init: Opening fusion device (world %d) as '%s' failed!\n", world_index,
                    role == FER_ANY ? "any" : (role == FER_MASTER ? "master" : "slave")  );
          ret = DR_INIT;
          goto error;
     }

     /* Drop "identity" when running another program. */
     if (fcntl( fd, F_SETFD, FD_CLOEXEC ) < 0)
          D_PERROR( "Fusion/Init: Setting FD_CLOEXEC flag failed!\n" );

     /* Fill enter information. */
     enter.api.major = FUSION_API_MAJOR_REQUIRED;
     enter.api.minor = FUSION_API_MINOR_REQUIRED;
     enter.fusion_id = 0;     /* Clear for check below. */
     enter.secure    = fusion_config->secure_fusion;

     /* Enter the fusion world. */
     while (ioctl( fd, FUSION_ENTER, &enter )) {
          if (errno != EINTR) {
               D_PERROR( "Fusion/Init: Could not enter world '%d'!\n", world_index );
               ret = DR_INIT;
               goto error;
          }
     }

     /* Check for valid Fusion ID. */
     if (!enter.fusion_id) {
          D_ERROR( "Fusion/Init: Got no ID from FUSION_ENTER! Kernel module might be too old.\n" );
          ret = DR_INIT;
          goto error;
     }

     D_DEBUG_AT( Fusion_Main, "  -> Fusion ID 0x%08lx\n", enter.fusion_id );

     /* Check slave role only, master is handled by O_EXCL earlier. */
     if (role == FER_SLAVE && enter.fusion_id == FUSION_ID_MASTER) {
          D_PERROR( "Fusion/Init: Entering world '%d' as a slave failed!\n", world_index );
          ret = DR_UNSUPPORTED;
          goto error;
     }


     unsigned long shm_base;

     if (ioctl( fd, FUSION_SHM_GET_BASE, &shm_base )) {
          ret = errno2result(errno);
          D_PERROR( "Fusion/Init: FUSION_SHM_GET_BASE failed!\n" );
          goto error;
     }


     /* Map shared area. */
     ret = map_shared_root( (void*) shm_base, world_index, enter.fusion_id == FUSION_ID_MASTER, &shared );
     if (ret)
          goto error;

     D_DEBUG_AT( Fusion_Main, "  -> shared area at %p, size %zu\n", shared, sizeof(FusionWorldShared) );

     /* Initialize shared data. */
     if (enter.fusion_id == FUSION_ID_MASTER) {
          /* Initialize reference counter. */
          shared->refs = 1;
          
          /* Set ABI version. */
          shared->world_abi = abi_version;

          /* Set the world index. */
          shared->world_index = world_index;

          /* Set start time of world clock. */
          gettimeofday( &shared->start_time, NULL );

          D_MAGIC_SET( shared, FusionWorldShared );
     }
     else {
          D_MAGIC_ASSERT( shared, FusionWorldShared );

          /* Check ABI version. */
          if (shared->world_abi != abi_version) {
               D_ERROR( "Fusion/Init: World ABI (%d) doesn't match own (%d)!\n",
                        shared->world_abi, abi_version );
               ret = DR_VERSIONMISMATCH;
               goto error;
          }
     }

     /* Synchronize to world clock. */
     direct_clock_set_start( &shared->start_time );
     

     /* Allocate local data. */
     world = D_CALLOC( 1, sizeof(FusionWorld) );
     if (!world) {
          ret = D_OOM();
          goto error;
     }

     /* Initialize local data. */
     world->refs      = 1;
     world->shared    = shared;
     world->fusion_fd = fd;
     world->fusion_id = enter.fusion_id;

     direct_mutex_init( &world->reactor_nodes_lock );
     direct_mutex_init( &world->bins_lock );

     D_MAGIC_SET( world, FusionWorld );

     fusion_worlds[world_index] = world;

     /* Initialize shared memory part. */
     ret = fusion_shm_init( world );
     if (ret)
          goto error2;


     D_DEBUG_AT( Fusion_Main, "  -> initializing other parts...\n" );

     /* Initialize other parts. */
     if (enter.fusion_id == FUSION_ID_MASTER) {
          fusion_skirmish_init( &shared->arenas_lock, "Fusion Arenas", world );
          fusion_skirmish_init( &shared->reactor_globals, "Fusion Reactor Globals", world );

          fusion_skirmish_add_permissions( &shared->arenas_lock, 0, FUSION_SKIRMISH_PERMIT_PREVAIL | FUSION_SKIRMISH_PERMIT_DISMISS );
          fusion_skirmish_add_permissions( &shared->reactor_globals, 0, FUSION_SKIRMISH_PERMIT_PREVAIL | FUSION_SKIRMISH_PERMIT_DISMISS );

          /* Create the main pool. */
          ret = fusion_shm_pool_create( world, "Fusion Main Pool", 0x1000000,
                                        fusion_config->debugshm, &shared->main_pool );
          if (ret)
               goto error3;
     }


     D_DEBUG_AT( Fusion_Main, "  -> starting dispatcher loop...\n" );

     /* Start the dispatcher thread. */
     world->dispatch_loop = direct_thread_create( DTT_MESSAGING,
                                                  fusion_dispatch_loop,
                                                  world, "Fusion Dispatch" );
     if (!world->dispatch_loop) {
          ret = DR_FAILURE;
          goto error4;
     }


     /* Let others enter the world. */
     if (enter.fusion_id == FUSION_ID_MASTER) {
          D_DEBUG_AT( Fusion_Main, "  -> unblocking world...\n" );

          while (ioctl( fd, FUSION_UNBLOCK )) {
               if (errno != EINTR) {
                    D_PERROR( "Fusion/Init: Could not unblock world!\n" );
                    ret = DR_FUSION;
                    goto error4;
               }
          }
     }

     D_DEBUG_AT( Fusion_Main, "  -> done. (%p)\n", world );

     pthread_mutex_unlock( &fusion_worlds_lock );

     /* Return the fusion world. */
     *ret_world = world;

     return DR_OK;


error4:
     if (world->dispatch_loop)
          direct_thread_destroy( world->dispatch_loop );

     if (enter.fusion_id == FUSION_ID_MASTER)
          fusion_shm_pool_destroy( world, shared->main_pool );

error3:
     if (enter.fusion_id == FUSION_ID_MASTER) {
          fusion_skirmish_destroy( &shared->arenas_lock );
          fusion_skirmish_destroy( &shared->reactor_globals );
     }

     fusion_shm_deinit( world );


error2:
     fusion_worlds[world_index] = world;

     D_MAGIC_CLEAR( world );

     D_FREE( world );

error:
     if (shared && shared != MAP_FAILED) {
          if (enter.fusion_id == FUSION_ID_MASTER)
               D_MAGIC_CLEAR( shared );

          munmap( shared, sizeof(FusionWorldShared) );
     }

     if (fd != -1)
          close( fd );

     pthread_mutex_unlock( &fusion_worlds_lock );

     direct_shutdown();

     return ret;
}

DirectResult
fusion_stop_dispatcher( FusionWorld *world,
                        bool         emergency )
{
     if (!emergency) {
          fusion_sync( world );

          direct_thread_lock( world->dispatch_loop );
     }
          
     world->dispatch_stop = true;

     if (!emergency) {
          direct_thread_unlock( world->dispatch_loop );

          fusion_sync( world );
     }

     return DR_OK;
}

/*
 * Exits the fusion world.
 *
 * If 'emergency' is true the function won't join but kill the dispatcher thread.
 */
DirectResult
fusion_exit( FusionWorld *world,
             bool         emergency )
{
     FusionWorldShared *shared;

     D_DEBUG_AT( Fusion_Main, "%s( %p, %semergency )\n", __FUNCTION__, world, emergency ? "" : "no " );

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );


     pthread_mutex_lock( &fusion_worlds_lock );

     D_ASSERT( world->refs > 0 );

     if (--world->refs) {
          pthread_mutex_unlock( &fusion_worlds_lock );
          return DR_OK;
     }

     if (!emergency) {
          int               foo;
          FusionSendMessage msg;

          /* Wake up the read loop thread. */
          msg.fusion_id = world->fusion_id;
          msg.msg_id    = 0;
          msg.msg_data  = &foo;
          msg.msg_size  = sizeof(foo);

          fusion_world_flush_calls( world, 1 );

          while (ioctl( world->fusion_fd, FUSION_SEND_MESSAGE, &msg ) < 0) {
               if (errno != EINTR) {
                    D_PERROR( "FUSION_SEND_MESSAGE" );
                    direct_thread_cancel( world->dispatch_loop );
                    break;
               }
          }

          /* Wait for its termination. */
          direct_thread_join( world->dispatch_loop );
     }

     direct_thread_destroy( world->dispatch_loop );

     /* Master has to deinitialize shared data. */
     if (fusion_master( world )) {
          shared->refs--;
          if (shared->refs == 0) {
               fusion_skirmish_destroy( &shared->reactor_globals );
               fusion_skirmish_destroy( &shared->arenas_lock );

               fusion_shm_pool_destroy( world, shared->main_pool );
          
               /* Deinitialize shared memory. */
               fusion_shm_deinit( world );
          }
     }
     else {
          /* Leave shared memory. */
          fusion_shm_deinit( world );
     }

     /* Reset local dispatch nodes. */
     _fusion_reactor_free_all( world );


     /* Remove world from global list. */
     fusion_worlds[shared->world_index] = NULL;


     /* Unmap shared area. */
     if (fusion_master( world ) && shared->refs == 0) {
          char         tmpfs[FUSION_SHM_TMPFS_PATH_NAME_LEN];
          char         root_file[FUSION_SHM_TMPFS_PATH_NAME_LEN+32];

          if (fusion_config->tmpfs)
               direct_snputs( tmpfs, fusion_config->tmpfs, FUSION_SHM_TMPFS_PATH_NAME_LEN );
          else if (!fusion_find_tmpfs( tmpfs, FUSION_SHM_TMPFS_PATH_NAME_LEN ))
               direct_snputs( tmpfs, "/dev/shm", FUSION_SHM_TMPFS_PATH_NAME_LEN );

          snprintf( root_file, sizeof(root_file), "%s/fusion.%d", tmpfs, shared->world_index );

          if (unlink( root_file ) < 0)
               D_PERROR( "Fusion/Main: could not unlink shared memory file (%s)!\n", root_file );

          D_MAGIC_CLEAR( shared );
     }

     munmap( shared, sizeof(FusionWorldShared) );


     /* Close Fusion Kernel Device. */
     close( world->fusion_fd );


     /* Free local world data. */
     D_MAGIC_CLEAR( world );

     D_FREE( world );


     pthread_mutex_unlock( &fusion_worlds_lock );

     direct_shutdown();

     return DR_OK;
}

/*
 * Sends a signal to one or more fusionees and optionally waits
 * for their processes to terminate.
 *
 * A fusion_id of zero means all fusionees but the calling one.
 * A timeout of zero means infinite waiting while a negative value
 * means no waiting at all.
 */
DirectResult
fusion_kill( FusionWorld *world,
             FusionID     fusion_id,
             int          signal,
             int          timeout_ms )
{
     FusionKill param;

     D_MAGIC_ASSERT( world, FusionWorld );

     param.fusion_id  = fusion_id;
     param.signal     = signal;
     param.timeout_ms = timeout_ms;

     fusion_world_flush_calls( world, 1 );

     while (ioctl( world->fusion_fd, FUSION_KILL, &param )) {
          switch (errno) {
               case EINTR:
                    continue;
               case ETIMEDOUT:
                    return DR_TIMEOUT;
               default:
                    break;
          }

          D_PERROR ("FUSION_KILL");

          return DR_FAILURE;
     }

     return DR_OK;
}

const char *
fusion_get_tmpfs( FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );
     D_MAGIC_ASSERT( world->shared, FusionWorldShared );

     return world->shared->shm.tmpfs;
}

/**********************************************************************************************************************/

static void *
fusion_dispatch_loop( DirectThread *thread, void *arg )
{
     int          len = 0;
     int          result;
     char         buf[FUSION_MESSAGE_SIZE];
     FusionWorld *world = arg;

     D_DEBUG_AT( Fusion_Main_Dispatch, "%s() running...\n", __FUNCTION__ );

     while (true) {
          char *buf_p = buf;

          D_MAGIC_ASSERT( world, FusionWorld );

          len = read( world->fusion_fd, buf, FUSION_MESSAGE_SIZE );
          if (len < 0)
               break;

          D_DEBUG_AT( Fusion_Main_Dispatch, "  -> got %d bytes...\n", len );

          direct_thread_lock( world->dispatch_loop );

          if (world->dispatch_stop) {
               D_DEBUG_AT( Fusion_Main_Dispatch, "  -> IGNORING (dispatch_stop!)\n" );
          }
          else {
               while (buf_p < buf + len) {
                    FusionReadMessage *header = (FusionReadMessage*) buf_p;
                    void              *data   = buf_p + sizeof(FusionReadMessage);

                    if (world->dispatch_stop) {
                         D_DEBUG_AT( Fusion_Main_Dispatch, "  -> ABORTING (dispatch_stop!)\n" );
                         break;
                    }

                    D_MAGIC_ASSERT( world, FusionWorld );
                    D_ASSERT( (buf + len - buf_p) >= sizeof(FusionReadMessage) );

                    switch (header->msg_type) {
                         case FMT_SEND:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_SEND!\n" );
                              break; 
                         case FMT_CALL:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_CALL...\n" );
                              _fusion_call_process( world, header->msg_id, data,
                                                    (header->msg_size != sizeof(FusionCallMessage))
                                                    ? data + sizeof(FusionCallMessage) : NULL );
                              break;
                         case FMT_REACTOR:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_REACTOR...\n" );
                              _fusion_reactor_process_message( world, header->msg_id, header->msg_channel, data );
                              break;
                         case FMT_SHMPOOL:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_SHMPOOL...\n" );
                              _fusion_shmpool_process( world, header->msg_id, data );
                              break;
                         case FMT_CALL3:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_CALL3...\n" );
                              _fusion_call_process3( world, header->msg_id, data,
                                                     (header->msg_size != sizeof(FusionCallMessage3))
                                                     ?data + sizeof(FusionCallMessage3) : NULL );
                              break;
                         case FMT_LEAVE:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_LEAVE...\n" );

                              if (world->leave_callback)
                                   world->leave_callback( world, *((FusionID*)data), world->leave_ctx );
                              break;
                         default:
                              D_DEBUG( "Fusion/Receiver: discarding message of unknown type '%d'\n",
                                       header->msg_type );
                              break;
                    }

                    buf_p = data + ((header->msg_size + 3) & ~3);
               }
          }

          direct_thread_unlock( world->dispatch_loop );

          if (!world->refs) {
               D_DEBUG_AT( Fusion_Main_Dispatch, "  -> good bye!\n" );
               return NULL;
          }
     }

     return NULL;
}

/**********************************************************************************************************************/

DirectResult
fusion_get_fusionee_path( const FusionWorld *world,
                          FusionID           fusion_id,
                          char              *buf,
                          size_t             buf_size,
                          size_t            *ret_size )
{
     FusionGetFusioneeInfo info;
     size_t                len;

     D_ASSERT( world != NULL );
     D_ASSERT( buf != NULL );
     D_ASSERT( ret_size != NULL );

     info.fusion_id = fusion_id;

     while (ioctl( world->fusion_fd, FUSION_GET_FUSIONEE_INFO, &info ) < 0) {
          switch (errno) {
               case EINTR:
                    continue;

               default:
                    break;
          }

          D_PERROR( "Fusion: FUSION_GET_FUSIONEE_INFO failed!\n" );

          return DR_FUSION;
     }

     len = strlen( info.exe_file ) + 1;

     if (len > buf_size) {
          *ret_size = len;
          return DR_LIMITEXCEEDED;
     }

     memcpy( buf, info.exe_file, len );

     *ret_size = len;

     return DR_OK;
}

#else /* FUSION_BUILD_KERNEL */

#include <dirent.h>

#include <direct/system.h>

typedef struct {
     DirectLink   link;

     FusionRef   *ref;

     int          count;
} __FusioneeRef;

typedef struct {
     DirectLink   link;
     
     FusionID     id;
     pid_t        pid;

     DirectLink  *refs;
} __Fusionee;


/**********************************************************************************************************************/

static DirectResult
_fusion_add_fusionee( FusionWorld *world, FusionID fusion_id )
{
     DirectResult       ret;
     FusionWorldShared *shared;
     __Fusionee        *fusionee;

     D_DEBUG_AT( Fusion_Main, "%s( %p, %lu )\n", __FUNCTION__, world, fusion_id );

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     fusionee = SHCALLOC( shared->main_pool, 1, sizeof(__Fusionee) );
     if (!fusionee)
          return D_OOSHM();
     
     fusionee->id  = fusion_id;
     fusionee->pid = direct_gettid();
     
     ret = fusion_skirmish_prevail( &shared->fusionees_lock );
     if (ret) {
          SHFREE( shared->main_pool, fusionee );
          return ret;
     }
     
     direct_list_append( &shared->fusionees, &fusionee->link );
     
     fusion_skirmish_dismiss( &shared->fusionees_lock );

     /* Set local pointer. */
     world->fusionee = fusionee;

     return DR_OK;
}    

void
_fusion_add_local( FusionWorld *world, FusionRef *ref, int add )
{
     FusionWorldShared *shared;
     __Fusionee        *fusionee;
     __FusioneeRef     *fusionee_ref;

     //D_DEBUG_AT( Fusion_Main, "%s( %p, %p, %d )\n", __FUNCTION__, world, ref, add );

     D_ASSERT( ref != NULL );
     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;
     D_MAGIC_ASSERT( shared, FusionWorldShared );

     fusionee = world->fusionee;
     D_ASSERT( fusionee != NULL );

     direct_list_foreach (fusionee_ref, fusionee->refs) {
          if (fusionee_ref->ref == ref)
               break;
     }

     if (fusionee_ref) { 
          fusionee_ref->count += add;

          //D_DEBUG_AT( Fusion_Main, " -> refs = %d\n", fusionee_ref->count );
          
          if (fusionee_ref->count == 0) {
               direct_list_remove( &fusionee->refs, &fusionee_ref->link );

               SHFREE( shared->main_pool, fusionee_ref );
          }
     }
     else {
          if (add <= 0) /* called from _fusion_remove_fusionee() */
               return;
          
          //D_DEBUG_AT( Fusion_Main, " -> new ref\n" );
          
          fusionee_ref = SHCALLOC( shared->main_pool, 1, sizeof(__FusioneeRef) );
          if (!fusionee_ref) {
               D_OOSHM();
               return;
          }

          fusionee_ref->ref   = ref;
          fusionee_ref->count = add;

          direct_list_prepend( &fusionee->refs, &fusionee_ref->link );
     }
}

void
_fusion_check_locals( FusionWorld *world, FusionRef *ref )
{
     FusionWorldShared *shared;
     __Fusionee        *fusionee;
     __FusioneeRef     *fusionee_ref, *temp;
     DirectLink        *list = NULL;

     D_DEBUG_AT( Fusion_Main, "%s( %p, %p )\n", __FUNCTION__, world, ref );

     D_ASSERT( ref != NULL );

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     if (fusion_skirmish_prevail( &shared->fusionees_lock ))
          return;

     direct_list_foreach (fusionee, shared->fusionees) { 
          if (fusionee->id == world->fusion_id)
               continue;
          
          direct_list_foreach (fusionee_ref, fusionee->refs) {    
               if (fusionee_ref->ref == ref) {
                    if (kill( fusionee->pid, 0 ) < 0 && errno == ESRCH) { 
                         direct_list_remove( &fusionee->refs, &fusionee_ref->link );
                         direct_list_append( &list, &fusionee_ref->link );
                    }
                    break;
               }
          }
     }

     fusion_skirmish_dismiss( &shared->fusionees_lock );

     direct_list_foreach_safe (fusionee_ref, temp, list) {
          _fusion_ref_change( ref, -fusionee_ref->count, false );
          
          SHFREE( shared->main_pool, fusionee_ref );
     }
}

void
_fusion_remove_all_locals( FusionWorld *world, const FusionRef *ref )
{
     FusionWorldShared *shared;
     __Fusionee        *fusionee;
     __FusioneeRef     *fusionee_ref, *temp;

     D_DEBUG_AT( Fusion_Main, "%s( %p, %p )\n", __FUNCTION__, world, ref );

     D_ASSERT( ref != NULL );

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     if (fusion_skirmish_prevail( &shared->fusionees_lock ))
          return;

     direct_list_foreach (fusionee, shared->fusionees) {
          direct_list_foreach_safe (fusionee_ref, temp, fusionee->refs) {
               if (fusionee_ref->ref == ref) {
                    direct_list_remove( &fusionee->refs, &fusionee_ref->link );
                    
                    SHFREE( shared->main_pool, fusionee_ref );
               }
          }
     }

     fusion_skirmish_dismiss( &shared->fusionees_lock );
}

static void
_fusion_remove_fusionee( FusionWorld *world, FusionID fusion_id )
{
     FusionWorldShared *shared;
     __Fusionee        *fusionee;
     __FusioneeRef     *fusionee_ref, *temp;

     D_DEBUG_AT( Fusion_Main, "%s( %p, %lu )\n", __FUNCTION__, world, fusion_id );

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     fusion_skirmish_prevail( &shared->fusionees_lock );

     if (fusion_id == world->fusion_id) {
          fusionee = world->fusionee;
     }
     else {
          direct_list_foreach (fusionee, shared->fusionees) {
               if (fusionee->id == fusion_id)
                    break;
          }
     }

     if (!fusionee) {
          D_DEBUG_AT( Fusion_Main, "Fusionee %lu not found!\n", fusion_id );
          fusion_skirmish_dismiss( &shared->fusionees_lock );
          return;
     }

     direct_list_remove( &shared->fusionees, &fusionee->link );

     fusion_skirmish_dismiss( &shared->fusionees_lock );
     
     direct_list_foreach_safe (fusionee_ref, temp, fusionee->refs) {
          direct_list_remove( &fusionee->refs, &fusionee_ref->link );
               
          _fusion_ref_change( fusionee_ref->ref, -fusionee_ref->count, false );
          
          SHFREE( shared->main_pool, fusionee_ref );
     }

     SHFREE( shared->main_pool, fusionee );
}

/**********************************************************************************************************************/

DirectResult
_fusion_send_message( int                 fd, 
                      const void         *msg, 
                      size_t              msg_size, 
                      struct sockaddr_un *addr )
{
     socklen_t len = sizeof(struct sockaddr_un);
     
     D_ASSERT( msg != NULL );

     if (!addr) {
          addr = alloca( sizeof(struct sockaddr_un) );
          getsockname( fd, (struct sockaddr*)addr, &len );
     }          
    
     while (sendto( fd, msg, msg_size, 0, (struct sockaddr*)addr, len ) < 0) {
          switch (errno) {
               case EINTR:
                    continue;
               case ECONNREFUSED:
                    return DR_FUSION;
               default:
                    break;
          }

          D_PERROR( "Fusion: sendto()\n" );

          return DR_IO;
     }
     
     return DR_OK;
}

DirectResult
_fusion_recv_message( int                 fd, 
                      void               *msg,
                      size_t              msg_size,
                      struct sockaddr_un *addr )
{
     socklen_t len = addr ? sizeof(struct sockaddr_un) : 0;
     
     D_ASSERT( msg != NULL );
    
     while (recvfrom( fd, msg, msg_size, 0, (struct sockaddr*)addr, &len ) < 0) {
          switch (errno) {
               case EINTR:
                    continue;
               case ECONNREFUSED:
                    return DR_FUSION;
               default:
                    break;
          }

          D_PERROR( "Fusion: recvfrom()\n" );
          
          return DR_IO;
     }
     
     return DR_OK;
}

/**********************************************************************************************************************/

static void
fusion_fork_handler_prepare( void )
{
     int i;
     
     D_DEBUG_AT( Fusion_Main, "%s()\n", __FUNCTION__ );
     
     for (i=0; i<FUSION_MAX_WORLDS; i++) {
          FusionWorld *world = fusion_worlds[i];

          if (!world)
               continue;
               
          D_MAGIC_ASSERT( world, FusionWorld );
          
          if (world->fork_callback)
               world->fork_callback( world->fork_action, FFS_PREPARE );
     }
}

static void
fusion_fork_handler_parent( void )
{
     int i;

     D_DEBUG_AT( Fusion_Main, "%s()\n", __FUNCTION__ );
    
     for (i=0; i<FUSION_MAX_WORLDS; i++) {
          FusionWorld        *world = fusion_worlds[i];
           FusionWorldShared *shared;

          if (!world)
               continue;

          D_MAGIC_ASSERT( world, FusionWorld );
          
          shared = world->shared;
          
          D_MAGIC_ASSERT( shared, FusionWorldShared );
          
          if (world->fork_callback)
               world->fork_callback( world->fork_action, FFS_PARENT );
               
          if (world->fork_action == FFA_FORK) {
               /* Increase the shared reference counter. */
               if (fusion_master( world ))
                    shared->refs++;
               
               /* Cancel the dispatcher to prevent conflicts. */
               direct_thread_cancel( world->dispatch_loop );
          }
     }
}

static void
fusion_fork_handler_child( void )
{
     int i;

     D_DEBUG_AT( Fusion_Main, "%s()\n", __FUNCTION__ );

     for (i=0; i<FUSION_MAX_WORLDS; i++) {
          FusionWorld       *world = fusion_worlds[i];
          FusionWorldShared *shared;

          if (!world)
               continue;

          D_MAGIC_ASSERT( world, FusionWorld );

          shared = world->shared;

          D_MAGIC_ASSERT( shared, FusionWorldShared );
          
          if (world->fork_callback)
               world->fork_callback( world->fork_action, FFS_CHILD );

          switch (world->fork_action) {
               default:
                    D_BUG( "unknown fork action %d", world->fork_action );

               case FFA_CLOSE:
                    D_DEBUG_AT( Fusion_Main, "  -> closing world %d\n", i );

                    /* Remove world from global list. */
                    fusion_worlds[i] = NULL;

                    /* Unmap shared area. */
                    munmap( world->shared, sizeof(FusionWorldShared) );

                    /* Close socket. */
                    close( world->fusion_fd );

                    /* Free local world data. */
                    D_MAGIC_CLEAR( world );
                    D_FREE( world );

                    break;

               case FFA_FORK: {
                    __Fusionee    *fusionee;
                    __FusioneeRef *fusionee_ref;
                    
                    D_DEBUG_AT( Fusion_Main, "  -> forking in world %d\n", i );

                    fusionee = world->fusionee;
                    
                    D_DEBUG_AT( Fusion_Main, "  -> duplicating fusion id %lu\n", world->fusion_id );
                    
                    fusion_skirmish_prevail( &shared->fusionees_lock );
                   
                    if (_fusion_add_fusionee( world, world->fusion_id )) {
                         fusion_skirmish_dismiss( &shared->fusionees_lock );
                         raise( SIGTRAP );
                    }

                    D_DEBUG_AT( Fusion_Main, "  -> duplicating local refs...\n" );
                    
                    direct_list_foreach (fusionee_ref, fusionee->refs) {
                         __FusioneeRef *new_ref;

                         new_ref = SHCALLOC( shared->main_pool, 1, sizeof(__FusioneeRef) );
                         if (!new_ref) {  
                              D_OOSHM();
                              fusion_skirmish_dismiss( &shared->fusionees_lock );
                              raise( SIGTRAP );
                         }

                         new_ref->ref   = fusionee_ref->ref;
                         new_ref->count = fusionee_ref->count;
                         /* Avoid locking. */ 
                         new_ref->ref->multi.builtin.local += new_ref->count;

                         direct_list_append( &((__Fusionee*)world->fusionee)->refs, &new_ref->link );
                    }

                    fusion_skirmish_dismiss( &shared->fusionees_lock );

                    D_DEBUG_AT( Fusion_Main, "  -> restarting dispatcher loop...\n" );
                    
                    /* Restart the dispatcher thread. FIXME: free old struct */
                    world->dispatch_loop = direct_thread_create( DTT_MESSAGING,
                                                                 fusion_dispatch_loop,
                                                                 world, "Fusion Dispatch" );
                    if (!world->dispatch_loop)
                         raise( SIGTRAP );
               
               }    break;
          }
     }
}

/**********************************************************************************************************************/

/*
 * Enters a fusion world by joining or creating it.
 *
 * If <b>world</b> is negative, the next free index is used to create a new world.
 * Otherwise the world with the specified index is joined or created.
 */
DirectResult
fusion_enter( int               world_index,
              int               abi_version,
              FusionEnterRole   role,
              FusionWorld     **ret_world )
{
     DirectResult        ret     = DR_OK;
     int                 fd      = -1;
     FusionID            id      = -1;
     FusionWorld        *world   = NULL;
     FusionWorldShared  *shared  = MAP_FAILED;
     struct sockaddr_un  addr;
     char                buf[128];
     int                 len, err;

     D_DEBUG_AT( Fusion_Main, "%s( %d, %d, %p )\n", __FUNCTION__, world_index, abi_version, ret_world );

     D_ASSERT( ret_world != NULL );

     if (world_index >= FUSION_MAX_WORLDS) {
          D_ERROR( "Fusion/Init: World index %d exceeds maximum index %d!\n", world_index, FUSION_MAX_WORLDS - 1 );
          return DR_INVARG;
     }
     
     if (fusion_config->force_slave)
          role = FER_SLAVE;

     pthread_once( &fusion_init_once, init_once );

     direct_initialize();
     
     fd = socket( PF_LOCAL, SOCK_RAW, 0 );
     if (fd < 0) {
          D_PERROR( "Fusion/Init: Error creating local socket!\n" );
          return DR_IO;
     }
          
     /* Set close-on-exec flag. */
     if (fcntl( fd, F_SETFD, FD_CLOEXEC ) < 0)
          D_PERROR( "Fusion/Init: Couldn't set close-on-exec flag!\n" );

     pthread_mutex_lock( &fusion_worlds_lock );
     
     addr.sun_family = AF_UNIX;
     
     if (world_index < 0) {
          if (role == FER_SLAVE) {
               D_ERROR( "Fusion/Init: Slave role and a new world (index -1) was requested!\n" );
               pthread_mutex_unlock( &fusion_worlds_lock );
               close( fd );
               return DR_INVARG;
          }
          
          for (world_index=0; world_index<FUSION_MAX_WORLDS; world_index++) {
               if (fusion_worlds[world_index])
                    continue;

               len = snprintf( addr.sun_path, sizeof(addr.sun_path), "/tmp/.fusion-%d/", world_index );
               /* Make socket directory if it doesn't exits. */
               if (mkdir( addr.sun_path, 0775 ) == 0) {
                    chmod( addr.sun_path, 0775 );
                    if (fusion_config->shmfile_gid != (gid_t)-1)
                         chown( addr.sun_path, -1, fusion_config->shmfile_gid );
               }
               
               snprintf( addr.sun_path+len, sizeof(addr.sun_path)-len, "%lx", FUSION_ID_MASTER );
               
               /* Bind to address. */
               err = bind( fd, (struct sockaddr*)&addr, sizeof(addr) );
               if (err == 0) {
                    chmod( addr.sun_path, 0660 );
                    /* Change group, if requested. */
                    if (fusion_config->shmfile_gid != (gid_t)-1)
                         chown( addr.sun_path, -1, fusion_config->shmfile_gid );
                    id = FUSION_ID_MASTER;
                    break;
               }
          }            
     }
     else {
          world = fusion_worlds[world_index];
          if (!world) {
               len = snprintf( addr.sun_path, sizeof(addr.sun_path), "/tmp/.fusion-%d/", world_index );
               /* Make socket directory if it doesn't exits. */
               if (mkdir( addr.sun_path, 0775 ) == 0) {
                    chmod( addr.sun_path, 0775 );
                    if (fusion_config->shmfile_gid != (gid_t)-1)
                         chown( addr.sun_path, -1, fusion_config->shmfile_gid );
               }
               
               /* Check wether we are master. */
               snprintf( addr.sun_path+len, sizeof(addr.sun_path)-len, "%lx", FUSION_ID_MASTER );
               
               err = bind( fd, (struct sockaddr*)&addr, sizeof(addr) );
               if (err < 0) {
                    if (role == FER_MASTER) {
                         D_ERROR( "Fusion/Main: Couldn't start session as master! Remove %s.\n", addr.sun_path );
                         ret = DR_INIT;
                         goto error;
                    }
                    
                    /* Auto generate slave id. */
                    for (id = FUSION_ID_MASTER+1; id < (FusionID)-1; id++) {
                         snprintf( addr.sun_path+len, sizeof(addr.sun_path)-len, "%lx", id );
                         err = bind( fd, (struct sockaddr*)&addr, sizeof(addr) );
                         if (err == 0) {
                              chmod( addr.sun_path, 0660 );
                               /* Change group, if requested. */
                              if (fusion_config->shmfile_gid != (gid_t)-1)
                                   chown( addr.sun_path, -1, fusion_config->shmfile_gid );
                              break;
                         }
                    }
               }
               else if (err == 0 && role != FER_SLAVE) { 
                    chmod( addr.sun_path, 0660 );
                    /* Change group, if requested. */
                    if (fusion_config->shmfile_gid != (gid_t)-1)
                         chown( addr.sun_path, -1, fusion_config->shmfile_gid ); 
                    id = FUSION_ID_MASTER;
               }
          }
     }
     
     /* Enter a world again? */
     if (world) {
          D_MAGIC_ASSERT( world, FusionWorld );
          D_ASSERT( world->refs > 0 );

          /* Check the role again. */
          switch (role) {
               case FER_MASTER:
                    if (world->fusion_id != FUSION_ID_MASTER) {
                         D_ERROR( "Fusion/Init: Master role requested for a world (%d) "
                                  "we're already slave in!\n", world_index );
                         ret = DR_UNSUPPORTED;
                         goto error;
                    }
                    break;

               case FER_SLAVE:
                    if (world->fusion_id == FUSION_ID_MASTER) {
                         D_ERROR( "Fusion/Init: Slave role requested for a world (%d) "
                                  "we're already master in!\n", world_index );
                         ret = DR_UNSUPPORTED;
                         goto error;
                    }
                    break;

               case FER_ANY:
                    break;
          }

          shared = world->shared;

          D_MAGIC_ASSERT( shared, FusionWorldShared );

          if (shared->world_abi != abi_version) {
               D_ERROR( "Fusion/Init: World ABI (%d) of world '%d' doesn't match own (%d)!\n",
                        shared->world_abi, world_index, abi_version );
               ret = DR_VERSIONMISMATCH;
               goto error;
          }

          world->refs++;

          pthread_mutex_unlock( &fusion_worlds_lock );

          D_DEBUG_AT( Fusion_Main, "  -> using existing world %p [%d]\n", world, world_index );

          close( fd );

          /* Return the world. */
          *ret_world = world;

          return DR_OK;
     }
     
     if (id == (FusionID)-1) {
          D_ERROR( "Fusion/Init: Opening fusion socket (world %d) as '%s' failed!\n", world_index,
                    role == FER_ANY ? "any" : (role == FER_MASTER ? "master" : "slave")  );
          ret = DR_INIT;
          goto error;
     }
     
     D_DEBUG_AT( Fusion_Main, "  -> Fusion ID 0x%08lx\n", id );

     if (id == FUSION_ID_MASTER) {
          int shared_fd;
          
          snprintf( buf, sizeof(buf), "%s/fusion.%d.core", 
                    fusion_config->tmpfs ? : "/dev/shm", world_index );
           
          /* Open shared memory file. */         
          shared_fd = open( buf, O_RDWR | O_CREAT | O_TRUNC, 0660 );
          if (shared_fd < 0) {
               D_PERROR( "Fusion/Init: Couldn't open shared memory file!\n" );
               ret = DR_INIT;
               goto error;
          }

          if (fusion_config->shmfile_gid != (gid_t)-1) {
               if (fchown( shared_fd, -1, fusion_config->shmfile_gid ) != 0)
                    D_INFO( "Fusion/Init: Changing owner on %s failed... continuing on.\n", buf );
          }
         
          fchmod( shared_fd, 0660 );
          ftruncate( shared_fd, sizeof(FusionWorldShared) );
          
          /* Map shared area. */
          shared = mmap( (void*) 0x20000000 + 0x2000 * world_index, sizeof(FusionWorldShared),
                         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, shared_fd, 0 );
          if (shared == MAP_FAILED) {
               D_PERROR( "Fusion/Init: Mapping shared area failed!\n" );
               close( shared_fd );
               ret = DR_INIT;
               goto error;
          }
          
          close( shared_fd );
          
          D_DEBUG_AT( Fusion_Main, "  -> shared area at %p, size %zu\n", shared, sizeof(FusionWorldShared) );
          
          /* Initialize reference counter. */
          shared->refs = 1;
          
          /* Set ABI version. */
          shared->world_abi = abi_version;

          /* Set the world index. */
          shared->world_index = world_index;

          /* Set pool allocation base/max. */
          shared->pool_base = (void*)0x20000000 + 0x2000 * FUSION_MAX_WORLDS + 0x8000000 * world_index;
          shared->pool_max  = shared->pool_base + 0x8000000 - 1;

          /* Set start time of world clock. */
          gettimeofday( &shared->start_time, NULL );

          D_MAGIC_SET( shared, FusionWorldShared );
     }
     else {
          FusionEnter enter;
          int         shared_fd;
          
          /* Fill enter information. */
          enter.type      = FMT_ENTER;
          enter.fusion_id = id;
          
          snprintf( addr.sun_path, sizeof(addr.sun_path),
                    "/tmp/.fusion-%d/%lx", world_index, FUSION_ID_MASTER );
          
          /* Send enter message (used to sync with the master) */
          ret = _fusion_send_message( fd, &enter, sizeof(FusionEnter), &addr );
          if (ret == DR_OK) {
               ret = _fusion_recv_message( fd, &enter, sizeof(FusionEnter), NULL );
               if (ret == DR_OK && enter.type != FMT_ENTER) {
                    D_ERROR( "Fusion/Init: Expected message ENTER, got '%d'!\n", enter.type );
                    ret = DR_FUSION;
               }
          }
               
          if (ret) {
               D_ERROR( "Fusion/Init: Could not enter world '%d'!\n", world_index );
               goto error;
          }
          
          snprintf( buf, sizeof(buf), "%s/fusion.%d.core", 
                    fusion_config->tmpfs ? : "/dev/shm", world_index );
          
          /* Open shared memory file. */
          shared_fd = open( buf, O_RDWR );
          if (shared_fd < 0) {
               D_PERROR( "Fusion/Init: Couldn't open shared memory file!\n" );
               ret = DR_INIT;
               goto error;
          }
          
          /* Map shared area. */
          shared = mmap( (void*) 0x20000000 + 0x2000 * world_index, sizeof(FusionWorldShared),
                         PROT_READ | PROT_WRITE, MAP_SHARED | MAP_FIXED, shared_fd, 0 );
          if (shared == MAP_FAILED) {
               D_PERROR( "Fusion/Init: Mapping shared area failed!\n" );
               close( shared_fd );
               ret = DR_INIT;
               goto error;
          }
          
          close( shared_fd );
          
          D_DEBUG_AT( Fusion_Main, "  -> shared area at %p, size %zu\n", shared, sizeof(FusionWorldShared) );
          
          D_MAGIC_ASSERT( shared, FusionWorldShared );

          /* Check ABI version. */
          if (shared->world_abi != abi_version) {
               D_ERROR( "Fusion/Init: World ABI (%d) doesn't match own (%d)!\n",
                        shared->world_abi, abi_version );
               ret = DR_VERSIONMISMATCH;
               goto error;
          }
     }

     /* Synchronize to world clock. */
     direct_clock_set_start( &shared->start_time );
     
     /* Allocate local data. */
     world = D_CALLOC( 1, sizeof(FusionWorld) );
     if (!world) {
          ret = D_OOM();
          goto error;
     }

     /* Initialize local data. */
     world->refs      = 1;
     world->shared    = shared;
     world->fusion_fd = fd;
     world->fusion_id = id;

     D_MAGIC_SET( world, FusionWorld );

     fusion_worlds[world_index] = world;

     /* Initialize shared memory part. */
     ret = fusion_shm_init( world );
     if (ret)
          goto error2;

     D_DEBUG_AT( Fusion_Main, "  -> initializing other parts...\n" );

     /* Initialize other parts. */
     if (world->fusion_id == FUSION_ID_MASTER) {
          fusion_skirmish_init( &shared->arenas_lock, "Fusion Arenas", world );
          fusion_skirmish_init( &shared->reactor_globals, "Fusion Reactor Globals", world );
          fusion_skirmish_init( &shared->fusionees_lock, "Fusionees", world );

          /* Create the main pool. */
          ret = fusion_shm_pool_create( world, "Fusion Main Pool", 0x100000,
                                        fusion_config->debugshm, &shared->main_pool );
          if (ret)
               goto error3;
     }
     
     /* Add ourselves to the list of fusionees. */
     ret = _fusion_add_fusionee( world, id );
     if (ret)
          goto error4;
          
     D_DEBUG_AT( Fusion_Main, "  -> starting dispatcher loop...\n" );

     /* Start the dispatcher thread. */
     world->dispatch_loop = direct_thread_create( DTT_MESSAGING,
                                                  fusion_dispatch_loop,
                                                  world, "Fusion Dispatch" );
     if (!world->dispatch_loop) {
          ret = DR_FAILURE;
          goto error5;
     }

     D_DEBUG_AT( Fusion_Main, "  -> done. (%p)\n", world );

     pthread_mutex_unlock( &fusion_worlds_lock );

     /* Return the fusion world. */
     *ret_world = world;

     return DR_OK;


error5:
     if (world->dispatch_loop)
          direct_thread_destroy( world->dispatch_loop );
     
     _fusion_remove_fusionee( world, id );
     
error4:
     if (world->fusion_id == FUSION_ID_MASTER)
          fusion_shm_pool_destroy( world, shared->main_pool );

error3:
     if (world->fusion_id == FUSION_ID_MASTER) {
          fusion_skirmish_destroy( &shared->arenas_lock );
          fusion_skirmish_destroy( &shared->reactor_globals );
          fusion_skirmish_destroy( &shared->fusionees_lock );
     }

     fusion_shm_deinit( world );


error2:
     fusion_worlds[world_index] = world;

     D_MAGIC_CLEAR( world );

     D_FREE( world );

error:
     if (shared != MAP_FAILED) {
          if (id == FUSION_ID_MASTER)
               D_MAGIC_CLEAR( shared );

          munmap( shared, sizeof(FusionWorldShared) );
     }

     if (fd != -1) {
          /* Unbind. */
          socklen_t len = sizeof(addr);
          if (getsockname( fd, (struct sockaddr*)&addr, &len ) == 0)
               unlink( addr.sun_path );
               
          close( fd );
     }

     pthread_mutex_unlock( &fusion_worlds_lock );

     direct_shutdown();

     return ret;
}

DirectResult
fusion_stop_dispatcher( FusionWorld *world,
                        bool         emergency )
{
     if (!emergency) {
          fusion_sync( world );

          direct_thread_lock( world->dispatch_loop );
     }

     world->dispatch_stop = true;

     if (!emergency) {
          direct_thread_unlock( world->dispatch_loop );

          fusion_sync( world );
     }

     return DR_OK;
}

/*
 * Exits the fusion world.
 *
 * If 'emergency' is true the function won't join but kill the dispatcher thread.
 */
DirectResult
fusion_exit( FusionWorld *world,
             bool         emergency )
{
     FusionWorldShared *shared;
     int                world_index;
     bool               clear = false;

     D_DEBUG_AT( Fusion_Main, "%s( %p, %semergency )\n", __FUNCTION__, world, emergency ? "" : "no " );

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );
     
     world_index = shared->world_index;

     pthread_mutex_lock( &fusion_worlds_lock );

     D_ASSERT( world->refs > 0 );

     if (--world->refs) {
          pthread_mutex_unlock( &fusion_worlds_lock );
          return DR_OK;
     }
 
     if (!emergency) {
          FusionMessageType msg = FMT_SEND;

          /* Wakeup dispatcher. */
          if (_fusion_send_message( world->fusion_fd, &msg, sizeof(msg), NULL ))
               direct_thread_cancel( world->dispatch_loop );

          /* Wait for its termination. */
          direct_thread_join( world->dispatch_loop );
     }

     direct_thread_destroy( world->dispatch_loop );

     /* Remove ourselves from list. */
     if (!emergency || fusion_master( world )) {
          _fusion_remove_fusionee( world, world->fusion_id );
     }
     else {
          struct sockaddr_un addr;
          FusionLeave        leave;

          addr.sun_family = AF_UNIX;
          snprintf( addr.sun_path, sizeof(addr.sun_path), 
                    "/tmp/.fusion-%d/%lx", world_index, FUSION_ID_MASTER );

          leave.type      = FMT_LEAVE;
          leave.fusion_id = world->fusion_id;

          _fusion_send_message( world->fusion_fd, &leave, sizeof(FusionLeave), &addr );
     }

     /* Master has to deinitialize shared data. */
     if (fusion_master( world )) {
          shared->refs--;
          if (shared->refs == 0) {
               fusion_skirmish_destroy( &shared->reactor_globals );
               fusion_skirmish_destroy( &shared->arenas_lock );
               fusion_skirmish_destroy( &shared->fusionees_lock );

               fusion_shm_pool_destroy( world, shared->main_pool );
          
               /* Deinitialize shared memory. */
               fusion_shm_deinit( world );
          
               clear = true;
          }
     }
     else {
          /* Leave shared memory. */
          fusion_shm_deinit( world );
     }

     /* Reset local dispatch nodes. */
     _fusion_reactor_free_all( world );

     /* Remove world from global list. */
     fusion_worlds[shared->world_index] = NULL;

     /* Unmap shared area. */
     if (clear)
          D_MAGIC_CLEAR( shared );
     
     munmap( shared, sizeof(FusionWorldShared) );
     
     /* Close socket. */     
     close( world->fusion_fd );

     if (clear) {
          DIR  *dir;
          char  buf[128];
          int   len;
          
          /* Remove core shmfile. */
          snprintf( buf, sizeof(buf), "%s/fusion.%d.core", 
                    fusion_config->tmpfs ? : "/dev/shm", world_index );
          D_DEBUG_AT( Fusion_Main, "Removing shmfile %s.\n", buf );
          unlink( buf );
          
          /* Cleanup socket directory. */
          len = snprintf( buf, sizeof(buf), "/tmp/.fusion-%d/", world_index );
          dir = opendir( buf );
          if (dir) {
               struct dirent *entry = NULL;
               struct dirent  tmp;
               
               while (readdir_r( dir, &tmp, &entry ) == 0 && entry) {
                    if (entry->d_name[0] != '.') {
                         struct stat st;
                         
                         direct_snputs( buf+len, entry->d_name, sizeof(buf)-len );
                         if (stat( buf, &st ) == 0 && S_ISSOCK(st.st_mode)) {
                              D_DEBUG_AT( Fusion_Main, "Removing socket %s.\n", buf );
                              unlink( buf );
                         }
                    }
               }
               
               closedir( dir );
          }
          else {
               D_PERROR( "Fusion/Main: Couldn't open socket directory %s", buf );
          }
     }
     
     /* Free local world data. */
     D_MAGIC_CLEAR( world );
     D_FREE( world );

     D_DEBUG_AT( Fusion_Main, "%s( %p ) done.\n", __FUNCTION__, world );

     pthread_mutex_unlock( &fusion_worlds_lock );

     direct_shutdown();

     return DR_OK;
}

/*
 * Sends a signal to one or more fusionees and optionally waits
 * for their processes to terminate.
 *
 * A fusion_id of zero means all fusionees but the calling one.
 * A timeout of zero means infinite waiting while a negative value
 * means no waiting at all.
 */
DirectResult
fusion_kill( FusionWorld *world,
             FusionID     fusion_id,
             int          signal,
             int          timeout_ms )
{
     FusionWorldShared *shared;
     __Fusionee        *fusionee, *temp;
     int                result;

     D_DEBUG_AT( Fusion_Main, "%s( %p, %lu, %d, %d )\n", 
                 __FUNCTION__, world, fusion_id, signal, timeout_ms );
     
     D_MAGIC_ASSERT( world, FusionWorld );
     
     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );
     
     fusion_skirmish_prevail( &shared->fusionees_lock );
     
     direct_list_foreach_safe (fusionee, temp, shared->fusionees) {          
          if (fusion_id == 0 && fusionee->id == world->fusion_id)
               continue;
                
          if (fusion_id != 0 && fusionee->id != fusion_id)
               continue;

          D_DEBUG_AT( Fusion_Main, " -> killing fusionee %lu (%d)...\n", fusionee->id, fusionee->pid );
          
          result = kill( fusionee->pid, signal );
          if (result == 0 && timeout_ms >= 0) {
               pid_t     pid  = fusionee->pid;
               long long stop = timeout_ms ? (direct_clock_get_micros() + timeout_ms*1000) : 0;
               
               fusion_skirmish_dismiss( &shared->fusionees_lock );
          
               while (kill( pid, 0 ) == 0) { 
                    usleep( 1000 );
                    
                    if (timeout_ms && direct_clock_get_micros() >= stop)
                         break;
               };
               
               fusion_skirmish_prevail( &shared->fusionees_lock );
          }
          else if (result < 0) {
               if (errno == ESRCH) {
                    D_DEBUG_AT( Fusion_Main, " ... fusionee %lu exited without removing itself!\n", fusionee->id );

                    _fusion_remove_fusionee( world, fusionee->id );
               }
               else {
                    D_PERROR( "Fusion/Main: kill(%d, %d)\n", fusionee->pid, signal );
               }
          } 
     }
     
     fusion_skirmish_dismiss( &shared->fusionees_lock );

     return DR_OK;
}

/**********************************************************************************************************************/

static void *
fusion_dispatch_loop( DirectThread *self, void *arg )
{
     FusionWorld        *world = arg;
     struct sockaddr_un  addr;
     socklen_t           addr_len = sizeof(addr); 
     fd_set              set;
     char                buf[FUSION_MESSAGE_SIZE];

     D_DEBUG_AT( Fusion_Main_Dispatch, "%s() running...\n", __FUNCTION__ );

     while (true) {
          int result;
          
          D_MAGIC_ASSERT( world, FusionWorld );

          FD_ZERO( &set );
          FD_SET( world->fusion_fd, &set );

          result = select( world->fusion_fd + 1, &set, NULL, NULL, NULL );
          if (result < 0) {
               switch (errno) {
                    case EINTR:
                         continue;

                    default:
                         D_PERROR( "Fusion/Dispatcher: select() failed!\n" );
                         return NULL;
               }
          }

          D_MAGIC_ASSERT( world, FusionWorld );

          if (FD_ISSET( world->fusion_fd, &set ) && 
              recvfrom( world->fusion_fd, buf, sizeof(buf), 0, (struct sockaddr*)&addr, &addr_len ) > 0) {
               FusionMessage *msg = (FusionMessage*)buf;               

               pthread_setcancelstate( PTHREAD_CANCEL_DISABLE, NULL );

               D_DEBUG_AT( Fusion_Main_Dispatch, " -> message from '%s'...\n", addr.sun_path );

               direct_thread_lock( world->dispatch_loop );

               if (world->dispatch_stop) {
                    D_DEBUG_AT( Fusion_Main_Dispatch, "  -> IGNORING (dispatch_stop!)\n" );
               }
               else {
                    switch (msg->type) {
                         case FMT_SEND:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_SEND...\n" );
                              break;

                         case FMT_ENTER:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_ENTER...\n" ); 
                              if (!fusion_master( world )) {
                                   D_ERROR( "Fusion/Dispatch: Got ENTER request, but I'm not master!\n" );
                                   break;
                              }
                              if (msg->enter.fusion_id == world->fusion_id) {
                                   D_ERROR( "Fusion/Dispatch: Received ENTER request from myself!\n" );
                                   break;
                              }
                              /* Nothing to do here. Send back message. */
                              _fusion_send_message( world->fusion_fd, msg, sizeof(FusionEnter), &addr );
                              break;

                         case FMT_LEAVE:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_LEAVE...\n" );
                              if (!fusion_master( world )) {
                                   D_ERROR( "Fusion/Dispatch: Got LEAVE request, but I'm not master!\n" );
                                   break;
                              }
                              if (msg->leave.fusion_id == world->fusion_id) {
                                   D_ERROR( "Fusion/Dispatch: Received LEAVE request from myself!\n" );
                                   break;
                              }
                              _fusion_remove_fusionee( world, msg->leave.fusion_id );
                              break;

                         case FMT_CALL:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_CALL...\n" );
                              _fusion_call_process( world, msg->call.call_id, &msg->call );
                              break;

                         case FMT_REACTOR:
                              D_DEBUG_AT( Fusion_Main_Dispatch, "  -> FMT_REACTOR...\n" );
                              _fusion_reactor_process_message( world, msg->reactor.id, msg->reactor.channel, 
                                                               &buf[sizeof(FusionReactorMessage)] );
                              if (msg->reactor.ref) {
                                   fusion_ref_down( msg->reactor.ref, true );
                                   if (fusion_ref_zero_trylock( msg->reactor.ref ) == DR_OK) {
                                        fusion_ref_destroy( msg->reactor.ref );
                                        SHFREE( world->shared->main_pool, msg->reactor.ref );
                                   }
                              }
                              break;                    

                         default:
                              D_BUG( "unexpected message type (%d)", msg->type );
                              break;
                    }
               }

               direct_thread_unlock( world->dispatch_loop );

               if (!world->refs) {
                    D_DEBUG_AT( Fusion_Main_Dispatch, "  -> good bye!\n" );
                    return NULL;
               }

               D_DEBUG_AT( Fusion_Main_Dispatch, " ...done\n" );

               pthread_setcancelstate( PTHREAD_CANCEL_ENABLE, NULL );
          }
     }

     return NULL;
}

/**********************************************************************************************************************/

#endif /* FUSION_BUILD_KERNEL */

/*
 * Wait until all pending messages are processed.
 */
DirectResult
fusion_sync( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     D_DEBUG_AT( Fusion_Main, "%s( %p )\n", __FUNCTION__, world );

     D_DEBUG_AT( Fusion_Main, "  -> syncing with fusion device...\n" );

     while (ioctl( world->fusion_fd, FUSION_SYNC )) {
          switch (errno) {
               case EINTR:
                    continue;
               default:
                    break;
          }

          D_PERROR( "Fusion/Main: FUSION_SYNC failed!\n" );

          return DR_FAILURE;
     }

     D_DEBUG_AT( Fusion_Main, "  -> synced\n");

     return DR_OK;
}

/*
 * Sets the fork() action of the calling Fusionee within the world.
 */
void
fusion_world_set_fork_action( FusionWorld      *world,
                              FusionForkAction  action )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     world->fork_action = action;
}

/*
 * Gets the current fork() action.
 */
FusionForkAction 
fusion_world_get_fork_action( FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );
     
     return world->fork_action;
}

/*
 * Registers a callback called upon fork().
 */
void
fusion_world_set_fork_callback( FusionWorld        *world,
                                FusionForkCallback  callback )
{
     D_MAGIC_ASSERT( world, FusionWorld );
     
     world->fork_callback = callback;
}

/*
 * Registers a callback called when a slave exits.
 */
void
fusion_world_set_leave_callback( FusionWorld         *world,
                                 FusionLeaveCallback  callback,
                                 void                *ctx )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     world->leave_callback = callback;
     world->leave_ctx      = ctx;
}

/*
 * Return the index of the specified world.
 */
int
fusion_world_index( const FusionWorld *world )
{
     FusionWorldShared *shared;

     D_MAGIC_ASSERT( world, FusionWorld );

     shared = world->shared;

     D_MAGIC_ASSERT( shared, FusionWorldShared );

     return shared->world_index;
}

/*
 * Return the own Fusion ID within the specified world.
 */
FusionID
fusion_id( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return world->fusion_id;
}

/*
 * Return if the world is a multi application world.
 */
bool
fusion_is_multi( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return true;
}

/*
 * Return the thread ID of the Fusion Dispatcher within the specified world.
 */
pid_t
fusion_dispatcher_tid( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return direct_thread_get_tid( world->dispatch_loop );
}

/*
 * Return true if this process is the master.
 */
bool
fusion_master( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return world->fusion_id == FUSION_ID_MASTER;
}

/*
 * Check if a pointer points to the shared memory.
 */
bool
fusion_is_shared( FusionWorld *world,
                  const void  *ptr )
{
     int              i;
     DirectResult     ret;
     FusionSHM       *shm;
     FusionSHMShared *shared;

     D_MAGIC_ASSERT( world, FusionWorld );

     shm = &world->shm;

     D_MAGIC_ASSERT( shm, FusionSHM );

     shared = shm->shared;

     D_MAGIC_ASSERT( shared, FusionSHMShared );

     if (ptr >= (void*) world->shared && ptr < (void*) world->shared + sizeof(FusionWorldShared))
          return true;

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret)
          return false;

     for (i=0; i<FUSION_SHM_MAX_POOLS; i++) {
          if (shared->pools[i].active) {
               shmalloc_heap       *heap;
               FusionSHMPoolShared *pool = &shared->pools[i];

               D_MAGIC_ASSERT( pool, FusionSHMPoolShared );

               heap = pool->heap;

               D_MAGIC_ASSERT( heap, shmalloc_heap );

               if (ptr >= pool->addr_base && ptr < pool->addr_base + heap->size) {
                    fusion_skirmish_dismiss( &shared->lock );
                    return true;
               }
          }
     }

     fusion_skirmish_dismiss( &shared->lock );

     return false;
}

#else /* FUSION_BUILD_MULTI */

/*
 * Enters a fusion world by joining or creating it.
 *
 * If <b>world_index</b> is negative, the next free index is used to create a new world.
 * Otherwise the world with the specified index is joined or created.
 */
DirectResult fusion_enter( int               world_index,
                           int               abi_version,
                           FusionEnterRole   role,
                           FusionWorld     **ret_world )
{
     DirectResult  ret;
     FusionWorld  *world = NULL;

     D_ASSERT( ret_world != NULL );

     ret = direct_initialize();
     if (ret)
          return ret;

     world = D_CALLOC( 1, sizeof(FusionWorld) );
     if (!world) {
          ret = D_OOM();
          goto error;
     }

     world->shared = D_CALLOC( 1, sizeof(FusionWorldShared) );
     if (!world->shared) {
          ret = D_OOM();
          goto error;
     }

     /* Create the main pool. */
     ret = fusion_shm_pool_create( world, "Fusion Main Pool", 0x100000,
                                   fusion_config->debugshm, &world->shared->main_pool );
     if (ret)
          goto error;

     D_MAGIC_SET( world, FusionWorld );
     D_MAGIC_SET( world->shared, FusionWorldShared );

     *ret_world = world;

     return DR_OK;


error:
     if (world) {
          if (world->shared)
               D_FREE( world->shared );

          D_FREE( world );
     }

     direct_shutdown();

     return ret;
}

DirectResult
fusion_stop_dispatcher( FusionWorld *world,
                        bool         emergency )
{
     return DR_OK;
}

/*
 * Exits the fusion world.
 *
 * If 'emergency' is true the function won't join but kill the dispatcher thread.
 */
DirectResult
fusion_exit( FusionWorld *world,
             bool         emergency )
{
     D_MAGIC_ASSERT( world, FusionWorld );
     D_MAGIC_ASSERT( world->shared, FusionWorldShared );

     fusion_shm_pool_destroy( world, world->shared->main_pool );

     D_MAGIC_CLEAR( world->shared );

     D_FREE( world->shared );

     D_MAGIC_CLEAR( world );

     D_FREE( world );

     direct_shutdown();

     return DR_OK;
}

/*
 * Sets the fork() action of the calling Fusionee within the world.
 */
void
fusion_world_set_fork_action( FusionWorld      *world,
                              FusionForkAction  action )
{
     D_MAGIC_ASSERT( world, FusionWorld );
}

/*
 * Gets the current fork() action.
 */
FusionForkAction 
fusion_world_get_fork_action( FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );
     
     return world->fork_action;
}

/*
 * Registers a callback called upon fork().
 */
void
fusion_world_set_fork_callback( FusionWorld        *world,
                                FusionForkCallback  callback )
{
     D_MAGIC_ASSERT( world, FusionWorld );
}

/*
 * Registers a callback called when a slave exits.
 */
void
fusion_world_set_leave_callback( FusionWorld         *world,
                                 FusionLeaveCallback  callback,
                                 void                *ctx )
{
     D_MAGIC_ASSERT( world, FusionWorld );
}

/*
 * Return the index of the specified world.
 */
int
fusion_world_index( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return 0;
}


/*
 * Return true if this process is the master.
 */
bool
fusion_master( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return true;
}

/*
 * Sends a signal to one or more fusionees and optionally waits
 * for their processes to terminate.
 *
 * A fusion_id of zero means all fusionees but the calling one.
 * A timeout of zero means infinite waiting while a negative value
 * means no waiting at all.
 */
DirectResult
fusion_kill( FusionWorld *world,
             FusionID     fusion_id,
             int          signal,
             int          timeout_ms )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return DR_OK;
}

/*
 * Return the own Fusion ID within the specified world.
 */
FusionID
fusion_id( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return 1;
}

/*
 * Return if the world is a multi application world.
 */
bool
fusion_is_multi( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return false;
}

/*
 * Return the thread ID of the Fusion Dispatcher within the specified world.
 */
pid_t
fusion_dispatcher_tid( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     // must return 0 here to not let the dispatcher thread sleep in CoreDFB_CallMode()
     return 0;
}

/*
 * Wait until all pending messages are processed.
 */
DirectResult
fusion_sync( const FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return DR_OK;
}

/* Check if a pointer points to the shared memory. */
bool
fusion_is_shared( FusionWorld *world,
                  const void  *ptr )
{
     D_MAGIC_ASSERT( world, FusionWorld );

     return true;
}

const char *
fusion_get_tmpfs( FusionWorld *world )
{
     D_MAGIC_ASSERT( world, FusionWorld );
     D_MAGIC_ASSERT( world->shared, FusionWorldShared );

     return "/tmp";
}

DirectResult
fusion_get_fusionee_path( const FusionWorld *world,
                          FusionID           fusion_id,
                          char              *buf,
                          size_t             buf_size,
                          size_t            *ret_size )
{
     D_UNIMPLEMENTED();

     return DR_UNIMPLEMENTED;
}

#endif

