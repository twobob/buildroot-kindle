/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   directfb-csource is based on gdk-pixbuf-csource, a GdkPixbuf
   based image CSource generator  Copyright (C) 1999, 2001 Tim Janik

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
#include <errno.h>
#include <assert.h>

#include <sys/types.h>
#include <sys/stat.h>

#include <png.h>

#include <directfb.h>

#include <direct/types.h>
#include <direct/util.h>

#include <gfx/convert.h>

#define DFB_DITHER565 DFB_DITHER_ADVANCED
#include <misc/dither565.h>


static struct {
     DFBSurfacePixelFormat  format;
     const char             *name;
} pixelformats[] = {
     { DSPF_ARGB,     "ARGB"     },
     { DSPF_ARGB1555, "ARGB1555" },
     { DSPF_RGBA5551, "RGBA5551" },
     { DSPF_ARGB2554, "ARGB2554" },
     { DSPF_ARGB4444, "ARGB4444" },
     { DSPF_ARGB8565, "ARGB8565" },
     { DSPF_RGB32,    "RGB32"    },
     { DSPF_RGB24,    "RGB24"    },
     { DSPF_RGB16,    "RGB16"    },
     { DSPF_RGB332,   "RGB332"   },
     { DSPF_A8,       "A8"       },
     { DSPF_LUT8,     "LUT8"     }
};
static int n_pixelformats = D_ARRAY_SIZE( pixelformats );


static void       print_usage     (const char             *prg_name);
static DFBResult  load_image      (const char             *filename,
                                   DFBSurfaceDescription  *desc,
                                   DFBColor               *palette,
                                   int                    *palette_size,
                                   const DFBColor         *transparent,
                                   DFBSurfacePixelFormat   rgbformat,
                                   bool                    dither565);
static void       dither_rgb16    (const u32              *src,
                                   u16                    *dest,
                                   int                     y,
                                   int                     width);
static DFBResult  merge_images    (DFBSurfaceDescription  *images,
                                   int                     num_images,
                                   DFBSurfaceDescription  *dest,
                                   DFBRectangle           *rectangles);
static DFBResult  dump_raw_data   (const char             *name,
                                   const unsigned char    *data,
                                   unsigned int            len);
static DFBResult  dump_image      (const char             *name,
                                   DFBSurfaceDescription  *desc,
                                   DFBColor               *palette,
                                   int                     palette_size);
static DFBResult  dump_rectangles (const char             *name,
                                   DFBRectangle           *rectangles,
                                   const char            **names,
                                   int                     num_rects);
static char *     variable_name   (const char             *name);
static char *     base_name       (const char             *name);


int main (int         argc,
          const char *argv[])
{
     DFBSurfaceDescription desc      = { .flags = 0 };
     DFBSurfacePixelFormat format    = DSPF_UNKNOWN;
     DFBSurfacePixelFormat rgbformat = DSPF_UNKNOWN;
     DFBColor  palette[256];

     const char *filename[argc];
     const char *name           = NULL;
     int         palette_size   = 0;
     int         num_images     = 0;
     int         i, n;
     bool        rawmode        = 0;
     bool        dither565      = 0;
     DFBColor   *transparent    = NULL;

     /*  parse command line  */

     for (n = 1; n < argc; n++) {
          if (strncmp (argv[n], "--", 2) == 0) {

               const char *arg = argv[n] + 2;

               if (strcmp (arg, "help") == 0) {
                    print_usage (argv[0]);
                    return EXIT_SUCCESS;
               }
               if (strcmp (arg, "version") == 0) {
                    fprintf (stderr, "directfb-csource version %s\n",
                             DIRECTFB_VERSION);
                    return EXIT_SUCCESS;
               }
               if (strcmp (arg, "raw") == 0) {
                    rawmode = 1;
                    continue;
               }
               if (strcmp (arg, "dither-rgb16") == 0) {
                    dither565 = 1;
                    continue;
               }
               if (strncmp (arg, "format=", 7) == 0 && !format) {
                    for (i = 0; i < n_pixelformats && !format; i++)
                         if (!strcasecmp (pixelformats[i].name, arg + 7))
                              format = pixelformats[i].format;
                    if (format)
                         continue;
               }
               if (strncmp (arg, "rgbformat=", 10) == 0 && !rgbformat) {
                    for (i = 0; i < n_pixelformats && !rgbformat; i++)
                         if (!strcasecmp (pixelformats[i].name, arg + 10))
                              rgbformat = pixelformats[i].format;
                    if (rgbformat)
                         continue;
               }
               if (strncmp (arg, "name=", 5) == 0 && !name) {
                    name = arg + 5;
                    if (*name)
                         continue;
               }
               if (strncmp (arg, "transparent=", 12) == 0 && !transparent) {
                    char *error;
                    u32   argb = strtoul (arg + 12, &error, 16);

                    if (! error) {
                         transparent = alloca(sizeof (DFBColor));

                         transparent->b = argb & 0xFF;
                         argb >>= 8;
                         transparent->g = argb & 0xFF;
                         argb >>= 8;
                         transparent->r = argb & 0xFF;
                         argb >>= 8;
                         transparent->a = argb & 0xFF;
                    }
                    if (transparent)
                         continue;
               }
          }

          filename[num_images++] = argv[n];
     }

     /*  check parameters  */

     if (! num_images) {
          print_usage (argv[0]);
          return EXIT_FAILURE;
     }

     if (num_images > 1 && rawmode) {
          fprintf (stderr,
                   "Multiple input files not allowed in raw mode.\n");
          return EXIT_FAILURE;
     }

     if (num_images > 1 && !name) {
          fprintf (stderr,
                   "You must specify a variable name when using multiple images.\n");
          return EXIT_FAILURE;
     }

     /*  load the first image  */

     if (rawmode) {

          struct stat statbuf; 
          if (0 == stat(filename[0], &statbuf))
          {
               FILE *f;
               unsigned char *data = alloca(statbuf.st_size);
               memset(data, 0, statbuf.st_size);

               f = fopen(filename[0], "r");
               if (f)
               {
                    fread(data, statbuf.st_size, 1, f);
                    fclose(f);
               }

               return dump_raw_data(name ? : strrchr (filename[0], '/') ? : filename[0],
                                    data, statbuf.st_size);
          }
          
     }
     else {
          if (format) {
               desc.flags = DSDESC_PIXELFORMAT;
               desc.pixelformat = format;
          }

          if (load_image (filename[0],
                          &desc, palette, &palette_size, transparent,
                          rgbformat, dither565) != DFB_OK)
               return EXIT_FAILURE;

          /*  dump it and quit if this is the only image on the command line  */
          
          if (num_images == 1)
               return dump_image (name ? : strrchr (filename[0], '/') ? : filename[0],
                                  &desc, palette, palette_size);
     }

     /*  merge multiple images into one surface  */
     {
          DFBSurfaceDescription  image[num_images];
          DFBRectangle           rect[num_images];
          DFBColor               foo[256];
          int                    foo_size;

          image[0] = desc;

          for (i = 1; i < num_images; i++) {
               image[i].flags = DSDESC_PIXELFORMAT;
               image[i].pixelformat = desc.pixelformat;

               if (load_image (filename[i],
                               image + i, foo, &foo_size, transparent,
                               rgbformat, dither565) != DFB_OK)
                    return EXIT_FAILURE;
          }

          if (merge_images (image, num_images, &desc, rect) != DFB_OK)
               return EXIT_FAILURE;

          /*  dump the rectangles, then the surface  */

          if (dump_rectangles (name, rect, filename, num_images) != DFB_OK)
               return EXIT_FAILURE;

          return dump_image (name, &desc, palette, palette_size);
     }
}

static void print_usage (const char *prg_name)
{
     fprintf (stderr, "directfb-csource version %s\n\n", DIRECTFB_VERSION);
     fprintf (stderr, "Usage: %s [options] <imagefile>\n", prg_name);
     fprintf (stderr, "   --name=<identifer>      specifies variable name\n");
     fprintf (stderr, "   --format=<identifer>    specifies surface format\n");
     fprintf (stderr, "   --rgbformat=<identifer> specifies format for non-alpha images\n");
     fprintf (stderr, "   --multi                 multiple images\n");
     fprintf (stderr, "   --raw                   dump a single file directly to header\n");
     fprintf (stderr, "   --dither-rgb16          dither images rendered to RGB16 surfaces\n");
     fprintf (stderr, "   --help                  show this help message\n");
     fprintf (stderr, "   --version               print version information\n");
     fprintf (stderr, "\n");
     fprintf (stderr, "See the directfb-csource(1) man-page for more information.\n");
     fprintf (stderr, "\n");
}

static DFBResult load_image (const char            *filename,
                             DFBSurfaceDescription *desc,
                             DFBColor              *palette,
                             int                   *palette_size,
                             const DFBColor        *transparent,
                             DFBSurfacePixelFormat  rgbformat,
                             bool                   dither565)
{
     DFBSurfacePixelFormat dest_format;
     DFBSurfacePixelFormat src_format;
     FILE          *fp;
     png_structp    png_ptr  = NULL;
     png_infop      info_ptr = NULL;
     png_uint_32    width, height;
     unsigned char *data     = NULL;
     int            type;
     char           header[8];
     int            bytes, pitch;

     dest_format =
       (desc->flags & DSDESC_PIXELFORMAT) ? desc->pixelformat : DSPF_UNKNOWN;

     desc->flags = 0;
     desc->preallocated[0].data = NULL;

     if (!(fp = fopen (filename, "rb"))) {
          fprintf (stderr, "Failed to open file '%s': %s.\n",
                   filename, strerror (errno));
          goto cleanup;
     }

     bytes = fread (header, 1, sizeof(header), fp);
     if (png_sig_cmp ((unsigned char*) header, 0, bytes)) {
          fprintf (stderr, "File '%s' doesn't seem to be a PNG image file.\n",
                   filename);
          goto cleanup;
     }

     png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING,
                                       NULL, NULL, NULL);
     if (!png_ptr)
          goto cleanup;

     if (setjmp (png_jmpbuf(png_ptr))) {
          if (desc->preallocated[0].data) {
               free (desc->preallocated[0].data);
               desc->preallocated[0].data = NULL;
          }

          /* data might have been clobbered,
             set it to NULL and leak instead of crashing */
          data = NULL;

          goto cleanup;
     }

     info_ptr = png_create_info_struct (png_ptr);
     if (!info_ptr)
          goto cleanup;

     png_init_io (png_ptr, fp);
     png_set_sig_bytes (png_ptr, bytes);

     png_read_info (png_ptr, info_ptr);

     png_get_IHDR (png_ptr, info_ptr,
                   &width, &height, &bytes, &type, NULL, NULL, NULL);

     if (bytes == 16)
          png_set_strip_16 (png_ptr);

#ifdef WORDS_BIGENDIAN
     png_set_swap_alpha (png_ptr);
#else
     png_set_bgr (png_ptr);
#endif

     src_format = (type & PNG_COLOR_MASK_ALPHA) ? DSPF_ARGB : DSPF_RGB32;
     switch (type) {
          case PNG_COLOR_TYPE_GRAY:
               if (dest_format == DSPF_A8) {
                    src_format = DSPF_A8;
                    break;
               }
               /* fallthru */
          case PNG_COLOR_TYPE_GRAY_ALPHA:
               png_set_gray_to_rgb (png_ptr);
               if (rgbformat)
                    dest_format = rgbformat;
               break;

          case PNG_COLOR_TYPE_PALETTE:
               if (dest_format == DSPF_LUT8) {
                    src_format = DSPF_LUT8;
                    break;
               }
               png_set_palette_to_rgb (png_ptr);
               /* fallthru */
          case PNG_COLOR_TYPE_RGB:
               if (rgbformat)
                    dest_format = rgbformat;
          case PNG_COLOR_TYPE_RGB_ALPHA:
               if (dest_format == DSPF_RGB24) {
                    png_set_strip_alpha (png_ptr);
                    src_format = DSPF_RGB24;
               }
               break;
       }

     switch (src_format) {
          case DSPF_LUT8:
          {
               png_colorp *info_palette;
               int num_palette;

               png_get_PLTE(png_ptr,info_ptr,info_palette,&num_palette);

               if (num_palette) {
                    png_byte *alpha;
                    int       i, num;

                    *palette_size = MIN (num_palette, 256);
                    for (i = 0; i < *palette_size; i++) {
                         palette[i].a = 0xFF;
                         palette[i].r = info_palette[i]->red;
                         palette[i].g = info_palette[i]->green;
                         palette[i].b = info_palette[i]->blue;
                    }
                    if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS)) {
                         png_get_tRNS (png_ptr, info_ptr, &alpha, &num, NULL);
                         for (i = 0; i < MIN (num, *palette_size); i++)
                              palette[i].a = alpha[i];
                    }
               }
               break;
          }
          case DSPF_RGB32:
                png_set_filler (png_ptr, 0xFF,
#ifdef WORDS_BIGENDIAN
                                PNG_FILLER_BEFORE
#else
                                PNG_FILLER_AFTER
#endif
                                );
                break;
          case DSPF_ARGB:
          case DSPF_A8:
               if (png_get_valid (png_ptr, info_ptr, PNG_INFO_tRNS))
                    png_set_tRNS_to_alpha (png_ptr);
               break;
          default:
               break;
     }

     pitch = width * DFB_BYTES_PER_PIXEL (src_format);
     if (pitch & 3)
          pitch += 4 - (pitch & 3);

     data  = malloc (height * pitch);
     if (!data) {
          fprintf (stderr, "Failed to allocate %ld bytes.\n", height * pitch);
          goto cleanup;
     }

     {
          unsigned int i;
          png_bytep bptrs[height];

          for (i = 0; i < height; i++)
               bptrs[i] = data + i * pitch;

          png_read_image (png_ptr, bptrs);
     }

     if (!dest_format)
          dest_format = src_format;

     /*  replace color in completely transparent pixels  */
     if (transparent && DFB_PIXELFORMAT_HAS_ALPHA(src_format)) {
          unsigned char *row;
          int            h;

          for (row = data, h = height; h; h--, row += pitch) {
                 u32 *pixel;
                 int  w;

                 for (pixel = (u32 *) row, w = width; w; w--, pixel++) {
                      if ((*pixel & 0xff000000) == 0x0)
                           *pixel = dfb_color_to_argb (transparent);
                 }
          }
     }

     if (DFB_BYTES_PER_PIXEL(src_format) != DFB_BYTES_PER_PIXEL(dest_format)) {
          unsigned char *s, *d, *dest;
          int            d_pitch, h;

          assert (DFB_BYTES_PER_PIXEL (src_format) == 4);

          d_pitch = width * DFB_BYTES_PER_PIXEL (dest_format);
          if (d_pitch & 3)
               d_pitch += 4 - (d_pitch & 3);

          dest = malloc (height * d_pitch);
          if (!dest) {
               fprintf (stderr, "Failed to allocate %ld bytes.\n",
                        height * d_pitch);
               goto cleanup;
          }

          h = height;
          switch (dest_format) {
               case DSPF_RGB16:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         if (dither565)
                              dither_rgb16 ((u32 *) s, (u16 *) d, height - h, width);
                         else
                              dfb_argb_to_rgb16 ((u32 *) s, (u16 *) d, width);
                    break;
               case DSPF_ARGB1555:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         dfb_argb_to_argb1555 ((u32 *) s, (u16 *) d, width);
                    break;
               case DSPF_RGBA5551:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         dfb_argb_to_rgba5551 ((u32 *) s, (u16 *) d, width);
                    break;
               case DSPF_ARGB2554:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         dfb_argb_to_argb2554 ((u32 *) s, (u16 *) d, width);
                    break;
               case DSPF_ARGB4444:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         dfb_argb_to_argb4444 ((u32 *) s, (u16 *) d, width);
                    break;
               case DSPF_ARGB8565:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         dfb_argb_to_argb8565 ((u32 *) s, (u8 *) d, width);
                    break;
               case DSPF_RGB332:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         dfb_argb_to_rgb332 ((u32 *) s, (u8 *) d, width);
                    break;
               case DSPF_A8:
                    for (s = data, d = dest; h; h--, s += pitch, d += d_pitch)
                         dfb_argb_to_a8 ((u32 *) s, (u8 *) d, width);
                    break;
               default:
                    fprintf (stderr,
                             "Sorry, unsupported format conversion.\n");
                    goto cleanup;
          }

          free (data);
          data = dest;
          pitch = d_pitch;
     }

     desc->flags = (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT |
                    DSDESC_PREALLOCATED);
     desc->width       = width;
     desc->height      = height;
     desc->pixelformat = dest_format;
     desc->preallocated[0].pitch = pitch;
     desc->preallocated[0].data  = data;

     data = NULL;

 cleanup:
     if (fp)
          fclose (fp);

     if (png_ptr)
          png_destroy_read_struct (&png_ptr, &info_ptr, NULL);

     if (data)
          free (data);

     return ((desc->flags) ? DFB_OK : DFB_FAILURE);
}

static DFBResult merge_images (DFBSurfaceDescription *images,
                               int                    num_images,
                               DFBSurfaceDescription *dest,
                               DFBRectangle          *rectangles)
{
  DFBSurfaceDescription *image = images;
  DFBRectangle          *rect  = rectangles;
  unsigned char         *data;
  int                    bpp;
  int                    pitch, i;

  rect->x = 0;
  rect->y = 0;
  rect->w = image->width;
  rect->h = image->height;

  dest->flags       = (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT |
                       DSDESC_PREALLOCATED);
  dest->pixelformat = image->pixelformat;

  bpp = DFB_BYTES_PER_PIXEL (dest->pixelformat);

  if (bpp == 1)
       dest->width  = (rect->w + 3) & ~3;
  else
       dest->width  = rect->w;

  dest->height      = rect->h;

  for (i = 1; i < num_images; i++) {
       image++;
       rect++;

       if (image->pixelformat != dest->pixelformat)
            return DFB_INVARG;

       rect->x = dest->width;
       rect->y = 0;
       rect->w = image->width;
       rect->h = image->height;

       if (bpp == 1)
            dest->width += (rect->w + 3) & ~3;
       else
            dest->width += rect->w;

       if (dest->height < rect->h)
            dest->height = rect->h;
  }

  pitch = (dest->width * bpp + 3) &~ 3;
  data  = malloc (dest->height * pitch);
  if (!data) {
       fprintf (stderr, "Failed to allocate %ld bytes.\n",
                (long) dest->height * pitch);
       return DFB_FAILURE;
  }


  for (i = 0, image = images, rect = rectangles;
       i < num_images;
       i++, image++, rect++) {

       unsigned char *dest   = data + rect->x * bpp;
       unsigned char *src    = image->preallocated[0].data;
       int            height = rect->h;

       do {
            memcpy (dest, src, rect->w * bpp);
            src  += image->preallocated[0].pitch;
            dest += pitch;
       }
       while (--height);
  }

  dest->preallocated[0].pitch = pitch;
  dest->preallocated[0].data  = data;

  return DFB_OK;
}

static void
dither_rgb16 (const u32 *src, u16 *dest, int y, int width)
{
     const u32 *dm = DM_565 + ((y & (DM_565_HEIGHT - 1)) << DM_565_WIDTH_SHIFT);
     int        x;

     for (x = 0; x < width; x++) {
          u32 rgb = ((src[x] & 0xFF)          |
                     (src[x] & 0xFF00)   << 2 |
                     (src[x] & 0xFF0000) << 4);

          rgb += dm[x & (DM_565_WIDTH - 1)];
          rgb += (0x10040100
                  - ((rgb & 0x1e0001e0) >> 5)
                  - ((rgb & 0x00070000) >> 6));

          dest[x] = (((rgb & 0x0f800000) >> 12) |
                     ((rgb & 0x0003f000) >> 7)  |
                     ((rgb & 0x000000f8) >> 3));
     }
}


typedef struct {
     FILE  *fp;
     int     pos;
     bool    pad;
} CSourceData;

static inline void save_uchar (CSourceData   *csource,
                               unsigned char  d)
{
     if (csource->pos > 70) {
          fprintf (csource->fp, "\"\n  \"");

          csource->pos = 3;
          csource->pad = false;
     }
     if (d < 33 || d > 126) {
          fprintf (csource->fp, "\\%o", d);
          csource->pos += 1 + 1 + (d > 7) + (d > 63);
          csource->pad = d < 64;
          return;
     }
     if (d == '\\') {
          fprintf (csource->fp, "\\\\");
          csource->pos += 2;
     }
     else if (d == '"') {
          fprintf (csource->fp, "\\\"");
          csource->pos += 2;
     }
     else if (csource->pad && d >= '0' && d <= '9') {
          fprintf (csource->fp, "\"\"%c", d);
          csource->pos += 3;
     }
     else {
          fputc (d, csource->fp);
          csource->pos += 1;
     }
     csource->pad = false;

     return;
}

static void dump_data(CSourceData         *csource,
                      const char          *name,
                      const unsigned char *data,
                      unsigned int         len)
{
     fprintf (csource->fp,
              "static const unsigned char %s_data[] =\n", name);
     fprintf (csource->fp, "  \"");
     
     csource->pos = 3;
     do
          save_uchar (csource, *data++);
     while (--len);

     fprintf (csource->fp, "\";\n\n");
}

static DFBResult dump_raw_data(const char          *name,
                               const unsigned char *data,
                               unsigned int         len)
{
     CSourceData  csource = { stdout, 0, 0 };
     char        *vname   = variable_name (name);

     if (!data || !len)
          return DFB_INVARG;

     fprintf (csource.fp,
              "/* DirectFB raw data dump created by directfb-csource %s */\n\n",
              DIRECTFB_VERSION);

     dump_data(&csource, vname, data, len);

     free (vname);

     return DFB_OK;
}


static DFBResult dump_image (const char            *name,
                             DFBSurfaceDescription *desc,
                             DFBColor              *palette,
                             int                    palette_size)
{
     CSourceData    csource = { stdout, 0, 0 };
     const char    *format  = NULL;
     char          *vname   = variable_name (name);
     unsigned char *data;
     unsigned long  len;
     int            i;

     if (desc &&
         desc->flags != (DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT |
                         DSDESC_PREALLOCATED))
          return DFB_INVARG;

     for (i = 0; i < n_pixelformats && !format; i++)
          if (pixelformats[i].format == desc->pixelformat)
               format = pixelformats[i].name;

     if (!format)
          return DFB_INVARG;

     data = (unsigned char *) desc->preallocated[0].data;
     len = desc->height * desc->preallocated[0].pitch;

     if (!len)
          return DFB_INVARG;

     /* dump comment */
     fprintf (csource.fp,
              "/* DirectFB surface dump created by directfb-csource %s */\n\n",
              DIRECTFB_VERSION);

     /* dump data */
     dump_data(&csource, vname, data, len);

     /* dump palette */
     if (palette_size > 0) {
          fprintf (csource.fp,
                   "static const DFBColor %s_palette[%d] = {\n", vname, palette_size);
          for (i = 0; i < palette_size; i++)
               fprintf (csource.fp,
                        "  { 0x%02x, 0x%02x, 0x%02x, 0x%02x }%c\n",
                        palette[i].a, palette[i].r, palette[i].g, palette[i].b,
                        i+1 < palette_size ? ',' : ' ');
          fprintf (csource.fp, "};\n\n");
     }

     /* dump description */
     fprintf (csource.fp,
              "static const DFBSurfaceDescription %s_desc = {\n", vname);
     fprintf (csource.fp,
              "  flags                   : DSDESC_WIDTH | DSDESC_HEIGHT | DSDESC_PIXELFORMAT |\n"
              "                            DSDESC_PREALLOCATED");
     if (palette_size > 0)
          fprintf (csource.fp, " | DSDESC_PALETTE");
     fprintf (csource.fp, ",\n");
     fprintf (csource.fp,
              "  width                   : %d,\n", desc->width);
     fprintf (csource.fp,
              "  height                  : %d,\n", desc->height);
     fprintf (csource.fp,
              "  pixelformat             : DSPF_%s,\n", format);
     fprintf (csource.fp,
              "  preallocated : {{  data : (void *) %s_data,\n", vname);
     fprintf (csource.fp,
              "                    pitch : %d  }}", desc->preallocated[0].pitch);
     if (palette_size > 0) {
          fprintf (csource.fp, ",\n");
          fprintf (csource.fp,
                   "  palette :    {  entries : %s_palette,\n", vname);
          fprintf (csource.fp,
                   "                     size : %d  }", palette_size);
     }
     fprintf (csource.fp, "\n};\n\n");

     free (vname);

     return DFB_OK;
}

static DFBResult dump_rectangles (const char    *name,
                                  DFBRectangle  *rectangles,
                                  const char   **names,
                                  int            num_rects)
{
     DFBRectangle *rect;
     const char   *blanks = "                                  ";
     char         *vname  = variable_name (name);
     FILE         *fp     = stdout;
     int           len, i;

     if (num_rects < 1)
          return DFB_INVARG;

     fprintf (fp,
              "/* DirectFB multi-surface dump created by directfb-csource %s */\n\n",
              DIRECTFB_VERSION);

     fprintf (fp,
              "static const struct {\n"
              "  const char   *name;\n"
              "  DFBRectangle  rect;\n"
              "} %s[] = {\n", vname);

     for (i = 0, len = 0; i < num_rects; i++)
          len = MAX (len, strlen (names[i]));

     len = len + 4 - strlen (blanks);

     for (i = 0, rect = rectangles; i < num_rects; i++, rect++) {

          char *v = base_name (names[i]);

          if (i)
               fprintf (fp, ",\n");

          if (len < 0) {
               int l = fprintf (fp, "  { \"%s\", ", v);

               fprintf (fp, "%s", blanks - len + l);
               fprintf (fp, "{ x : %4d, y : %4d, w : %4d, h : %4d } }",
                        rect->x, rect->y, rect->w, rect->h);
          }
          else {
               fprintf (fp,
                        "  { \"%s\",\n"
                        "    { x : %4d, y : %4d, w : %4d, h : %4d } }",
                        v, rect->x, rect->y, rect->w, rect->h);
          }

          free (v);
     }
     fprintf (fp, "\n};\n\n");

     free (vname);

     return DFB_OK;
}

static char *
variable_name (const char *name)
{
     char *vname = strdup (name);
     char *v     = vname;

     while (DFB_TRUE) {
          switch (*v) {
               case 0:
                    return vname;
               case 'a'...'z':
               case 'A'...'Z':
               case '0'...'9':
               case '_':
                    break;
               default:
                    *v = '_';
          }
          v++;
     }
}

static char *
base_name (const char *name)
{
     char *vname = strdup (name);
     char *v     = vname;

     while (DFB_TRUE) {
          switch (*v) {
               case '.':
                    *v = 0;
               case 0:
                    return vname;
               default:
                    break;
          }
          v++;
     }
}
