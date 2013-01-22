/*
   (c) Copyright 2001-2010  The world wide DirectFB Open Source Community (directfb.org)
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>
#include <errno.h>

#include <sys/time.h>
#include <sys/socket.h>

#include <pthread.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/interface.h>
#include <direct/list.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>

#include <fusion/reactor.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/input.h>
#include <core/windows.h>
#include <core/windows_internal.h>

#include <misc/util.h>

#include <idirectfb.h>

#include "idirectfbinputbuffer.h"


D_DEBUG_DOMAIN( IDFBEvBuf, "IDFBEventBuffer", "IDirectFBEventBuffer Interface" );


typedef struct {
     DirectLink   link;
     DFBEvent     evt;
} EventBufferItem;

typedef struct {
     DirectLink       link;

     CoreInputDevice *device;       /* pointer to input core device struct */
     Reaction         reaction;
     
     DFBInputDeviceDescription desc;
} AttachedDevice;

typedef struct {
     DirectLink   link;

     CoreWindow  *window;       /* pointer to core window struct */
     Reaction     reaction;
} AttachedWindow;

/*
 * private data struct of IDirectFBInputDevice
 */
typedef struct {
     int                           ref;            /* reference counter */

     EventBufferFilterCallback     filter;
     void                         *filter_ctx;

     DirectLink                   *devices;        /* attached devices */

     DirectLink                   *windows;        /* attached windows */

     DirectLink                   *events;         /* linked list containing events */

     pthread_mutex_t               events_mutex;   /* mutex lock for accessing the event queue */

     pthread_cond_t                wait_condition; /* condition for idle wait in WaitForEvent() */

     bool                          pipe;           /* file descriptor mode? */
     int                           pipe_fds[2];    /* read & write file descriptor */

     DirectThread                 *pipe_thread;    /* thread feeding the pipe */

     DFBEventBufferStats           stats;
     bool                          stats_enabled;
} IDirectFBEventBuffer_data;

/*
 * adds an event to the event queue
 */
static void IDirectFBEventBuffer_AddItem( IDirectFBEventBuffer_data *data,
                                          EventBufferItem           *item );

#if !DIRECTFB_BUILD_PURE_VOODOO
static ReactionResult IDirectFBEventBuffer_InputReact( const void *msg_data,
                                                       void       *ctx );

static ReactionResult IDirectFBEventBuffer_WindowReact( const void *msg_data,
                                                        void       *ctx );
#endif

static void *IDirectFBEventBuffer_Feed( DirectThread *thread, void *arg );

static void CollectEventStatistics( DFBEventBufferStats *stats,
                                    const DFBEvent      *event,
                                    int                  incdec );


static void
IDirectFBEventBuffer_Destruct( IDirectFBEventBuffer *thiz )
{
     IDirectFBEventBuffer_data *data = thiz->priv;
#if !DIRECTFB_BUILD_PURE_VOODOO
     AttachedDevice            *device;
     AttachedWindow            *window;
#endif
     EventBufferItem           *item;
     DirectLink                *n;

     D_DEBUG_AT( IDFBEvBuf, "%s( %p )\n", __FUNCTION__, thiz );

#if !DIRECTFB_BUILD_PURE_VOODOO
     /* Remove the event buffer from the containers linked list. */
     containers_remove_input_eventbuffer( thiz );
#endif

     pthread_mutex_lock( &data->events_mutex );

     if (data->pipe) {
          data->pipe = false;

          pthread_cond_broadcast( &data->wait_condition );

          pthread_mutex_unlock( &data->events_mutex );

          direct_thread_join( data->pipe_thread );
          direct_thread_destroy( data->pipe_thread );

          pthread_mutex_lock( &data->events_mutex );

          close( data->pipe_fds[0] );
          close( data->pipe_fds[1] );
     }

#if !DIRECTFB_BUILD_PURE_VOODOO
     direct_list_foreach_safe (device, n, data->devices) {
          dfb_input_detach( device->device, &device->reaction );

          D_FREE( device );
     }

     direct_list_foreach_safe (window, n, data->windows) {
          if (window->window) {
               dfb_window_detach( window->window, &window->reaction );
               dfb_window_unref( window->window );
          }
               
          D_FREE( window );
     }
#endif

     direct_list_foreach_safe (item, n, data->events)
          D_FREE( item );

     pthread_cond_destroy( &data->wait_condition );
     pthread_mutex_destroy( &data->events_mutex );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

static DirectResult
IDirectFBEventBuffer_AddRef( IDirectFBEventBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p )\n", __FUNCTION__, thiz );

     data->ref++;

     return DR_OK;
}

static DirectResult
IDirectFBEventBuffer_Release( IDirectFBEventBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p )\n", __FUNCTION__, thiz );

     if (--data->ref == 0)
          IDirectFBEventBuffer_Destruct( thiz );

     return DR_OK;
}

static DFBResult
IDirectFBEventBuffer_Reset( IDirectFBEventBuffer *thiz )
{
     EventBufferItem *item;
     DirectLink      *n;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p )\n", __FUNCTION__, thiz );

     if (data->pipe)
          return DFB_UNSUPPORTED;

     pthread_mutex_lock( &data->events_mutex );

     direct_list_foreach_safe (item, n, data->events)
          D_FREE( item );

     data->events = NULL;

     pthread_mutex_unlock( &data->events_mutex );

     return DFB_OK;
}

static DFBResult
IDirectFBEventBuffer_WaitForEvent( IDirectFBEventBuffer *thiz )
{
     DFBResult ret = DFB_OK;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p )\n", __FUNCTION__, thiz );

     if (data->pipe)
          return DFB_UNSUPPORTED;

     pthread_mutex_lock( &data->events_mutex );

     if (!data->events)
          pthread_cond_wait( &data->wait_condition, &data->events_mutex );
     if (!data->events)
          ret = DFB_INTERRUPTED;

     pthread_mutex_unlock( &data->events_mutex );

     return ret;
}

static DFBResult
IDirectFBEventBuffer_WaitForEventWithTimeout( IDirectFBEventBuffer *thiz,
                                              unsigned int          seconds,
                                              unsigned int          milli_seconds )
{
     struct timeval  now;
     struct timespec timeout;
     DFBResult       ret    = DFB_OK;
     int             locked = 0;
     long int        nano_seconds = milli_seconds * 1000000;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %u, %u )\n", __FUNCTION__, thiz, seconds, milli_seconds );

     if (data->pipe)
          return DFB_UNSUPPORTED;

     if (pthread_mutex_trylock( &data->events_mutex ) == 0) {
          if (data->events) {
               pthread_mutex_unlock ( &data->events_mutex );
               return ret;
          }
          locked = 1;
     }

     gettimeofday( &now, NULL );

     timeout.tv_sec  = now.tv_sec + seconds;
     timeout.tv_nsec = (now.tv_usec * 1000) + nano_seconds;

     timeout.tv_sec  += timeout.tv_nsec / 1000000000;
     timeout.tv_nsec %= 1000000000;

     if (!locked)
          pthread_mutex_lock( &data->events_mutex );

     if (!data->events) {
          if (pthread_cond_timedwait( &data->wait_condition,
                                      &data->events_mutex,
                                      &timeout ) == ETIMEDOUT)
               ret = DFB_TIMEOUT;
          else if (!data->events)
               ret = DFB_INTERRUPTED;
     }

     pthread_mutex_unlock( &data->events_mutex );

     return ret;
}

static DFBResult
IDirectFBEventBuffer_WakeUp( IDirectFBEventBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p )\n", __FUNCTION__, thiz );

     if (data->pipe)
          return DFB_UNSUPPORTED;

     pthread_cond_broadcast( &data->wait_condition );

     return DFB_OK;
}

static DFBResult
IDirectFBEventBuffer_GetEvent( IDirectFBEventBuffer *thiz,
                               DFBEvent             *event )
{
     EventBufferItem *item;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p )\n", __FUNCTION__, thiz, event );

     if (data->pipe)
          return DFB_UNSUPPORTED;

     pthread_mutex_lock( &data->events_mutex );

     if (!data->events) {
          pthread_mutex_unlock( &data->events_mutex );
          return DFB_BUFFEREMPTY;
     }

     item = (EventBufferItem*) data->events;

     switch (item->evt.clazz) {
          case DFEC_INPUT:
               event->input = item->evt.input;
               break;

          case DFEC_WINDOW:
               event->window = item->evt.window;
               break;

          case DFEC_USER:
               event->user = item->evt.user;
               break;

          case DFEC_VIDEOPROVIDER:
               event->videoprovider = item->evt.videoprovider;
               break;

          case DFEC_UNIVERSAL:
               direct_memcpy( event, &item->evt, item->evt.universal.size );
               break;

          default:
               D_BUG("unknown event class");
     }

     if (data->stats_enabled)
          CollectEventStatistics( &data->stats, &item->evt, -1 );

     direct_list_remove( &data->events, &item->link );

     D_FREE( item );

     pthread_mutex_unlock( &data->events_mutex );

     D_DEBUG_AT( IDFBEvBuf, "  -> class %d, type/size %d, data/id %p\n", event->clazz, event->user.type, event->user.data );

     return DFB_OK;
}

static DFBResult
IDirectFBEventBuffer_PeekEvent( IDirectFBEventBuffer *thiz,
                                DFBEvent             *event )
{
     EventBufferItem *item;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p )\n", __FUNCTION__, thiz, event );

     if (data->pipe)
          return DFB_UNSUPPORTED;

     pthread_mutex_lock( &data->events_mutex );

     if (!data->events) {
          pthread_mutex_unlock( &data->events_mutex );
          return DFB_BUFFEREMPTY;
     }

     item = (EventBufferItem*) data->events;

     switch (item->evt.clazz) {
          case DFEC_INPUT:
               event->input = item->evt.input;
               break;

          case DFEC_WINDOW:
               event->window = item->evt.window;
               break;

          case DFEC_USER:
               event->user = item->evt.user;
               break;

          case DFEC_VIDEOPROVIDER:
               event->videoprovider = item->evt.videoprovider;
               break;

          case DFEC_UNIVERSAL:
               direct_memcpy( event, &item->evt, item->evt.universal.size );
               break;

          default:
               D_BUG("unknown event class");
     }

     pthread_mutex_unlock( &data->events_mutex );

     D_DEBUG_AT( IDFBEvBuf, "  -> class %d, type/size %d, data/id %p\n", event->clazz, event->user.type, event->user.data );

     return DFB_OK;
}

static DFBResult
IDirectFBEventBuffer_HasEvent( IDirectFBEventBuffer *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p ) <- events: %p, pipe: %d\n", __FUNCTION__, thiz, data->events, data->pipe );

     if (data->pipe)
          return DFB_UNSUPPORTED;

     return (data->events ? DFB_OK : DFB_BUFFEREMPTY);
}

static DFBResult
IDirectFBEventBuffer_PostEvent( IDirectFBEventBuffer *thiz,
                                const DFBEvent       *event )
{
     EventBufferItem *item;
     int              size;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p [class %d, type/size %d, data/id %p] )\n", __FUNCTION__,
                 thiz, event, event->clazz, event->user.type, event->user.data );

     switch (event->clazz) {
          case DFEC_INPUT:
          case DFEC_WINDOW:
          case DFEC_USER:
          case DFEC_VIDEOPROVIDER:
               size = sizeof(EventBufferItem);
               break;

          case DFEC_UNIVERSAL:
               size = event->universal.size;
               if (size < sizeof(DFBUniversalEvent))
                    return DFB_INVARG;
               /* We must not exceed the union to avoid crashes in generic code (reading DFBEvents)
                * and to support pipe mode where each written block has to have a fixed size. */
               if (size > sizeof(DFBEvent))
                    return DFB_INVARG;
               size += sizeof(DirectLink);
               break;

          default:
               return DFB_INVARG;
     }

     item = D_CALLOC( 1, size );
     if (!item)
          return D_OOM();

     switch (event->clazz) {
          case DFEC_INPUT:
               item->evt.input = event->input;
               break;

          case DFEC_WINDOW:
               item->evt.window = event->window;
               break;

          case DFEC_USER:
               item->evt.user = event->user;
               break;

          case DFEC_VIDEOPROVIDER:
               item->evt.videoprovider = event->videoprovider;
               break;

          case DFEC_UNIVERSAL:
               direct_memcpy( &item->evt, event, event->universal.size );
               break;

          default:
               D_BUG("unexpected event class");
     }

     IDirectFBEventBuffer_AddItem( data, item );

     return DFB_OK;
}

static DFBResult
IDirectFBEventBuffer_CreateFileDescriptor( IDirectFBEventBuffer *thiz,
                                           int                  *ret_fd )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p )\n", __FUNCTION__, thiz );

     /* Check arguments. */
     if (!ret_fd)
          return DFB_INVARG;

     /* Lock the event queue. */
     pthread_mutex_lock( &data->events_mutex );

     /* Already in pipe mode? */
     if (data->pipe) {
          pthread_mutex_unlock( &data->events_mutex );
          return DFB_BUSY;
     }

     /* Create the file descriptor(s). */
     if (socketpair( PF_LOCAL, SOCK_STREAM, 0, data->pipe_fds )) {
          D_PERROR( "%s(): socketpair( PF_LOCAL, SOCK_STREAM, 0, fds ) failed!\n", __FUNCTION__ );
          pthread_mutex_unlock( &data->events_mutex );
          return errno2result( errno );
     }

     D_DEBUG_AT( IDFBEvBuf, "  -> entering pipe mode\n" );

     /* Enter pipe mode. */
     data->pipe = true;

     /* Signal any waiting processes. */
     pthread_cond_broadcast( &data->wait_condition );

     /* Create the feeding thread. */
     data->pipe_thread = direct_thread_create( DTT_MESSAGING,
                                               IDirectFBEventBuffer_Feed, data,
                                               "EventBufferFeed" );

     /* Unlock the event queue. */
     pthread_mutex_unlock( &data->events_mutex );

     /* Return the file descriptor for reading. */
     *ret_fd = data->pipe_fds[0];

     D_DEBUG_AT( IDFBEvBuf, "  -> fd %d/%d\n", data->pipe_fds[0], data->pipe_fds[1] );

     return DFB_OK;
}

static DFBResult
IDirectFBEventBuffer_EnableStatistics( IDirectFBEventBuffer *thiz,
                                       DFBBoolean            enable )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %sable )\n", __FUNCTION__, thiz, enable ? "en" : "dis" );

     /* Lock the event queue. */
     pthread_mutex_lock( &data->events_mutex );

     /* Already enabled/disabled? */
     if (data->stats_enabled == !!enable) {
          pthread_mutex_unlock( &data->events_mutex );
          return DFB_OK;
     }

     if (enable) {
          EventBufferItem *item;

          /* Collect statistics for events already in the queue. */
          direct_list_foreach (item, data->events)
               CollectEventStatistics( &data->stats, &item->evt, 1 );
     }
     else {
          /* Clear statistics. */
          memset( &data->stats, 0, sizeof(DFBEventBufferStats) );
     }

     /* Remember state. */
     data->stats_enabled = !!enable;

     /* Unlock the event queue. */
     pthread_mutex_unlock( &data->events_mutex );

     return DFB_OK;
}

static DFBResult
IDirectFBEventBuffer_GetStatistics( IDirectFBEventBuffer *thiz,
                                    DFBEventBufferStats  *ret_stats )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p )\n", __FUNCTION__, thiz, ret_stats );

     if (!ret_stats)
          return DFB_INVARG;

     /* Lock the event queue. */
     pthread_mutex_lock( &data->events_mutex );

     /* Not enabled? */
     if (!data->stats_enabled) {
          pthread_mutex_unlock( &data->events_mutex );
          return DFB_UNSUPPORTED;
     }

     /* Return current stats. */
     *ret_stats = data->stats;

     /* Unlock the event queue. */
     pthread_mutex_unlock( &data->events_mutex );

     return DFB_OK;
}

DFBResult
IDirectFBEventBuffer_Construct( IDirectFBEventBuffer      *thiz,
                                EventBufferFilterCallback  filter,
                                void                      *filter_ctx )
{
     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBEventBuffer)

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, filter %p, ctx %p )\n", __FUNCTION__, thiz, filter, filter_ctx );

     data->ref        = 1;
     data->filter     = filter;
     data->filter_ctx = filter_ctx;

     direct_util_recursive_pthread_mutex_init( &data->events_mutex );
     pthread_cond_init( &data->wait_condition, NULL );

     thiz->AddRef                  = IDirectFBEventBuffer_AddRef;
     thiz->Release                 = IDirectFBEventBuffer_Release;
     thiz->Reset                   = IDirectFBEventBuffer_Reset;
     thiz->WaitForEvent            = IDirectFBEventBuffer_WaitForEvent;
     thiz->WaitForEventWithTimeout = IDirectFBEventBuffer_WaitForEventWithTimeout;
     thiz->GetEvent                = IDirectFBEventBuffer_GetEvent;
     thiz->PeekEvent               = IDirectFBEventBuffer_PeekEvent;
     thiz->HasEvent                = IDirectFBEventBuffer_HasEvent;
     thiz->PostEvent               = IDirectFBEventBuffer_PostEvent;
     thiz->WakeUp                  = IDirectFBEventBuffer_WakeUp;
     thiz->CreateFileDescriptor    = IDirectFBEventBuffer_CreateFileDescriptor;
     thiz->EnableStatistics        = IDirectFBEventBuffer_EnableStatistics;
     thiz->GetStatistics           = IDirectFBEventBuffer_GetStatistics;

     D_DEBUG_AT( IDFBEvBuf, "  -> %p [%p]\n", thiz, thiz->priv );

     return DFB_OK;
}

/* directfb internals */

#if !DIRECTFB_BUILD_PURE_VOODOO
DFBResult IDirectFBEventBuffer_AttachInputDevice( IDirectFBEventBuffer *thiz,
                                                  CoreInputDevice      *device )
{
     AttachedDevice            *attached;
     DFBInputDeviceDescription  desc;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_ASSERT( device != NULL );

     dfb_input_device_description( device, &desc );

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p [%02u - %s] )\n", __FUNCTION__, thiz, device,
                 dfb_input_device_id(device), desc.name );

     attached = D_CALLOC( 1, sizeof(AttachedDevice) );
     attached->device = device;
     attached->desc   = desc;

     direct_list_prepend( &data->devices, &attached->link );

     dfb_input_attach( device, IDirectFBEventBuffer_InputReact,
                       data, &attached->reaction );

     return DFB_OK;
}

DFBResult IDirectFBEventBuffer_DetachInputDevice( IDirectFBEventBuffer *thiz,
                                                  CoreInputDevice      *device )
{
     AttachedDevice *attached;
     DirectLink     *link;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_ASSERT( device != NULL );

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p [%02u] )\n", __FUNCTION__, thiz, device,
                 dfb_input_device_id(device) );
     
     direct_list_foreach_safe (attached, link, data->devices) {
          if (attached->device == device) {
               direct_list_remove( &data->devices, &attached->link );
               
               dfb_input_detach( attached->device, &attached->reaction );
               
               D_FREE( attached );
               
               return DFB_OK;
          }
     }

     return DFB_ITEMNOTFOUND;
}

DFBResult IDirectFBEventBuffer_AttachWindow( IDirectFBEventBuffer *thiz,
                                             CoreWindow           *window )
{
     AttachedWindow *attached;
     
     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_ASSERT( window != NULL );

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p [%02u - %d,%d-%dx%d] )\n", __FUNCTION__, thiz,
                 window, window->id, window->config.bounds.x, window->config.bounds.y,
                 window->config.bounds.w, window->config.bounds.h );

     attached = D_CALLOC( 1, sizeof(AttachedWindow) );
     attached->window = window;

     dfb_window_ref( window );

     direct_list_prepend( &data->windows, &attached->link );

     dfb_window_attach( window, IDirectFBEventBuffer_WindowReact,
                        data, &attached->reaction );

     return DFB_OK;
}

DFBResult IDirectFBEventBuffer_DetachWindow( IDirectFBEventBuffer *thiz,
                                             CoreWindow           *window )
{
     AttachedWindow *attached;
     DirectLink     *link;

     DIRECT_INTERFACE_GET_DATA(IDirectFBEventBuffer)

     D_ASSERT( window != NULL );
     
     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p [%02u - %d,%d-%dx%d] )\n", __FUNCTION__, thiz,
                 window, window->id, window->config.bounds.x, window->config.bounds.y,
                 window->config.bounds.w, window->config.bounds.h );

     direct_list_foreach_safe (attached, link, data->windows) {
          if (!attached->window || attached->window == window) {
               direct_list_remove( &data->windows, &attached->link );

               if (attached->window) {
                    dfb_window_detach( attached->window, &attached->reaction );
                    dfb_window_unref( attached->window );
               }
               
               D_FREE( attached );
          }
     }

     return DFB_OK;
}
#endif

/* file internals */

static void IDirectFBEventBuffer_AddItem( IDirectFBEventBuffer_data *data,
                                          EventBufferItem           *item )
{
     if (data->filter && data->filter( &item->evt, data->filter_ctx )) {
          D_FREE( item );
          return;
     }

     pthread_mutex_lock( &data->events_mutex );

     if (data->stats_enabled)
          CollectEventStatistics( &data->stats, &item->evt, 1 );

     direct_list_append( &data->events, &item->link );

     pthread_cond_broadcast( &data->wait_condition );

     pthread_mutex_unlock( &data->events_mutex );
}

#if !DIRECTFB_BUILD_PURE_VOODOO
static ReactionResult IDirectFBEventBuffer_InputReact( const void *msg_data,
                                                       void       *ctx )
{
     const DFBInputEvent       *evt  = msg_data;
     IDirectFBEventBuffer_data *data = ctx;
     EventBufferItem           *item;

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p ) <- type %06x\n", __FUNCTION__, evt, data, evt->type );

     item = D_CALLOC( 1, sizeof(EventBufferItem) );

     item->evt.input = *evt;
     item->evt.clazz = DFEC_INPUT;

     IDirectFBEventBuffer_AddItem( data, item );

     return RS_OK;
}

static ReactionResult IDirectFBEventBuffer_WindowReact( const void *msg_data,
                                                        void       *ctx )
{
     const DFBWindowEvent      *evt  = msg_data;
     IDirectFBEventBuffer_data *data = ctx;
     EventBufferItem           *item;

     D_DEBUG_AT( IDFBEvBuf, "%s( %p, %p ) <- type %06x\n", __FUNCTION__, evt, data, evt->type );

     item = D_CALLOC( 1, sizeof(EventBufferItem) );

     item->evt.window = *evt;
     item->evt.clazz  = DFEC_WINDOW;

     IDirectFBEventBuffer_AddItem( data, item );

     if (evt->type == DWET_DESTROYED) {
          AttachedWindow *window;

          direct_list_foreach (window, data->windows) {
               if (!window->window)
                    continue;

               if (dfb_window_id( window->window ) == evt->window_id) {
                    /* FIXME: free memory later, because reactor writes to it
                       after we return RS_REMOVE */
                    dfb_window_unref( window->window );
                    window->window = NULL;
               }
          }

          return RS_REMOVE;
     }

     return RS_OK;
}
#endif

static void *
IDirectFBEventBuffer_Feed( DirectThread *thread, void *arg )
{
     IDirectFBEventBuffer_data *data = arg;

     pthread_mutex_lock( &data->events_mutex );

     while (data->pipe) {
          while (data->events && data->pipe) {
               int              ret;
               EventBufferItem *item = (EventBufferItem*) data->events;

               if (data->stats_enabled)
                    CollectEventStatistics( &data->stats, &item->evt, -1 );

               direct_list_remove( &data->events, &item->link );

               if (item->evt.clazz == DFEC_UNIVERSAL) {
                    D_WARN( "universal events not supported in pipe mode" );
                    continue;
               }

               pthread_mutex_unlock( &data->events_mutex );

               D_DEBUG_AT( IDFBEvBuf, "Going to write %zu bytes to file descriptor %d...\n",
                           sizeof(DFBEvent), data->pipe_fds[1] );

               ret = write( data->pipe_fds[1], &item->evt, sizeof(DFBEvent) );

               D_DEBUG_AT( IDFBEvBuf, "...wrote %d bytes to file descriptor %d.\n",
                           ret, data->pipe_fds[1] );

               D_FREE( item );

               pthread_mutex_lock( &data->events_mutex );
          }

          if (data->pipe)
               pthread_cond_wait( &data->wait_condition, &data->events_mutex );
     }

     pthread_mutex_unlock( &data->events_mutex );

     return NULL;
}

static void
CollectEventStatistics( DFBEventBufferStats *stats,
                        const DFBEvent      *event,
                        int                  incdec )
{
     stats->num_events += incdec;

     switch (event->clazz) {
          case DFEC_INPUT:
               stats->DFEC_INPUT += incdec;

               switch (event->input.type) {
                    case DIET_KEYPRESS:
                         stats->DIET_KEYPRESS += incdec;
                         break;

                    case DIET_KEYRELEASE:
                         stats->DIET_KEYRELEASE += incdec;
                         break;

                    case DIET_BUTTONPRESS:
                         stats->DIET_BUTTONPRESS += incdec;
                         break;

                    case DIET_BUTTONRELEASE:
                         stats->DIET_BUTTONRELEASE += incdec;
                         break;

                    case DIET_AXISMOTION:
                         stats->DIET_AXISMOTION += incdec;
                         break;

                    default:
                         D_BUG( "unknown input event type 0x%08x\n", event->input.type );
               }
               break;

          case DFEC_WINDOW:
               stats->DFEC_WINDOW += incdec;

               switch (event->window.type) {
                    case DWET_POSITION:
                         stats->DWET_POSITION += incdec;
                         break;

                    case DWET_SIZE:
                         stats->DWET_SIZE += incdec;
                         break;

                    case DWET_CLOSE:
                         stats->DWET_CLOSE += incdec;
                         break;

                    case DWET_DESTROYED:
                         stats->DWET_DESTROYED += incdec;
                         break;

                    case DWET_GOTFOCUS:
                         stats->DWET_GOTFOCUS += incdec;
                         break;

                    case DWET_LOSTFOCUS:
                         stats->DWET_LOSTFOCUS += incdec;
                         break;

                    case DWET_KEYDOWN:
                         stats->DWET_KEYDOWN += incdec;
                         break;

                    case DWET_KEYUP:
                         stats->DWET_KEYUP += incdec;
                         break;

                    case DWET_BUTTONDOWN:
                         stats->DWET_BUTTONDOWN += incdec;
                         break;

                    case DWET_BUTTONUP:
                         stats->DWET_BUTTONUP += incdec;
                         break;

                    case DWET_MOTION:
                         stats->DWET_MOTION += incdec;
                         break;

                    case DWET_ENTER:
                         stats->DWET_ENTER += incdec;
                         break;

                    case DWET_LEAVE:
                         stats->DWET_LEAVE += incdec;
                         break;

                    case DWET_WHEEL:
                         stats->DWET_WHEEL += incdec;
                         break;

                    case DWET_POSITION_SIZE:
                         stats->DWET_POSITION_SIZE += incdec;
                         break;

                    default:
                         D_BUG( "unknown window event type 0x%08x\n", event->window.type );
               }
               break;

          case DFEC_USER:
               stats->DFEC_USER += incdec;
               break;

          case DFEC_VIDEOPROVIDER:
               stats->DFEC_VIDEOPROVIDER +=incdec;

               switch (event->videoprovider.type) {
                    case DVPET_STARTED:
                         stats->DVPET_STARTED += incdec;
                         break;

                    case DVPET_STOPPED:
                         stats->DVPET_STOPPED += incdec;
                         break;

                    case DVPET_SPEEDCHANGE:
                         stats->DVPET_SPEEDCHANGE += incdec;
                         break;

                    case DVPET_STREAMCHANGE:
                         stats->DVPET_STREAMCHANGE += incdec;
                         break;

                    case DVPET_FATALERROR:
                         stats->DVPET_FATALERROR += incdec;
                         break;
                    
                    case DVPET_FINISHED:
                         stats->DVPET_FINISHED += incdec;
                         break;
                         
                    case DVPET_SURFACECHANGE:
                         stats->DVPET_SURFACECHANGE += incdec;
                         break;

                    case DVPET_FRAMEDECODED:
                         stats->DVPET_FRAMEDECODED += incdec;
                         break;

                    case DVPET_FRAMEDISPLAYED:
                         stats->DVPET_FRAMEDISPLAYED += incdec;
                         break;

                    case DVPET_DATAEXHAUSTED:
                         stats->DVPET_DATAEXHAUSTED += incdec;
                         break;

                    case DVPET_VIDEOACTION:
                         stats->DVPET_VIDEOACTION += incdec;
                         break;

                    case DVPET_DATALOW:
                         stats->DVPET_DATALOW += incdec;
                         break;

                   case DVPET_DATAHIGH:
                         stats->DVPET_DATAHIGH += incdec;
                         break;

                   case DVPET_BUFFERTIMELOW:
                         stats->DVPET_BUFFERTIMELOW += incdec;
                         break;

                   case DVPET_BUFFERTIMEHIGH:
                         stats->DVPET_BUFFERTIMEHIGH += incdec;
                         break;

                    default:
                         D_BUG( "unknown video provider event type 0x%08x\n", event->videoprovider.type );
               }
               break;

          case DFEC_UNIVERSAL:
               stats->DFEC_UNIVERSAL += incdec;
               break;

          default:
               D_BUG( "unknown event class 0x%08x\n", event->clazz );
     }
}

