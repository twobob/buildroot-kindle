/*
   (c) Copyright 2001-2011  The world wide DirectFB Open Source Community (directfb.org)
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

#include "CoreDFB.h"

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <fusion/conf.h>

#include <core/core.h>

#include <core/CoreDFB_CallMode.h>

D_DEBUG_DOMAIN( DirectFB_CoreDFB, "DirectFB/CoreDFB", "DirectFB CoreDFB" );

/*********************************************************************************************************************/

DFBResult
CoreDFB_Register(
                    CoreDFB                                   *obj,
                    u32                                        slave_call
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ICore_Real__Register( obj, slave_call );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ICore_Requestor__Register( obj, slave_call );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreDFB_CreateSurface(
                    CoreDFB                                   *obj,
                    const CoreSurfaceConfig                   *config,
                    CoreSurfaceTypeFlags                       type,
                    u64                                        resource_id,
                    CorePalette                               *palette,
                    CoreSurface                              **ret_surface
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ICore_Real__CreateSurface( obj, config, type, resource_id, palette, ret_surface );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ICore_Requestor__CreateSurface( obj, config, type, resource_id, palette, ret_surface );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreDFB_CreatePalette(
                    CoreDFB                                   *obj,
                    u32                                        size,
                    CorePalette                              **ret_palette
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ICore_Real__CreatePalette( obj, size, ret_palette );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ICore_Requestor__CreatePalette( obj, size, ret_palette );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreDFB_CreateState(
                    CoreDFB                                   *obj,
                    CoreGraphicsState                        **ret_state
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ICore_Real__CreateState( obj, ret_state );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ICore_Requestor__CreateState( obj, ret_state );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreDFB_WaitIdle(
                    CoreDFB                                   *obj

)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ICore_Real__WaitIdle( obj );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ICore_Requestor__WaitIdle( obj );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreDFB_CreateImageProvider(
                    CoreDFB                                   *obj,
                    u32                                        buffer_call,
                    u32                                       *ret_call
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ICore_Real__CreateImageProvider( obj, buffer_call, ret_call );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ICore_Requestor__CreateImageProvider( obj, buffer_call, ret_call );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

/*********************************************************************************************************************/

static FusionCallHandlerResult
CoreDFB_Dispatch( int           caller,   /* fusion id of the caller */
                     int           call_arg, /* optional call parameter */
                     void         *ptr, /* optional call parameter */
                     unsigned int  length,
                     void         *ctx,      /* optional handler context */
                     unsigned int  serial,
                     void         *ret_ptr,
                     unsigned int  ret_size,
                     unsigned int *ret_length )
{
    CoreDFB *obj = (CoreDFB*) ctx;
    CoreDFBDispatch__Dispatch( obj, caller, call_arg, ptr, length, ret_ptr, ret_size, ret_length );

    return FCHR_RETURN;
}

void CoreDFB_Init_Dispatch(
                    CoreDFB              *core,
                    CoreDFB              *obj,
                    FusionCall           *call
)
{
    fusion_call_init3( call, CoreDFB_Dispatch, obj, core->world );
}

void  CoreDFB_Deinit_Dispatch(
                    FusionCall           *call
)
{
     fusion_call_destroy( call );
}

/*********************************************************************************************************************/


DFBResult
ICore_Requestor__Register( CoreDFB *obj,
                    u32                                        slave_call
)
{
    DFBResult           ret;
    CoreDFBRegister       *args = (CoreDFBRegister*) alloca( sizeof(CoreDFBRegister) );
    CoreDFBRegisterReturn *return_args = (CoreDFBRegisterReturn*) alloca( sizeof(CoreDFBRegisterReturn) );

    D_DEBUG_AT( DirectFB_CoreDFB, "ICore_Requestor::%s()\n", __FUNCTION__ );


    args->slave_call = slave_call;

    ret = (DFBResult) CoreDFB_Call( obj, FCEF_NONE, _CoreDFB_Register, args, sizeof(CoreDFBRegister), return_args, sizeof(CoreDFBRegisterReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreDFB_Call( CoreDFB_Register ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreDFB_Register failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ICore_Requestor__CreateSurface( CoreDFB *obj,
                    const CoreSurfaceConfig                   *config,
                    CoreSurfaceTypeFlags                       type,
                    u64                                        resource_id,
                    CorePalette                               *palette,
                    CoreSurface                              **ret_surface
)
{
    DFBResult           ret;
    CoreSurface *surface = NULL;
    CoreDFBCreateSurface       *args = (CoreDFBCreateSurface*) alloca( sizeof(CoreDFBCreateSurface) );
    CoreDFBCreateSurfaceReturn *return_args = (CoreDFBCreateSurfaceReturn*) alloca( sizeof(CoreDFBCreateSurfaceReturn) );

    D_DEBUG_AT( DirectFB_CoreDFB, "ICore_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( config != NULL );
    D_ASSERT( ret_surface != NULL );

    args->config = *config;
    args->type = type;
    args->resource_id = resource_id;
  if (palette) {
    args->palette_id = CorePalette_GetID( palette );
    args->palette_set = true;
  }
  else
    args->palette_set = false;

    ret = (DFBResult) CoreDFB_Call( obj, FCEF_NONE, _CoreDFB_CreateSurface, args, sizeof(CoreDFBCreateSurface), return_args, sizeof(CoreDFBCreateSurfaceReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreDFB_Call( CoreDFB_CreateSurface ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreDFB_CreateSurface failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreSurface_Catch( core_dfb, return_args->surface_id, &surface );
    if (ret) {
         D_DERROR( ret, "%s: Catching surface by ID %u failed!\n", __FUNCTION__, return_args->surface_id );
         return ret;
    }

    *ret_surface = surface;

    return DFB_OK;
}


DFBResult
ICore_Requestor__CreatePalette( CoreDFB *obj,
                    u32                                        size,
                    CorePalette                              **ret_palette
)
{
    DFBResult           ret;
    CorePalette *palette = NULL;
    CoreDFBCreatePalette       *args = (CoreDFBCreatePalette*) alloca( sizeof(CoreDFBCreatePalette) );
    CoreDFBCreatePaletteReturn *return_args = (CoreDFBCreatePaletteReturn*) alloca( sizeof(CoreDFBCreatePaletteReturn) );

    D_DEBUG_AT( DirectFB_CoreDFB, "ICore_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_palette != NULL );

    args->size = size;

    ret = (DFBResult) CoreDFB_Call( obj, FCEF_NONE, _CoreDFB_CreatePalette, args, sizeof(CoreDFBCreatePalette), return_args, sizeof(CoreDFBCreatePaletteReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreDFB_Call( CoreDFB_CreatePalette ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreDFB_CreatePalette failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CorePalette_Catch( core_dfb, return_args->palette_id, &palette );
    if (ret) {
         D_DERROR( ret, "%s: Catching palette by ID %u failed!\n", __FUNCTION__, return_args->palette_id );
         return ret;
    }

    *ret_palette = palette;

    return DFB_OK;
}


DFBResult
ICore_Requestor__CreateState( CoreDFB *obj,
                    CoreGraphicsState                        **ret_state
)
{
    DFBResult           ret;
    CoreGraphicsState *state = NULL;
    CoreDFBCreateState       *args = (CoreDFBCreateState*) alloca( sizeof(CoreDFBCreateState) );
    CoreDFBCreateStateReturn *return_args = (CoreDFBCreateStateReturn*) alloca( sizeof(CoreDFBCreateStateReturn) );

    D_DEBUG_AT( DirectFB_CoreDFB, "ICore_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_state != NULL );


    ret = (DFBResult) CoreDFB_Call( obj, FCEF_NONE, _CoreDFB_CreateState, args, sizeof(CoreDFBCreateState), return_args, sizeof(CoreDFBCreateStateReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreDFB_Call( CoreDFB_CreateState ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreDFB_CreateState failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreGraphicsState_Catch( core_dfb, return_args->state_id, &state );
    if (ret) {
         D_DERROR( ret, "%s: Catching state by ID %u failed!\n", __FUNCTION__, return_args->state_id );
         return ret;
    }

    *ret_state = state;

    return DFB_OK;
}


DFBResult
ICore_Requestor__WaitIdle( CoreDFB *obj

)
{
    DFBResult           ret;
    CoreDFBWaitIdle       *args = (CoreDFBWaitIdle*) alloca( sizeof(CoreDFBWaitIdle) );
    CoreDFBWaitIdleReturn *return_args = (CoreDFBWaitIdleReturn*) alloca( sizeof(CoreDFBWaitIdleReturn) );

    D_DEBUG_AT( DirectFB_CoreDFB, "ICore_Requestor::%s()\n", __FUNCTION__ );



    ret = (DFBResult) CoreDFB_Call( obj, FCEF_NONE, _CoreDFB_WaitIdle, args, sizeof(CoreDFBWaitIdle), return_args, sizeof(CoreDFBWaitIdleReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreDFB_Call( CoreDFB_WaitIdle ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreDFB_WaitIdle failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ICore_Requestor__CreateImageProvider( CoreDFB *obj,
                    u32                                        buffer_call,
                    u32                                       *ret_call
)
{
    DFBResult           ret;
    CoreDFBCreateImageProvider       *args = (CoreDFBCreateImageProvider*) alloca( sizeof(CoreDFBCreateImageProvider) );
    CoreDFBCreateImageProviderReturn *return_args = (CoreDFBCreateImageProviderReturn*) alloca( sizeof(CoreDFBCreateImageProviderReturn) );

    D_DEBUG_AT( DirectFB_CoreDFB, "ICore_Requestor::%s()\n", __FUNCTION__ );


    args->buffer_call = buffer_call;

    ret = (DFBResult) CoreDFB_Call( obj, FCEF_NONE, _CoreDFB_CreateImageProvider, args, sizeof(CoreDFBCreateImageProvider), return_args, sizeof(CoreDFBCreateImageProviderReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreDFB_Call( CoreDFB_CreateImageProvider ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreDFB_CreateImageProvider failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }

    *ret_call = return_args->call;

    return DFB_OK;
}

/*********************************************************************************************************************/

static DFBResult
__CoreDFBDispatch__Dispatch( CoreDFB *obj,
                                FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length )
{
    D_UNUSED
    DFBResult ret;


    switch (method) {
        case _CoreDFB_Register: {
            D_UNUSED
            CoreDFBRegister       *args        = (CoreDFBRegister *) ptr;
            CoreDFBRegisterReturn *return_args = (CoreDFBRegisterReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreDFB, "=-> CoreDFB_Register\n" );

            return_args->result = ICore_Real__Register( obj, args->slave_call );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreDFBRegisterReturn);

            return DFB_OK;
        }

        case _CoreDFB_CreateSurface: {
    CorePalette *palette = NULL;
    CoreSurface *surface = NULL;
            D_UNUSED
            CoreDFBCreateSurface       *args        = (CoreDFBCreateSurface *) ptr;
            CoreDFBCreateSurfaceReturn *return_args = (CoreDFBCreateSurfaceReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreDFB, "=-> CoreDFB_CreateSurface\n" );

            if (args->palette_set) {
                ret = (DFBResult) CorePalette_Lookup( core_dfb, args->palette_id, caller, &palette );
                if (ret) {
                     D_DERROR( ret, "%s: Looking up palette by ID %u failed!\n", __FUNCTION__, args->palette_id );
                     return_args->result = ret;
                     return DFB_OK;
                }
            }

            return_args->result = ICore_Real__CreateSurface( obj, &args->config, args->type, args->resource_id, args->palette_set ? palette : NULL, &surface );
            if (return_args->result == DFB_OK) {
                CoreSurface_Throw( surface, caller, &return_args->surface_id );
            }

            *ret_length = sizeof(CoreDFBCreateSurfaceReturn);

            if (palette)
                CorePalette_Unref( palette );

            return DFB_OK;
        }

        case _CoreDFB_CreatePalette: {
    CorePalette *palette = NULL;
            D_UNUSED
            CoreDFBCreatePalette       *args        = (CoreDFBCreatePalette *) ptr;
            CoreDFBCreatePaletteReturn *return_args = (CoreDFBCreatePaletteReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreDFB, "=-> CoreDFB_CreatePalette\n" );

            return_args->result = ICore_Real__CreatePalette( obj, args->size, &palette );
            if (return_args->result == DFB_OK) {
                CorePalette_Throw( palette, caller, &return_args->palette_id );
            }

            *ret_length = sizeof(CoreDFBCreatePaletteReturn);

            return DFB_OK;
        }

        case _CoreDFB_CreateState: {
    CoreGraphicsState *state = NULL;
            D_UNUSED
            CoreDFBCreateState       *args        = (CoreDFBCreateState *) ptr;
            CoreDFBCreateStateReturn *return_args = (CoreDFBCreateStateReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreDFB, "=-> CoreDFB_CreateState\n" );

            return_args->result = ICore_Real__CreateState( obj, &state );
            if (return_args->result == DFB_OK) {
                CoreGraphicsState_Throw( state, caller, &return_args->state_id );
            }

            *ret_length = sizeof(CoreDFBCreateStateReturn);

            return DFB_OK;
        }

        case _CoreDFB_WaitIdle: {
            D_UNUSED
            CoreDFBWaitIdle       *args        = (CoreDFBWaitIdle *) ptr;
            CoreDFBWaitIdleReturn *return_args = (CoreDFBWaitIdleReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreDFB, "=-> CoreDFB_WaitIdle\n" );

            return_args->result = ICore_Real__WaitIdle( obj );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreDFBWaitIdleReturn);

            return DFB_OK;
        }

        case _CoreDFB_CreateImageProvider: {
            D_UNUSED
            CoreDFBCreateImageProvider       *args        = (CoreDFBCreateImageProvider *) ptr;
            CoreDFBCreateImageProviderReturn *return_args = (CoreDFBCreateImageProviderReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreDFB, "=-> CoreDFB_CreateImageProvider\n" );

            return_args->result = ICore_Real__CreateImageProvider( obj, args->buffer_call, &return_args->call );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreDFBCreateImageProviderReturn);

            return DFB_OK;
        }

    }

    return DFB_NOSUCHMETHOD;
}
/*********************************************************************************************************************/

DFBResult
CoreDFBDispatch__Dispatch( CoreDFB *obj,
                                FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length )
{
    DFBResult ret;

    D_DEBUG_AT( DirectFB_CoreDFB, "CoreDFBDispatch::%s( %p )\n", __FUNCTION__, obj );

    Core_PushIdentity( caller );

    ret = __CoreDFBDispatch__Dispatch( obj, caller, method, ptr, length, ret_ptr, ret_size, ret_length );

    Core_PopIdentity();

    return ret;
}
