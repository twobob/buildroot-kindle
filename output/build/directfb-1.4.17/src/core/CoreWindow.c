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

#include "CoreWindow.h"

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <fusion/conf.h>

#include <core/core.h>

#include <core/CoreDFB_CallMode.h>

D_DEBUG_DOMAIN( DirectFB_CoreWindow, "DirectFB/CoreWindow", "DirectFB CoreWindow" );

/*********************************************************************************************************************/

DFBResult
CoreWindow_Repaint(
                    CoreWindow                                *obj,
                    const DFBRegion                           *left,
                    const DFBRegion                           *right,
                    DFBSurfaceFlipFlags                        flags
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__Repaint( obj, left, right, flags );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__Repaint( obj, left, right, flags );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_BeginUpdates(
                    CoreWindow                                *obj,
                    const DFBRegion                           *update
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__BeginUpdates( obj, update );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__BeginUpdates( obj, update );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_Restack(
                    CoreWindow                                *obj,
                    CoreWindow                                *relative,
                    int                                        relation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__Restack( obj, relative, relation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__Restack( obj, relative, relation );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetConfig(
                    CoreWindow                                *obj,
                    const CoreWindowConfig                    *config,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys,
                    CoreWindow                                *parent,
                    CoreWindowConfigFlags                      flags
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetConfig( obj, config, keys, num_keys, parent, flags );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetConfig( obj, config, keys, num_keys, parent, flags );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_Bind(
                    CoreWindow                                *obj,
                    CoreWindow                                *source,
                    int                                        x,
                    int                                        y
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__Bind( obj, source, x, y );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__Bind( obj, source, x, y );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_Unbind(
                    CoreWindow                                *obj,
                    CoreWindow                                *source
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__Unbind( obj, source );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__Unbind( obj, source );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_RequestFocus(
                    CoreWindow                                *obj

)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__RequestFocus( obj );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__RequestFocus( obj );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_ChangeGrab(
                    CoreWindow                                *obj,
                    CoreWMGrabTarget                           target,
                    bool                                       grab
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__ChangeGrab( obj, target, grab );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__ChangeGrab( obj, target, grab );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_GrabKey(
                    CoreWindow                                *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__GrabKey( obj, symbol, modifiers );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__GrabKey( obj, symbol, modifiers );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_UngrabKey(
                    CoreWindow                                *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__UngrabKey( obj, symbol, modifiers );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__UngrabKey( obj, symbol, modifiers );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_Move(
                    CoreWindow                                *obj,
                    int                                        dx,
                    int                                        dy
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__Move( obj, dx, dy );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__Move( obj, dx, dy );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_MoveTo(
                    CoreWindow                                *obj,
                    int                                        x,
                    int                                        y
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__MoveTo( obj, x, y );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__MoveTo( obj, x, y );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_Resize(
                    CoreWindow                                *obj,
                    int                                        width,
                    int                                        height
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__Resize( obj, width, height );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__Resize( obj, width, height );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_Destroy(
                    CoreWindow                                *obj

)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__Destroy( obj );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__Destroy( obj );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetCursorPosition(
                    CoreWindow                                *obj,
                    int                                        x,
                    int                                        y
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetCursorPosition( obj, x, y );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetCursorPosition( obj, x, y );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_ChangeEvents(
                    CoreWindow                                *obj,
                    DFBWindowEventType                         disable,
                    DFBWindowEventType                         enable
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__ChangeEvents( obj, disable, enable );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__ChangeEvents( obj, disable, enable );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_ChangeOptions(
                    CoreWindow                                *obj,
                    DFBWindowOptions                           disable,
                    DFBWindowOptions                           enable
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__ChangeOptions( obj, disable, enable );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__ChangeOptions( obj, disable, enable );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetColor(
                    CoreWindow                                *obj,
                    const DFBColor                            *color
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetColor( obj, color );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetColor( obj, color );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetColorKey(
                    CoreWindow                                *obj,
                    u32                                        key
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetOpaque(
                    CoreWindow                                *obj,
                    const DFBRegion                           *opaque
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetOpaque( obj, opaque );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetOpaque( obj, opaque );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetOpacity(
                    CoreWindow                                *obj,
                    u8                                         opacity
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetOpacity( obj, opacity );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetOpacity( obj, opacity );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetStacking(
                    CoreWindow                                *obj,
                    DFBWindowStackingClass                     stacking
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetStacking( obj, stacking );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetStacking( obj, stacking );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetBounds(
                    CoreWindow                                *obj,
                    const DFBRectangle                        *bounds
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetBounds( obj, bounds );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetBounds( obj, bounds );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetKeySelection(
                    CoreWindow                                *obj,
                    DFBWindowKeySelection                      selection,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetKeySelection( obj, selection, keys, num_keys );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetKeySelection( obj, selection, keys, num_keys );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetRotation(
                    CoreWindow                                *obj,
                    int                                        rotation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetRotation( obj, rotation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetRotation( obj, rotation );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_GetSurface(
                    CoreWindow                                *obj,
                    CoreSurface                              **ret_surface
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__GetSurface( obj, ret_surface );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__GetSurface( obj, ret_surface );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreWindow_SetCursorShape(
                    CoreWindow                                *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IWindow_Real__SetCursorShape( obj, shape, hotspot );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IWindow_Requestor__SetCursorShape( obj, shape, hotspot );
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
CoreWindow_Dispatch( int           caller,   /* fusion id of the caller */
                     int           call_arg, /* optional call parameter */
                     void         *ptr, /* optional call parameter */
                     unsigned int  length,
                     void         *ctx,      /* optional handler context */
                     unsigned int  serial,
                     void         *ret_ptr,
                     unsigned int  ret_size,
                     unsigned int *ret_length )
{
    CoreWindow *obj = (CoreWindow*) ctx;
    CoreWindowDispatch__Dispatch( obj, caller, call_arg, ptr, length, ret_ptr, ret_size, ret_length );

    return FCHR_RETURN;
}

void CoreWindow_Init_Dispatch(
                    CoreDFB              *core,
                    CoreWindow           *obj,
                    FusionCall           *call
)
{
    fusion_call_init3( call, CoreWindow_Dispatch, obj, core->world );
}

void  CoreWindow_Deinit_Dispatch(
                    FusionCall           *call
)
{
     fusion_call_destroy( call );
}

/*********************************************************************************************************************/


DFBResult
IWindow_Requestor__Repaint( CoreWindow *obj,
                    const DFBRegion                           *left,
                    const DFBRegion                           *right,
                    DFBSurfaceFlipFlags                        flags
)
{
    DFBResult           ret;
    CoreWindowRepaint       *args = (CoreWindowRepaint*) alloca( sizeof(CoreWindowRepaint) );
    CoreWindowRepaintReturn *return_args = (CoreWindowRepaintReturn*) alloca( sizeof(CoreWindowRepaintReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( left != NULL );
    D_ASSERT( right != NULL );

    args->left = *left;
    args->right = *right;
    args->flags = flags;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_Repaint, args, sizeof(CoreWindowRepaint), return_args, sizeof(CoreWindowRepaintReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_Repaint ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_Repaint failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__BeginUpdates( CoreWindow *obj,
                    const DFBRegion                           *update
)
{
    DFBResult           ret;
    CoreWindowBeginUpdates       *args = (CoreWindowBeginUpdates*) alloca( sizeof(CoreWindowBeginUpdates) );
    CoreWindowBeginUpdatesReturn *return_args = (CoreWindowBeginUpdatesReturn*) alloca( sizeof(CoreWindowBeginUpdatesReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


  if (update) {
    args->update = *update;
    args->update_set = true;
  }
  else
    args->update_set = false;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_BeginUpdates, args, sizeof(CoreWindowBeginUpdates), return_args, sizeof(CoreWindowBeginUpdatesReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_BeginUpdates ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_BeginUpdates failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__Restack( CoreWindow *obj,
                    CoreWindow                                *relative,
                    int                                        relation
)
{
    DFBResult           ret;
    CoreWindowRestack       *args = (CoreWindowRestack*) alloca( sizeof(CoreWindowRestack) );
    CoreWindowRestackReturn *return_args = (CoreWindowRestackReturn*) alloca( sizeof(CoreWindowRestackReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


  if (relative) {
    args->relative_id = CoreWindow_GetID( relative );
    args->relative_set = true;
  }
  else
    args->relative_set = false;
    args->relation = relation;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_Restack, args, sizeof(CoreWindowRestack), return_args, sizeof(CoreWindowRestackReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_Restack ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_Restack failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetConfig( CoreWindow *obj,
                    const CoreWindowConfig                    *config,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys,
                    CoreWindow                                *parent,
                    CoreWindowConfigFlags                      flags
)
{
    DFBResult           ret;
    CoreWindowSetConfig       *args = (CoreWindowSetConfig*) alloca( sizeof(CoreWindowSetConfig) + num_keys * sizeof(DFBInputDeviceKeySymbol) );
    CoreWindowSetConfigReturn *return_args = (CoreWindowSetConfigReturn*) alloca( sizeof(CoreWindowSetConfigReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( config != NULL );

    args->config = *config;
    args->num_keys = num_keys;
  if (parent) {
    args->parent_id = CoreWindow_GetID( parent );
    args->parent_set = true;
  }
  else
    args->parent_set = false;
    args->flags = flags;
  if (keys) {
    direct_memcpy( (char*) (args + 1), keys, num_keys * sizeof(DFBInputDeviceKeySymbol) );
    args->keys_set = true;
  }
  else {
    args->keys_set = false;
    keys = 0;
  }

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetConfig, args, sizeof(CoreWindowSetConfig) + num_keys * sizeof(DFBInputDeviceKeySymbol), return_args, sizeof(CoreWindowSetConfigReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetConfig ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetConfig failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__Bind( CoreWindow *obj,
                    CoreWindow                                *source,
                    int                                        x,
                    int                                        y
)
{
    DFBResult           ret;
    CoreWindowBind       *args = (CoreWindowBind*) alloca( sizeof(CoreWindowBind) );
    CoreWindowBindReturn *return_args = (CoreWindowBindReturn*) alloca( sizeof(CoreWindowBindReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( source != NULL );

    args->source_id = CoreWindow_GetID( source );
    args->x = x;
    args->y = y;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_Bind, args, sizeof(CoreWindowBind), return_args, sizeof(CoreWindowBindReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_Bind ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_Bind failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__Unbind( CoreWindow *obj,
                    CoreWindow                                *source
)
{
    DFBResult           ret;
    CoreWindowUnbind       *args = (CoreWindowUnbind*) alloca( sizeof(CoreWindowUnbind) );
    CoreWindowUnbindReturn *return_args = (CoreWindowUnbindReturn*) alloca( sizeof(CoreWindowUnbindReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( source != NULL );

    args->source_id = CoreWindow_GetID( source );

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_Unbind, args, sizeof(CoreWindowUnbind), return_args, sizeof(CoreWindowUnbindReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_Unbind ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_Unbind failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__RequestFocus( CoreWindow *obj

)
{
    DFBResult           ret;
    CoreWindowRequestFocus       *args = (CoreWindowRequestFocus*) alloca( sizeof(CoreWindowRequestFocus) );
    CoreWindowRequestFocusReturn *return_args = (CoreWindowRequestFocusReturn*) alloca( sizeof(CoreWindowRequestFocusReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );



    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_RequestFocus, args, sizeof(CoreWindowRequestFocus), return_args, sizeof(CoreWindowRequestFocusReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_RequestFocus ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_RequestFocus failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__ChangeGrab( CoreWindow *obj,
                    CoreWMGrabTarget                           target,
                    bool                                       grab
)
{
    DFBResult           ret;
    CoreWindowChangeGrab       *args = (CoreWindowChangeGrab*) alloca( sizeof(CoreWindowChangeGrab) );
    CoreWindowChangeGrabReturn *return_args = (CoreWindowChangeGrabReturn*) alloca( sizeof(CoreWindowChangeGrabReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->target = target;
    args->grab = grab;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_ChangeGrab, args, sizeof(CoreWindowChangeGrab), return_args, sizeof(CoreWindowChangeGrabReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_ChangeGrab ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_ChangeGrab failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__GrabKey( CoreWindow *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers
)
{
    DFBResult           ret;
    CoreWindowGrabKey       *args = (CoreWindowGrabKey*) alloca( sizeof(CoreWindowGrabKey) );
    CoreWindowGrabKeyReturn *return_args = (CoreWindowGrabKeyReturn*) alloca( sizeof(CoreWindowGrabKeyReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->symbol = symbol;
    args->modifiers = modifiers;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_GrabKey, args, sizeof(CoreWindowGrabKey), return_args, sizeof(CoreWindowGrabKeyReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_GrabKey ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_GrabKey failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__UngrabKey( CoreWindow *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers
)
{
    DFBResult           ret;
    CoreWindowUngrabKey       *args = (CoreWindowUngrabKey*) alloca( sizeof(CoreWindowUngrabKey) );
    CoreWindowUngrabKeyReturn *return_args = (CoreWindowUngrabKeyReturn*) alloca( sizeof(CoreWindowUngrabKeyReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->symbol = symbol;
    args->modifiers = modifiers;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_UngrabKey, args, sizeof(CoreWindowUngrabKey), return_args, sizeof(CoreWindowUngrabKeyReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_UngrabKey ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_UngrabKey failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__Move( CoreWindow *obj,
                    int                                        dx,
                    int                                        dy
)
{
    DFBResult           ret;
    CoreWindowMove       *args = (CoreWindowMove*) alloca( sizeof(CoreWindowMove) );
    CoreWindowMoveReturn *return_args = (CoreWindowMoveReturn*) alloca( sizeof(CoreWindowMoveReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->dx = dx;
    args->dy = dy;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_Move, args, sizeof(CoreWindowMove), return_args, sizeof(CoreWindowMoveReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_Move ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_Move failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__MoveTo( CoreWindow *obj,
                    int                                        x,
                    int                                        y
)
{
    DFBResult           ret;
    CoreWindowMoveTo       *args = (CoreWindowMoveTo*) alloca( sizeof(CoreWindowMoveTo) );
    CoreWindowMoveToReturn *return_args = (CoreWindowMoveToReturn*) alloca( sizeof(CoreWindowMoveToReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->x = x;
    args->y = y;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_MoveTo, args, sizeof(CoreWindowMoveTo), return_args, sizeof(CoreWindowMoveToReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_MoveTo ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_MoveTo failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__Resize( CoreWindow *obj,
                    int                                        width,
                    int                                        height
)
{
    DFBResult           ret;
    CoreWindowResize       *args = (CoreWindowResize*) alloca( sizeof(CoreWindowResize) );
    CoreWindowResizeReturn *return_args = (CoreWindowResizeReturn*) alloca( sizeof(CoreWindowResizeReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->width = width;
    args->height = height;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_Resize, args, sizeof(CoreWindowResize), return_args, sizeof(CoreWindowResizeReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_Resize ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_Resize failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__Destroy( CoreWindow *obj

)
{
    DFBResult           ret;
    CoreWindowDestroy       *args = (CoreWindowDestroy*) alloca( sizeof(CoreWindowDestroy) );
    CoreWindowDestroyReturn *return_args = (CoreWindowDestroyReturn*) alloca( sizeof(CoreWindowDestroyReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );



    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_Destroy, args, sizeof(CoreWindowDestroy), return_args, sizeof(CoreWindowDestroyReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_Destroy ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_Destroy failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetCursorPosition( CoreWindow *obj,
                    int                                        x,
                    int                                        y
)
{
    DFBResult           ret;
    CoreWindowSetCursorPosition       *args = (CoreWindowSetCursorPosition*) alloca( sizeof(CoreWindowSetCursorPosition) );
    CoreWindowSetCursorPositionReturn *return_args = (CoreWindowSetCursorPositionReturn*) alloca( sizeof(CoreWindowSetCursorPositionReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->x = x;
    args->y = y;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetCursorPosition, args, sizeof(CoreWindowSetCursorPosition), return_args, sizeof(CoreWindowSetCursorPositionReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetCursorPosition ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetCursorPosition failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__ChangeEvents( CoreWindow *obj,
                    DFBWindowEventType                         disable,
                    DFBWindowEventType                         enable
)
{
    DFBResult           ret;
    CoreWindowChangeEvents       *args = (CoreWindowChangeEvents*) alloca( sizeof(CoreWindowChangeEvents) );
    CoreWindowChangeEventsReturn *return_args = (CoreWindowChangeEventsReturn*) alloca( sizeof(CoreWindowChangeEventsReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->disable = disable;
    args->enable = enable;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_ChangeEvents, args, sizeof(CoreWindowChangeEvents), return_args, sizeof(CoreWindowChangeEventsReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_ChangeEvents ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_ChangeEvents failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__ChangeOptions( CoreWindow *obj,
                    DFBWindowOptions                           disable,
                    DFBWindowOptions                           enable
)
{
    DFBResult           ret;
    CoreWindowChangeOptions       *args = (CoreWindowChangeOptions*) alloca( sizeof(CoreWindowChangeOptions) );
    CoreWindowChangeOptionsReturn *return_args = (CoreWindowChangeOptionsReturn*) alloca( sizeof(CoreWindowChangeOptionsReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->disable = disable;
    args->enable = enable;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_ChangeOptions, args, sizeof(CoreWindowChangeOptions), return_args, sizeof(CoreWindowChangeOptionsReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_ChangeOptions ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_ChangeOptions failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetColor( CoreWindow *obj,
                    const DFBColor                            *color
)
{
    DFBResult           ret;
    CoreWindowSetColor       *args = (CoreWindowSetColor*) alloca( sizeof(CoreWindowSetColor) );
    CoreWindowSetColorReturn *return_args = (CoreWindowSetColorReturn*) alloca( sizeof(CoreWindowSetColorReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( color != NULL );

    args->color = *color;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetColor, args, sizeof(CoreWindowSetColor), return_args, sizeof(CoreWindowSetColorReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetColor ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetColor failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetColorKey( CoreWindow *obj,
                    u32                                        key
)
{
    DFBResult           ret;
    CoreWindowSetColorKey       *args = (CoreWindowSetColorKey*) alloca( sizeof(CoreWindowSetColorKey) );
    CoreWindowSetColorKeyReturn *return_args = (CoreWindowSetColorKeyReturn*) alloca( sizeof(CoreWindowSetColorKeyReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->key = key;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetColorKey, args, sizeof(CoreWindowSetColorKey), return_args, sizeof(CoreWindowSetColorKeyReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetColorKey ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetColorKey failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetOpaque( CoreWindow *obj,
                    const DFBRegion                           *opaque
)
{
    DFBResult           ret;
    CoreWindowSetOpaque       *args = (CoreWindowSetOpaque*) alloca( sizeof(CoreWindowSetOpaque) );
    CoreWindowSetOpaqueReturn *return_args = (CoreWindowSetOpaqueReturn*) alloca( sizeof(CoreWindowSetOpaqueReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( opaque != NULL );

    args->opaque = *opaque;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetOpaque, args, sizeof(CoreWindowSetOpaque), return_args, sizeof(CoreWindowSetOpaqueReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetOpaque ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetOpaque failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetOpacity( CoreWindow *obj,
                    u8                                         opacity
)
{
    DFBResult           ret;
    CoreWindowSetOpacity       *args = (CoreWindowSetOpacity*) alloca( sizeof(CoreWindowSetOpacity) );
    CoreWindowSetOpacityReturn *return_args = (CoreWindowSetOpacityReturn*) alloca( sizeof(CoreWindowSetOpacityReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->opacity = opacity;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetOpacity, args, sizeof(CoreWindowSetOpacity), return_args, sizeof(CoreWindowSetOpacityReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetOpacity ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetOpacity failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetStacking( CoreWindow *obj,
                    DFBWindowStackingClass                     stacking
)
{
    DFBResult           ret;
    CoreWindowSetStacking       *args = (CoreWindowSetStacking*) alloca( sizeof(CoreWindowSetStacking) );
    CoreWindowSetStackingReturn *return_args = (CoreWindowSetStackingReturn*) alloca( sizeof(CoreWindowSetStackingReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->stacking = stacking;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetStacking, args, sizeof(CoreWindowSetStacking), return_args, sizeof(CoreWindowSetStackingReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetStacking ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetStacking failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetBounds( CoreWindow *obj,
                    const DFBRectangle                        *bounds
)
{
    DFBResult           ret;
    CoreWindowSetBounds       *args = (CoreWindowSetBounds*) alloca( sizeof(CoreWindowSetBounds) );
    CoreWindowSetBoundsReturn *return_args = (CoreWindowSetBoundsReturn*) alloca( sizeof(CoreWindowSetBoundsReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( bounds != NULL );

    args->bounds = *bounds;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetBounds, args, sizeof(CoreWindowSetBounds), return_args, sizeof(CoreWindowSetBoundsReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetBounds ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetBounds failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetKeySelection( CoreWindow *obj,
                    DFBWindowKeySelection                      selection,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys
)
{
    DFBResult           ret;
    CoreWindowSetKeySelection       *args = (CoreWindowSetKeySelection*) alloca( sizeof(CoreWindowSetKeySelection) + num_keys * sizeof(DFBInputDeviceKeySymbol) );
    CoreWindowSetKeySelectionReturn *return_args = (CoreWindowSetKeySelectionReturn*) alloca( sizeof(CoreWindowSetKeySelectionReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->selection = selection;
    args->num_keys = num_keys;
  if (keys) {
    direct_memcpy( (char*) (args + 1), keys, num_keys * sizeof(DFBInputDeviceKeySymbol) );
    args->keys_set = true;
  }
  else {
    args->keys_set = false;
    keys = 0;
  }

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetKeySelection, args, sizeof(CoreWindowSetKeySelection) + num_keys * sizeof(DFBInputDeviceKeySymbol), return_args, sizeof(CoreWindowSetKeySelectionReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetKeySelection ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetKeySelection failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__SetRotation( CoreWindow *obj,
                    int                                        rotation
)
{
    DFBResult           ret;
    CoreWindowSetRotation       *args = (CoreWindowSetRotation*) alloca( sizeof(CoreWindowSetRotation) );
    CoreWindowSetRotationReturn *return_args = (CoreWindowSetRotationReturn*) alloca( sizeof(CoreWindowSetRotationReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );


    args->rotation = rotation;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetRotation, args, sizeof(CoreWindowSetRotation), return_args, sizeof(CoreWindowSetRotationReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetRotation ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetRotation failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IWindow_Requestor__GetSurface( CoreWindow *obj,
                    CoreSurface                              **ret_surface
)
{
    DFBResult           ret;
    CoreSurface *surface = NULL;
    CoreWindowGetSurface       *args = (CoreWindowGetSurface*) alloca( sizeof(CoreWindowGetSurface) );
    CoreWindowGetSurfaceReturn *return_args = (CoreWindowGetSurfaceReturn*) alloca( sizeof(CoreWindowGetSurfaceReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_surface != NULL );


    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_GetSurface, args, sizeof(CoreWindowGetSurface), return_args, sizeof(CoreWindowGetSurfaceReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_GetSurface ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_GetSurface failed!\n", __FUNCTION__ );*/
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
IWindow_Requestor__SetCursorShape( CoreWindow *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot
)
{
    DFBResult           ret;
    CoreWindowSetCursorShape       *args = (CoreWindowSetCursorShape*) alloca( sizeof(CoreWindowSetCursorShape) );
    CoreWindowSetCursorShapeReturn *return_args = (CoreWindowSetCursorShapeReturn*) alloca( sizeof(CoreWindowSetCursorShapeReturn) );

    D_DEBUG_AT( DirectFB_CoreWindow, "IWindow_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( hotspot != NULL );

  if (shape) {
    args->shape_id = CoreSurface_GetID( shape );
    args->shape_set = true;
  }
  else
    args->shape_set = false;
    args->hotspot = *hotspot;

    ret = (DFBResult) CoreWindow_Call( obj, FCEF_NONE, _CoreWindow_SetCursorShape, args, sizeof(CoreWindowSetCursorShape), return_args, sizeof(CoreWindowSetCursorShapeReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreWindow_Call( CoreWindow_SetCursorShape ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: CoreWindow_SetCursorShape failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}

/*********************************************************************************************************************/

static DFBResult
__CoreWindowDispatch__Dispatch( CoreWindow *obj,
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
        case _CoreWindow_Repaint: {
            D_UNUSED
            CoreWindowRepaint       *args        = (CoreWindowRepaint *) ptr;
            CoreWindowRepaintReturn *return_args = (CoreWindowRepaintReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_Repaint\n" );

            return_args->result = IWindow_Real__Repaint( obj, &args->left, &args->right, args->flags );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowRepaintReturn);

            return DFB_OK;
        }

        case _CoreWindow_BeginUpdates: {
            D_UNUSED
            CoreWindowBeginUpdates       *args        = (CoreWindowBeginUpdates *) ptr;
            CoreWindowBeginUpdatesReturn *return_args = (CoreWindowBeginUpdatesReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_BeginUpdates\n" );

            return_args->result = IWindow_Real__BeginUpdates( obj, args->update_set ? &args->update : NULL );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowBeginUpdatesReturn);

            return DFB_OK;
        }

        case _CoreWindow_Restack: {
    CoreWindow *relative = NULL;
            D_UNUSED
            CoreWindowRestack       *args        = (CoreWindowRestack *) ptr;
            CoreWindowRestackReturn *return_args = (CoreWindowRestackReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_Restack\n" );

            if (args->relative_set) {
                ret = (DFBResult) CoreWindow_Lookup( core_dfb, args->relative_id, caller, &relative );
                if (ret) {
                     D_DERROR( ret, "%s: Looking up relative by ID %u failed!\n", __FUNCTION__, args->relative_id );
                     return_args->result = ret;
                     return DFB_OK;
                }
            }

            return_args->result = IWindow_Real__Restack( obj, args->relative_set ? relative : NULL, args->relation );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowRestackReturn);

            if (relative)
                CoreWindow_Unref( relative );

            return DFB_OK;
        }

        case _CoreWindow_SetConfig: {
    CoreWindow *parent = NULL;
            D_UNUSED
            CoreWindowSetConfig       *args        = (CoreWindowSetConfig *) ptr;
            CoreWindowSetConfigReturn *return_args = (CoreWindowSetConfigReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetConfig\n" );

            if (args->parent_set) {
                ret = (DFBResult) CoreWindow_Lookup( core_dfb, args->parent_id, caller, &parent );
                if (ret) {
                     D_DERROR( ret, "%s: Looking up parent by ID %u failed!\n", __FUNCTION__, args->parent_id );
                     return_args->result = ret;
                     return DFB_OK;
                }
            }

            return_args->result = IWindow_Real__SetConfig( obj, &args->config, args->keys_set ? (DFBInputDeviceKeySymbol*) ((char*)(args + 1)) : NULL, args->num_keys, args->parent_set ? parent : NULL, args->flags );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetConfigReturn);

            if (parent)
                CoreWindow_Unref( parent );

            return DFB_OK;
        }

        case _CoreWindow_Bind: {
    CoreWindow *source = NULL;
            D_UNUSED
            CoreWindowBind       *args        = (CoreWindowBind *) ptr;
            CoreWindowBindReturn *return_args = (CoreWindowBindReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_Bind\n" );

            ret = (DFBResult) CoreWindow_Lookup( core_dfb, args->source_id, caller, &source );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up source by ID %u failed!\n", __FUNCTION__, args->source_id );
                 return_args->result = ret;
                 return DFB_OK;
            }

            return_args->result = IWindow_Real__Bind( obj, source, args->x, args->y );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowBindReturn);

            if (source)
                CoreWindow_Unref( source );

            return DFB_OK;
        }

        case _CoreWindow_Unbind: {
    CoreWindow *source = NULL;
            D_UNUSED
            CoreWindowUnbind       *args        = (CoreWindowUnbind *) ptr;
            CoreWindowUnbindReturn *return_args = (CoreWindowUnbindReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_Unbind\n" );

            ret = (DFBResult) CoreWindow_Lookup( core_dfb, args->source_id, caller, &source );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up source by ID %u failed!\n", __FUNCTION__, args->source_id );
                 return_args->result = ret;
                 return DFB_OK;
            }

            return_args->result = IWindow_Real__Unbind( obj, source );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowUnbindReturn);

            if (source)
                CoreWindow_Unref( source );

            return DFB_OK;
        }

        case _CoreWindow_RequestFocus: {
            D_UNUSED
            CoreWindowRequestFocus       *args        = (CoreWindowRequestFocus *) ptr;
            CoreWindowRequestFocusReturn *return_args = (CoreWindowRequestFocusReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_RequestFocus\n" );

            return_args->result = IWindow_Real__RequestFocus( obj );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowRequestFocusReturn);

            return DFB_OK;
        }

        case _CoreWindow_ChangeGrab: {
            D_UNUSED
            CoreWindowChangeGrab       *args        = (CoreWindowChangeGrab *) ptr;
            CoreWindowChangeGrabReturn *return_args = (CoreWindowChangeGrabReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_ChangeGrab\n" );

            return_args->result = IWindow_Real__ChangeGrab( obj, args->target, args->grab );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowChangeGrabReturn);

            return DFB_OK;
        }

        case _CoreWindow_GrabKey: {
            D_UNUSED
            CoreWindowGrabKey       *args        = (CoreWindowGrabKey *) ptr;
            CoreWindowGrabKeyReturn *return_args = (CoreWindowGrabKeyReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_GrabKey\n" );

            return_args->result = IWindow_Real__GrabKey( obj, args->symbol, args->modifiers );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowGrabKeyReturn);

            return DFB_OK;
        }

        case _CoreWindow_UngrabKey: {
            D_UNUSED
            CoreWindowUngrabKey       *args        = (CoreWindowUngrabKey *) ptr;
            CoreWindowUngrabKeyReturn *return_args = (CoreWindowUngrabKeyReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_UngrabKey\n" );

            return_args->result = IWindow_Real__UngrabKey( obj, args->symbol, args->modifiers );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowUngrabKeyReturn);

            return DFB_OK;
        }

        case _CoreWindow_Move: {
            D_UNUSED
            CoreWindowMove       *args        = (CoreWindowMove *) ptr;
            CoreWindowMoveReturn *return_args = (CoreWindowMoveReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_Move\n" );

            return_args->result = IWindow_Real__Move( obj, args->dx, args->dy );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowMoveReturn);

            return DFB_OK;
        }

        case _CoreWindow_MoveTo: {
            D_UNUSED
            CoreWindowMoveTo       *args        = (CoreWindowMoveTo *) ptr;
            CoreWindowMoveToReturn *return_args = (CoreWindowMoveToReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_MoveTo\n" );

            return_args->result = IWindow_Real__MoveTo( obj, args->x, args->y );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowMoveToReturn);

            return DFB_OK;
        }

        case _CoreWindow_Resize: {
            D_UNUSED
            CoreWindowResize       *args        = (CoreWindowResize *) ptr;
            CoreWindowResizeReturn *return_args = (CoreWindowResizeReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_Resize\n" );

            return_args->result = IWindow_Real__Resize( obj, args->width, args->height );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowResizeReturn);

            return DFB_OK;
        }

        case _CoreWindow_Destroy: {
            D_UNUSED
            CoreWindowDestroy       *args        = (CoreWindowDestroy *) ptr;
            CoreWindowDestroyReturn *return_args = (CoreWindowDestroyReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_Destroy\n" );

            return_args->result = IWindow_Real__Destroy( obj );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowDestroyReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetCursorPosition: {
            D_UNUSED
            CoreWindowSetCursorPosition       *args        = (CoreWindowSetCursorPosition *) ptr;
            CoreWindowSetCursorPositionReturn *return_args = (CoreWindowSetCursorPositionReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetCursorPosition\n" );

            return_args->result = IWindow_Real__SetCursorPosition( obj, args->x, args->y );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetCursorPositionReturn);

            return DFB_OK;
        }

        case _CoreWindow_ChangeEvents: {
            D_UNUSED
            CoreWindowChangeEvents       *args        = (CoreWindowChangeEvents *) ptr;
            CoreWindowChangeEventsReturn *return_args = (CoreWindowChangeEventsReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_ChangeEvents\n" );

            return_args->result = IWindow_Real__ChangeEvents( obj, args->disable, args->enable );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowChangeEventsReturn);

            return DFB_OK;
        }

        case _CoreWindow_ChangeOptions: {
            D_UNUSED
            CoreWindowChangeOptions       *args        = (CoreWindowChangeOptions *) ptr;
            CoreWindowChangeOptionsReturn *return_args = (CoreWindowChangeOptionsReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_ChangeOptions\n" );

            return_args->result = IWindow_Real__ChangeOptions( obj, args->disable, args->enable );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowChangeOptionsReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetColor: {
            D_UNUSED
            CoreWindowSetColor       *args        = (CoreWindowSetColor *) ptr;
            CoreWindowSetColorReturn *return_args = (CoreWindowSetColorReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetColor\n" );

            return_args->result = IWindow_Real__SetColor( obj, &args->color );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetColorReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetColorKey: {
            D_UNUSED
            CoreWindowSetColorKey       *args        = (CoreWindowSetColorKey *) ptr;
            CoreWindowSetColorKeyReturn *return_args = (CoreWindowSetColorKeyReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetColorKey\n" );

            return_args->result = IWindow_Real__SetColorKey( obj, args->key );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetColorKeyReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetOpaque: {
            D_UNUSED
            CoreWindowSetOpaque       *args        = (CoreWindowSetOpaque *) ptr;
            CoreWindowSetOpaqueReturn *return_args = (CoreWindowSetOpaqueReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetOpaque\n" );

            return_args->result = IWindow_Real__SetOpaque( obj, &args->opaque );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetOpaqueReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetOpacity: {
            D_UNUSED
            CoreWindowSetOpacity       *args        = (CoreWindowSetOpacity *) ptr;
            CoreWindowSetOpacityReturn *return_args = (CoreWindowSetOpacityReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetOpacity\n" );

            return_args->result = IWindow_Real__SetOpacity( obj, args->opacity );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetOpacityReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetStacking: {
            D_UNUSED
            CoreWindowSetStacking       *args        = (CoreWindowSetStacking *) ptr;
            CoreWindowSetStackingReturn *return_args = (CoreWindowSetStackingReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetStacking\n" );

            return_args->result = IWindow_Real__SetStacking( obj, args->stacking );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetStackingReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetBounds: {
            D_UNUSED
            CoreWindowSetBounds       *args        = (CoreWindowSetBounds *) ptr;
            CoreWindowSetBoundsReturn *return_args = (CoreWindowSetBoundsReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetBounds\n" );

            return_args->result = IWindow_Real__SetBounds( obj, &args->bounds );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetBoundsReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetKeySelection: {
            D_UNUSED
            CoreWindowSetKeySelection       *args        = (CoreWindowSetKeySelection *) ptr;
            CoreWindowSetKeySelectionReturn *return_args = (CoreWindowSetKeySelectionReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetKeySelection\n" );

            return_args->result = IWindow_Real__SetKeySelection( obj, args->selection, args->keys_set ? (DFBInputDeviceKeySymbol*) ((char*)(args + 1)) : NULL, args->num_keys );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetKeySelectionReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetRotation: {
            D_UNUSED
            CoreWindowSetRotation       *args        = (CoreWindowSetRotation *) ptr;
            CoreWindowSetRotationReturn *return_args = (CoreWindowSetRotationReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetRotation\n" );

            return_args->result = IWindow_Real__SetRotation( obj, args->rotation );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetRotationReturn);

            return DFB_OK;
        }

        case _CoreWindow_GetSurface: {
    CoreSurface *surface = NULL;
            D_UNUSED
            CoreWindowGetSurface       *args        = (CoreWindowGetSurface *) ptr;
            CoreWindowGetSurfaceReturn *return_args = (CoreWindowGetSurfaceReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_GetSurface\n" );

            return_args->result = IWindow_Real__GetSurface( obj, &surface );
            if (return_args->result == DFB_OK) {
                CoreSurface_Throw( surface, caller, &return_args->surface_id );
            }

            *ret_length = sizeof(CoreWindowGetSurfaceReturn);

            return DFB_OK;
        }

        case _CoreWindow_SetCursorShape: {
    CoreSurface *shape = NULL;
            D_UNUSED
            CoreWindowSetCursorShape       *args        = (CoreWindowSetCursorShape *) ptr;
            CoreWindowSetCursorShapeReturn *return_args = (CoreWindowSetCursorShapeReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_CoreWindow, "=-> CoreWindow_SetCursorShape\n" );

            if (args->shape_set) {
                ret = (DFBResult) CoreSurface_Lookup( core_dfb, args->shape_id, caller, &shape );
                if (ret) {
                     D_DERROR( ret, "%s: Looking up shape by ID %u failed!\n", __FUNCTION__, args->shape_id );
                     return_args->result = ret;
                     return DFB_OK;
                }
            }

            return_args->result = IWindow_Real__SetCursorShape( obj, args->shape_set ? shape : NULL, &args->hotspot );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(CoreWindowSetCursorShapeReturn);

            if (shape)
                CoreSurface_Unref( shape );

            return DFB_OK;
        }

    }

    return DFB_NOSUCHMETHOD;
}
/*********************************************************************************************************************/

DFBResult
CoreWindowDispatch__Dispatch( CoreWindow *obj,
                                FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length )
{
    DFBResult ret;

    D_DEBUG_AT( DirectFB_CoreWindow, "CoreWindowDispatch::%s( %p )\n", __FUNCTION__, obj );

    Core_PushIdentity( caller );

    ret = __CoreWindowDispatch__Dispatch( obj, caller, method, ptr, length, ret_ptr, ret_size, ret_length );

    Core_PopIdentity();

    return ret;
}
