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

#define _GNU_SOURCE

#include "mbexp.h"

void /* Not Used  */
mb_init(void)
{
  /* XXXX init gtype system here ? */

}

MBColor *
mb_col_new_from_spec(MBPixbuf *pb, char *spec)
{
  MBColor *col = NULL;

  col = malloc(sizeof(MBColor));
  memset(col, 0, sizeof(MBColor));

  col->pb      = pb;

  if (!mb_col_set (col, spec))
    {
      free(col);
      return NULL;
    }

  col->ref_cnt = 1;

  return col;
}

static Bool
_col_init (MBColor *col)
{
  MBPixbuf     *pb = col->pb;
#if defined (USE_XFT) || defined (USE_PANGO)
  XRenderColor  colortmp;
#endif

  XAllocColor(pb->dpy,
	      DefaultColormap(pb->dpy, pb->scr),
	      &col->xcol);

#if defined (USE_XFT) || defined (USE_PANGO)
  colortmp.red   = col->r << 8;
  colortmp.green = col->g << 8;
  colortmp.blue  = col->b << 8;
  colortmp.alpha = col->a << 8;

  XftColorAllocValue(pb->dpy,
		     DefaultVisual(pb->dpy, pb->scr),
		     DefaultColormap(pb->dpy, pb->scr),
		     &colortmp, &col->xftcol);
#endif

  return True;
}

Bool
mb_col_set (MBColor *col, char *spec)
{
  MBPixbuf *pb = col->pb;

  mb_col_set_rgba (col, 0xff, 0xff, 0xff, 0xff);

  if (spec)
    {
      if (spec[0] == '#')
	{
	  int result;
	  if ( sscanf (spec+1, "%x", &result))
	    {
	      if (strlen(spec) == 9)
		{
		  col->r = result >> 24 & 0xff;
		  col->g = (result >> 16) & 0xff;
		  col->b = (result >> 8) & 0xff;
		  col->a = result & 0xff;
		}
	      else
		{
		  col->r = (result >> 16) & 0xff;
		  col->g = (result >> 8) & 0xff;
		  col->b = result & 0xff;
		  col->a = 0xff;
		}
	    }
	  else
	    {
	      if (mb_want_warnings())
		fprintf(stderr, "mbcolor: failed to parse color %s\n", spec);
	      return False;
	    } 

	  col->xcol.red   = col->r << 8;
	  col->xcol.green = col->g << 8;
	  col->xcol.blue  = col->b << 8;
	  col->xcol.flags = DoRed|DoGreen|DoBlue;
	}
      else
	{
	  if (!XParseColor(pb->dpy,
			   DefaultColormap(pb->dpy, pb->scr),
			   spec, &col->xcol))
	    {
	      if (mb_want_warnings())
		fprintf(stderr, "mbcolor: failed to parse color %s\n", spec);
	      return False;
	    } 

	  col->r = col->xcol.red >> 8;
	  col->g = col->xcol.green >> 8;
	  col->b = col->xcol.blue >> 8;

	}
    }

  return _col_init(col);
}

void
mb_col_get_rgba (MBColor       *col, 
		 unsigned char *red,
		 unsigned char *green,
		 unsigned char *blue,
		 unsigned char *alpha)
{
  *red    = col->r;
  *green  = col->g;
  *blue   = col->b;
  *alpha  = col->a;
}

void
mb_col_set_rgba (MBColor       *col, 
		 unsigned char red,
		 unsigned char green,
		 unsigned char blue,
		 unsigned char alpha)
{
  col->r = red;
  col->g = green;
  col->b = blue;
  col->a = alpha;

  _col_init(col);
}


void
mb_col_unref(MBColor *col)
{
  col->ref_cnt--;
  if (col->ref_cnt == 0)
    {
#if defined (USE_XFT) || defined (USE_PANGO)
      MBPixbuf *pb = col->pb;
      XftColorFree (pb->dpy,
                    DefaultVisual(pb->dpy, pb->scr),
                    DefaultColormap(pb->dpy, pb->scr),
                    &col->xftcol);
#endif
      free(col);
      col = NULL;
    }
}

static void
_mb_font_set_font_object_freshness (MBFont *font, Bool is_fresh)
{
  font->_have_fresh_font_object = is_fresh;
}

static Bool
_mb_font_is_font_object_fresh (MBFont *font)
{
  return font->_have_fresh_font_object;
}

static void
_mb_font_free(MBFont *font) 	/* FIXME: should be _unload */
{
  if (font->font == NULL) return;

#if defined (USE_PANGO)
  if (font->metrics) pango_font_metrics_unref(font->metrics);
  if (font->font)   g_object_unref (font->font);
#elif defined (USE_XFT)
  XftFontClose(font->dpy, font->font);
#else
  XFreeFont(font->dpy, font->font);
#endif
  font->font = NULL;
}

static int
_mb_font_load(MBFont *font)
{
  int i;
  int result = 0; /*   0 - complete failure, 1 - failed but loaded fallback
                   *   2 - loaded fine.  
		   */

#if defined (USE_PANGO)

  struct wlookup 
  {
    MBFontWeight mb_weight;
    PangoWeight  pgo_weight;
  } weight_lookup[] = {
    { MB_NORMAL , PANGO_WEIGHT_NORMAL } ,
    { MB_LIGHT,   PANGO_WEIGHT_LIGHT  } ,
    { MB_MEDIUM,  PANGO_WEIGHT_NORMAL } ,
    { MB_DEMIBOLD,PANGO_WEIGHT_BOLD   } ,
    { MB_BOLD,    PANGO_WEIGHT_ULTRABOLD } ,
    { MB_BLACK,   PANGO_WEIGHT_HEAVY    } ,
  };

  struct slookup 
  {
    MBFontSlant mb_slant;
    PangoStyle  pgo_slant;
  } slant_lookup[] = {
    { MB_ROMAN,   PANGO_STYLE_NORMAL },
    { MB_ITALIC,  PANGO_STYLE_ITALIC  },
    { MB_OBLIQUE, PANGO_STYLE_OBLIQUE },
  };

#elif defined (USE_XFT)

  struct wlookup 
  {
    MBFontWeight mb_weight;
    int          xft_weight;
  } weight_lookup[] = {
    { MB_NORMAL , 0                 } ,
    { MB_LIGHT,   XFT_WEIGHT_LIGHT  } ,
    { MB_MEDIUM,  XFT_WEIGHT_MEDIUM   },
    { MB_DEMIBOLD,XFT_WEIGHT_DEMIBOLD },
    { MB_BOLD,    XFT_WEIGHT_BOLD     },
    { MB_BLACK,   XFT_WEIGHT_BLACK    },
  };

  struct slookup 
  {
    MBFontSlant mb_slant;
    int         xft_slant;
  } slant_lookup[] = {
    { MB_ROMAN,   XFT_SLANT_ROMAN },
    { MB_ITALIC,  XFT_SLANT_ITALIC  },
    { MB_OBLIQUE, XFT_SLANT_OBLIQUE },
  };

  int weight = 0, slant = 0;

#else

  struct wlookup 
  {
    MBFontWeight  mb_weight;
    char         *x_weight;
  } weight_lookup[] = {
    { MB_NORMAL , "*"                 },
    { MB_LIGHT,   "normal"            },
    { MB_MEDIUM,  "medium"            },
    { MB_DEMIBOLD,"demibold"          },
    { MB_BOLD,    "bold"              },
    { MB_BLACK,   "black"             },
  };

  struct slookup 
  {
    MBFontSlant  mb_slant;
    char        *x_slant;
  } slant_lookup[] = {
    { MB_ROMAN,   "r" },
    { MB_ITALIC,  "l"  },
    { MB_OBLIQUE, "o"  },
  };

  char font_spec[256];
  char *weight = "*", *slant="*";

#endif

  if (_mb_font_is_font_object_fresh(font))
    _mb_font_free(font);

#ifdef USE_PANGO

  for (i=0; i < MB_N_WEIGHTS; i++)
    if (weight_lookup[i].mb_weight == font->weight)
	pango_font_description_set_weight( font->fontdes, 
					   weight_lookup[i].pgo_weight );

  for (i=0; i < MB_N_SLANTS; i++)
    if (slant_lookup[i].mb_slant == font->slant)
      pango_font_description_set_style( font->fontdes, 
					slant_lookup[i].pgo_slant );

  pango_font_description_set_family(font->fontdes, font->family);
  pango_font_description_set_size(font->fontdes, font->pt_size * PANGO_SCALE); 

  pango_context_set_font_description(font->pgo_context, 
				     font->fontdes);

  font->font = pango_font_map_load_font (font->pgo_fontmap, 
					 font->pgo_context, 
					 font->fontdes);

  font->metrics = pango_font_get_metrics(font->font, NULL);

  if (font->font != NULL) result = 2;

#elif defined (USE_XFT)

  for (i=0; i < MB_N_WEIGHTS; i++)
    if (weight_lookup[i].mb_weight == font->weight)
      weight = weight_lookup[i].xft_weight;

  for (i=0; i < MB_N_SLANTS; i++)
    if (slant_lookup[i].mb_slant == font->slant)
      slant = slant_lookup[i].xft_slant;


  font->font = XftFontOpen (font->dpy, DefaultScreen(font->dpy),
			    XFT_FAMILY, XftTypeString , font->family,
			    XFT_SIZE, XftTypeDouble   , (double)font->pt_size,
			    XFT_WEIGHT, XftTypeInteger, weight,
			    XFT_SLANT, XftTypeInteger , slant,
			    0);

  if (font->font != NULL ) result = 2;

#else

  /* 
   *  fndry-fmly-wght-slant-sWdth-asstyl-pxlsiz-ptsz
   *    -resx-resy-spc-avgwidth-registry-enconding
   *
   *  see http://www.meretrx.com/e93/docs/xlfd.html
   */

  for (i=0; i < MB_N_WEIGHTS; i++)
    if (weight_lookup[i].mb_weight == font->weight)
      weight = weight_lookup[i].x_weight;

  for (i=0; i < MB_N_SLANTS; i++)
    if (slant_lookup[i].mb_slant == font->slant)
      slant = slant_lookup[i].x_slant;

  snprintf(font_spec, 256, "*-%s-%s-%s-*-*-*-%i-*-*-*-*-iso8859-*",
	   font->family, weight, slant, font->pt_size * 10);
	     
  if ((font->font = XLoadQueryFont(font->dpy, font_spec)) == NULL)
    { 
      result = 1;  /* XXX: should retry with weight, slant set to '*' */ 

      if (mb_want_warnings())
	fprintf(stderr, "mbfont: failed to load %s, falling back to fixed\n", font_spec);

      if ((font->font = XLoadQueryFont(font->dpy, "fixed")) == NULL)
	{
	  if (mb_want_warnings())
	    fprintf(stderr, "mbfont: fixed failed, no usable fonts\n");
	  result = 0;
	}
    }
#endif

  _mb_font_set_font_object_freshness(font, True);

  return result;
}
	    

MBFont*
mb_font_new(Display *dpy, 
	    char    *family)
{
  MBFont *font = NULL;

#ifdef USE_PANGO
  /* 
   * Checking glib source it looks safe to recall this. 
   * Cannot find way to check if already called...
   */

  g_type_init() ;
#endif
  
  font = malloc(sizeof(MBFont));

  if (font == NULL)
    return NULL;

  memset(font, 0, sizeof(MBFont));

  if (family != NULL)
    font->family  = strdup(family);
  font->weight  = 0;
  font->slant   = 0;
  font->pt_size = 8;
  font->col     = NULL; 	/* XXX needs to be set to black ? */

  font->_have_fresh_font_object = False;
  font->dpy = dpy;

  font->ref_cnt = 1;

#ifdef USE_PANGO
   font->pgo_context = pango_xft_get_context (font->dpy, DefaultScreen(dpy));
   font->pgo_fontmap = pango_xft_get_font_map (font->dpy, DefaultScreen(dpy));
   font->fontdes     = pango_font_description_new ();

   /* If Pango is mis-setup the above will fail */
   if (font->pgo_context == NULL || font->pgo_fontmap == NULL || font->fontdes == NULL)
     {
       free(font);
       return NULL;
     }

#elif defined (USE_XFT)

#else
   font->gc = XCreateGC(dpy, RootWindow(dpy, 0), 0, NULL);
#endif

  return font;
}

MBFont*
mb_font_set_from_string(MBFont *font, char *spec) 
{
 struct wlookup 
  {
    MBFontWeight  mb_weight;
    char         *str;
  } weight_lookup[] = {
    { MB_NORMAL ,  "normal" } ,
    { MB_LIGHT,    "light"  } ,
    { MB_MEDIUM,   "medium" } ,
    { MB_DEMIBOLD, "bold"   } ,
    { MB_BOLD,     "ultrabold" } ,
    { MB_BLACK,    "heavy"    } ,
  };

  struct slookup 
  {
    MBFontSlant  mb_slant;
    char        *str;
  } slant_lookup[] = {
    { MB_ROMAN,   "roman" },
    { MB_ITALIC,  "italic"  },
    { MB_OBLIQUE, "oblique" },
  };

  char     *token, *p, *orig; 
  Bool     got_family      = False;
  Bool     finished        = False;
  Bool     has_comma_delim = False;

  if (spec == NULL) return NULL;

  _mb_font_set_font_object_freshness(font, False);

  orig = token = p = strdup(spec);

  /*
   *  FIXME: handle badly formed specs - eg extra spaces, commas etc
   */

  /* get the family */

  if (strchr(spec, ',') != NULL || strchr(spec, '-') != NULL) 
    has_comma_delim = True;

  while (!got_family) {
    while (*p != ',' && *p != ' ' && *p != '\0' && *p != '-' && *p != ':') 
      p++;

    if (*p == '\0')
      {
	finished = True;
	got_family = True;    
      }
    else if (*p == ' ' && !has_comma_delim)
      {
	got_family = True;
	*p = '\0';
      }
    else if (*p == ',')	  /* gtk font delimiter ',' */
      {
	/* FIXME: actually handle multiple font definitions */
	got_family = True;
	*p = '\0';
      }
    else if (*p == '-' || *p == ':' ) /* xft font family delimeter */
      {
	/* FIXME: actually handle multiple font definitions */
	got_family = True;
	*p = '\0';
      }
    else p++;

    /* FIXME: check we can actually load the font family ok */

  } 

  mb_font_set_family(font, token);

  if (finished) goto end;

  p++; token = p;

  while (!finished) 
    {
      int i;
      while (*p != ' ' && *p != '\0' && *p != ':' && *p != '|') p++;

      if (*p == '\0' || *p == '|')
	finished = True; 

      /* lookup the 'extra' styling */

      if (token[0] >= '0' && token[0] <= '9') /* Size parameter */
	{
	  /* If def ends in 'px' try and fit to pixels.
	   * 
	   * This could probably work better by using dpi and wanted
           * pixel size to estimate a list of sizes to pass to size_to_pixels()
	   */

	  if (token[strlen(token)-1] == 'x')
	    mb_font_set_size_to_pixels(font, atoi(token), NULL);	    
	  else
	    mb_font_set_point_size(font, atoi(token));
	}
      else
	{
	  *p = '\0'; 

	  for (i=0; i < MB_N_WEIGHTS; i++)
	    if (!strcasecmp(weight_lookup[i].str, token))
		mb_font_set_weight(font, weight_lookup[i].mb_weight);
	  
	  for (i=0; i < MB_N_SLANTS; i++)
	    if (!strcasecmp(slant_lookup[i].str, token))
	      mb_font_set_slant(font, slant_lookup[i].mb_slant);

	  if (!strcasecmp("shadow", token))
	    font->have_shadow = True;
	}

      p++; token = p;
    }

 end:
  free(orig);

  /* Check we can actually load the font ok */
  if (_mb_font_load(font) == 0)
    {
      mb_font_unref(font);
      return NULL;
    }

  return font;
}

MBFont*
mb_font_new_from_string(Display *dpy, char *spec) 
{
  MBFont *font = mb_font_new(dpy, NULL);

  if (font)
    return mb_font_set_from_string(font, spec); 

  return NULL;
}

MBFont*
mb_font_set_from_theme(MBFont *font)
{
  /* XXX gets via Gtk/prop etc       */
  /* will need loop to get updates.., or just let app update it */

  return NULL;
}


void
mb_font_ref(MBFont* font)
{
  font->ref_cnt++;
}

void
mb_font_unref(MBFont* font)
{
  font->ref_cnt--;

  if (!font->ref_cnt)
    {
      if (font->col)
	mb_col_unref (font->col);
#ifdef USE_PANGO
      if (font->fontdes)
	pango_font_description_free (font->fontdes);
      if (font->pgo_context)
	g_object_unref (font->pgo_context);
      /* The Pango fontmap is owned by Pango apparently */

#elif defined (USE_XFT)
      ;
#else
      if (font->gc) XFreeGC(font->dpy, font->gc);
#endif

      if (font->family) 
	free(font->family);

      _mb_font_free(font);

      free(font);
    }
}


void
mb_font_set_family(MBFont *font, const char *family)
{
  if (font->family) free(font->family);
  font->family = strdup(family);
}

char*
mb_font_get_family(MBFont *font)
{
  return font->family;
}


void 	/* Light, medium, demibold, bold or black */
mb_font_set_weight(MBFont* font, MBFontWeight weight)
{
  if (font->weight != weight)
    _mb_font_set_font_object_freshness(font, False);

  font->weight = weight;
}

MBFontWeight
mb_font_get_weight(MBFont* font)
{
  return font->weight;
}

void
mb_font_set_slant(MBFont* font, MBFontSlant slant)
{
  if (font->slant != slant)
    _mb_font_set_font_object_freshness(font, False);

  font->slant = slant;
} 

void
mb_font_set_point_size(MBFont* font, int points)
{
  if (points != font->pt_size)
    _mb_font_set_font_object_freshness(font, False);

  font->pt_size = points;
}

int
mb_font_set_size_to_pixels(MBFont* font, int max_pixels, int *points_to_try)
{
  int    likely_point_size;
  int    i = 0;
  double mm_per_pixel, inches_per_pixel;

  /* point stuff not reallu used any more */
  int    pt_sizes[]  = { 72, 48, 32, 24, 20, 18, 16, 14, 
			 12, 11, 10, 9, 8, 7, 6, 5, 0 }; 

  if (points_to_try == NULL)
    points_to_try = pt_sizes;

  mm_per_pixel = (double)DisplayHeightMM(font->dpy, DefaultScreen(font->dpy))
                   / DisplayHeight(font->dpy, DefaultScreen(font->dpy));

  /* 1 millimeter = 0.0393700787 inches */
  
  inches_per_pixel = (double)mm_per_pixel * 0.03;

  /* 1 inch = 72 PostScript points */

  likely_point_size = max_pixels * inches_per_pixel * 72;

  /*
    printf("%2f %2f, You asked for %i I reckon thats %ipt\n", 
    mm_per_pixel, inches_per_pixel, max_pixels, likely_point_size); 
  */      

  if (font->font) _mb_font_free(font);

  font->pt_size = likely_point_size;

  _mb_font_load(font);

  if (font->font && mb_font_get_height(font) < max_pixels)
    return 1;

  while (pt_sizes[i])
    {
      if (font->font) _mb_font_free(font);
      font->pt_size = pt_sizes[i];

      _mb_font_load(font);

      if (font->font && mb_font_get_height(font) < max_pixels)
	{
	  return 1;
	}
      i++;
    }
  
  return 0;
}

int
mb_font_get_point_size(MBFont* font)
{
  return font->pt_size;
}

void
mb_font_set_color(MBFont* font, MBColor *col)
{
  // if (font->col) mb_col_unref(font->col);
  font->col = col;
  font->col->ref_cnt++;
}

MBColor *
mb_font_get_color(MBFont* font)
{
  return font->col;
}

int 
mb_font_get_txt_width(MBFont        *font, 
		      unsigned char *txt, 
		      int            byte_len, 
		      int            encoding)
{

#if defined (USE_PANGO)
  GList         *items     = NULL, *items_head = NULL;
  PangoAttrList *attr_list = NULL;
  int            width     = 0;
  char          *str;

  if (!_mb_font_is_font_object_fresh (font)) /* XXX Make define ? */
    _mb_font_load(font);

  /* 
   * no markup 
   */

  attr_list = pango_attr_list_new (); /* no markup - empty attributes */
  str       = strdup(txt);

  str[byte_len] = '\0';
  /*
  pango_parse_markup (txt, byte_len, 0, &attr_list, (char **)&str, NULL,
		      &gerror );
  */

  items_head = items = pango_itemize (font->pgo_context, str, 0, 
				      strlen(str), attr_list, NULL);

  /* XXX this loop could maybe made more generic so only needed once
   *     as the render function does basically the same thing. 
   *
   *     - alternatively can pango_glyph_string_index_to_x () be used ?
   *
   *     - can some of the results be cached ? 
   */
   while (items)
     {
       PangoItem        *this   = (PangoItem *)items->data;
       PangoGlyphString *glyphs = pango_glyph_string_new ();
       PangoRectangle    rect;
       
       pango_shape  (&str[this->offset], this->length, 
		     &this->analysis, glyphs);

       pango_glyph_string_extents (  glyphs,
				     this->analysis.font,
				     &rect,
				     NULL);

       width += (( rect.x + rect.width ) / PANGO_SCALE);
       
       pango_item_free (this);
       pango_glyph_string_free (glyphs);

       items = items->next;
     }

   if (attr_list)  pango_attr_list_unref (attr_list);
   if (str)        free (str);
   if (items_head) g_list_free (items_head);
   
   return width;

#elif defined (USE_XFT)
   XGlyphInfo extents;

  if (!_mb_font_is_font_object_fresh (font)) /* XXX Make define ? */
    _mb_font_load(font);

   if (encoding == MB_ENCODING_UTF8)
     XftTextExtentsUtf8(font->dpy, font->font, txt, byte_len, &extents);
   else 
     XftTextExtents8(font->dpy, font->font, txt, byte_len, &extents);

   return extents.width;

#else

  if (!_mb_font_is_font_object_fresh (font)) /* XXX Make define ? */
    _mb_font_load(font);

   return XTextWidth(font->font, txt, byte_len);

#endif
}

int 
mb_font_get_height(MBFont *font)
{
  return mb_font_get_ascent(font) + mb_font_get_descent(font);
}

int 
mb_font_get_ascent(MBFont *font)
{
  if (!_mb_font_is_font_object_fresh (font))
    _mb_font_load(font);

#ifdef USE_PANGO
  return PANGO_PIXELS(pango_font_metrics_get_ascent(font->metrics));
#else
  return font->font->ascent;
#endif
}

int 
mb_font_get_descent(MBFont *font)
{
  if (!_mb_font_is_font_object_fresh (font))
    _mb_font_load(font);

#ifdef USE_PANGO
  return PANGO_PIXELS(pango_font_metrics_get_descent(font->metrics));
#else
  return font->font->descent;
#endif
}


/* XXX we'll just do this stuff in matchbox wm now with 
   --enable-extras or summin  */
int
mb_font_set_img_substitute(MBFont        *font, 
			   unsigned char *glyph, 
			   MBPixbufImage *img);

/* This should be public ? */
int
_clip_some_text (MBFont         *font, 
		 int             max_width,
		 unsigned char  *txt,
		 int             encoding,
		 int             opts)
{
  int len = strlen((char*)txt);

  /* we cant clip single char string */
  if (len < 2) return 0;

  /* XXX RTL case for pango */

  if (opts & MB_FONT_RENDER_OPTS_CLIP_TRAIL)
    {
      unsigned char *str = malloc(len+5);
      memset(str, 0, len+5);

      strcpy((char*)str, (char*)txt);

      do {
	  /* go back a glyth */
	  if (encoding == MB_ENCODING_UTF8)
	    {
	      do {
		len--;
	      }
	      while ((str[len] & 0xc0) == 0x80); 
	    }
	    else len--;

	  str[len]   = '.'; str[len+1] = '.'; 
	  str[len+2] = '.'; str[len+3] = '\0'; 

	  /* printf("%s() len %i str is now %s\n", __func__, len, str ); */
      }
      while (mb_font_get_txt_width(font, str, len+3, encoding) > max_width 
	     && len >= 3);

      if (len < 3) len = 0;

      free(str);
      return len;
    }
  
  while (mb_font_get_txt_width(font, txt, len, encoding) > max_width 
	 && len >= 0)
    {
      if (encoding == MB_ENCODING_UTF8) /* reverse over utf8 bytes */
	do {
	  len--;
	}
	while ((txt[len] & 0xc0) == 0x80); 
      else len--;
    }
  return len;
}

void
_render_some_text (MBFont        *font, 
		   MBDrawable    *drw, 
		   int            x,
		   int            y,
		   unsigned char *text,
		   int            bytes_to_render,
		   int            encoding)
{
  /* We Assume font is fresh */
#if defined(USE_PANGO)
  unsigned char *str = NULL;
  GList *items_head = NULL, *items = NULL;
  PangoAttrList *attr_list = NULL;


   /* XXX We dont do markup ? 
   GError *error;

   pango_parse_markup (text, strlen(text), 
		       0,
		       &attr_list,
		       (char **)&str,
		       NULL,
		       &error);
   */

   attr_list = pango_attr_list_new (); /* no markup - empty attributes */
   str       = strdup(text);

   /* analyse string, breaking up into items */
   items_head = items = pango_itemize (font->pgo_context, str, 
				       0, bytes_to_render,
				       attr_list, NULL);

   while (items)
     {
       PangoItem        *this   = (PangoItem *)items->data;
       PangoGlyphString *glyphs = pango_glyph_string_new ();
       PangoRectangle    rect;
       
       /* shape current item into run of glyphs */
       pango_shape  (&str[this->offset], this->length, 
		     &this->analysis, glyphs);

       /* render the glyphs */
       pango_xft_render (drw->xftdraw, 
			 &font->col->xftcol,
			 this->analysis.font,
			 glyphs,
			 x, y + mb_font_get_ascent(font));

       /* calculate rendered area */
       pango_glyph_string_extents (glyphs,
				   this->analysis.font,
				   &rect,
				   NULL);

       x += ( rect.x + rect.width ) / PANGO_SCALE; /* XXX Correct ? */
       
       pango_item_free (this);
       pango_glyph_string_free (glyphs);

       items = items->next;
     }

   if (attr_list)  pango_attr_list_unref (attr_list);
   if (str)        free(str); 
   if (items_head) g_list_free (items_head);


#elif defined(USE_XFT)
  if (encoding == MB_ENCODING_UTF8)
    {
      XftDrawStringUtf8(drw->xftdraw, &font->col->xftcol, font->font,
			x, y + font->font->ascent, text, bytes_to_render);
    }
  else 
    {
      XftDrawString8(drw->xftdraw, &font->col->xftcol, font->font,
		     x, y + font->font->ascent, text, bytes_to_render);
    }
#else

      XSetFont(font->dpy, font->gc, font->font->fid);
      XSetForeground(font->dpy, font->gc, font->col->xcol.pixel);
      XDrawString(font->dpy, drw->xpixmap, font->gc, 
		  x , y + font->font->ascent, 
		  text, bytes_to_render);
#endif

}

int
mb_font_render_simple (MBFont          *font, 
		       MBDrawable      *drw, 
		       int              x,
		       int              y,
		       int              width,
		       unsigned char   *text,
		       int              encoding,
		       MBFontRenderOpts opts )
{
  int render_w = 0, len = 0, orig_len = 0;
  unsigned char *str = NULL;
  Bool want_dots = False;

  if (text == NULL) return 0;

  if (font->col == NULL) 
    {
      if (mb_want_warnings())
	fprintf(stderr, 
		"libmb: **error** font has no color set. unable to render\n");
      return 0;
    }

  if (!_mb_font_is_font_object_fresh (font))
    _mb_font_load(font);

  orig_len = len = strlen((char*)text);

  str = malloc(len+3);
  memset(str, 0, len+3);

  strcpy((char*)str, (char*)text);

  render_w = mb_font_get_txt_width(font, str, len, encoding);

  if (render_w > width)
    {
      /*   XXX we need to clip RTF text differently. Ideas;
       *    - only process if all text is rtl
       *    - will pango just make it work - or should we 
       *      be removing chars from front of string ?
       *
       */


      len = _clip_some_text (font, width, str, encoding, opts);

      if (!len) { free(str); return 0; }
      
      if ((opts & MB_FONT_RENDER_OPTS_CLIP_TRAIL) && len > 3)
	{
	  /* Avoid having a space before the elipsis */
	  while (len-1 >= 0 && str[len-1] == ' ')
	    len--;

	  want_dots = True;
	}
    }
  else
    {
      /* Horizontal alignment - only done if text fits */
      if (opts & MB_FONT_RENDER_ALIGN_CENTER)
	x += (width - render_w)/2;
      else if (opts & MB_FONT_RENDER_ALIGN_RIGHT)
	x += (width - render_w);
    }

  if ((opts & MB_FONT_RENDER_OPTS_CLIP_TRAIL) && want_dots)
    {
      str[len] = '.'; str[len+1] = '.'; str[len+2] = '.'; str[len+3] = '\0'; 

      len += 3; 
      /*
      render_w = mb_font_get_txt_width(font, str, len, encoding);
      _render_some_text (font, drw, x+render_w, y, "..", 2, encoding); 
      */
    }

  if (opts & MB_FONT_RENDER_EFFECT_SHADOW || font->have_shadow)
    {
      unsigned char r,g,b,a;
      mb_col_get_rgba (font->col, &r, &g, &b, &a);
      mb_col_set (font->col, "black");

      _render_some_text (font, drw, x+1, y+1, str, len, encoding); 

      mb_col_set_rgba (font->col, r, g, b, a);
    }

  _render_some_text (font, drw, x, y, str, len, encoding); 
  
  free(str);
  
  return len;
}

int
mb_font_render_simple_get_width (MBFont          *font, 
				 int              width,
				 unsigned char   *text,
				 int              encoding,
				 MBFontRenderOpts opts )
{
  int render_w = 0, len = 0, orig_len = 0;
  unsigned char *str = NULL;
  Bool want_dots = False;

  if (text == NULL) return 0;

  if (!_mb_font_is_font_object_fresh (font))
    _mb_font_load(font);

  orig_len = len = strlen((char*)text);

  str = malloc(len+3);
  memset(str, 0, len+3);

  strcpy((char*)str, (char*)text);

  render_w = mb_font_get_txt_width(font, str, len, encoding);

  if (render_w > width)
    {
      len = _clip_some_text (font, width, str, encoding, opts);

      if (!len) { free(str); return 0; }
      
      if ((opts & MB_FONT_RENDER_OPTS_CLIP_TRAIL) && len > 3)
	  want_dots = True;
    }
  
  if ((opts & MB_FONT_RENDER_OPTS_CLIP_TRAIL) && want_dots)
    {
      str[len] = '.'; str[len+1] = '.'; str[len+2] = '.'; str[len+3] = '\0'; 

      len += 3; 
    }

  render_w = mb_font_get_txt_width(font, str, len, encoding);
  
  free(str);
  
  return render_w;
}


/* 
   Layout defualt to drawable size. 
 */

MBLayout *
mb_layout_new ()
{
  MBLayout *layout = NULL;
  
  layout = malloc(sizeof(MBLayout));
  memset(layout, 0, sizeof(MBLayout));

  return layout;
}


/* If layout not set size is autocalculated */
void
mb_layout_set_geometry(MBLayout *layout, int width, int height)
{
  layout->width  = width;
  layout->height = height;

  layout->_have_autocalc_size = False;
}

void
mb_layout_get_geometry (MBLayout *layout, int *width, int *height)
{
  unsigned char *txt = layout->txt, *start = NULL;
  int nbytes = 0, line_width = 0;

  if (txt == NULL || layout->width != 0 || layout->height != 0)
    goto end;

  layout->_have_autocalc_size = True;

   while( *txt != '\0' )
   {
     nbytes = 0;
     start  = txt;

     while( *txt != '\n' && *txt != '\0' ) 
       if (layout->txt_encoding == MB_ENCODING_UTF8)
	 { 
	   nbytes += mb_util_next_utf8_char(&txt); 
	 }
       else
	 { 
	   nbytes++; *txt++; 
	 }
      
     line_width = mb_font_get_txt_width(layout->font, start, 
					nbytes, layout->txt_encoding);


     if (line_width > layout->width)
       layout->width = line_width;

     layout->height += mb_font_get_height(layout->font) + layout->line_spacing;
     
     if (*txt == '\n') txt++;
   }

 end:
   *width = layout->width; *height = layout->height;
}

/* call set text, then get geometry for the space it takes up */
void
mb_layout_set_text(MBLayout       *layout, 
		   unsigned char  *text, 
		   MBEncoding      encoding)
{
  if (layout->txt) free(layout->txt);

  layout->txt = (unsigned char*)strdup((char*)text);
  layout->txt_encoding = encoding;
}

void
mb_layout_unref (MBLayout *layout)
{
  if (layout->txt) free(layout->txt);
  if (layout->font) mb_font_unref(layout->font);

  free(layout);
}

void
mb_layout_set_align (MBLayout *layout, int horizontal, int vertical);

void 	    /* dots, word, letter, none  */
mb_layout_set_clip_style (MBLayout *layout, int clip_stype);

void
mb_layout_set_line_spacing(MBLayout *layout, int pixels)
{
  layout->line_spacing = pixels;
}

int
mb_layout_get_line_spacing(MBLayout *layout)
{
  return layout->line_spacing;
}

void
mb_layout_set_font(MBLayout *layout, MBFont *font)
{
  if (layout->font) mb_font_unref(layout->font);
  layout->font = font;
  mb_font_ref(layout->font);
}

static int
_mb_layout_render_magic (MBLayout        *layout, 
			 MBDrawable      *drw, 
			 int              x, 
			 int              y,
			 MBFontRenderOpts opts,
			 Bool             do_render)
{
  unsigned char *orig_p, *p = (unsigned char*)strdup((char*)layout->txt);
  unsigned char *q = p;
  unsigned char *backtrack = NULL;
  int            v_offset  = 0;
  int            cur_width = 0;
  
  orig_p = p;

  while (*p != '\0')
    {                       /* Parse text till we hit a space */
      if (isspace(*p) || *(p+1) == '\0')
	{
	  Bool is_end = False;
	  
	  if (*(p+1) == '\0') /* Are we at the end of the text ? */
	    {
	      is_end = True;
	    }
	  else *p = '\0';
	  
	  /* XXX q should be current_line_start */

	  cur_width = mb_font_get_txt_width(layout->font, q, strlen((char*)q), 
					    layout->txt_encoding) ;
	  
	  if (cur_width > layout->width )
	    {
	      if (backtrack != NULL )
		{
		  /* Goto backtrack and render */
		  *backtrack = '\0';
		  p = backtrack + 1;
		  cur_width = 0;
		}
	      else if ( cur_width > layout->width )
		{
		  /* We cant backtrack to previous word, so we just clip
		     the text.
		  */
		  *p = '\0';
		}
	    }
	  else
	    {
	      /* Can render anything yet. Just store the backtrack 
		 position and carry on
	      */
	      if (!is_end)
		{
		  *p = ' ';
		  backtrack = p;
		  p++;
		  continue;
		}
	    }
	  
	  /* Now do the actual rendering */

	  if ((v_offset + mb_font_get_height(layout->font) + layout->line_spacing) > layout->height)
	    break; 		/* no vertical room left */

	  if (do_render)
	    {
	      mb_font_render_simple (layout->font,
				     drw, 
				     x,
				     y + v_offset,
				     layout->width,
				     q,
				     layout->txt_encoding,
				     opts );
	    }

	  v_offset += mb_font_get_height(layout->font) + layout->line_spacing;

	  q = p;
	  backtrack = NULL;
	}
      p++;
    }

  free(orig_p);

  return v_offset;
}


void
mb_layout_render (MBLayout        *layout, 
		  MBDrawable      *drw, 
		  int              x, 
		  int              y,
		  MBFontRenderOpts opts)
{
  if (!layout->font)   return;
  if (!layout->txt)   return;
  if (!layout->width)  return;
  if (!layout->height) return;

  if (layout->_have_autocalc_size) /* Easy case */
    {
      char *str = strdup((char*)layout->txt), *start = NULL, *orig = NULL;

      orig = str;

      while( *str != '\0' )
	{
	  start  = str;

	  while( *str != '\n' && *str != '\0' ) str++;

	  if (*str == '\n') 
	    {
	      *str = '\0';
	      str++;
	    }

	  mb_font_render_simple (layout->font,
				 drw, 
		                 x,
		                 y,
				 layout->width,
				 (unsigned char*)start,
		                 layout->txt_encoding,
		                 0 );
	  
	  y += mb_font_get_height(layout->font) + layout->line_spacing;
	  
	}
      
      free(orig);
      
    }
  else
    {
      int   v_offset  = 0;

      if (opts & MB_FONT_RENDER_VALIGN_MIDDLE)
	{
	  int rendered_height = _mb_layout_render_magic (layout, drw, x, y, 
							 opts, False); 

	  v_offset = (layout->height - rendered_height)/2;
	}

      _mb_layout_render_magic (layout, drw, x, y + v_offset, opts, True); 
    }
}

void 				/* Useful ? */
mb_layout_render_to_pixbuf (MBLayout *layout, MBPixbufImage *img );


MBDrawable*
mb_drawable_new(MBPixbuf *pb, int width, int height)
{
  MBDrawable *drw = NULL;
  drw = malloc(sizeof(MBDrawable));
  memset(drw, 0, sizeof(MBDrawable));

  drw->pb     = pb;
  drw->width  = width;
  drw->height = height;

  drw->xpixmap = XCreatePixmap(pb->dpy, pb->root, width, height, pb->depth);
  XFillRectangle(pb->dpy, drw->xpixmap, pb->gc, 0, 0, width, height);

   /* todo check for error if pixmap cant be created */
#if defined (USE_XFT) || defined (USE_PANGO)
  drw->xftdraw = XftDrawCreate(pb->dpy, (Drawable) drw->xpixmap, 
			       pb->vis,
			       DefaultColormap(pb->dpy, pb->scr));
#endif
   
   return drw;
}

MBDrawable*
mb_drawable_new_from_pixmap(MBPixbuf *pb, Pixmap pxm)
{
  Window      dummy;
  int          x, y;
  unsigned int width, height, border, depth;
  MBDrawable  *drw = NULL;

  drw = malloc(sizeof(MBDrawable));
  memset(drw, 0, sizeof(MBDrawable));

  XGetGeometry(pb->dpy, (Drawable)pxm, &dummy, &x, &y, 
	       &width, &height, &border, &depth);

  drw->pb      = pb;
  drw->width   = width;
  drw->height  = height;
  drw->xpixmap = pxm;

  drw->have_ext_pxm = True; 	/* Means we dont free the pixmap */

#if defined (USE_XFT) || defined (USE_PANGO)
  drw->xftdraw = XftDrawCreate(pb->dpy, (Drawable) drw->xpixmap, 
			       DefaultVisual(pb->dpy, pb->scr),
			       DefaultColormap(pb->dpy, pb->scr));
#endif
  return drw;
}

void
mb_drawable_unref(MBDrawable* drw)
{
  if (drw->xpixmap != None && !drw->have_ext_pxm)  
    XFreePixmap(drw->pb->dpy, drw->xpixmap);
#if defined (USE_XFT) || defined (USE_PANGO)
  if (drw->xftdraw != NULL)  XftDrawDestroy(drw->xftdraw);
#endif
  free(drw);
}

/* New misc code  */
int
mb_util_utf8_len()
{
  /* FcBool FcUtf8Len (FcChar8 *src, int len, int *nchar, int *wchar); */
  return 0;
}


int 				/* XXX this can be much smaller see glib */
mb_util_next_utf8_char(unsigned char **string)
{
  unsigned char *s, mask;
  int length;
  
  s = *string;

  if((*s & 0x80) == 0) {
    *string = s + 1; return 1;
  }
  
  if((*s & 0xc0) == 0x80) { /* 10xxxxxx is invalid as initial */
    return -1;
  }
  
  if((*s & 0xe0) == 0xc0) { /* 110xxxxx followed by one trailer */
    mask = 0x1f; length = 1;
  } else if ((*s & 0xf0) == 0xe0) { 
    /* 1110xxxx followed by two trailers */     
    mask = 0x0f; length = 2;
  } else if ((*s & 0xf8) == 0xf0) { 
    /* 11110xxx followed by three trailers */     
    mask = 0x07; length = 3;
  } else if((*s & 0xfc) == 0xf8) { 
    /* 111110xx followed by four trailers */     
    mask = 0x03; length = 4;
  } else if((*s * 0xfe) == 0xfc) { 
    /* 1111110x followed by five trailers */     
    mask = 0x01; length = 5;
  } else { /* 1111111x is invalid in UTF8 */     
    return -1;
  }
	    
  *s++;
  while(length-- > 0) {
    if((*s & 0xc0) != 0x80) { /* trailer must be 10xxxxxx */
      /* ERROR */ 
      return -1;
    }
    *s++;
  } 
  
  *string = s; 
  return length;

}
