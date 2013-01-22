/* libmb
 * Copyright (C) 2002 Matthew Allum
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

/* 
 * XXXXXXXXX WARNING XXXXXXXXXXXX
 *
 * mbexp contains experimetal unstable code that is subject to
 * constant change and therefore USE AT YOUR OWN RISK. 
 *
 * XXXXXXXXX WARNING XXXXXXXXXXXX
 */

#ifndef _MB_EXP_H_
#define _MB_EXP_H_

#include "libmb/mbconfig.h"
#include "mbpixbuf.h"
#include "mbdotdesktop.h"

#ifdef USE_XFT
#include <X11/Xft/Xft.h>
#include <locale.h>
#include <langinfo.h>
#endif

#ifdef USE_PANGO
#include <pango/pango.h>
#include <pango/pangoxft.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup MBFont MBFont - Simple font abstraction and rendering tools
 * @brief mbpixbuf contains simple image manipulation and composition 
 * functions for client side images.
 *
 *
 *
 * @{
 */

/**
 * @typedef MBColor
 *
 * 
 *
 * Its not recommended you touch structure internals directly. 
 */
typedef struct MBColor 
{
  MBPixbuf     *pb;

  unsigned char r;
  unsigned char g;
  unsigned char b;
  unsigned char a;

  XColor        xcol;
#if defined (USE_XFT) || defined (USE_PANGO)
  XftColor      xftcol;
#endif
  int           ref_cnt;

} MBColor;

/**
 * @typedef MBFont
 *
 * 
 *
 * Its not recommended you touch structure internals directly. 
 */
typedef struct MBFont 
{
  Display              *dpy; 
  char                 *family;
  int                   weight;
  int                   slant;
  int                   pt_size;
  Bool                  have_shadow; /* A *little* hakkie */

  MBColor              *col;

#ifdef USE_PANGO
  PangoContext         *pgo_context;
  PangoFontMap         *pgo_fontmap;
  PangoFontDescription *fontdes;
  PangoFont            *font;
  PangoFontMetrics     *metrics;
#elif defined (USE_XFT)
  XftFont              *font;
#else
  GC                    gc;
  XFontStruct          *font;
#endif

  Bool                  _have_fresh_font_object;
  int                   ref_cnt;

} MBFont;

/**
 * @typedef MBEncoding
 *
 * enumerated types for text encodings
 */
typedef enum {

  MB_ENCODING_LATIN,
  MB_ENCODING_UTF8

} MBEncoding;

/**
 * @typedef MBFontWeight
 *
 * enumerated types for font weights.
 */
typedef enum {

  MB_NORMAL = 0,
  MB_LIGHT,
  MB_MEDIUM,
  MB_DEMIBOLD,
  MB_BOLD,
  MB_BLACK,
  MB_N_WEIGHTS

} MBFontWeight;

/**
 * @typedef MBFontSlant
 *
 * enumerated types for font slant styles
 */
typedef enum {

  MB_ROMAN = 0,
  MB_ITALIC,
  MB_OBLIQUE,

  MB_N_SLANTS

} MBFontSlant;

/**
 * @typedef MBFontRenderOpts
 *
 * Option flags for text rendering
 */
typedef enum {

  MB_FONT_RENDER_OPTS_NONE       = 0,
  MB_FONT_RENDER_OPTS_CLIP_TRAIL = (1<<1),
  MB_FONT_RENDER_ALIGN_CENTER    = (1<<2),
  MB_FONT_RENDER_ALIGN_RIGHT     = (1<<3),
  MB_FONT_RENDER_EFFECT_SHADOW   = (1<<4),
  MB_FONT_RENDER_VALIGN_MIDDLE    = (1<<5)

} MBFontRenderOpts;

/**
 * @typedef MBDrawable
 *
 * Type for representing an mbdrawable. This basically
 * wraps X pixmaps and Xft Drawables 
 *
 * Its not recommended you touch this directly. 
 */
typedef struct MBDrawable 
{
  MBPixbuf   *pb;
  Pixmap      xpixmap;
  Bool        have_ext_pxm;
#if defined (USE_XFT) || defined (USE_PANGO)
  XftDraw    *xftdraw;
#endif

  int         width, height;
} MBDrawable;

/**
 * @typedef MBLayout
 *
 * Experimental and therefore not as yet documented
 *
 * Its not recommended you touch this directly. 
 */
typedef struct MBLayout 
{
  int            x;
  int            y;
  int            width;
  int            height;
  int            line_spacing;

  unsigned char *txt;
  MBEncoding     txt_encoding;
  MBFont        *font;

  Bool           _have_autocalc_size;

} MBLayout;

/* Color */

/**
 * Constructs a new MBColor from a string specification
 * 
 * The format of the specification can be a color name
 * or the format '#rrggbb[aa]'
 *
 * @param pb MBPixbuf reference
 * @param spec 
 * @returns a MBColor object or NULL on failure.
 */
MBColor*
mb_col_new_from_spec (MBPixbuf *pb, char *spec);


/**
 * Sets an existing MBColor from a string specification
 * 
 * The format of the specification can be a color name
 * or the format '#rrggbb[aa]'
 *
 * @param col  MBColor object to modify 
 * @param spec New Color specification
 * @returns True or False on failure.
 */
Bool
mb_col_set (MBColor *col, char *spec);

/**
 * Sets an existing MBColor from r,g,b,a values
 * 
 *
 * @param col MBColor object to modify 
 * @param red red component
 * @param green green component
 * @param blue blue component
 * @param alpha alpha component
 */
void
mb_col_set_rgba (MBColor       *col, 
		 unsigned char red,
		 unsigned char green,
		 unsigned char blue,
		 unsigned char alpha);

/**
 * Gets an r,g,b,a values from an existing MBColor
 * 
 *
 * @param col MBColor object to query
 * @param red red component
 * @param green green component
 * @param blue blue component
 * @param alpha alpha component
 */
void
mb_col_get_rgba (MBColor       *col, 
		 unsigned char *red,
		 unsigned char *green,
		 unsigned char *blue,
		 unsigned char *alpha);


#define mb_col_red(col)   (col)->r
#define mb_col_green(col) (col)->g
#define mb_col_blue(col)  (col)->b
#define mb_col_alpha(col) (col)->a
#define mb_col_xpixel(col) (col)->xcol.pixel


/* FIXME: need mb_col_ref func */

/**
 * Unrefs ( frees ) a created MBColor object.
 *
 * @param col MBColor object to unref
 */
void
mb_col_unref (MBColor *col);

/* Fonts */

/**
 * Constructs a new MBFont instance
 *
 * @param dpy X Display
 * @param family font family name or NULL
 * @returns a MBFont object or NULL on failure.
 */
MBFont*
mb_font_new (Display *dpy, 
	     char    *family);

/**
 * Refs a created MBFont object.
 *
 * @param font MBFont object to unref
 */
void
mb_font_ref(MBFont* font);

/**
 * Unrefs ( frees ) a created MBColor object.
 *
 * @param font MBFont object to unref
 */
void
mb_font_unref(MBFont* font);

/**
 * Constructs a new MBFont instance
 *
 * @param dpy X Display
 * @param spec A description of the font. 
 *             This will take Gtk2 style font descriptions or Xft style ones
 * @returns a MBFont object or NULL on failure.
 */
MBFont*
mb_font_new_from_string(Display *dpy, char *spec) ;

/**
 * Sets the font propertys from a textual description
 *
 * @param font the font to alter
 * @param spec A description of the font. 
 *             This will take Gtk2 style font descriptions or Xft style ones
 * @returns the MBFont object
 */
MBFont*
mb_font_set_from_string(MBFont *font, char *spec) ;

/**
 * Sets the fonts family.
 *
 * @param font the font to alter
 * @param family font family name
 */
void
mb_font_set_family (MBFont *font, const char *family);

/**
 * Gets the fonts family.
 *
 * @param font The font to query
 * @returns font family name
 */
char*
mb_font_get_family (MBFont *font);

/* FIXME: what is this ? */
char*
mb_font_get (MBFont *font);

/**
 * Sets the fonts weight.
 *
 * @param font The font to alter
 * @param weight The requested font weight
 */
void
mb_font_set_weight (MBFont *font, MBFontWeight weight);

/**
 * Gets the fonts weight.
 *
 * @param font The font to query
 * @returns The font weight
 */
MBFontWeight
mb_font_get_weight (MBFont *font);

/**
 * Sets the fonts slant.
 *
 * @param font The font to alter
 * @param slant The requested font slant
 */
void
mb_font_set_slant (MBFont *font, MBFontSlant slant);


/* FIXME: need mb_font_get_slant() */

/**
 * Sets the fonts size.
 *
 * @param font The font to alter
 * @param points requested size in points
 */
void
mb_font_set_point_size (MBFont *font, int points);

/**
 * Gets the fonts point size.
 *
 * @param font The font to query
 * @returns The font point size
 */
int
mb_font_get_point_size (MBFont *font);

/**
 * Attempts to fit the point size to a pixel size
 *
 * @param font The font to alter
 * @param max_pixels Pixel size to fit to
 * @param points_to_try a list on point sizes to try or NULL ( to use default values )
 * @returns 1 on a successful fit, 0 on failure. 
 */
int
mb_font_set_size_to_pixels (MBFont *font, int max_pixels, int *points_to_try);

/**
 * Sets the fonts color.
 * *Note* you must set a fonts color for it to be rendered 
 * - there is no default.
 *
 * @param font The font to alter
 * @param col The requested color.
 */
void
mb_font_set_color (MBFont *font, MBColor *col);

/**
 * Gets the fonts point size.
 *
 * @param font The font to query
 * @returns The fonts color 
 */
MBColor *
mb_font_get_color (MBFont *font);

/* FIXME: encoding param should be enum */
int 
mb_font_get_txt_width (MBFont        *font, 
		       unsigned char *txt, 
		       int            byte_len, 
		       int            encoding);

/**
 * Gets the fonts height in pixels
 *
 * @param font The font to query
 * @returns The font height in pixels
 */
int 
mb_font_get_height (MBFont *font);

/**
 * Gets the fonts ascent in pixels
 *
 * @param font The font to query
 * @returns The font ascent in pixels
 */
int 
mb_font_get_ascent(MBFont *font);

/**
 * Gets the fonts descent in pixels
 *
 * @param font The font to query
 * @returns The font descent in pixels
 */
int 
mb_font_get_descent(MBFont *font);


/**
 * Renders a line of text onto a MBDrawable
 *
 * @param font The font to render
 * @param drw  The MBDrawable to render too
 * @param x    The X position on MBDrawable to render too
 * @param y    The Y position on MBDrawable to render too
 * @param width The maximum width in pixels to render
 * @param text The text to render.
 * @param encoding the encoding of the text to render
 * @param opts Or'd #MBFontRenderOpts
 * @returns The number of glyths rendered
 */
int
mb_font_render_simple (MBFont          *font, 
		       MBDrawable      *drw, 
		       int              x,
		       int              y,
		       int              width,
		       unsigned char   *text,
		       int              encoding,
		       MBFontRenderOpts opts);

/**
 * Returns the width in pixels of any text rendered with
 * #mb_font_render_simple, taking into account any clipping.
 * 
 * @param font The font to render
 * @param width The maximum width in pixels to render
 * @param text The text to render.
 * @param encoding the encoding of the text to render
 * @param opts Or'd #MBFontRenderOpts
 * @returns The width in pixels
 */
int
mb_font_render_simple_get_width (MBFont          *font, 
				 int              width,
				 unsigned char   *text,
				 int              encoding,
				 MBFontRenderOpts opts );


/* Layout stuff 
 *
 * XXX: This stuff is experimental, subject to change and shouldn't 
 *      really be used as yet unless you really know what you doing.
 *
 */

MBLayout *
mb_layout_new (void);

void
mb_layout_unref (MBLayout *layout);

void
mb_layout_set_font(MBLayout *layout, MBFont *font);


/* If layout not set size is autocalculated */
void
mb_layout_set_geometry(MBLayout *layout,
		       int width, int height);

void
mb_layout_get_geometry(MBLayout *layout, 
		       int *width, int *height);

/* call set text, then get geometry for the space it takes up */
void
mb_layout_set_text(MBLayout       *layout, 
		   unsigned char  *text, 
		   MBEncoding      encoding); /* XXX plus text len ? */

void
mb_layout_destroy (MBLayout *layout);

void
mb_layout_set_multiline (MBLayout *layout, Bool want_multiline);

void
mb_layout_set_align (MBLayout *layout, int horizontal, int vertical);

void 	    /* dots, word, letter, none  */
mb_layout_set_clip_style (MBLayout *layout, int clip_stype);

void
mb_layout_render (MBLayout        *layout, 
		  MBDrawable      *drw, 
		  int              x, 
		  int              y,
		  MBFontRenderOpts opts);

#define mb_layout_width(l) (l)->width
#define mb_layout_height(l) (l)->height


/* Drawables */

/**
 * Creates a new MBDrawable instance. MBDrawables are what MBfonts get 
 * rendered too.  
 *
 * @param pixbuf A MBPixbuf Instance. 
 * @param width  Width of requested drawable. 
 * @param height Height of requested drawable. 
 * @returns A MBDrawable Instance
 */
MBDrawable*
mb_drawable_new (MBPixbuf *pixbuf, int width, int height);

/**
 * Creates a new MBDrawable instance from a pre-existing pixmap. 
 * Note, if created like this you are responsable for freeing the
 *       the drawables pixmap
 *
 * @param pixbuf A MBPixbuf Instance. 
 * @param pxm the pixmap to create the drawable from
 * @returns A MBDrawable Instance
 */
MBDrawable*
mb_drawable_new_from_pixmap (MBPixbuf *pixbuf, Pixmap pxm);

/**
 * Unrefs ( frees ) a drawable
 *
 * @param drw MBDrawable to unref
 * 
 */
void
mb_drawable_unref (MBDrawable* drw);

/**
 * @def mb_drawable_pixmap 
 *
 * Returns a drawables X pixmap
 */
#define mb_drawable_pixmap(drw) (drw)->xpixmap

/* FIXME: document */
int
mb_util_next_utf8_char (unsigned char **string);

/** @} */

#ifdef __cplusplus
}
#endif

#endif
