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
#include <string.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/list.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/modules.h>

#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/core_parts.h>
#include <core/layer_context.h>
#include <core/layers_internal.h>
#include <core/windows_internal.h>
#include <core/wm.h>

#include <misc/conf.h>
#include <misc/util.h>


DEFINE_MODULE_DIRECTORY( dfb_core_wm_modules, "wm", DFB_CORE_WM_ABI_VERSION );


D_DEBUG_DOMAIN( Core_WM, "Core/WM", "DirectFB WM Core" );

/**********************************************************************************************************************/

typedef struct {
     int                  magic;

     DirectLink          *stacks;

     int                  abi;

     char                *name;
     CoreWMInfo           info;
     void                *data;

     FusionSHMPoolShared *shmpool;
} DFBWMCoreShared;

struct __DFB_DFBWMCore {
     int                magic;

     CoreDFB           *core;

     DFBWMCoreShared   *shared;


     DirectModuleEntry *module;
     const CoreWMFuncs *funcs;
     void              *data;
};


DFB_CORE_PART( wm_core, WMCore );

/**********************************************************************************************************************/

static DFBResult load_module( const char *name );

/**********************************************************************************************************************/

static DFBWMCore       *wm_local  = NULL;  /* FIXME */
static DFBWMCoreShared *wm_shared = NULL;  /* FIXME */


static DFBResult
dfb_wm_core_initialize( CoreDFB         *core,
                        DFBWMCore       *data,
                        DFBWMCoreShared *shared )
{
     DFBResult ret;

     D_DEBUG_AT( Core_WM, "dfb_wm_core_initialize( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_ASSERT( shared != NULL );

     data->core   = core;
     data->shared = shared;


     wm_local  = data;   /* FIXME */
     wm_shared = shared; /* FIXME */

     wm_shared->shmpool = dfb_core_shmpool( core );

     /* Set ABI version for the session. */
     wm_shared->abi = DFB_CORE_WM_ABI_VERSION;

     /* Load the module. */
     ret = load_module( dfb_config->wm );
     if (ret)
          goto error;

     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->GetWMInfo != NULL );
     D_ASSERT( wm_local->funcs->Initialize != NULL );

     /* Query module information. */
     wm_local->funcs->GetWMInfo( &wm_shared->info );

     D_INFO( "DirectFB/Core/WM: %s %d.%d (%s)\n",
             wm_shared->info.name, wm_shared->info.version.major,
             wm_shared->info.version.minor, wm_shared->info.vendor );

     ret = DFB_NOSHAREDMEMORY;

     /* Store module name in shared memory. */
     wm_shared->name = SHSTRDUP( wm_shared->shmpool, wm_local->module->name );
     if (!wm_shared->name) {
          D_OOSHM();
          goto error;
     }

     /* Allocate shared window manager data. */
     if (wm_shared->info.wm_shared_size) {
          wm_shared->data = SHCALLOC( wm_shared->shmpool, 1, wm_shared->info.wm_shared_size );
          if (!wm_shared->data) {
               D_OOSHM();
               goto error;
          }
     }

     ret = DFB_NOSYSTEMMEMORY;

     /* Allocate local window manager data. */
     if (wm_shared->info.wm_data_size) {
          wm_local->data = D_CALLOC( 1, wm_shared->info.wm_data_size );
          if (!wm_local->data) {
               D_OOM();
               goto error;
          }
     }

     /* Initialize window manager. */
     ret = wm_local->funcs->Initialize( core, wm_local->data, wm_shared->data );
     if (ret) {
          D_DERROR( ret, "DirectFB/Core/WM: Could not initialize window manager!\n" );
          goto error;
     }

     D_MAGIC_SET( data, DFBWMCore );
     D_MAGIC_SET( shared, DFBWMCoreShared );

     return DFB_OK;

error:
     if (wm_local->data)
          D_FREE( wm_local->data );

     if (wm_shared->data)
          SHFREE( wm_shared->shmpool, wm_shared->data );

     if (wm_shared->name)
          SHFREE( wm_shared->shmpool, wm_shared->name );

     wm_local = NULL;
     wm_shared = NULL;

     return ret;
}

static DFBResult
dfb_wm_core_join( CoreDFB         *core,
                  DFBWMCore       *data,
                  DFBWMCoreShared *shared )
{
     DFBResult  ret;
     CoreWMInfo info;

     D_DEBUG_AT( Core_WM, "dfb_wm_core_join( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_MAGIC_ASSERT( shared, DFBWMCoreShared );

     data->core   = core;
     data->shared = shared;


     wm_local  = data;   /* FIXME */
     wm_shared = shared; /* FIXME */

     /* Check binary version numbers. */
     if (wm_shared->abi != DFB_CORE_WM_ABI_VERSION) {
          D_ERROR( "DirectFB/Core/WM: ABI version of running core instance (%d) doesn't match %d!\n",
                   wm_shared->abi, DFB_CORE_WM_ABI_VERSION );
          ret = DFB_VERSIONMISMATCH;
          goto error;
     }

     /* Load the module that is used by the running session. */
     ret = load_module( wm_shared->name );
     if (ret)
          goto error;

     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->GetWMInfo != NULL );
     D_ASSERT( wm_local->funcs->Join != NULL );

     /* Query module information. */
     wm_local->funcs->GetWMInfo( &info );

     /* Check binary version numbers. */
     if (info.version.binary != wm_shared->info.version.binary) {
          D_ERROR( "DirectFB/Core/WM: ABI version of running module instance (%d) doesn't match %d!\n",
                   wm_shared->info.version.binary, info.version.binary );
          ret = DFB_VERSIONMISMATCH;
          goto error;
     }

     /* Allocate window manager data. */
     if (wm_shared->info.wm_data_size) {
          wm_local->data = D_CALLOC( 1, wm_shared->info.wm_data_size );
          if (!wm_local->data) {
               D_WARN( "out of memory" );
               ret = DFB_NOSYSTEMMEMORY;
               goto error;
          }
     }

     /* Join window manager. */
     ret = wm_local->funcs->Join( core, wm_local->data, wm_shared->data );
     if (ret) {
          D_DERROR( ret, "DirectFB/Core/WM: Could not join window manager!\n" );
          goto error;
     }

     D_MAGIC_SET( data, DFBWMCore );

     return DFB_OK;

error:
     if (wm_local->data)
          D_FREE( wm_local->data );

     wm_local = NULL;
     wm_shared = NULL;

     return ret;
}

static DFBResult
dfb_wm_core_shutdown( DFBWMCore *data,
                      bool       emergency )
{
     DFBResult        ret;
     DFBWMCoreShared *shared;

     D_DEBUG_AT( Core_WM, "dfb_wm_core_shutdown( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBWMCore );
     D_MAGIC_ASSERT( data->shared, DFBWMCoreShared );

     shared = data->shared;


     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->Shutdown != NULL );
     D_ASSERT( wm_shared != NULL );

     /* Shutdown window manager. */
     ret = wm_local->funcs->Shutdown( emergency, wm_local->data, wm_shared->data );

     /* Unload the module. */
     direct_module_unref( wm_local->module );

     /* Deallocate local window manager data. */
     if (wm_local->data)
          D_FREE( wm_local->data );

     /* Deallocate shared window manager data. */
     if (wm_shared->data)
          SHFREE( wm_shared->shmpool, wm_shared->data );

     /* Free module name in shared memory. */
     SHFREE( wm_shared->shmpool, wm_shared->name );

     wm_local = NULL;
     wm_shared = NULL;


     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( shared );

     return ret;
}

static DFBResult
dfb_wm_core_leave( DFBWMCore *data,
                    bool        emergency )
{
     DFBResult         ret;
     DFBWMCoreShared *shared;

     D_DEBUG_AT( Core_WM, "dfb_wm_core_leave( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBWMCore );
     D_MAGIC_ASSERT( data->shared, DFBWMCoreShared );

     shared = data->shared;


     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->Leave != NULL );
     D_ASSERT( wm_shared != NULL );

     /* Leave window manager. */
     ret = wm_local->funcs->Leave( emergency, wm_local->data, wm_shared->data );

     /* Unload the module. */
     direct_module_unref( wm_local->module );

     /* Deallocate local window manager data. */
     if (wm_local->data)
          D_FREE( wm_local->data );

     wm_local = NULL;
     wm_shared = NULL;


     D_MAGIC_CLEAR( data );

     return ret;
}

static DFBResult
dfb_wm_core_suspend( DFBWMCore *data )
{
     DFBWMCoreShared *shared;

     D_DEBUG_AT( Core_WM, "%s( %p )\n", __FUNCTION__, data );

     D_MAGIC_ASSERT( data, DFBWMCore );
     D_MAGIC_ASSERT( data->shared, DFBWMCoreShared );

     shared = data->shared;

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->Suspend != NULL );
     D_ASSERT( wm_shared != NULL );

     return wm_local->funcs->Suspend( wm_local->data, wm_shared->data );
}

static DFBResult
dfb_wm_core_resume( DFBWMCore *data )
{
     DFBWMCoreShared *shared;

     D_DEBUG_AT( Core_WM, "%s( %p )\n", __FUNCTION__, data );

     D_MAGIC_ASSERT( data, DFBWMCore );
     D_MAGIC_ASSERT( data->shared, DFBWMCoreShared );

     shared = data->shared;

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->Resume != NULL );
     D_ASSERT( wm_shared != NULL );

     return wm_local->funcs->Resume( wm_local->data, wm_shared->data );
}

DFBResult
dfb_wm_close_all_stacks( void *data )
{
     CoreLayerContext *context;
     CoreWindowStack  *stack, *next;
     DFBWMCore        *local;
     DFBWMCoreShared  *shared;

     D_DEBUG_AT( Core_WM, "%s( %p )\n", __FUNCTION__, data );

     local = data;

     D_MAGIC_ASSERT( local, DFBWMCore );
     D_ASSERT( local->funcs != NULL );
     D_ASSERT( local->funcs->CloseStack != NULL );

     shared = local->shared;

     D_MAGIC_ASSERT( shared, DFBWMCoreShared );

     D_DEBUG_AT( Core_WM, "  -> checking %d stacks...\n", direct_list_count_elements_EXPENSIVE(shared->stacks) );

     direct_list_foreach_safe (stack, next, shared->stacks) {
          D_DEBUG_AT( Core_WM, "  -> checking %p...\n", stack );

          D_MAGIC_ASSERT( stack, CoreWindowStack );

          context = stack->context;
          D_MAGIC_ASSERT( context, CoreLayerContext );

          D_DEBUG_AT( Core_WM, "  -> ref context %p...\n", context );

          dfb_layer_context_ref( context );

          dfb_layer_context_lock( context );

          if (stack->flags & CWSF_INITIALIZED) {
               D_DEBUG_AT( Core_WM, "  => CLOSING %p\n", stack );
               dfb_wm_close_stack( stack );
          }

          dfb_layer_context_unlock( context );

          D_DEBUG_AT( Core_WM, "  -> unref context %p...\n", context );

          dfb_layer_context_unref( context );
     }

     return DFB_OK;
}

/**************************************************************************************************/

static DFBResult
load_module( const char *name )
{
     DirectLink *l;

     D_ASSERT( wm_local != NULL );

     direct_modules_explore_directory( &dfb_core_wm_modules );

     direct_list_foreach( l, dfb_core_wm_modules.entries ) {
          DirectModuleEntry *module = (DirectModuleEntry*) l;
          const CoreWMFuncs *funcs;

          funcs = direct_module_ref( module );
          if (!funcs)
               continue;

          if (!name || !strcasecmp( name, module->name )) {
               if (wm_local->module)
                    direct_module_unref( wm_local->module );

               wm_local->module = module;
               wm_local->funcs  = funcs;
          }
          else
               direct_module_unref( module );
     }

     if (!wm_local->module) {
          if (name)
               D_ERROR( "DirectFB/WM: Window manager module '%s' not found!\n", name );
          else
               D_ERROR( "DirectFB/WM: No window manager module found!\n" );

          return DFB_NOIMPL;
     }

     return DFB_OK;
}

/**************************************************************************************************/

void
dfb_wm_get_info( CoreWMInfo *info )
{
     D_ASSERT( wm_shared != NULL );

     D_ASSERT( info != NULL );

     *info = wm_shared->info;
}

DFBResult
dfb_wm_post_init( CoreDFB *core )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->Resume != NULL );
     D_ASSERT( wm_shared != NULL );

     return wm_local->funcs->PostInit( wm_local->data, wm_shared->data );
}

/**************************************************************************************************/

DFBResult
dfb_wm_init_stack( CoreWindowStack *stack )
{
     DFBResult ret;

     D_DEBUG_AT( Core_WM, "%s( %p )\n", __FUNCTION__, stack );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->InitStack != NULL );
     D_ASSERT( wm_shared != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( !(stack->flags & CWSF_INITIALIZED) );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     /* Allocate shared stack data. */
     if (wm_shared->info.stack_data_size) {
          if (stack->stack_data)
               SHFREE( stack->shmpool, stack->stack_data );
               
          stack->stack_data = SHCALLOC( stack->shmpool, 1, wm_shared->info.stack_data_size );
          if (!stack->stack_data) {
               D_WARN( "out of (shared) memory" );
               return D_OOSHM();
          }
     }

     /* Window manager specific initialization. */
     ret = wm_local->funcs->InitStack( stack, wm_local->data, stack->stack_data );
     if (ret) {
          if (stack->stack_data) {
               SHFREE( wm_shared->shmpool, stack->stack_data );
               stack->stack_data = NULL;
          }

          return ret;
     }

     stack->flags |= CWSF_INITIALIZED;

     /* Add window stack to list. */
     direct_list_append( &wm_shared->stacks, &stack->link );

     return DFB_OK;
}

DFBResult
dfb_wm_close_stack( CoreWindowStack *stack )
{
     D_DEBUG_AT( Core_WM, "%s( %p )\n", __FUNCTION__, stack );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->CloseStack != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );

     D_ASSUME( stack->flags & CWSF_INITIALIZED );

     if (!(stack->flags & CWSF_INITIALIZED)) {
          D_ASSUME( !(stack->flags & CWSF_ACTIVATED) );
          return DFB_OK;
     }

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     /* Deactivate before deinitialization. */
     if (stack->flags & CWSF_ACTIVATED)
          dfb_wm_set_active( stack, false );

     /*
      * Clear flag and remove stack first, because
      * CloseStack() may cause the stack to be destroyed!
      */
     stack->flags &= ~CWSF_INITIALIZED;

     /* Remove window stack from list. */
     direct_list_remove( &wm_shared->stacks, &stack->link );

     /* Window manager specific deinitialization. */
     return wm_local->funcs->CloseStack( stack, wm_local->data, stack->stack_data );
}

DFBResult
dfb_wm_set_active( CoreWindowStack *stack,
                   bool             active )
{
     DFBResult ret;

     D_DEBUG_AT( Core_WM, "%s( %p, %sactive )\n", __FUNCTION__, stack, active ? "" : "in" );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->SetActive != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     if (active) {
          D_ASSUME( !(stack->flags & CWSF_ACTIVATED) );

          if (stack->flags & CWSF_ACTIVATED)
               return DFB_OK;

          ret = wm_local->funcs->SetActive( stack, wm_local->data, stack->stack_data, true );

          stack->flags |= CWSF_ACTIVATED;
     }
     else {
          D_ASSUME( stack->flags & CWSF_ACTIVATED );

          if (!(stack->flags & CWSF_ACTIVATED))
               return DFB_OK;

          ret = wm_local->funcs->SetActive( stack, wm_local->data, stack->stack_data, false );

          stack->flags &= ~CWSF_ACTIVATED;
     }

     return ret;
}

DFBResult
dfb_wm_resize_stack( CoreWindowStack *stack,
                     int              width,
                     int              height )
{
     D_DEBUG_AT( Core_WM, "%s( %p, %dx%d )\n", __FUNCTION__, stack, width, height );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->ResizeStack != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     /* Notify window manager about the new size. */
     return wm_local->funcs->ResizeStack( stack, wm_local->data, stack->stack_data, width, height );
}

DFBResult
dfb_wm_process_input( CoreWindowStack     *stack,
                      const DFBInputEvent *event )
{
     D_DEBUG_AT( Core_WM, "%s( %p, %p )\n", __FUNCTION__, stack, event );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->ProcessInput != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( event != NULL );

     /* Dispatch input event via window manager. */
     return wm_local->funcs->ProcessInput( stack, wm_local->data, stack->stack_data, event );
}

DFBResult
dfb_wm_flush_keys( CoreWindowStack *stack )
{
     D_DEBUG_AT( Core_WM, "%s( %p )\n", __FUNCTION__, stack );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->FlushKeys != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     return wm_local->funcs->FlushKeys( stack, wm_local->data, stack->stack_data );
}

DFBResult
dfb_wm_window_at( CoreWindowStack  *stack,
                  int               x,
                  int               y,
                  CoreWindow      **ret_window )
{
     D_DEBUG_AT( Core_WM, "%s( %p, %d,%d )\n", __FUNCTION__, stack, x, y );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->WindowAt != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( ret_window != NULL );

     return wm_local->funcs->WindowAt( stack, wm_local->data, stack->stack_data, x, y, ret_window );
}

DFBResult
dfb_wm_window_lookup( CoreWindowStack  *stack,
                      DFBWindowID       window_id,
                      CoreWindow      **ret_window )
{
     D_DEBUG_AT( Core_WM, "%s( %p, %u )\n", __FUNCTION__, stack, window_id );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->WindowLookup != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( ret_window != NULL );

     return wm_local->funcs->WindowLookup( stack, wm_local->data,
                                           stack->stack_data, window_id, ret_window );
}

DFBResult
dfb_wm_enum_windows( CoreWindowStack      *stack,
                     CoreWMWindowCallback  callback,
                     void                 *callback_ctx )
{
     D_DEBUG_AT( Core_WM, "%s( %p, %p, %p )\n", __FUNCTION__, stack, callback, callback_ctx );

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->EnumWindows != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( callback != NULL );

     return wm_local->funcs->EnumWindows( stack, wm_local->data,
                                          stack->stack_data, callback, callback_ctx );
}

/**
 * Give the wm a chance to specifiy a border
 */
DFBResult
dfb_wm_get_insets( CoreWindowStack *stack,
                   CoreWindow      *window,
                   DFBInsets       *insets)
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->GetInsets != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( window != NULL );
     D_ASSERT( insets != NULL );

     return wm_local->funcs->GetInsets( stack, window, insets );
}

/**
 * Give the wm a chance to override the windows configuration 
 */
DFBResult
dfb_wm_preconfigure_window( CoreWindowStack *stack,
                            CoreWindow      *window )
{
     DFBResult  ret;
     void      *window_data = NULL;

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_shared != NULL );
     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );
     D_ASSERT( window != NULL );
     D_ASSERT( wm_local->funcs->PreConfigureWindow != NULL );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_DEBUG_AT( Core_WM, "%s( %p, %p [%d,%d-%dx%d] )\n", __FUNCTION__,
                 stack, window, DFB_RECTANGLE_VALS(&window->config.bounds) );

     /* Allocate shared window data. */
     if (wm_shared->info.window_data_size) {
          window_data = SHCALLOC( wm_shared->shmpool, 1, wm_shared->info.window_data_size );
          if (!window_data) {
               D_WARN( "out of (shared) memory" );
               return D_OOSHM();
          }
     }

     /* Keep shared window data. */
     window->window_data = window_data;

     /* Tell window manager about the new window. */
     ret = wm_local->funcs->PreConfigureWindow( stack, wm_local->data,
                                       stack->stack_data, window, window_data );
     if (ret) {
          if (window_data) {
               SHFREE( wm_shared->shmpool, window_data );
               window->window_data = NULL;
          }

          return ret;
     }

     return DFB_OK;
}

DFBResult
dfb_wm_add_window( CoreWindowStack *stack,
                   CoreWindow      *window )
{
     DFBResult  ret;

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->AddWindow != NULL );
     D_ASSERT( wm_shared != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( window != NULL );

     D_DEBUG_AT( Core_WM, "%s( %p, %p [%d,%d-%dx%d] )\n", __FUNCTION__,
                 stack, window, DFB_RECTANGLE_VALS(&window->config.bounds) );

     /* Tell window manager about the new window. */
     ret = wm_local->funcs->AddWindow( stack, wm_local->data,
                                       stack->stack_data, window, window->window_data );
     if (ret) {
          if (window->window_data)
               SHFREE( wm_shared->shmpool, window->window_data );
          return ret;
     }
     return DFB_OK;
}

DFBResult
dfb_wm_remove_window( CoreWindowStack *stack,
                      CoreWindow      *window )
{
     DFBResult ret;

     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->RemoveWindow != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( window != NULL );

     D_DEBUG_AT( Core_WM, "%s( %p, %p [%d,%d-%dx%d] )\n", __FUNCTION__,
                 stack, window, DFB_RECTANGLE_VALS(&window->config.bounds) );

     /* Remove window from window manager. */
     ret = wm_local->funcs->RemoveWindow( stack, wm_local->data,
                                          stack->stack_data, window, window->window_data );

     /* Deallocate shared stack data. */
     if (window->window_data)
          SHFREE( wm_shared->shmpool, window->window_data );

     return ret;
}

/**
 * Let the wm set a property on a window 
 */
DFBResult
dfb_wm_set_window_property( CoreWindowStack  *stack,
                            CoreWindow       *window,
                            const char       *key,
                            void             *value,
                            void            **ret_old_value )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->SetWindowProperty != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( window != NULL );
     D_ASSERT( key != NULL );

     D_DEBUG_AT( Core_WM, "%s( %p, %p [%d,%d-%dx%d], '%s' = %p )\n", __FUNCTION__,
                 stack, window, DFB_RECTANGLE_VALS(&window->config.bounds), key, value );

     return wm_local->funcs->SetWindowProperty( stack, wm_local->data, stack->stack_data,
                                                window, window->window_data,
                                                key, value, ret_old_value );
}

/**
 * get the wm  property on a window 
 */
DFBResult
dfb_wm_get_window_property( CoreWindowStack  *stack,
                            CoreWindow       *window,
                            const char       *key,
                            void            **ret_value )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->GetWindowProperty != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( window != NULL );
     D_ASSERT( key != NULL );

     D_DEBUG_AT( Core_WM, "%s( %p, %p [%d,%d-%dx%d], '%s' )\n", __FUNCTION__,
                 stack, window, DFB_RECTANGLE_VALS(&window->config.bounds), key );

     return wm_local->funcs->GetWindowProperty( stack, wm_local->data, stack->stack_data,
                                                window, window->window_data, key, ret_value );
}

/**
 * remove th wm  property on a window 
 */
DFBResult
dfb_wm_remove_window_property( CoreWindowStack  *stack,
                               CoreWindow       *window,
                               const char       *key,
                               void            **ret_value )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->RemoveWindowProperty != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_ASSERT( window != NULL );
     D_ASSERT( key != NULL );

     D_DEBUG_AT( Core_WM, "%s( %p, %p [%d,%d-%dx%d], '%s' )\n", __FUNCTION__,
                 stack, window, DFB_RECTANGLE_VALS(&window->config.bounds), key );

     return wm_local->funcs->RemoveWindowProperty( stack, wm_local->data, stack->stack_data,
                                                   window, window->window_data, key, ret_value );
}

DFBResult
dfb_wm_set_window_config( CoreWindow             *window,
                          const CoreWindowConfig *config,
                          CoreWindowConfigFlags   flags )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->SetWindowConfig != NULL );

     D_ASSERT( window != NULL );
     D_ASSERT( config != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d-%dx%d], %p, 0x%x )\n", __FUNCTION__,
                 window, DFB_RECTANGLE_VALS(&window->config.bounds), config, flags );

     return wm_local->funcs->SetWindowConfig( window, wm_local->data,
                                              window->window_data, config, flags );
}

DFBResult
dfb_wm_restack_window( CoreWindow *window,
                       CoreWindow *relative,
                       int         relation )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->RestackWindow != NULL );

     D_ASSERT( window != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     D_ASSERT( relative == NULL || relative == window || relation != 0);

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d-%dx%d], %p, %d )\n", __FUNCTION__,
                 window, DFB_RECTANGLE_VALS(&window->config.bounds), relative, relation );

     return wm_local->funcs->RestackWindow( window, wm_local->data, window->window_data, relative,
                                            relative ? relative->window_data : NULL, relation );
}

DFBResult
dfb_wm_grab( CoreWindow *window,
             CoreWMGrab *grab )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->Grab != NULL );

     D_ASSERT( window != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     D_ASSERT( grab != NULL );

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d-%dx%d], %d )\n", __FUNCTION__,
                 window, DFB_RECTANGLE_VALS(&window->config.bounds), grab->target );

     return wm_local->funcs->Grab( window, wm_local->data, window->window_data, grab );
}

DFBResult
dfb_wm_ungrab( CoreWindow *window,
               CoreWMGrab *grab )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->Ungrab != NULL );

     D_ASSERT( window != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     D_ASSERT( grab != NULL );

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d-%dx%d], %d )\n", __FUNCTION__,
                 window, DFB_RECTANGLE_VALS(&window->config.bounds), grab->target );

     return wm_local->funcs->Ungrab( window, wm_local->data, window->window_data, grab );
}

DFBResult
dfb_wm_request_focus( CoreWindow *window )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->RequestFocus != NULL );

     D_ASSERT( window != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d-%dx%d] )\n", __FUNCTION__,
                 window, DFB_RECTANGLE_VALS(&window->config.bounds) );

     return wm_local->funcs->RequestFocus( window, wm_local->data, window->window_data );
}

DFBResult
dfb_wm_begin_updates( CoreWindow      *window,
                      const DFBRegion *update )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->RequestFocus != NULL );

     D_ASSERT( window != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d-%dx%d] )\n", __FUNCTION__,
                 window, DFB_RECTANGLE_VALS(&window->config.bounds) );

     return wm_local->funcs->BeginUpdates( window, wm_local->data, window->window_data, update );
}

DFBResult
dfb_wm_set_cursor_position( CoreWindow *window,
                            int         x,
                            int         y )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->SetCursorPosition != NULL );

     D_ASSERT( window != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d] )\n", __FUNCTION__, window, x, y );

     return wm_local->funcs->SetCursorPosition( window, wm_local->data, window->window_data, x, y );
}

DFBResult
dfb_wm_update_stack( CoreWindowStack     *stack,
                     const DFBRegion     *region,
                     DFBSurfaceFlipFlags  flags )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->UpdateStack != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     DFB_REGION_ASSERT( region );

     D_DEBUG_AT( Core_WM, "%s( %p, [%d,%d-%dx%d], 0x%x )\n", __FUNCTION__,
                 stack, DFB_RECTANGLE_VALS_FROM_REGION(region), flags );

     return wm_local->funcs->UpdateStack( stack, wm_local->data,
                                          stack->stack_data, region, flags );
}

DFBResult
dfb_wm_update_window( CoreWindow          *window,
                      const DFBRegion     *region,
                      DFBSurfaceFlipFlags  flags )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->UpdateWindow != NULL );

     D_ASSERT( window != NULL );

     D_MAGIC_ASSERT( window->stack, CoreWindowStack );
     D_MAGIC_ASSERT( window->stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &window->stack->context->lock );

     DFB_REGION_ASSERT_IF( region );

     D_DEBUG_AT( Core_WM, "%s( %p [%d,%d-%dx%d], [%d,%d-%dx%d], 0x%x )\n", __FUNCTION__,
                 window, DFB_RECTANGLE_VALS(&window->config.bounds),
                 DFB_RECTANGLE_VALS_FROM_REGION(region), flags );

     return wm_local->funcs->UpdateWindow( window, wm_local->data,
                                           window->window_data, region, flags );
}

DFBResult
dfb_wm_update_cursor( CoreWindowStack       *stack,
                      CoreCursorUpdateFlags  flags )
{
     D_ASSERT( wm_local != NULL );
     D_ASSERT( wm_local->funcs != NULL );
     D_ASSERT( wm_local->funcs->UpdateStack != NULL );

     D_MAGIC_ASSERT( stack, CoreWindowStack );
     D_ASSERT( stack->flags & CWSF_INITIALIZED );

     D_MAGIC_ASSERT( stack->context, CoreLayerContext );
     FUSION_SKIRMISH_ASSERT( &stack->context->lock );

     D_FLAGS_ASSERT( flags, CCUF_ALL );

     if (dfb_config->no_cursor || dfb_config->no_cursor_updates)
          return DFB_OK;

     return wm_local->funcs->UpdateCursor( stack, wm_local->data,
                                           stack->stack_data, flags );
}

