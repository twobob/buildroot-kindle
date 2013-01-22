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

#ifndef ___CoreWindow__H___
#define ___CoreWindow__H___

#include <core/CoreWindow_includes.h>

/**********************************************************************************************************************
 * CoreWindow
 */

#ifdef __cplusplus
#include <core/Interface.h>

extern "C" {
#endif


DFBResult CoreWindow_Repaint(
                    CoreWindow                                *obj,
                    const DFBRegion                           *left,
                    const DFBRegion                           *right,
                    DFBSurfaceFlipFlags                        flags);

DFBResult CoreWindow_BeginUpdates(
                    CoreWindow                                *obj,
                    const DFBRegion                           *update);

DFBResult CoreWindow_Restack(
                    CoreWindow                                *obj,
                    CoreWindow                                *relative,
                    int                                        relation);

DFBResult CoreWindow_SetConfig(
                    CoreWindow                                *obj,
                    const CoreWindowConfig                    *config,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys,
                    CoreWindow                                *parent,
                    CoreWindowConfigFlags                      flags);

DFBResult CoreWindow_Bind(
                    CoreWindow                                *obj,
                    CoreWindow                                *source,
                    int                                        x,
                    int                                        y);

DFBResult CoreWindow_Unbind(
                    CoreWindow                                *obj,
                    CoreWindow                                *source);

DFBResult CoreWindow_RequestFocus(
                    CoreWindow                                *obj
);

DFBResult CoreWindow_ChangeGrab(
                    CoreWindow                                *obj,
                    CoreWMGrabTarget                           target,
                    bool                                       grab);

DFBResult CoreWindow_GrabKey(
                    CoreWindow                                *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers);

DFBResult CoreWindow_UngrabKey(
                    CoreWindow                                *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers);

DFBResult CoreWindow_Move(
                    CoreWindow                                *obj,
                    int                                        dx,
                    int                                        dy);

DFBResult CoreWindow_MoveTo(
                    CoreWindow                                *obj,
                    int                                        x,
                    int                                        y);

DFBResult CoreWindow_Resize(
                    CoreWindow                                *obj,
                    int                                        width,
                    int                                        height);

DFBResult CoreWindow_Destroy(
                    CoreWindow                                *obj
);

DFBResult CoreWindow_SetCursorPosition(
                    CoreWindow                                *obj,
                    int                                        x,
                    int                                        y);

DFBResult CoreWindow_ChangeEvents(
                    CoreWindow                                *obj,
                    DFBWindowEventType                         disable,
                    DFBWindowEventType                         enable);

DFBResult CoreWindow_ChangeOptions(
                    CoreWindow                                *obj,
                    DFBWindowOptions                           disable,
                    DFBWindowOptions                           enable);

DFBResult CoreWindow_SetColor(
                    CoreWindow                                *obj,
                    const DFBColor                            *color);

DFBResult CoreWindow_SetColorKey(
                    CoreWindow                                *obj,
                    u32                                        key);

DFBResult CoreWindow_SetOpaque(
                    CoreWindow                                *obj,
                    const DFBRegion                           *opaque);

DFBResult CoreWindow_SetOpacity(
                    CoreWindow                                *obj,
                    u8                                         opacity);

DFBResult CoreWindow_SetStacking(
                    CoreWindow                                *obj,
                    DFBWindowStackingClass                     stacking);

DFBResult CoreWindow_SetBounds(
                    CoreWindow                                *obj,
                    const DFBRectangle                        *bounds);

DFBResult CoreWindow_SetKeySelection(
                    CoreWindow                                *obj,
                    DFBWindowKeySelection                      selection,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys);

DFBResult CoreWindow_SetRotation(
                    CoreWindow                                *obj,
                    int                                        rotation);

DFBResult CoreWindow_GetSurface(
                    CoreWindow                                *obj,
                    CoreSurface                              **ret_surface);

DFBResult CoreWindow_SetCursorShape(
                    CoreWindow                                *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot);


void CoreWindow_Init_Dispatch(
                    CoreDFB              *core,
                    CoreWindow           *obj,
                    FusionCall           *call
);

void  CoreWindow_Deinit_Dispatch(
                    FusionCall           *call
);


#ifdef __cplusplus
}
#endif




/*
 * CoreWindow Calls
 */
typedef enum {
    _CoreWindow_Repaint = 1,
    _CoreWindow_BeginUpdates = 2,
    _CoreWindow_Restack = 3,
    _CoreWindow_SetConfig = 4,
    _CoreWindow_Bind = 5,
    _CoreWindow_Unbind = 6,
    _CoreWindow_RequestFocus = 7,
    _CoreWindow_ChangeGrab = 8,
    _CoreWindow_GrabKey = 9,
    _CoreWindow_UngrabKey = 10,
    _CoreWindow_Move = 11,
    _CoreWindow_MoveTo = 12,
    _CoreWindow_Resize = 13,
    _CoreWindow_Destroy = 14,
    _CoreWindow_SetCursorPosition = 15,
    _CoreWindow_ChangeEvents = 16,
    _CoreWindow_ChangeOptions = 17,
    _CoreWindow_SetColor = 18,
    _CoreWindow_SetColorKey = 19,
    _CoreWindow_SetOpaque = 20,
    _CoreWindow_SetOpacity = 21,
    _CoreWindow_SetStacking = 22,
    _CoreWindow_SetBounds = 23,
    _CoreWindow_SetKeySelection = 24,
    _CoreWindow_SetRotation = 25,
    _CoreWindow_GetSurface = 26,
    _CoreWindow_SetCursorShape = 27,
} CoreWindowCall;

/*
 * CoreWindow_Repaint
 */
typedef struct {
    DFBRegion                                  left;
    DFBRegion                                  right;
    DFBSurfaceFlipFlags                        flags;
} CoreWindowRepaint;

typedef struct {
    DFBResult                                  result;
} CoreWindowRepaintReturn;


/*
 * CoreWindow_BeginUpdates
 */
typedef struct {
    bool                                       update_set;
    DFBRegion                                  update;
} CoreWindowBeginUpdates;

typedef struct {
    DFBResult                                  result;
} CoreWindowBeginUpdatesReturn;


/*
 * CoreWindow_Restack
 */
typedef struct {
    bool                                       relative_set;
    u32                                        relative_id;
    int                                        relation;
} CoreWindowRestack;

typedef struct {
    DFBResult                                  result;
} CoreWindowRestackReturn;


/*
 * CoreWindow_SetConfig
 */
typedef struct {
    CoreWindowConfig                           config;
    u32                                        num_keys;
    bool                                       parent_set;
    u32                                        parent_id;
    CoreWindowConfigFlags                      flags;
    bool                                       keys_set;
    /* 'num_keys' DFBInputDeviceKeySymbol follow (keys) */
} CoreWindowSetConfig;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetConfigReturn;


/*
 * CoreWindow_Bind
 */
typedef struct {
    u32                                        source_id;
    int                                        x;
    int                                        y;
} CoreWindowBind;

typedef struct {
    DFBResult                                  result;
} CoreWindowBindReturn;


/*
 * CoreWindow_Unbind
 */
typedef struct {
    u32                                        source_id;
} CoreWindowUnbind;

typedef struct {
    DFBResult                                  result;
} CoreWindowUnbindReturn;


/*
 * CoreWindow_RequestFocus
 */
typedef struct {
} CoreWindowRequestFocus;

typedef struct {
    DFBResult                                  result;
} CoreWindowRequestFocusReturn;


/*
 * CoreWindow_ChangeGrab
 */
typedef struct {
    CoreWMGrabTarget                           target;
    bool                                       grab;
} CoreWindowChangeGrab;

typedef struct {
    DFBResult                                  result;
} CoreWindowChangeGrabReturn;


/*
 * CoreWindow_GrabKey
 */
typedef struct {
    DFBInputDeviceKeySymbol                    symbol;
    DFBInputDeviceModifierMask                 modifiers;
} CoreWindowGrabKey;

typedef struct {
    DFBResult                                  result;
} CoreWindowGrabKeyReturn;


/*
 * CoreWindow_UngrabKey
 */
typedef struct {
    DFBInputDeviceKeySymbol                    symbol;
    DFBInputDeviceModifierMask                 modifiers;
} CoreWindowUngrabKey;

typedef struct {
    DFBResult                                  result;
} CoreWindowUngrabKeyReturn;


/*
 * CoreWindow_Move
 */
typedef struct {
    int                                        dx;
    int                                        dy;
} CoreWindowMove;

typedef struct {
    DFBResult                                  result;
} CoreWindowMoveReturn;


/*
 * CoreWindow_MoveTo
 */
typedef struct {
    int                                        x;
    int                                        y;
} CoreWindowMoveTo;

typedef struct {
    DFBResult                                  result;
} CoreWindowMoveToReturn;


/*
 * CoreWindow_Resize
 */
typedef struct {
    int                                        width;
    int                                        height;
} CoreWindowResize;

typedef struct {
    DFBResult                                  result;
} CoreWindowResizeReturn;


/*
 * CoreWindow_Destroy
 */
typedef struct {
} CoreWindowDestroy;

typedef struct {
    DFBResult                                  result;
} CoreWindowDestroyReturn;


/*
 * CoreWindow_SetCursorPosition
 */
typedef struct {
    int                                        x;
    int                                        y;
} CoreWindowSetCursorPosition;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetCursorPositionReturn;


/*
 * CoreWindow_ChangeEvents
 */
typedef struct {
    DFBWindowEventType                         disable;
    DFBWindowEventType                         enable;
} CoreWindowChangeEvents;

typedef struct {
    DFBResult                                  result;
} CoreWindowChangeEventsReturn;


/*
 * CoreWindow_ChangeOptions
 */
typedef struct {
    DFBWindowOptions                           disable;
    DFBWindowOptions                           enable;
} CoreWindowChangeOptions;

typedef struct {
    DFBResult                                  result;
} CoreWindowChangeOptionsReturn;


/*
 * CoreWindow_SetColor
 */
typedef struct {
    DFBColor                                   color;
} CoreWindowSetColor;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetColorReturn;


/*
 * CoreWindow_SetColorKey
 */
typedef struct {
    u32                                        key;
} CoreWindowSetColorKey;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetColorKeyReturn;


/*
 * CoreWindow_SetOpaque
 */
typedef struct {
    DFBRegion                                  opaque;
} CoreWindowSetOpaque;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetOpaqueReturn;


/*
 * CoreWindow_SetOpacity
 */
typedef struct {
    u8                                         opacity;
} CoreWindowSetOpacity;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetOpacityReturn;


/*
 * CoreWindow_SetStacking
 */
typedef struct {
    DFBWindowStackingClass                     stacking;
} CoreWindowSetStacking;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetStackingReturn;


/*
 * CoreWindow_SetBounds
 */
typedef struct {
    DFBRectangle                               bounds;
} CoreWindowSetBounds;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetBoundsReturn;


/*
 * CoreWindow_SetKeySelection
 */
typedef struct {
    DFBWindowKeySelection                      selection;
    u32                                        num_keys;
    bool                                       keys_set;
    /* 'num_keys' DFBInputDeviceKeySymbol follow (keys) */
} CoreWindowSetKeySelection;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetKeySelectionReturn;


/*
 * CoreWindow_SetRotation
 */
typedef struct {
    int                                        rotation;
} CoreWindowSetRotation;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetRotationReturn;


/*
 * CoreWindow_GetSurface
 */
typedef struct {
} CoreWindowGetSurface;

typedef struct {
    DFBResult                                  result;
    u32                                        surface_id;
} CoreWindowGetSurfaceReturn;


/*
 * CoreWindow_SetCursorShape
 */
typedef struct {
    bool                                       shape_set;
    u32                                        shape_id;
    DFBPoint                                   hotspot;
} CoreWindowSetCursorShape;

typedef struct {
    DFBResult                                  result;
} CoreWindowSetCursorShapeReturn;


DFBResult IWindow_Real__Repaint( CoreWindow *obj,
                    const DFBRegion                           *left,
                    const DFBRegion                           *right,
                    DFBSurfaceFlipFlags                        flags );

DFBResult IWindow_Real__BeginUpdates( CoreWindow *obj,
                    const DFBRegion                           *update );

DFBResult IWindow_Real__Restack( CoreWindow *obj,
                    CoreWindow                                *relative,
                    int                                        relation );

DFBResult IWindow_Real__SetConfig( CoreWindow *obj,
                    const CoreWindowConfig                    *config,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys,
                    CoreWindow                                *parent,
                    CoreWindowConfigFlags                      flags );

DFBResult IWindow_Real__Bind( CoreWindow *obj,
                    CoreWindow                                *source,
                    int                                        x,
                    int                                        y );

DFBResult IWindow_Real__Unbind( CoreWindow *obj,
                    CoreWindow                                *source );

DFBResult IWindow_Real__RequestFocus( CoreWindow *obj
 );

DFBResult IWindow_Real__ChangeGrab( CoreWindow *obj,
                    CoreWMGrabTarget                           target,
                    bool                                       grab );

DFBResult IWindow_Real__GrabKey( CoreWindow *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers );

DFBResult IWindow_Real__UngrabKey( CoreWindow *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers );

DFBResult IWindow_Real__Move( CoreWindow *obj,
                    int                                        dx,
                    int                                        dy );

DFBResult IWindow_Real__MoveTo( CoreWindow *obj,
                    int                                        x,
                    int                                        y );

DFBResult IWindow_Real__Resize( CoreWindow *obj,
                    int                                        width,
                    int                                        height );

DFBResult IWindow_Real__Destroy( CoreWindow *obj
 );

DFBResult IWindow_Real__SetCursorPosition( CoreWindow *obj,
                    int                                        x,
                    int                                        y );

DFBResult IWindow_Real__ChangeEvents( CoreWindow *obj,
                    DFBWindowEventType                         disable,
                    DFBWindowEventType                         enable );

DFBResult IWindow_Real__ChangeOptions( CoreWindow *obj,
                    DFBWindowOptions                           disable,
                    DFBWindowOptions                           enable );

DFBResult IWindow_Real__SetColor( CoreWindow *obj,
                    const DFBColor                            *color );

DFBResult IWindow_Real__SetColorKey( CoreWindow *obj,
                    u32                                        key );

DFBResult IWindow_Real__SetOpaque( CoreWindow *obj,
                    const DFBRegion                           *opaque );

DFBResult IWindow_Real__SetOpacity( CoreWindow *obj,
                    u8                                         opacity );

DFBResult IWindow_Real__SetStacking( CoreWindow *obj,
                    DFBWindowStackingClass                     stacking );

DFBResult IWindow_Real__SetBounds( CoreWindow *obj,
                    const DFBRectangle                        *bounds );

DFBResult IWindow_Real__SetKeySelection( CoreWindow *obj,
                    DFBWindowKeySelection                      selection,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys );

DFBResult IWindow_Real__SetRotation( CoreWindow *obj,
                    int                                        rotation );

DFBResult IWindow_Real__GetSurface( CoreWindow *obj,
                    CoreSurface                              **ret_surface );

DFBResult IWindow_Real__SetCursorShape( CoreWindow *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot );

DFBResult IWindow_Requestor__Repaint( CoreWindow *obj,
                    const DFBRegion                           *left,
                    const DFBRegion                           *right,
                    DFBSurfaceFlipFlags                        flags );

DFBResult IWindow_Requestor__BeginUpdates( CoreWindow *obj,
                    const DFBRegion                           *update );

DFBResult IWindow_Requestor__Restack( CoreWindow *obj,
                    CoreWindow                                *relative,
                    int                                        relation );

DFBResult IWindow_Requestor__SetConfig( CoreWindow *obj,
                    const CoreWindowConfig                    *config,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys,
                    CoreWindow                                *parent,
                    CoreWindowConfigFlags                      flags );

DFBResult IWindow_Requestor__Bind( CoreWindow *obj,
                    CoreWindow                                *source,
                    int                                        x,
                    int                                        y );

DFBResult IWindow_Requestor__Unbind( CoreWindow *obj,
                    CoreWindow                                *source );

DFBResult IWindow_Requestor__RequestFocus( CoreWindow *obj
 );

DFBResult IWindow_Requestor__ChangeGrab( CoreWindow *obj,
                    CoreWMGrabTarget                           target,
                    bool                                       grab );

DFBResult IWindow_Requestor__GrabKey( CoreWindow *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers );

DFBResult IWindow_Requestor__UngrabKey( CoreWindow *obj,
                    DFBInputDeviceKeySymbol                    symbol,
                    DFBInputDeviceModifierMask                 modifiers );

DFBResult IWindow_Requestor__Move( CoreWindow *obj,
                    int                                        dx,
                    int                                        dy );

DFBResult IWindow_Requestor__MoveTo( CoreWindow *obj,
                    int                                        x,
                    int                                        y );

DFBResult IWindow_Requestor__Resize( CoreWindow *obj,
                    int                                        width,
                    int                                        height );

DFBResult IWindow_Requestor__Destroy( CoreWindow *obj
 );

DFBResult IWindow_Requestor__SetCursorPosition( CoreWindow *obj,
                    int                                        x,
                    int                                        y );

DFBResult IWindow_Requestor__ChangeEvents( CoreWindow *obj,
                    DFBWindowEventType                         disable,
                    DFBWindowEventType                         enable );

DFBResult IWindow_Requestor__ChangeOptions( CoreWindow *obj,
                    DFBWindowOptions                           disable,
                    DFBWindowOptions                           enable );

DFBResult IWindow_Requestor__SetColor( CoreWindow *obj,
                    const DFBColor                            *color );

DFBResult IWindow_Requestor__SetColorKey( CoreWindow *obj,
                    u32                                        key );

DFBResult IWindow_Requestor__SetOpaque( CoreWindow *obj,
                    const DFBRegion                           *opaque );

DFBResult IWindow_Requestor__SetOpacity( CoreWindow *obj,
                    u8                                         opacity );

DFBResult IWindow_Requestor__SetStacking( CoreWindow *obj,
                    DFBWindowStackingClass                     stacking );

DFBResult IWindow_Requestor__SetBounds( CoreWindow *obj,
                    const DFBRectangle                        *bounds );

DFBResult IWindow_Requestor__SetKeySelection( CoreWindow *obj,
                    DFBWindowKeySelection                      selection,
                    const DFBInputDeviceKeySymbol             *keys,
                    u32                                        num_keys );

DFBResult IWindow_Requestor__SetRotation( CoreWindow *obj,
                    int                                        rotation );

DFBResult IWindow_Requestor__GetSurface( CoreWindow *obj,
                    CoreSurface                              **ret_surface );

DFBResult IWindow_Requestor__SetCursorShape( CoreWindow *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot );


DFBResult CoreWindowDispatch__Dispatch( CoreWindow *obj,
                    FusionID      caller,
                    int           method,
                    void         *ptr,
                    unsigned int  length,
                    void         *ret_ptr,
                    unsigned int  ret_size,
                    unsigned int *ret_length );


#endif
