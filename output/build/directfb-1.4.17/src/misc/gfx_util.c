/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   Scaling routines ported from gdk_pixbuf by Sven Neumann
   <sven@convergence.de>.

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

#include <stdlib.h>
#include <stdio.h>

#include <pthread.h>

#include <directfb.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/palette.h>
#include <core/surface.h>

#include <direct/memcpy.h>
#include <direct/mem.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <misc/util.h>
#include <misc/dither.h>
#include <misc/dither565.h>
#include <misc/gfx_util.h>

#include <gfx/clip.h>
#include <gfx/convert.h>


#define SUBSAMPLE_BITS 4
#define SUBSAMPLE (1 << SUBSAMPLE_BITS)
#define SUBSAMPLE_MASK ((1 << SUBSAMPLE_BITS)-1)
#define SCALE_SHIFT 16


typedef struct _PixopsFilter PixopsFilter;

struct _PixopsFilter {
     int *weights;
     int n_x;
     int n_y;
     float x_offset;
     float y_offset;
};


static void write_argb_span (u32 *src, u8 *dst[], int len,
                             int dx, int dy, CoreSurface *dst_surface)
{
     CorePalette *palette = dst_surface->palette;
     u8          *d       = dst[0];
     u8          *d1,*d2;
     int          i, j;

     if (dst_surface->config.caps & DSCAPS_PREMULTIPLIED) {
          for (i = 0; i < len; i++) {
               const u32 s = src[i];
               const u32 a = (s >> 24) + 1;

               src[i] = ((((s & 0x00ff00ff) * a) >> 8) & 0x00ff00ff) |
                        ((((s & 0x0000ff00) * a) >> 8) & 0x0000ff00) |
                        ((((s & 0xff000000)    )     )             );
          }
     }

     switch (dst_surface->config.format) {
          case DSPF_A1:
               for (i = 0; i < len; i++) {
                    if (i & 7)
                         d[i>>3] |= (src[i] >> 31) << (7-(i&7));
                    else
                         d[i>>3]  = (src[i] >> 24) & 0x80;
               }
               break;
          case DSPF_A1_LSB:
               for (i = 0; i < len; i++) {
                    if (i & 7)
                         d[i>>3] |= (src[i] >> 31) << (i&7);
                    else
                         d[i>>3]  = (src[i] >> 31);
               }
               break;

          case DSPF_A4:
               for (i = 0, j = 0; i < len; i += 2, j++)
                    d[j] = ((src[i] >> 24) & 0xF0) | (src[i+1] >> 28);
               break;

          case DSPF_A8:
               for (i = 0; i < len; i++)
                    d[i] = src[i] >> 24;
               break;

          case DSPF_RGB332:
               for (i = 0; i < len; i++)
                    d[i] = RGB32_TO_RGB332( src[i] );
               break;

          case DSPF_ARGB1555:
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_ARGB1555( src[i] );
               break;

          case DSPF_ARGB2554:
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_ARGB2554( src[i] );
               break;

          case DSPF_ARGB4444:
#ifdef DFB_DITHER
               /* use a pre-generated dither matrix to improve the appearance of the result */
               {
                    const u8 *dm = DM[ dy & (DM_HEIGHT - 1) ];

                    for (i = 0; i < len; i++) {
                         u32 x = (dm[(dx + i) & (DM_WIDTH - 1)] << 2) >> 4;
                         u32 a = ((src[i] & 0xFF000000) >> 24) + x;
                         u32 r = ((src[i] & 0x00FF0000) >> 16) + x;
                         u32 g = ((src[i] & 0x0000FF00) >>  8) + x;
                         u32 b = ((src[i] & 0x000000FF)      ) + x;

                         ((u16*)d)[i] = PIXEL_ARGB4444( a - (a >> 4),
                                                        r - (r >> 4),
                                                        g - (g >> 4),
                                                        b - (b >> 4) );
                    }
               }
#else
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_ARGB4444( src[i] );
#endif
               break;

          case DSPF_RGBA4444:
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_RGBA4444( src[i] );
               break;

          case DSPF_ARGB8565:
               for (i = 0, j = -1; i < len; i++) {
                    register u32 pixel = ARGB_TO_ARGB8565 ( src[i] );
#ifdef WORDS_BIGENDIAN
                    d[++j] = (pixel >> 16) & 0xff;
                    d[++j] = (pixel >>  8) & 0xff;
                    d[++j] = (pixel >>  0) & 0xff;
#else
                    d[++j] = (pixel >>  0) & 0xff;
                    d[++j] = (pixel >>  8) & 0xff;
                    d[++j] = (pixel >> 16) & 0xff;
#endif
               }
               break;

          case DSPF_RGB16:
#ifdef DFB_DITHER565
               /* use a pre-generated dither matrix to improve the appearance of the result */
               {
                    const u32 *dm = DM_565 + ((dy & (DM_565_HEIGHT - 1)) << DM_565_WIDTH_SHIFT);

                    for (i = 0; i < len; i++) {
                         u32 rgb = ((src[i] & 0xFF)          |
                                    (src[i] & 0xFF00)   << 2 |
                                    (src[i] & 0xFF0000) << 4);

                         rgb += dm[(dx + i) & (DM_565_WIDTH - 1)];
                         rgb += (0x10040100
                                 - ((rgb & 0x1e0001e0) >> 5)
                                 - ((rgb & 0x00070000) >> 6));

                         ((u16*)d)[i] = (((rgb & 0x0f800000) >> 12) |
                                         ((rgb & 0x0003f000) >> 7)  |
                                         ((rgb & 0x000000f8) >> 3));
                    }
               }
#else
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = RGB32_TO_RGB16( src[i] );
#endif
               break;

          case DSPF_ARGB1666:
               for (i = 0; i < len; i++) {
                    const u32 pixel = PIXEL_ARGB1666( src[i] >> 24,
                                                      src[i] >> 16,
                                                      src[i] >> 8,
                                                      src[i] );

                    *d++ = pixel;
                    *d++ = pixel >> 8;
                    *d++ = pixel >> 16;
               }
               break;

          case DSPF_ARGB6666:
               for (i = 0; i < len; i++) {
                    const u32 pixel = PIXEL_ARGB6666( src[i] >> 24,
                                                      src[i] >> 16,
                                                      src[i] >> 8,
                                                      src[i] );

                    *d++ = pixel;
                    *d++ = pixel >> 8;
                    *d++ = pixel >> 16;
               }
               break;

          case DSPF_RGB18:
               for (i = 0; i < len; i++) {
                    const u32 pixel = PIXEL_RGB18( src[i] >> 16,
                                                   src[i] >> 8,
                                                   src[i] );

                    *d++ = pixel;
                    *d++ = pixel >> 8;
                    *d++ = pixel >> 16;
               }
               break;

          case DSPF_RGB24:
               for (i = 0; i < len; i++) {
#ifdef WORDS_BIGENDIAN
                    *d++ = src[i] >> 16;
                    *d++ = src[i] >> 8;
                    *d++ = src[i];
#else
                    *d++ = src[i];
                    *d++ = src[i] >> 8;
                    *d++ = src[i] >> 16;
#endif
               }
               break;

          case DSPF_RGB32:
          case DSPF_ARGB:
               direct_memcpy( d, src, len*4 );
               break;

          case DSPF_AiRGB:
               for (i = 0; i < len; i++)
                    ((u32*)d)[i] = src[i] ^ 0xff000000;
               break;

          case DSPF_LUT8:
               if (palette) {
                    for (i = 0; i < len; i++) {
                         d[i] = dfb_palette_search( palette,
                                                    (src[i] >> 16) & 0xff,
                                                    (src[i] >>  8) & 0xff,
                                                    (src[i]      ) & 0xff,
                                                    (src[i] >> 24) & 0xff );
                    }
               }
               break;

          case DSPF_ALUT44:
               if (palette) {
                    for (i = 0; i < len; i++) {
                         d[i] = ((src[i] >> 24) & 0xf0) +
                                dfb_palette_search( palette,
                                                    (src[i] >> 16) & 0xff,
                                                    (src[i] >>  8) & 0xff,
                                                    (src[i]      ) & 0xff, 0x80 );
                    }
               }
               break;

          case DSPF_YUY2:
               if (dx & 1) {
                    u32 y, u, v;

                    RGB_TO_YCBCR( (src[0] >> 16) & 0xff,
                                  (src[0] >>  8) & 0xff,
                                  (src[0]      ) & 0xff, y, u, v );
                    *((u16*)d) = y | (v << 8);
                    d += 2;
                    src++;
                    len--;
               }
               for (i = 0; i < (len-1); i += 2) {
                    u32 y0, u, v;
                    u32 y1, u1, v1;

                    RGB_TO_YCBCR( (src[i+0] >> 16) & 0xff,
                                  (src[i+0] >>  8) & 0xff,
                                  (src[i+0]      ) & 0xff, y0, u, v );
                    RGB_TO_YCBCR( (src[i+1] >> 16) & 0xff,
                                  (src[i+1] >>  8) & 0xff,
                                  (src[i+1]      ) & 0xff, y1, u1, v1 );

                    u = (u + u1) >> 1;
                    v = (v + v1) >> 1;

                    ((u16*)d)[i+0] = y0 | (u << 8);
                    ((u16*)d)[i+1] = y1 | (v << 8);
               }
               if (len & 1) {
                    u32 y, u, v;

                    src += len-1;
                    d   += (len-1) * 2;

                    RGB_TO_YCBCR( (*src >> 16) & 0xff,
                                  (*src >>  8) & 0xff,
                                  (*src      ) & 0xff, y, u, v );
                    *((u16*)d) = y | (u << 8);
               }
               break;

          case DSPF_UYVY:
               if (dx & 1) {
                    u32 y, u, v;

                    RGB_TO_YCBCR( (src[0] >> 16) & 0xff,
                                  (src[0] >>  8) & 0xff,
                                  (src[0]      ) & 0xff, y, u, v );
                    *((u16*)d) = v | (y << 8);
                    d += 2;
                    src++;
                    len--;
               }
               for (i = 0; i < (len-1); i += 2) {
                    u32 y0, u, v;
                    u32 y1, u1, v1;

                    RGB_TO_YCBCR( (src[i+0] >> 16) & 0xff,
                                  (src[i+0] >>  8) & 0xff,
                                  (src[i+0]      ) & 0xff, y0, u, v );
                    RGB_TO_YCBCR( (src[i+1] >> 16) & 0xff,
                                  (src[i+1] >>  8) & 0xff,
                                  (src[i+1]      ) & 0xff, y1, u1, v1 );

                    u = (u + u1) >> 1;
                    v = (v + v1) >> 1;

                    ((u16*)d)[i+0] = u | (y0 << 8);
                    ((u16*)d)[i+1] = v | (y1 << 8);
               }
               if (len & 1) {
                    u32 y, u, v;

                    src += len-1;
                    d   += (len-1) * 2;

                    RGB_TO_YCBCR( (*src >> 16) & 0xff,
                                  (*src >>  8) & 0xff,
                                  (*src      ) & 0xff, y, u, v );
                    *((u16*)d) = u | (y << 8);
               }
               break;

          case DSPF_AYUV:
               for (i = 0; i < len; i++) {
                    u32 a, y, u, v;

                    RGB_TO_YCBCR( (src[i] >> 16) & 0xff,
                                  (src[i] >>  8) & 0xff,
                                  (src[i]      ) & 0xff, y, u, v );
                    a = (src[i] >> 24) & 0xff;

                    ((u32*)d)[i] = PIXEL_AYUV( a, y, u, v );
               }
               break;

          case DSPF_AVYU:
               for (i = 0; i < len; i++) {
                    u32 a, y, u, v;

                    RGB_TO_YCBCR( (src[i] >> 16) & 0xff,
                                  (src[i] >>  8) & 0xff,
                                  (src[i]      ) & 0xff, y, u, v );
                    a = (src[i] >> 24) & 0xff;

                    ((u32*)d)[i] = PIXEL_AVYU( a, y, u, v );
               }
               break;

          case DSPF_VYU:
               for (i = 0, j = -1; i < len; i++) {
                    u32 y, u, v;

                    RGB_TO_YCBCR( (src[i] >> 16) & 0xff,
                                  (src[i] >>  8) & 0xff,
                                  (src[i]      ) & 0xff, y, u, v );

#ifdef WORDS_BIGENDIAN
                    d[++j] = v;
                    d[++j] = y;
                    d[++j] = u;
#else
                    d[++j] = u;
                    d[++j] = y;
                    d[++j] = v;
#endif
               }
               break;

          case DSPF_YV12:
          case DSPF_YV16:
          case DSPF_I420:
               d1 = dst[1];
               d2 = dst[2];
               for (i = 0; i < (len-1); i += 2) {
                    u32 y0, u0, v0;
                    u32 y1, u1, v1;

                    RGB_TO_YCBCR( (src[i+0] >> 16) & 0xff,
                                  (src[i+0] >>  8) & 0xff,
                                  (src[i+0]      ) & 0xff, y0, u0, v0 );
                    RGB_TO_YCBCR( (src[i+1] >> 16) & 0xff,
                                  (src[i+1] >>  8) & 0xff,
                                  (src[i+1]      ) & 0xff, y1, u1, v1 );

                    d[i+0] = y0;
                    d[i+1] = y1;

                    if (dy & 1) {
                         d1[i>>1] = (u0 + u1) >> 1;
                         d2[i>>1] = (v0 + v1) >> 1;
                    }
               }
               if (len & 1) {
                    u32 y, u, v;

                    i = len-1;

                    RGB_TO_YCBCR( (src[i] >> 16) & 0xff,
                                  (src[i] >>  8) & 0xff,
                                  (src[i]      ) & 0xff, y, u, v );

                    d[i] = y;
                    if (dy & 1) {
                         d1[i>>1] = u;
                         d2[i>>1] = v;
                    }
               }
               break;

          case DSPF_NV12:
          case DSPF_NV16:
               d1 = dst[1];
               for (i = 0; i < (len-1); i += 2) {
                    u32 y0, u0, v0;
                    u32 y1, u1, v1;

                    RGB_TO_YCBCR( (src[i+0] >> 16) & 0xff,
                                  (src[i+0] >>  8) & 0xff,
                                  (src[i+0]      ) & 0xff, y0, u0, v0 );
                    RGB_TO_YCBCR( (src[i+1] >> 16) & 0xff,
                                  (src[i+1] >>  8) & 0xff,
                                  (src[i+1]      ) & 0xff, y1, u1, v1 );

                    d[i+0] = y0;
                    d[i+1] = y1;

                    if (dst_surface->config.format == DSPF_NV16 || dy & 1) {
#ifdef WORDS_BIGENDIAN
                         ((u16*)d1)[i>>1] =    ((v0 + v1) >> 1)     |
                                              (((u0 + u1) >> 1) << 8);
#else
                         ((u16*)d1)[i>>1] =    ((u0 + u1) >> 1)     |
                                              (((v0 + v1) >> 1) << 8);
#endif
                    }
               }
               if (len & 1) {
                    u32 y, u, v;

                    i = len-1;

                    RGB_TO_YCBCR( (src[i] >> 16) & 0xff,
                                  (src[i] >>  8) & 0xff,
                                  (src[i]      ) & 0xff, y, u, v );

                    d[i] = y;
                    if (dst_surface->config.format == DSPF_NV16 || dy & 1)
#ifdef WORDS_BIGENDIAN
                         ((u16*)d1)[i>>1] = v | (u << 8);
#else
                         ((u16*)d1)[i>>1] = u | (v << 8);
#endif
               }
               break;

          case DSPF_NV21:
               d1 = dst[1];
               for (i = 0; i < (len-1); i += 2) {
                    u32 y0, u0, v0;
                    u32 y1, u1, v1;

                    RGB_TO_YCBCR( (src[i+0] >> 16) & 0xff,
                                  (src[i+0] >>  8) & 0xff,
                                  (src[i+0]      ) & 0xff, y0, u0, v0 );
                    RGB_TO_YCBCR( (src[i+1] >> 16) & 0xff,
                                  (src[i+1] >>  8) & 0xff,
                                  (src[i+1]      ) & 0xff, y1, u1, v1 );

                    d[i+0] = y0;
                    d[i+1] = y1;

                    if (dy & 1) {
#ifdef WORDS_BIGENDIAN
                         ((u16*)d1)[i>>1] =    ((u0 + u1) >> 1)     |
                                              (((v0 + v1) >> 1) << 8);
#else
                         ((u16*)d1)[i>>1] =    ((v0 + v1) >> 1)     |
                                              (((u0 + u1) >> 1) << 8);
#endif
                    }
               }
               if (len & 1) {
                    u32 y, u, v;

                    i = len-1;

                    RGB_TO_YCBCR( (src[i] >> 16) & 0xff,
                                  (src[i] >>  8) & 0xff,
                                  (src[i]      ) & 0xff, y, u, v );

                    d[i] = y;
                    if (dy & 1)
#ifdef WORDS_BIGENDIAN
                         ((u16*)d1)[i>>1] = u | (v << 8);
#else
                         ((u16*)d1)[i>>1] = v | (u << 8);
#endif
               }
               break;

          case DSPF_RGB555:
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_RGB555( src[i] );
               break;

          case DSPF_BGR555:
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_BGR555( src[i] );
               break;

          case DSPF_RGB444:
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_RGB444( src[i] );
               break;

          case DSPF_RGBA5551:
               for (i = 0; i < len; i++)
                    ((u16*)d)[i] = ARGB_TO_RGBA5551( src[i] );
               break;

          case DSPF_YUV444P:
               {
               u8 * __restrict Dy = dst[0];
               u8 * __restrict Du = dst[1];
               u8 * __restrict Dv = dst[2];
               for (i = 0; i < len; i++) {
                    u32 y, u, v;

                    RGB_TO_YCBCR( (src[i] >> 16) & 0xff,
                                  (src[i] >>  8) & 0xff,
                                  (src[i]      ) & 0xff, y, u, v );

                    Dy[i] = y;
                    Du[i] = u;
                    Dv[i] = v;
               }
               }
               break;

          default:
               D_ONCE( "unimplemented destination format (0x%08x)", dst_surface->config.format );
               break;
     }
}

#define LINE_PTR(dst,caps,y,h,pitch) \
     ((caps & DSCAPS_SEPARATED) \
          ? (((u8*)(dst)) + (y)/2 * (pitch) + (((y)%2) ? (h)/2 * (pitch) : 0)) \
          : (((u8*)(dst)) + (y) * (pitch)))

void dfb_copy_buffer_32( u32 *src,
                         void  *dst, int dpitch, DFBRectangle *drect,
                         CoreSurface *dst_surface, const DFBRegion *dst_clip )
{
     u8   *dst1, *dst2;
     int   sw = drect->w;
     int   y, x;

     if (dst_clip) {
          int sx = 0, sy = 0;

          if (drect->x < dst_clip->x1) {
               sx = dst_clip->x1 - drect->x;
               drect->w -= sx;
               drect->x += sx;
          }
          if (drect->y < dst_clip->y1) {
               sy = dst_clip->y1 - drect->y;
               drect->h -= sy;
               drect->y += sy;
          }
          if ((drect->x + drect->w - 1) > dst_clip->x2) {
               drect->w -= drect->x + drect->w - 1 - dst_clip->x2;
          }
          if ((drect->y + drect->h - 1) > dst_clip->y2) {
               drect->h -= drect->y + drect-> h - 1 - dst_clip->y2;
          }

          src += sy * sw + sx;
     }

     if (drect->w < 1 || drect->h < 1)
          return;
     x = drect->x;

     switch (dst_surface->config.format) {
          case DSPF_YV12:
          case DSPF_I420:
               if (dst_surface->config.format == DSPF_I420) {
                    dst1 = (u8*)dst  + dpitch   * dst_surface->config.size.h;
                    dst2 = dst1 + dpitch/2 * dst_surface->config.size.h/2;
               } else {
                    dst2 = (u8*)dst  + dpitch   * dst_surface->config.size.h;
                    dst1 = dst2 + dpitch/2 * dst_surface->config.size.h/2;
               }

               for (y = drect->y; y < drect->y + drect->h; y++) {
                    u8 *d[3];

                    d[0] = LINE_PTR( dst, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + x;
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, y/2,
                                     dst_surface->config.size.h/2, dpitch/2 ) + x/2;
                    d[2] = LINE_PTR( dst2, dst_surface->config.caps, y/2,
                                     dst_surface->config.size.h/2, dpitch/2 ) + x/2;

                    write_argb_span( src, d, drect->w, x, y, dst_surface );

                    src += sw;
               }
               break;

          case DSPF_YV16:
               dst2 = (u8*)dst  + dpitch   * dst_surface->config.size.h;
               dst1 = dst2 + dpitch/2 * dst_surface->config.size.h;

               for (y = drect->y; y < drect->y + drect->h; y++) {
                    u8 *d[3];

                    d[0] = LINE_PTR( dst, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + x;
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch/2 ) + x/2;
                    d[2] = LINE_PTR( dst2, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch/2 ) + x/2;

                    write_argb_span( src, d, drect->w, x, y, dst_surface );

                    src += sw;
               }
               break;

          case DSPF_NV12:
          case DSPF_NV21:
               dst1 = (u8*)dst + dpitch * dst_surface->config.size.h;

               for (y = drect->y; y < drect->y + drect->h; y++) {
                    u8 *d[2];

                    d[0] = LINE_PTR( dst, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + x;
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, y/2,
                                     dst_surface->config.size.h/2, dpitch ) + (x&~1);

                    write_argb_span( src, d, drect->w, x, y, dst_surface );

                    src += sw;
               }
               break;

          case DSPF_NV16:
               dst1 = (u8*)dst + dpitch * dst_surface->config.size.h;

               for (y = drect->y; y < drect->y + drect->h; y++) {
                    u8 *d[2];

                    d[0] = LINE_PTR( dst, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + x;
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + (x&~1);

                    write_argb_span( src, d, drect->w, x, y, dst_surface );

                    src += sw;
               }
               break;

          case DSPF_YUV444P:
               dst1 = (u8*)dst  + dpitch * dst_surface->config.size.h;
               dst2 = dst1 + dpitch * dst_surface->config.size.h;

               for (y = drect->y; y < drect->y + drect->h; y++) {
                    u8 *d[3];

                    d[0] = LINE_PTR( dst, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + x;
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + x;
                    d[2] = LINE_PTR( dst2, dst_surface->config.caps, y,
                                     dst_surface->config.size.h, dpitch ) + x;

                    write_argb_span( src, d, drect->w, x, y, dst_surface );

                    src += sw;
            }
            break;

          default:
               for (y = drect->y; y < drect->y + drect->h; y++) {
                    u8 *d[1];

                    d[0] = LINE_PTR( dst, dst_surface->config.caps,
                                     y, dst_surface->config.size.h, dpitch ) +
                           DFB_BYTES_PER_LINE( dst_surface->config.format, x );

                    write_argb_span( src, d, drect->w, x, y, dst_surface );

                    src += sw;
               }
               break;
     }
}

static int bilinear_make_fast_weights( PixopsFilter *filter,
                                       const float x_scale, const float y_scale )
{
     int i_offset, j_offset;
     float *x_weights, *y_weights;
     int n_x, n_y;

     if (x_scale > 1.0) {      /* Bilinear */
          n_x = 2;
          filter->x_offset = 0.5 * (1.0 / x_scale - 1);
     }
     else {                    /* Tile */
          n_x = D_ICEIL (1.0 + 1.0 / x_scale);
          filter->x_offset = 0.0;
     }

     if (y_scale > 1.0) {      /* Bilinear */
          n_y = 2;
          filter->y_offset = 0.5 * (1.0 / y_scale - 1);
     }
     else {                    /* Tile */
          n_y = D_ICEIL (1.0 + 1.0 / y_scale);
          filter->y_offset = 0.0;
     }

     if (n_x > 64)
          n_x = 64;

     if (n_y > 64)
          n_y = 64;

     filter->n_y = n_y;
     filter->n_x = n_x;
     filter->weights = (int *) D_MALLOC( SUBSAMPLE * SUBSAMPLE * n_x * n_y *
                                         sizeof (int) );
     if (!filter->weights) {
          D_WARN ("couldn't allocate memory for scaling");
          return 0;
     }

     x_weights = (float *) alloca (n_x * sizeof (float));
     y_weights = (float *) alloca (n_y * sizeof (float));

     if (!x_weights || !y_weights) {
          D_FREE( filter->weights );

          D_WARN ("couldn't allocate memory for scaling");
          return 0;
     }

     for (i_offset = 0; i_offset < SUBSAMPLE; i_offset++)
          for (j_offset = 0; j_offset < SUBSAMPLE; j_offset++) {
               int *pixel_weights = filter->weights
                                    + ((i_offset * SUBSAMPLE) + j_offset)
                                    * n_x * n_y;

               float x = (float)j_offset / 16;
               float y = (float)i_offset / 16;
               int i, j;

               if (x_scale > 1.0) {     /* Bilinear */
                    for (i = 0; i < n_x; i++) {
                         x_weights[i] = ((i == 0) ? (1 - x) : x) / x_scale;
                    }
               }
               else {                   /* Tile */
                    for (i = 0; i < n_x; i++) {
                         if (i < x) {
                              if (i + 1 > x)
                                   x_weights[i] = MIN( i + 1, x + 1.0 / x_scale ) -x;
                              else
                                   x_weights[i] = 0;
                         }
                         else {
                              if (x + 1/x_scale > i)
                                   x_weights[i] = MIN( i + 1, x + 1.0 / x_scale ) -i;
                              else
                                   x_weights[i] = 0;
                         }
                    }
               }

               if (y_scale > 1.0) {     /* Bilinear */
                    for (i = 0; i < n_y; i++) {
                         y_weights[i] = ((i == 0) ? (1 - y) : y) / y_scale;
                    }
               }
               else {                   /* Tile */
                    for (i = 0; i < n_y; i++) {
                         if (i < y) {
                              if (i + 1 > y)
                                   y_weights[i] = MIN( i + 1, y + 1.0 / y_scale ) -y;
                              else
                                   y_weights[i] = 0;
                         }
                         else {
                              if (y + 1/y_scale > i)
                                   y_weights[i] = MIN( i + 1, y + 1.0 / y_scale ) -i;
                              else
                                   y_weights[i] = 0;
                         }
                    }
               }

               for (i = 0; i < n_y; i++) {
                    for (j = 0; j < n_x; j++) {
                         pixel_weights[n_x * i + j] =
                              65536.0 * x_weights[j] * x_scale
                              * y_weights[i] * y_scale;
                    }
               }
          }

     return 1;
}

static void scale_pixel( const int *weights, int n_x, int n_y,
                         u32 *dst, const u32 **src, int x, int sw )
{
     u32 r = 0, g = 0, b = 0, a = 0;
     int i, j;

     for (i = 0; i < n_y; i++) {
          const int *pixel_weights = weights + n_x * i;

          for (j = 0; j < n_x; j++) {
               const u32 *q;

               if (x + j < 0)
                    q = src[i];
               else if (x + j < sw)
                    q = src[i] + x + j;
               else
                    q = src[i] + sw - 1;

               {
                    const u32 ta = ((*q & 0xFF000000) >> 24) * pixel_weights[j];

                    b += ta * (((*q & 0xFF)) + 1);
                    g += ta * (((*q & 0xFF00) >> 8) + 1);
                    r += ta * (((*q & 0xFF0000) >> 16) + 1);
                    a += ta;
               }
          }
     }

     r = (r >> 24) == 0xFF ? 0xFF : (r + 0x800000) >> 24;
     g = (g >> 24) == 0xFF ? 0xFF : (g + 0x800000) >> 24;
     b = (b >> 24) == 0xFF ? 0xFF : (b + 0x800000) >> 24;
     a = (a >> 16) == 0xFF ? 0xFF : (a + 0x8000) >> 16;

     *dst = (a << 24) | (r << 16) | (g << 8) | b;
}

static u32* scale_line( const int *weights, int n_x, int n_y,
                        u32 *dst, u32 *dst_end,
                        const u32 **src, int x, int x_step, int sw )
{

     D_UNUSED_P( sw );

     while (dst < dst_end) {
          const int  x_scaled      = x >> SCALE_SHIFT;
          const int *pixel_weights = weights + ((x >> (SCALE_SHIFT -
                                                       SUBSAMPLE_BITS))
                                                & SUBSAMPLE_MASK) * n_x * n_y;
          u32 r = 0, g = 0, b = 0, a = 0;
          int i, j;

          for (i = 0; i < n_y; i++) {
               const int *line_weights = pixel_weights + n_x * i;
               const u32 *q            = src[i] + x_scaled;

               for (j = 0; j < n_x; j++) {
                    const u32 ta = ((*q & 0xFF000000) >> 24) * line_weights[j];

                    b += ta * (((*q & 0xFF)) + 1);
                    g += ta * (((*q & 0xFF00) >> 8) + 1);
                    r += ta * (((*q & 0xFF0000) >> 16) + 1);
                    a += ta;

                    q++;
               }
          }

          r = (r >> 24) == 0xFF ? 0xFF : (r + 0x800000) >> 24;
          g = (g >> 24) == 0xFF ? 0xFF : (g + 0x800000) >> 24;
          b = (b >> 24) == 0xFF ? 0xFF : (b + 0x800000) >> 24;
          a = (a >> 16) == 0xFF ? 0xFF : (a + 0x8000) >> 16;

          *dst++ = (a << 24) | (r << 16) | (g << 8) | b;

          x += x_step;
     }

     return dst;
}

void dfb_scale_linear_32( u32 *src, int sw, int sh,
                          void  *dst, int dpitch, DFBRectangle *drect,
                          CoreSurface *dst_surface, const DFBRegion *dst_clip )
{
     DFBRectangle srect = { 0, 0, sw, sh };
     float scale_x, scale_y;
     int i, j;
     int sx, sy;
     int x_step, y_step;
     int scaled_x_offset;
     PixopsFilter filter;
     u8   *dst1 = NULL, *dst2 = NULL;
     u32  *buf;

     if (drect->w == sw && drect->h == sh) {
          dfb_copy_buffer_32( src, dst, dpitch, drect, dst_surface, dst_clip );
          return;
     }

     if (dst_clip)
          dfb_clip_stretchblit( dst_clip, &srect, drect );

     if (srect.w < 1 || srect.h < 1 || drect->w < 1 || drect->h < 1)
          return;

     src += srect.y * sw + srect.x;

     scale_x = (float)drect->w / srect.w;
     scale_y = (float)drect->h / srect.h;

     x_step = (1 << SCALE_SHIFT) / scale_x;
     y_step = (1 << SCALE_SHIFT) / scale_y;

     if (! bilinear_make_fast_weights( &filter, scale_x, scale_y ))
          return;

     scaled_x_offset = D_IFLOOR( filter.x_offset * (1 << SCALE_SHIFT) );
     sy = D_IFLOOR( filter.y_offset * (1 << SCALE_SHIFT) );

     switch (dst_surface->config.format) {
          case DSPF_I420:
               dst1 = (u8*)dst  + dpitch   * dst_surface->config.size.h;
               dst2 = dst1 + dpitch/2 * dst_surface->config.size.h/2;
               break;
          case DSPF_YV12:
               dst2 = (u8*)dst  + dpitch   * dst_surface->config.size.h;
               dst1 = dst2 + dpitch/2 * dst_surface->config.size.h/2;
               break;
          case DSPF_YV16:
               dst2 = (u8*)dst  + dpitch   * dst_surface->config.size.h;
               dst1 = dst2 + dpitch/2 * dst_surface->config.size.h;
               break;
          case DSPF_NV12:
          case DSPF_NV21:
          case DSPF_NV16:
               dst1 = (u8*)dst + dpitch * dst_surface->config.size.h;
               break;
          case DSPF_YUV444P:
               dst1 = (u8*)dst  + dpitch * dst_surface->config.size.h;
               dst2 = dst1 + dpitch * dst_surface->config.size.h;
               break;
          default:
               break;
     }

     buf = (u32*) alloca( drect->w * 4 );

     for (i = drect->y; i < drect->y + drect->h; i++) {
          int         x_start;
          int         y_start;
          const int  *run_weights;
          u32        *outbuf     = buf;
          u32        *outbuf_end = buf + drect->w;
          u32        *new_outbuf;
          const u32 **line_bufs;
          u8         *d[3];

          y_start = sy >> SCALE_SHIFT;

          run_weights = filter.weights + ((sy >> (SCALE_SHIFT - SUBSAMPLE_BITS))
                                          & SUBSAMPLE_MASK) * filter.n_x * filter.n_y * SUBSAMPLE;

          line_bufs = (const u32 **) alloca( filter.n_y * sizeof (void *) );

          for (j = 0; j < filter.n_y; j++) {
               if (y_start <  0)
                    line_bufs[j] = src;
               else if (y_start < sh)
                    line_bufs[j] = src + sw * y_start;
               else
                    line_bufs[j] = src + sw * (sh - 1);

               y_start++;
          }

          sx = scaled_x_offset;
          x_start = sx >> SCALE_SHIFT;

          while (x_start < 0 && outbuf < outbuf_end) {
               scale_pixel( run_weights + ((sx >> (SCALE_SHIFT - SUBSAMPLE_BITS))
                                            & SUBSAMPLE_MASK) * (filter.n_x * filter.n_y),
                            filter.n_x, filter.n_y,
                            outbuf, line_bufs, sx >> SCALE_SHIFT, sw );
               sx += x_step;
               x_start = sx >> SCALE_SHIFT;
               outbuf++;
          }

          new_outbuf = scale_line( run_weights, filter.n_x, filter.n_y,
                                   outbuf, outbuf_end, line_bufs,
                                   sx >> SCALE_SHIFT, x_step, sw );
          sx = ((outbuf_end - outbuf) >> 2) * x_step + scaled_x_offset;
          outbuf = new_outbuf;

          while (outbuf < outbuf_end) {
               scale_pixel( run_weights + ((sx >> (SCALE_SHIFT - SUBSAMPLE_BITS))
                                            & SUBSAMPLE_MASK) * (filter.n_x * filter.n_y),
                            filter.n_x, filter.n_y,
                            outbuf, line_bufs, sx >> SCALE_SHIFT, sw );
               sx += x_step;
               outbuf++;
          }

          sy += y_step;

          d[0] = LINE_PTR( dst, dst_surface->config.caps,
                           i, dst_surface->config.size.h, dpitch ) +
                 DFB_BYTES_PER_LINE( dst_surface->config.format, drect->x );

          switch (dst_surface->config.format) {
               case DSPF_I420:
               case DSPF_YV12:
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, i/2,
                                     dst_surface->config.size.h/2, dpitch/2 ) + drect->x/2;
                    d[2] = LINE_PTR( dst2, dst_surface->config.caps, i/2,
                                     dst_surface->config.size.h/2, dpitch/2 ) + drect->x/2;
                    break;
               case DSPF_YV16:
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, i,
                                     dst_surface->config.size.h, dpitch/2 ) + drect->x/2;
                    d[2] = LINE_PTR( dst2, dst_surface->config.caps, i,
                                     dst_surface->config.size.h, dpitch/2 ) + drect->x/2;
                    break;
               case DSPF_NV12:
               case DSPF_NV21:
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, i/2,
                                     dst_surface->config.size.h/2, dpitch ) + (drect->x&~1);
                    break;
               case DSPF_NV16:
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, i,
                                     dst_surface->config.size.h, dpitch ) + (drect->x&~1);
                    break;
               case DSPF_YUV444P:
                    d[1] = LINE_PTR( dst1, dst_surface->config.caps, i,
                                     dst_surface->config.size.h, dpitch ) + drect->x;
                    d[2] = LINE_PTR( dst2, dst_surface->config.caps, i,
                                     dst_surface->config.size.h, dpitch ) + drect->x;
                    break;
               default:
                    break;
          }

          write_argb_span( buf, d, drect->w, drect->x, i, dst_surface );
     }

     D_FREE(filter.weights);
}

