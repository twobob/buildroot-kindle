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

#ifndef ___CoreWindowStack__H___
#define ___CoreWindowStack__H___

#include <core/CoreWindowStack_includes.h>

/**********************************************************************************************************************
 * CoreWindowStack
 */

#ifdef __cplusplus
#include <core/Interface.h>

extern "C" {
#endif


DFBResult CoreWindowStack_RepaintAll(
                    CoreWindowStack                           *obj
);

DFBResult CoreWindowStack_GetInsets(
                    CoreWindowStack                           *obj,
                    CoreWindow                                *window,
                    DFBInsets                                 *ret_insets);

DFBResult CoreWindowStack_CursorEnable(
                    CoreWindowStack                           *obj,
                    bool                                       enable);

DFBResult CoreWindowStack_CursorSetShape(
                    CoreWindowStack                           *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot);

DFBResult CoreWindowStack_CursorSetOpacity(
                    CoreWindowStack                           *obj,
                    u8                                         opacity);

DFBResult CoreWindowStack_CursorSetAcceleration(
                    CoreWindowStack                           *obj,
                    u32                                        numerator,
                    u32                                        denominator,
                    u32                                        threshold);

DFBResult CoreWindowStack_CursorWarp(
                    CoreWindowStack                           *obj,
                    const DFBPoint                            *position);

DFBResult CoreWindowStack_CursorGetPosition(
                    CoreWindowStack                           *obj,
                    DFBPoint                                  *ret_position);

DFBResult CoreWindowStack_BackgroundSetMode(
                    CoreWindowStack                           *obj,
                    DFBDisplayLayerBackgroundMode              mode);

DFBResult CoreWindowStack_BackgroundSetImage(
                    CoreWindowStack                           *obj,
                    CoreSurface                               *image);

DFBResult CoreWindowStack_BackgroundSetColor(
                    CoreWindowStack                           *obj,
                    const DFBColor                            *color);

DFBResult CoreWindowStack_BackgroundSetColorIndex(
                    CoreWindowStack                           *obj,
                    s32                                        index);


void CoreWindowStack_Init_Dispatch(
                    CoreDFB              *core,
                    CoreWindowStack      *obj,
                    FusionCall           *call
);

void  CoreWindowStack_Deinit_Dispatch(
                    FusionCall           *call
);


#ifdef __cplusplus
}
#endif




/*
 * CoreWindowStack Calls
 */
typedef enum {
    _CoreWindowStack_RepaintAll = 1,
    _CoreWindowStack_GetInsets = 2,
    _CoreWindowStack_CursorEnable = 3,
    _CoreWindowStack_CursorSetShape = 4,
    _CoreWindowStack_CursorSetOpacity = 5,
    _CoreWindowStack_CursorSetAcceleration = 6,
    _CoreWindowStack_CursorWarp = 7,
    _CoreWindowStack_CursorGetPosition = 8,
    _CoreWindowStack_BackgroundSetMode = 9,
    _CoreWindowStack_BackgroundSetImage = 10,
    _CoreWindowStack_BackgroundSetColor = 11,
    _CoreWindowStack_BackgroundSetColorIndex = 12,
} CoreWindowStackCall;

/*
 * CoreWindowStack_RepaintAll
 */
typedef struct {
} CoreWindowStackRepaintAll;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackRepaintAllReturn;


/*
 * CoreWindowStack_GetInsets
 */
typedef struct {
    u32                                        window_id;
} CoreWindowStackGetInsets;

typedef struct {
    DFBResult                                  result;
    DFBInsets                                  insets;
} CoreWindowStackGetInsetsReturn;


/*
 * CoreWindowStack_CursorEnable
 */
typedef struct {
    bool                                       enable;
} CoreWindowStackCursorEnable;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackCursorEnableReturn;


/*
 * CoreWindowStack_CursorSetShape
 */
typedef struct {
    u32                                        shape_id;
    DFBPoint                                   hotspot;
} CoreWindowStackCursorSetShape;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackCursorSetShapeReturn;


/*
 * CoreWindowStack_CursorSetOpacity
 */
typedef struct {
    u8                                         opacity;
} CoreWindowStackCursorSetOpacity;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackCursorSetOpacityReturn;


/*
 * CoreWindowStack_CursorSetAcceleration
 */
typedef struct {
    u32                                        numerator;
    u32                                        denominator;
    u32                                        threshold;
} CoreWindowStackCursorSetAcceleration;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackCursorSetAccelerationReturn;


/*
 * CoreWindowStack_CursorWarp
 */
typedef struct {
    DFBPoint                                   position;
} CoreWindowStackCursorWarp;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackCursorWarpReturn;


/*
 * CoreWindowStack_CursorGetPosition
 */
typedef struct {
} CoreWindowStackCursorGetPosition;

typedef struct {
    DFBResult                                  result;
    DFBPoint                                   position;
} CoreWindowStackCursorGetPositionReturn;


/*
 * CoreWindowStack_BackgroundSetMode
 */
typedef struct {
    DFBDisplayLayerBackgroundMode              mode;
} CoreWindowStackBackgroundSetMode;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackBackgroundSetModeReturn;


/*
 * CoreWindowStack_BackgroundSetImage
 */
typedef struct {
    u32                                        image_id;
} CoreWindowStackBackgroundSetImage;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackBackgroundSetImageReturn;


/*
 * CoreWindowStack_BackgroundSetColor
 */
typedef struct {
    DFBColor                                   color;
} CoreWindowStackBackgroundSetColor;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackBackgroundSetColorReturn;


/*
 * CoreWindowStack_BackgroundSetColorIndex
 */
typedef struct {
    s32                                        index;
} CoreWindowStackBackgroundSetColorIndex;

typedef struct {
    DFBResult                                  result;
} CoreWindowStackBackgroundSetColorIndexReturn;


DFBResult IWindowStack_Real__RepaintAll( CoreWindowStack *obj
 );

DFBResult IWindowStack_Real__GetInsets( CoreWindowStack *obj,
                    CoreWindow                                *window,
                    DFBInsets                                 *ret_insets );

DFBResult IWindowStack_Real__CursorEnable( CoreWindowStack *obj,
                    bool                                       enable );

DFBResult IWindowStack_Real__CursorSetShape( CoreWindowStack *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot );

DFBResult IWindowStack_Real__CursorSetOpacity( CoreWindowStack *obj,
                    u8                                         opacity );

DFBResult IWindowStack_Real__CursorSetAcceleration( CoreWindowStack *obj,
                    u32                                        numerator,
                    u32                                        denominator,
                    u32                                        threshold );

DFBResult IWindowStack_Real__CursorWarp( CoreWindowStack *obj,
                    const DFBPoint                            *position );

DFBResult IWindowStack_Real__CursorGetPosition( CoreWindowStack *obj,
                    DFBPoint                                  *ret_position );

DFBResult IWindowStack_Real__BackgroundSetMode( CoreWindowStack *obj,
                    DFBDisplayLayerBackgroundMode              mode );

DFBResult IWindowStack_Real__BackgroundSetImage( CoreWindowStack *obj,
                    CoreSurface                               *image );

DFBResult IWindowStack_Real__BackgroundSetColor( CoreWindowStack *obj,
                    const DFBColor                            *color );

DFBResult IWindowStack_Real__BackgroundSetColorIndex( CoreWindowStack *obj,
                    s32                                        index );

DFBResult IWindowStack_Requestor__RepaintAll( CoreWindowStack *obj
 );

DFBResult IWindowStack_Requestor__GetInsets( CoreWindowStack *obj,
                    CoreWindow                                *window,
                    DFBInsets                                 *ret_insets );

DFBResult IWindowStack_Requestor__CursorEnable( CoreWindowStack *obj,
                    bool                                       enable );

DFBResult IWindowStack_Requestor__CursorSetShape( CoreWindowStack *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot );

DFBResult IWindowStack_Requestor__CursorSetOpacity( CoreWindowStack *obj,
                    u8                                         opacity );

DFBResult IWindowStack_Requestor__CursorSetAcceleration( CoreWindowStack *obj,
                    u32                                        numerator,
                    u32                                        denominator,
                    u32                                        threshold );

DFBResult IWindowStack_Requestor__CursorWarp( CoreWindowStack *obj,
                    const DFBPoint                            *position );

DFBResult IWindowStack_Requestor__CursorGetPosition( CoreWindowStack *obj,
                    DFBPoint                                  *ret_position );

DFBResult IWindowStack_Requestor__BackgroundSetMode( CoreWindowStack *obj,
                    DFBDisplayLayerBackgroundMode              mode );

DFBResult IWindowStack_Requestor__BackgroundSetImage( CoreWindowStack *obj,
                    CoreSurface                               *image );

DFBResult IWindowStack_Requestor__BackgroundSetColor( CoreWindowStack *obj,
                    const DFBColor                            *color );

DFBResult IWindowStack_Requestor__BackgroundSetColorIndex( CoreWindowStack *obj,
                    s32                                        index );


DFBResult CoreWindowStackDispatch__Dispatch( CoreWindowStack *obj,
                    FusionID      caller,
                    int           method,
                    void         *ptr,
                    unsigned int  length,
                    void         *ret_ptr,
                    unsigned int  ret_size,
                    unsigned int *ret_length );


#endif
