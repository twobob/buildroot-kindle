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

#ifndef __IDIRECTFBSURFACE_DISPATCHER_H__
#define __IDIRECTFBSURFACE_DISPATCHER_H__

#define IDIRECTFBSURFACE_METHOD_ID_AddRef                     1
#define IDIRECTFBSURFACE_METHOD_ID_Release                    2
#define IDIRECTFBSURFACE_METHOD_ID_GetCapabilities            3
#define IDIRECTFBSURFACE_METHOD_ID_GetSize                    4
#define IDIRECTFBSURFACE_METHOD_ID_GetVisibleRectangle        5
#define IDIRECTFBSURFACE_METHOD_ID_GetPixelFormat             6
#define IDIRECTFBSURFACE_METHOD_ID_GetAccelerationMask        7
#define IDIRECTFBSURFACE_METHOD_ID_GetPalette                 8
#define IDIRECTFBSURFACE_METHOD_ID_SetPalette                 9
#define IDIRECTFBSURFACE_METHOD_ID_Lock                      10
#define IDIRECTFBSURFACE_METHOD_ID_Unlock                    11
#define IDIRECTFBSURFACE_METHOD_ID_Flip                      12
#define IDIRECTFBSURFACE_METHOD_ID_SetField                  13
#define IDIRECTFBSURFACE_METHOD_ID_Clear                     14
#define IDIRECTFBSURFACE_METHOD_ID_SetClip                   15
#define IDIRECTFBSURFACE_METHOD_ID_SetColor                  16
#define IDIRECTFBSURFACE_METHOD_ID_SetColorIndex             17
#define IDIRECTFBSURFACE_METHOD_ID_SetSrcBlendFunction       18
#define IDIRECTFBSURFACE_METHOD_ID_SetDstBlendFunction       19
#define IDIRECTFBSURFACE_METHOD_ID_SetPorterDuff             20
#define IDIRECTFBSURFACE_METHOD_ID_SetSrcColorKey            21
#define IDIRECTFBSURFACE_METHOD_ID_SetSrcColorKeyIndex       22
#define IDIRECTFBSURFACE_METHOD_ID_SetDstColorKey            23
#define IDIRECTFBSURFACE_METHOD_ID_SetDstColorKeyIndex       24
#define IDIRECTFBSURFACE_METHOD_ID_SetBlittingFlags          25
#define IDIRECTFBSURFACE_METHOD_ID_Blit                      26
#define IDIRECTFBSURFACE_METHOD_ID_TileBlit                  27
#define IDIRECTFBSURFACE_METHOD_ID_BatchBlit                 28
#define IDIRECTFBSURFACE_METHOD_ID_StretchBlit               29
#define IDIRECTFBSURFACE_METHOD_ID_TextureTriangles          30
#define IDIRECTFBSURFACE_METHOD_ID_SetDrawingFlags           31
#define IDIRECTFBSURFACE_METHOD_ID_FillRectangle             32
#define IDIRECTFBSURFACE_METHOD_ID_DrawLine                  33
#define IDIRECTFBSURFACE_METHOD_ID_DrawLines                 34
#define IDIRECTFBSURFACE_METHOD_ID_DrawRectangle             35
#define IDIRECTFBSURFACE_METHOD_ID_FillTriangle              36
#define IDIRECTFBSURFACE_METHOD_ID_SetFont                   37
#define IDIRECTFBSURFACE_METHOD_ID_GetFont                   38
#define IDIRECTFBSURFACE_METHOD_ID_DrawString                39
#define IDIRECTFBSURFACE_METHOD_ID_DrawGlyph                 40
#define IDIRECTFBSURFACE_METHOD_ID_GetSubSurface             41
#define IDIRECTFBSURFACE_METHOD_ID_GetGL                     42
#define IDIRECTFBSURFACE_METHOD_ID_Dump                      43
#define IDIRECTFBSURFACE_METHOD_ID_FillRectangles            44
#define IDIRECTFBSURFACE_METHOD_ID_FillSpans                 45
#define IDIRECTFBSURFACE_METHOD_ID_GetPosition               46
#define IDIRECTFBSURFACE_METHOD_ID_SetEncoding               47
#define IDIRECTFBSURFACE_METHOD_ID_DisableAcceleration       48
#define IDIRECTFBSURFACE_METHOD_ID_ReleaseSource             49
#define IDIRECTFBSURFACE_METHOD_ID_SetIndexTranslation       50
#define IDIRECTFBSURFACE_METHOD_ID_SetRenderOptions          51
#define IDIRECTFBSURFACE_METHOD_ID_SetMatrix                 52
#define IDIRECTFBSURFACE_METHOD_ID_SetSourceMask             53
#define IDIRECTFBSURFACE_METHOD_ID_MakeSubSurface            54
#define IDIRECTFBSURFACE_METHOD_ID_Write                     55
#define IDIRECTFBSURFACE_METHOD_ID_Read                      56
#define IDIRECTFBSURFACE_METHOD_ID_SetColors                 57
#define IDIRECTFBSURFACE_METHOD_ID_BatchBlit2                58
#define IDIRECTFBSURFACE_METHOD_ID_SetRemoteInstance         59
#define IDIRECTFBSURFACE_METHOD_ID_FillTrapezoids            60

#endif
