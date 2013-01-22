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
#include <string.h>

#include <directfb.h>

#include <fusion/arena.h>
#include <fusion/shmalloc.h>

#include <Carbon/Carbon.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/messages.h>

#include "osx.h"
#include "primary.h"

#include <core/core_system.h>

DFB_CORE_SYSTEM( osx )


DFBOSX  *dfb_osx      = NULL;
CoreDFB *dfb_osx_core = NULL;


static void
system_get_info( CoreSystemInfo *info )
{
     info->type = CORE_OSX;

     snprintf( info->name, DFB_CORE_SYSTEM_INFO_NAME_LENGTH, "OSX" );
}

static DFBResult
system_initialize( CoreDFB *core, void **data )
{
     char       *driver;
     CoreScreen *screen;

     D_ASSERT( dfb_osx == NULL );

     dfb_osx = (DFBOSX*) SHCALLOC( dfb_core_shmpool(dfb_osx_core), 1, sizeof(DFBOSX) );
     if (!dfb_osx) {
          D_ERROR( "DirectFB/OSX: Couldn't allocate shared memory!\n" );
          return D_OOSHM();
     }

     dfb_osx_core = core;

     /* Initialize OSX */
     fusion_skirmish_init( &dfb_osx->lock, "OSX System", dfb_core_world(core) );

     fusion_call_init( &dfb_osx->call, dfb_osx_call_handler, NULL, dfb_core_world(core) );

     screen = dfb_screens_register( NULL, NULL, osxPrimaryScreenFuncs );

     dfb_layers_register( screen, NULL, osxPrimaryLayerFuncs );

     fusion_arena_add_shared_field( dfb_core_arena( core ), "OSX", dfb_osx );

     *data = dfb_osx;

     return DFB_OK;
}

static DFBResult
system_join( CoreDFB *core, void **data )
{
     void       *ret;
     CoreScreen *screen;

     D_ASSERT( dfb_osx == NULL );

     fusion_arena_get_shared_field( dfb_core_arena( core ), "OSX", &ret );

     dfb_osx = ret;
     dfb_osx_core = core;

     screen = dfb_screens_register( NULL, NULL, osxPrimaryScreenFuncs );

     dfb_layers_register( screen, NULL, osxPrimaryLayerFuncs );

     *data = dfb_osx;

     return DFB_OK;
}

static DFBResult
system_shutdown( bool emergency )
{
     D_ASSERT( dfb_osx != NULL );

     fusion_call_destroy( &dfb_osx->call );

     fusion_skirmish_prevail( &dfb_osx->lock );

     fusion_skirmish_destroy( &dfb_osx->lock );

     SHFREE( dfb_core_shmpool(dfb_osx_core), dfb_osx );
     dfb_osx = NULL;
     dfb_osx_core = NULL;

     return DFB_OK;
}

static DFBResult
system_leave( bool emergency )
{
     D_ASSERT( dfb_osx != NULL );

     dfb_osx = NULL;
     dfb_osx_core = NULL;

     return DFB_OK;
}

static DFBResult
system_suspend( void )
{
     return DFB_UNIMPLEMENTED;
}

static DFBResult
system_resume( void )
{
     return DFB_UNIMPLEMENTED;
}

static volatile void *
system_map_mmio( unsigned int    offset,
                 int             length )
{
    return NULL;
}

static void
system_unmap_mmio( volatile void  *addr,
                   int             length )
{
}

static int
system_get_accelerator( void )
{
     return -1;
}

static VideoMode *
system_get_modes( void )
{
     return NULL;
}

static VideoMode *
system_get_current_mode( void )
{
     return NULL;
}

static DFBResult
system_thread_init( void )
{
     return DFB_OK;
}

static bool
system_input_filter( CoreInputDevice   *device,
                     DFBInputEvent *event )
{
     return false;
}

static unsigned long
system_video_memory_physical( unsigned int offset )
{
     return 0;
}

static void *
system_video_memory_virtual( unsigned int offset )
{
     return NULL;
}

static unsigned int
system_videoram_length( void )
{
     return 0;
}

static unsigned long
system_aux_memory_physical( unsigned int offset )
{
     return 0;
}

static void *
system_aux_memory_virtual( unsigned int offset )
{
     return NULL;
}

static unsigned int
system_auxram_length( void )
{
     return 0;
}

static void
system_get_busid( int *ret_bus, int *ret_dev, int *ret_func )
{
     return;
}

static int
system_surface_data_size( void )
{
     /* Return zero because shared surface data is unneeded. */
     return 0;
}

static void
system_surface_data_init( CoreSurface *surface, void *data )
{
     /* Ignore since unneeded. */
     return;
}

static void
system_surface_data_destroy( CoreSurface *surface, void *data )
{
     /* Ignore since unneeded. */
     return;
}

static void
system_get_deviceid( unsigned int *ret_vendor_id,
                     unsigned int *ret_device_id )
{
     return;
}

