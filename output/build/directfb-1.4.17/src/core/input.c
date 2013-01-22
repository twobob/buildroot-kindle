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
#include <unistd.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <directfb.h>
#include <directfb_keynames.h>

#include <direct/debug.h>
#include <direct/list.h>
#include <direct/memcpy.h>
#include <direct/messages.h>


#include <fusion/shmalloc.h>
#include <fusion/reactor.h>
#include <fusion/arena.h>

#include <core/CoreInputDevice.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/core_parts.h>

#include <core/gfxcard.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/system.h>
#include <core/layer_context.h>
#include <core/layer_control.h>
#include <core/layer_region.h>
#include <core/layers.h>
#include <core/input.h>
#include <core/windows.h>
#include <core/windows_internal.h>

#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/modules.h>
#include <direct/trace.h>

#include <fusion/build.h>

#include <misc/conf.h>
#include <misc/util.h>

#include <gfx/convert.h>


#define CHECK_INTERVAL 20000  // Microseconds
#define CHECK_NUMBER   200


D_DEBUG_DOMAIN( Core_Input,    "Core/Input",     "DirectFB Input Core" );
D_DEBUG_DOMAIN( Core_InputEvt, "Core/Input/Evt", "DirectFB Input Core Events & Dispatch" );


DEFINE_MODULE_DIRECTORY( dfb_input_modules, "inputdrivers", DFB_INPUT_DRIVER_ABI_VERSION );

/**********************************************************************************************************************/

typedef enum {
     CIDC_RELOAD_KEYMAP
} CoreInputDeviceCommand;

typedef struct {
     DirectLink               link;

     int                      magic;

     DirectModuleEntry       *module;

     const InputDriverFuncs  *funcs;

     InputDriverInfo          info;

     int                      nr_devices;
} InputDriver;

typedef struct {
     int                          min_keycode;
     int                          max_keycode;
     int                          num_entries;
     DFBInputDeviceKeymapEntry   *entries;
} InputDeviceKeymap;

typedef struct {
     int                          magic;

     DFBInputDeviceID             id;            /* unique device id */

     int                          num;

     InputDeviceInfo              device_info;

     InputDeviceKeymap            keymap;

     CoreInputDeviceState         state;

     DFBInputDeviceKeyIdentifier  last_key;      /* last key pressed */
     DFBInputDeviceKeySymbol      last_symbol;   /* last symbol pressed */
     bool                         first_press;   /* first press of key */

     FusionReactor               *reactor;       /* event dispatcher */
     FusionSkirmish               lock;

     unsigned int                 axis_num;
     DFBInputDeviceAxisInfo      *axis_info;
     FusionRef                    ref; /* Ref between shared device & local device */

     FusionCall                   call;
} InputDeviceShared;

struct __DFB_CoreInputDevice {
     DirectLink          link;

     int                 magic;

     InputDeviceShared  *shared;

     InputDriver        *driver;
     void               *driver_data;

     CoreDFB            *core;
};

/**********************************************************************************************************************/

DirectResult
CoreInputDevice_Call( CoreInputDevice     *device,
                      FusionCallExecFlags  flags,
                      int                  call_arg,
                      void                *ptr,
                      unsigned int         length,
                      void                *ret_ptr,
                      unsigned int         ret_size,
                      unsigned int        *ret_length )
{
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     return fusion_call_execute3( &device->shared->call, flags, call_arg, ptr, length, ret_ptr, ret_size, ret_length );
}

/**********************************************************************************************************************/

typedef struct {
     int                 magic;

     int                 num;
     InputDeviceShared  *devices[MAX_INPUTDEVICES];
     FusionReactor      *reactor; /* For input hot-plug event */
} DFBInputCoreShared;

struct __DFB_DFBInputCore {
     int                 magic;

     CoreDFB            *core;

     DFBInputCoreShared *shared;

     DirectLink         *drivers;
     DirectLink         *devices;
};


DFB_CORE_PART( input_core, InputCore );

/**********************************************************************************************************************/

typedef struct {
     DFBInputDeviceKeySymbol      target;
     DFBInputDeviceKeySymbol      result;
} DeadKeyCombo;

typedef struct {
     DFBInputDeviceKeySymbol      deadkey;
     const DeadKeyCombo          *combos;
} DeadKeyMap;

/* Data struct of input device hotplug event */
typedef struct {
     bool             is_plugin;   /* Hotplug in or not */
     int              dev_id;      /* Input device ID*/
     struct timeval   stamp;       /* Time stamp of event */
} InputDeviceHotplugEvent;

/**********************************************************************************************************************/

static const DeadKeyCombo combos_grave[] = {         
     { DIKS_SPACE,     (unsigned char) '`' },        
     { DIKS_SMALL_A,   (unsigned char) 0xe0 }, // '�'
     { DIKS_SMALL_E,   (unsigned char) 0xe8 }, // '�'
     { DIKS_SMALL_I,   (unsigned char) 0xec }, // '�'
     { DIKS_SMALL_O,   (unsigned char) 0xf2 }, // '�'
     { DIKS_SMALL_U,   (unsigned char) 0xf9 }, // '�'
     { DIKS_CAPITAL_A, (unsigned char) 0xc0 }, // '�'
     { DIKS_CAPITAL_E, (unsigned char) 0xc8 }, // '�'
     { DIKS_CAPITAL_I, (unsigned char) 0xcc }, // '�'
     { DIKS_CAPITAL_O, (unsigned char) 0xd2 }, // '�'
     { DIKS_CAPITAL_U, (unsigned char) 0xd9 }, // '�'
     { 0, 0 }                                        
};                                                   
                                                     
static const DeadKeyCombo combos_acute[] = {         
     { DIKS_SPACE,     (unsigned char) '\'' },       
     { DIKS_SMALL_A,   (unsigned char) '�' },        
     { DIKS_SMALL_E,   (unsigned char) '�' },        
     { DIKS_SMALL_I,   (unsigned char) '�' },        
     { DIKS_SMALL_O,   (unsigned char) '�' },        
     { DIKS_SMALL_U,   (unsigned char) '�' },        
     { DIKS_SMALL_Y,   (unsigned char) '�' },        
     { DIKS_CAPITAL_A, (unsigned char) '�' },        
     { DIKS_CAPITAL_E, (unsigned char) '�' },        
     { DIKS_CAPITAL_I, (unsigned char) '�' },        
     { DIKS_CAPITAL_O, (unsigned char) '�' },        
     { DIKS_CAPITAL_U, (unsigned char) '�' },        
     { DIKS_CAPITAL_Y, (unsigned char) '�' },        
     { 0, 0 }                                        
};                                                   
                                                     
static const DeadKeyCombo combos_circumflex[] = {    
     { DIKS_SPACE,     (unsigned char) '^' },        
     { DIKS_SMALL_A,   (unsigned char) '�' },        
     { DIKS_SMALL_E,   (unsigned char) '�' },        
     { DIKS_SMALL_I,   (unsigned char) '�' },        
     { DIKS_SMALL_O,   (unsigned char) '�' },        
     { DIKS_SMALL_U,   (unsigned char) '�' },        
     { DIKS_CAPITAL_A, (unsigned char) '�' },        
     { DIKS_CAPITAL_E, (unsigned char) '�' },        
     { DIKS_CAPITAL_I, (unsigned char) '�' },        
     { DIKS_CAPITAL_O, (unsigned char) '�' },        
     { DIKS_CAPITAL_U, (unsigned char) '�' },        
     { 0, 0 }                                        
};                                                   
                                                     
static const DeadKeyCombo combos_diaeresis[] = {     
     { DIKS_SPACE,     (unsigned char) '�' },        
     { DIKS_SMALL_A,   (unsigned char) '�' },        
     { DIKS_SMALL_E,   (unsigned char) '�' },        
     { DIKS_SMALL_I,   (unsigned char) '�' },        
     { DIKS_SMALL_O,   (unsigned char) '�' },        
     { DIKS_SMALL_U,   (unsigned char) '�' },        
     { DIKS_CAPITAL_A, (unsigned char) '�' },        
     { DIKS_CAPITAL_E, (unsigned char) '�' },        
     { DIKS_CAPITAL_I, (unsigned char) '�' },        
     { DIKS_CAPITAL_O, (unsigned char) '�' },        
     { DIKS_CAPITAL_U, (unsigned char) '�' },        
     { 0, 0 }                                        
};                                                   
                                                     
static const DeadKeyCombo combos_tilde[] = {         
     { DIKS_SPACE,     (unsigned char) '~' },        
     { DIKS_SMALL_A,   (unsigned char) '�' },        
     { DIKS_SMALL_N,   (unsigned char) '�' },        
     { DIKS_SMALL_O,   (unsigned char) '�' },        
     { DIKS_CAPITAL_A, (unsigned char) '�' },        
     { DIKS_CAPITAL_N, (unsigned char) '�' },        
     { DIKS_CAPITAL_O, (unsigned char) '�' },        
     { 0, 0 }                                        
};

static const DeadKeyMap deadkey_maps[] = {           
     { DIKS_DEAD_GRAVE,      combos_grave },         
     { DIKS_DEAD_ACUTE,      combos_acute },         
     { DIKS_DEAD_CIRCUMFLEX, combos_circumflex },    
     { DIKS_DEAD_DIAERESIS,  combos_diaeresis },     
     { DIKS_DEAD_TILDE,      combos_tilde }          
};

/* define a lookup table to go from key IDs to names.
 * This is used to look up the names provided in a loaded key table */
/* this table is roughly 4Kb in size */
DirectFBKeySymbolNames(KeySymbolNames);
DirectFBKeyIdentifierNames(KeyIdentifierNames);

/**********************************************************************************************************************/

static void init_devices( CoreDFB *core );

static void allocate_device_keymap( CoreDFB *core, CoreInputDevice *device );

static DFBInputDeviceKeymapEntry *get_keymap_entry( CoreInputDevice *device,
                                                    int              code );

static DFBResult set_keymap_entry( CoreInputDevice                 *device,
                                   int                              code,
                                   const DFBInputDeviceKeymapEntry *entry );

static DFBResult load_keymap( CoreInputDevice           *device,
                              char                      *filename );

static DFBResult reload_keymap( CoreInputDevice *device );

static DFBInputDeviceKeySymbol     lookup_keysymbol( char *symbolname );
static DFBInputDeviceKeyIdentifier lookup_keyidentifier( char *identifiername );

/**********************************************************************************************************************/

static bool lookup_from_table( CoreInputDevice    *device,
                               DFBInputEvent      *event,
                               DFBInputEventFlags  lookup );

static void fixup_key_event  ( CoreInputDevice    *device,
                               DFBInputEvent      *event );

static void fixup_mouse_event( CoreInputDevice    *device,
                               DFBInputEvent      *event );

static void flush_keys       ( CoreInputDevice    *device );

static bool core_input_filter( CoreInputDevice    *device,
                               DFBInputEvent      *event );

/**********************************************************************************************************************/

static DFBInputDeviceKeyIdentifier symbol_to_id( DFBInputDeviceKeySymbol     symbol );

static DFBInputDeviceKeySymbol     id_to_symbol( DFBInputDeviceKeyIdentifier id,
                                                 DFBInputDeviceModifierMask  modifiers,
                                                 DFBInputDeviceLockState     locks );

/**********************************************************************************************************************/

static ReactionResult local_processing_hotplug( const void *msg_data, void *ctx );

/**********************************************************************************************************************/

static ReactionFunc dfb_input_globals[MAX_INPUT_GLOBALS+1] = {
/* 0 */   _dfb_windowstack_inputdevice_listener,
          NULL
};

DFBResult
dfb_input_add_global( ReactionFunc  func,
                      int          *ret_index )
{
     int i;

     D_DEBUG_AT( Core_Input, "%s( %p, %p )\n", __FUNCTION__, func, ret_index );

     D_ASSERT( func != NULL );
     D_ASSERT( ret_index != NULL );

     for (i=0; i<MAX_INPUT_GLOBALS; i++) {
          if (!dfb_input_globals[i]) {
               dfb_input_globals[i] = func;

               D_DEBUG_AT( Core_Input, "  -> index %d\n", i );

               *ret_index = i;

               return DFB_OK;
          }
     }

     return DFB_LIMITEXCEEDED;
}

DFBResult
dfb_input_set_global( ReactionFunc func,
                      int          index )
{
     D_DEBUG_AT( Core_Input, "%s( %p, %d )\n", __FUNCTION__, func, index );

     D_ASSERT( func != NULL );
     D_ASSERT( index >= 0 );
     D_ASSERT( index < MAX_INPUT_GLOBALS );

     D_ASSUME( dfb_input_globals[index] == NULL );

     dfb_input_globals[index] = func;

     return DFB_OK;
}

/**********************************************************************************************************************/

static DFBInputCore       *core_local; /* FIXME */
static DFBInputCoreShared *core_input; /* FIXME */

#if FUSION_BUILD_MULTI
static Reaction            local_processing_react; /* Local reaction to hot-plug event */
#endif


static DFBResult
dfb_input_core_initialize( CoreDFB            *core,
                           DFBInputCore       *data,
                           DFBInputCoreShared *shared )
{
#if FUSION_BUILD_MULTI
     DFBResult result = DFB_OK;
#endif

     D_DEBUG_AT( Core_Input, "dfb_input_core_initialize( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_ASSERT( shared != NULL );

     core_local = data;   /* FIXME */
     core_input = shared; /* FIXME */

     data->core   = core;
     data->shared = shared;


     direct_modules_explore_directory( &dfb_input_modules );

#if FUSION_BUILD_MULTI
     /* Create the reactor that responds input device hot-plug events. */
     core_input->reactor = fusion_reactor_new(
                              sizeof( InputDeviceHotplugEvent ),
                              "Input Hotplug",
                              dfb_core_world(core) );
     if (!core_input->reactor) {
          D_ERROR( "DirectFB/Input: fusion_reactor_new() failed!\n" );
          result = DFB_FAILURE;
          goto errorExit;
     }

     fusion_reactor_add_permissions( core_input->reactor, 0, FUSION_REACTOR_PERMIT_ATTACH_DETACH );

     /* Attach local process function to the input hot-plug reactor. */
     result = fusion_reactor_attach(
                              core_input->reactor,
                              local_processing_hotplug,
                              (void*) core,
                              &local_processing_react );
     if (result) {
          D_ERROR( "DirectFB/Input: fusion_reactor_attach() failed!\n" );
          goto errorExit;
     }
#endif

     init_devices( core );


     D_MAGIC_SET( data, DFBInputCore );
     D_MAGIC_SET( shared, DFBInputCoreShared );

     return DFB_OK;

#if FUSION_BUILD_MULTI
errorExit:
     /* Destroy the hot-plug reactor if it was created. */
     if (core_input->reactor)
          fusion_reactor_destroy(core_input->reactor);

     return result;
#endif
}

static DFBResult
dfb_input_core_join( CoreDFB            *core,
                     DFBInputCore       *data,
                     DFBInputCoreShared *shared )
{
     int i;
#if FUSION_BUILD_MULTI
     DFBResult result;
#endif

     D_DEBUG_AT( Core_Input, "dfb_input_core_join( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_MAGIC_ASSERT( shared, DFBInputCoreShared );
     D_ASSERT( shared->reactor != NULL );

     core_local = data;   /* FIXME */
     core_input = shared; /* FIXME */

     data->core   = core;
     data->shared = shared;

#if FUSION_BUILD_MULTI
     /* Attach the local process function to the input hot-plug reactor. */
     result = fusion_reactor_attach( core_input->reactor,
                                     local_processing_hotplug,
                                     (void*) core,
                                     &local_processing_react );
     if (result) {
          D_ERROR( "DirectFB/Input: fusion_reactor_attach failed!\n" );
          return result;
     }
#endif

     for (i=0; i<core_input->num; i++) {
          CoreInputDevice *device;

          device = D_CALLOC( 1, sizeof(CoreInputDevice) );
          if (!device) {
               D_OOM();
               continue;
          }

          device->shared = core_input->devices[i];

#if FUSION_BUILD_MULTI
          /* Increase the reference counter. */
          fusion_ref_up( &device->shared->ref, false );
#endif

          /* add it to the list */
          direct_list_append( &data->devices, &device->link );

          D_MAGIC_SET( device, CoreInputDevice );
     }


     D_MAGIC_SET( data, DFBInputCore );

     return DFB_OK;
}

static DFBResult
dfb_input_core_shutdown( DFBInputCore *data,
                         bool          emergency )
{
     DFBInputCoreShared  *shared;
     DirectLink          *n;
     CoreInputDevice     *device;
     FusionSHMPoolShared *pool = dfb_core_shmpool( data->core );
     InputDriver         *driver;

     D_DEBUG_AT( Core_Input, "dfb_input_core_shutdown( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBInputCore );
     D_MAGIC_ASSERT( data->shared, DFBInputCoreShared );

     shared = data->shared;

     /* Stop each input provider's hot-plug thread that supports device hot-plugging. */
     direct_list_foreach_safe (driver, n, core_local->drivers) {
          if (driver->funcs->GetCapability && driver->funcs->StopHotplug) {
               if (IDC_HOTPLUG & driver->funcs->GetCapability()) {
                    D_DEBUG_AT( Core_Input, "Stopping hot-plug detection thread "
                                "within %s\n ", driver->module->name );
                    if (driver->funcs->StopHotplug()) {
                         D_ERROR( "DirectFB/Input: StopHotplug() failed with %s\n",
                                  driver->module->name );
                    }
               }
          }
     }

#if FUSION_BUILD_MULTI
     fusion_reactor_detach( core_input->reactor, &local_processing_react );
     fusion_reactor_destroy( core_input->reactor );
#endif

     direct_list_foreach_safe (device, n, data->devices) {
          InputDeviceShared *devshared;

          D_MAGIC_ASSERT( device, CoreInputDevice );

          driver = device->driver;
          D_ASSERT( driver != NULL );

          devshared = device->shared;
          D_ASSERT( devshared != NULL );

          CoreInputDevice_Deinit_Dispatch( &devshared->call );

          fusion_skirmish_destroy( &devshared->lock );

          if (device->driver_data != NULL) {
               void *driver_data;

               D_ASSERT( driver->funcs != NULL );
               D_ASSERT( driver->funcs->CloseDevice != NULL );

               D_DEBUG_AT( Core_Input, "  -> closing '%s' (%d) %d.%d (%s)\n",
                           devshared->device_info.desc.name, devshared->num + 1,
                           driver->info.version.major,
                           driver->info.version.minor, driver->info.vendor );

               driver_data = device->driver_data;
               device->driver_data = NULL;
               driver->funcs->CloseDevice( driver_data );
          }

          if (!--driver->nr_devices) {
               direct_module_unref( driver->module );
               D_FREE( driver );
          }

#if FUSION_BUILD_MULTI
          fusion_ref_destroy( &device->shared->ref );
#endif

          fusion_reactor_free( devshared->reactor );

          if (devshared->keymap.entries)
               SHFREE( pool, devshared->keymap.entries );
  
          if (devshared->axis_info)
               SHFREE( pool, devshared->axis_info );

          SHFREE( pool, devshared );

          D_MAGIC_CLEAR( device );

          D_FREE( device );
     }

     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( shared );

     return DFB_OK;
}

static DFBResult
dfb_input_core_leave( DFBInputCore *data,
                      bool          emergency )
{
     DFBInputCoreShared *shared;
     DirectLink         *n;
     CoreInputDevice    *device;

     D_DEBUG_AT( Core_Input, "dfb_input_core_leave( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBInputCore );
     D_MAGIC_ASSERT( data->shared, DFBInputCoreShared );

     shared = data->shared;

#if FUSION_BUILD_MULTI
     fusion_reactor_detach( core_input->reactor, &local_processing_react );
#endif

     direct_list_foreach_safe (device, n, data->devices) {
          D_MAGIC_ASSERT( device, CoreInputDevice );

#if FUSION_BUILD_MULTI
          /* Decrease the ref between shared device and local device. */
          fusion_ref_down( &device->shared->ref, false );
#endif

          D_FREE( device );
     }


     D_MAGIC_CLEAR( data );

     return DFB_OK;
}

static DFBResult
dfb_input_core_suspend( DFBInputCore *data )
{
     DFBInputCoreShared *shared;
     CoreInputDevice    *device;
     InputDriver        *driver;

     D_DEBUG_AT( Core_Input, "dfb_input_core_suspend( %p )\n", data );

     D_MAGIC_ASSERT( data, DFBInputCore );
     D_MAGIC_ASSERT( data->shared, DFBInputCoreShared );

     shared = data->shared;

     D_DEBUG_AT( Core_Input, "  -> suspending...\n" );

     /* Go through the drivers list and attempt to suspend all of the drivers that
      * support the Suspend function.
      */
     direct_list_foreach (driver, core_local->drivers) {
          DFBResult ret;

          D_ASSERT( driver->funcs->Suspend != NULL );
          ret = driver->funcs->Suspend();

          if (ret != DFB_OK && ret != DFB_UNSUPPORTED) {
               D_DERROR( ret, "driver->Suspend failed during suspend (%s)\n",
                         driver->info.name );
          }
     }

     direct_list_foreach (device, data->devices) {
          InputDeviceShared *devshared;
          
          D_MAGIC_ASSERT( device, CoreInputDevice );
          
          driver = device->driver;
          D_ASSERT( driver != NULL );
          
          devshared = device->shared;
          D_ASSERT( devshared != NULL );

          if (device->driver_data != NULL) {
               void *driver_data;

               D_ASSERT( driver->funcs != NULL );
               D_ASSERT( driver->funcs->CloseDevice != NULL );

               D_DEBUG_AT( Core_Input, "  -> closing '%s' (%d) %d.%d (%s)\n",
                           devshared->device_info.desc.name, devshared->num + 1,
                           driver->info.version.major,
                           driver->info.version.minor, driver->info.vendor );

               driver_data = device->driver_data;
               device->driver_data = NULL;
               driver->funcs->CloseDevice( driver_data );
          }

          flush_keys( device );
     }

     D_DEBUG_AT( Core_Input, "  -> suspended.\n" );

     return DFB_OK;
}

static DFBResult
dfb_input_core_resume( DFBInputCore *data )
{
     DFBInputCoreShared *shared;
     DFBResult           ret;
     CoreInputDevice    *device;
     InputDriver        *driver;

     D_DEBUG_AT( Core_Input, "dfb_input_core_resume( %p )\n", data );

     D_MAGIC_ASSERT( data, DFBInputCore );
     D_MAGIC_ASSERT( data->shared, DFBInputCoreShared );

     shared = data->shared;

     D_DEBUG_AT( Core_Input, "  -> resuming...\n" );

     direct_list_foreach (device, data->devices) {
          D_MAGIC_ASSERT( device, CoreInputDevice );

          D_DEBUG_AT( Core_Input, "  -> reopening '%s' (%d) %d.%d (%s)\n",
                      device->shared->device_info.desc.name, device->shared->num + 1,
                      device->driver->info.version.major,
                      device->driver->info.version.minor,
                      device->driver->info.vendor );

          D_ASSERT( device->driver_data == NULL );

          ret = device->driver->funcs->OpenDevice( device, device->shared->num,
                                                   &device->shared->device_info,
                                                   &device->driver_data );
          if (ret) {
               D_DERROR( ret, "DirectFB/Input: Failed reopening device "
                         "during resume (%s)!\n", device->shared->device_info.desc.name );
               device->driver_data = NULL;
          }
     }

     /* Go through the drivers list and attempt to resume all of the drivers that
      * support the Resume function.
      */
     direct_list_foreach (driver, core_local->drivers) {
          D_ASSERT( driver->funcs->Resume != NULL );

          ret = driver->funcs->Resume();
          if (ret != DFB_OK && ret != DFB_UNSUPPORTED) {
               D_DERROR( ret, "driver->Resume failed during resume (%s)\n",
                         driver->info.name );
          }
     }

     D_DEBUG_AT( Core_Input, "  -> resumed.\n" );

     return DFB_OK;
}

void
dfb_input_enumerate_devices( InputDeviceCallback         callback,
                             void                       *ctx,
                             DFBInputDeviceCapabilities  caps )
{
     CoreInputDevice *device;

     D_ASSERT( core_input != NULL );

     direct_list_foreach (device, core_local->devices) {
          DFBInputDeviceCapabilities dev_caps;

          D_MAGIC_ASSERT( device, CoreInputDevice );
          D_ASSERT( device->shared != NULL );

          dev_caps = device->shared->device_info.desc.caps;

          /* Always match if unclassified */
          if (!dev_caps)
               dev_caps = DICAPS_ALL;

          if ((dev_caps & caps) && callback( device, ctx ) == DFENUM_CANCEL)
               break;
     }
}

DirectResult
dfb_input_attach( CoreInputDevice *device,
                  ReactionFunc     func,
                  void            *ctx,
                  Reaction        *reaction )
{
     D_DEBUG_AT( Core_Input, "%s( %p, %p, %p, %p )\n", __FUNCTION__, device, func, ctx, reaction );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     return fusion_reactor_attach( device->shared->reactor, func, ctx, reaction );
}

DirectResult
dfb_input_detach( CoreInputDevice *device,
                  Reaction        *reaction )
{
     D_DEBUG_AT( Core_Input, "%s( %p, %p )\n", __FUNCTION__, device, reaction );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     return fusion_reactor_detach( device->shared->reactor, reaction );
}

DirectResult
dfb_input_attach_global( CoreInputDevice *device,
                         int              index,
                         void            *ctx,
                         GlobalReaction  *reaction )
{
     D_DEBUG_AT( Core_Input, "%s( %p, %d, %p, %p )\n", __FUNCTION__, device, index, ctx, reaction );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     return fusion_reactor_attach_global( device->shared->reactor, index, ctx, reaction );
}

DirectResult
dfb_input_detach_global( CoreInputDevice *device,
                         GlobalReaction  *reaction )
{
     D_DEBUG_AT( Core_Input, "%s( %p, %p )\n", __FUNCTION__, device, reaction );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     return fusion_reactor_detach_global( device->shared->reactor, reaction );
}

const char *
dfb_input_event_type_name( DFBInputEventType type )
{
     switch (type) {
          case DIET_UNKNOWN:
               return "UNKNOWN";

          case DIET_KEYPRESS:
               return "KEYPRESS";

          case DIET_KEYRELEASE:
               return "KEYRELEASE";

          case DIET_BUTTONPRESS:
               return "BUTTONPRESS";

          case DIET_BUTTONRELEASE:
               return "BUTTONRELEASE";

          case DIET_AXISMOTION:
               return "AXISMOTION";

          default:
               break;
     }

     return "<invalid>";
}

void
dfb_input_dispatch( CoreInputDevice *device, DFBInputEvent *event )
{
     D_DEBUG_AT( Core_Input, "%s( %p, %p )\n", __FUNCTION__, device, event );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( event != NULL );

     /*
      * When a USB device is hot-removed, it is possible that there are pending events
      * still being dispatched and the shared field becomes NULL.
      */

     /*
      * 0. Sanity checks & debugging...
      */
     if (!device->shared) {
          D_DEBUG_AT( Core_Input, "  -> No shared data!\n" );
          return;
     }

     D_ASSUME( device->shared->reactor != NULL );

     if (!device->shared->reactor) {
          D_DEBUG_AT( Core_Input, "  -> No reactor!\n" );
          return;
     }

     D_DEBUG_AT( Core_InputEvt, "  -> (%02x) %s%s%s\n", event->type,
                 dfb_input_event_type_name( event->type ),
                 (event->flags & DIEF_FOLLOW) ? " [FOLLOW]" : "",
                 (event->flags & DIEF_REPEAT) ? " [REPEAT]" : "" );

#if D_DEBUG_ENABLED
     if (event->flags & DIEF_TIMESTAMP)
          D_DEBUG_AT( Core_InputEvt, "  -> TIMESTAMP  %lu.%06lu\n", event->timestamp.tv_sec, event->timestamp.tv_usec );
     if (event->flags & DIEF_AXISABS)
          D_DEBUG_AT( Core_InputEvt, "  -> AXISABS    %d at %d\n",  event->axis, event->axisabs );
     if (event->flags & DIEF_AXISREL)
          D_DEBUG_AT( Core_InputEvt, "  -> AXISREL    %d by %d\n",  event->axis, event->axisrel );
     if (event->flags & DIEF_KEYCODE)
          D_DEBUG_AT( Core_InputEvt, "  -> KEYCODE    %d\n",        event->key_code );
     if (event->flags & DIEF_KEYID)
          D_DEBUG_AT( Core_InputEvt, "  -> KEYID      0x%04x\n",    event->key_id );
     if (event->flags & DIEF_KEYSYMBOL)
          D_DEBUG_AT( Core_InputEvt, "  -> KEYSYMBOL  0x%04x\n",    event->key_symbol );
     if (event->flags & DIEF_MODIFIERS)
          D_DEBUG_AT( Core_InputEvt, "  -> MODIFIERS  0x%04x\n",    event->modifiers );
     if (event->flags & DIEF_LOCKS)
          D_DEBUG_AT( Core_InputEvt, "  -> LOCKS      0x%04x\n",    event->locks );
     if (event->flags & DIEF_BUTTONS)
          D_DEBUG_AT( Core_InputEvt, "  -> BUTTONS    0x%04x\n",    event->buttons );
     if (event->flags & DIEF_GLOBAL)
          D_DEBUG_AT( Core_InputEvt, "  -> GLOBAL\n" );
#endif

     /*
      * 1. Fixup event...
      */
     event->clazz     = DFEC_INPUT;
     event->device_id = device->shared->id;

     if (!(event->flags & DIEF_TIMESTAMP)) {
          gettimeofday( &event->timestamp, NULL );
          event->flags |= DIEF_TIMESTAMP;
     }

     switch (event->type) {
          case DIET_BUTTONPRESS:
          case DIET_BUTTONRELEASE:
               D_DEBUG_AT( Core_InputEvt, "  -> BUTTON     0x%04x\n", event->button );

               if (dfb_config->lefty) {
                    if (event->button == DIBI_LEFT)
                         event->button = DIBI_RIGHT;
                    else if (event->button == DIBI_RIGHT)
                         event->button = DIBI_LEFT;

                    D_DEBUG_AT( Core_InputEvt, "  -> lefty!  => 0x%04x <=\n", event->button );
               }
               /* fallthru */

          case DIET_AXISMOTION:
               fixup_mouse_event( device, event );
               break;

          case DIET_KEYPRESS:
          case DIET_KEYRELEASE:
               if (dfb_config->capslock_meta) {
                    if (device->shared->keymap.num_entries && (event->flags & DIEF_KEYCODE))
                         lookup_from_table( device, event, (DIEF_KEYID |
                                                            DIEF_KEYSYMBOL) & ~event->flags );

                    if (event->key_id == DIKI_CAPS_LOCK || event->key_symbol == DIKS_CAPS_LOCK) {
                         event->flags     |= DIEF_KEYID | DIEF_KEYSYMBOL;
                         event->key_code   = -1;
                         event->key_id     = DIKI_META_L;
                         event->key_symbol = DIKS_META;
                    }
               }

               fixup_key_event( device, event );
               break;

          default:
               ;
     }

#if D_DEBUG_ENABLED
     if (event->flags & DIEF_TIMESTAMP)
          D_DEBUG_AT( Core_InputEvt, "  => TIMESTAMP  %lu.%06lu\n", event->timestamp.tv_sec, event->timestamp.tv_usec );
     if (event->flags & DIEF_AXISABS)
          D_DEBUG_AT( Core_InputEvt, "  => AXISABS    %d at %d\n",  event->axis, event->axisabs );
     if (event->flags & DIEF_AXISREL)
          D_DEBUG_AT( Core_InputEvt, "  => AXISREL    %d by %d\n",  event->axis, event->axisrel );
     if (event->flags & DIEF_KEYCODE)
          D_DEBUG_AT( Core_InputEvt, "  => KEYCODE    %d\n",        event->key_code );
     if (event->flags & DIEF_KEYID)
          D_DEBUG_AT( Core_InputEvt, "  => KEYID      0x%04x\n",    event->key_id );
     if (event->flags & DIEF_KEYSYMBOL)
          D_DEBUG_AT( Core_InputEvt, "  => KEYSYMBOL  0x%04x\n",    event->key_symbol );
     if (event->flags & DIEF_MODIFIERS)
          D_DEBUG_AT( Core_InputEvt, "  => MODIFIERS  0x%04x\n",    event->modifiers );
     if (event->flags & DIEF_LOCKS)
          D_DEBUG_AT( Core_InputEvt, "  => LOCKS      0x%04x\n",    event->locks );
     if (event->flags & DIEF_BUTTONS)
          D_DEBUG_AT( Core_InputEvt, "  => BUTTONS    0x%04x\n",    event->buttons );
     if (event->flags & DIEF_GLOBAL)
          D_DEBUG_AT( Core_InputEvt, "  => GLOBAL\n" );
#endif

     if (core_input_filter( device, event ))
          D_DEBUG_AT( Core_InputEvt, "  ****>> FILTERED\n" );
     else
          fusion_reactor_dispatch( device->shared->reactor, event, true, dfb_input_globals );
}

DFBInputDeviceID
dfb_input_device_id( const CoreInputDevice *device )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     return device->shared->id;
}

CoreInputDevice *
dfb_input_device_at( DFBInputDeviceID id )
{
     CoreInputDevice *device;

     D_ASSERT( core_input != NULL );

     direct_list_foreach (device, core_local->devices) {
          D_MAGIC_ASSERT( device, CoreInputDevice );

          if (device->shared->id == id)
               return device;
     }

     return NULL;
}

/* Get an input device's capabilities. */
DFBInputDeviceCapabilities
dfb_input_device_caps( const CoreInputDevice *device )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     return device->shared->device_info.desc.caps;
}

void
dfb_input_device_description( const CoreInputDevice     *device,
                              DFBInputDeviceDescription *desc )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     *desc = device->shared->device_info.desc;
}

DFBResult
dfb_input_device_get_keymap_entry( CoreInputDevice           *device,
                                   int                        keycode,
                                   DFBInputDeviceKeymapEntry *entry )
{
     DFBInputDeviceKeymapEntry *keymap_entry;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( entry != NULL );

     keymap_entry = get_keymap_entry( device, keycode );
     if (!keymap_entry)
          return DFB_FAILURE;

     *entry = *keymap_entry;

     return DFB_OK;
}

DFBResult
dfb_input_device_set_keymap_entry( CoreInputDevice                 *device,
                                   int                              keycode,
                                   const DFBInputDeviceKeymapEntry *entry )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( entry != NULL );

     return set_keymap_entry( device, keycode, entry );
}

DFBResult
dfb_input_device_load_keymap   ( CoreInputDevice           *device,
                                 char                      *filename )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( filename != NULL );

     return load_keymap( device, filename );
}

DFBResult
dfb_input_device_reload_keymap( CoreInputDevice *device )
{
     InputDeviceShared *shared;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     shared = device->shared;
     D_ASSERT( shared != NULL );

     D_INFO( "DirectFB/Input: Reloading keymap for '%s' [0x%02x]...\n",
             shared->device_info.desc.name, shared->id );

     return reload_keymap( device );
}

DFBResult
dfb_input_device_get_state( CoreInputDevice      *device,
                            CoreInputDeviceState *ret_state )
{
     InputDeviceShared *shared;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     shared = device->shared;
     D_ASSERT( shared != NULL );

     *ret_state = shared->state;

     return DFB_OK;
}

/** internal **/

static void
input_add_device( CoreInputDevice *device )
{
     D_DEBUG_AT( Core_Input, "%s( %p )\n", __FUNCTION__, device );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     if (core_input->num == MAX_INPUTDEVICES) {
          D_ERROR( "DirectFB/Input: Maximum number of devices reached!\n" );
          return;
     }

     direct_list_append( &core_local->devices, &device->link );

     core_input->devices[ core_input->num++ ] = device->shared;
}

static void
allocate_device_keymap( CoreDFB *core, CoreInputDevice *device )
{
     int                        i;
     DFBInputDeviceKeymapEntry *entries;
     FusionSHMPoolShared       *pool        = dfb_core_shmpool( core );
     InputDeviceShared         *shared      = device->shared;
     DFBInputDeviceDescription *desc        = &shared->device_info.desc;
     int                        num_entries = desc->max_keycode -
                                              desc->min_keycode + 1;

     D_DEBUG_AT( Core_Input, "%s( %p, %p )\n", __FUNCTION__, core, device );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );

     entries = SHCALLOC( pool, num_entries, sizeof(DFBInputDeviceKeymapEntry) );
     if (!entries) {
          D_OOSHM();
          return;
     }

     /* write -1 indicating entry is not fetched yet from driver */
     for (i=0; i<num_entries; i++)
          entries[i].code = -1;

     shared->keymap.min_keycode = desc->min_keycode;
     shared->keymap.max_keycode = desc->max_keycode;
     shared->keymap.num_entries = num_entries;
     shared->keymap.entries     = entries;

#if FUSION_BUILD_MULTI
     /* we need to fetch the whole map, otherwise a slave would try to */
     for (i=desc->min_keycode; i<=desc->max_keycode; i++)
          get_keymap_entry( device, i );
#endif
}

static int
make_id( DFBInputDeviceID prefered )
{
     CoreInputDevice *device;

     D_DEBUG_AT( Core_Input, "%s( 0x%02x )\n", __FUNCTION__, prefered );

     D_ASSERT( core_input != NULL );

     direct_list_foreach (device, core_local->devices) {
          D_MAGIC_ASSERT( device, CoreInputDevice );

          if (device->shared->id == prefered)
               return make_id( (prefered < DIDID_ANY) ? DIDID_ANY : (prefered + 1) );
     }

     return prefered;
}

static DFBResult
reload_keymap( CoreInputDevice *device )
{
     int                i;
     InputDeviceShared *shared;

     D_DEBUG_AT( Core_Input, "%s( %p )\n", __FUNCTION__, device );

     D_MAGIC_ASSERT( device, CoreInputDevice );

     shared = device->shared;

     D_ASSERT( shared != NULL );

     if (shared->device_info.desc.min_keycode < 0 ||
         shared->device_info.desc.max_keycode < 0)
          return DFB_UNSUPPORTED;

     /* write -1 indicating entry is not fetched yet from driver */
     for (i=0; i<shared->keymap.num_entries; i++)
          shared->keymap.entries[i].code = -1;

     /* fetch the whole map */
     for (i=shared->keymap.min_keycode; i<=shared->keymap.max_keycode; i++)
          get_keymap_entry( device, i );

     D_INFO( "DirectFB/Input: Reloaded keymap for '%s' [0x%02x]\n",
             shared->device_info.desc.name, shared->id );

     return DFB_OK;
}

static DFBResult
init_axes( CoreInputDevice *device )
{
     int                     i, num;
     DFBResult               ret;
     InputDeviceShared      *shared;
     const InputDriverFuncs *funcs;

     D_DEBUG_AT( Core_Input, "%s( %p )\n", __FUNCTION__, device );

     D_MAGIC_ASSERT( device, CoreInputDevice );
     D_ASSERT( device->driver != NULL );

     funcs = device->driver->funcs;
     D_ASSERT( funcs != NULL );

     shared = device->shared;
     D_ASSERT( shared != NULL );

     if (shared->device_info.desc.max_axis < 0)
          return DFB_OK;

     num = shared->device_info.desc.max_axis + 1;

     shared->axis_info = SHCALLOC( dfb_core_shmpool(device->core), num, sizeof(DFBInputDeviceAxisInfo) );
     if (!shared->axis_info)
          return D_OOSHM();

     shared->axis_num = num;

     if (funcs->GetAxisInfo) {
          for (i=0; i<num; i++) {
               ret = funcs->GetAxisInfo( device, device->driver_data, i, &shared->axis_info[i] );
               if (ret)
                    D_DERROR( ret, "Core/Input: GetAxisInfo() failed for '%s' [%d] on axis %d!\n",
                              shared->device_info.desc.name, shared->id, i );
          }
     }

     return DFB_OK;
}

static void
init_devices( CoreDFB *core )
{
     DirectLink          *next;
     DirectModuleEntry   *module;
     FusionSHMPoolShared *pool = dfb_core_shmpool( core );

     D_DEBUG_AT( Core_Input, "%s( %p )\n", __FUNCTION__, core );

     D_ASSERT( core_input != NULL );

     direct_list_foreach_safe (module, next, dfb_input_modules.entries) {
          int                     n;
          InputDriver            *driver;
          const InputDriverFuncs *funcs;
          InputDriverCapability   driver_cap;
          DFBResult               result;

          driver_cap = IDC_NONE;

          funcs = direct_module_ref( module );
          if (!funcs)
               continue;

          driver = D_CALLOC( 1, sizeof(InputDriver) );
          if (!driver) {
               D_OOM();
               direct_module_unref( module );
               continue;
          }

          D_ASSERT( funcs->GetDriverInfo != NULL );

          funcs->GetDriverInfo( &driver->info );

          D_DEBUG_AT( Core_Input, "  -> probing '%s'...\n", driver->info.name );

          driver->nr_devices = funcs->GetAvailable();

          /*
           * If the input provider supports hot-plug, always load the module.
           */
          if (!funcs->GetCapability) {
               D_DEBUG_AT(Core_Input, "InputDriverFuncs::GetCapability is NULL\n");
          }
          else {
               driver_cap = funcs->GetCapability();
          }

          if (!driver->nr_devices && !(driver_cap & IDC_HOTPLUG)) {
               direct_module_unref( module );
               D_FREE( driver );
               continue;
          }

          D_DEBUG_AT( Core_Input, "  -> %d available device(s) provided by '%s'.\n",
                      driver->nr_devices, driver->info.name );

          driver->module = module;
          driver->funcs  = funcs;

          direct_list_prepend( &core_local->drivers, &driver->link );


          for (n=0; n<driver->nr_devices; n++) {
               char               buf[128];
               CoreInputDevice   *device;
               InputDeviceInfo    device_info;
               InputDeviceShared *shared;
               void              *driver_data;

               device = D_CALLOC( 1, sizeof(CoreInputDevice) );
               if (!device) {
                    D_OOM();
                    continue;
               }

               shared = SHCALLOC( pool, 1, sizeof(InputDeviceShared) );
               if (!shared) {
                    D_OOSHM();
                    D_FREE( device );
                    continue;
               }

               device->core = core;

               memset( &device_info, 0, sizeof(InputDeviceInfo) );

               device_info.desc.min_keycode = -1;
               device_info.desc.max_keycode = -1;

               D_MAGIC_SET( device, CoreInputDevice );

               if (funcs->OpenDevice( device, n, &device_info, &driver_data )) {
                    SHFREE( pool, shared );
                    D_MAGIC_CLEAR( device );
                    D_FREE( device );
                    continue;
               }

               D_DEBUG_AT( Core_Input, "  -> opened '%s' (%d) %d.%d (%s)\n",
                           device_info.desc.name, n + 1, driver->info.version.major,
                           driver->info.version.minor, driver->info.vendor );

               if (driver->nr_devices > 1)
                    snprintf( buf, sizeof(buf), "%s (%d)", device_info.desc.name, n+1 );
               else
                    snprintf( buf, sizeof(buf), "%s", device_info.desc.name );

               /* init skirmish */
               fusion_skirmish_init( &shared->lock, buf, dfb_core_world(core) );

               /* create reactor */
               shared->reactor = fusion_reactor_new( sizeof(DFBInputEvent), buf, dfb_core_world(core) );

               fusion_reactor_add_permissions( shared->reactor, 0, FUSION_REACTOR_PERMIT_ATTACH_DETACH );

               fusion_reactor_set_lock( shared->reactor, &shared->lock );

               /* init call */
               CoreInputDevice_Init_Dispatch( core, device, &shared->call );

               /* initialize shared data */
               shared->id          = make_id(device_info.prefered_id);
               shared->num         = n;
               shared->device_info = device_info;
               shared->last_key    = DIKI_UNKNOWN;
               shared->first_press = true;

               /* initialize local data */
               device->shared      = shared;
               device->driver      = driver;
               device->driver_data = driver_data;

               D_INFO( "DirectFB/Input: %s %d.%d (%s)\n",
                       buf, driver->info.version.major,
                       driver->info.version.minor, driver->info.vendor );

#if FUSION_BUILD_MULTI
               /* Initialize the ref between shared device and local device. */
               snprintf( buf, sizeof(buf), "Ref of input device(%d)", shared->id );
               fusion_ref_init( &shared->ref, buf, dfb_core_world(core) );

               fusion_ref_add_permissions( &shared->ref, 0, FUSION_REF_PERMIT_REF_UNREF_LOCAL );

               /* Increase reference counter. */
               fusion_ref_up( &shared->ref, false );
#endif

               if (device_info.desc.min_keycode > device_info.desc.max_keycode) {
                    D_BUG("min_keycode > max_keycode");
                    device_info.desc.min_keycode = -1;
                    device_info.desc.max_keycode = -1;
               }
               else if (device_info.desc.min_keycode >= 0 &&
                        device_info.desc.max_keycode >= 0)
                    allocate_device_keymap( core, device );

               init_axes( device );

               /* add it to the list */
               input_add_device( device );
          }

          /*
           * If the driver supports hot-plug, launch its hot-plug thread to respond to
           * hot-plug events.  Failures in launching the hot-plug thread will only
           * result in no hot-plug feature being available.
           */
          if (driver_cap == IDC_HOTPLUG) {
               result = funcs->LaunchHotplug(core, (void*) driver);

               /* On failure, the input provider can still be used without hot-plug. */
               if (result) {
                    D_INFO( "DirectFB/Input: Failed to enable hot-plug "
                            "detection with %s\n ", driver->info.name);
               }
               else {
                    D_INFO( "DirectFB/Input: Hot-plug detection enabled with %s \n",
                            driver->info.name);
               }
          }
     }
}

/*
 * Create the DFB shared core input device, add the input device into the
 * local device list and shared dev array and broadcast the hot-plug in
 * message to all slaves.
 */
DFBResult
dfb_input_create_device(int device_index, CoreDFB *core_in, void *driver_in)
{
     char                    buf[128];
     CoreInputDevice        *device;
     InputDeviceInfo         device_info;
     InputDeviceShared      *shared;
     void                   *driver_data;
     InputDriver            *driver = NULL;
     const InputDriverFuncs *funcs;
     FusionSHMPoolShared    *pool;
     DFBResult               result;

     D_DEBUG_AT(Core_Input, "Enter: %s()\n", __FUNCTION__);

     driver = (InputDriver *)driver_in;

     pool = dfb_core_shmpool(core_in);
     funcs = driver->funcs;
     if (!funcs) {
          D_ERROR("DirectFB/Input: driver->funcs is NULL\n");
          goto errorExit;
     }

     device = D_CALLOC( 1, sizeof(CoreInputDevice) );
     if (!device) {
          D_OOM();
          goto errorExit;
     }
     shared = SHCALLOC( pool, 1, sizeof(InputDeviceShared) );
     if (!shared) {
          D_OOM();
          D_FREE( device );
          goto errorExit;
     }

     device->core = core_in;

     memset( &device_info, 0, sizeof(InputDeviceInfo) );

     device_info.desc.min_keycode = -1;
     device_info.desc.max_keycode = -1;

     D_MAGIC_SET( device, CoreInputDevice );

     if (funcs->OpenDevice( device, device_index, &device_info, &driver_data )) {
          SHFREE( pool, shared );
          D_MAGIC_CLEAR( device );
          D_FREE( device );
          D_DEBUG_AT( Core_Input,
                      "DirectFB/Input: Cannot open device in %s, at %d in %s\n",
                      __FUNCTION__, __LINE__, __FILE__);
          goto errorExit;
     }

     snprintf( buf, sizeof(buf), "%s (%d)", device_info.desc.name, device_index);

     /* init skirmish */
     result = fusion_skirmish_init( &shared->lock, buf, dfb_core_world(device->core) );
     if (result) {
          funcs->CloseDevice( driver_data );
          SHFREE( pool, shared );
          D_MAGIC_CLEAR( device );
          D_FREE( device );
          D_ERROR("DirectFB/Input: fusion_skirmish_init() failed! in %s, at %d in %s\n",
                  __FUNCTION__, __LINE__, __FILE__);
          goto errorExit;
     }

     /* create reactor */
     shared->reactor = fusion_reactor_new( sizeof(DFBInputEvent),
                                           buf,
                                           dfb_core_world(device->core) );
     if (!shared->reactor) {
          funcs->CloseDevice( driver_data );
          SHFREE( pool, shared );
          D_MAGIC_CLEAR( device );
          D_FREE( device );
          fusion_skirmish_destroy(&shared->lock);
          D_ERROR("DirectFB/Input: fusion_reactor_new() failed! in %s, at %d in %s\n",
                  __FUNCTION__, __LINE__, __FILE__);
          goto errorExit;
     }

     fusion_reactor_add_permissions( shared->reactor, 0, FUSION_REACTOR_PERMIT_ATTACH_DETACH );

     fusion_reactor_set_lock( shared->reactor, &shared->lock );

     /* init call */
     CoreInputDevice_Init_Dispatch( device->core, device, &shared->call );

     /* initialize shared data */
     shared->id          = make_id(device_info.prefered_id);
     shared->num         = device_index;
     shared->device_info = device_info;
     shared->last_key    = DIKI_UNKNOWN;
     shared->first_press = true;

     /* initialize local data */
     device->shared      = shared;
     device->driver      = driver;
     device->driver_data = driver_data;

     D_INFO( "DirectFB/Input: %s %d.%d (%s)\n",
             buf, driver->info.version.major,
             driver->info.version.minor, driver->info.vendor );

#if FUSION_BUILD_MULTI
     snprintf( buf, sizeof(buf), "Ref of input device(%d)", shared->id);
     fusion_ref_init( &shared->ref, buf, dfb_core_world(core_in));
     fusion_ref_add_permissions( &shared->ref, 0, FUSION_REF_PERMIT_REF_UNREF_LOCAL );
     fusion_ref_up( &shared->ref, false );
#endif

     if (device_info.desc.min_keycode > device_info.desc.max_keycode) {
          D_BUG("min_keycode > max_keycode");
          device_info.desc.min_keycode = -1;
          device_info.desc.max_keycode = -1;
     }
     else if (device_info.desc.min_keycode >= 0 && device_info.desc.max_keycode >= 0)
          allocate_device_keymap( device->core, device );

     /* add it into local device list and shared dev array */
     D_DEBUG_AT(Core_Input,
                "In master, add a new device with dev_id=%d\n",
                shared->id);

     input_add_device( device );
     driver->nr_devices++;

     D_DEBUG_AT(Core_Input,
                "Successfully added new input device with dev_id=%d in shared array\n",
                shared->id);

     InputDeviceHotplugEvent    message;

     /* Setup the hot-plug in message. */
     message.is_plugin = true;
     message.dev_id = shared->id;
     gettimeofday (&message.stamp, NULL);

     /* Send the hot-plug in message */
#if FUSION_BUILD_MULTI
     fusion_reactor_dispatch(core_input->reactor, &message, true, NULL);
#else
     local_processing_hotplug((const void *) &message, (void*) core_in);
#endif
     return DFB_OK;

errorExit:
     return DFB_FAILURE;
}

/*
 * Tell whether the DFB input device handling of the system input device
 * indicated by device_index is already created.
 */
static CoreInputDevice *
search_device_created(int device_index, void *driver_in)
{
     DirectLink  *n;
     CoreInputDevice *device;

     D_ASSERT(driver_in != NULL);

     direct_list_foreach_safe(device, n, core_local->devices) {
          if (device->driver != driver_in)
               continue;

          if (device->driver_data == NULL) {
               D_DEBUG_AT(Core_Input,
                          "The device %d has been closed!\n",
                          device->shared->id);
               return NULL;
          }

          if (device->driver->funcs->IsCreated &&
              !device->driver->funcs->IsCreated(device_index, device->driver_data)) {
               return device;
          }
     }

     return NULL;
}

/*
 * Remove the DFB shared core input device handling of the system input device
 * indicated by device_index and broadcast the hot-plug out message to all slaves.
 */
DFBResult
dfb_input_remove_device(int device_index, void *driver_in)
{
     CoreInputDevice    *device;
     InputDeviceShared  *shared;
     FusionSHMPoolShared *pool;
     int                 i;
     int                 found = 0;
     int                 device_id;

     D_DEBUG_AT(Core_Input, "Enter: %s()\n", __FUNCTION__);

     D_ASSERT(driver_in !=NULL);

     device = search_device_created(device_index, driver_in);
     if (device == NULL) {
          D_DEBUG_AT(Core_Input,
                     "DirectFB/input: Failed to find the device[%d] or the device is "
                     "closed.\n",
                     device_index);
          goto errorExit;
     }

     shared = device->shared;
     pool = dfb_core_shmpool( device->core );
     device_id = shared->id;

     D_DEBUG_AT(Core_Input, "Find the device with dev_id=%d\n", device_id);

     device->driver->funcs->CloseDevice( device->driver_data );

     device->driver->nr_devices--;

     InputDeviceHotplugEvent    message;

     /* Setup the hot-plug out message */
     message.is_plugin = false;
     message.dev_id = device_id;
     gettimeofday (&message.stamp, NULL);

     /* Send the hot-plug out message */
#if FUSION_BUILD_MULTI
     fusion_reactor_dispatch( core_input->reactor, &message, true, NULL);

     int    loop = CHECK_NUMBER;

     while (--loop) {
          if (fusion_ref_zero_trylock( &device->shared->ref ) == DR_OK) {
               fusion_ref_unlock(&device->shared->ref);
               break;
          }

          usleep(CHECK_INTERVAL);
     }

     if (!loop)
          D_DEBUG_AT(Core_Input, "Shared device might be connected to by others\n");

     fusion_ref_destroy(&device->shared->ref);
#else
     local_processing_hotplug((const void*) &message, (void*) device->core);
#endif

     /* Remove the device from shared array */
     for (i = 0; i < core_input->num; i++) {
          if (!found && (core_input->devices[i]->id == shared->id))
               found = 1;

          if (found)
               core_input->devices[i] = core_input->devices[(i + 1) % MAX_INPUTDEVICES];
     }
     if (found)
          core_input->devices[core_input->num -1] = NULL;

     core_input->num--;

     CoreInputDevice_Deinit_Dispatch( &shared->call );

     fusion_skirmish_destroy( &shared->lock );

     fusion_reactor_free( shared->reactor );

     if (shared->keymap.entries)
          SHFREE( pool, shared->keymap.entries );

     SHFREE( pool, shared );

     D_DEBUG_AT(Core_Input,
                "Successfully remove the device with dev_id=%d in shared array\n",
                device_id);

     return DFB_OK;

errorExit:
     return DFB_FAILURE;
}

/*
 * Create local input device and add it into the local input devices list.
 */
static CoreInputDevice *
add_device_into_local_list(int dev_id)
{
     int i;
     D_DEBUG_AT(Core_Input, "Enter: %s()\n", __FUNCTION__);
     for (i = 0; i < core_input->num; i++) {
          if (core_input->devices[i]->id == dev_id) {
               CoreInputDevice *device;

               D_DEBUG_AT(Core_Input,
                          "Find the device with dev_id=%d in shared array, and "
                          "allocate local device\n",
                          dev_id);

               device = D_CALLOC( 1, sizeof(CoreInputDevice) );
               if (!device) {
                   return NULL;
               }

               device->shared = core_input->devices[i];

#if FUSION_BUILD_MULTI
               /* Increase the reference counter. */
               fusion_ref_up( &device->shared->ref, false );
#endif

               /* add it to the list */
               direct_list_append( &core_local->devices, &device->link );

               D_MAGIC_SET( device, CoreInputDevice );
               return device;
          }
     }

     return NULL;
}

/*
 * Local input device function that handles hot-plug in/out messages.
 */
static ReactionResult
local_processing_hotplug( const void *msg_data, void *ctx )
{
     const InputDeviceHotplugEvent *message = msg_data;
     CoreInputDevice               *device = NULL;

     D_DEBUG_AT(Core_Input, "Enter: %s()\n", __FUNCTION__);

     D_DEBUG_AT(Core_Input,
                "<PID:%6d> hotplug-in:%d device_id=%d message!\n",
                getpid(),
                message->is_plugin,
                message->dev_id );

     if (message->is_plugin) {

          device = dfb_input_device_at(message->dev_id);

          if (!device){
               /* Update local device list according to shared devices array */
               if (!(device = add_device_into_local_list(message->dev_id))) {
                    D_ERROR("DirectFB/Input: update_local_devices_list() failed\n" );
                    goto errorExit;
               }
          }

          /* attach the device to event containers */
          containers_attach_device( device );
          /* attach the device to stack containers  */
          stack_containers_attach_device( device );
     }
     else {
          device = dfb_input_device_at(message->dev_id);
          if (device) {

               direct_list_remove(&core_local->devices, &device->link);

               containers_detach_device(device);
               stack_containers_detach_device(device);

#if FUSION_BUILD_MULTI
               /* Decrease reference counter. */
               fusion_ref_down( &device->shared->ref, false );
#endif
               D_MAGIC_CLEAR( device );
               D_FREE(device);
          }
          else
               D_ERROR("DirectFB/Input:Can't find the device to be removed!\n");

     }

     return DFB_OK;

errorExit:
     return DFB_FAILURE;
}

static DFBInputDeviceKeymapEntry *
get_keymap_entry( CoreInputDevice *device,
                  int              code )
{
     InputDeviceKeymap         *map;
     DFBInputDeviceKeymapEntry *entry;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     map = &device->shared->keymap;

     /* safety check */
     if (code < map->min_keycode || code > map->max_keycode)
          return NULL;

     /* point to right array index */
     entry = &map->entries[code - map->min_keycode];

     /* need to initialize? */
     if (entry->code != code) {
          DFBResult    ret;
          InputDriver *driver = device->driver;

          if (!driver) {
               D_BUG("seem to be a slave with an empty keymap");
               return NULL;
          }

          /* write keycode to entry */
          entry->code = code;

          /* fetch entry from driver */
          ret = driver->funcs->GetKeymapEntry( device,
                                               device->driver_data, entry );
          if (ret)
               return NULL;

          /* drivers may leave this blank */
          if (entry->identifier == DIKI_UNKNOWN)
               entry->identifier = symbol_to_id( entry->symbols[DIKSI_BASE] );

          if (entry->symbols[DIKSI_BASE_SHIFT] == DIKS_NULL)
               entry->symbols[DIKSI_BASE_SHIFT] = entry->symbols[DIKSI_BASE];

          if (entry->symbols[DIKSI_ALT] == DIKS_NULL)
               entry->symbols[DIKSI_ALT] = entry->symbols[DIKSI_BASE];

          if (entry->symbols[DIKSI_ALT_SHIFT] == DIKS_NULL)
               entry->symbols[DIKSI_ALT_SHIFT] = entry->symbols[DIKSI_ALT];
     }

     return entry;
}

/* replace a single keymap entry with the code-entry pair */
static DFBResult
set_keymap_entry( CoreInputDevice                 *device,
                  int                              code,
                  const DFBInputDeviceKeymapEntry *entry )
{
     InputDeviceKeymap         *map;

     D_ASSERT( device->shared != NULL );
     D_ASSERT( device->shared->keymap.entries != NULL );

     map = &device->shared->keymap;

     /* sanity check */
     if (code < map->min_keycode || code > map->max_keycode)
          return DFB_FAILURE;

     /* copy the entry to the map */
     map->entries[code - map->min_keycode] = *entry;
     
     return DFB_OK;
}

/* replace the complete current keymap with a keymap from a file.
 * the minimum-maximum keycodes of the driver are to be respected. 
 */
static DFBResult
load_keymap( CoreInputDevice           *device,
             char                      *filename )
{
     DFBResult                  ret       = DFB_OK;
     InputDeviceKeymap         *map       = 0;
     FILE                      *file      = 0;
     DFBInputDeviceLockState    lockstate = 0;

     D_ASSERT( device->shared != NULL );
     D_ASSERT( device->shared->keymap.entries != NULL );

     map = &device->shared->keymap;

     /* open the file */
     file = fopen( filename, "r" );
     if( !file )
     {
          return errno2result( errno );
     }

     /* read the file, line by line, and consume the mentioned scancodes */
     while(1)
     {
          int   i;
          int   dummy;
          char  buffer[201];
          int   keycode;
          char  diki[201];
          char  diks[4][201];
          char *b;
          
          DFBInputDeviceKeymapEntry entry = { .code = 0 };
          
          b = fgets( buffer, 200, file );
          if( !b ) {
               if( feof(file) ) {
                    fclose(file);
                    return DFB_OK;
               }
               fclose(file);
               return errno2result(errno);
          }

          /* comment or empty line */
          if( buffer[0]=='#' || strcmp(buffer,"\n")==0 )
               continue;

          /* check for lock state change */
          if( !strncmp(buffer,"capslock:",9) ) { lockstate |=  DILS_CAPS; continue; }
          if( !strncmp(buffer,":capslock",9) ) { lockstate &= ~DILS_CAPS; continue; }
          if( !strncmp(buffer,"numlock:",8)  ) { lockstate |=  DILS_NUM;  continue; }
          if( !strncmp(buffer,":numlock",8)  ) { lockstate &= ~DILS_NUM;  continue; }

          i = sscanf( buffer, " keycode %i = %s = %s %s %s %s %i\n",
                    &keycode, diki, diks[0], diks[1], diks[2], diks[3], &dummy );

          if( i < 3 || i > 6 ) {
               /* we want 1 to 4 key symbols */
               D_INFO( "DirectFB/Input: skipped erroneous input line %s\n", buffer );
               continue;
          }

          if( keycode > map->max_keycode || keycode < map->min_keycode ) {
               D_INFO( "DirectFB/Input: skipped keycode %d out of range\n", keycode );
               continue;
          }

          entry.code       = keycode;
          entry.locks      = lockstate;
          entry.identifier = lookup_keyidentifier( diki );

          switch( i ) {
               case 6:  entry.symbols[3] = lookup_keysymbol( diks[3] );
               case 5:  entry.symbols[2] = lookup_keysymbol( diks[2] );
               case 4:  entry.symbols[1] = lookup_keysymbol( diks[1] );
               case 3:  entry.symbols[0] = lookup_keysymbol( diks[0] );

                    /* fall through */
          }

          switch( i ) {
               case 3:  entry.symbols[1] = entry.symbols[0];
               case 4:  entry.symbols[2] = entry.symbols[0];
               case 5:  entry.symbols[3] = entry.symbols[1];

               /* fall through */
          }

          ret = CoreInputDevice_SetKeymapEntry( device, keycode, &entry );
          if( ret )
               return ret;
     }
}

static DFBInputDeviceKeySymbol lookup_keysymbol( char *symbolname )
{
     int i;

     /* we want uppercase */  
     for( i=0; i<strlen(symbolname); i++ )
          if( symbolname[i] >= 'a' && symbolname[i] <= 'z' )
               symbolname[i] = symbolname[i] - 'a' + 'A';

     for( i=0; i < sizeof (KeySymbolNames) / sizeof (KeySymbolNames[0]); i++ ) {
          if( strcmp( symbolname, KeySymbolNames[i].name ) == 0 )
               return KeySymbolNames[i].symbol;
     }
     
     /* not found, maybe starting with 0x for raw conversion.
      * We are already at uppercase.
      */
     if( symbolname[0]=='0' && symbolname[1]=='X' ) {
          int code=0;
          symbolname+=2;
          while(*symbolname) {
               if( *symbolname >= '0' && *symbolname <= '9' ) {
                    code = code*16 + *symbolname - '0';
               } else if( *symbolname >= 'A' && *symbolname <= 'F' ) {
                    code = code*16 + *symbolname - 'A' + 10;
               } else {
                    /* invalid character */
                    return DIKS_NULL;
               }
               symbolname++;
          }
          return code;
     }
     
     return DIKS_NULL;
}

static DFBInputDeviceKeyIdentifier lookup_keyidentifier( char *identifiername )
{
     int i;

     /* we want uppercase */  
     for( i=0; i<strlen(identifiername); i++ )
          if( identifiername[i] >= 'a' && identifiername[i] <= 'z' )
               identifiername[i] = identifiername[i] - 'a' + 'A';

     for( i=0; i < sizeof (KeyIdentifierNames) / sizeof (KeyIdentifierNames[0]); i++ ) {
          if( strcmp( identifiername, KeyIdentifierNames[i].name ) == 0 )
               return KeyIdentifierNames[i].identifier;
     }
     
     return DIKI_UNKNOWN;
}

static bool
lookup_from_table( CoreInputDevice    *device,
                   DFBInputEvent      *event,
                   DFBInputEventFlags  lookup )
{
     DFBInputDeviceKeymapEntry *entry;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );
     D_ASSERT( event != NULL );

     /* fetch the entry from the keymap, possibly calling the driver */
     entry = get_keymap_entry( device, event->key_code );
     if (!entry)
          return false;

     /* lookup identifier */
     if (lookup & DIEF_KEYID)
          event->key_id = entry->identifier;

     /* lookup symbol */
     if (lookup & DIEF_KEYSYMBOL) {
          DFBInputDeviceKeymapSymbolIndex index =
               (event->modifiers & DIMM_ALTGR) ? DIKSI_ALT : DIKSI_BASE;

          if ((event->modifiers & DIMM_SHIFT) || (entry->locks & event->locks))
               index++;

          /* don't modify modifiers */
          if (DFB_KEY_TYPE( entry->symbols[DIKSI_BASE] ) == DIKT_MODIFIER)
               event->key_symbol = entry->symbols[DIKSI_BASE];
          else
               event->key_symbol = entry->symbols[index];
     }

     return true;
}

static int
find_key_code_by_id( CoreInputDevice             *device,
                     DFBInputDeviceKeyIdentifier  id )
{
     int                i;
     InputDeviceKeymap *map;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     map = &device->shared->keymap;

     for (i=0; i<map->num_entries; i++) {
          DFBInputDeviceKeymapEntry *entry = &map->entries[i];

          if (entry->identifier == id)
               return entry->code;
     }

     return -1;
}

static int
find_key_code_by_symbol( CoreInputDevice         *device,
                         DFBInputDeviceKeySymbol  symbol )
{
     int                i;
     InputDeviceKeymap *map;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     D_ASSERT( core_input != NULL );
     D_ASSERT( device != NULL );
     D_ASSERT( device->shared != NULL );

     map = &device->shared->keymap;

     for (i=0; i<map->num_entries; i++) {
          int                        n;
          DFBInputDeviceKeymapEntry *entry = &map->entries[i];

          for (n=0; n<=DIKSI_LAST; n++)
               if (entry->symbols[n] == symbol)
                    return entry->code;
     }

     return -1;
}

#define FIXUP_KEY_FIELDS     (DIEF_MODIFIERS | DIEF_LOCKS | \
                              DIEF_KEYCODE | DIEF_KEYID | DIEF_KEYSYMBOL)

/*
 * Fill partially missing values for key_code, key_id and key_symbol by
 * translating those that are set. Fix modifiers/locks before if not set.
 *
 *
 * There are five valid constellations that give reasonable values.
 * (not counting the constellation where everything is set)
 *
 * Device has no translation table
 *   1. key_id is set, key_symbol not
 *      -> key_code defaults to -1, key_symbol from key_id (up-translation)
 *   2. key_symbol is set, key_id not
 *      -> key_code defaults to -1, key_id from key_symbol (down-translation)
 *
 * Device has a translation table
 *   3. key_code is set
 *      -> look up key_id and/or key_symbol (key_code being the index)
 *   4. key_id is set
 *      -> look up key_code and possibly key_symbol (key_id being searched for)
 *   5. key_symbol is set
 *      -> look up key_code and key_id (key_symbol being searched for)
 *
 * Fields remaining will be set to the default, e.g. key_code to -1.
 */
static void
fixup_key_event( CoreInputDevice *device, DFBInputEvent *event )
{
     int                 i;
     DFBInputEventFlags  valid   = event->flags & FIXUP_KEY_FIELDS;
     DFBInputEventFlags  missing = valid ^ FIXUP_KEY_FIELDS;
     InputDeviceShared  *shared  = device->shared;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     /* Add missing flags */
     event->flags |= missing;

     /*
      * Use cached values for modifiers/locks if they are missing.
      */
     if (missing & DIEF_MODIFIERS)
          event->modifiers = shared->state.modifiers_l | shared->state.modifiers_r;

     if (missing & DIEF_LOCKS)
          event->locks = shared->state.locks;

     /*
      * With translation table
      */
     if (device->shared->keymap.num_entries) {
          if (valid & DIEF_KEYCODE) {
               lookup_from_table( device, event, missing );

               missing &= ~(DIEF_KEYID | DIEF_KEYSYMBOL);
          }
          else if (valid & DIEF_KEYID) {
               event->key_code = find_key_code_by_id( device, event->key_id );

               if (event->key_code != -1) {
                    lookup_from_table( device, event, missing );

                    missing &= ~(DIEF_KEYCODE | DIEF_KEYSYMBOL);
               }
               else if (missing & DIEF_KEYSYMBOL) {
                    event->key_symbol = id_to_symbol( event->key_id,
                                                      event->modifiers,
                                                      event->locks );
                    missing &= ~DIEF_KEYSYMBOL;
               }
          }
          else if (valid & DIEF_KEYSYMBOL) {
               event->key_code = find_key_code_by_symbol( device,
                                                          event->key_symbol );

               if (event->key_code != -1) {
                    lookup_from_table( device, event, missing );

                    missing &= ~(DIEF_KEYCODE | DIEF_KEYID);
               }
               else {
                    event->key_id = symbol_to_id( event->key_symbol );
                    missing &= ~DIEF_KEYSYMBOL;
               }
          }
     }
     else {
          /*
           * Without translation table
           */
          if (valid & DIEF_KEYID) {
               if (missing & DIEF_KEYSYMBOL) {
                    event->key_symbol = id_to_symbol( event->key_id,
                                                      event->modifiers,
                                                      event->locks );
                    missing &= ~DIEF_KEYSYMBOL;
               }
          }
          else if (valid & DIEF_KEYSYMBOL) {
               event->key_id = symbol_to_id( event->key_symbol );
               missing &= ~DIEF_KEYID;
          }
     }

     /*
      * Clear remaining fields.
      */
     if (missing & DIEF_KEYCODE)
          event->key_code = -1;

     if (missing & DIEF_KEYID)
          event->key_id = DIKI_UNKNOWN;

     if (missing & DIEF_KEYSYMBOL)
          event->key_symbol = DIKS_NULL;

     /*
      * Update cached values for modifiers.
      */
     if (DFB_KEY_TYPE(event->key_symbol) == DIKT_MODIFIER) {
          if (event->type == DIET_KEYPRESS) {
               switch (event->key_id) {
                    case DIKI_SHIFT_L:
                         shared->state.modifiers_l |= DIMM_SHIFT;
                         break;
                    case DIKI_SHIFT_R:
                         shared->state.modifiers_r |= DIMM_SHIFT;
                         break;
                    case DIKI_CONTROL_L:
                         shared->state.modifiers_l |= DIMM_CONTROL;
                         break;
                    case DIKI_CONTROL_R:
                         shared->state.modifiers_r |= DIMM_CONTROL;
                         break;
                    case DIKI_ALT_L:
                         shared->state.modifiers_l |= DIMM_ALT;
                         break;
                    case DIKI_ALT_R:
                         shared->state.modifiers_r |= (event->key_symbol == DIKS_ALTGR) ? DIMM_ALTGR : DIMM_ALT;
                         break;
                    case DIKI_META_L:
                         shared->state.modifiers_l |= DIMM_META;
                         break;
                    case DIKI_META_R:
                         shared->state.modifiers_r |= DIMM_META;
                         break;
                    case DIKI_SUPER_L:
                         shared->state.modifiers_l |= DIMM_SUPER;
                         break;
                    case DIKI_SUPER_R:
                         shared->state.modifiers_r |= DIMM_SUPER;
                         break;
                    case DIKI_HYPER_L:
                         shared->state.modifiers_l |= DIMM_HYPER;
                         break;
                    case DIKI_HYPER_R:
                         shared->state.modifiers_r |= DIMM_HYPER;
                         break;
                    default:
                         ;
               }
          }
          else {
               switch (event->key_id) {
                    case DIKI_SHIFT_L:
                         shared->state.modifiers_l &= ~DIMM_SHIFT;
                         break;
                    case DIKI_SHIFT_R:
                         shared->state.modifiers_r &= ~DIMM_SHIFT;
                         break;
                    case DIKI_CONTROL_L:
                         shared->state.modifiers_l &= ~DIMM_CONTROL;
                         break;
                    case DIKI_CONTROL_R:
                         shared->state.modifiers_r &= ~DIMM_CONTROL;
                         break;
                    case DIKI_ALT_L:
                         shared->state.modifiers_l &= ~DIMM_ALT;
                         break;
                    case DIKI_ALT_R:
                         shared->state.modifiers_r &= (event->key_symbol == DIKS_ALTGR) ? ~DIMM_ALTGR : ~DIMM_ALT;
                         break;
                    case DIKI_META_L:
                         shared->state.modifiers_l &= ~DIMM_META;
                         break;
                    case DIKI_META_R:
                         shared->state.modifiers_r &= ~DIMM_META;
                         break;
                    case DIKI_SUPER_L:
                         shared->state.modifiers_l &= ~DIMM_SUPER;
                         break;
                    case DIKI_SUPER_R:
                         shared->state.modifiers_r &= ~DIMM_SUPER;
                         break;
                    case DIKI_HYPER_L:
                         shared->state.modifiers_l &= ~DIMM_HYPER;
                         break;
                    case DIKI_HYPER_R:
                         shared->state.modifiers_r &= ~DIMM_HYPER;
                         break;
                    default:
                         ;
               }
          }

          /* write back to event */
          if (missing & DIEF_MODIFIERS)
               event->modifiers = shared->state.modifiers_l | shared->state.modifiers_r;
     }

     /*
      * Update cached values for locks.
      */
     if (event->type == DIET_KEYPRESS) {

          /* When we receive a new key press, toggle lock flags */
          if (shared->first_press || shared->last_key != event->key_id) {
              switch (event->key_id) {
                   case DIKI_CAPS_LOCK:
                        shared->state.locks ^= DILS_CAPS;
                        break;
                   case DIKI_NUM_LOCK:
                        shared->state.locks ^= DILS_NUM;
                        break;
                   case DIKI_SCROLL_LOCK:
                        shared->state.locks ^= DILS_SCROLL;
                        break;
                   default:
                        ;
              }
          }

          /* write back to event */
          if (missing & DIEF_LOCKS)
               event->locks = shared->state.locks;

          /* store last pressed key */
          shared->last_key = event->key_id;
          shared->first_press = false;
     }
     else if (event->type == DIET_KEYRELEASE) {
          
          shared->first_press = true;
     }

     /* Handle dead keys. */
     if (DFB_KEY_TYPE(shared->last_symbol) == DIKT_DEAD) {
          for (i=0; i<D_ARRAY_SIZE(deadkey_maps); i++) {
               const DeadKeyMap *map = &deadkey_maps[i];

               if (map->deadkey == shared->last_symbol) {
                    for (i=0; map->combos[i].target; i++) {
                         if (map->combos[i].target == event->key_symbol) {
                              event->key_symbol = map->combos[i].result;
                              break;
                         }
                    }
                    break;
               }
          }

          if (event->type == DIET_KEYRELEASE &&
              DFB_KEY_TYPE(event->key_symbol) != DIKT_MODIFIER)
               shared->last_symbol = event->key_symbol;
     }
     else
          shared->last_symbol = event->key_symbol;
}

static void
fixup_mouse_event( CoreInputDevice *device, DFBInputEvent *event )
{
     InputDeviceShared *shared = device->shared;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     if (event->flags & DIEF_BUTTONS) {
          shared->state.buttons = event->buttons;
     }
     else {
          switch (event->type) {
               case DIET_BUTTONPRESS:
                    shared->state.buttons |= (1 << event->button);
                    break;
               case DIET_BUTTONRELEASE:
                    shared->state.buttons &= ~(1 << event->button);
                    break;
               default:
                    ;
          }

          /* Add missing flag */
          event->flags |= DIEF_BUTTONS;

          event->buttons = shared->state.buttons;
     }

     switch (event->type) {
          case DIET_AXISMOTION:
               if ((event->flags & DIEF_AXISABS) && event->axis >= 0 && event->axis < shared->axis_num) {
                    if (!(event->flags & DIEF_MIN) && (shared->axis_info[event->axis].flags & DIAIF_ABS_MIN)) {
                         event->min = shared->axis_info[event->axis].abs_min;

                         event->flags |= DIEF_MIN;
                    }
                         
                    if (!(event->flags & DIEF_MAX) && (shared->axis_info[event->axis].flags & DIAIF_ABS_MAX)) {
                         event->max = shared->axis_info[event->axis].abs_max;

                         event->flags |= DIEF_MAX;
                    }
               }
               break;

          default:
               break;
     }
}

static DFBInputDeviceKeyIdentifier
symbol_to_id( DFBInputDeviceKeySymbol symbol )
{
     if (symbol >= 'a' && symbol <= 'z')
          return DIKI_A + symbol - 'a';

     if (symbol >= 'A' && symbol <= 'Z')
          return DIKI_A + symbol - 'A';

     if (symbol >= '0' && symbol <= '9')
          return DIKI_0 + symbol - '0';

     if (symbol >= DIKS_F1 && symbol <= DIKS_F12)
          return DIKI_F1 + symbol - DIKS_F1;

     switch (symbol) {
          case DIKS_ESCAPE:
               return DIKI_ESCAPE;

          case DIKS_CURSOR_LEFT:
               return DIKI_LEFT;

          case DIKS_CURSOR_RIGHT:
               return DIKI_RIGHT;

          case DIKS_CURSOR_UP:
               return DIKI_UP;

          case DIKS_CURSOR_DOWN:
               return DIKI_DOWN;

          case DIKS_ALTGR:
               return DIKI_ALT_R;

          case DIKS_CONTROL:
               return DIKI_CONTROL_L;

          case DIKS_SHIFT:
               return DIKI_SHIFT_L;

          case DIKS_ALT:
               return DIKI_ALT_L;

          case DIKS_META:
               return DIKI_META_L;

          case DIKS_SUPER:
               return DIKI_SUPER_L;

          case DIKS_HYPER:
               return DIKI_HYPER_L;

          case DIKS_TAB:
               return DIKI_TAB;

          case DIKS_ENTER:
               return DIKI_ENTER;

          case DIKS_SPACE:
               return DIKI_SPACE;

          case DIKS_BACKSPACE:
               return DIKI_BACKSPACE;

          case DIKS_INSERT:
               return DIKI_INSERT;

          case DIKS_DELETE:
               return DIKI_DELETE;

          case DIKS_HOME:
               return DIKI_HOME;

          case DIKS_END:
               return DIKI_END;

          case DIKS_PAGE_UP:
               return DIKI_PAGE_UP;

          case DIKS_PAGE_DOWN:
               return DIKI_PAGE_DOWN;

          case DIKS_CAPS_LOCK:
               return DIKI_CAPS_LOCK;

          case DIKS_NUM_LOCK:
               return DIKI_NUM_LOCK;

          case DIKS_SCROLL_LOCK:
               return DIKI_SCROLL_LOCK;

          case DIKS_PRINT:
               return DIKI_PRINT;

          case DIKS_PAUSE:
               return DIKI_PAUSE;

          case DIKS_BACKSLASH:
               return DIKI_BACKSLASH;

          case DIKS_PERIOD:
               return DIKI_PERIOD;

          case DIKS_COMMA:
               return DIKI_COMMA;

          default:
               ;
     }

     return DIKI_UNKNOWN;
}

static DFBInputDeviceKeySymbol
id_to_symbol( DFBInputDeviceKeyIdentifier id,
              DFBInputDeviceModifierMask  modifiers,
              DFBInputDeviceLockState     locks )
{
     bool shift = (modifiers & DIMM_SHIFT) || (locks & DILS_CAPS);

     if (id >= DIKI_A && id <= DIKI_Z)
          return (shift ? DIKS_CAPITAL_A : DIKS_SMALL_A) + id - DIKI_A;

     if (id >= DIKI_0 && id <= DIKI_9)
          return DIKS_0 + id - DIKI_0;

     if (id >= DIKI_KP_0 && id <= DIKI_KP_9)
          return DIKS_0 + id - DIKI_KP_0;

     if (id >= DIKI_F1 && id <= DIKI_F12)
          return DIKS_F1 + id - DIKI_F1;

     switch (id) {
          case DIKI_ESCAPE:
               return DIKS_ESCAPE;

          case DIKI_LEFT:
               return DIKS_CURSOR_LEFT;

          case DIKI_RIGHT:
               return DIKS_CURSOR_RIGHT;

          case DIKI_UP:
               return DIKS_CURSOR_UP;

          case DIKI_DOWN:
               return DIKS_CURSOR_DOWN;

          case DIKI_CONTROL_L:
          case DIKI_CONTROL_R:
               return DIKS_CONTROL;

          case DIKI_SHIFT_L:
          case DIKI_SHIFT_R:
               return DIKS_SHIFT;

          case DIKI_ALT_L:
          case DIKI_ALT_R:
               return DIKS_ALT;

          case DIKI_META_L:
          case DIKI_META_R:
               return DIKS_META;

          case DIKI_SUPER_L:
          case DIKI_SUPER_R:
               return DIKS_SUPER;

          case DIKI_HYPER_L:
          case DIKI_HYPER_R:
               return DIKS_HYPER;

          case DIKI_TAB:
               return DIKS_TAB;

          case DIKI_ENTER:
               return DIKS_ENTER;

          case DIKI_SPACE:
               return DIKS_SPACE;

          case DIKI_BACKSPACE:
               return DIKS_BACKSPACE;

          case DIKI_INSERT:
               return DIKS_INSERT;

          case DIKI_DELETE:
               return DIKS_DELETE;

          case DIKI_HOME:
               return DIKS_HOME;

          case DIKI_END:
               return DIKS_END;

          case DIKI_PAGE_UP:
               return DIKS_PAGE_UP;

          case DIKI_PAGE_DOWN:
               return DIKS_PAGE_DOWN;

          case DIKI_CAPS_LOCK:
               return DIKS_CAPS_LOCK;

          case DIKI_NUM_LOCK:
               return DIKS_NUM_LOCK;

          case DIKI_SCROLL_LOCK:
               return DIKS_SCROLL_LOCK;

          case DIKI_PRINT:
               return DIKS_PRINT;

          case DIKI_PAUSE:
               return DIKS_PAUSE;

          case DIKI_KP_DIV:
               return DIKS_SLASH;

          case DIKI_KP_MULT:
               return DIKS_ASTERISK;

          case DIKI_KP_MINUS:
               return DIKS_MINUS_SIGN;

          case DIKI_KP_PLUS:
               return DIKS_PLUS_SIGN;

          case DIKI_KP_ENTER:
               return DIKS_ENTER;

          case DIKI_KP_SPACE:
               return DIKS_SPACE;

          case DIKI_KP_TAB:
               return DIKS_TAB;

          case DIKI_KP_EQUAL:
               return DIKS_EQUALS_SIGN;

          case DIKI_KP_DECIMAL:
               return DIKS_PERIOD;

          case DIKI_KP_SEPARATOR:
               return DIKS_COMMA;

          case DIKI_BACKSLASH:
               return DIKS_BACKSLASH;

          case DIKI_EQUALS_SIGN:
               return DIKS_EQUALS_SIGN;

          case DIKI_LESS_SIGN:
               return DIKS_LESS_THAN_SIGN;

          case DIKI_MINUS_SIGN:
               return DIKS_MINUS_SIGN;

          case DIKI_PERIOD:
               return DIKS_PERIOD;

          case DIKI_QUOTE_LEFT:
          case DIKI_QUOTE_RIGHT:
               return DIKS_QUOTATION;

          case DIKI_SEMICOLON:
               return DIKS_SEMICOLON;

          case DIKI_SLASH:
               return DIKS_SLASH;

          default:
               ;
     }

     return DIKS_NULL;
}

static void
release_key( CoreInputDevice *device, DFBInputDeviceKeyIdentifier id )
{
     DFBInputEvent evt;

     D_MAGIC_ASSERT( device, CoreInputDevice );

     evt.type = DIET_KEYRELEASE;

     if (DFB_KEY_TYPE(id) == DIKT_IDENTIFIER) {
          evt.flags  = DIEF_KEYID;
          evt.key_id = id;
     }
     else {
          evt.flags      = DIEF_KEYSYMBOL;
          evt.key_symbol = id;
     }

     dfb_input_dispatch( device, &evt );
}

static void
flush_keys( CoreInputDevice *device )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     if (device->shared->state.modifiers_l) {
          if (device->shared->state.modifiers_l & DIMM_ALT)
               release_key( device, DIKI_ALT_L );

          if (device->shared->state.modifiers_l & DIMM_CONTROL)
               release_key( device, DIKI_CONTROL_L );

          if (device->shared->state.modifiers_l & DIMM_HYPER)
               release_key( device, DIKI_HYPER_L );

          if (device->shared->state.modifiers_l & DIMM_META)
               release_key( device, DIKI_META_L );

          if (device->shared->state.modifiers_l & DIMM_SHIFT)
               release_key( device, DIKI_SHIFT_L );

          if (device->shared->state.modifiers_l & DIMM_SUPER)
               release_key( device, DIKI_SUPER_L );
     }

     if (device->shared->state.modifiers_r) {
          if (device->shared->state.modifiers_r & DIMM_ALTGR)
               release_key( device, DIKS_ALTGR );

          if (device->shared->state.modifiers_r & DIMM_ALT)
               release_key( device, DIKI_ALT_R );

          if (device->shared->state.modifiers_r & DIMM_CONTROL)
               release_key( device, DIKI_CONTROL_R );

          if (device->shared->state.modifiers_r & DIMM_HYPER)
               release_key( device, DIKI_HYPER_R );

          if (device->shared->state.modifiers_r & DIMM_META)
               release_key( device, DIKI_META_R );

          if (device->shared->state.modifiers_r & DIMM_SHIFT)
               release_key( device, DIKI_SHIFT_R );

          if (device->shared->state.modifiers_r & DIMM_SUPER)
               release_key( device, DIKI_SUPER_R );
     }
}

static void
dump_primary_layer_surface( CoreDFB *core )
{
     CoreLayer        *layer = dfb_layer_at( DLID_PRIMARY );
     CoreLayerContext *context;

     /* Get the currently active context. */
     if (dfb_layer_get_active_context( layer, &context ) == DFB_OK) {
          CoreLayerRegion *region;

          /* Get the first region. */
          if (dfb_layer_context_get_primary_region( context,
                                                    false, &region ) == DFB_OK)
          {
               CoreSurface *surface;

               /* Lock the region to avoid tearing due to concurrent updates. */
               dfb_layer_region_lock( region );

               /* Get the surface of the region. */
               if (dfb_layer_region_get_surface( region, &surface ) == DFB_OK) {
                    /* Dump the surface contents. */
                    dfb_surface_dump_buffer( surface, CSBR_FRONT, dfb_config->screenshot_dir, "dfb" );

                    /* Release the surface. */
                    dfb_surface_unref( surface );
               }

               /* Unlock the region. */
               dfb_layer_region_unlock( region );

               /* Release the region. */
               dfb_layer_region_unref( region );
          }

          /* Release the context. */
          dfb_layer_context_unref( context );
     }
}

static bool
core_input_filter( CoreInputDevice *device, DFBInputEvent *event )
{
     D_MAGIC_ASSERT( device, CoreInputDevice );

     if (dfb_system_input_filter( device, event ))
          return true;

     if (event->type == DIET_KEYPRESS) {
          switch (event->key_symbol) {
               case DIKS_PRINT:
                    if (!event->modifiers && dfb_config->screenshot_dir) {
                         dump_primary_layer_surface( device->core );
                         return true;
                    }
                    break;

               case DIKS_BACKSPACE:
                    if (event->modifiers == DIMM_META)
                         direct_trace_print_stacks();

                    break;

               case DIKS_ESCAPE:
                    if (event->modifiers == DIMM_META) {
#if FUSION_BUILD_MULTI
                         DFBResult         ret;
                         CoreLayer        *layer = dfb_layer_at( DLID_PRIMARY );
                         CoreLayerContext *context;

                         /* Get primary (shared) context. */
                         ret = dfb_layer_get_primary_context( layer,
                                                              false, &context );
                         if (ret)
                              return false;

                         /* Activate the context. */
                         dfb_layer_activate_context( layer, context );

                         /* Release the context. */
                         dfb_layer_context_unref( context );

#else
                         kill( 0, SIGINT );
#endif

                         return true;
                    }
                    break;

               default:
                    break;
          }
     }

     return false;
}

