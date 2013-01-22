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
#include <string.h>

#include <directfb.h>

#include <direct/list.h>

#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/core_parts.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>

#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/modules.h>

DEFINE_MODULE_DIRECTORY( dfb_core_systems, "systems", DFB_CORE_SYSTEM_ABI_VERSION );


D_DEBUG_DOMAIN( Core_System, "Core/System", "DirectFB System Core" );

/**********************************************************************************************************************/

typedef struct {
     int                  magic;

     CoreSystemInfo       system_info;
} DFBSystemCoreShared;

struct __DFB_DFBSystemCore {
     int                  magic;

     CoreDFB             *core;

     DFBSystemCoreShared *shared;
};


DFB_CORE_PART( system_core, SystemCore );

/**********************************************************************************************************************/

static DFBSystemCoreShared   *system_field  = NULL;    /* FIXME */

static DirectModuleEntry     *system_module = NULL;    /* FIXME */
static const CoreSystemFuncs *system_funcs  = NULL;    /* FIXME */
static CoreSystemInfo         system_info;             /* FIXME */
static void                  *system_data   = NULL;    /* FIXME */

/**********************************************************************************************************************/

static DFBResult
dfb_system_core_initialize( CoreDFB             *core,
                            DFBSystemCore       *data,
                            DFBSystemCoreShared *shared )
{
     DFBResult ret;

     D_DEBUG_AT( Core_System, "dfb_system_core_initialize( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_ASSERT( shared != NULL );

     data->core   = core;
     data->shared = shared;


     system_field = shared;    /* FIXME */

     system_field->system_info = system_info;

     ret = system_funcs->Initialize( core, &system_data );
     if (ret)
          return ret;


     D_MAGIC_SET( data, DFBSystemCore );
     D_MAGIC_SET( shared, DFBSystemCoreShared );

     return DFB_OK;
}

static DFBResult
dfb_system_core_join( CoreDFB             *core,
                      DFBSystemCore       *data,
                      DFBSystemCoreShared *shared )
{
     DFBResult ret;

     D_DEBUG_AT( Core_System, "dfb_system_core_join( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_MAGIC_ASSERT( shared, DFBSystemCoreShared );

     data->core   = core;
     data->shared = shared;


     system_field = shared;    /* FIXME */

     if (system_field->system_info.type != system_info.type ||
         strcmp( system_field->system_info.name, system_info.name ))
     {
          D_ERROR( "DirectFB/core/system: "
                    "running system '%s' doesn't match system '%s'!\n",
                    system_field->system_info.name, system_info.name );

          system_field = NULL;

          return DFB_UNSUPPORTED;
     }

     if (system_field->system_info.version.major != system_info.version.major ||
         system_field->system_info.version.minor != system_info.version.minor)
     {
          D_ERROR( "DirectFB/core/system: running system version '%d.%d' "
                    "doesn't match version '%d.%d'!\n",
                    system_field->system_info.version.major,
                    system_field->system_info.version.minor,
                    system_info.version.major,
                    system_info.version.minor );

          system_field = NULL;

          return DFB_UNSUPPORTED;
     }

     ret = system_funcs->Join( core, &system_data );
     if (ret)
          return ret;


     D_MAGIC_SET( data, DFBSystemCore );

     return DFB_OK;
}

static DFBResult
dfb_system_core_shutdown( DFBSystemCore *data,
                          bool           emergency )
{
     DFBResult         ret;
     DFBSystemCoreShared *shared;

     D_DEBUG_AT( Core_System, "dfb_system_core_shutdown( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBSystemCore );
     D_MAGIC_ASSERT( data->shared, DFBSystemCoreShared );

     shared = data->shared;


     ret = system_funcs->Shutdown( emergency );

     direct_module_unref( system_module );

     system_module = NULL;
     system_funcs  = NULL;
     system_field  = NULL;
     system_data   = NULL;


     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( shared );

     return ret;
}

static DFBResult
dfb_system_core_leave( DFBSystemCore *data,
                       bool           emergency )
{
     DFBResult         ret;
     DFBSystemCoreShared *shared;

     D_DEBUG_AT( Core_System, "dfb_system_core_leave( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBSystemCore );
     D_MAGIC_ASSERT( data->shared, DFBSystemCoreShared );

     shared = data->shared;


     ret = system_funcs->Leave( emergency );

     direct_module_unref( system_module );

     system_module = NULL;
     system_funcs  = NULL;
     system_field  = NULL;
     system_data   = NULL;


     D_MAGIC_CLEAR( data );

     return ret;
}

static DFBResult
dfb_system_core_suspend( DFBSystemCore *data )
{
     DFBSystemCoreShared *shared;

     D_DEBUG_AT( Core_System, "dfb_system_core_suspend( %p )\n", data );

     D_MAGIC_ASSERT( data, DFBSystemCore );
     D_MAGIC_ASSERT( data->shared, DFBSystemCoreShared );

     shared = data->shared;

     return system_funcs->Suspend();
}

static DFBResult
dfb_system_core_resume( DFBSystemCore *data )
{
     DFBSystemCoreShared *shared;

     D_DEBUG_AT( Core_System, "dfb_system_core_resume( %p )\n", data );

     D_MAGIC_ASSERT( data, DFBSystemCore );
     D_MAGIC_ASSERT( data->shared, DFBSystemCoreShared );

     shared = data->shared;

     return system_funcs->Resume();
}

/**********************************************************************************************************************/

DFBResult
dfb_system_lookup( void )
{
     DirectLink *l;

     direct_modules_explore_directory( &dfb_core_systems );

     direct_list_foreach( l, dfb_core_systems.entries ) {
          DirectModuleEntry     *module = (DirectModuleEntry*) l;
          const CoreSystemFuncs *funcs;

          funcs = direct_module_ref( module );
          if (!funcs)
               continue;

          if (!system_module || (!dfb_config->system ||
              !strcasecmp( dfb_config->system, module->name )))
          {
               if (system_module)
                    direct_module_unref( system_module );

               system_module = module;
               system_funcs  = funcs;

               funcs->GetSystemInfo( &system_info );
          }
          else
               direct_module_unref( module );
     }

     if (!system_module) {
          D_ERROR("DirectFB/core/system: No system found!\n");

          return DFB_NOIMPL;
     }

     return DFB_OK;
}

CoreSystemType
dfb_system_type( void )
{
     return system_info.type;
}

CoreSystemCapabilities
dfb_system_caps( void )
{
     return system_info.caps;
}

void *
dfb_system_data( void )
{
     return system_data;
}

volatile void *
dfb_system_map_mmio( unsigned int    offset,
                     int             length )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->MapMMIO( offset, length );
}

void
dfb_system_unmap_mmio( volatile void  *addr,
                       int             length )
{
     D_ASSERT( system_funcs != NULL );

     system_funcs->UnmapMMIO( addr, length );
}

int
dfb_system_get_accelerator( void )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->GetAccelerator();
}

VideoMode *
dfb_system_modes( void )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->GetModes();
}

VideoMode *
dfb_system_current_mode( void )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->GetCurrentMode();
}

DFBResult
dfb_system_thread_init( void )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->ThreadInit();
}

bool
dfb_system_input_filter( CoreInputDevice *device,
                         DFBInputEvent   *event )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->InputFilter( device, event );
}

unsigned long
dfb_system_video_memory_physical( unsigned int offset )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->VideoMemoryPhysical( offset );
}

void *
dfb_system_video_memory_virtual( unsigned int offset )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->VideoMemoryVirtual( offset );
}

unsigned int
dfb_system_videoram_length( void )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->VideoRamLength();
}

unsigned long
dfb_system_aux_memory_physical( unsigned int offset )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->AuxMemoryPhysical( offset );
}

void *
dfb_system_aux_memory_virtual( unsigned int offset )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->AuxMemoryVirtual( offset );
}

unsigned int
dfb_system_auxram_length( void )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->AuxRamLength();
}

void
dfb_system_get_busid( int *ret_bus, int *ret_dev, int *ret_func )
{
     int bus = -1, dev = -1, func = -1;
     
     D_ASSERT( system_funcs != NULL );

     system_funcs->GetBusID( &bus, &dev, &func );

     if (ret_bus)
          *ret_bus = bus;
     if (ret_dev)
          *ret_dev = dev;
     if (ret_func)
          *ret_func = func;
}

void
dfb_system_get_deviceid( unsigned int *ret_vendor_id,
                         unsigned int *ret_device_id )
{
     unsigned int vendor_id = 0, device_id = 0;

     D_ASSERT( system_funcs != NULL );

     system_funcs->GetDeviceID( &vendor_id, &device_id );

     if (ret_vendor_id)
          *ret_vendor_id = vendor_id;
     if (ret_device_id)
          *ret_device_id = device_id;
}

int
dfb_system_surface_data_size( void )
{
     D_ASSERT( system_funcs != NULL );

     return system_funcs->SurfaceDataSize();
}

void
dfb_system_surface_data_init( CoreSurface *surface, void *data )
{
     D_ASSERT( surface );
     D_ASSERT( system_funcs != NULL );

     system_funcs->SurfaceDataInit(surface,data);
}

void
dfb_system_surface_data_destroy( CoreSurface *surface, void *data )
{
     D_ASSERT( surface );
     D_ASSERT( system_funcs != NULL );

     system_funcs->SurfaceDataDestroy(surface,data);
}

