/*
 * Copyright (C) 2006 Claudio Ciccani <klan@users.sf.net>
 *
 * Graphics driver for ATI Radeon cards written by
 *             Claudio Ciccani <klan@users.sf.net>.  
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */
 
#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <dfb_types.h>
#include <directfb.h>

#include <direct/types.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/state.h>
#include <core/gfxcard.h>

#include "radeon.h"
#include "radeon_regs.h"
#include "radeon_mmio.h"
#include "radeon_3d.h"


#define EMIT_VERTICES( rdrv, rdev, mmio ) { \
     u32 *_v = (rdev)->vb; \
     u32  _s = (rdev)->vb_size; \
     radeon_waitfifo( rdrv, rdev, 1 ); \
     radeon_out32( mmio, SE_VF_CNTL, rdev->vb_type | VF_PRIM_WALK_DATA | \
                                    (rdev->vb_count << VF_NUM_VERTICES_SHIFT) ); \
     do { \
          u32 _n = MIN(_s, 64); \
          _s -= _n; \
          radeon_waitfifo( rdrv, rdev, _n ); \
          while (_n--) \
               radeon_out32( mmio, SE_PORT_DATA0, *_v++ ); \
     } while (_s); \
}

static void
r200_flush_vb( RadeonDriverData *rdrv, RadeonDeviceData *rdev )
{
     volatile u8 *mmio = rdrv->mmio_base;
    
     EMIT_VERTICES( rdrv, rdev, mmio );

     if (DFB_PLANAR_PIXELFORMAT(rdev->dst_format)) {
          DFBRegion *clip = &rdev->clip;
          bool       s420 = DFB_PLANAR_PIXELFORMAT(rdev->src_format);
          int        i;
          
          if (DFB_BLITTING_FUNCTION(rdev->accel)) {
               for (i = 0; i < rdev->vb_size; i += 4) {
                    rdev->vb[i+0] = f2d(d2f(rdev->vb[i+0])*0.5f);
                    rdev->vb[i+1] = f2d(d2f(rdev->vb[i+1])*0.5f);
                    if (s420) {
                         rdev->vb[i+2] = f2d(d2f(rdev->vb[i+2])*0.5f);
                         rdev->vb[i+3] = f2d(d2f(rdev->vb[i+3])*0.5f);
                    }
               }
          } else {
               for (i = 0; i < rdev->vb_size; i += 2) {
                    rdev->vb[i+0] = f2d(d2f(rdev->vb[i+0])*0.5f);
                    rdev->vb[i+1] = f2d(d2f(rdev->vb[i+1])*0.5f);
               }
          }

          /* Prepare Cb plane */
          radeon_waitfifo( rdrv, rdev, 5 );
          radeon_out32( mmio, RB3D_COLOROFFSET, rdev->dst_offset_cb );
          radeon_out32( mmio, RB3D_COLORPITCH, rdev->dst_pitch/2 );
          radeon_out32( mmio, RE_TOP_LEFT, (clip->y1/2 << 16) |
                                           (clip->x1/2 & 0xffff) );
          radeon_out32( mmio, RE_BOTTOM_RIGHT, (clip->y2/2 << 16) |
                                               (clip->x2/2 & 0xffff) );
          if (DFB_BLITTING_FUNCTION(rdev->accel)) {
               radeon_out32( mmio, R200_PP_TFACTOR_0, rdev->cb_cop );
               if (s420) {
                    radeon_waitfifo( rdrv, rdev, 3 );
                    radeon_out32( mmio, R200_PP_TXSIZE_0, ((rdev->src_height/2-1) << 16) |
                                                          ((rdev->src_width/2-1) & 0xffff) );
                    radeon_out32( mmio, R200_PP_TXPITCH_0, rdev->src_pitch/2 - 32 );
                    radeon_out32( mmio, R200_PP_TXOFFSET_0, rdev->src_offset_cb );
               }
          } else {
               radeon_out32( mmio, R200_PP_TFACTOR_1, rdev->cb_cop );
          }

          /* Fill Cb plane */
          EMIT_VERTICES( rdrv, rdev, mmio );

          /* Prepare Cr plane */
          radeon_waitfifo( rdrv, rdev, 2 );
          radeon_out32( mmio, RB3D_COLOROFFSET, rdev->dst_offset_cr );
          if (DFB_BLITTING_FUNCTION(rdev->accel)) {
               radeon_out32( mmio, R200_PP_TFACTOR_0, rdev->cr_cop );
               if (s420) {
                    radeon_waitfifo( rdrv, rdev, 1 );
                    radeon_out32( mmio, R200_PP_TXOFFSET_0, rdev->src_offset_cr );
               }
          } else {
               radeon_out32( mmio, R200_PP_TFACTOR_1, rdev->cr_cop );
          }

          /* Fill Cr plane */
          EMIT_VERTICES( rdrv, rdev, mmio );

          /* Reset */
          radeon_waitfifo( rdrv, rdev, 5 );
          radeon_out32( mmio, RB3D_COLOROFFSET, rdev->dst_offset );
          radeon_out32( mmio, RB3D_COLORPITCH, rdev->dst_pitch );
          radeon_out32( mmio, RE_TOP_LEFT, (clip->y1 << 16) |
                                           (clip->x1 & 0xffff) );
          radeon_out32( mmio, RE_BOTTOM_RIGHT, (clip->y2 << 16) |
                                               (clip->x2 & 0xffff) );
          if (DFB_BLITTING_FUNCTION(rdev->accel)) {
               radeon_out32( mmio, R200_PP_TFACTOR_0, rdev->y_cop );
               if (s420) {
                    radeon_waitfifo( rdrv, rdev, 3 );
                    radeon_out32( mmio, R200_PP_TXSIZE_0, ((rdev->src_height-1) << 16) |
                                                          ((rdev->src_width-1) & 0xffff) );
                    radeon_out32( mmio, R200_PP_TXPITCH_0, rdev->src_pitch - 32 );
                    radeon_out32( mmio, R200_PP_TXOFFSET_0, rdev->src_offset );
               }
          } else {
               radeon_out32( mmio, R200_PP_TFACTOR_1, rdev->y_cop );
          }
     }
    
     rdev->vb_size  = 0;
     rdev->vb_count = 0;
}

static inline u32*
r200_init_vb( RadeonDriverData *rdrv, RadeonDeviceData *rdev, u32 type, u32 count, u32 size )
{
     u32 *vb;
    
     if ((rdev->vb_size && rdev->vb_type != type) ||
          rdev->vb_size+size > D_ARRAY_SIZE(rdev->vb))
          r200_flush_vb( rdrv, rdev );
        
     vb = &rdev->vb[rdev->vb_size];
     rdev->vb_type   = type;
     rdev->vb_size  += size;
     rdev->vb_count += count;
    
     return vb;
}


bool r200FillRectangle3D( void *drv, void *dev, DFBRectangle *rect )
{
     RadeonDriverData *rdrv = (RadeonDriverData*) drv;
     RadeonDeviceData *rdev = (RadeonDeviceData*) dev;
     float             x1, y1;
     float             x2, y2;
     u32              *v;

     if (rect->w == 1 && rect->h == 1) {
          x1 = rect->x+1; y1 = rect->y+1;
          if (rdev->matrix)
               RADEON_TRANSFORM( x1, y1, x1, y1, rdev->matrix, rdev->affine_matrix );

          v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_POINT_LIST, 1, 2 );
          *v++ = f2d(x1); *v++ = f2d(y1);

          return true;
     }
     
     x1 = rect->x;         y1 = rect->y;
     x2 = rect->x+rect->w; y2 = rect->y+rect->h;
     if (rdev->matrix) {
          float x, y;

          v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_QUAD_LIST, 4, 8 );
          RADEON_TRANSFORM( x1, y1, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);
          RADEON_TRANSFORM( x2, y1, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);
          RADEON_TRANSFORM( x2, y2, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);
          RADEON_TRANSFORM( x1, y2, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);     
     }
     else {
          v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_RECTANGLE_LIST, 3, 6 );
          *v++ = f2d(x1); *v++ = f2d(y1);
          *v++ = f2d(x2); *v++ = f2d(y1);
          *v++ = f2d(x2); *v++ = f2d(y2);
     }

     return true;
}

bool r200FillTriangle( void *drv, void *dev, DFBTriangle *tri )
{
     RadeonDriverData *rdrv = (RadeonDriverData*) drv;
     RadeonDeviceData *rdev = (RadeonDeviceData*) dev;
     float             x1, y1;
     float             x2, y2;
     float             x3, y3;
     u32              *v;

     x1 = tri->x1; y1 = tri->y1;
     x2 = tri->x2; y2 = tri->y2;
     x3 = tri->x3; y3 = tri->y3;
     if (rdev->matrix) {
          RADEON_TRANSFORM( x1, y1, x1, y1, rdev->matrix, rdev->affine_matrix );
          RADEON_TRANSFORM( x2, y2, x2, y2, rdev->matrix, rdev->affine_matrix );
          RADEON_TRANSFORM( x3, y3, x3, y3, rdev->matrix, rdev->affine_matrix );
     }

     v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_TRIANGLE_LIST, 3, 6 );
     *v++ = f2d(x1); *v++ = f2d(y1);
     *v++ = f2d(x2); *v++ = f2d(y2);
     *v++ = f2d(x3); *v++ = f2d(y3);
     
     return true;
}

bool r200DrawRectangle3D( void *drv, void *dev, DFBRectangle *rect )
{
     RadeonDriverData *rdrv = (RadeonDriverData*) drv;
     RadeonDeviceData *rdev = (RadeonDeviceData*) dev;
     float             x1, y1;
     float             x2, y2;
     u32              *v;
     
     x1 = rect->x;         y1 = rect->y;
     x2 = rect->x+rect->w; y2 = rect->y+rect->h;
     if (rdev->matrix) {
          float x, y;

          v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_LINE_LOOP, 4, 8 );
          RADEON_TRANSFORM( x1, y1, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);
          RADEON_TRANSFORM( x2, y1, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);
          RADEON_TRANSFORM( x2, y2, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);
          RADEON_TRANSFORM( x1, y2, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y);
     }
     else {
          v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_RECTANGLE_LIST, 12, 24 );
          /* top line */
          *v++ = f2d(x1);   *v++ = f2d(y1);
          *v++ = f2d(x2);   *v++ = f2d(y1);
          *v++ = f2d(x2);   *v++ = f2d(y1+1);
          /* right line */
          *v++ = f2d(x2-1); *v++ = f2d(y1+1);
          *v++ = f2d(x2);   *v++ = f2d(y1+1);
          *v++ = f2d(x2);   *v++ = f2d(y2-1);
          /* bottom line */
          *v++ = f2d(x1);   *v++ = f2d(y2-1);
          *v++ = f2d(x2);   *v++ = f2d(y2-1);
          *v++ = f2d(x2);   *v++ = f2d(y2);
          /* left line */
          *v++ = f2d(x1);   *v++ = f2d(y1+1);
          *v++ = f2d(x1+1); *v++ = f2d(y1+1);
          *v++ = f2d(x1+1); *v++ = f2d(y2-1);
     }

     return true;
}

bool r200DrawLine3D( void *drv, void *dev, DFBRegion *line )
{
     RadeonDriverData *rdrv = (RadeonDriverData*) drv;
     RadeonDeviceData *rdev = (RadeonDeviceData*) dev;
     float             x1, y1;
     float             x2, y2;
     u32              *v;

     x1 = line->x1; y1 = line->y1;
     x2 = line->x2; y2 = line->y2;
     if (rdev->matrix) {
          RADEON_TRANSFORM( x1, y1, x1, y1, rdev->matrix, rdev->affine_matrix );
          RADEON_TRANSFORM( x2, y2, x2, y2, rdev->matrix, rdev->affine_matrix );
     }

     v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_LINE_LIST, 2, 4 );
     *v++ = f2d(x1); *v++ = f2d(y1);
     *v++ = f2d(x2); *v++ = f2d(y2);

     return true;
}

bool r200Blit3D( void *drv, void *dev, DFBRectangle *sr, int dx, int dy )
{
     DFBRectangle dr = { dx, dy, sr->w, sr->h };
     
     return r200StretchBlit( drv, dev, sr, &dr );
}

bool r200StretchBlit( void *drv, void *dev, DFBRectangle *sr, DFBRectangle *dr )
{
     RadeonDriverData *rdrv = (RadeonDriverData*) drv;
     RadeonDeviceData *rdev = (RadeonDeviceData*) dev;
     float             x1, y1;
     float             x2, y2;
     float             s1, t1;
     float             s2, t2;
     u32              *v;
    
     if (rdev->blittingflags & DSBLIT_DEINTERLACE) {
          sr->y /= 2;
          sr->h /= 2;
     }

     s1 = sr->x;       t1 = sr->y;
     s2 = sr->x+sr->w; t2 = sr->y+sr->h;
     if (rdev->blittingflags & DSBLIT_ROTATE180) {
          float tmp;
          tmp = s2; s2 = s1; s1 = tmp;
          tmp = t2; t2 = t1; t1 = tmp;
     }

     x1 = dr->x;       y1 = dr->y;
     x2 = dr->x+dr->w; y2 = dr->y+dr->h;
     if (rdev->matrix) {
          float x, y;

          v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_QUAD_LIST, 4, 16 );
          RADEON_TRANSFORM( x1, y1, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y); *v++ = f2d(s1); *v++ = f2d(t1);
          RADEON_TRANSFORM( x2, y1, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y); *v++ = f2d(s2); *v++ = f2d(t1);
          RADEON_TRANSFORM( x2, y2, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y); *v++ = f2d(s2); *v++ = f2d(t2);
          RADEON_TRANSFORM( x1, y2, x, y, rdev->matrix, rdev->affine_matrix );
          *v++ = f2d(x); *v++ = f2d(y); *v++ = f2d(s1); *v++ = f2d(t2);
     }
     else {
          v = r200_init_vb( rdrv, rdev, VF_PRIM_TYPE_RECTANGLE_LIST, 3, 12 );
          *v++ = f2d(x1); *v++ = f2d(y1); *v++ = f2d(s1); *v++ = f2d(t1);
          *v++ = f2d(x2); *v++ = f2d(y1); *v++ = f2d(s2); *v++ = f2d(t1);
          *v++ = f2d(x2); *v++ = f2d(y2); *v++ = f2d(s2); *v++ = f2d(t2);
     }

     return true;
}

static void
r200DoTextureTriangles( RadeonDriverData *rdrv, RadeonDeviceData *rdev,
                        DFBVertex *ve, int num, u32 primitive )
{
     volatile u8 *mmio = rdrv->mmio_base;
     int          i;
 
     radeon_waitfifo( rdrv, rdev, 1 ); 
     
     radeon_out32( mmio, SE_VF_CNTL, primitive | VF_PRIM_WALK_DATA |
                                     (num << VF_NUM_VERTICES_SHIFT) );

     for (; num >= 10; num -= 10) {
          radeon_waitfifo( rdrv, rdev, 60 );
          for (i = 0; i < 10; i++) {
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].x) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].y) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].z) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].w) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].s) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].t) );
          }
          ve += 10;
     }

     if (num > 0) {
          radeon_waitfifo( rdrv, rdev, num*6 );
          for (i = 0; i < num; i++) {
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].x) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].y) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].z) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].w) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].s) );
               radeon_out32( mmio, SE_PORT_DATA0, f2d(ve[i].t) );
          }
     }
}

bool r200TextureTriangles( void *drv, void *dev, DFBVertex *ve,
                           int num, DFBTriangleFormation formation )
{ 
     RadeonDriverData *rdrv = (RadeonDriverData*) drv;
     RadeonDeviceData *rdev = (RadeonDeviceData*) dev;
     u32               prim = 0;
     int               i;

     if (num > 65535) {
          D_WARN( "R200 supports maximum 65535 vertices" );
          return false;
     }

     switch (formation) {
          case DTTF_LIST:
               prim = VF_PRIM_TYPE_TRIANGLE_LIST;
               break;
          case DTTF_STRIP:
               prim = VF_PRIM_TYPE_TRIANGLE_STRIP;
               break;
          case DTTF_FAN:
               prim = VF_PRIM_TYPE_TRIANGLE_FAN;
               break;
          default:
               D_BUG( "unexpected triangle formation" );
               return false;
     }

     if (rdev->matrix) {
          for (i = 0; i < num; i++)
               RADEON_TRANSFORM( ve[i].x, ve[i].y, ve[i].x, ve[i].y, rdev->matrix, rdev->affine_matrix );
     }

     r200DoTextureTriangles( rdrv, rdev, ve, num, prim );
     
     if (DFB_PLANAR_PIXELFORMAT(rdev->dst_format)) {
          DFBRegion   *clip = &rdev->clip;
          volatile u8 *mmio = rdrv->mmio_base;
          bool         s420 = DFB_PLANAR_PIXELFORMAT(rdev->src_format);
          
          /* Scale coordinates */
          for (i = 0; i < num; i++) {
               ve[i].x *= 0.5;
               ve[i].y *= 0.5;
          }

          /* Prepare Cb plane */
          radeon_waitfifo( rdrv, rdev, s420 ? 8 : 5 );
          radeon_out32( mmio, RB3D_COLOROFFSET, rdev->dst_offset_cb );
          radeon_out32( mmio, RB3D_COLORPITCH, rdev->dst_pitch/2 );
          if (s420) {
               radeon_out32( mmio, R200_PP_TXSIZE_0, ((rdev->src_height/2-1) << 16) |
                                                     ((rdev->src_width/2-1) & 0xffff) );
               radeon_out32( mmio, R200_PP_TXPITCH_0, rdev->src_pitch/2 - 32 );
               radeon_out32( mmio, R200_PP_TXOFFSET_0, rdev->src_offset_cb );
          }
          radeon_out32( mmio, RE_TOP_LEFT, (clip->y1/2 << 16) |
                                           (clip->x1/2 & 0xffff) );
          radeon_out32( mmio, RE_BOTTOM_RIGHT, (clip->y2/2 << 16) | 
                                               (clip->x2/2 & 0xffff) );
          radeon_out32( mmio, R200_PP_TFACTOR_0, rdev->cb_cop );
     
          /* Map Cb plane */
          r200DoTextureTriangles( rdrv, rdev, ve, num, prim );
     
          /* Prepare Cr plane */
          radeon_waitfifo( rdrv, rdev, s420 ? 3 : 2 );
          radeon_out32( mmio, RB3D_COLOROFFSET, rdev->dst_offset_cr );
          if (s420)
               radeon_out32( mmio, R200_PP_TXOFFSET_0, rdev->src_offset_cr );
          radeon_out32( mmio, R200_PP_TFACTOR_0, rdev->cr_cop );

          /* Map Cr plane */
          r200DoTextureTriangles( rdrv, rdev, ve, num, prim );
     
          /* Reset */
          radeon_waitfifo( rdrv, rdev, s420 ? 8 : 5 );
          radeon_out32( mmio, RB3D_COLOROFFSET, rdev->dst_offset );
          radeon_out32( mmio, RB3D_COLORPITCH, rdev->dst_pitch );
          if (s420) {
               radeon_out32( mmio, R200_PP_TXSIZE_0, ((rdev->src_height-1) << 16) |
                                                     ((rdev->src_width-1) & 0xffff) );
               radeon_out32( mmio, R200_PP_TXPITCH_0, rdev->src_pitch - 32 );
               radeon_out32( mmio, R200_PP_TXOFFSET_0, rdev->src_offset );
          }
          radeon_out32( mmio, RE_TOP_LEFT, (clip->y1 << 16) |
                                           (clip->x1 & 0xffff) );
          radeon_out32( mmio, RE_BOTTOM_RIGHT, (clip->y2 << 16) |
                                               (clip->x2 & 0xffff) );
          radeon_out32( mmio, R200_PP_TFACTOR_0, rdev->y_cop );
     }
     
     return true;
}

void r200EmitCommands3D( void *drv, void *dev )
{
     RadeonDriverData *rdrv = (RadeonDriverData*) drv;
     RadeonDeviceData *rdev = (RadeonDeviceData*) dev;
     
     if (rdev->vb_count)
          r200_flush_vb( rdrv, rdev );
}
