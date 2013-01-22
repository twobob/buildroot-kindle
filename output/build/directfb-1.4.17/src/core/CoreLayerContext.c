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

#include "CoreLayerContext.h"

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <fusion/conf.h>

#include <core/core.h>

#include <core/CoreDFB_CallMode.h>

D_DEBUG_DOMAIN( DirectFB_CoreLayerContext, "DirectFB/CoreLayerContext", "DirectFB CoreLayerContext" );

/*********************************************************************************************************************/

DFBResult
CoreLayerContext_GetPrimaryRegion(
                    CoreLayerContext                          *obj,
                    bool                                       create,
                    CoreLayerRegion                          **ret_region
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__GetPrimaryRegion( obj, create, ret_region );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__GetPrimaryRegion( obj, create, ret_region );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_TestConfiguration(
                    CoreLayerContext                          *obj,
                    const DFBDisplayLayerConfig               *config,
                    DFBDisplayLayerConfigFlags                *ret_failed
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__TestConfiguration( obj, config, ret_failed );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__TestConfiguration( obj, config, ret_failed );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetConfiguration(
                    CoreLayerContext                          *obj,
                    const DFBDisplayLayerConfig               *config
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetConfiguration( obj, config );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetConfiguration( obj, config );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetSrcColorKey(
                    CoreLayerContext                          *obj,
                    const DFBColorKey                         *key
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetSrcColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetSrcColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetDstColorKey(
                    CoreLayerContext                          *obj,
                    const DFBColorKey                         *key
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetDstColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetDstColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetSourceRectangle(
                    CoreLayerContext                          *obj,
                    const DFBRectangle                        *rectangle
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetSourceRectangle( obj, rectangle );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetSourceRectangle( obj, rectangle );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetScreenLocation(
                    CoreLayerContext                          *obj,
                    const DFBLocation                         *location
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetScreenLocation( obj, location );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetScreenLocation( obj, location );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetScreenRectangle(
                    CoreLayerContext                          *obj,
                    const DFBRectangle                        *rectangle
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetScreenRectangle( obj, rectangle );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetScreenRectangle( obj, rectangle );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetScreenPosition(
                    CoreLayerContext                          *obj,
                    const DFBPoint                            *position
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetScreenPosition( obj, position );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetScreenPosition( obj, position );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetOpacity(
                    CoreLayerContext                          *obj,
                    u8                                         opacity
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetOpacity( obj, opacity );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetOpacity( obj, opacity );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetRotation(
                    CoreLayerContext                          *obj,
                    s32                                        rotation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetRotation( obj, rotation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetRotation( obj, rotation );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetColorAdjustment(
                    CoreLayerContext                          *obj,
                    const DFBColorAdjustment                  *adjustment
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetColorAdjustment( obj, adjustment );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetColorAdjustment( obj, adjustment );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetFieldParity(
                    CoreLayerContext                          *obj,
                    u32                                        field
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetFieldParity( obj, field );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetFieldParity( obj, field );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_SetClipRegions(
                    CoreLayerContext                          *obj,
                    const DFBRegion                           *regions,
                    u32                                        num,
                    bool                                       positive
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__SetClipRegions( obj, regions, num, positive );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__SetClipRegions( obj, regions, num, positive );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_CreateWindow(
                    CoreLayerContext                          *obj,
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__CreateWindow( obj, description, parent, toplevel, ret_window );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__CreateWindow( obj, description, parent, toplevel, ret_window );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_FindWindow(
                    CoreLayerContext                          *obj,
                    DFBWindowID                                window_id,
                    CoreWindow                               **ret_window
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__FindWindow( obj, window_id, ret_window );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__FindWindow( obj, window_id, ret_window );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreLayerContext_FindWindowByResourceID(
                    CoreLayerContext                          *obj,
                    u64                                        resource_id,
                    CoreWindow                               **ret_window
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = ILayerContext_Real__FindWindowByResourceID( obj, resource_id, ret_window );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = ILayerContext_Requestor__FindWindowByResourceID( obj, resource_id, ret_window );
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
CoreLayerContext_Dispatch( int           caller,   /* fusion id of the caller */
                     int           call_arg, /* optional call parameter */
                     void         *ptr, /* optional call parameter */
                     unsigned int  length,
                     void         *ctx,      /* optional handler context */
                     unsigned int  serial,
                     void         *ret_ptr,
                     unsigned int  ret_size,
                     unsigned int *ret_length )
{
    CoreLayerContext *obj = (CoreLayerContext*) ctx;
    CoreLayerContextDispatch__Dispatch( obj, caller, call_arg, ptr, length, ret_ptr, ret_size, ret_length );

    return FCHR_RETURN;
}

void CoreLayerContext_Init_Dispatch(
                    CoreDFB              *core,
                    CoreLayerContext     *obj,
                    FusionCall           *call
)
{
    fusion_call_init3( call, CoreLayerContext_Dispatch, obj, core->world );
}

void  CoreLayerContext_Deinit_Dispatch(
                    FusionCall           *call
)
{
     fusion_call_destroy( call );
}

/*********************************************************************************************************************/


DFBResult
ILayerContext_Requestor__GetPrimaryRegion( CoreLayerContext *obj,
                    bool                                       create,
                    CoreLayerRegion                          **ret_region
)
{
    DFBResult           ret;
    CoreLayerRegion *region = NULL;
    CoreLayerContextGetPrimaryRegion       *args = (CoreLayerContextGetPrimaryRegion*) alloca( sizeof(CoreLayerContextGetPrimaryRegion) );
    CoreLayerContextGetPrimaryRegionReturn *return_args = (CoreLayerContextGetPrimaryRegionReturn*) alloca( sizeof(CoreLayerContextGetPrimaryRegionReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_region != NULL );

    args->create = create;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_GetPrimaryRegion, args, sizeof(CoreLayerContextGetPrimaryRegion), return_args, sizeof(CoreLayerContextGetPrimaryRegionReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_GetPrimaryRegion ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_GetPrimaryRegion failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreLayerRegion_Catch( core_dfb, return_args->region_id, &region );
    if (ret) {
         D_DERROR( ret, "%s: Catching region by ID %u failed!\n", __FUNCTION__, return_args->region_id );
         return ret;
    }

    *ret_region = region;

    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__TestConfiguration( CoreLayerContext *obj,
                    const DFBDisplayLayerConfig               *config,
                    DFBDisplayLayerConfigFlags                *ret_failed
)
{
    DFBResult           ret;
    CoreLayerContextTestConfiguration       *args = (CoreLayerContextTestConfiguration*) alloca( sizeof(CoreLayerContextTestConfiguration) );
    CoreLayerContextTestConfigurationReturn *return_args = (CoreLayerContextTestConfigurationReturn*) alloca( sizeof(CoreLayerContextTestConfigurationReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( config != NULL );

    args->config = *config;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_TestConfiguration, args, sizeof(CoreLayerContextTestConfiguration), return_args, sizeof(CoreLayerContextTestConfigurationReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_TestConfiguration ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_TestConfiguration failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }

    if (ret_failed)
        *ret_failed = return_args->failed;

    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetConfiguration( CoreLayerContext *obj,
                    const DFBDisplayLayerConfig               *config
)
{
    DFBResult           ret;
    CoreLayerContextSetConfiguration       *args = (CoreLayerContextSetConfiguration*) alloca( sizeof(CoreLayerContextSetConfiguration) );
    CoreLayerContextSetConfigurationReturn *return_args = (CoreLayerContextSetConfigurationReturn*) alloca( sizeof(CoreLayerContextSetConfigurationReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( config != NULL );

    args->config = *config;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetConfiguration, args, sizeof(CoreLayerContextSetConfiguration), return_args, sizeof(CoreLayerContextSetConfigurationReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetConfiguration ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetConfiguration failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetSrcColorKey( CoreLayerContext *obj,
                    const DFBColorKey                         *key
)
{
    DFBResult           ret;
    CoreLayerContextSetSrcColorKey       *args = (CoreLayerContextSetSrcColorKey*) alloca( sizeof(CoreLayerContextSetSrcColorKey) );
    CoreLayerContextSetSrcColorKeyReturn *return_args = (CoreLayerContextSetSrcColorKeyReturn*) alloca( sizeof(CoreLayerContextSetSrcColorKeyReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( key != NULL );

    args->key = *key;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetSrcColorKey, args, sizeof(CoreLayerContextSetSrcColorKey), return_args, sizeof(CoreLayerContextSetSrcColorKeyReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetSrcColorKey ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetSrcColorKey failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetDstColorKey( CoreLayerContext *obj,
                    const DFBColorKey                         *key
)
{
    DFBResult           ret;
    CoreLayerContextSetDstColorKey       *args = (CoreLayerContextSetDstColorKey*) alloca( sizeof(CoreLayerContextSetDstColorKey) );
    CoreLayerContextSetDstColorKeyReturn *return_args = (CoreLayerContextSetDstColorKeyReturn*) alloca( sizeof(CoreLayerContextSetDstColorKeyReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( key != NULL );

    args->key = *key;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetDstColorKey, args, sizeof(CoreLayerContextSetDstColorKey), return_args, sizeof(CoreLayerContextSetDstColorKeyReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetDstColorKey ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetDstColorKey failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetSourceRectangle( CoreLayerContext *obj,
                    const DFBRectangle                        *rectangle
)
{
    DFBResult           ret;
    CoreLayerContextSetSourceRectangle       *args = (CoreLayerContextSetSourceRectangle*) alloca( sizeof(CoreLayerContextSetSourceRectangle) );
    CoreLayerContextSetSourceRectangleReturn *return_args = (CoreLayerContextSetSourceRectangleReturn*) alloca( sizeof(CoreLayerContextSetSourceRectangleReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( rectangle != NULL );

    args->rectangle = *rectangle;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetSourceRectangle, args, sizeof(CoreLayerContextSetSourceRectangle), return_args, sizeof(CoreLayerContextSetSourceRectangleReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetSourceRectangle ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetSourceRectangle failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetScreenLocation( CoreLayerContext *obj,
                    const DFBLocation                         *location
)
{
    DFBResult           ret;
    CoreLayerContextSetScreenLocation       *args = (CoreLayerContextSetScreenLocation*) alloca( sizeof(CoreLayerContextSetScreenLocation) );
    CoreLayerContextSetScreenLocationReturn *return_args = (CoreLayerContextSetScreenLocationReturn*) alloca( sizeof(CoreLayerContextSetScreenLocationReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( location != NULL );

    args->location = *location;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetScreenLocation, args, sizeof(CoreLayerContextSetScreenLocation), return_args, sizeof(CoreLayerContextSetScreenLocationReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetScreenLocation ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetScreenLocation failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetScreenRectangle( CoreLayerContext *obj,
                    const DFBRectangle                        *rectangle
)
{
    DFBResult           ret;
    CoreLayerContextSetScreenRectangle       *args = (CoreLayerContextSetScreenRectangle*) alloca( sizeof(CoreLayerContextSetScreenRectangle) );
    CoreLayerContextSetScreenRectangleReturn *return_args = (CoreLayerContextSetScreenRectangleReturn*) alloca( sizeof(CoreLayerContextSetScreenRectangleReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( rectangle != NULL );

    args->rectangle = *rectangle;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetScreenRectangle, args, sizeof(CoreLayerContextSetScreenRectangle), return_args, sizeof(CoreLayerContextSetScreenRectangleReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetScreenRectangle ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetScreenRectangle failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetScreenPosition( CoreLayerContext *obj,
                    const DFBPoint                            *position
)
{
    DFBResult           ret;
    CoreLayerContextSetScreenPosition       *args = (CoreLayerContextSetScreenPosition*) alloca( sizeof(CoreLayerContextSetScreenPosition) );
    CoreLayerContextSetScreenPositionReturn *return_args = (CoreLayerContextSetScreenPositionReturn*) alloca( sizeof(CoreLayerContextSetScreenPositionReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( position != NULL );

    args->position = *position;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetScreenPosition, args, sizeof(CoreLayerContextSetScreenPosition), return_args, sizeof(CoreLayerContextSetScreenPositionReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetScreenPosition ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetScreenPosition failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetOpacity( CoreLayerContext *obj,
                    u8                                         opacity
)
{
    DFBResult           ret;
    CoreLayerContextSetOpacity       *args = (CoreLayerContextSetOpacity*) alloca( sizeof(CoreLayerContextSetOpacity) );
    CoreLayerContextSetOpacityReturn *return_args = (CoreLayerContextSetOpacityReturn*) alloca( sizeof(CoreLayerContextSetOpacityReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );


    args->opacity = opacity;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetOpacity, args, sizeof(CoreLayerContextSetOpacity), return_args, sizeof(CoreLayerContextSetOpacityReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetOpacity ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetOpacity failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetRotation( CoreLayerContext *obj,
                    s32                                        rotation
)
{
    DFBResult           ret;
    CoreLayerContextSetRotation       *args = (CoreLayerContextSetRotation*) alloca( sizeof(CoreLayerContextSetRotation) );
    CoreLayerContextSetRotationReturn *return_args = (CoreLayerContextSetRotationReturn*) alloca( sizeof(CoreLayerContextSetRotationReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );


    args->rotation = rotation;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetRotation, args, sizeof(CoreLayerContextSetRotation), return_args, sizeof(CoreLayerContextSetRotationReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetRotation ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetRotation failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetColorAdjustment( CoreLayerContext *obj,
                    const DFBColorAdjustment                  *adjustment
)
{
    DFBResult           ret;
    CoreLayerContextSetColorAdjustment       *args = (CoreLayerContextSetColorAdjustment*) alloca( sizeof(CoreLayerContextSetColorAdjustment) );
    CoreLayerContextSetColorAdjustmentReturn *return_args = (CoreLayerContextSetColorAdjustmentReturn*) alloca( sizeof(CoreLayerContextSetColorAdjustmentReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( adjustment != NULL );

    args->adjustment = *adjustment;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetColorAdjustment, args, sizeof(CoreLayerContextSetColorAdjustment), return_args, sizeof(CoreLayerContextSetColorAdjustmentReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetColorAdjustment ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetColorAdjustment failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetFieldParity( CoreLayerContext *obj,
                    u32                                        field
)
{
    DFBResult           ret;
    CoreLayerContextSetFieldParity       *args = (CoreLayerContextSetFieldParity*) alloca( sizeof(CoreLayerContextSetFieldParity) );
    CoreLayerContextSetFieldParityReturn *return_args = (CoreLayerContextSetFieldParityReturn*) alloca( sizeof(CoreLayerContextSetFieldParityReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );


    args->field = field;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetFieldParity, args, sizeof(CoreLayerContextSetFieldParity), return_args, sizeof(CoreLayerContextSetFieldParityReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetFieldParity ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetFieldParity failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__SetClipRegions( CoreLayerContext *obj,
                    const DFBRegion                           *regions,
                    u32                                        num,
                    bool                                       positive
)
{
    DFBResult           ret;
    CoreLayerContextSetClipRegions       *args = (CoreLayerContextSetClipRegions*) alloca( sizeof(CoreLayerContextSetClipRegions) + num * sizeof(DFBRegion) );
    CoreLayerContextSetClipRegionsReturn *return_args = (CoreLayerContextSetClipRegionsReturn*) alloca( sizeof(CoreLayerContextSetClipRegionsReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( regions != NULL );

    args->num = num;
    args->positive = positive;
    direct_memcpy( (char*) (args + 1), regions, num * sizeof(DFBRegion) );

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_SetClipRegions, args, sizeof(CoreLayerContextSetClipRegions) + num * sizeof(DFBRegion), return_args, sizeof(CoreLayerContextSetClipRegionsReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_SetClipRegions ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_SetClipRegions failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__CreateWindow( CoreLayerContext *obj,
                    const DFBWindowDescription                *description,
                    CoreWindow                                *parent,
                    CoreWindow                                *toplevel,
                    CoreWindow                               **ret_window
)
{
    DFBResult           ret;
    CoreWindow *window = NULL;
    CoreLayerContextCreateWindow       *args = (CoreLayerContextCreateWindow*) alloca( sizeof(CoreLayerContextCreateWindow) );
    CoreLayerContextCreateWindowReturn *return_args = (CoreLayerContextCreateWindowReturn*) alloca( sizeof(CoreLayerContextCreateWindowReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( description != NULL );
    D_ASSERT( ret_window != NULL );

    args->description = *description;
  if (parent) {
    args->parent_id = CoreWindow_GetID( parent );
    args->parent_set = true;
  }
  else
    args->parent_set = false;
  if (toplevel) {
    args->toplevel_id = CoreWindow_GetID( toplevel );
    args->toplevel_set = true;
  }
  else
    args->toplevel_set = false;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_CreateWindow, args, sizeof(CoreLayerContextCreateWindow), return_args, sizeof(CoreLayerContextCreateWindowReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_CreateWindow ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_CreateWindow failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreWindow_Catch( core_dfb, return_args->window_id, &window );
    if (ret) {
         D_DERROR( ret, "%s: Catching window by ID %u failed!\n", __FUNCTION__, return_args->window_id );
         return ret;
    }

    *ret_window = window;

    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__FindWindow( CoreLayerContext *obj,
                    DFBWindowID                                window_id,
                    CoreWindow                               **ret_window
)
{
    DFBResult           ret;
    CoreWindow *window = NULL;
    CoreLayerContextFindWindow       *args = (CoreLayerContextFindWindow*) alloca( sizeof(CoreLayerContextFindWindow) );
    CoreLayerContextFindWindowReturn *return_args = (CoreLayerContextFindWindowReturn*) alloca( sizeof(CoreLayerContextFindWindowReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_window != NULL );

    args->window_id = window_id;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_FindWindow, args, sizeof(CoreLayerContextFindWindow), return_args, sizeof(CoreLayerContextFindWindowReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_FindWindow ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_FindWindow failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreWindow_Catch( core_dfb, return_args->window_id, &window );
    if (ret) {
         D_DERROR( ret, "%s: Catching window by ID %u failed!\n", __FUNCTION__, return_args->window_id );
         return ret;
    }

    *ret_window = window;

    return DFB_OK;
}


DFBResult
ILayerContext_Requestor__FindWindowByResourceID( CoreLayerContext *obj,
                    u64                                        resource_id,
                    CoreWindow                               **ret_window
)
{
    DFBResult           ret;
    CoreWindow *window = NULL;
    CoreLayerContextFindWindowByResourceID       *args = (CoreLayerContextFindWindowByResourceID*) alloca( sizeof(CoreLayerContextFindWindowByResourceID) );
    CoreLayerContextFindWindowByResourceIDReturn *return_args = (CoreLayerContextFindWindowByResourceIDReturn*) alloca( sizeof(CoreLayerContextFindWindowByResourceIDReturn) );

    D_DEBUG_AT( DirectFB_CoreLayerContext, "ILayerContext_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_window != NULL );

    args->resource_id = resource_id;

    ret = (DFBResult) CoreLayerContext_Call( obj, FCEF_NONE, _CoreLayerContext_FindWindowByResourceID, args, sizeof(CoreLayerContextFindWindowByResourceID), return_args, sizeof(CoreLayerContextFindWindowByResourceIDReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreLayerContext_Call( CoreLayerContext_FindWindowByResourceID ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreLayerContext_FindWindowByResourceID failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    ret = (DFBResult) CoreWindow_Catch( core_dfb, return_args->window_id, &window );
    if (ret) {
         D_DERROR( ret, "%s: Catching window by ID %u failed!\n", __FUNCTION__, return_args->window_id );
         return ret;
    }

    *ret_window = window;

    return DFB_OK;
}

/*********************************************************************************************************************/

static DFBResult
__CoreLayerContextDispatch__Dispatch( CoreLayerContext *obj,
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
        case _CoreLayerContext_GetPrimaryRegion: {
    CoreLayerRegion *region = NULL;
            D_UNUSED
            CoreLayerContextGetPrimaryRegion       *args        = (CoreLayerContextGetPrimaryRegion *) ptr;
            CoreLayerContextGetPrimaryRegionReturn *return_args = (CoreLayerContextGetPrimaryRegionReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_GetPrimaryRegion\n" );

            return_args->result = ILayerContext_Real__GetPrimaryRegion( obj, args->create, &region );
            if (return_args->result == DFB_OK) {
                CoreLayerRegion_Throw( region, caller, &return_args->region_id );
            }

            *ret_length = sizeof(CoreLayerContextGetPrimaryRegionReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_TestConfiguration: {
            D_UNUSED
            CoreLayerContextTestConfiguration       *args        = (CoreLayerContextTestConfiguration *) ptr;
            CoreLayerContextTestConfigurationReturn *return_args = (CoreLayerContextTestConfigurationReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_TestConfiguration\n" );

            return_args->result = ILayerContext_Real__TestConfiguration( obj, &args->config, &return_args->failed );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextTestConfigurationReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetConfiguration: {
            D_UNUSED
            CoreLayerContextSetConfiguration       *args        = (CoreLayerContextSetConfiguration *) ptr;
            CoreLayerContextSetConfigurationReturn *return_args = (CoreLayerContextSetConfigurationReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetConfiguration\n" );

            return_args->result = ILayerContext_Real__SetConfiguration( obj, &args->config );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetConfigurationReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetSrcColorKey: {
            D_UNUSED
            CoreLayerContextSetSrcColorKey       *args        = (CoreLayerContextSetSrcColorKey *) ptr;
            CoreLayerContextSetSrcColorKeyReturn *return_args = (CoreLayerContextSetSrcColorKeyReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetSrcColorKey\n" );

            return_args->result = ILayerContext_Real__SetSrcColorKey( obj, &args->key );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetSrcColorKeyReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetDstColorKey: {
            D_UNUSED
            CoreLayerContextSetDstColorKey       *args        = (CoreLayerContextSetDstColorKey *) ptr;
            CoreLayerContextSetDstColorKeyReturn *return_args = (CoreLayerContextSetDstColorKeyReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetDstColorKey\n" );

            return_args->result = ILayerContext_Real__SetDstColorKey( obj, &args->key );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetDstColorKeyReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetSourceRectangle: {
            D_UNUSED
            CoreLayerContextSetSourceRectangle       *args        = (CoreLayerContextSetSourceRectangle *) ptr;
            CoreLayerContextSetSourceRectangleReturn *return_args = (CoreLayerContextSetSourceRectangleReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetSourceRectangle\n" );

            return_args->result = ILayerContext_Real__SetSourceRectangle( obj, &args->rectangle );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetSourceRectangleReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetScreenLocation: {
            D_UNUSED
            CoreLayerContextSetScreenLocation       *args        = (CoreLayerContextSetScreenLocation *) ptr;
            CoreLayerContextSetScreenLocationReturn *return_args = (CoreLayerContextSetScreenLocationReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetScreenLocation\n" );

            return_args->result = ILayerContext_Real__SetScreenLocation( obj, &args->location );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetScreenLocationReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetScreenRectangle: {
            D_UNUSED
            CoreLayerContextSetScreenRectangle       *args        = (CoreLayerContextSetScreenRectangle *) ptr;
            CoreLayerContextSetScreenRectangleReturn *return_args = (CoreLayerContextSetScreenRectangleReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetScreenRectangle\n" );

            return_args->result = ILayerContext_Real__SetScreenRectangle( obj, &args->rectangle );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetScreenRectangleReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetScreenPosition: {
            D_UNUSED
            CoreLayerContextSetScreenPosition       *args        = (CoreLayerContextSetScreenPosition *) ptr;
            CoreLayerContextSetScreenPositionReturn *return_args = (CoreLayerContextSetScreenPositionReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetScreenPosition\n" );

            return_args->result = ILayerContext_Real__SetScreenPosition( obj, &args->position );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetScreenPositionReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetOpacity: {
            D_UNUSED
            CoreLayerContextSetOpacity       *args        = (CoreLayerContextSetOpacity *) ptr;
            CoreLayerContextSetOpacityReturn *return_args = (CoreLayerContextSetOpacityReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetOpacity\n" );

            return_args->result = ILayerContext_Real__SetOpacity( obj, args->opacity );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetOpacityReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetRotation: {
            D_UNUSED
            CoreLayerContextSetRotation       *args        = (CoreLayerContextSetRotation *) ptr;
            CoreLayerContextSetRotationReturn *return_args = (CoreLayerContextSetRotationReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetRotation\n" );

            return_args->result = ILayerContext_Real__SetRotation( obj, args->rotation );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetRotationReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetColorAdjustment: {
            D_UNUSED
            CoreLayerContextSetColorAdjustment       *args        = (CoreLayerContextSetColorAdjustment *) ptr;
            CoreLayerContextSetColorAdjustmentReturn *return_args = (CoreLayerContextSetColorAdjustmentReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetColorAdjustment\n" );

            return_args->result = ILayerContext_Real__SetColorAdjustment( obj, &args->adjustment );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetColorAdjustmentReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetFieldParity: {
            D_UNUSED
            CoreLayerContextSetFieldParity       *args        = (CoreLayerContextSetFieldParity *) ptr;
            CoreLayerContextSetFieldParityReturn *return_args = (CoreLayerContextSetFieldParityReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetFieldParity\n" );

            return_args->result = ILayerContext_Real__SetFieldParity( obj, args->field );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetFieldParityReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_SetClipRegions: {
            D_UNUSED
            CoreLayerContextSetClipRegions       *args        = (CoreLayerContextSetClipRegions *) ptr;
            CoreLayerContextSetClipRegionsReturn *return_args = (CoreLayerContextSetClipRegionsReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_SetClipRegions\n" );

            return_args->result = ILayerContext_Real__SetClipRegions( obj, (DFBRegion*) ((char*)(args + 1)), args->num, args->positive );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreLayerContextSetClipRegionsReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_CreateWindow: {
    CoreWindow *parent = NULL;
    CoreWindow *toplevel = NULL;
    CoreWindow *window = NULL;
            D_UNUSED
            CoreLayerContextCreateWindow       *args        = (CoreLayerContextCreateWindow *) ptr;
            CoreLayerContextCreateWindowReturn *return_args = (CoreLayerContextCreateWindowReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_CreateWindow\n" );

            if (args->parent_set) {
                ret = (DFBResult) CoreWindow_Lookup( core_dfb, args->parent_id, caller, &parent );
                if (ret) {
                     D_DERROR( ret, "%s: Looking up parent by ID %u failed!\n", __FUNCTION__, args->parent_id );
                     return_args->result = ret;
                     return DFB_OK;
                }
            }

            if (args->toplevel_set) {
                ret = (DFBResult) CoreWindow_Lookup( core_dfb, args->toplevel_id, caller, &toplevel );
                if (ret) {
                     D_DERROR( ret, "%s: Looking up toplevel by ID %u failed!\n", __FUNCTION__, args->toplevel_id );
                     return_args->result = ret;
                     return DFB_OK;
                }
            }

            return_args->result = ILayerContext_Real__CreateWindow( obj, &args->description, args->parent_set ? parent : NULL, args->toplevel_set ? toplevel : NULL, &window );
            if (return_args->result == DFB_OK) {
                CoreWindow_Throw( window, caller, &return_args->window_id );
            }

            *ret_length = sizeof(CoreLayerContextCreateWindowReturn);

            if (parent)
                CoreWindow_Unref( parent );

            if (toplevel)
                CoreWindow_Unref( toplevel );

            return DFB_OK;
        }

        case _CoreLayerContext_FindWindow: {
    CoreWindow *window = NULL;
            D_UNUSED
            CoreLayerContextFindWindow       *args        = (CoreLayerContextFindWindow *) ptr;
            CoreLayerContextFindWindowReturn *return_args = (CoreLayerContextFindWindowReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_FindWindow\n" );

            return_args->result = ILayerContext_Real__FindWindow( obj, args->window_id, &window );
            if (return_args->result == DFB_OK) {
                CoreWindow_Throw( window, caller, &return_args->window_id );
            }

            *ret_length = sizeof(CoreLayerContextFindWindowReturn);

            return DFB_OK;
        }

        case _CoreLayerContext_FindWindowByResourceID: {
    CoreWindow *window = NULL;
            D_UNUSED
            CoreLayerContextFindWindowByResourceID       *args        = (CoreLayerContextFindWindowByResourceID *) ptr;
            CoreLayerContextFindWindowByResourceIDReturn *return_args = (CoreLayerContextFindWindowByResourceIDReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreLayerContext, "=-> CoreLayerContext_FindWindowByResourceID\n" );

            return_args->result = ILayerContext_Real__FindWindowByResourceID( obj, args->resource_id, &window );
            if (return_args->result == DFB_OK) {
                CoreWindow_Throw( window, caller, &return_args->window_id );
            }

            *ret_length = sizeof(CoreLayerContextFindWindowByResourceIDReturn);

            return DFB_OK;
        }

    }

    return DFB_NOSUCHMETHOD;
}
/*********************************************************************************************************************/

DFBResult
CoreLayerContextDispatch__Dispatch( CoreLayerContext *obj,
                                FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length )
{
    DFBResult ret;

    D_DEBUG_AT( DirectFB_CoreLayerContext, "CoreLayerContextDispatch::%s( %p )\n", __FUNCTION__, obj );

    Core_PushIdentity( caller );

    ret = __CoreLayerContextDispatch__Dispatch( obj, caller, method, ptr, length, ret_ptr, ret_size, ret_length );

    Core_PopIdentity();

    return ret;
}
