/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <config.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <directfb.h>
#include <directfb_strings.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/direct.h>

#include <gfx/convert.h>

#include <dgiff.h>

#undef SIZEOF_LONG
#include <ft2build.h>
#include FT_GLYPH_H

#define MAX_SIZE_COUNT    256
#define MAX_ROW_WIDTH    2047

D_DEBUG_DOMAIN( mkdgiff, "mkdgiff", "DirectFB Glyph Image File Format Tool" );

static DirectFBPixelFormatNames( format_names );

static const char            *filename;
static int                    face_index;
static DFBSurfacePixelFormat  format = DSPF_A8;

static int                    size_count;
static int                    face_sizes[MAX_SIZE_COUNT];

static bool                   premult;

/**********************************************************************************************************************/

static void
print_usage (const char *prg_name)
{
     int i = 0;

     fprintf (stderr, "\nDirectFB Glyph Image File Format Tool (version %s)\n\n", DIRECTFB_VERSION);
     fprintf (stderr, "Usage: %s [options]\n\n", prg_name);
     fprintf (stderr, "Options:\n");
     fprintf (stderr, "   -f, --format    <pixelformat>   Choose the pixel format (default A8)\n");
     fprintf (stderr, "   -s, --sizes     <s1>[,s2...]    Choose sizes to generate glyph images for\n");
     fprintf (stderr, "   -p, --premult                   Use premultiplied alpha\n");
     fprintf (stderr, "   -h, --help                      Show this help message\n");
     fprintf (stderr, "   -v, --version                   Print version information\n");
     fprintf (stderr, "\n");

     fprintf (stderr, "Known pixel formats with alpha:\n");

     while (format_names[i].format != DSPF_UNKNOWN) {
          DFBSurfacePixelFormat format = format_names[i].format;

          if (DFB_PIXELFORMAT_HAS_ALPHA(format)) {
               fprintf (stderr, "   %-10s %2d bits, %d bytes",
                        format_names[i].name, DFB_BITS_PER_PIXEL(format),
                        DFB_BYTES_PER_PIXEL(format));

               if (DFB_PIXELFORMAT_IS_INDEXED(format))
                    fprintf (stderr, "   INDEXED");

               if (DFB_PLANAR_PIXELFORMAT(format)) {
                    int planes = DFB_PLANE_MULTIPLY(format, 1000);

                    fprintf (stderr, "   PLANAR (x%d.%03d)",
                             planes / 1000, planes % 1000);
               }

               fprintf (stderr, "\n");
          }

          ++i;
     }
     fprintf (stderr, "\n");
}

static DFBBoolean
parse_format( const char *arg, DFBSurfacePixelFormat *ret_format )
{
     int i = 0;

     while (format_names[i].format != DSPF_UNKNOWN) {
          if (!strcasecmp( arg, format_names[i].name )) {
               *ret_format = format_names[i].format;
               return DFB_TRUE;
          }

          ++i;
     }

     fprintf (stderr, "\nInvalid format specified!\n\n" );

     return DFB_FALSE;
}

static DFBBoolean
parse_sizes( const char *arg )
{
     int i    = 0;
     int size = 0;

     for (i=0; arg[i]; i++) {
          switch (arg[i]) {
               case '0' ... '9':
                    if (size_count == MAX_SIZE_COUNT) {
                         fprintf (stderr, "\nMaximum number of sizes (%d) exceeded!\n\n", MAX_SIZE_COUNT );
                         return DFB_FALSE;
                    }
                    size = size * 10 + arg[i] - '0';
                    break;

               case ',':
                    if (size) {
                         face_sizes[size_count++] = size;
                         size = 0;
                    }
                    break;

               default:
                    fprintf (stderr, "\nInvalid character used in sizes argument!\n\n" );
                    return DFB_FALSE;
          }
     }

     if (size)
          face_sizes[size_count++] = size;

     return DFB_TRUE;
}

static DFBBoolean
parse_command_line( int argc, char *argv[] )
{
     int n;

     for (n = 1; n < argc; n++) {
          const char *arg = argv[n];

          if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0) {
               print_usage (argv[0]);
               return DFB_FALSE;
          }

          if (strcmp (arg, "-v") == 0 || strcmp (arg, "--version") == 0) {
               fprintf (stderr, "mkdgiff version %s\n", DIRECTFB_VERSION);
               return DFB_FALSE;
          }

          if (strcmp (arg, "-f") == 0 || strcmp (arg, "--format") == 0) {
               if (++n == argc) {
                    print_usage (argv[0]);
                    return DFB_FALSE;
               }

               if (!parse_format( argv[n], &format ))
                    return DFB_FALSE;

               continue;
          }

          if (strcmp (arg, "-s") == 0 || strcmp (arg, "--sizes") == 0) {
               if (++n == argc) {
                    print_usage (argv[0]);
                    return DFB_FALSE;
               }

               if (!parse_sizes( argv[n] ))
                    return DFB_FALSE;

               continue;
          }

          if (strcmp (arg, "-p") == 0 || strcmp (arg, "--premult") == 0) {
               premult = true;
               continue;
          }

          if (filename || access( arg, R_OK )) {
               print_usage (argv[0]);
               return DFB_FALSE;
          }

          filename = arg;
     }

     if (!filename) {
          print_usage (argv[0]);
          return DFB_FALSE;
     }

     return DFB_TRUE;
}

/**********************************************************************************************************************/

static void
write_glyph( DGIFFGlyphInfo *glyph, FT_GlyphSlot slot, void *dst, int pitch )
{
     int  y;
     u8  *src = slot->bitmap.buffer;

     D_DEBUG_AT( mkdgiff, "%s( %p, %p, %p, %d ) <- width %d\n",
                 __FUNCTION__, glyph, slot, dst, pitch, glyph->width );

     for (y=0; y < glyph->height; y++) {
          int  i, j, n;
          u8  *dst8  = dst;
          u16 *dst16 = dst;
          u32 *dst32 = dst;

          switch (slot->bitmap.pixel_mode) {
               case ft_pixel_mode_grays:
                    switch (format) {
                         case DSPF_ARGB:
                              if (premult) {
                                   for (i=0; i<glyph->width; i++)
                                        dst32[i] = ((src[i] << 24) |
                                                    (src[i] << 16) |
                                                    (src[i] <<  8) | src[i]);
                              }
                              else {
                                   for (i=0; i<glyph->width; i++)
                                        dst32[i] = (src[i] << 24) | 0xFFFFFF;
                              }
                              break;
                         case DSPF_AiRGB:
                              for (i=0; i<glyph->width; i++)
                                   dst32[i] = ((src[i] ^ 0xFF) << 24) | 0xFFFFFF;
                              break;
                         case DSPF_ARGB8565:
                              for (i=0, j = -1; i<glyph->width; ++i) {
                                   u32 d = (src[i] << 16) | 0xFFFF;
#ifdef WORDS_BIGENDIAN
                                   dst8[++j] = (d >> 16) & 0xff;
                                   dst8[++j] = (d >>  8) & 0xff;
                                   dst8[++j] = (d >>  0) & 0xff;
#else
                                   dst8[++j] = (d >>  0) & 0xff;
                                   dst8[++j] = (d >>  8) & 0xff;
                                   dst8[++j] = (d >> 16) & 0xff;
#endif
                              }
                              break;
                         case DSPF_ARGB4444:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = (src[i] << 8) | 0xFFF;
                              break;
                         case DSPF_ARGB2554:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = (src[i] << 8) | 0x3FFF;
                              break;
                         case DSPF_ARGB1555:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = (src[i] << 8) | 0x7FFF;
                              break;
                         case DSPF_RGBA5551:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = 0xFFFE | ((src[i] & 0x80) >> 7);
                              break;
                         case DSPF_A8:
                              direct_memcpy( dst, src, glyph->width );
                              break;
                         case DSPF_A4:
                              for (i=0, j=0; i<glyph->width; i+=2, j++)
                                   dst8[j] = (src[i] & 0xF0) | (src[i+1] >> 4);
                              break;
                         case DSPF_A1:
                              for (i=0, j=0; i < glyph->width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<glyph->width; ++i, ++n)
                                        p |= (src[i] & 0x80) >> n;

                                   dst8[j] = p;
                              }
                              break;
                         case DSPF_A1_LSB:
                              for (i=0, j=0; i < glyph->width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<glyph->width; ++i, ++n)
                                        p |= (src[i] & 0x80) >> (7-n);

                                   dst8[j] = p;
                              }
                              break;
                         default:
                              break;
                    }
                    break;

               case ft_pixel_mode_mono:
                    switch (format) {
                         case DSPF_ARGB:
                              if (premult) {
                                   for (i=0; i<glyph->width; i++)
                                        dst32[i] = ((src[i>>3] & (1<<(7-(i%8)))) ?
                                                     0xFFFFFFFF : 0x00000000);
                              }
                              else {
                                   for (i=0; i<glyph->width; i++)
                                        dst32[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                     0xFF : 0x00) << 24) | 0xFFFFFF;
                              }
                              break;
                         case DSPF_AiRGB:
                              for (i=0; i<glyph->width; i++)
                                   dst32[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0x00 : 0xFF) << 24) | 0xFFFFFF;
                              break;
                         case DSPF_ARGB8565:
                              for (i=0, j = -1; i<glyph->width; ++i) {
                                   u32 d = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                             0xFF : 0x00) << 16) | 0xFFFF;
#ifdef WORDS_BIGENDIAN
                                   dst8[++j] = (d >> 16) & 0xff;
                                   dst8[++j] = (d >>  8) & 0xff;
                                   dst8[++j] = (d >>  0) & 0xff;
#else
                                   dst8[++j] = (d >>  0) & 0xff;
                                   dst8[++j] = (d >>  8) & 0xff;
                                   dst8[++j] = (d >> 16) & 0xff;
#endif
                              }
                              break;
                         case DSPF_ARGB4444:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0xF : 0x0) << 12) | 0xFFF;
                              break;
                         case DSPF_ARGB2554:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0x3 : 0x0) << 14) | 0x3FFF;
                              break;
                         case DSPF_ARGB1555:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = (((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0x1 : 0x0) << 15) | 0x7FFF;
                              break;
                         case DSPF_RGBA5551:
                              for (i=0; i<glyph->width; i++)
                                   dst16[i] = 0xFFFE | ((src[i>>3] & (1<<(7-(i%8)))) ?
                                                0x1 : 0x0);
                              break;
                         case DSPF_A8:
                              for (i=0; i<glyph->width; i++)
                                   dst8[i] = (src[i>>3] &
                                              (1<<(7-(i%8)))) ? 0xFF : 0x00;
                              break;
                         case DSPF_A4:
                              for (i=0, j=0; i<glyph->width; i+=2, j++)
                                   dst8[j] = ((src[i>>3] &
                                               (1<<(7-(i%8)))) ? 0xF0 : 0x00) |
                                             ((src[(i+1)>>3] &
                                               (1<<(7-((i+1)%8)))) ? 0x0F : 0x00);
                              break;
                         case DSPF_A1:
                              direct_memcpy( dst, src, DFB_BYTES_PER_LINE(DSPF_A1, glyph->width) );
                              break;
                         case DSPF_A1_LSB:
                              for (i=0, j=0; i < glyph->width; ++j) {
                                   register u8 p = 0;

                                   for (n=0; n<8 && i<glyph->width; ++i, ++n)
                                        p |= (((src[i] >> n) & 1) << (7-n));

                                   dst8[j] = p;
                              }
                              break;
                         default:
                              break;
                    }
                    break;

               default:
                    break;

          }

          src += slot->bitmap.pitch;
          dst += pitch;
     }
}

static int
do_face( FT_Face face, int size )
{
     int              i, ret;
     int              align        = DFB_PIXELFORMAT_ALIGNMENT( format );
     int              num_glyphs   = 0;
     int              num_rows     = 1;
     int              row_index    = 0;
     int              row_offset   = 0;
     int              next_face    = sizeof(DGIFFFaceHeader);
     int              total_height = 0;
     FT_ULong         code;
     FT_UInt          index;
     DGIFFFaceHeader  header;
     DGIFFGlyphInfo  *glyphs;
     DGIFFGlyphRow   *rows;
     void           **row_data;

     D_DEBUG_AT( mkdgiff, "%s( %p, %d ) <- %ld glyphs\n", __FUNCTION__, face, size, face->num_glyphs );

     /* Clear to not leak any data into file. */
     memset( &header, 0, sizeof(header) );

     /* Set the desired size. */
     ret = FT_Set_Char_Size( face, 0, size << 6, 0, 0 );
     if (ret) {
          D_ERROR( "Could not set pixel size to %d!\n", size );
          return ret;
     }

     /* Allocate glyph info array. */
     glyphs   = D_CALLOC( face->num_glyphs, sizeof(DGIFFGlyphInfo) );
     rows     = D_CALLOC( face->num_glyphs, sizeof(DGIFFGlyphRow) ); /* WORST case :) */
     row_data = D_CALLOC( face->num_glyphs, sizeof(void*) );         /* WORST case :) */

     for (code = FT_Get_First_Char( face, &index );
          index;
          code = FT_Get_Next_Char( face, code, &index ))
     {
          FT_GlyphSlot    slot;
          DGIFFGlyphInfo *glyph = &glyphs[num_glyphs];
          DGIFFGlyphRow  *row   = &rows[num_rows - 1];

          D_DEBUG_AT( mkdgiff, "  -> code %3lu - index %3u\n", code, index );

          if (num_glyphs == face->num_glyphs) {
               D_ERROR( "Actual number of characters is bigger than number of glyphs!\n" );
               break;
          }

          ret = FT_Load_Glyph( face, index, FT_LOAD_RENDER );
          if (ret) {
               D_ERROR( "Could not render glyph for character index %d!\n", index );
               goto out;
          }

          slot = face->glyph;

          glyph->unicode = code;

          glyph->width   = slot->bitmap.width;
          glyph->height  = slot->bitmap.rows;

          glyph->left    = slot->bitmap_left;
          glyph->top     = (face->size->metrics.ascender >> 6) - slot->bitmap_top;
          glyph->advance = slot->advance.x >> 6;

          num_glyphs++;

          if (row->width > 0 && row->width + glyph->width > MAX_ROW_WIDTH) {
               num_rows++;
               row++;
          }

          row->width += (glyph->width + align) & ~align;

          if (row->height < glyph->height)
               row->height = glyph->height;
     }

     for (i=0; i<num_rows; i++) {
          DGIFFGlyphRow *row = &rows[i];

          D_DEBUG_AT( mkdgiff, "  ->   row %d, width %d, height %d\n", i, row->width, row->height );

          total_height += row->height;

          row->pitch = (DFB_BYTES_PER_LINE( format, row->width ) + 7) & ~7;

          row_data[i] = D_CALLOC( row->height, row->pitch );

          next_face += row->height * row->pitch;
     }

     D_DEBUG_AT( mkdgiff, "  -> %d glyphs, %d rows, total height %d\n", num_glyphs, num_rows, total_height );

     next_face += num_glyphs * sizeof(DGIFFGlyphInfo);
     next_face += num_rows * sizeof(DGIFFGlyphRow);

     for (i=0; i<num_glyphs; i++) {
          DGIFFGlyphInfo *glyph = &glyphs[i];

          D_DEBUG_AT( mkdgiff, "  -> reloading character 0x%x (%d)\n", glyph->unicode, i );

          ret = FT_Load_Char( face, glyph->unicode, FT_LOAD_RENDER );
          if (ret) {
               D_ERROR( "Could not render glyph for unicode character 0x%x!\n", glyph->unicode );
               goto out;
          }

          if (row_offset > 0 && row_offset + glyph->width > MAX_ROW_WIDTH) {
               row_index++;
               row_offset = 0;
          }


          D_DEBUG_AT( mkdgiff, "  -> row offset %d\n", row_offset );

          write_glyph( glyph, face->glyph,
                       row_data[row_index] + DFB_BYTES_PER_LINE( format, row_offset ),
                       rows[row_index].pitch );

          glyph->row    = row_index;
          glyph->offset = row_offset;

          row_offset += (glyph->width + align) & ~align;
     }

     D_ASSERT( row_index == num_rows - 1 );

     header.next_face   = next_face;
     header.size        = size;

     header.ascender    = face->size->metrics.ascender >> 6;
     header.descender   = face->size->metrics.descender >> 6;
     header.height      = header.ascender - header.descender + 1;

     header.max_advance = face->size->metrics.max_advance >> 6;

     header.pixelformat = format;

     header.num_glyphs  = num_glyphs;
     header.num_rows    = num_rows;

     D_DEBUG_AT( mkdgiff, "  -> ascender %d, descender %d\n", header.ascender, header.descender );
     D_DEBUG_AT( mkdgiff, "  -> height %d, max advance %d\n", header.height, header.max_advance );

     fwrite( &header, sizeof(header), 1, stdout );

     fwrite( glyphs, sizeof(*glyphs), num_glyphs, stdout );

     for (i=0; i<num_rows; i++) {
          DGIFFGlyphRow *row = &rows[i];

          fwrite( row, sizeof(*row), 1, stdout );

          fwrite( row_data[i], row->pitch, row->height, stdout );
     }

out:
     for (i=0; i<num_rows; i++) {
          if (row_data[i])
               D_FREE( row_data[i] );
     }

     D_FREE( row_data );
     D_FREE( rows );
     D_FREE( glyphs );

     return ret;
}

/**********************************************************************************************************************/

static DGIFFHeader header = {
     .magic = { 'D', 'G', 'I', 'F', 'F'},
     .major = 0,
     .minor = 0,
     .flags = DGIFF_FLAG_LITTLE_ENDIAN,
     .num_faces = 0
};

int
main( int argc, char *argv[] )
{
     int        i, ret;
     FT_Library library = NULL;
     FT_Face    face    = NULL;

     direct_initialize();

     direct_config->debug    = true;
     direct_config->debugmem = true;

     /* Parse the command line. */
     if (!parse_command_line( argc, argv ))
          return -1;

     if (premult && format != DSPF_ARGB) {
          fprintf( stderr, "\n\nPremultiplied alpha only implemented for ARGB!\n" );
          return -2;
     }

     if (!size_count) {
          fprintf( stderr, "\n\nUsing default sizes 8, 10, 12, 16, 22, 32\n" );

          size_count = 6;

          face_sizes[0] =  8;
          face_sizes[1] = 10;
          face_sizes[2] = 12;
          face_sizes[3] = 16;
          face_sizes[4] = 22;
          face_sizes[5] = 32;
     }

     header.num_faces = size_count;


     ret = FT_Init_FreeType( &library );
     if (ret) {
          D_ERROR( "Initialization of the FreeType2 library failed!\n" );
          goto out;
     }

     ret = FT_New_Face( library, filename, face_index, &face );
     if (ret) {
          if (ret == FT_Err_Unknown_File_Format)
               D_ERROR( "Unsupported font format in file `%s'!\n", filename );
          else
               D_ERROR( "Failed loading face %d from font file `%s'!\n", face_index, filename );

          goto out;
     }

     ret = FT_Select_Charmap( face, ft_encoding_unicode );
     if (ret) {
          D_ERROR( "Couldn't select Unicode encoding, falling back to Latin1.\n" );

          ret = FT_Select_Charmap( face, ft_encoding_latin_1 );
          if (ret)
               D_ERROR( "Couldn't even select Latin1 encoding!\n" );
     }

     fwrite( &header, sizeof(header), 1, stdout );

     for (i=0; i<size_count; i++) {
          ret = do_face( face, face_sizes[i] );
          if (ret)
               goto out;
     }


out:
     if (face)
          FT_Done_Face( face );

     if (library)
          FT_Done_FreeType( library );

     direct_print_memleaks();

     direct_shutdown();

     return ret;
}


