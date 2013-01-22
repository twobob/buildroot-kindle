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
#include <stdlib.h>
#include <string.h>

#include <math.h>


#include <directfb.h>
#include <directfb_util.h>

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <voodoo/interface.h>
#include <voodoo/manager.h>
#include <voodoo/message.h>

#include <idirectfbsurface_dispatcher.h>

#include "idirectfbfont_requestor.h"
#include "idirectfbpalette_requestor.h"
#include "idirectfbsurface_requestor.h"


D_DEBUG_DOMAIN( IDirectFBSurface_Requestor_, "IDirectFBSurface/Requestor", "IDirectFBSurface Requestor" );

static DFBResult Probe( void );
static DFBResult Construct( IDirectFBSurface *thiz,
                            VoodooManager    *manager,
                            VoodooInstanceID  instance,
                            void             *arg );

#include <direct/interface_implementation.h>

DIRECT_INTERFACE_IMPLEMENTATION( IDirectFBSurface, Requestor )

/**************************************************************************************************/

static void
IDirectFBSurface_Requestor_Destruct( IDirectFBSurface *thiz )
{
     IDirectFBSurface_Requestor_data *data = thiz->priv;

     D_DEBUG( "%s (%p)\n", __FUNCTION__, thiz );

     voodoo_manager_request( data->manager, data->instance,
                             IDIRECTFBSURFACE_METHOD_ID_Release, VREQ_NONE, NULL,
                             VMBT_NONE );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

/**************************************************************************************************/

static DirectResult
IDirectFBSurface_Requestor_AddRef( IDirectFBSurface *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     data->ref++;

     return DFB_OK;
}

static DirectResult
IDirectFBSurface_Requestor_Release( IDirectFBSurface *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (--data->ref == 0)
          IDirectFBSurface_Requestor_Destruct( thiz );

     return DFB_OK;
}


static DFBResult
IDirectFBSurface_Requestor_GetPixelFormat( IDirectFBSurface      *thiz,
                                           DFBSurfacePixelFormat *ret_format )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!ret_format)
          return DFB_INVARG;

     if (!data->format) {
          DFBResult              ret;
          VoodooResponseMessage *response;
          VoodooMessageParser    parser;

          ret = voodoo_manager_request( data->manager, data->instance,
                                        IDIRECTFBSURFACE_METHOD_ID_GetPixelFormat, VREQ_RESPOND, &response,
                                        VMBT_NONE );
          if (ret)
               return ret;

          ret = response->result;
          if (ret) {
               voodoo_manager_finish_request( data->manager, response );
               return ret;
          }

          VOODOO_PARSER_BEGIN( parser, response );
          VOODOO_PARSER_GET_INT( parser, data->format );
          VOODOO_PARSER_END( parser );

          voodoo_manager_finish_request( data->manager, response );
     }

     *ret_format = data->format;

     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Requestor_GetAccelerationMask( IDirectFBSurface    *thiz,
                                                IDirectFBSurface    *source,
                                                DFBAccelerationMask *ret_mask )
{
     DirectResult           ret;
     VoodooResponseMessage *response;
     VoodooMessageParser    parser;
     DFBAccelerationMask    mask;

     IDirectFBSurface_Requestor_data *source_data = NULL;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (source)
          DIRECT_INTERFACE_GET_DATA_FROM( source, source_data, IDirectFBSurface_Requestor );

     ret = voodoo_manager_request( data->manager, data->instance,
                                   IDIRECTFBSURFACE_METHOD_ID_GetAccelerationMask, VREQ_RESPOND, &response,
                                   VMBT_ID, source_data ? source_data->instance : VOODOO_INSTANCE_NONE,
                                   VMBT_NONE );
     if (ret)
          return ret;

     ret = response->result;
     if (ret) {
          voodoo_manager_finish_request( data->manager, response );
          return ret;
     }

     VOODOO_PARSER_BEGIN( parser, response );
     VOODOO_PARSER_GET_INT( parser, mask );
     VOODOO_PARSER_END( parser );

     voodoo_manager_finish_request( data->manager, response );

     *ret_mask = mask;

     return ret;
}

static DFBResult
IDirectFBSurface_Requestor_GetPosition( IDirectFBSurface *thiz,
                                        int              *x,
                                        int              *y )
{
     DFBResult              ret;
     VoodooResponseMessage *response;
     VoodooMessageParser    parser;
     const DFBPoint        *position;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!x && !y)
          return DFB_INVARG;

     ret = voodoo_manager_request( data->manager, data->instance,
                                   IDIRECTFBSURFACE_METHOD_ID_GetPosition,
                                   VREQ_RESPOND, &response,
                                   VMBT_NONE );
     if (ret)
          return ret;

     ret = response->result;
     if (ret) {
          voodoo_manager_finish_request( data->manager, response );
          return ret;
     }

     VOODOO_PARSER_BEGIN( parser, response );
     VOODOO_PARSER_GET_DATA( parser, position );
     VOODOO_PARSER_END( parser );

     if (x)
          *x = position->x;

     if (y)
          *y = position->y;

     voodoo_manager_finish_request( data->manager, response );

     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Requestor_GetSize( IDirectFBSurface *thiz,
                                    int              *width,
                                    int              *height )
{
     DFBResult              ret;
     VoodooResponseMessage *response;
     VoodooMessageParser    parser;
     const DFBDimension    *dimension;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!width && !height)
          return DFB_INVARG;

     ret = voodoo_manager_request( data->manager, data->instance,
                                   IDIRECTFBSURFACE_METHOD_ID_GetSize, VREQ_RESPOND, &response,
                                   VMBT_NONE );
     if (ret)
          return ret;

     ret = response->result;
     if (ret) {
          voodoo_manager_finish_request( data->manager, response );
          return ret;
     }

     VOODOO_PARSER_BEGIN( parser, response );
     VOODOO_PARSER_GET_DATA( parser, dimension );
     VOODOO_PARSER_END( parser );

     if (width)
          *width = dimension->w;

     if (height)
          *height = dimension->h;

     voodoo_manager_finish_request( data->manager, response );

     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Requestor_GetVisibleRectangle( IDirectFBSurface *thiz,
                                                DFBRectangle     *rect )
{
     DFBResult              ret;
     VoodooResponseMessage *response;
     VoodooMessageParser    parser;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!rect)
          return DFB_INVARG;

     ret = voodoo_manager_request( data->manager, data->instance,
                                   IDIRECTFBSURFACE_METHOD_ID_GetVisibleRectangle,
                                   VREQ_RESPOND, &response,
                                   VMBT_NONE );
     if (ret)
          return ret;

     ret = response->result;
     if (ret) {
          voodoo_manager_finish_request( data->manager, response );
          return ret;
     }

     VOODOO_PARSER_BEGIN( parser, response );
     VOODOO_PARSER_READ_DATA( parser, rect, sizeof(DFBRectangle) );
     VOODOO_PARSER_END( parser );

     voodoo_manager_finish_request( data->manager, response );

     return DFB_OK;
}

static DFBResult
IDirectFBSurface_Requestor_GetCapabilities( IDirectFBSurface       *thiz,
                                            DFBSurfaceCapabilities *caps )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!caps)
          return DFB_INVARG;

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_GetPalette( IDirectFBSurface  *thiz,
                                       IDirectFBPalette **ret_interface )
{
     DirectResult           ret;
     VoodooResponseMessage *response;
     void                  *interface_ptr = NULL;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!ret_interface)
          return DFB_INVARG;

     ret = voodoo_manager_request( data->manager, data->instance,
                                   IDIRECTFBSURFACE_METHOD_ID_GetPalette, VREQ_RESPOND, &response,
                                   VMBT_NONE );
     if (ret)
          return ret;

     ret = response->result;
     if (ret == DR_OK)
          ret = voodoo_construct_requestor( data->manager, "IDirectFBPalette",
                                            response->instance, NULL, &interface_ptr );

     voodoo_manager_finish_request( data->manager, response );

     *ret_interface = interface_ptr;

     return ret;
}

static DFBResult
IDirectFBSurface_Requestor_SetPalette( IDirectFBSurface *thiz,
                                       IDirectFBPalette *palette )
{
     IDirectFBPalette_Requestor_data *palette_data = NULL;

     DIRECT_INTERFACE_GET_DATA( IDirectFBSurface_Requestor )

     if (!palette)
          return DFB_INVARG;

     DIRECT_INTERFACE_GET_DATA_FROM( palette, palette_data, IDirectFBPalette_Requestor );

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetPalette, VREQ_QUEUE, NULL,
                                    VMBT_ID, palette_data->instance,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetAlphaRamp( IDirectFBSurface *thiz,
                                         u8 a0, u8 a1, u8 a2, u8 a3 )
{
     DIRECT_INTERFACE_GET_DATA( IDirectFBSurface_Requestor )

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_Lock( IDirectFBSurface *thiz,
                                 DFBSurfaceLockFlags flags,
                                 void **ret_ptr, int *ret_pitch )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!flags || !ret_ptr || !ret_pitch)
          return DFB_INVARG;

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_GetFramebufferOffset( IDirectFBSurface *thiz,
                                                 int              *offset )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!offset)
          return DFB_INVARG;

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_GetPhysicalAddress( IDirectFBSurface *thiz,
                                               unsigned long    *addr )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!addr)
          return DFB_INVARG;

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_Unlock( IDirectFBSurface *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_Flip( IDirectFBSurface    *thiz,
                                 const DFBRegion     *region,
                                 DFBSurfaceFlipFlags  flags )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     /* HACK for performance */
//     flags &= ~DSFLIP_WAITFORSYNC;

     if (flags & DSFLIP_WAIT) {
          DirectResult           ret;
          VoodooResponseMessage *response;

          ret = voodoo_manager_request( data->manager, data->instance,
                                        IDIRECTFBSURFACE_METHOD_ID_Flip, VREQ_RESPOND, &response,
                                        VMBT_ODATA, sizeof(DFBRegion), region,
                                        VMBT_INT, flags,
                                        VMBT_NONE );
          if (ret)
               return ret;

          ret = response->result;

          voodoo_manager_finish_request( data->manager, response );

          return ret;
     }

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_Flip, VREQ_NONE, NULL,
                                    VMBT_ODATA, sizeof(DFBRegion), region,
                                    VMBT_INT, flags,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetField( IDirectFBSurface    *thiz,
                                     int                  field )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (field < 0 || field > 1)
          return DFB_INVARG;

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_Clear( IDirectFBSurface *thiz,
                                  u8 r, u8 g, u8 b, u8 a )
{
     DFBColor color = { a, r, g, b };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_Clear, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBColor), &color,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetClip( IDirectFBSurface *thiz, const DFBRegion *clip )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetClip, VREQ_QUEUE, NULL,
                                    VMBT_ODATA, sizeof(DFBRegion), clip,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetColor( IDirectFBSurface *thiz,
                                     u8 r, u8 g, u8 b, u8 a )
{
     DFBColor color = { a, r, g, b };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetColor, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBColor), &color,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetColorIndex( IDirectFBSurface *thiz,
                                          unsigned int      index )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_SetSrcBlendFunction( IDirectFBSurface        *thiz,
                                                DFBSurfaceBlendFunction  src )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_SetDstBlendFunction( IDirectFBSurface        *thiz,
                                                DFBSurfaceBlendFunction  dst )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_SetPorterDuff( IDirectFBSurface         *thiz,
                                          DFBSurfacePorterDuffRule  rule )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_SetSrcColorKey( IDirectFBSurface *thiz,
                                           u8                r,
                                           u8                g,
                                           u8                b )
{
     DFBColor color = { 0, r, g, b };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetSrcColorKey, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBColor), &color,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetSrcColorKeyIndex( IDirectFBSurface *thiz,
                                                unsigned int      index )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetSrcColorKeyIndex, VREQ_QUEUE, NULL,
                                    VMBT_UINT, index,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetDstColorKey( IDirectFBSurface *thiz,
                                           u8                r,
                                           u8                g,
                                           u8                b )
{
     DFBColor color = { 0, r, g, b };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetDstColorKey, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBColor), &color,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetDstColorKeyIndex( IDirectFBSurface *thiz,
                                                unsigned int      index )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetDstColorKeyIndex, VREQ_QUEUE, NULL,
                                    VMBT_UINT, index,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetFont( IDirectFBSurface *thiz,
                                    IDirectFBFont    *font )
{
     IDirectFBFont_Requestor_data *font_data = NULL;

     DIRECT_INTERFACE_GET_DATA( IDirectFBSurface_Requestor )

     if (data->font == font)
          return DFB_OK;

     if (font) {
          font->AddRef (font);

          DIRECT_INTERFACE_GET_DATA_FROM( font, font_data, IDirectFBFont_Requestor );
     }

     if (data->font)
          data->font->Release (data->font);

     data->font = font;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetFont, VREQ_QUEUE, NULL,
                                    VMBT_ID, font_data ? font_data->instance : VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_GetFont( IDirectFBSurface  *thiz,
                                    IDirectFBFont    **font )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_SetDrawingFlags( IDirectFBSurface       *thiz,
                                            DFBSurfaceDrawingFlags  flags )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetDrawingFlags, VREQ_QUEUE, NULL,
                                    VMBT_INT, flags,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_FillRectangle( IDirectFBSurface *thiz,
                                          int x, int y, int w, int h )
{
     DFBRectangle rect = { x, y, w, h };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (w <= 0 || h <= 0)
          return DFB_INVARG;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_FillRectangle, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBRectangle), &rect,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_FillRectangles( IDirectFBSurface   *thiz,
                                           const DFBRectangle *rects,
                                           unsigned int        num_rects )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!rects || !num_rects)
          return DFB_INVARG;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_FillRectangles, VREQ_QUEUE, NULL,
                                    VMBT_UINT, num_rects,
                                    VMBT_DATA, num_rects * sizeof(DFBRectangle), rects,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_FillTrapezoids( IDirectFBSurface   *thiz,
                                           const DFBTrapezoid *traps,
                                           unsigned int        num_traps )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!traps || !num_traps)
          return DFB_INVARG;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_FillTrapezoids, VREQ_QUEUE, NULL,
                                    VMBT_UINT, num_traps,
                                    VMBT_DATA, num_traps * sizeof(DFBTrapezoid), traps,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_FillSpans( IDirectFBSurface *thiz,
                                      int               y,
                                      const DFBSpan    *spans,
                                      unsigned int      num_spans )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!spans || !num_spans)
          return DFB_INVARG;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_FillSpans, VREQ_QUEUE, NULL,
                                    VMBT_INT, y,
                                    VMBT_UINT, num_spans,
                                    VMBT_DATA, num_spans * sizeof(DFBSpan), spans,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_DrawLine( IDirectFBSurface *thiz,
                                     int x1, int y1, int x2, int y2 )
{
     DFBRegion line = { x1, y1, x2, y2 };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_DrawLine, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBRegion), &line,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_DrawLines( IDirectFBSurface *thiz,
                                      const DFBRegion  *lines,
                                      unsigned int      num_lines )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!lines || !num_lines)
          return DFB_INVARG;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_DrawLines, VREQ_QUEUE, NULL,
                                    VMBT_UINT, num_lines,
                                    VMBT_DATA, num_lines * sizeof(DFBRegion), lines,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_DrawRectangle( IDirectFBSurface *thiz,
                                          int x, int y, int w, int h )
{
     DFBRectangle rect = { x, y, w, h };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (w <= 0 || h <= 0)
          return DFB_INVARG;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_DrawRectangle, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBRectangle), &rect,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_FillTriangle( IDirectFBSurface *thiz,
                                         int x1, int y1,
                                         int x2, int y2,
                                         int x3, int y3 )
{
     DFBTriangle triangle = { x1, y1, x2, y2, x3, y3 };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_FillTriangle, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(DFBTriangle), &triangle,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetBlittingFlags( IDirectFBSurface        *thiz,
                                             DFBSurfaceBlittingFlags  flags )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetBlittingFlags, VREQ_QUEUE, NULL,
                                    VMBT_INT, flags,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_Blit( IDirectFBSurface   *thiz,
                                 IDirectFBSurface   *source,
                                 const DFBRectangle *rect,
                                 int                 x,
                                 int                 y )
{
     DFBPoint point = { x, y };

     IDirectFBSurface_Requestor_data *source_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!source)
          return DFB_INVARG;

     DIRECT_INTERFACE_GET_DATA_FROM( source, source_data, IDirectFBSurface_Requestor );

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_Blit, VREQ_QUEUE, NULL,
                                    VMBT_ID, source_data->instance,
                                    VMBT_ODATA, sizeof(DFBRectangle), rect,
                                    VMBT_DATA, sizeof(point), &point,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_TileBlit( IDirectFBSurface   *thiz,
                                     IDirectFBSurface   *source,
                                     const DFBRectangle *rect,
                                     int                 x,
                                     int                 y )
{
     DFBPoint point = { x, y };

     IDirectFBSurface_Requestor_data *source_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!source)
          return DFB_INVARG;

     DIRECT_INTERFACE_GET_DATA_FROM( source, source_data, IDirectFBSurface_Requestor );

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_TileBlit, VREQ_QUEUE, NULL,
                                    VMBT_ID, source_data->instance,
                                    VMBT_ODATA, sizeof(DFBRectangle), rect,
                                    VMBT_DATA, sizeof(point), &point,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_BatchBlit( IDirectFBSurface   *thiz,
                                      IDirectFBSurface   *source,
                                      const DFBRectangle *source_rects,
                                      const DFBPoint     *dest_points,
                                      int                 num )
{
     IDirectFBSurface_Requestor_data *source_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!source || !source_rects || !dest_points || num < 1)
          return DFB_INVARG;

     DIRECT_INTERFACE_GET_DATA_FROM( source, source_data, IDirectFBSurface_Requestor );

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_BatchBlit, VREQ_QUEUE, NULL,
                                    VMBT_ID, source_data->instance,
                                    VMBT_UINT, num,
                                    VMBT_DATA, num * sizeof(DFBRectangle), source_rects,
                                    VMBT_DATA, num * sizeof(DFBPoint), dest_points,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_StretchBlit( IDirectFBSurface   *thiz,
                                        IDirectFBSurface   *source,
                                        const DFBRectangle *source_rect,
                                        const DFBRectangle *destination_rect )
{
     IDirectFBSurface_Requestor_data *source_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!source)
          return DFB_INVARG;

     DIRECT_INTERFACE_GET_DATA_FROM( source, source_data, IDirectFBSurface_Requestor );

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_StretchBlit, VREQ_QUEUE, NULL,
                                    VMBT_ID, source_data->instance,
                                    VMBT_ODATA, sizeof(DFBRectangle), source_rect,
                                    VMBT_ODATA, sizeof(DFBRectangle), destination_rect,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_TextureTriangles( IDirectFBSurface     *thiz,
                                             IDirectFBSurface     *source,
                                             const DFBVertex      *vertices,
                                             const int            *indices,
                                             int                   num,
                                             DFBTriangleFormation  formation )
{
     int                              i;
     int                              num_vertices = 0;
     IDirectFBSurface_Requestor_data *source_data;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!source || !vertices || num < 3)
          return DFB_INVARG;

     if (indices) {
          for (i=0; i<num; i++) {
               if (num_vertices <= indices[i])
                    num_vertices = indices[i] + 1;
          }
     }

     DIRECT_INTERFACE_GET_DATA_FROM( source, source_data, IDirectFBSurface_Requestor );

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_TextureTriangles, VREQ_QUEUE, NULL,
                                    VMBT_ID, source_data->instance,
                                    VMBT_DATA, num_vertices * sizeof(DFBVertex), vertices,
                                    VMBT_ODATA, num * sizeof(int), indices,
                                    VMBT_INT, num,
                                    VMBT_INT, formation,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_DrawString( IDirectFBSurface    *thiz,
                                       const char          *text,
                                       int                  bytes,
                                       int                  x,
                                       int                  y,
                                       DFBSurfaceTextFlags  flags )
{
     DFBPoint point = { x, y };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!text)
          return DFB_INVARG;

     if (!data->font)
          return DFB_MISSINGFONT;


     if (bytes < 0)
          bytes = strlen (text);

     if (bytes == 0)
          return DFB_OK;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_DrawString, VREQ_QUEUE, NULL,
                                    VMBT_DATA, bytes, text,
                                    VMBT_INT, bytes,
                                    VMBT_DATA, sizeof(point), &point,
                                    VMBT_INT, flags,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_DrawGlyph( IDirectFBSurface    *thiz,
                                      unsigned int         index,
                                      int                  x,
                                      int                  y,
                                      DFBSurfaceTextFlags  flags )
{
     DFBPoint point = { x, y };

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!index)
          return DFB_INVARG;

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_DrawGlyph, VREQ_QUEUE, NULL,
                                    VMBT_UINT, index,
                                    VMBT_DATA, sizeof(point), &point,
                                    VMBT_INT, flags,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetEncoding( IDirectFBSurface  *thiz,
                                        DFBTextEncodingID  encoding )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetEncoding, VREQ_QUEUE, NULL,
                                    VMBT_UINT, encoding,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_GetSubSurface( IDirectFBSurface    *thiz,
                                          const DFBRectangle  *rect,
                                          IDirectFBSurface   **ret_interface )
{
     DirectResult           ret;
     VoodooResponseMessage *response;
     VoodooInstanceID       instance_id;
     void                  *interface_ptr = NULL;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!ret_interface)
          return DFB_INVARG;

     ret = voodoo_manager_request( data->manager, data->instance,
                                   IDIRECTFBSURFACE_METHOD_ID_GetSubSurface, VREQ_RESPOND, &response,
                                   VMBT_ODATA, sizeof(DFBRectangle), rect,
                                   VMBT_NONE );
     if (ret)
          return ret;

     instance_id = response->instance;

     voodoo_manager_finish_request( data->manager, response );

     ret = response->result;
     if (ret == DR_OK)
          ret = voodoo_construct_requestor( data->manager, "IDirectFBSurface",
                                            instance_id, NULL, &interface_ptr );

     *ret_interface = interface_ptr;

     return ret;
}

static DFBResult
IDirectFBSurface_Requestor_GetGL( IDirectFBSurface   *thiz,
                                  IDirectFBGL       **interface_ptr )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!interface_ptr)
          return DFB_INVARG;

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_Dump( IDirectFBSurface   *thiz,
                                 const char         *directory,
                                 const char         *prefix )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!directory || !prefix)
          return DFB_INVARG;

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_DisableAcceleration( IDirectFBSurface   *thiz,
                                                DFBAccelerationMask mask )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_DisableAcceleration,
                                    VREQ_QUEUE, NULL,
                                    VMBT_UINT, mask,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_ReleaseSource( IDirectFBSurface *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_ReleaseSource, VREQ_QUEUE, NULL,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetIndexTranslation( IDirectFBSurface *thiz,
                                                const int        *indices,
                                                int               num_indices )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBSurface_Requestor_SetRenderOptions( IDirectFBSurface        *thiz,
                                             DFBSurfaceRenderOptions  options )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetRenderOptions, VREQ_QUEUE, NULL,
                                    VMBT_INT, options,
                                    VMBT_NONE );
}

static DFBResult
IDirectFBSurface_Requestor_SetMatrix( IDirectFBSurface *thiz,
                                      const s32        *matrix )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     return voodoo_manager_request( data->manager, data->instance,
                                    IDIRECTFBSURFACE_METHOD_ID_SetMatrix, VREQ_QUEUE, NULL,
                                    VMBT_DATA, sizeof(s32) * 9, matrix,
                                    VMBT_NONE );
}

#define RLE16_KEY   0xf001

static bool
rle16_encode( const u16    *src,
              u16          *dst,
              unsigned int  num,
              unsigned int *ret_num )
{
     unsigned int n, last, count = 0, out = 0;

     for (n=0; n<num; n++) {
          if (out + 3 > num) {
               *ret_num = num;
               return false;
          }

          if (count > 0) {
               D_ASSERT( src[n] == last );

               count++;
          }
          else {
               count = 1;
               last  = src[n];
          }

          if (n == num-1 || src[n+1] != last) {
               if (count > 2 || (count > 1 && last == RLE16_KEY)) {
                    dst[out++] = RLE16_KEY;
                    dst[out++] = count;
                    dst[out++] = last;
               }
               else {
                    if (count > 1 || last == RLE16_KEY)
                         dst[out++] = last;

                    dst[out++] = last;
               }

               count = 0;
          }
     }

     *ret_num = out;

     return true;
}

#define RLE32_KEY   0xf0012345

static bool
rle32_encode( const u32    *src,
              u32          *dst,
              unsigned int  num,
              unsigned int *ret_num )
{
     unsigned int n, last, count = 0, out = 0;

     for (n=0; n<num; n++) {
          if (out + 3 > num) {
               *ret_num = num;
               return false;
          }

          if (count > 0) {
               D_ASSERT( src[n] == last );

               count++;
          }
          else {
               count = 1;
               last  = src[n];
          }

          if (n == num-1 || src[n+1] != last) {
               if (count > 2 || (count > 1 && last == RLE32_KEY)) {
                    dst[out++] = RLE32_KEY;
                    dst[out++] = count;
                    dst[out++] = last;
               }
               else {
                    if (count > 1 || last == RLE32_KEY)
                         dst[out++] = last;

                    dst[out++] = last;
               }

               count = 0;
          }
     }

     *ret_num = out;

     return true;
}

static DFBResult
IDirectFBSurface_Requestor_Write( IDirectFBSurface   *thiz,
                                  const DFBRectangle *rect,
                                  const void         *ptr,
                                  int                 pitch )
{
     DFBResult    ret = DFB_OK;
     int          y;
     DFBRectangle r;
     DFBSurfacePixelFormat format;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     if (!rect || !ptr)
          return DFB_INVARG;

     thiz->GetPixelFormat( thiz, &format );

     r.x = rect->x;
     r.y = rect->y;
     r.w = rect->w;
     r.h = 1;

     switch (format) {
          case DSPF_RGB16:
          case DSPF_ARGB1555: {
               unsigned int  num;
               u16          *buf = (u16*) D_MALLOC( rect->w * 2 );

               if (buf) {
                    for (y=0; y<rect->h; y++) {
                         bool encoded = rle16_encode( (u16*)((char*) ptr + y * pitch), buf, rect->w, &num );

                         //D_INFO( "%3d: %u -> %u\n", r.y, rect->w, num );

                         ret = voodoo_manager_request( data->manager, data->instance,
                                                       IDIRECTFBSURFACE_METHOD_ID_Write, VREQ_QUEUE, NULL,
                                                       VMBT_UINT, encoded ? 2 : 0,
                                                       VMBT_DATA, sizeof(DFBRectangle), &r,
                                                       VMBT_DATA, DFB_BYTES_PER_LINE( format, num ),
                                                                  encoded ? buf : (u16*)((char*) ptr + y * pitch),
                                                       VMBT_INT, ABS(pitch),
                                                       VMBT_NONE );
                         if (ret)
                              break;

                         r.y++;
                    }

                    D_FREE( buf );
               }
               else
                    D_OOM();

               break;
          }

          case DSPF_RGB32:
          case DSPF_ARGB: {
               unsigned int  num;
               u32          *buf = (u32*) D_MALLOC( rect->w * 4 );

               if (buf) {
                    for (y=0; y<rect->h; y++) {
                         bool encoded = rle32_encode( (u32*)((char*) ptr + y * pitch), buf, rect->w, &num );

                         //D_INFO( "%3d: %u -> %u\n", r.y, rect->w, num );

                         ret = voodoo_manager_request( data->manager, data->instance,
                                                       IDIRECTFBSURFACE_METHOD_ID_Write, VREQ_QUEUE, NULL,
                                                       VMBT_UINT, encoded ? 4 : 0,
                                                       VMBT_DATA, sizeof(DFBRectangle), &r,
                                                       VMBT_DATA, DFB_BYTES_PER_LINE( format, num ),
                                                                  encoded ? buf : (u32*)((char*) ptr + y * pitch),
                                                       VMBT_INT, ABS(pitch),
                                                       VMBT_NONE );
                         if (ret)
                              break;

                         r.y++;
                    }

                    D_FREE( buf );
               }
               else
                    D_OOM();

               break;
          }

          default:
               for (y=0; y<rect->h; y++) {
                    ret = voodoo_manager_request( data->manager, data->instance,
                                                  IDIRECTFBSURFACE_METHOD_ID_Write, VREQ_QUEUE, NULL,
                                                  VMBT_UINT, false,
                                                  VMBT_DATA, sizeof(DFBRectangle), &r,
                                                  VMBT_DATA, DFB_BYTES_PER_LINE( format, rect->w ), (char*) ptr + y * pitch,
                                                  VMBT_INT, ABS(pitch),
                                                  VMBT_NONE );
                    if (ret)
                         break;

                    r.y++;
               }
               break;
     }

     return ret;
}


#define RLE16_KEY   0xf001

static void
rle16_decode( const u16    *src,
              u16          *dst,
              unsigned int  num )
{
     unsigned int n = 0, last, count, out = 0;

     while (out < num) {
          last = src[n++];

          if (last == RLE16_KEY) {
               count = src[n++];

               if (count == RLE16_KEY) {
                    dst[out++] = RLE16_KEY;
               }
               else {
                    last = src[n++];

                    while (count >= 4) {
                         dst[out+0] =
                         dst[out+1] =
                         dst[out+2] =
                         dst[out+3] = last;

                         out   += 4;
                         count -= 4;
                    }

                    while (count >= 2) {
                         dst[out+0] =
                         dst[out+1] = last;

                         out   += 2;
                         count -= 2;
                    }

                    while (count--)
                         dst[out++] = last;
               }
          }
          else
               dst[out++] = last;
     }

     D_ASSERT( out == num );
}

#define RLE32_KEY   0xf0012345

static void
rle32_decode( const u32    *src,
              u32          *dst,
              unsigned int  num )
{
     unsigned int n = 0, last, count, out = 0;

     while (out < num) {
          last = src[n++];

          if (last == RLE32_KEY) {
               count = src[n++];

               if (count == RLE32_KEY) {
                    dst[out++] = RLE32_KEY;
               }
               else {
                    last = src[n++];

                    while (count >= 4) {
                         dst[out+0] =
                         dst[out+1] =
                         dst[out+2] =
                         dst[out+3] = last;

                         out   += 4;
                         count -= 4;
                    }

                    while (count >= 2) {
                         dst[out+0] =
                         dst[out+1] = last;

                         out   += 2;
                         count -= 2;
                    }

                    while (count--)
                         dst[out++] = last;
               }
          }
          else
               dst[out++] = last;
     }

     D_ASSERT( out == num );
}

static DFBResult
IDirectFBSurface_Requestor_Read( IDirectFBSurface   *thiz,
                                 const DFBRectangle *rect,
                                 void               *ptr,
                                 int                 pitch )
{
     DFBResult              ret = DFB_OK;
     int                    y;
     DFBSurfacePixelFormat  format;
     VoodooMessageParser    parser;
     VoodooResponseMessage *response;

     DIRECT_INTERFACE_GET_DATA(IDirectFBSurface_Requestor)

     D_DEBUG_AT( IDirectFBSurface_Requestor_, "%s( %p, %p, %d )\n", __FUNCTION__, rect, ptr, pitch );

     if (!rect || !ptr)
          return DFB_INVARG;

     D_DEBUG_AT( IDirectFBSurface_Requestor_, "  -> %4d,%4d-%4dx%4d\n", DFB_RECTANGLE_VALS(rect) );

     thiz->GetPixelFormat( thiz, &format );

     ret = voodoo_manager_request( data->manager, data->instance,
                                   IDIRECTFBSURFACE_METHOD_ID_Read, VREQ_RESPOND, &response,
                                   VMBT_DATA, sizeof(DFBRectangle), rect,
                                   VMBT_NONE );
     if (ret)
          return ret;

     for (y=0; y<rect->h; y++) {
          unsigned int  encoded;
          const void   *buf;

          D_DEBUG_AT( IDirectFBSurface_Requestor_, "  -> [%d]\n", y );

          VOODOO_PARSER_BEGIN( parser, response );
          VOODOO_PARSER_GET_UINT( parser, encoded );
          VOODOO_PARSER_GET_DATA( parser, buf );
          VOODOO_PARSER_END( parser );

          ret = response->result;
          if (ret)
               break;


          if (encoded) {
               switch (encoded) {
                    case 2:
                         rle16_decode( buf, (u16*)((char*) ptr + pitch * y), rect->w );
                         break;

                    case 4:
                         rle32_decode( buf, (u32*)((char*) ptr + pitch * y), rect->w );
                         break;

                    default:
                         D_UNIMPLEMENTED();
                         break;
               }
          }
          else
               direct_memcpy( (char*) ptr + pitch * y, buf, DFB_BYTES_PER_LINE(format, rect->w) );


          if (y < rect->h - 1)
               voodoo_manager_next_response( data->manager, response, &response );
     }

     voodoo_manager_finish_request( data->manager, response );

     return ret;
}


/**************************************************************************************************/

static DFBResult
Probe()
{
     /* This implementation has to be loaded explicitly. */
     return DFB_UNSUPPORTED;
}

static DFBResult
Construct( IDirectFBSurface *thiz,
           VoodooManager    *manager,
           VoodooInstanceID  instance,
           void             *arg )
{
     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBSurface_Requestor)

     data->ref      = 1;
     data->manager  = manager;
     data->instance = instance;

     thiz->AddRef = IDirectFBSurface_Requestor_AddRef;
     thiz->Release = IDirectFBSurface_Requestor_Release;

     thiz->GetCapabilities = IDirectFBSurface_Requestor_GetCapabilities;
     thiz->GetPosition = IDirectFBSurface_Requestor_GetPosition;
     thiz->GetSize = IDirectFBSurface_Requestor_GetSize;
     thiz->GetVisibleRectangle = IDirectFBSurface_Requestor_GetVisibleRectangle;
     thiz->GetPixelFormat = IDirectFBSurface_Requestor_GetPixelFormat;
     thiz->GetAccelerationMask = IDirectFBSurface_Requestor_GetAccelerationMask;

     thiz->GetPalette = IDirectFBSurface_Requestor_GetPalette;
     thiz->SetPalette = IDirectFBSurface_Requestor_SetPalette;
     thiz->SetAlphaRamp = IDirectFBSurface_Requestor_SetAlphaRamp;

     thiz->Lock = IDirectFBSurface_Requestor_Lock;
     thiz->GetFramebufferOffset = IDirectFBSurface_Requestor_GetFramebufferOffset;
     thiz->GetPhysicalAddress = IDirectFBSurface_Requestor_GetPhysicalAddress;
     thiz->Unlock = IDirectFBSurface_Requestor_Unlock;
     thiz->Flip = IDirectFBSurface_Requestor_Flip;
     thiz->SetField = IDirectFBSurface_Requestor_SetField;
     thiz->Clear = IDirectFBSurface_Requestor_Clear;

     thiz->SetClip = IDirectFBSurface_Requestor_SetClip;
     thiz->SetColor = IDirectFBSurface_Requestor_SetColor;
     thiz->SetColorIndex = IDirectFBSurface_Requestor_SetColorIndex;
     thiz->SetSrcBlendFunction = IDirectFBSurface_Requestor_SetSrcBlendFunction;
     thiz->SetDstBlendFunction = IDirectFBSurface_Requestor_SetDstBlendFunction;
     thiz->SetPorterDuff = IDirectFBSurface_Requestor_SetPorterDuff;
     thiz->SetSrcColorKey = IDirectFBSurface_Requestor_SetSrcColorKey;
     thiz->SetSrcColorKeyIndex = IDirectFBSurface_Requestor_SetSrcColorKeyIndex;
     thiz->SetDstColorKey = IDirectFBSurface_Requestor_SetDstColorKey;
     thiz->SetDstColorKeyIndex = IDirectFBSurface_Requestor_SetDstColorKeyIndex;

     thiz->SetBlittingFlags = IDirectFBSurface_Requestor_SetBlittingFlags;
     thiz->Blit = IDirectFBSurface_Requestor_Blit;
     thiz->TileBlit = IDirectFBSurface_Requestor_TileBlit;
     thiz->BatchBlit = IDirectFBSurface_Requestor_BatchBlit;
     thiz->StretchBlit = IDirectFBSurface_Requestor_StretchBlit;
     thiz->TextureTriangles = IDirectFBSurface_Requestor_TextureTriangles;

     thiz->SetDrawingFlags = IDirectFBSurface_Requestor_SetDrawingFlags;
     thiz->FillRectangle = IDirectFBSurface_Requestor_FillRectangle;
     thiz->FillRectangles = IDirectFBSurface_Requestor_FillRectangles;
     thiz->FillTrapezoids = IDirectFBSurface_Requestor_FillTrapezoids;
     thiz->FillSpans = IDirectFBSurface_Requestor_FillSpans;
     thiz->DrawLine = IDirectFBSurface_Requestor_DrawLine;
     thiz->DrawLines = IDirectFBSurface_Requestor_DrawLines;
     thiz->DrawRectangle = IDirectFBSurface_Requestor_DrawRectangle;
     thiz->FillTriangle = IDirectFBSurface_Requestor_FillTriangle;

     thiz->SetFont = IDirectFBSurface_Requestor_SetFont;
     thiz->GetFont = IDirectFBSurface_Requestor_GetFont;
     thiz->DrawString = IDirectFBSurface_Requestor_DrawString;
     thiz->DrawGlyph = IDirectFBSurface_Requestor_DrawGlyph;
     thiz->SetEncoding = IDirectFBSurface_Requestor_SetEncoding;

     thiz->GetSubSurface = IDirectFBSurface_Requestor_GetSubSurface;

     thiz->GetGL = IDirectFBSurface_Requestor_GetGL;

     thiz->Dump = IDirectFBSurface_Requestor_Dump;

     thiz->DisableAcceleration = IDirectFBSurface_Requestor_DisableAcceleration;

     thiz->ReleaseSource = IDirectFBSurface_Requestor_ReleaseSource;

     thiz->SetIndexTranslation = IDirectFBSurface_Requestor_SetIndexTranslation;

     thiz->SetRenderOptions = IDirectFBSurface_Requestor_SetRenderOptions;
     thiz->SetMatrix = IDirectFBSurface_Requestor_SetMatrix;

     thiz->Read  = IDirectFBSurface_Requestor_Read;
     thiz->Write = IDirectFBSurface_Requestor_Write;

     return DFB_OK;
}

