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

#ifndef ___CoreGraphicsState__H___
#define ___CoreGraphicsState__H___

#include <core/CoreGraphicsState_includes.h>

/**********************************************************************************************************************
 * CoreGraphicsState
 */

#ifdef __cplusplus
#include <core/Interface.h>

extern "C" {
#endif


DFBResult CoreGraphicsState_SetDrawingFlags(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceDrawingFlags                     flags);

DFBResult CoreGraphicsState_SetBlittingFlags(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceBlittingFlags                    flags);

DFBResult CoreGraphicsState_SetClip(
                    CoreGraphicsState                         *obj,
                    const DFBRegion                           *region);

DFBResult CoreGraphicsState_SetColor(
                    CoreGraphicsState                         *obj,
                    const DFBColor                            *color);

DFBResult CoreGraphicsState_SetColorAndIndex(
                    CoreGraphicsState                         *obj,
                    const DFBColor                            *color,
                    u32                                        index);

DFBResult CoreGraphicsState_SetSrcBlend(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceBlendFunction                    function);

DFBResult CoreGraphicsState_SetDstBlend(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceBlendFunction                    function);

DFBResult CoreGraphicsState_SetSrcColorKey(
                    CoreGraphicsState                         *obj,
                    u32                                        key);

DFBResult CoreGraphicsState_SetDstColorKey(
                    CoreGraphicsState                         *obj,
                    u32                                        key);

DFBResult CoreGraphicsState_SetDestination(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface);

DFBResult CoreGraphicsState_SetSource(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface);

DFBResult CoreGraphicsState_SetSourceMask(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface);

DFBResult CoreGraphicsState_SetSourceMaskVals(
                    CoreGraphicsState                         *obj,
                    const DFBPoint                            *offset,
                    DFBSurfaceMaskFlags                        flags);

DFBResult CoreGraphicsState_SetIndexTranslation(
                    CoreGraphicsState                         *obj,
                    const s32                                 *indices,
                    u32                                        num);

DFBResult CoreGraphicsState_SetColorKey(
                    CoreGraphicsState                         *obj,
                    const DFBColorKey                         *key);

DFBResult CoreGraphicsState_SetRenderOptions(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceRenderOptions                    options);

DFBResult CoreGraphicsState_SetMatrix(
                    CoreGraphicsState                         *obj,
                    const s32                                 *values);

DFBResult CoreGraphicsState_SetSource2(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface);

DFBResult CoreGraphicsState_DrawRectangles(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num);

DFBResult CoreGraphicsState_DrawLines(
                    CoreGraphicsState                         *obj,
                    const DFBRegion                           *lines,
                    u32                                        num);

DFBResult CoreGraphicsState_FillRectangles(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num);

DFBResult CoreGraphicsState_FillTriangles(
                    CoreGraphicsState                         *obj,
                    const DFBTriangle                         *triangles,
                    u32                                        num);

DFBResult CoreGraphicsState_FillTrapezoids(
                    CoreGraphicsState                         *obj,
                    const DFBTrapezoid                        *trapezoids,
                    u32                                        num);

DFBResult CoreGraphicsState_FillSpans(
                    CoreGraphicsState                         *obj,
                    s32                                        y,
                    const DFBSpan                             *spans,
                    u32                                        num);

DFBResult CoreGraphicsState_Blit(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points,
                    u32                                        num);

DFBResult CoreGraphicsState_Blit2(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num);

DFBResult CoreGraphicsState_StretchBlit(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *srects,
                    const DFBRectangle                        *drects,
                    u32                                        num);

DFBResult CoreGraphicsState_TileBlit(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num);

DFBResult CoreGraphicsState_TextureTriangles(
                    CoreGraphicsState                         *obj,
                    const DFBVertex                           *vertices,
                    u32                                        num,
                    DFBTriangleFormation                       formation);

DFBResult CoreGraphicsState_ReleaseSource(
                    CoreGraphicsState                         *obj
);


void CoreGraphicsState_Init_Dispatch(
                    CoreDFB              *core,
                    CoreGraphicsState    *obj,
                    FusionCall           *call
);

void  CoreGraphicsState_Deinit_Dispatch(
                    FusionCall           *call
);


#ifdef __cplusplus
}
#endif




/*
 * CoreGraphicsState Calls
 */
typedef enum {
    _CoreGraphicsState_SetDrawingFlags = 1,
    _CoreGraphicsState_SetBlittingFlags = 2,
    _CoreGraphicsState_SetClip = 3,
    _CoreGraphicsState_SetColor = 4,
    _CoreGraphicsState_SetColorAndIndex = 5,
    _CoreGraphicsState_SetSrcBlend = 6,
    _CoreGraphicsState_SetDstBlend = 7,
    _CoreGraphicsState_SetSrcColorKey = 8,
    _CoreGraphicsState_SetDstColorKey = 9,
    _CoreGraphicsState_SetDestination = 10,
    _CoreGraphicsState_SetSource = 11,
    _CoreGraphicsState_SetSourceMask = 12,
    _CoreGraphicsState_SetSourceMaskVals = 13,
    _CoreGraphicsState_SetIndexTranslation = 14,
    _CoreGraphicsState_SetColorKey = 15,
    _CoreGraphicsState_SetRenderOptions = 16,
    _CoreGraphicsState_SetMatrix = 17,
    _CoreGraphicsState_SetSource2 = 18,
    _CoreGraphicsState_DrawRectangles = 19,
    _CoreGraphicsState_DrawLines = 20,
    _CoreGraphicsState_FillRectangles = 21,
    _CoreGraphicsState_FillTriangles = 22,
    _CoreGraphicsState_FillTrapezoids = 23,
    _CoreGraphicsState_FillSpans = 24,
    _CoreGraphicsState_Blit = 25,
    _CoreGraphicsState_Blit2 = 26,
    _CoreGraphicsState_StretchBlit = 27,
    _CoreGraphicsState_TileBlit = 28,
    _CoreGraphicsState_TextureTriangles = 29,
    _CoreGraphicsState_ReleaseSource = 30,
} CoreGraphicsStateCall;

/*
 * CoreGraphicsState_SetDrawingFlags
 */
typedef struct {
    DFBSurfaceDrawingFlags                     flags;
} CoreGraphicsStateSetDrawingFlags;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetDrawingFlagsReturn;


/*
 * CoreGraphicsState_SetBlittingFlags
 */
typedef struct {
    DFBSurfaceBlittingFlags                    flags;
} CoreGraphicsStateSetBlittingFlags;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetBlittingFlagsReturn;


/*
 * CoreGraphicsState_SetClip
 */
typedef struct {
    DFBRegion                                  region;
} CoreGraphicsStateSetClip;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetClipReturn;


/*
 * CoreGraphicsState_SetColor
 */
typedef struct {
    DFBColor                                   color;
} CoreGraphicsStateSetColor;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetColorReturn;


/*
 * CoreGraphicsState_SetColorAndIndex
 */
typedef struct {
    DFBColor                                   color;
    u32                                        index;
} CoreGraphicsStateSetColorAndIndex;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetColorAndIndexReturn;


/*
 * CoreGraphicsState_SetSrcBlend
 */
typedef struct {
    DFBSurfaceBlendFunction                    function;
} CoreGraphicsStateSetSrcBlend;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetSrcBlendReturn;


/*
 * CoreGraphicsState_SetDstBlend
 */
typedef struct {
    DFBSurfaceBlendFunction                    function;
} CoreGraphicsStateSetDstBlend;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetDstBlendReturn;


/*
 * CoreGraphicsState_SetSrcColorKey
 */
typedef struct {
    u32                                        key;
} CoreGraphicsStateSetSrcColorKey;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetSrcColorKeyReturn;


/*
 * CoreGraphicsState_SetDstColorKey
 */
typedef struct {
    u32                                        key;
} CoreGraphicsStateSetDstColorKey;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetDstColorKeyReturn;


/*
 * CoreGraphicsState_SetDestination
 */
typedef struct {
    u32                                        surface_id;
} CoreGraphicsStateSetDestination;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetDestinationReturn;


/*
 * CoreGraphicsState_SetSource
 */
typedef struct {
    u32                                        surface_id;
} CoreGraphicsStateSetSource;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetSourceReturn;


/*
 * CoreGraphicsState_SetSourceMask
 */
typedef struct {
    u32                                        surface_id;
} CoreGraphicsStateSetSourceMask;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetSourceMaskReturn;


/*
 * CoreGraphicsState_SetSourceMaskVals
 */
typedef struct {
    DFBPoint                                   offset;
    DFBSurfaceMaskFlags                        flags;
} CoreGraphicsStateSetSourceMaskVals;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetSourceMaskValsReturn;


/*
 * CoreGraphicsState_SetIndexTranslation
 */
typedef struct {
    u32                                        num;
    /* 'num' s32 follow (indices) */
} CoreGraphicsStateSetIndexTranslation;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetIndexTranslationReturn;


/*
 * CoreGraphicsState_SetColorKey
 */
typedef struct {
    DFBColorKey                                key;
} CoreGraphicsStateSetColorKey;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetColorKeyReturn;


/*
 * CoreGraphicsState_SetRenderOptions
 */
typedef struct {
    DFBSurfaceRenderOptions                    options;
} CoreGraphicsStateSetRenderOptions;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetRenderOptionsReturn;


/*
 * CoreGraphicsState_SetMatrix
 */
typedef struct {
    /* '9' s32 follow (values) */
} CoreGraphicsStateSetMatrix;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetMatrixReturn;


/*
 * CoreGraphicsState_SetSource2
 */
typedef struct {
    u32                                        surface_id;
} CoreGraphicsStateSetSource2;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateSetSource2Return;


/*
 * CoreGraphicsState_DrawRectangles
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBRectangle follow (rects) */
} CoreGraphicsStateDrawRectangles;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateDrawRectanglesReturn;


/*
 * CoreGraphicsState_DrawLines
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBRegion follow (lines) */
} CoreGraphicsStateDrawLines;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateDrawLinesReturn;


/*
 * CoreGraphicsState_FillRectangles
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBRectangle follow (rects) */
} CoreGraphicsStateFillRectangles;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateFillRectanglesReturn;


/*
 * CoreGraphicsState_FillTriangles
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBTriangle follow (triangles) */
} CoreGraphicsStateFillTriangles;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateFillTrianglesReturn;


/*
 * CoreGraphicsState_FillTrapezoids
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBTrapezoid follow (trapezoids) */
} CoreGraphicsStateFillTrapezoids;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateFillTrapezoidsReturn;


/*
 * CoreGraphicsState_FillSpans
 */
typedef struct {
    s32                                        y;
    u32                                        num;
    /* 'num' DFBSpan follow (spans) */
} CoreGraphicsStateFillSpans;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateFillSpansReturn;


/*
 * CoreGraphicsState_Blit
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBRectangle follow (rects) */
    /* 'num' DFBPoint follow (points) */
} CoreGraphicsStateBlit;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateBlitReturn;


/*
 * CoreGraphicsState_Blit2
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBRectangle follow (rects) */
    /* 'num' DFBPoint follow (points1) */
    /* 'num' DFBPoint follow (points2) */
} CoreGraphicsStateBlit2;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateBlit2Return;


/*
 * CoreGraphicsState_StretchBlit
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBRectangle follow (srects) */
    /* 'num' DFBRectangle follow (drects) */
} CoreGraphicsStateStretchBlit;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateStretchBlitReturn;


/*
 * CoreGraphicsState_TileBlit
 */
typedef struct {
    u32                                        num;
    /* 'num' DFBRectangle follow (rects) */
    /* 'num' DFBPoint follow (points1) */
    /* 'num' DFBPoint follow (points2) */
} CoreGraphicsStateTileBlit;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateTileBlitReturn;


/*
 * CoreGraphicsState_TextureTriangles
 */
typedef struct {
    u32                                        num;
    DFBTriangleFormation                       formation;
    /* 'num' DFBVertex follow (vertices) */
} CoreGraphicsStateTextureTriangles;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateTextureTrianglesReturn;


/*
 * CoreGraphicsState_ReleaseSource
 */
typedef struct {
} CoreGraphicsStateReleaseSource;

typedef struct {
    DFBResult                                  result;
} CoreGraphicsStateReleaseSourceReturn;


DFBResult IGraphicsState_Real__SetDrawingFlags( CoreGraphicsState *obj,
                    DFBSurfaceDrawingFlags                     flags );

DFBResult IGraphicsState_Real__SetBlittingFlags( CoreGraphicsState *obj,
                    DFBSurfaceBlittingFlags                    flags );

DFBResult IGraphicsState_Real__SetClip( CoreGraphicsState *obj,
                    const DFBRegion                           *region );

DFBResult IGraphicsState_Real__SetColor( CoreGraphicsState *obj,
                    const DFBColor                            *color );

DFBResult IGraphicsState_Real__SetColorAndIndex( CoreGraphicsState *obj,
                    const DFBColor                            *color,
                    u32                                        index );

DFBResult IGraphicsState_Real__SetSrcBlend( CoreGraphicsState *obj,
                    DFBSurfaceBlendFunction                    function );

DFBResult IGraphicsState_Real__SetDstBlend( CoreGraphicsState *obj,
                    DFBSurfaceBlendFunction                    function );

DFBResult IGraphicsState_Real__SetSrcColorKey( CoreGraphicsState *obj,
                    u32                                        key );

DFBResult IGraphicsState_Real__SetDstColorKey( CoreGraphicsState *obj,
                    u32                                        key );

DFBResult IGraphicsState_Real__SetDestination( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Real__SetSource( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Real__SetSourceMask( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Real__SetSourceMaskVals( CoreGraphicsState *obj,
                    const DFBPoint                            *offset,
                    DFBSurfaceMaskFlags                        flags );

DFBResult IGraphicsState_Real__SetIndexTranslation( CoreGraphicsState *obj,
                    const s32                                 *indices,
                    u32                                        num );

DFBResult IGraphicsState_Real__SetColorKey( CoreGraphicsState *obj,
                    const DFBColorKey                         *key );

DFBResult IGraphicsState_Real__SetRenderOptions( CoreGraphicsState *obj,
                    DFBSurfaceRenderOptions                    options );

DFBResult IGraphicsState_Real__SetMatrix( CoreGraphicsState *obj,
                    const s32                                 *values );

DFBResult IGraphicsState_Real__SetSource2( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Real__DrawRectangles( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num );

DFBResult IGraphicsState_Real__DrawLines( CoreGraphicsState *obj,
                    const DFBRegion                           *lines,
                    u32                                        num );

DFBResult IGraphicsState_Real__FillRectangles( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num );

DFBResult IGraphicsState_Real__FillTriangles( CoreGraphicsState *obj,
                    const DFBTriangle                         *triangles,
                    u32                                        num );

DFBResult IGraphicsState_Real__FillTrapezoids( CoreGraphicsState *obj,
                    const DFBTrapezoid                        *trapezoids,
                    u32                                        num );

DFBResult IGraphicsState_Real__FillSpans( CoreGraphicsState *obj,
                    s32                                        y,
                    const DFBSpan                             *spans,
                    u32                                        num );

DFBResult IGraphicsState_Real__Blit( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points,
                    u32                                        num );

DFBResult IGraphicsState_Real__Blit2( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num );

DFBResult IGraphicsState_Real__StretchBlit( CoreGraphicsState *obj,
                    const DFBRectangle                        *srects,
                    const DFBRectangle                        *drects,
                    u32                                        num );

DFBResult IGraphicsState_Real__TileBlit( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num );

DFBResult IGraphicsState_Real__TextureTriangles( CoreGraphicsState *obj,
                    const DFBVertex                           *vertices,
                    u32                                        num,
                    DFBTriangleFormation                       formation );

DFBResult IGraphicsState_Real__ReleaseSource( CoreGraphicsState *obj
 );

DFBResult IGraphicsState_Requestor__SetDrawingFlags( CoreGraphicsState *obj,
                    DFBSurfaceDrawingFlags                     flags );

DFBResult IGraphicsState_Requestor__SetBlittingFlags( CoreGraphicsState *obj,
                    DFBSurfaceBlittingFlags                    flags );

DFBResult IGraphicsState_Requestor__SetClip( CoreGraphicsState *obj,
                    const DFBRegion                           *region );

DFBResult IGraphicsState_Requestor__SetColor( CoreGraphicsState *obj,
                    const DFBColor                            *color );

DFBResult IGraphicsState_Requestor__SetColorAndIndex( CoreGraphicsState *obj,
                    const DFBColor                            *color,
                    u32                                        index );

DFBResult IGraphicsState_Requestor__SetSrcBlend( CoreGraphicsState *obj,
                    DFBSurfaceBlendFunction                    function );

DFBResult IGraphicsState_Requestor__SetDstBlend( CoreGraphicsState *obj,
                    DFBSurfaceBlendFunction                    function );

DFBResult IGraphicsState_Requestor__SetSrcColorKey( CoreGraphicsState *obj,
                    u32                                        key );

DFBResult IGraphicsState_Requestor__SetDstColorKey( CoreGraphicsState *obj,
                    u32                                        key );

DFBResult IGraphicsState_Requestor__SetDestination( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Requestor__SetSource( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Requestor__SetSourceMask( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Requestor__SetSourceMaskVals( CoreGraphicsState *obj,
                    const DFBPoint                            *offset,
                    DFBSurfaceMaskFlags                        flags );

DFBResult IGraphicsState_Requestor__SetIndexTranslation( CoreGraphicsState *obj,
                    const s32                                 *indices,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__SetColorKey( CoreGraphicsState *obj,
                    const DFBColorKey                         *key );

DFBResult IGraphicsState_Requestor__SetRenderOptions( CoreGraphicsState *obj,
                    DFBSurfaceRenderOptions                    options );

DFBResult IGraphicsState_Requestor__SetMatrix( CoreGraphicsState *obj,
                    const s32                                 *values );

DFBResult IGraphicsState_Requestor__SetSource2( CoreGraphicsState *obj,
                    CoreSurface                               *surface );

DFBResult IGraphicsState_Requestor__DrawRectangles( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__DrawLines( CoreGraphicsState *obj,
                    const DFBRegion                           *lines,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__FillRectangles( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__FillTriangles( CoreGraphicsState *obj,
                    const DFBTriangle                         *triangles,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__FillTrapezoids( CoreGraphicsState *obj,
                    const DFBTrapezoid                        *trapezoids,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__FillSpans( CoreGraphicsState *obj,
                    s32                                        y,
                    const DFBSpan                             *spans,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__Blit( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__Blit2( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__StretchBlit( CoreGraphicsState *obj,
                    const DFBRectangle                        *srects,
                    const DFBRectangle                        *drects,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__TileBlit( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num );

DFBResult IGraphicsState_Requestor__TextureTriangles( CoreGraphicsState *obj,
                    const DFBVertex                           *vertices,
                    u32                                        num,
                    DFBTriangleFormation                       formation );

DFBResult IGraphicsState_Requestor__ReleaseSource( CoreGraphicsState *obj
 );


DFBResult CoreGraphicsStateDispatch__Dispatch( CoreGraphicsState *obj,
                    FusionID      caller,
                    int           method,
                    void         *ptr,
                    unsigned int  length,
                    void         *ret_ptr,
                    unsigned int  ret_size,
                    unsigned int *ret_length );


#endif
