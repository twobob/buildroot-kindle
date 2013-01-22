#include "mbdesktop_view.h"

static void
mbdesktop_view_paint_items(MBDesktop *mb, MBPixbufImage *img_dest);

static void
_set_win_title(MBDesktop *mb, unsigned char *title)
{
  XChangeProperty(mbdesktop_xdisplay(mb), mb->win_top_level, 
		  mb->window_utf8_name_atom, 
		  mb->utf8_atom, 8, PropModeReplace, 
		  title, strlen(title));

  XStoreName(mb->dpy, mb->win_top_level, title );
}

void
mbdesktop_view_init_bg(MBDesktop *mb)
{
  MBPixbufImage *img_tmp;
  int dw, dh, dx, dy, r, g, b;

  mb->font_col_type = DKTP_FONT_COL_UNKOWN;

  if (mb->bg_img != NULL)
    mb_pixbuf_img_free(mb->pixbuf, mb->bg_img);

  switch (mb->bg->type)
    {
    case BG_SOLID:
      mb->bg_img = mb_pixbuf_img_rgba_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);
      mb_pixbuf_img_fill(mb->pixbuf, mb->bg_img, 
			 mb->bg->data.cols[0], 
			 mb->bg->data.cols[1],
			 mb->bg->data.cols[2], 
			 0);

      mb->use_text_outline = False;

      if (!mb->user_overide_font_col)
	{
	  if ((((mb->bg->data.cols[0] * 54) + (mb->bg->data.cols[1] * 183) + (mb->bg->data.cols[2] * 19)) / 256) > 127 )
	    {
	      mbdesktop_set_font_color(mb, "black");
	      mb->font_col_type = DKTP_FONT_COL_BLACK;
	    } else {
	      mbdesktop_set_font_color(mb, "white");
	      mb->font_col_type = DKTP_FONT_COL_WHITE;
	    }
	}

      break;
    case BG_TILED_PXM:
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);

      if ((img_tmp = mb_pixbuf_img_new_from_file(mb->pixbuf, 
						 mb->bg->data.filename)) 
	  == NULL)
	{
	  fprintf(stderr,"Failed to load background : %s", 
		  mb->bg->data.filename);
	  mbdesktop_bg_parse_spec(mb, "col-solid:red");
	  mbdesktop_view_init_bg(mb);
	  return;
	}

      for (dy=0; dy < mb->desktop_height; dy += img_tmp->height)
	for (dx=0; dx < mb->desktop_width; dx += img_tmp->width)
	  {
	    if ( (dx + img_tmp->width) > mb->desktop_width )
	      dw = img_tmp->width - ((dx + img_tmp->width)-mb->desktop_width);
	    else
	      dw = img_tmp->width;

	    if ( (dy + img_tmp->height) > mb->desktop_height )
	      dh = img_tmp->height-((dy + img_tmp->height)-mb->desktop_height);
	    else
	      dh = img_tmp->height;
	    mb_pixbuf_img_copy(mb->pixbuf, mb->bg_img, img_tmp,
			       0, 0, dw, dh, dx, dy);
	  }
      mb_pixbuf_img_free(mb->pixbuf, img_tmp);

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_STRETCHED_PXM:
      if ((img_tmp = mb_pixbuf_img_new_from_file(mb->pixbuf, 
						 mb->bg->data.filename)) 
	  == NULL)
	{
	  fprintf(stderr,"Failed to load background : %s", mb->bg->data.filename);
	  mbdesktop_bg_parse_spec(mb, "col-solid:red");
	  mbdesktop_view_init_bg(mb);
	  return;
	}
      mb->bg_img = mb_pixbuf_img_scale(mb->pixbuf, img_tmp, mb->desktop_width, 
				       mb->desktop_height);
      mb_pixbuf_img_free(mb->pixbuf, img_tmp);

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_CENTERED_PXM:
      if ((img_tmp = mb_pixbuf_img_new_from_file(mb->pixbuf, 
						 mb->bg->data.filename)) 
	  == NULL)
	{
	  fprintf(stderr,"Failed to load background : %s", mb->bg->data.filename);
	  mbdesktop_bg_parse_spec(mb, "col-solid:red");
	  mbdesktop_view_init_bg(mb);
	  return;
	}
      dx = (mb->desktop_width - img_tmp->width) / 2;
      if (dx < 0) dx = 0;
      dy = (mb->desktop_height - img_tmp->height) / 2;
      if (dy < 0) dy = 0;
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
				     mb->desktop_height);
      mb_pixbuf_img_copy(mb->pixbuf, mb->bg_img, img_tmp,
			 0, 0,
			 (img_tmp->width > mb->desktop_width) ?
			 mb->desktop_width : img_tmp->width ,
			 (img_tmp->height > mb->desktop_height) ?
			 mb->desktop_height : img_tmp->height ,
			 dx, dy);
      mb_pixbuf_img_free(mb->pixbuf, img_tmp);

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_GRADIENT_HORIZ:
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);
      dw = mb->desktop_width;
      dh = mb->desktop_height;
      
      for(dx=0; dx<dw; dx++)
	{
	  r = mb->bg->data.gcols[0] + (( dx * (mb->bg->data.gcols[1] - mb->bg->data.gcols[0])) / dw); 
	  g = mb->bg->data.gcols[2] + (( dx * (mb->bg->data.gcols[3] - mb->bg->data.gcols[2])) / dw); 
	  b = mb->bg->data.gcols[4] + (( dx * (mb->bg->data.gcols[5] - mb->bg->data.gcols[4])) / dw); 

	  for(dy=0; dy<dh; dy++)
	    {
	      mb_pixbuf_img_plot_pixel (mb->pixbuf, mb->bg_img, 
					dx, dy, r ,g, b);
	    }
	}

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    case BG_GRADIENT_VERT:
      mb->bg_img = mb_pixbuf_img_rgb_new(mb->pixbuf, mb->desktop_width, 
					 mb->desktop_height);

      dw = mb->desktop_width;
      dh = mb->desktop_height;
      
      for(dy=0; dy<dh; dy++)
	{
	  r = mb->bg->data.gcols[0] + (( dy * (mb->bg->data.gcols[1] - mb->bg->data.gcols[0])) / dh); 
	  g = mb->bg->data.gcols[2] + (( dy * (mb->bg->data.gcols[3] - mb->bg->data.gcols[2])) / dh); 
	  b = mb->bg->data.gcols[4] + (( dy * (mb->bg->data.gcols[5] - mb->bg->data.gcols[4])) / dh); 

	  for(dx=0; dx<dw; dx++)
	    {
	      mb_pixbuf_img_plot_pixel (mb->pixbuf, mb->bg_img, 
					dx, dy, r ,g, b);
	    }
	}

      if (!mb->user_overide_font_outline) mb->use_text_outline = True;
      if (!mb->user_overide_font_col) mbdesktop_set_font_color(mb, "white");

      break;
    }

  /* Now set the background */
  mbdesktop_view_set_root_pixmap(mb, mb->bg_img);

  /* and figure out what font is best */

}

void
mbdesktop_view_set_root_pixmap(MBDesktop *mb, MBPixbufImage *img)
{
  Atom atom_root_pixmap_id = XInternAtom(mb->dpy, "_XROOTPMAP_ID", False); 
  
  if (mb->root_pxm != None) XFreePixmap(mb->dpy, mb->root_pxm);
  
  mb->root_pxm = XCreatePixmap(mb->dpy, mb->root, 
			       img->width, img->height,
			       mb->pixbuf->depth ); 

  mb_pixbuf_img_render_to_drawable(mb->pixbuf, img, (Drawable)mb->root_pxm,
				   0, 0);

  XChangeProperty(mb->dpy, mb->root, atom_root_pixmap_id, XA_PIXMAP, 
		  32, PropModeReplace, (unsigned char *) &mb->root_pxm, 1);
}


void  /* called only when desktop/workarea changes size or bgimage changes */
mbdesktop_view_configure(MBDesktop *mb)
{
  /* Assume mb->desktop_width, etc is up to date */

  int x,y,w,h;

  /* This is a bit crap - probably already called */
  if (mbdesktop_get_workarea(mb, &x, &y, &w, &h))
    {
      mb->workarea_x = x; 
      mb->workarea_y = y ;
      mb->workarea_width = w;
      mb->workarea_height = h;
    }

  if (mb->workarea_width > mb->desktop_width
      || mb->workarea_height > mb->desktop_height)
    return; 			/* Abort - probably in mid rotation */

  mbdesktop_view_init_bg(mb); 	   /* reset the background */
  mbdesktop_view_paint(mb, False); /* repaint */

}

void
mbdesktop_view_advance(MBDesktop *mb, int n_items)
{
  /* 
     XXX NOT USED YET - JUST EXPERIMENTAL XXXX

  */

  int n = 0;

  /* 
     rendering starts from    :: mb->scroll_offset_item
     highlighted item is      :: mb->kbd_focus_item
     last visble item in view :: mb->last_visible_item
  */

  if (n_items > ( mb->current_view_columns * mb->current_view_rows ))
    {
      /* Means we expecting to scroll the view */

      /* if keyboard focus and advanced key focus position is 
       * pass last viewable item then we also need to scroll.
       */
    }

  while (mb->kbd_focus_item->item_next_sibling && n <= n_items)
    {
      if (mb->kbd_focus_item == mb->last_visible_item
	  && mb->kbd_focus_item->item_next_sibling != NULL)
	{
	  /* we will need to scroll  */
	}

      mb->kbd_focus_item = mb->kbd_focus_item->item_next_sibling;

      n++;
    }

  /* 
     if (we_need_to_scroll)
     {
       rows_to_scroll = 

     }
  */

}

void
mbdesktop_view_retreat(MBDesktop *mb, int n_items)
{

}


void  /* TODO: implement multiple views */
mbdesktop_set_view(MBDesktop *mb, int view)
{
  ;
}

static void 
mbdesktop_view_header_paint(MBDesktop *mb, unsigned char* folder_title)
{
  int opts              = MB_FONT_RENDER_OPTS_CLIP_TRAIL;
  int extra_render_opts = 0;

  if (!mb->use_title_header)
    {
      _set_win_title(mb, folder_title);
      return;
    }

  XSetLineAttributes(mb->dpy, mb->gc, 1, LineSolid, CapRound, JoinRound);

  if (mb->use_text_outline)
    extra_render_opts = MB_FONT_RENDER_EFFECT_SHADOW;

  mb_font_render_simple (mb->titlefont,
			 mb->backing_cache,
			 mb->workarea_x + ((48-32)/2) + 1,
			 mb->workarea_y + 1,
			 (mb->workarea_width - (48-32)),
			 (unsigned char *)folder_title,
			 MB_ENCODING_UTF8,
			 opts|extra_render_opts);

   XDrawLine(mb->dpy, mb_drawable_pixmap(mb->backing_cache), 
	     mb->gc, 
	     mb->workarea_x + ((48-32)/2),
	     mb->workarea_y + mb_font_get_height(mb->titlefont) + 2,
	     mb->workarea_x + mb->workarea_width - ((48-32)/2),
	     mb->workarea_y + mb_font_get_height(mb->titlefont) + 2);

  XSetLineAttributes(mb->dpy, mb->gc, 1, LineOnOffDash, CapRound, JoinRound);

}


void 
mbdesktop_view_paint(MBDesktop *mb, Bool use_cache)
{
  MBPixbufImage *img_dest;
  char *folder_title = "Home";

  if (use_cache && mb->backing_cache != NULL)
    {
      if (mb->had_kbd_input && mb->kbd_focus_item)
	{
	  if (mb->have_focus)
	    mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, 
					   HIGHLIGHT_OUTLINE); 
	}
      return;
    }

  if (mb->backing_cache != NULL)
    mb_drawable_unref(mb->backing_cache);

  mb->backing_cache = mb_drawable_new(mb->pixbuf, 
				      mb->desktop_width, mb->desktop_height); 

  img_dest = mb_pixbuf_img_clone(mb->pixbuf, mb->bg_img);

  mbdesktop_calculate_item_dimentions(mb);

  /* no items to paint - current item is very top - no items loaded */
  if (mb->current_head_item == mb->top_head_item)
    {
      mb_pixbuf_img_render_to_drawable(mb->pixbuf, img_dest, 
				       mb_drawable_pixmap(mb->backing_cache), 
				       0, 0);
    }
  else
    {
      if ( mbdesktop_current_folder_view (mb) == VIEW_ICONS 
	   || mbdesktop_current_folder_view (mb) == VIEW_ICONS_ONLY)
	mbdesktop_view_paint_items(mb, img_dest);
      else
	mbdesktop_view_paint_list(mb, img_dest);
    }
     
  if (mb->current_head_item->item_parent)
    folder_title 
      = (mb->current_head_item->item_parent->name_extended) ? strdup(mb->current_head_item->item_parent->name_extended) : strdup(mb->current_head_item->item_parent->name);
  else 
    folder_title 
      = (mb->top_head_item->name_extended) ? strdup(mb->top_head_item->name_extended) : strdup(mb->top_head_item->name);

  mbdesktop_view_header_paint(mb, folder_title);

  XSetWindowBackgroundPixmap(mb->dpy, mb->win_top_level, 
			     mb_drawable_pixmap(mb->backing_cache));
  XClearWindow(mb->dpy, mb->win_top_level);

  if (mb->kbd_focus_item && mb->had_kbd_input)
    {
      if (mb->have_focus)
	mbdesktop_view_item_highlight (mb, mb->kbd_focus_item, 
				       HIGHLIGHT_OUTLINE); 
    }

  if (img_dest)
    mb_pixbuf_img_free(mb->pixbuf, img_dest);  

  if (folder_title)
    free(folder_title);
}

void
mbdesktop_view_paint_list(MBDesktop *mb, MBPixbufImage *dest_img)
{
  MBDesktopItem *item;
  MBPixbufImage *icon_img_small;
  MBLayout *layout = NULL;

  int cur_y, cur_x, limit_y;


  if (mb->scroll_offset_item  == mb->current_head_item)
    mb->scroll_active = False;
  else
    mb->scroll_active = True;

  cur_x = mb->workarea_x + ((48-32)/2);
  cur_y = mb->workarea_y + mb->title_offset; /* + mb->win_plugin_rect.height; */
  limit_y = mb->workarea_y + mb->workarea_height - mb->title_offset; /*  - mb->win_plugin_rect.height; */

  mb->current_view_columns = 1;
  mb->current_view_rows 
    = ( mb->workarea_height - mb->title_offset /* - mb->win_plugin_rect.height */) / mb->icon_size ;


  for(item = mb->scroll_offset_item; 
      item != NULL; 
      item = item->item_next_sibling)
    {
      if (item->icon)
	{
	  if ( (cur_y + mb->item_height ) > limit_y) /* Off display ? */
	    {
	      mb->scroll_active = True;
	      break;
	    }
	  
	  if (mbdesktop_current_folder_view ( mb ) != VIEW_TEXT_ONLY)
	    {
	      
	      /* Arg, we shouldn't scale *every* render ! */
	      icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, 
						   item->icon,
						   mb->icon_size, 
						   mb->icon_size);
	      
	      mb_pixbuf_img_composite(mb->pixbuf, dest_img, 
				      icon_img_small, 
				      cur_x, 
				      cur_y );
	      
	      mb_pixbuf_img_free(mb->pixbuf, icon_img_small);
	    }
	  
	  item->x = cur_x;
	  item->y = cur_y;

	  item->width  = mb->workarea_width - ((48-32));
	  item->height = mb->item_height;

	  cur_y += mb->item_height;
	}
    }
  

  if (mb->scroll_active)
    {
      mb_pixbuf_img_composite(mb->pixbuf, dest_img, mb->img_scroll_up,
			      mb->workarea_x + mb->workarea_width-24, 
			      mb->workarea_y + 2);
      
      mb_pixbuf_img_composite(mb->pixbuf, dest_img,
			      mb->img_scroll_down, 
			      mb->workarea_x + mb->workarea_width-40, 
			      mb->workarea_y + 2);
    }


  mb_pixbuf_img_render_to_drawable(mb->pixbuf, dest_img, 
				   mb_drawable_pixmap(mb->backing_cache), 
				   0, 0);

  mb->last_visible_item = item;

  layout = mb_layout_new();

  mb_layout_set_font(layout, mb->font);


  for(item = mb->scroll_offset_item; 
      item != NULL; 
      item = item->item_next_sibling)
    {
      int offset_y  = 0;
      int offset_x  = 0;
      int extra_render_opt = 0;

      if (mbdesktop_current_folder_view ( mb ) != VIEW_TEXT_ONLY)
	{
	  offset_x = mb->icon_size + mb->icon_size/4;
	  offset_y = 0;
	}
      else
	offset_x = 0;

      if (offset_y < 0) offset_y = 0;
      
      if (item == mb->last_visible_item)
	break;
      
      if (mb->use_text_outline)
	extra_render_opt = MB_FONT_RENDER_EFFECT_SHADOW;

      mb_layout_set_geometry(layout, 
			     item->width - mb->icon_size - mb->icon_size/8, 
			     item->height);

      mb_layout_set_text(layout, item->name, MB_ENCODING_UTF8);

      mb_layout_render(layout, mb->backing_cache, 
		       item->x + offset_x,
		       item->y + offset_y,
		       extra_render_opt|MB_FONT_RENDER_OPTS_CLIP_TRAIL|MB_FONT_RENDER_VALIGN_MIDDLE);

    }

  mb_layout_unref(layout);

}


static void
mbdesktop_view_paint_items(MBDesktop *mb, MBPixbufImage *img_dest)
{
  MBDesktopItem *item;
  MBPixbufImage *icon_img_small;

  int cur_x = 0, cur_y = 0, limit_x, limit_y, cur_row = 1;
  int item_horiz_border = (mb->item_width-(mb->icon_size))/2;


  if (mb->scroll_offset_item  == mb->current_head_item)
    mb->scroll_active = False;
  else
    mb->scroll_active = True;

  cur_x = mb->workarea_x;
  cur_y = mb->workarea_y + mb->title_offset; /* + mb->win_plugin_rect.height; */

  limit_x = mb->workarea_x + mb->workarea_width;
  limit_y = mb->workarea_y + mb->workarea_height - mb->title_offset; /* - mb->win_plugin_rect.height; */

  mb->current_view_columns = mb->workarea_width  / mb->item_width;
  mb->current_view_rows  
    = ( mb->workarea_height - mb->title_offset /* - mb->win_plugin_rect.height */) / mb->item_height;


  for(item = mb->scroll_offset_item; 
      item != NULL; 
      item = item->item_next_sibling)
    {
      if (item->type == ITEM_TYPE_MODULE_WINDOW)
	{
	  /* 
	     May need to 'newline' thigs here.

	  */

	    item->x      = mb->workarea_x;
	    item->y      = cur_y;
	    item->width  = mb->workarea_width;
	    /* item->height stays the same
	    
	    mbdesktop_xembed_configure (mb, item);

	    */

	    cur_x = mb->workarea_x;
	    cur_y += mb->item_height;


	    /*
	     Its a window. 
	       - Move it ?
	       - Is it offscreen ?
	       - resize its width if changed ?
	       - send it an expose ?
	       - cur_y + window height. 
	    */
	}
      else
	{
	  if ((cur_x + mb->item_width) > limit_x) /* 'newline' */
	    {
	      cur_x  = mb->workarea_x;
	      cur_y += mb->item_height;

	      cur_row++;

	      //if ( (cur_y+mb->item_height) > limit_y) /* Off display ? */
	      if (cur_row >   mb->current_view_rows ) 
		{

		  /* 
		     set a flag to turn on scrolling
		  */
		  mb->scroll_active = True;
		  break;
		}
	    }

	  item->x      = cur_x;
	  item->y      = cur_y;
	  item->width  = mb->item_width;
	  item->height = mb->item_height;

	  if (item->icon)
	    {
	      if (mb_pixbuf_img_get_width(item->icon) != mb->icon_size
		  || mb_pixbuf_img_get_height(item->icon) != mb->icon_size)
		{
		  icon_img_small = mb_pixbuf_img_scale(mb->pixbuf, item->icon,
						       mb->icon_size, 
						       mb->icon_size);
		  
		  mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					  icon_img_small, 
					  item->x + item_horiz_border, 
					  item->y );
		  
		  mb_pixbuf_img_free(mb->pixbuf, icon_img_small);
		  
		} else mb_pixbuf_img_composite(mb->pixbuf, img_dest, 
					       item->icon, 
					       item->x + item_horiz_border, 
					   item->y );
	    }
	  
	  cur_x += mb->item_width;
	}
    }

  if (mb->scroll_active)
    {
      mb_pixbuf_img_composite(mb->pixbuf, img_dest, mb->img_scroll_up,
			      mb->workarea_x + mb->workarea_width-24, 
			      mb->workarea_y + 2);
    
      mb_pixbuf_img_composite(mb->pixbuf, img_dest, mb->img_scroll_down, 
			      mb->workarea_x + mb->workarea_width-40, 
			      mb->workarea_y + 2);

    }


  mb_pixbuf_img_render_to_drawable(mb->pixbuf, img_dest, 
				   mb_drawable_pixmap(mb->backing_cache), 
				   0, 0);

  mb->last_visible_item = item;


  if (mbdesktop_current_folder_view (mb) != VIEW_ICONS_ONLY)
    {
      /* 2nd pass render text */
      MBLayout *layout = NULL;

      layout = mb_layout_new();

      mb_layout_set_font(layout, mb->font);

      for(item = mb->scroll_offset_item; 
	  (item != NULL && item != mb->last_visible_item); 
	  item = item->item_next_sibling)
	{
	  int extra_render_opt = 0;

	  mb_layout_set_geometry(layout, item->width, 600);

	  mb_layout_set_text(layout, item->name, MB_ENCODING_UTF8);

	  if (mb->use_text_outline)
	    extra_render_opt = MB_FONT_RENDER_EFFECT_SHADOW;

	  mb_layout_render(layout, mb->backing_cache, 
			   item->x, item->y + mb->icon_size,
			   MB_FONT_RENDER_OPTS_CLIP_TRAIL
			   |MB_FONT_RENDER_ALIGN_CENTER|extra_render_opt );
	}


      mb_layout_unref(layout);
    }

}

void
mbdesktop_view_item_highlight (MBDesktop     *mb, 
			       MBDesktopItem *item,
			       int            highlight_style)
{
  MBPixbufImage    *img_cache = NULL;
  MBDrawable       *pxm;
  MBLayout         *layout;
  MBFontRenderOpts  opts = 0;

  int text_y_offset = 0;
  int text_x_offset = 0;
  int cur_view = 0;
  int x = 0, y = 0, w = 0, h = 0, xx = 0, yy = 0;
  unsigned char r,g,b;

  cur_view = mbdesktop_current_folder_view(mb);

  switch (cur_view)
    {
    case VIEW_ICONS:
      x = item->x;
      y = item->y      + mb->icon_size;
      w = item->width;
      h = item->height - mb->icon_size;
      opts = MB_FONT_RENDER_OPTS_CLIP_TRAIL|MB_FONT_RENDER_ALIGN_CENTER;
      text_x_offset = 0;
      break;

    case VIEW_ICONS_ONLY:
    case VIEW_TEXT_ONLY:
      x = item->x;
      y = item->y;
      w = item->width;
      h = item->height;
      opts = MB_FONT_RENDER_OPTS_CLIP_TRAIL;
      break;

    case VIEW_LIST:
      x = item->x + mb->icon_size + mb->icon_size/8;
      text_x_offset = mb->icon_size/8;
      y = item->y;
      w = item->width - mb->icon_size - mb->icon_size/8;
      h = item->height;

      /*
      text_y_offset = (mb->icon_size - (mb_font_get_height(mb->font)))/2;
      if (text_y_offset < 0) text_y_offset = 0;
      */
      
      text_y_offset = 0; 

      opts = MB_FONT_RENDER_OPTS_CLIP_TRAIL|MB_FONT_RENDER_VALIGN_MIDDLE;
      break;
    }
  
  switch (highlight_style)
    {
    case HIGHLIGHT_OUTLINE:
    case HIGHLIGHT_FILL:

      /* XXX was draw 
      XFillRectangle(mb->dpy, mb->win_top_level, mb->invert_gc, x, y, w, h);
      */
      
      r = mb_col_red(mb->hl_col);
      g = mb_col_green(mb->hl_col);
      b = mb_col_blue(mb->hl_col);

      img_cache = mb_pixbuf_img_rgba_new(mb->pixbuf, w, h);
      
      mb_pixbuf_img_copy (mb->pixbuf, img_cache, mb->bg_img,
			  x, y, w, h, 0, 0);

      for ( xx=2; xx < (w - 2); xx++)
	{
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, 0,
				   r, g, b);
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, h-2,
				   r, g, b);
	}
      
      for ( xx=1; xx < (w - 1); xx++)
	{
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, 1,
				   r, g, b);
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, h-3,
				   r, g, b);
	}
      
      for ( xx=0; xx < w; xx++)
	for ( yy=2; yy < h-3; yy++)
	  mb_pixbuf_img_plot_pixel(mb->pixbuf, img_cache, xx, yy,
				   r, g, b);

      pxm = mb_drawable_new(mb->pixbuf, w, h);

      mb_pixbuf_img_render_to_drawable(mb->pixbuf, img_cache, 
				       mb_drawable_pixmap(pxm), 0, 0);

      if (cur_view != VIEW_ICONS_ONLY)
	{
	  if (mb->font_col_type == DKTP_FONT_COL_BLACK)
	    mbdesktop_set_font_color(mb, "white");
	  else  if (mb->font_col_type == DKTP_FONT_COL_WHITE)
	    mbdesktop_set_font_color(mb, "black");
	  else if (mb->use_text_outline)
	    opts |= MB_FONT_RENDER_EFFECT_SHADOW;

	  layout = mb_layout_new();

	  mb_layout_set_font(layout, mb->font);

	  mb_layout_set_geometry(layout, w, h);

	  mb_layout_set_text(layout, item->name, MB_ENCODING_UTF8);

	  mb_layout_render(layout, pxm, 
			   text_x_offset, text_y_offset,
			   opts);

	  mb_layout_unref(layout);

	  if (mb->font_col_type == DKTP_FONT_COL_BLACK)
	    mbdesktop_set_font_color(mb, "black");
	  else  if (mb->font_col_type == DKTP_FONT_COL_WHITE)
	    mbdesktop_set_font_color(mb, "white");
	}
      else 
	{
	  /* 
	     XXX Needs work.. 
	  */
	  mbdesktop_view_header_paint(mb, item->name);
	}

      XCopyArea(mb->dpy, mb_drawable_pixmap(pxm), mb->win_top_level, mb->gc, 0, 0, w, h, x, y);
  
      mb_pixbuf_img_free(mb->pixbuf, img_cache);

      mb_drawable_unref(pxm);

      break;

    case HIGHLIGHT_OUTLINE_CLEAR:
    case HIGHLIGHT_FILL_CLEAR:
      XClearWindow(mb->dpy, mb->win_top_level);
      break;
    }

  XFlush(mb->dpy);
}

