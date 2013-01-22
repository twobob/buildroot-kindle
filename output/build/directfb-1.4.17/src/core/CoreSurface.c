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

#include "CoreSurface.h"

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <fusion/conf.h>

#include <core/core.h>

#include <core/CoreDFB_CallMode.h>

D_DEBUG_DOMAIN( DirectFB_CoreSurface, "DirectFB/CoreSurface", "DirectFB CoreSurface" );

/*********************************************************************************************************************/

DFBResult
CoreSurface_SetConfig(
                    CoreSurface                               *obj,
                    const CoreSurfaceConfig                   *config
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__SetConfig( obj, config );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__SetConfig( obj, config );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_Flip(
                    CoreSurface                               *obj,
                    bool                                       swap
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__Flip( obj, swap );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__Flip( obj, swap );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_GetPalette(
                    CoreSurface                               *obj,
                    CorePalette                              **ret_palette
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__GetPalette( obj, ret_palette );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__GetPalette( obj, ret_palette );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_SetPalette(
                    CoreSurface                               *obj,
                    CorePalette                               *palette
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__SetPalette( obj, palette );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__SetPalette( obj, palette );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_SetAlphaRamp(
                    CoreSurface                               *obj,
                    u8                                         a0,
                    u8                                         a1,
                    u8                                         a2,
                    u8                                         a3
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__SetAlphaRamp( obj, a0, a1, a2, a3 );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__SetAlphaRamp( obj, a0, a1, a2, a3 );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_SetField(
                    CoreSurface                               *obj,
                    s32                                        field
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__SetField( obj, field );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__SetField( obj, field );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_PreLockBuffer(
                    CoreSurface                               *obj,
                    CoreSurfaceBuffer                         *buffer,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__PreLockBuffer( obj, buffer, accessor, access, ret_allocation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__PreLockBuffer( obj, buffer, accessor, access, ret_allocation );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_PreLockBuffer2(
                    CoreSurface                               *obj,
                    CoreSurfaceBufferRole                      role,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    bool                                       lock,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__PreLockBuffer2( obj, role, accessor, access, lock, ret_allocation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__PreLockBuffer2( obj, role, accessor, access, lock, ret_allocation );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_PreReadBuffer(
                    CoreSurface                               *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__PreReadBuffer( obj, buffer, rect, ret_allocation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__PreReadBuffer( obj, buffer, rect, ret_allocation );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreSurface_PreWriteBuffer(
                    CoreSurface                               *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ISurface_Real__PreWriteBuffer( obj, buffer, rect, ret_allocation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ISurface_Requestor__PreWriteBuffer( obj, buffer, rect, ret_allocation );
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
CoreSurface_Dispatch( int           caller,   /* fusion id of the caller */
                     int           call_arg, /* optional call parameter */
                     void         *ptr, /* optional call parameter */
                     unsigned int  length,
                     void         *ctx,      /* optional handler context */
                     unsigned int  serial,
                     void         *ret_ptr,
                     unsigned int  ret_size,
                     unsigned int *ret_length )
{
    CoreSurface *obj = (CoreSurface*) ctx;
    CoreSurfaceDispatch__Dispatch( obj, caller, call_arg, ptr, length, ret_ptr, ret_size, ret_length );

    return FCHR_RETURN;
}

void CoreSurface_Init_Dispatch(
                    CoreDFB              *core,
                    CoreSurface          *obj,
                    FusionCall           *call
)
{
    fusion_call_init3( call, CoreSurface_Dispatch, obj, core->world );
}

void  CoreSurface_Deinit_Dispatch(
                    FusionCall           *call
)
{
     fusion_call_destroy( call );
}

/*********************************************************************************************************************/


DFBResult
ISurface_Requestor__SetConfig( CoreSurface *obj,
                    const CoreSurfaceConfig                   *config
)
{
    DFBResult           ret;
    CoreSurfaceSetConfig       *args = (CoreSurfaceSetConfig*) alloca( sizeof(CoreSurfaceSetConfig) );
    CoreSurfaceSetConfigReturn *return_args = (CoreSurfaceSetConfigReturn*) alloca( sizeof(CoreSurfaceSetConfigReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( config != NULL );

    args->config = *config;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_SetConfig, args, sizeof(CoreSurfaceSetConfig), return_args, sizeof(CoreSurfaceSetConfigReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_SetConfig ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_SetConfig failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ISurface_Requestor__Flip( CoreSurface *obj,
                    bool                                       swap
)
{
    DFBResult           ret;
    CoreSurfaceFlip       *args = (CoreSurfaceFlip*) alloca( sizeof(CoreSurfaceFlip) );
    CoreSurfaceFlipReturn *return_args = (CoreSurfaceFlipReturn*) alloca( sizeof(CoreSurfaceFlipReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );


    args->swap = swap;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_Flip, args, sizeof(CoreSurfaceFlip), return_args, sizeof(CoreSurfaceFlipReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_Flip ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_Flip failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ISurface_Requestor__GetPalette( CoreSurface *obj,
                    CorePalette                              **ret_palette
)
{
    DFBResult           ret;
    CorePalette *palette = NULL;
    CoreSurfaceGetPalette       *args = (CoreSurfaceGetPalette*) alloca( sizeof(CoreSurfaceGetPalette) );
    CoreSurfaceGetPaletteReturn *return_args = (CoreSurfaceGetPaletteReturn*) alloca( sizeof(CoreSurfaceGetPaletteReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_palette != NULL );


    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_GetPalette, args, sizeof(CoreSurfaceGetPalette), return_args, sizeof(CoreSurfaceGetPaletteReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_GetPalette ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_GetPalette failed!\n", __FUNCTION__ );*/
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
ISurface_Requestor__SetPalette( CoreSurface *obj,
                    CorePalette                               *palette
)
{
    DFBResult           ret;
    CoreSurfaceSetPalette       *args = (CoreSurfaceSetPalette*) alloca( sizeof(CoreSurfaceSetPalette) );
    CoreSurfaceSetPaletteReturn *return_args = (CoreSurfaceSetPaletteReturn*) alloca( sizeof(CoreSurfaceSetPaletteReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( palette != NULL );

    args->palette_id = CorePalette_GetID( palette );

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_SetPalette, args, sizeof(CoreSurfaceSetPalette), return_args, sizeof(CoreSurfaceSetPaletteReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_SetPalette ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_SetPalette failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ISurface_Requestor__SetAlphaRamp( CoreSurface *obj,
                    u8                                         a0,
                    u8                                         a1,
                    u8                                         a2,
                    u8                                         a3
)
{
    DFBResult           ret;
    CoreSurfaceSetAlphaRamp       *args = (CoreSurfaceSetAlphaRamp*) alloca( sizeof(CoreSurfaceSetAlphaRamp) );
    CoreSurfaceSetAlphaRampReturn *return_args = (CoreSurfaceSetAlphaRampReturn*) alloca( sizeof(CoreSurfaceSetAlphaRampReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );


    args->a0 = a0;
    args->a1 = a1;
    args->a2 = a2;
    args->a3 = a3;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_SetAlphaRamp, args, sizeof(CoreSurfaceSetAlphaRamp), return_args, sizeof(CoreSurfaceSetAlphaRampReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_SetAlphaRamp ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_SetAlphaRamp failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ISurface_Requestor__SetField( CoreSurface *obj,
                    s32                                        field
)
{
    DFBResult           ret;
    CoreSurfaceSetField       *args = (CoreSurfaceSetField*) alloca( sizeof(CoreSurfaceSetField) );
    CoreSurfaceSetFieldReturn *return_args = (CoreSurfaceSetFieldReturn*) alloca( sizeof(CoreSurfaceSetFieldReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );


    args->field = field;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_SetField, args, sizeof(CoreSurfaceSetField), return_args, sizeof(CoreSurfaceSetFieldReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_SetField ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_SetField failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ISurface_Requestor__PreLockBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult           ret;
    CoreSurfaceAllocation *allocation = NULL;
    CoreSurfacePreLockBuffer       *args = (CoreSurfacePreLockBuffer*) alloca( sizeof(CoreSurfacePreLockBuffer) );
    CoreSurfacePreLockBufferReturn *return_args = (CoreSurfacePreLockBufferReturn*) alloca( sizeof(CoreSurfacePreLockBufferReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( buffer != NULL );
    D_ASSERT( ret_allocation != NULL );

    args->buffer_id = CoreSurfaceBuffer_GetID( buffer );
    args->accessor = accessor;
    args->access = access;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_PreLockBuffer, args, sizeof(CoreSurfacePreLockBuffer), return_args, sizeof(CoreSurfacePreLockBufferReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_PreLockBuffer ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_PreLockBuffer failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreSurfaceAllocation_Catch( core_dfb, return_args->allocation_id, &allocation );
    if (ret) {
         D_DERROR( ret, "%s: Catching allocation by ID %u failed!\n", __FUNCTION__, return_args->allocation_id );
         return ret;
    }

    *ret_allocation = allocation;

    return DFB_OK;
}


DFBResult
ISurface_Requestor__PreLockBuffer2( CoreSurface *obj,
                    CoreSurfaceBufferRole                      role,
                    CoreSurfaceAccessorID                      accessor,
                    CoreSurfaceAccessFlags                     access,
                    bool                                       lock,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult           ret;
    CoreSurfaceAllocation *allocation = NULL;
    CoreSurfacePreLockBuffer2       *args = (CoreSurfacePreLockBuffer2*) alloca( sizeof(CoreSurfacePreLockBuffer2) );
    CoreSurfacePreLockBuffer2Return *return_args = (CoreSurfacePreLockBuffer2Return*) alloca( sizeof(CoreSurfacePreLockBuffer2Return) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_allocation != NULL );

    args->role = role;
    args->accessor = accessor;
    args->access = access;
    args->lock = lock;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_PreLockBuffer2, args, sizeof(CoreSurfacePreLockBuffer2), return_args, sizeof(CoreSurfacePreLockBuffer2Return), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_PreLockBuffer2 ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_PreLockBuffer2 failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreSurfaceAllocation_Catch( core_dfb, return_args->allocation_id, &allocation );
    if (ret) {
         D_DERROR( ret, "%s: Catching allocation by ID %u failed!\n", __FUNCTION__, return_args->allocation_id );
         return ret;
    }

    *ret_allocation = allocation;

    return DFB_OK;
}


DFBResult
ISurface_Requestor__PreReadBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult           ret;
    CoreSurfaceAllocation *allocation = NULL;
    CoreSurfacePreReadBuffer       *args = (CoreSurfacePreReadBuffer*) alloca( sizeof(CoreSurfacePreReadBuffer) );
    CoreSurfacePreReadBufferReturn *return_args = (CoreSurfacePreReadBufferReturn*) alloca( sizeof(CoreSurfacePreReadBufferReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( buffer != NULL );
    D_ASSERT( rect != NULL );
    D_ASSERT( ret_allocation != NULL );

    args->buffer_id = CoreSurfaceBuffer_GetID( buffer );
    args->rect = *rect;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_PreReadBuffer, args, sizeof(CoreSurfacePreReadBuffer), return_args, sizeof(CoreSurfacePreReadBufferReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_PreReadBuffer ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_PreReadBuffer failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreSurfaceAllocation_Catch( core_dfb, return_args->allocation_id, &allocation );
    if (ret) {
         D_DERROR( ret, "%s: Catching allocation by ID %u failed!\n", __FUNCTION__, return_args->allocation_id );
         return ret;
    }

    *ret_allocation = allocation;

    return DFB_OK;
}


DFBResult
ISurface_Requestor__PreWriteBuffer( CoreSurface *obj,
                    CoreSurfaceBuffer                         *buffer,
                    const DFBRectangle                        *rect,
                    CoreSurfaceAllocation                    **ret_allocation
)
{
    DFBResult           ret;
    CoreSurfaceAllocation *allocation = NULL;
    CoreSurfacePreWriteBuffer       *args = (CoreSurfacePreWriteBuffer*) alloca( sizeof(CoreSurfacePreWriteBuffer) );
    CoreSurfacePreWriteBufferReturn *return_args = (CoreSurfacePreWriteBufferReturn*) alloca( sizeof(CoreSurfacePreWriteBufferReturn) );

    D_DEBUG_AT( DirectFB_CoreSurface, "ISurface_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( buffer != NULL );
    D_ASSERT( rect != NULL );
    D_ASSERT( ret_allocation != NULL );

    args->buffer_id = CoreSurfaceBuffer_GetID( buffer );
    args->rect = *rect;

    ret = (DFBResult) CoreSurface_Call( obj, FCEF_NONE, _CoreSurface_PreWriteBuffer, args, sizeof(CoreSurfacePreWriteBuffer), return_args, sizeof(CoreSurfacePreWriteBufferReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreSurface_Call( CoreSurface_PreWriteBuffer ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreSurface_PreWriteBuffer failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreSurfaceAllocation_Catch( core_dfb, return_args->allocation_id, &allocation );
    if (ret) {
         D_DERROR( ret, "%s: Catching allocation by ID %u failed!\n", __FUNCTION__, return_args->allocation_id );
         return ret;
    }

    *ret_allocation = allocation;

    return DFB_OK;
}

/*********************************************************************************************************************/

static DFBResult
__CoreSurfaceDispatch__Dispatch( CoreSurface *obj,
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
        case _CoreSurface_SetConfig: {
            D_UNUSED
            CoreSurfaceSetConfig       *args        = (CoreSurfaceSetConfig *) ptr;
            CoreSurfaceSetConfigReturn *return_args = (CoreSurfaceSetConfigReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_SetConfig\n" );

            return_args->result = ISurface_Real__SetConfig( obj, &args->config );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreSurfaceSetConfigReturn);

            return DFB_OK;
        }

        case _CoreSurface_Flip: {
            D_UNUSED
            CoreSurfaceFlip       *args        = (CoreSurfaceFlip *) ptr;
            CoreSurfaceFlipReturn *return_args = (CoreSurfaceFlipReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_Flip\n" );

            return_args->result = ISurface_Real__Flip( obj, args->swap );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreSurfaceFlipReturn);

            return DFB_OK;
        }

        case _CoreSurface_GetPalette: {
    CorePalette *palette = NULL;
            D_UNUSED
            CoreSurfaceGetPalette       *args        = (CoreSurfaceGetPalette *) ptr;
            CoreSurfaceGetPaletteReturn *return_args = (CoreSurfaceGetPaletteReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_GetPalette\n" );

            return_args->result = ISurface_Real__GetPalette( obj, &palette );
            if (return_args->result == DFB_OK) {
                CorePalette_Throw( palette, caller, &return_args->palette_id );
            }

            *ret_length = sizeof(CoreSurfaceGetPaletteReturn);

            return DFB_OK;
        }

        case _CoreSurface_SetPalette: {
    CorePalette *palette = NULL;
            D_UNUSED
            CoreSurfaceSetPalette       *args        = (CoreSurfaceSetPalette *) ptr;
            CoreSurfaceSetPaletteReturn *return_args = (CoreSurfaceSetPaletteReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_SetPalette\n" );

            ret = (DFBResult) CorePalette_Lookup( core_dfb, args->palette_id, caller, &palette );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up palette by ID %u failed!\n", __FUNCTION__, args->palette_id );
                 return_args->result = ret;
                 return DFB_OK;
            }

            return_args->result = ISurface_Real__SetPalette( obj, palette );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreSurfaceSetPaletteReturn);

            if (palette)
                CorePalette_Unref( palette );

            return DFB_OK;
        }

        case _CoreSurface_SetAlphaRamp: {
            D_UNUSED
            CoreSurfaceSetAlphaRamp       *args        = (CoreSurfaceSetAlphaRamp *) ptr;
            CoreSurfaceSetAlphaRampReturn *return_args = (CoreSurfaceSetAlphaRampReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_SetAlphaRamp\n" );

            return_args->result = ISurface_Real__SetAlphaRamp( obj, args->a0, args->a1, args->a2, args->a3 );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreSurfaceSetAlphaRampReturn);

            return DFB_OK;
        }

        case _CoreSurface_SetField: {
            D_UNUSED
            CoreSurfaceSetField       *args        = (CoreSurfaceSetField *) ptr;
            CoreSurfaceSetFieldReturn *return_args = (CoreSurfaceSetFieldReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_SetField\n" );

            return_args->result = ISurface_Real__SetField( obj, args->field );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreSurfaceSetFieldReturn);

            return DFB_OK;
        }

        case _CoreSurface_PreLockBuffer: {
    CoreSurfaceBuffer *buffer = NULL;
    CoreSurfaceAllocation *allocation = NULL;
            D_UNUSED
            CoreSurfacePreLockBuffer       *args        = (CoreSurfacePreLockBuffer *) ptr;
            CoreSurfacePreLockBufferReturn *return_args = (CoreSurfacePreLockBufferReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_PreLockBuffer\n" );

            ret = (DFBResult) CoreSurfaceBuffer_Lookup( core_dfb, args->buffer_id, caller, &buffer );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up buffer by ID %u failed!\n", __FUNCTION__, args->buffer_id );
                 return_args->result = ret;
                 return DFB_OK;
            }

            return_args->result = ISurface_Real__PreLockBuffer( obj, buffer, args->accessor, args->access, &allocation );
            if (return_args->result == DFB_OK) {
                CoreSurfaceAllocation_Throw( allocation, caller, &return_args->allocation_id );
            }

            *ret_length = sizeof(CoreSurfacePreLockBufferReturn);

            if (buffer)
                CoreSurfaceBuffer_Unref( buffer );

            return DFB_OK;
        }

        case _CoreSurface_PreLockBuffer2: {
    CoreSurfaceAllocation *allocation = NULL;
            D_UNUSED
            CoreSurfacePreLockBuffer2       *args        = (CoreSurfacePreLockBuffer2 *) ptr;
            CoreSurfacePreLockBuffer2Return *return_args = (CoreSurfacePreLockBuffer2Return *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_PreLockBuffer2\n" );

            return_args->result = ISurface_Real__PreLockBuffer2( obj, args->role, args->accessor, args->access, args->lock, &allocation );
            if (return_args->result == DFB_OK) {
                CoreSurfaceAllocation_Throw( allocation, caller, &return_args->allocation_id );
            }

            *ret_length = sizeof(CoreSurfacePreLockBuffer2Return);

            return DFB_OK;
        }

        case _CoreSurface_PreReadBuffer: {
    CoreSurfaceBuffer *buffer = NULL;
    CoreSurfaceAllocation *allocation = NULL;
            D_UNUSED
            CoreSurfacePreReadBuffer       *args        = (CoreSurfacePreReadBuffer *) ptr;
            CoreSurfacePreReadBufferReturn *return_args = (CoreSurfacePreReadBufferReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_PreReadBuffer\n" );

            ret = (DFBResult) CoreSurfaceBuffer_Lookup( core_dfb, args->buffer_id, caller, &buffer );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up buffer by ID %u failed!\n", __FUNCTION__, args->buffer_id );
                 return_args->result = ret;
                 return DFB_OK;
            }

            return_args->result = ISurface_Real__PreReadBuffer( obj, buffer, &args->rect, &allocation );
            if (return_args->result == DFB_OK) {
                CoreSurfaceAllocation_Throw( allocation, caller, &return_args->allocation_id );
            }

            *ret_length = sizeof(CoreSurfacePreReadBufferReturn);

            if (buffer)
                CoreSurfaceBuffer_Unref( buffer );

            return DFB_OK;
        }

        case _CoreSurface_PreWriteBuffer: {
    CoreSurfaceBuffer *buffer = NULL;
    CoreSurfaceAllocation *allocation = NULL;
            D_UNUSED
            CoreSurfacePreWriteBuffer       *args        = (CoreSurfacePreWriteBuffer *) ptr;
            CoreSurfacePreWriteBufferReturn *return_args = (CoreSurfacePreWriteBufferReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreSurface, "=-> CoreSurface_PreWriteBuffer\n" );

            ret = (DFBResult) CoreSurfaceBuffer_Lookup( core_dfb, args->buffer_id, caller, &buffer );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up buffer by ID %u failed!\n", __FUNCTION__, args->buffer_id );
                 return_args->result = ret;
                 return DFB_OK;
            }

            return_args->result = ISurface_Real__PreWriteBuffer( obj, buffer, &args->rect, &allocation );
            if (return_args->result == DFB_OK) {
                CoreSurfaceAllocation_Throw( allocation, caller, &return_args->allocation_id );
            }

            *ret_length = sizeof(CoreSurfacePreWriteBufferReturn);

            if (buffer)
                CoreSurfaceBuffer_Unref( buffer );

            return DFB_OK;
        }

    }

    return DFB_NOSUCHMETHOD;
}
/*********************************************************************************************************************/

DFBResult
CoreSurfaceDispatch__Dispatch( CoreSurface *obj,
                                FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length )
{
    DFBResult ret;

    D_DEBUG_AT( DirectFB_CoreSurface, "CoreSurfaceDispatch::%s( %p )\n", __FUNCTION__, obj );

    Core_PushIdentity( caller );

    ret = __CoreSurfaceDispatch__Dispatch( obj, caller, method, ptr, length, ret_ptr, ret_size, ret_length );

    Core_PopIdentity();

    return ret;
}
