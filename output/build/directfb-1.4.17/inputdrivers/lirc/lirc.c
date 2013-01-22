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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <directfb.h>
#include <directfb_keynames.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/input.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/thread.h>
#include <direct/util.h>

#include <core/input_driver.h>


DFB_INPUT_DRIVER( lirc )

static DirectFBKeySymbolNames(keynames);

static bool keynames_sorted = false;

typedef struct {
     CoreInputDevice  *device;
     DirectThread *thread;

     int           fd;
} LircData;


static int keynames_compare (const void *key,
                             const void *base)
{
  return strcmp ((const char *) key,
                 ((const struct DFBKeySymbolName *) base)->name);
}

static int keynames_sort_compare (const void *d1,
                                  const void *d2)
{
  return strcmp (((const struct DFBKeySymbolName *) d1)->name,
                 ((const struct DFBKeySymbolName *) d2)->name);
}

static DFBInputDeviceKeySymbol lirc_parse_line(const char *line)
{
     struct DFBKeySymbolName *symbol_name;
     char *s, *name;

     if (!keynames_sorted) {
          qsort ( keynames,
                  D_ARRAY_SIZE( keynames ),
                  sizeof(keynames[0]),
                  (__compar_fn_t) keynames_sort_compare );
          keynames_sorted = true;
     }

     s = strchr( line, ' ' );
     if (!s || !s[1])
          return DIKS_NULL;

     s = strchr( ++s, ' ' );
     if (!s|| !s[1])
          return DIKS_NULL;

     name = ++s;

     s = strchr( name, ' ' );
     if (s)
          *s = '\0';

     switch (strlen( name )) {
          case 0:
               return DIKS_NULL;
          case 1:
               return (DFBInputDeviceKeySymbol) name[0];
          default:
               symbol_name = bsearch( name, keynames,
                                      D_ARRAY_SIZE( keynames ),
                                      sizeof(keynames[0]),
                                      (__compar_fn_t) keynames_compare );
               if (symbol_name)
                    return symbol_name->symbol;
               break;
     }

     return DIKS_NULL;
}

static void*
lircEventThread( DirectThread *thread, void *driver_data )
{
     int                      repeats = 0;
     DFBInputDeviceKeySymbol  last    = DIKS_NULL;
     LircData                *data    = (LircData*) driver_data;

     while (true) {
          struct timeval          tv;
          fd_set                  set;
          int                     result;
          int                     readlen;
          char                    buf[128];
          DFBInputEvent           evt;
          DFBInputDeviceKeySymbol symbol;

          FD_ZERO(&set);
          FD_SET(data->fd,&set);

          tv.tv_sec  = 0;
          tv.tv_usec = 200000;

          result = select (data->fd+1, &set, NULL, NULL, &tv);

          switch (result) {
               case -1:
                    /* error */
                    if (errno == EINTR)
                         continue;

                    D_PERROR("DirectFB/LIRC: select() failed\n");
                    return NULL;

               case 0:
                    /* timeout, release last key */
                    if (last != DIKS_NULL) {
                         evt.flags      = DIEF_KEYSYMBOL;
                         evt.type       = DIET_KEYRELEASE;
                         evt.key_symbol = last;

                         dfb_input_dispatch( data->device, &evt );

                         last = DIKS_NULL;
                    }
                    continue;

               default:
                    /* data arrived */
                    break;
          }

          direct_thread_testcancel( thread );

          /* read data */
          readlen = read( data->fd, buf, 128 );
          if (readlen < 1)
               continue;

          /* get new key */
          symbol = lirc_parse_line( buf );
          if (symbol == DIKS_NULL)
               continue;

          /* repeated key? */
          if (symbol == last) {
               /* swallow the first three repeats */
               if (++repeats < 4)
                    continue;
          }
          else {
               /* reset repeat counter */
               repeats = 0;

               /* release previous key if not released yet */
               if (last != DIKS_NULL) {
                    evt.flags      = DIEF_KEYSYMBOL;
                    evt.type       = DIET_KEYRELEASE;
                    evt.key_symbol = last;

                    dfb_input_dispatch( data->device, &evt );
               }
          }

          /* send the press event */
          evt.flags      = DIEF_KEYSYMBOL;
          evt.type       = DIET_KEYPRESS;
          evt.key_symbol = symbol;

          dfb_input_dispatch( data->device, &evt );

          /* remember last key */
          last = symbol;
     }

     return NULL;
}

/* exported symbols */

static int
driver_get_available( void )
{
     int fd;
     struct sockaddr_un addr;

     addr.sun_family = AF_UNIX;
     direct_snputs( addr.sun_path, "/dev/lircd", sizeof(addr.sun_path) );

     fd = socket( PF_UNIX, SOCK_STREAM, 0 );
     if (fd < 0)
          return 0;

     if (connect( fd, (struct sockaddr*)&addr, sizeof(addr) ) < 0) {
          close( fd );
          return 0;
     }

     close( fd );

     return 1;
}

static void
driver_get_info( InputDriverInfo *info )
{
     /* fill driver info structure */
     snprintf( info->name,
               DFB_INPUT_DRIVER_INFO_NAME_LENGTH, "LIRC Driver" );

     snprintf( info->vendor,
               DFB_INPUT_DRIVER_INFO_VENDOR_LENGTH, "directfb.org" );

     info->version.major = 0;
     info->version.minor = 2;
}

static DFBResult
driver_open_device( CoreInputDevice      *device,
                    unsigned int      number,
                    InputDeviceInfo  *info,
                    void            **driver_data )
{
     int                 fd;
     LircData           *data;
     struct sockaddr_un  sa;

     /* create socket */
     sa.sun_family = AF_UNIX;
     direct_snputs( sa.sun_path, "/dev/lircd", sizeof(sa.sun_path) );

     fd = socket( PF_UNIX, SOCK_STREAM, 0 );
     if (fd < 0) {
          D_PERROR( "DirectFB/LIRC: socket" );
          return DFB_INIT;
     }

     /* initiate connection */
     if (connect( fd, (struct sockaddr*)&sa, sizeof(sa) ) < 0) {
          D_PERROR( "DirectFB/LIRC: connect" );
          close( fd );
          return DFB_INIT;
     }

     /* fill driver info structure */
     snprintf( info->desc.name,
               DFB_INPUT_DEVICE_DESC_NAME_LENGTH, "LIRC Device" );

     snprintf( info->desc.vendor,
               DFB_INPUT_DEVICE_DESC_VENDOR_LENGTH, "Unknown" );

     info->prefered_id = DIDID_REMOTE;

     info->desc.type   = DIDTF_REMOTE;
     info->desc.caps   = DICAPS_KEYS;

     /* allocate and fill private data */
     data = D_CALLOC( 1, sizeof(LircData) );
     if (!data) {
          close( fd );
          return D_OOM();
     }

     data->fd     = fd;
     data->device = device;

     /* start input thread */
     data->thread = direct_thread_create( DTT_INPUT, lircEventThread, data, "LiRC Input" );

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
     LircData *data = (LircData*) driver_data;

     /* stop input thread */
     direct_thread_cancel( data->thread );
     direct_thread_join( data->thread );
     direct_thread_destroy( data->thread );

     /* close socket */
     close( data->fd );

     /* free private data */
     D_FREE( data );
}

