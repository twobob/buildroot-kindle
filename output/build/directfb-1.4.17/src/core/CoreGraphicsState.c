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

#include "CoreGraphicsState.h"

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <fusion/conf.h>

#include <core/core.h>

#include <core/CoreDFB_CallMode.h>

D_DEBUG_DOMAIN( DirectFB_CoreGraphicsState, "DirectFB/CoreGraphicsState", "DirectFB CoreGraphicsState" );

/*********************************************************************************************************************/

DFBResult
CoreGraphicsState_SetDrawingFlags(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceDrawingFlags                     flags
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetDrawingFlags( obj, flags );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetDrawingFlags( obj, flags );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetBlittingFlags(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceBlittingFlags                    flags
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetBlittingFlags( obj, flags );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetBlittingFlags( obj, flags );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetClip(
                    CoreGraphicsState                         *obj,
                    const DFBRegion                           *region
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetClip( obj, region );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetClip( obj, region );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetColor(
                    CoreGraphicsState                         *obj,
                    const DFBColor                            *color
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetColor( obj, color );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetColor( obj, color );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetColorAndIndex(
                    CoreGraphicsState                         *obj,
                    const DFBColor                            *color,
                    u32                                        index
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetColorAndIndex( obj, color, index );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetColorAndIndex( obj, color, index );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetSrcBlend(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceBlendFunction                    function
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetSrcBlend( obj, function );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetSrcBlend( obj, function );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetDstBlend(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceBlendFunction                    function
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetDstBlend( obj, function );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetDstBlend( obj, function );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetSrcColorKey(
                    CoreGraphicsState                         *obj,
                    u32                                        key
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetSrcColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetSrcColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetDstColorKey(
                    CoreGraphicsState                         *obj,
                    u32                                        key
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetDstColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetDstColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetDestination(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetDestination( obj, surface );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetDestination( obj, surface );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetSource(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetSource( obj, surface );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetSource( obj, surface );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetSourceMask(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetSourceMask( obj, surface );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetSourceMask( obj, surface );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetSourceMaskVals(
                    CoreGraphicsState                         *obj,
                    const DFBPoint                            *offset,
                    DFBSurfaceMaskFlags                        flags
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetSourceMaskVals( obj, offset, flags );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetSourceMaskVals( obj, offset, flags );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetIndexTranslation(
                    CoreGraphicsState                         *obj,
                    const s32                                 *indices,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetIndexTranslation( obj, indices, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetIndexTranslation( obj, indices, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetColorKey(
                    CoreGraphicsState                         *obj,
                    const DFBColorKey                         *key
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetColorKey( obj, key );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetRenderOptions(
                    CoreGraphicsState                         *obj,
                    DFBSurfaceRenderOptions                    options
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetRenderOptions( obj, options );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetRenderOptions( obj, options );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetMatrix(
                    CoreGraphicsState                         *obj,
                    const s32                                 *values
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetMatrix( obj, values );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetMatrix( obj, values );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_SetSource2(
                    CoreGraphicsState                         *obj,
                    CoreSurface                               *surface
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__SetSource2( obj, surface );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__SetSource2( obj, surface );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_DrawRectangles(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__DrawRectangles( obj, rects, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__DrawRectangles( obj, rects, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_DrawLines(
                    CoreGraphicsState                         *obj,
                    const DFBRegion                           *lines,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__DrawLines( obj, lines, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__DrawLines( obj, lines, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_FillRectangles(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__FillRectangles( obj, rects, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__FillRectangles( obj, rects, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_FillTriangles(
                    CoreGraphicsState                         *obj,
                    const DFBTriangle                         *triangles,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__FillTriangles( obj, triangles, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__FillTriangles( obj, triangles, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_FillTrapezoids(
                    CoreGraphicsState                         *obj,
                    const DFBTrapezoid                        *trapezoids,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__FillTrapezoids( obj, trapezoids, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__FillTrapezoids( obj, trapezoids, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_FillSpans(
                    CoreGraphicsState                         *obj,
                    s32                                        y,
                    const DFBSpan                             *spans,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__FillSpans( obj, y, spans, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__FillSpans( obj, y, spans, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_Blit(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__Blit( obj, rects, points, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__Blit( obj, rects, points, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_Blit2(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__Blit2( obj, rects, points1, points2, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__Blit2( obj, rects, points1, points2, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_StretchBlit(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *srects,
                    const DFBRectangle                        *drects,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__StretchBlit( obj, srects, drects, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__StretchBlit( obj, srects, drects, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_TileBlit(
                    CoreGraphicsState                         *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__TileBlit( obj, rects, points1, points2, num );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__TileBlit( obj, rects, points1, points2, num );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_TextureTriangles(
                    CoreGraphicsState                         *obj,
                    const DFBVertex                           *vertices,
                    u32                                        num,
                    DFBTriangleFormation                       formation
)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__TextureTriangles( obj, vertices, num, formation );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__TextureTriangles( obj, vertices, num, formation );
            Core_PopCalling();

            return ret;
        }
        case COREDFB_CALL_DENY:
            return DFB_DEAD;
    }

    return DFB_UNIMPLEMENTED;
}

DFBResult
CoreGraphicsState_ReleaseSource(
                    CoreGraphicsState                         *obj

)
{
    DFBResult ret;

    switch (CoreDFB_CallMode( core_dfb )) {
        case COREDFB_CALL_DIRECT:{
            Core_PushCalling();
            ret = IGraphicsState_Real__ReleaseSource( obj );
            Core_PopCalling();

            return ret;
        }

        case COREDFB_CALL_INDIRECT: {
            Core_PushCalling();
            ret = IGraphicsState_Requestor__ReleaseSource( obj );
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
CoreGraphicsState_Dispatch( int           caller,   /* fusion id of the caller */
                     int           call_arg, /* optional call parameter */
                     void         *ptr, /* optional call parameter */
                     unsigned int  length,
                     void         *ctx,      /* optional handler context */
                     unsigned int  serial,
                     void         *ret_ptr,
                     unsigned int  ret_size,
                     unsigned int *ret_length )
{
    CoreGraphicsState *obj = (CoreGraphicsState*) ctx;
    CoreGraphicsStateDispatch__Dispatch( obj, caller, call_arg, ptr, length, ret_ptr, ret_size, ret_length );

    return FCHR_RETURN;
}

void CoreGraphicsState_Init_Dispatch(
                    CoreDFB              *core,
                    CoreGraphicsState    *obj,
                    FusionCall           *call
)
{
    fusion_call_init3( call, CoreGraphicsState_Dispatch, obj, core->world );
}

void  CoreGraphicsState_Deinit_Dispatch(
                    FusionCall           *call
)
{
     fusion_call_destroy( call );
}

/*********************************************************************************************************************/


DFBResult
IGraphicsState_Requestor__SetDrawingFlags( CoreGraphicsState *obj,
                    DFBSurfaceDrawingFlags                     flags
)
{
    DFBResult           ret;
    CoreGraphicsStateSetDrawingFlags       *args = (CoreGraphicsStateSetDrawingFlags*) alloca( sizeof(CoreGraphicsStateSetDrawingFlags) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );


    args->flags = flags;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetDrawingFlags, args, sizeof(CoreGraphicsStateSetDrawingFlags), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetDrawingFlags ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetBlittingFlags( CoreGraphicsState *obj,
                    DFBSurfaceBlittingFlags                    flags
)
{
    DFBResult           ret;
    CoreGraphicsStateSetBlittingFlags       *args = (CoreGraphicsStateSetBlittingFlags*) alloca( sizeof(CoreGraphicsStateSetBlittingFlags) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );


    args->flags = flags;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetBlittingFlags, args, sizeof(CoreGraphicsStateSetBlittingFlags), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetBlittingFlags ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetClip( CoreGraphicsState *obj,
                    const DFBRegion                           *region
)
{
    DFBResult           ret;
    CoreGraphicsStateSetClip       *args = (CoreGraphicsStateSetClip*) alloca( sizeof(CoreGraphicsStateSetClip) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( region != NULL );

    args->region = *region;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetClip, args, sizeof(CoreGraphicsStateSetClip), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetClip ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetColor( CoreGraphicsState *obj,
                    const DFBColor                            *color
)
{
    DFBResult           ret;
    CoreGraphicsStateSetColor       *args = (CoreGraphicsStateSetColor*) alloca( sizeof(CoreGraphicsStateSetColor) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( color != NULL );

    args->color = *color;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetColor, args, sizeof(CoreGraphicsStateSetColor), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetColor ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetColorAndIndex( CoreGraphicsState *obj,
                    const DFBColor                            *color,
                    u32                                        index
)
{
    DFBResult           ret;
    CoreGraphicsStateSetColorAndIndex       *args = (CoreGraphicsStateSetColorAndIndex*) alloca( sizeof(CoreGraphicsStateSetColorAndIndex) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( color != NULL );

    args->color = *color;
    args->index = index;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetColorAndIndex, args, sizeof(CoreGraphicsStateSetColorAndIndex), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetColorAndIndex ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetSrcBlend( CoreGraphicsState *obj,
                    DFBSurfaceBlendFunction                    function
)
{
    DFBResult           ret;
    CoreGraphicsStateSetSrcBlend       *args = (CoreGraphicsStateSetSrcBlend*) alloca( sizeof(CoreGraphicsStateSetSrcBlend) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );


    args->function = function;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetSrcBlend, args, sizeof(CoreGraphicsStateSetSrcBlend), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetSrcBlend ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetDstBlend( CoreGraphicsState *obj,
                    DFBSurfaceBlendFunction                    function
)
{
    DFBResult           ret;
    CoreGraphicsStateSetDstBlend       *args = (CoreGraphicsStateSetDstBlend*) alloca( sizeof(CoreGraphicsStateSetDstBlend) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );


    args->function = function;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetDstBlend, args, sizeof(CoreGraphicsStateSetDstBlend), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetDstBlend ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetSrcColorKey( CoreGraphicsState *obj,
                    u32                                        key
)
{
    DFBResult           ret;
    CoreGraphicsStateSetSrcColorKey       *args = (CoreGraphicsStateSetSrcColorKey*) alloca( sizeof(CoreGraphicsStateSetSrcColorKey) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );


    args->key = key;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetSrcColorKey, args, sizeof(CoreGraphicsStateSetSrcColorKey), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetSrcColorKey ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetDstColorKey( CoreGraphicsState *obj,
                    u32                                        key
)
{
    DFBResult           ret;
    CoreGraphicsStateSetDstColorKey       *args = (CoreGraphicsStateSetDstColorKey*) alloca( sizeof(CoreGraphicsStateSetDstColorKey) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );


    args->key = key;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetDstColorKey, args, sizeof(CoreGraphicsStateSetDstColorKey), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetDstColorKey ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetDestination( CoreGraphicsState *obj,
                    CoreSurface                               *surface
)
{
    DFBResult           ret;
    CoreGraphicsStateSetDestination       *args = (CoreGraphicsStateSetDestination*) alloca( sizeof(CoreGraphicsStateSetDestination) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( surface != NULL );

    args->surface_id = CoreSurface_GetID( surface );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY), _CoreGraphicsState_SetDestination, args, sizeof(CoreGraphicsStateSetDestination), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetDestination ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetSource( CoreGraphicsState *obj,
                    CoreSurface                               *surface
)
{
    DFBResult           ret;
    CoreGraphicsStateSetSource       *args = (CoreGraphicsStateSetSource*) alloca( sizeof(CoreGraphicsStateSetSource) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( surface != NULL );

    args->surface_id = CoreSurface_GetID( surface );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY), _CoreGraphicsState_SetSource, args, sizeof(CoreGraphicsStateSetSource), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetSource ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetSourceMask( CoreGraphicsState *obj,
                    CoreSurface                               *surface
)
{
    DFBResult           ret;
    CoreGraphicsStateSetSourceMask       *args = (CoreGraphicsStateSetSourceMask*) alloca( sizeof(CoreGraphicsStateSetSourceMask) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( surface != NULL );

    args->surface_id = CoreSurface_GetID( surface );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY), _CoreGraphicsState_SetSourceMask, args, sizeof(CoreGraphicsStateSetSourceMask), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetSourceMask ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetSourceMaskVals( CoreGraphicsState *obj,
                    const DFBPoint                            *offset,
                    DFBSurfaceMaskFlags                        flags
)
{
    DFBResult           ret;
    CoreGraphicsStateSetSourceMaskVals       *args = (CoreGraphicsStateSetSourceMaskVals*) alloca( sizeof(CoreGraphicsStateSetSourceMaskVals) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( offset != NULL );

    args->offset = *offset;
    args->flags = flags;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetSourceMaskVals, args, sizeof(CoreGraphicsStateSetSourceMaskVals), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetSourceMaskVals ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetIndexTranslation( CoreGraphicsState *obj,
                    const s32                                 *indices,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateSetIndexTranslation       *args = (CoreGraphicsStateSetIndexTranslation*) alloca( sizeof(CoreGraphicsStateSetIndexTranslation) + num * sizeof(s32) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( indices != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), indices, num * sizeof(s32) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetIndexTranslation, args, sizeof(CoreGraphicsStateSetIndexTranslation) + num * sizeof(s32), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetIndexTranslation ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetColorKey( CoreGraphicsState *obj,
                    const DFBColorKey                         *key
)
{
    DFBResult           ret;
    CoreGraphicsStateSetColorKey       *args = (CoreGraphicsStateSetColorKey*) alloca( sizeof(CoreGraphicsStateSetColorKey) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( key != NULL );

    args->key = *key;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetColorKey, args, sizeof(CoreGraphicsStateSetColorKey), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetColorKey ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetRenderOptions( CoreGraphicsState *obj,
                    DFBSurfaceRenderOptions                    options
)
{
    DFBResult           ret;
    CoreGraphicsStateSetRenderOptions       *args = (CoreGraphicsStateSetRenderOptions*) alloca( sizeof(CoreGraphicsStateSetRenderOptions) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );


    args->options = options;

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetRenderOptions, args, sizeof(CoreGraphicsStateSetRenderOptions), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetRenderOptions ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetMatrix( CoreGraphicsState *obj,
                    const s32                                 *values
)
{
    DFBResult           ret;
    CoreGraphicsStateSetMatrix       *args = (CoreGraphicsStateSetMatrix*) alloca( sizeof(CoreGraphicsStateSetMatrix) + 9 * sizeof(s32) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( values != NULL );

    direct_memcpy( (char*) (args + 1), values, 9 * sizeof(s32) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_SetMatrix, args, sizeof(CoreGraphicsStateSetMatrix) + 9 * sizeof(s32), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetMatrix ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__SetSource2( CoreGraphicsState *obj,
                    CoreSurface                               *surface
)
{
    DFBResult           ret;
    CoreGraphicsStateSetSource2       *args = (CoreGraphicsStateSetSource2*) alloca( sizeof(CoreGraphicsStateSetSource2) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( surface != NULL );

    args->surface_id = CoreSurface_GetID( surface );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY), _CoreGraphicsState_SetSource2, args, sizeof(CoreGraphicsStateSetSource2), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_SetSource2 ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__DrawRectangles( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateDrawRectangles       *args = (CoreGraphicsStateDrawRectangles*) alloca( sizeof(CoreGraphicsStateDrawRectangles) + num * sizeof(DFBRectangle) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( rects != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), rects, num * sizeof(DFBRectangle) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_DrawRectangles, args, sizeof(CoreGraphicsStateDrawRectangles) + num * sizeof(DFBRectangle), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_DrawRectangles ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__DrawLines( CoreGraphicsState *obj,
                    const DFBRegion                           *lines,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateDrawLines       *args = (CoreGraphicsStateDrawLines*) alloca( sizeof(CoreGraphicsStateDrawLines) + num * sizeof(DFBRegion) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( lines != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), lines, num * sizeof(DFBRegion) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_DrawLines, args, sizeof(CoreGraphicsStateDrawLines) + num * sizeof(DFBRegion), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_DrawLines ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__FillRectangles( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateFillRectangles       *args = (CoreGraphicsStateFillRectangles*) alloca( sizeof(CoreGraphicsStateFillRectangles) + num * sizeof(DFBRectangle) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( rects != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), rects, num * sizeof(DFBRectangle) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_FillRectangles, args, sizeof(CoreGraphicsStateFillRectangles) + num * sizeof(DFBRectangle), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_FillRectangles ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__FillTriangles( CoreGraphicsState *obj,
                    const DFBTriangle                         *triangles,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateFillTriangles       *args = (CoreGraphicsStateFillTriangles*) alloca( sizeof(CoreGraphicsStateFillTriangles) + num * sizeof(DFBTriangle) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( triangles != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), triangles, num * sizeof(DFBTriangle) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_FillTriangles, args, sizeof(CoreGraphicsStateFillTriangles) + num * sizeof(DFBTriangle), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_FillTriangles ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__FillTrapezoids( CoreGraphicsState *obj,
                    const DFBTrapezoid                        *trapezoids,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateFillTrapezoids       *args = (CoreGraphicsStateFillTrapezoids*) alloca( sizeof(CoreGraphicsStateFillTrapezoids) + num * sizeof(DFBTrapezoid) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( trapezoids != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), trapezoids, num * sizeof(DFBTrapezoid) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_FillTrapezoids, args, sizeof(CoreGraphicsStateFillTrapezoids) + num * sizeof(DFBTrapezoid), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_FillTrapezoids ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__FillSpans( CoreGraphicsState *obj,
                    s32                                        y,
                    const DFBSpan                             *spans,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateFillSpans       *args = (CoreGraphicsStateFillSpans*) alloca( sizeof(CoreGraphicsStateFillSpans) + num * sizeof(DFBSpan) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( spans != NULL );

    args->y = y;
    args->num = num;
    direct_memcpy( (char*) (args + 1), spans, num * sizeof(DFBSpan) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_FillSpans, args, sizeof(CoreGraphicsStateFillSpans) + num * sizeof(DFBSpan), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_FillSpans ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__Blit( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateBlit       *args = (CoreGraphicsStateBlit*) alloca( sizeof(CoreGraphicsStateBlit) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( rects != NULL );
    D_ASSERT( points != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), rects, num * sizeof(DFBRectangle) );
    direct_memcpy( (char*) (args + 1) + num * sizeof(DFBRectangle), points, num * sizeof(DFBPoint) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_Blit, args, sizeof(CoreGraphicsStateBlit) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_Blit ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__Blit2( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateBlit2       *args = (CoreGraphicsStateBlit2*) alloca( sizeof(CoreGraphicsStateBlit2) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint) + num * sizeof(DFBPoint) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( rects != NULL );
    D_ASSERT( points1 != NULL );
    D_ASSERT( points2 != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), rects, num * sizeof(DFBRectangle) );
    direct_memcpy( (char*) (args + 1) + num * sizeof(DFBRectangle), points1, num * sizeof(DFBPoint) );
    direct_memcpy( (char*) (args + 1) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint), points2, num * sizeof(DFBPoint) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_Blit2, args, sizeof(CoreGraphicsStateBlit2) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint) + num * sizeof(DFBPoint), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_Blit2 ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__StretchBlit( CoreGraphicsState *obj,
                    const DFBRectangle                        *srects,
                    const DFBRectangle                        *drects,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateStretchBlit       *args = (CoreGraphicsStateStretchBlit*) alloca( sizeof(CoreGraphicsStateStretchBlit) + num * sizeof(DFBRectangle) + num * sizeof(DFBRectangle) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( srects != NULL );
    D_ASSERT( drects != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), srects, num * sizeof(DFBRectangle) );
    direct_memcpy( (char*) (args + 1) + num * sizeof(DFBRectangle), drects, num * sizeof(DFBRectangle) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_StretchBlit, args, sizeof(CoreGraphicsStateStretchBlit) + num * sizeof(DFBRectangle) + num * sizeof(DFBRectangle), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_StretchBlit ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__TileBlit( CoreGraphicsState *obj,
                    const DFBRectangle                        *rects,
                    const DFBPoint                            *points1,
                    const DFBPoint                            *points2,
                    u32                                        num
)
{
    DFBResult           ret;
    CoreGraphicsStateTileBlit       *args = (CoreGraphicsStateTileBlit*) alloca( sizeof(CoreGraphicsStateTileBlit) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint) + num * sizeof(DFBPoint) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( rects != NULL );
    D_ASSERT( points1 != NULL );
    D_ASSERT( points2 != NULL );

    args->num = num;
    direct_memcpy( (char*) (args + 1), rects, num * sizeof(DFBRectangle) );
    direct_memcpy( (char*) (args + 1) + num * sizeof(DFBRectangle), points1, num * sizeof(DFBPoint) );
    direct_memcpy( (char*) (args + 1) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint), points2, num * sizeof(DFBPoint) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_TileBlit, args, sizeof(CoreGraphicsStateTileBlit) + num * sizeof(DFBRectangle) + num * sizeof(DFBPoint) + num * sizeof(DFBPoint), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_TileBlit ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__TextureTriangles( CoreGraphicsState *obj,
                    const DFBVertex                           *vertices,
                    u32                                        num,
                    DFBTriangleFormation                       formation
)
{
    DFBResult           ret;
    CoreGraphicsStateTextureTriangles       *args = (CoreGraphicsStateTextureTriangles*) alloca( sizeof(CoreGraphicsStateTextureTriangles) + num * sizeof(DFBVertex) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( vertices != NULL );

    args->num = num;
    args->formation = formation;
    direct_memcpy( (char*) (args + 1), vertices, num * sizeof(DFBVertex) );

    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_TextureTriangles, args, sizeof(CoreGraphicsStateTextureTriangles) + num * sizeof(DFBVertex), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_TextureTriangles ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}


DFBResult
IGraphicsState_Requestor__ReleaseSource( CoreGraphicsState *obj

)
{
    DFBResult           ret;
    CoreGraphicsStateReleaseSource       *args = (CoreGraphicsStateReleaseSource*) alloca( sizeof(CoreGraphicsStateReleaseSource) );

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "IGraphicsState_Requestor::%s()\n", __FUNCTION__ );



    ret = (DFBResult) CoreGraphicsState_Call( obj, (FusionCallExecFlags)(FCEF_ONEWAY | FCEF_QUEUE), _CoreGraphicsState_ReleaseSource, args, sizeof(CoreGraphicsStateReleaseSource), NULL, 0, NULL );
    if (ret) {
        D_DERROR( ret, "%s: CoreGraphicsState_Call( CoreGraphicsState_ReleaseSource ) failed!\n", __FUNCTION__ );
        return ret;
    }


    return DFB_OK;
}

/*********************************************************************************************************************/

static DFBResult
__CoreGraphicsStateDispatch__Dispatch( CoreGraphicsState *obj,
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
        case _CoreGraphicsState_SetDrawingFlags: {
            D_UNUSED
            CoreGraphicsStateSetDrawingFlags       *args        = (CoreGraphicsStateSetDrawingFlags *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetDrawingFlags\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> flags = %x\n", args->flags );

            IGraphicsState_Real__SetDrawingFlags( obj, args->flags );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetBlittingFlags: {
            D_UNUSED
            CoreGraphicsStateSetBlittingFlags       *args        = (CoreGraphicsStateSetBlittingFlags *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetBlittingFlags\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> flags = %x\n", args->flags );

            IGraphicsState_Real__SetBlittingFlags( obj, args->flags );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetClip: {
            D_UNUSED
            CoreGraphicsStateSetClip       *args        = (CoreGraphicsStateSetClip *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetClip\n" );

         ;    // TODO: DFBRegion_debug args->region;

            IGraphicsState_Real__SetClip( obj, &args->region );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetColor: {
            D_UNUSED
            CoreGraphicsStateSetColor       *args        = (CoreGraphicsStateSetColor *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetColor\n" );

         ;    // TODO: DFBColor_debug args->color;

            IGraphicsState_Real__SetColor( obj, &args->color );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetColorAndIndex: {
            D_UNUSED
            CoreGraphicsStateSetColorAndIndex       *args        = (CoreGraphicsStateSetColorAndIndex *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetColorAndIndex\n" );

         ;    // TODO: DFBColor_debug args->color;
            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> index = %u\n", args->index );

            IGraphicsState_Real__SetColorAndIndex( obj, &args->color, args->index );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetSrcBlend: {
            D_UNUSED
            CoreGraphicsStateSetSrcBlend       *args        = (CoreGraphicsStateSetSrcBlend *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetSrcBlend\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> function = %x\n", args->function );

            IGraphicsState_Real__SetSrcBlend( obj, args->function );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetDstBlend: {
            D_UNUSED
            CoreGraphicsStateSetDstBlend       *args        = (CoreGraphicsStateSetDstBlend *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetDstBlend\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> function = %x\n", args->function );

            IGraphicsState_Real__SetDstBlend( obj, args->function );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetSrcColorKey: {
            D_UNUSED
            CoreGraphicsStateSetSrcColorKey       *args        = (CoreGraphicsStateSetSrcColorKey *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetSrcColorKey\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> key = %u\n", args->key );

            IGraphicsState_Real__SetSrcColorKey( obj, args->key );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetDstColorKey: {
            D_UNUSED
            CoreGraphicsStateSetDstColorKey       *args        = (CoreGraphicsStateSetDstColorKey *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetDstColorKey\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> key = %u\n", args->key );

            IGraphicsState_Real__SetDstColorKey( obj, args->key );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetDestination: {
    CoreSurface *surface = NULL;
            D_UNUSED
            CoreGraphicsStateSetDestination       *args        = (CoreGraphicsStateSetDestination *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetDestination\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> surface = %d\n", args->surface_id );

            ret = (DFBResult) CoreSurface_Lookup( core_dfb, args->surface_id, caller, &surface );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up surface by ID %u failed!\n", __FUNCTION__, args->surface_id );
                 return DFB_OK;
            }

            IGraphicsState_Real__SetDestination( obj, surface );

            if (surface)
                CoreSurface_Unref( surface );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetSource: {
    CoreSurface *surface = NULL;
            D_UNUSED
            CoreGraphicsStateSetSource       *args        = (CoreGraphicsStateSetSource *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetSource\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> surface = %d\n", args->surface_id );

            ret = (DFBResult) CoreSurface_Lookup( core_dfb, args->surface_id, caller, &surface );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up surface by ID %u failed!\n", __FUNCTION__, args->surface_id );
                 return DFB_OK;
            }

            IGraphicsState_Real__SetSource( obj, surface );

            if (surface)
                CoreSurface_Unref( surface );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetSourceMask: {
    CoreSurface *surface = NULL;
            D_UNUSED
            CoreGraphicsStateSetSourceMask       *args        = (CoreGraphicsStateSetSourceMask *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetSourceMask\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> surface = %d\n", args->surface_id );

            ret = (DFBResult) CoreSurface_Lookup( core_dfb, args->surface_id, caller, &surface );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up surface by ID %u failed!\n", __FUNCTION__, args->surface_id );
                 return DFB_OK;
            }

            IGraphicsState_Real__SetSourceMask( obj, surface );

            if (surface)
                CoreSurface_Unref( surface );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetSourceMaskVals: {
            D_UNUSED
            CoreGraphicsStateSetSourceMaskVals       *args        = (CoreGraphicsStateSetSourceMaskVals *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetSourceMaskVals\n" );

         ;    // TODO: DFBPoint_debug args->offset;
            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> flags = %x\n", args->flags );

            IGraphicsState_Real__SetSourceMaskVals( obj, &args->offset, args->flags );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetIndexTranslation: {
            D_UNUSED
            CoreGraphicsStateSetIndexTranslation       *args        = (CoreGraphicsStateSetIndexTranslation *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetIndexTranslation\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__SetIndexTranslation( obj, (s32*) ((char*)(args + 1)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetColorKey: {
            D_UNUSED
            CoreGraphicsStateSetColorKey       *args        = (CoreGraphicsStateSetColorKey *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetColorKey\n" );

         ;    // TODO: DFBColorKey_debug args->key;

            IGraphicsState_Real__SetColorKey( obj, &args->key );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetRenderOptions: {
            D_UNUSED
            CoreGraphicsStateSetRenderOptions       *args        = (CoreGraphicsStateSetRenderOptions *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetRenderOptions\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> options = %x\n", args->options );

            IGraphicsState_Real__SetRenderOptions( obj, args->options );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetMatrix: {
            D_UNUSED
            CoreGraphicsStateSetMatrix       *args        = (CoreGraphicsStateSetMatrix *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetMatrix\n" );


            IGraphicsState_Real__SetMatrix( obj, (s32*) ((char*)(args + 1)) );

            return DFB_OK;
        }

        case _CoreGraphicsState_SetSource2: {
    CoreSurface *surface = NULL;
            D_UNUSED
            CoreGraphicsStateSetSource2       *args        = (CoreGraphicsStateSetSource2 *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_SetSource2\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> surface = %d\n", args->surface_id );

            ret = (DFBResult) CoreSurface_Lookup( core_dfb, args->surface_id, caller, &surface );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up surface by ID %u failed!\n", __FUNCTION__, args->surface_id );
                 return DFB_OK;
            }

            IGraphicsState_Real__SetSource2( obj, surface );

            if (surface)
                CoreSurface_Unref( surface );

            return DFB_OK;
        }

        case _CoreGraphicsState_DrawRectangles: {
            D_UNUSED
            CoreGraphicsStateDrawRectangles       *args        = (CoreGraphicsStateDrawRectangles *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_DrawRectangles\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__DrawRectangles( obj, (DFBRectangle*) ((char*)(args + 1)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_DrawLines: {
            D_UNUSED
            CoreGraphicsStateDrawLines       *args        = (CoreGraphicsStateDrawLines *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_DrawLines\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__DrawLines( obj, (DFBRegion*) ((char*)(args + 1)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_FillRectangles: {
            D_UNUSED
            CoreGraphicsStateFillRectangles       *args        = (CoreGraphicsStateFillRectangles *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_FillRectangles\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__FillRectangles( obj, (DFBRectangle*) ((char*)(args + 1)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_FillTriangles: {
            D_UNUSED
            CoreGraphicsStateFillTriangles       *args        = (CoreGraphicsStateFillTriangles *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_FillTriangles\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__FillTriangles( obj, (DFBTriangle*) ((char*)(args + 1)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_FillTrapezoids: {
            D_UNUSED
            CoreGraphicsStateFillTrapezoids       *args        = (CoreGraphicsStateFillTrapezoids *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_FillTrapezoids\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__FillTrapezoids( obj, (DFBTrapezoid*) ((char*)(args + 1)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_FillSpans: {
            D_UNUSED
            CoreGraphicsStateFillSpans       *args        = (CoreGraphicsStateFillSpans *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_FillSpans\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> y = %d\n", args->y );
            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__FillSpans( obj, args->y, (DFBSpan*) ((char*)(args + 1)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_Blit: {
            D_UNUSED
            CoreGraphicsStateBlit       *args        = (CoreGraphicsStateBlit *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_Blit\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__Blit( obj, (DFBRectangle*) ((char*)(args + 1)), (DFBPoint*) ((char*)(args + 1) + args->num * sizeof(DFBRectangle)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_Blit2: {
            D_UNUSED
            CoreGraphicsStateBlit2       *args        = (CoreGraphicsStateBlit2 *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_Blit2\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__Blit2( obj, (DFBRectangle*) ((char*)(args + 1)), (DFBPoint*) ((char*)(args + 1) + args->num * sizeof(DFBRectangle)), (DFBPoint*) ((char*)(args + 1) + args->num * sizeof(DFBRectangle) + args->num * sizeof(DFBPoint)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_StretchBlit: {
            D_UNUSED
            CoreGraphicsStateStretchBlit       *args        = (CoreGraphicsStateStretchBlit *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_StretchBlit\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__StretchBlit( obj, (DFBRectangle*) ((char*)(args + 1)), (DFBRectangle*) ((char*)(args + 1) + args->num * sizeof(DFBRectangle)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_TileBlit: {
            D_UNUSED
            CoreGraphicsStateTileBlit       *args        = (CoreGraphicsStateTileBlit *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_TileBlit\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );

            IGraphicsState_Real__TileBlit( obj, (DFBRectangle*) ((char*)(args + 1)), (DFBPoint*) ((char*)(args + 1) + args->num * sizeof(DFBRectangle)), (DFBPoint*) ((char*)(args + 1) + args->num * sizeof(DFBRectangle) + args->num * sizeof(DFBPoint)), args->num );

            return DFB_OK;
        }

        case _CoreGraphicsState_TextureTriangles: {
            D_UNUSED
            CoreGraphicsStateTextureTriangles       *args        = (CoreGraphicsStateTextureTriangles *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_TextureTriangles\n" );

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> num = %u\n", args->num );
            D_DEBUG_AT( DirectFB_CoreGraphicsState, "  -> formation = %x\n", args->formation );

            IGraphicsState_Real__TextureTriangles( obj, (DFBVertex*) ((char*)(args + 1)), args->num, args->formation );

            return DFB_OK;
        }

        case _CoreGraphicsState_ReleaseSource: {
            D_UNUSED
            CoreGraphicsStateReleaseSource       *args        = (CoreGraphicsStateReleaseSource *) ptr;

            D_DEBUG_AT( DirectFB_CoreGraphicsState, "=-> CoreGraphicsState_ReleaseSource\n" );


            IGraphicsState_Real__ReleaseSource( obj );

            return DFB_OK;
        }

    }

    return DFB_NOSUCHMETHOD;
}
/*********************************************************************************************************************/

DFBResult
CoreGraphicsStateDispatch__Dispatch( CoreGraphicsState *obj,
                                FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length )
{
    DFBResult ret;

    D_DEBUG_AT( DirectFB_CoreGraphicsState, "CoreGraphicsStateDispatch::%s( %p )\n", __FUNCTION__, obj );

    Core_PushIdentity( caller );

    ret = __CoreGraphicsStateDispatch__Dispatch( obj, caller, method, ptr, length, ret_ptr, ret_size, ret_length );

    Core_PopIdentity();

    return ret;
}
