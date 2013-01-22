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

#include "ImageProvider.h"

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <fusion/conf.h>

#include <core/core.h>

D_DEBUG_DOMAIN( DirectFB_ImageProvider, "DirectFB/ImageProvider", "DirectFB ImageProvider" );

/*********************************************************************************************************************/

DFBResult
ImageProvider_Dispose(
                    ImageProvider                             *obj

)
{
    return IImageProvider_Requestor__Dispose( obj );
}

DFBResult
ImageProvider_GetSurfaceDescription(
                    ImageProvider                             *obj,
                    DFBSurfaceDescription                     *ret_description
)
{
    return IImageProvider_Requestor__GetSurfaceDescription( obj, ret_description );
}

DFBResult
ImageProvider_GetImageDescription(
                    ImageProvider                             *obj,
                    DFBImageDescription                       *ret_description
)
{
    return IImageProvider_Requestor__GetImageDescription( obj, ret_description );
}

DFBResult
ImageProvider_RenderTo(
                    ImageProvider                             *obj,
                    CoreSurface                               *destination,
                    const DFBRectangle                        *rect
)
{
    return IImageProvider_Requestor__RenderTo( obj, destination, rect );
}

/*********************************************************************************************************************/

static FusionCallHandlerResult
ImageProvider_Dispatch( int           caller,   /* fusion id of the caller */
                     int           call_arg, /* optional call parameter */
                     void         *ptr, /* optional call parameter */
                     unsigned int  length,
                     void         *ctx,      /* optional handler context */
                     unsigned int  serial,
                     void         *ret_ptr,
                     unsigned int  ret_size,
                     unsigned int *ret_length )
{
    ImageProviderDispatch *obj = (ImageProviderDispatch*) ctx;
    ImageProviderDispatch__Dispatch( obj, caller, call_arg, ptr, length, ret_ptr, ret_size, ret_length );

    return FCHR_RETURN;
}

void ImageProvider_Init_Dispatch(
                    CoreDFB              *core,
                    ImageProviderDispatch *obj,
                    FusionCall           *call
)
{
    fusion_call_init3( call, ImageProvider_Dispatch, obj, core->world );
}

void  ImageProvider_Deinit_Dispatch(
                    FusionCall           *call
)
{
     fusion_call_destroy( call );
}

/*********************************************************************************************************************/


DFBResult
IImageProvider_Requestor__Dispose( ImageProvider *obj

)
{
    DFBResult           ret;
    ImageProviderDispose       *args = (ImageProviderDispose*) alloca( sizeof(ImageProviderDispose) );
    ImageProviderDisposeReturn *return_args = (ImageProviderDisposeReturn*) alloca( sizeof(ImageProviderDisposeReturn) );

    D_DEBUG_AT( DirectFB_ImageProvider, "IImageProvider_Requestor::%s()\n", __FUNCTION__ );



    ret = (DFBResult) ImageProvider_Call( obj, FCEF_NONE, _ImageProvider_Dispose, args, sizeof(ImageProviderDispose), return_args, sizeof(ImageProviderDisposeReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: ImageProvider_Call( ImageProvider_Dispose ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: ImageProvider_Dispose failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}


DFBResult
IImageProvider_Requestor__GetSurfaceDescription( ImageProvider *obj,
                    DFBSurfaceDescription                     *ret_description
)
{
    DFBResult           ret;
    ImageProviderGetSurfaceDescription       *args = (ImageProviderGetSurfaceDescription*) alloca( sizeof(ImageProviderGetSurfaceDescription) );
    ImageProviderGetSurfaceDescriptionReturn *return_args = (ImageProviderGetSurfaceDescriptionReturn*) alloca( sizeof(ImageProviderGetSurfaceDescriptionReturn) );

    D_DEBUG_AT( DirectFB_ImageProvider, "IImageProvider_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_description != NULL );


    ret = (DFBResult) ImageProvider_Call( obj, FCEF_NONE, _ImageProvider_GetSurfaceDescription, args, sizeof(ImageProviderGetSurfaceDescription), return_args, sizeof(ImageProviderGetSurfaceDescriptionReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: ImageProvider_Call( ImageProvider_GetSurfaceDescription ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: ImageProvider_GetSurfaceDescription failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }

    *ret_description = return_args->description;

    return DFB_OK;
}


DFBResult
IImageProvider_Requestor__GetImageDescription( ImageProvider *obj,
                    DFBImageDescription                       *ret_description
)
{
    DFBResult           ret;
    ImageProviderGetImageDescription       *args = (ImageProviderGetImageDescription*) alloca( sizeof(ImageProviderGetImageDescription) );
    ImageProviderGetImageDescriptionReturn *return_args = (ImageProviderGetImageDescriptionReturn*) alloca( sizeof(ImageProviderGetImageDescriptionReturn) );

    D_DEBUG_AT( DirectFB_ImageProvider, "IImageProvider_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( ret_description != NULL );


    ret = (DFBResult) ImageProvider_Call( obj, FCEF_NONE, _ImageProvider_GetImageDescription, args, sizeof(ImageProviderGetImageDescription), return_args, sizeof(ImageProviderGetImageDescriptionReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: ImageProvider_Call( ImageProvider_GetImageDescription ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: ImageProvider_GetImageDescription failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }

    *ret_description = return_args->description;

    return DFB_OK;
}


DFBResult
IImageProvider_Requestor__RenderTo( ImageProvider *obj,
                    CoreSurface                               *destination,
                    const DFBRectangle                        *rect
)
{
    DFBResult           ret;
    ImageProviderRenderTo       *args = (ImageProviderRenderTo*) alloca( sizeof(ImageProviderRenderTo) );
    ImageProviderRenderToReturn *return_args = (ImageProviderRenderToReturn*) alloca( sizeof(ImageProviderRenderToReturn) );

    D_DEBUG_AT( DirectFB_ImageProvider, "IImageProvider_Requestor::%s()\n", __FUNCTION__ );

    D_ASSERT( destination != NULL );

    args->destination_id = CoreSurface_GetID( destination );
  if (rect) {
    args->rect = *rect;
    args->rect_set = true;
  }
  else
    args->rect_set = false;

    ret = (DFBResult) ImageProvider_Call( obj, FCEF_NONE, _ImageProvider_RenderTo, args, sizeof(ImageProviderRenderTo), return_args, sizeof(ImageProviderRenderToReturn), NULL );
    if (ret) {
        D_DERROR( ret, "%s: ImageProvider_Call( ImageProvider_RenderTo ) failed!\n", __FUNCTION__ );
        return ret;
    }

    if (return_args->result) {
         /*D_DERROR( return_args->result, "%s: ImageProvider_RenderTo failed!\n", __FUNCTION__ );*/
         return return_args->result;
    }


    return DFB_OK;
}

/*********************************************************************************************************************/

static DFBResult
__ImageProviderDispatch__Dispatch( ImageProviderDispatch *obj,
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
        case _ImageProvider_Dispose: {
            D_UNUSED
            ImageProviderDispose       *args        = (ImageProviderDispose *) ptr;
            ImageProviderDisposeReturn *return_args = (ImageProviderDisposeReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_ImageProvider, "=-> ImageProvider_Dispose\n" );

            return_args->result = IImageProvider_Real__Dispose( obj );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(ImageProviderDisposeReturn);

            return DFB_OK;
        }

        case _ImageProvider_GetSurfaceDescription: {
            D_UNUSED
            ImageProviderGetSurfaceDescription       *args        = (ImageProviderGetSurfaceDescription *) ptr;
            ImageProviderGetSurfaceDescriptionReturn *return_args = (ImageProviderGetSurfaceDescriptionReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_ImageProvider, "=-> ImageProvider_GetSurfaceDescription\n" );

            return_args->result = IImageProvider_Real__GetSurfaceDescription( obj, &return_args->description );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(ImageProviderGetSurfaceDescriptionReturn);

            return DFB_OK;
        }

        case _ImageProvider_GetImageDescription: {
            D_UNUSED
            ImageProviderGetImageDescription       *args        = (ImageProviderGetImageDescription *) ptr;
            ImageProviderGetImageDescriptionReturn *return_args = (ImageProviderGetImageDescriptionReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_ImageProvider, "=-> ImageProvider_GetImageDescription\n" );

            return_args->result = IImageProvider_Real__GetImageDescription( obj, &return_args->description );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(ImageProviderGetImageDescriptionReturn);

            return DFB_OK;
        }

        case _ImageProvider_RenderTo: {
    CoreSurface *destination = NULL;
            D_UNUSED
            ImageProviderRenderTo       *args        = (ImageProviderRenderTo *) ptr;
            ImageProviderRenderToReturn *return_args = (ImageProviderRenderToReturn *) ret_ptr;

            D_DEBUG_AT( DirectFB_ImageProvider, "=-> ImageProvider_RenderTo\n" );

            ret = (DFBResult) CoreSurface_Lookup( core_dfb, args->destination_id, caller, &destination );
            if (ret) {
                 D_DERROR( ret, "%s: Looking up destination by ID %u failed!\n", __FUNCTION__, args->destination_id );
                 return_args->result = ret;
                 return DFB_OK;
            }

            return_args->result = IImageProvider_Real__RenderTo( obj, destination, args->rect_set ? &args->rect : NULL );
            if (return_args->result == DFB_OK) {
            }

            *ret_length = sizeof(ImageProviderRenderToReturn);

            if (destination)
                CoreSurface_Unref( destination );

            return DFB_OK;
        }

    }

    return DFB_NOSUCHMETHOD;
}
/*********************************************************************************************************************/

DFBResult
ImageProviderDispatch__Dispatch( ImageProviderDispatch *obj,
                                FusionID      caller,
                                int           method,
                                void         *ptr,
                                unsigned int  length,
                                void         *ret_ptr,
                                unsigned int  ret_size,
                                unsigned int *ret_length )
{
    DFBResult ret;

    D_DEBUG_AT( DirectFB_ImageProvider, "ImageProviderDispatch::%s( %p )\n", __FUNCTION__, obj );

    Core_PushIdentity( caller );

    ret = __ImageProviderDispatch__Dispatch( obj, caller, method, ptr, length, ret_ptr, ret_size, ret_length );

    Core_PopIdentity();

    return ret;
}
