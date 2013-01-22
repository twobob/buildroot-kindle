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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#include <directfb.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/input.h>

#include <misc/conf.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/memcpy.h>
#include <direct/thread.h>

#include <core/input_driver.h>

#define FIX_PRECISION 8
#define inttofix(A) ((A) << FIX_PRECISION)
#define fixtoint(A) ((A) >> FIX_PRECISION)
#define fixmult(A, B) (((A) * (B)) >> FIX_PRECISION)
#define fixdiv(A, B) (((A) << FIX_PRECISION) / (B))

DFB_INPUT_DRIVER( ucb1x00_ts )

typedef struct {
     CoreInputDevice   *device;
     DirectThread  *thread;

     int            fd;
} ucb1x00TSData;

/* I copied this from the kernel driver since there is no include file */
typedef struct {
     u16               pressure;
     u16               x;
     u16               y;
     u16               pad;
     struct timeval  stamp;
} TS_EVENT;

static struct {
     int xmin;
     int xmax;
     int ymin;
     int ymax;
     int xswap;
     int yswap;
     int zthresh;
     int jthresh;
     int numevents;
     int xres;
     int yres;
} config = { 50, 930, 50, 930, 1, 1, 50, 5, 8, 640, 480};

/* needed for event filter */
static TS_EVENT *event_buffer = NULL;

static int
avg_events(int which, int count)
{
     int i, sum = 0;

     for(i = 0; i < count; i++)
          sum += which ? event_buffer[i].x : event_buffer[i].y;

     return(sum / count);
}

static void
filter_event(TS_EVENT *ts_event)
{
     int dx, dy;
     static int lastx = 0, lasty = 0;
     static int lastb = 0;
     static int evcnt = 0;
     static int event_index = 0;

     /* Increment the buffer index and the count of items in the buffer. */
     if(++evcnt > config.numevents) evcnt = config.numevents;
     if(++event_index == config.numevents) event_index = 0;

     /* If the pen has just been pressed down, reset the counters. */
     if(ts_event->pressure > config.zthresh) {
          if(!lastb) {
               lastb = 1;
               evcnt = 1;
               event_index = 0;
          }
     } else lastb = 0;

     /* Store this event in the circular buffer. */
     direct_memcpy(&event_buffer[event_index], ts_event, sizeof(TS_EVENT));

     /* Don't try to average and filter if we only have one reading. */
     if(evcnt > 1) {
          /* Average the closest values */
          ts_event->y = avg_events(0, evcnt);
          ts_event->x = avg_events(1, evcnt);

          /* Ignore movements which are below the minimum threshold */
          dx = ts_event->x - lastx;
          if(dx < 0) dx = -dx;
          if(dx < config.jthresh) ts_event->x = lastx;
          dy = ts_event->y - lasty;
          if(dy < 0) dy = -dy;
          if(dy < config.jthresh) ts_event->y = lasty;
     }

     /* Remember the values we are returning. */
     lastx = ts_event->x;
     lasty = ts_event->y;
}

static void
scale_point( TS_EVENT *ts_event )
{
     int x = ts_event->x;
     int y = ts_event->y;

     /* Clip any values outside the expected range. */
     if(x > config.xmax) x = config.xmax;
     if(x < config.xmin) x = config.xmin;
     if(y > config.ymax) y = config.ymax;
     if(y < config.ymin) y = config.ymin;

     /* ((x - config.xmin) / (config.xmax - config.xmin)) * config.xres */
     x = fixtoint(fixmult(fixdiv(inttofix(x - config.xmin),
                  inttofix(config.xmax - config.xmin)), inttofix(config.xres)));

     /* ((y - config.ymin) / (config.ymax - config.ymin)) * config.yres */
     y = fixtoint(fixmult(fixdiv(inttofix(y - config.ymin),
                  inttofix(config.ymax - config.ymin)), inttofix(config.yres)));

     /* Swap the X and/or Y axes if necessary.*/
     ts_event->x = config.xswap ? (config.xres - x) : x;
     ts_event->y = config.yswap ? (config.yres - y) : y;
}

static void *
ucb1x00tsEventThread( DirectThread *thread, void *driver_data )
{
     ucb1x00TSData *data = (ucb1x00TSData*) driver_data;

     TS_EVENT ts_event;

     int readlen;

     unsigned short old_x = -1;
     unsigned short old_y = -1;
     unsigned short old_pressure = 0;

     while ((readlen = read(data->fd, &ts_event, sizeof(TS_EVENT))) > 0  ||
            errno == EINTR)
     {
          DFBInputEvent evt;

          direct_thread_testcancel( thread );

          if (readlen < 1)
               continue;

          filter_event( &ts_event );
          scale_point( &ts_event );
	
	  ts_event.pressure = (ts_event.pressure > config.zthresh );
	
          if (ts_event.pressure) {
               if (ts_event.x != old_x) {
                    evt.type    = DIET_AXISMOTION;
                    evt.flags   = DIEF_AXISABS;
                    evt.axis    = DIAI_X;
                    evt.axisabs = ts_event.x;

                    dfb_input_dispatch( data->device, &evt );

                    old_x = ts_event.x;
               }

               if (ts_event.y != old_y) {
                    evt.type    = DIET_AXISMOTION;
                    evt.flags   = DIEF_AXISABS;
                    evt.axis    = DIAI_Y;
                    evt.axisabs = ts_event.y;

                    dfb_input_dispatch( data->device, &evt );

                    old_y = ts_event.y;
               }
          }

          if ((ts_event.pressure && !old_pressure) ||
              (!ts_event.pressure && old_pressure)) {
               evt.type   = (ts_event.pressure ?
                             DIET_BUTTONPRESS : DIET_BUTTONRELEASE);
               evt.flags  = DIEF_NONE;
               evt.button = DIBI_LEFT;

               dfb_input_dispatch( data->device, &evt );

               old_pressure = ts_event.pressure;
          }
     }

     if (readlen <= 0)
          D_PERROR ("ucb1x00 Touchscreen thread died\n");

     return NULL;
}


/* exported symbols */

static int
driver_get_available( void )
{
     int fd;

     fd = open( "/dev/ucb1x00-ts", O_RDONLY | O_NOCTTY );
     if (fd < 0)
          return 0;

     close( fd );

     return 1;
}

static void
driver_get_info( InputDriverInfo *info )
{
     /* fill driver info structure */
     snprintf( info->name,
               DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "ucb1x00 Touchscreen Driver" );

     snprintf( info->vendor,
               DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH,
               "fischlustig" );

     info->version.major = 0;
     info->version.minor = 4;
}

static DFBResult
driver_open_device( CoreInputDevice      *device,
                    unsigned int      number,
                    InputDeviceInfo  *info,
                    void            **driver_data )
{
     int          fd;
     ucb1x00TSData *data;

     /* open device */
     fd = open( "/dev/ucb1x00-ts", O_RDONLY | O_NOCTTY );
     if (fd < 0) {
          D_PERROR( "DirectFB/ucb1x00: Error opening `/dev/ucb1x00-ts'!\n" );
          return DFB_INIT;
     }

     /* fill device info structure */
     snprintf( info->desc.name,
               DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "ucb1x00 Touchscreen" );

     snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "Unknown" );

     info->prefered_id     = DIDID_MOUSE;

     info->desc.type       = DIDTF_MOUSE;
     info->desc.caps       = DICAPS_AXES | DICAPS_BUTTONS;
     info->desc.max_axis   = DIAI_Y;
     info->desc.max_button = DIBI_LEFT;

     /* allocate and fill private data */
     data = D_CALLOC( 1, sizeof(ucb1x00TSData) );
     if (!data) {
          close( fd );
          return D_OOM();
     }

     data->fd     = fd;
     data->device = device;

     event_buffer = malloc( sizeof( TS_EVENT) * config.numevents );

     /* start input thread */
     data->thread = direct_thread_create( DTT_INPUT, ucb1x00tsEventThread, data, "UCB TS Input" );

     /* set private data pointer */
     *driver_data = data;

     return DFB_OK;
}

/*
 * Fetch one entry from the device's keymap if supported.
 */
static DFBResult
driver_get_keymap_entry( CoreInputDevice               *device,
                         void                      *driver_data,
                         DFBInputDeviceKeymapEntry *entry )
{
     return DFB_UNSUPPORTED;
}

static void
driver_close_device( void *driver_data )
{
     ucb1x00TSData *data = (ucb1x00TSData*) driver_data;

     /* stop input thread */
     direct_thread_cancel( data->thread );
     direct_thread_join( data->thread );
     direct_thread_destroy( data->thread );

     /* close device */
     if (close( data->fd ) < 0)
          D_PERROR( "DirectFB/ucb1x00: Error closing `/dev/ucb1x00-ts'!\n" );

     if (event_buffer)
          free( event_buffer );

     /* free private data */
     D_FREE( data );
}
