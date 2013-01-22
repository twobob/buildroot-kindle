#include "msg.h"

static MBLayout*
msg_calc_win_size(MBPanel *d, MBPanelMessageQueue *m, int *w, int *h);


/*
static int
_get_text_length(MBPanel *d, char *str, int cnt)
{
#ifdef USE_XFT
   XGlyphInfo extents;
   XftTextExtents8(d->dpy, d->msg_font, (unsigned char *) str,
		   cnt, &extents);
   return extents.width;
#else
   return XTextWidth(d->msg_font, str, cnt);
#endif
}
*/

static unsigned long 
_get_server_time(MBPanel *d)
{
  XEvent xevent;
  unsigned char c = 'a';

  XChangeProperty (d->dpy, d->win_root, 
		   d->atoms[ATOM_MB_DOCK_TIMESTAMP],
		   d->atoms[ATOM_MB_DOCK_TIMESTAMP], 
		   8, PropModeReplace, &c, 1);

  for (;;) {
    XMaskEvent(d->dpy, PropertyChangeMask, &xevent);
    if (xevent.xproperty.atom == d->atoms[ATOM_MB_DOCK_TIMESTAMP])
      {
	  return xevent.xproperty.time;
      }
  }
}

static void
_send_tray_context_message(MBPanel *panel,
			   Window     win)
{
   XEvent ev;
   
   memset(&ev, 0, sizeof(ev));
   ev.xclient.type = ClientMessage;
   ev.xclient.window = win;
   ev.xclient.message_type = panel->atoms[ATOM_MB_SYSTEM_TRAY_CONTEXT];
   ev.xclient.format = 32;
   ev.xclient.data.l[0] = _get_server_time(panel);
   
   XSendEvent(panel->dpy, win, False, NoEventMask, &ev);
   XSync(panel->dpy, False);
}


MBPanelMessageQueue*
msg_new(MBPanel *dock, XClientMessageEvent *e)
{
   MBPanelMessageQueue *m;
   MBPanelApp *sender;

   if ((sender = panel_app_get_from_window(dock, e->window )) == NULL)
      return NULL;

   m = (MBPanelMessageQueue *)malloc(sizeof(MBPanelMessageQueue));
   m->sender = sender;

   m->starttime = e->data.l[0];
   m->timeout   = e->data.l[2];

   m->total_msg_length = e->data.l[3];
   m->id = e->data.l[4];
   m->data = (unsigned char *)malloc(sizeof(unsigned char)
				     *(m->total_msg_length+1));
   m->current_msg_length = 0;
   m->pending = False;
   m->next = NULL;
   
   if (dock->msg_queue_start == NULL)
   {
      DBG("queue is empty\n");
      dock->msg_queue_start = dock->msg_queue_end = m;
   } else {
      DBG("queue is not empty\n");
      dock->msg_queue_end->next = m;
      dock->msg_queue_end = m;
   }
   
   return m;
}

void
msg_destroy(MBPanel *panel, MBPanelMessageQueue *msg)
{
   MBPanelMessageQueue *msg_q;
   if (panel->msg_queue_start == msg)
   {
      panel->msg_queue_start = msg->next;
   } else {
      for(msg_q=panel->msg_queue_start; msg_q->next != NULL; msg_q=msg_q->next)
      {
	 if (msg_q->next == msg)
	 {
	    msg_q->next = msg->next;
	    break;
	 }
      }
   }

   if (msg->extra_context_data) XFree(msg->extra_context_data);
   free(msg->data);
   free(msg);
}

void
msg_add_data(MBPanel *d, XClientMessageEvent *e)
{
   MBPanelMessageQueue *m;

   for(m=d->msg_queue_start; m != NULL; m=m->next)
   {
      if (m->sender->win == e->window)
      {
	 if ( (m->total_msg_length - m->current_msg_length) > 20)
	 {
	    memcpy(&m->data[m->current_msg_length],e->data.b,sizeof(char)*20);
	    m->current_msg_length += 20;
	 } else {
	    memcpy(&m->data[m->current_msg_length],e->data.b,
		   (m->total_msg_length-m->current_msg_length) );
	    m->current_msg_length = m->total_msg_length;
	    m->data[m->total_msg_length] = '\0';
	
	    m->pending = True;
	 }
	 return;
      }
   }
}

void
msg_win_create(MBPanel             *panel,
	       MBPanelMessageQueue *msg)
{
  int msg_win_x = 0, msg_win_y = 0, msg_win_w = 0, msg_win_h = 0,
    box_x_offset = 0, box_y_offset = 0, arrow_offset = 0, cnt = 0,
    box_w, box_h;
  
  int x, y, txt_v_offset;
  unsigned char r, g, b, fr, fg, fb;
  
  MBPixbufImage       *img_backing = NULL;
  MBDrawable          *tmp_drw;

  Pixmap               mask;
  GC                   mask_gc;
  XSetWindowAttributes attr;
  XWindowAttributes    root_attr;
  XWMHints            *wm_hints;
  long                 winmask;
  MBLayout            *layout;

  /*
#ifdef USE_XFT
  XftDraw             *xftdraw;
  XftColor             txt_xftcol;
  XRenderColor         colortmp;
#endif
  */

  layout = msg_calc_win_size(panel, msg, &msg_win_w, &msg_win_h);
  
  msg_win_h += (2*MSG_WIN_Y_PAD);
  
  box_w = msg_win_w; box_h = msg_win_h;

  XGetWindowAttributes(panel->dpy, panel->win_root, &root_attr);
  
#define ARROW_SIZE 4
#define MSG_TEXT_MARGIN 10

  switch (panel->orientation)
    {
    case East:
      msg_win_w    += ARROW_SIZE;
      msg_win_x     = panel->x - msg_win_w - 5;	     
      break;
    case West:
      msg_win_w    += ARROW_SIZE;
      box_x_offset += ARROW_SIZE;
      msg_win_x     = panel->x + panel->w + 5;
      break;
    case South:
      msg_win_h    += ARROW_SIZE;
      msg_win_y     = panel->y - msg_win_h - 5;
      break;
    case North:
      msg_win_h    += ARROW_SIZE;
      box_y_offset += ARROW_SIZE;
      msg_win_y     = panel->y + panel->h + 5;
      break;
    }

  if (PANEL_IS_VERTICAL(panel))
    {
      arrow_offset = msg_win_h / 2; 
      msg_win_y    = panel->y + msg->sender->y + (msg->sender->h/2) - arrow_offset;
      
      if (msg_win_y < 0)
	{
	  msg_win_y    = 5; 	/* a little margin to display top */
	  arrow_offset = msg->sender->y + (msg->sender->h/2);
	}
      
      if (msg_win_y + msg_win_h > panel->y + panel->h)
	{
	   /* reposition with a bit of margin - 5px */
	  msg_win_y = (panel->y + panel->h) - msg_win_h - 5;
	  arrow_offset = panel->y + msg->sender->y - msg_win_y + (msg->sender->h/2);
	}
    } 
  else
    {
      arrow_offset = msg_win_w / 2; 
      msg_win_x    = panel->x + msg->sender->x + (msg->sender->w/2) - arrow_offset;
      
      if (msg_win_x < 0)
	{
	  msg_win_x    = 5;
	  arrow_offset = msg->sender->x + (msg->sender->w/2);
	}
      
      if (msg_win_x + msg_win_w > panel->x + panel->w)
	{
	  msg_win_x = (panel->x + panel->w) - msg_win_w - 5;
	  arrow_offset = panel->x + msg->sender->x - msg_win_x + (msg->sender->w/2);
	}
    }
  
  attr.event_mask            = ButtonPressMask|ButtonReleaseMask|ExposureMask;
  attr.background_pixel      = BlackPixel(panel->dpy, panel->screen);
  attr.do_not_propagate_mask = ButtonPressMask|ButtonReleaseMask;
  winmask                    = CWBackPixel|CWEventMask|CWDontPropagate;

  if (panel->use_overide_wins)
    {
      winmask |= CWOverrideRedirect;
      attr.override_redirect = True;
    }
  
  panel->msg_win = XCreateWindow(panel->dpy, panel->win_root,
				 msg_win_x,
				 msg_win_y, 
				 msg_win_w, 
				 msg_win_h, 0,
				 CopyFromParent,
				 CopyFromParent,
				 CopyFromParent,
				 winmask,
				 &attr);
  
  XChangeProperty(panel->dpy, panel->msg_win, 
		  panel->atoms[ATOM_WM_WINDOW_TYPE],
		  XA_ATOM, 32,  PropModeReplace,
		  (unsigned char *)
		  &panel->atoms[ATOM_WM_WINDOW_TYPE_SPLASH], 1);
  
  wm_hints = XAllocWMHints();
  wm_hints->input = False;
  wm_hints->flags = InputHint;
  XSetWMHints(panel->dpy, panel->msg_win, wm_hints);
  
  tmp_drw = mb_drawable_new(panel->pb, msg_win_w, msg_win_h);
  
  img_backing = mb_pixbuf_img_rgba_new(panel->pb, msg_win_w, msg_win_h);
  
  mask = XCreatePixmap(panel->dpy, panel->win_root, msg_win_w, msg_win_h, 1 );
  mask_gc = XCreateGC(panel->dpy, mask, 0, 0 );

  XSetForeground(panel->dpy, mask_gc, WhitePixel( panel->dpy, panel->screen ));
  XFillRectangle(panel->dpy, mask, mask_gc, 0, 0, msg_win_w, msg_win_h );
  XSetForeground(panel->dpy, mask_gc, BlackPixel( panel->dpy, panel->screen ));
  
  /* 
   *  - fill entire block with forground color 
   *  - alpha clear part
   *  - draw arrow part
   *  - curve corners ? 
   */
  
  r = mb_col_red(panel->msg_col);
  g = mb_col_green(panel->msg_col);
  b = mb_col_blue(panel->msg_col);

  fr = mb_col_red(panel->msg_fg_col);
  fg = mb_col_green(panel->msg_fg_col);
  fb = mb_col_blue(panel->msg_fg_col);

  mb_pixbuf_img_fill (panel->pb, img_backing, r, g, b, 255);

  /* border */

  /* top/bottom */
  for (x = box_x_offset; x < box_x_offset + box_w; x++)
    {
      mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, box_y_offset,
			       fr, fg, fb);
      mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, 
			       box_h + box_y_offset - 1,
			       fr, fg, fb);
    }
  
  /* sides */
  for (y = box_y_offset; y < box_y_offset + box_h; y++)
    {
      mb_pixbuf_img_plot_pixel(panel->pb, img_backing, box_x_offset, y, 
			       fr, fg, fb);
      mb_pixbuf_img_plot_pixel(panel->pb, img_backing, 
			       box_w + box_x_offset - 1, y,
			       fr, fg, fb);
    }

  /* arrow */
   
  switch (panel->orientation)
    {
    case East:
      
      for (x= box_w; x < msg_win_w; x++)
	for (y=0; y < box_h; y++)
	  XDrawPoint(panel->dpy, mask, mask_gc, x, y);
      
      XSetForeground( panel->dpy, mask_gc, 
		      WhitePixel( panel->dpy, panel->screen ));
      
      for (x= msg_win_w - 1; x > msg_win_w - ARROW_SIZE - 2 ; x--)
	{
	  for (y = arrow_offset - cnt; y <= arrow_offset + cnt; y++)
	    {
	      XDrawPoint(panel->dpy, mask, mask_gc, x, y);
	      if ( y == (arrow_offset - cnt) || y == (arrow_offset + cnt) )
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, 
					 fr, fg, fb);
	      else
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, r,g,b);
	    }
	  cnt++;
	}
      
      break;
      
    case West:

      for (x=0; x<ARROW_SIZE; x++)
	for (y=0; y < msg_win_h; y++)
	  XDrawPoint(panel->dpy, mask, mask_gc, x, y);
      
      XSetForeground( panel->dpy, mask_gc, 
		      WhitePixel( panel->dpy, panel->screen ));
      
      for (x=0; x<ARROW_SIZE + 1; x++)
	{
	  for (y = arrow_offset - cnt; y <= arrow_offset + cnt; y++)
	    {
	      XDrawPoint(panel->dpy, mask, mask_gc, x, y);
	      if ( y == (arrow_offset - cnt) || y == (arrow_offset + cnt) )
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, 
					 fr, fg, fb);
	      else
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, r,g,b);
	    }
	  cnt++;
	}
      
      break;
      
    case South:

      for (y=msg_win_h-ARROW_SIZE; y < msg_win_h; y++)
	for (x=0; x < msg_win_w; x++)
	  XDrawPoint(panel->dpy, mask, mask_gc, x, y);
      
      XSetForeground( panel->dpy, mask_gc, 
		      WhitePixel( panel->dpy, panel->screen ));
      
      for (y=msg_win_h; y >= msg_win_h - ARROW_SIZE - 1; y--)
	{
	  for (x = arrow_offset - cnt; x <= arrow_offset + cnt; x++)
	    {
	      XDrawPoint(panel->dpy, mask, mask_gc, x, y);
	      if ( x == (arrow_offset - cnt) || x == (arrow_offset + cnt) )
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, 
					 fr, fg, fb);
	      else
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, r,g,b);
	    }
	  cnt++;
	}
      
      break;

    case North:

      for (y=0; y < ARROW_SIZE; y++)
	for (x=0; x < msg_win_w; x++)
	  XDrawPoint(panel->dpy, mask, mask_gc, x, y);

      XSetForeground( panel->dpy, mask_gc, 
		      WhitePixel( panel->dpy, panel->screen ));
      
      for (y=0; y < ARROW_SIZE + 1; y++)
	{
	  for (x = arrow_offset - cnt; x <= arrow_offset + cnt; x++)
	    {
	      XDrawPoint(panel->dpy, mask, mask_gc, x, y);

	      if ( x == (arrow_offset - cnt) || x == (arrow_offset + cnt) )
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, 
					 fr, fg, fb);
	      else
		mb_pixbuf_img_plot_pixel(panel->pb, img_backing, x, y, r,g,b);
	    }

	  cnt++;
	}
      
      break;
      
    }
  
  /* Render picture to pixmap  */
  mb_pixbuf_img_render_to_drawable(panel->pb, img_backing, 
				   mb_drawable_pixmap(tmp_drw),
				   0, 0);
  
  
  mb_font_set_color(panel->msg_font, panel->msg_fg_col);

  txt_v_offset = MSG_WIN_Y_PAD + box_y_offset;
  
  /*
  i = strlen(msg->sender->name);
  while (_get_text_length(panel, msg->sender->name, i) > ( box_w + ( 2 * MSG_TEXT_MARGIN )) 
	 && i > 0)
    i--;
  */

  /* Title */

  XSetForeground(panel->dpy, panel->msg_gc, mb_col_xpixel(panel->msg_fg_col));
		
  mb_font_render_simple (panel->msg_font,
			 tmp_drw,
			 MSG_TEXT_MARGIN + box_x_offset, 
			 txt_v_offset,
			 box_w, /* XXX - TEXT_MARGIN ? */
			 (unsigned char *)msg->sender->name,
			 MB_ENCODING_UTF8,
			 0);

  /* close box */
  
  XDrawRectangle(panel->dpy, mb_drawable_pixmap(tmp_drw), panel->msg_gc, 
		 box_x_offset + box_w - MSG_TEXT_MARGIN - mb_font_get_height(panel->msg_font),   
		 txt_v_offset, 
		 mb_font_get_height(panel->msg_font), 
		 mb_font_get_height(panel->msg_font) );

  XDrawLine(panel->dpy, mb_drawable_pixmap(tmp_drw), panel->msg_gc, 
	    box_x_offset + box_w - MSG_TEXT_MARGIN - mb_font_get_height(panel->msg_font),   
	    txt_v_offset ,
	    box_x_offset + box_w - MSG_TEXT_MARGIN, 
	    txt_v_offset + mb_font_get_height(panel->msg_font));

  XDrawLine(panel->dpy, mb_drawable_pixmap(tmp_drw), panel->msg_gc, 
	    box_x_offset + box_w - MSG_TEXT_MARGIN - mb_font_get_height(panel->msg_font),   
	    txt_v_offset + mb_font_get_height(panel->msg_font),
	    box_x_offset + box_w - MSG_TEXT_MARGIN, 
	    txt_v_offset );

  /* Title underline */

  XDrawLine(panel->dpy, mb_drawable_pixmap(tmp_drw), panel->msg_gc,
	    box_x_offset + MSG_TEXT_MARGIN / 2 , 
	    txt_v_offset + mb_font_get_height(panel->msg_font) + ( MSG_LINE_SPC / 2 ), 
	    box_x_offset + box_w - (MSG_TEXT_MARGIN / 2), 
	    txt_v_offset + mb_font_get_height(panel->msg_font) + ( MSG_LINE_SPC / 2 ) );

  /* Forward render postion on */
  txt_v_offset += mb_font_get_height(panel->msg_font) + MSG_LINE_SPC;

  /* render msg text */
  mb_layout_render (layout, tmp_drw, MSG_TEXT_MARGIN + box_x_offset, txt_v_offset, 0);

  txt_v_offset += mb_layout_height(layout);

  /* Context button, if applicable */
   if (msg->extra_context_data)
     {
       int context_width = mb_font_get_txt_width ( panel->msg_font,  
						   msg->extra_context_data, 
						   strlen((char*)msg->extra_context_data),
						   MB_ENCODING_UTF8);

       XSetForeground( panel->dpy, panel->msg_gc, mb_col_xpixel(panel->msg_link_col));

       panel->msg_context_y1 = txt_v_offset;
       panel->msg_context_y2 = txt_v_offset + mb_font_get_height(panel->msg_font);

       mb_font_set_color(panel->msg_font, panel->msg_link_col);

       mb_font_render_simple (panel->msg_font,
			      tmp_drw,
			      MSG_TEXT_MARGIN + box_x_offset, 
			      txt_v_offset,
			      box_w,
			      (unsigned char *)msg->extra_context_data,
			      MB_ENCODING_UTF8,
			      0);

      /* underline */

      XDrawLine(panel->dpy, mb_drawable_pixmap(tmp_drw), panel->msg_gc, 
		MSG_TEXT_MARGIN + box_x_offset,
		txt_v_offset + mb_font_get_height(panel->msg_font),
		MSG_TEXT_MARGIN + box_x_offset + context_width,
		txt_v_offset + mb_font_get_height(panel->msg_font));
     }

  
  XSetWindowBackgroundPixmap(panel->dpy, panel->msg_win, 
			     mb_drawable_pixmap(tmp_drw));


  mb_drawable_unref(tmp_drw);		
  
  XShapeCombineMask( panel->dpy, panel->msg_win, ShapeBounding, 0, 0, mask, ShapeSet);	 

  XMapWindow(panel->dpy, panel->msg_win);
  XFree(wm_hints);
  
  mb_pixbuf_img_free(panel->pb, img_backing);
  XFreePixmap(panel->dpy, mask);
}

void
msg_cancel (MBPanel *panel, XClientMessageEvent *e)
{
   MBPanelApp *sender;

   if ((sender = panel_app_get_from_window(panel, e->window )) == NULL)
     return;
   
   if (panel->msg_win 
       && panel->msg_win_sender == sender 
       && panel->msg_sender_id == e->data.l[2])
     {
       XDestroyWindow(panel->dpy, panel->msg_win);
       panel->msg_win = None;
     }
}

void
msg_handle_events(MBPanel *panel, XEvent *e)
{
   MBPanelMessageQueue *msg = NULL;

   for(msg=panel->msg_queue_start; msg != NULL; msg=msg->next)
   {
      if (msg->pending)
      {
	 if (panel->msg_win)
	 {
	    XDestroyWindow(panel->dpy, panel->msg_win);
	    panel->msg_win = None;
	 }

	 panel->msg_starttime  = msg->starttime;
	 panel->msg_timeout    = msg->timeout; 
	 panel->msg_win_sender = msg->sender;
	 panel->msg_sender_id  = msg->id;

	 panel->msg_has_context = False;

	 if ((msg->extra_context_data  = util_get_utf8_prop(panel, panel->msg_win_sender->win, panel->atoms[ATOM_MB_SYSTEM_TRAY_CONTEXT])) != NULL)
	     panel->msg_has_context = True;

	 msg_win_create(panel, msg);

	 msg_destroy(panel, msg);

	 return;
      }
   }

   if (panel->msg_win)
   {
     switch (e->type)
       {
       case Expose:
	 break;
       case ButtonRelease:
	 if ( e->xbutton.window == panel->msg_win)
	   {
	     if (panel->msg_has_context 
		 && e->xbutton.y >= panel->msg_context_y1
		 && e->xbutton.y <= panel->msg_context_y2)
	       {
		 _send_tray_context_message(panel, panel->msg_win_sender->win);
	       }

	     XDestroyWindow(panel->dpy, panel->msg_win);
	     panel->msg_win = None;
	   }
	 break;
       }
   }
}

Bool
msg_set_timeout (MBPanel *d, struct timeval *tv, struct timeval **tvp)
{
   if (d->msg_win)
   {
     if (d->msg_timeout) 	/* NON ZERO */
       {
	 int timeleft, sec, usec;
	 
	 timeleft = d->msg_timeout - (_get_server_time (d) - d->msg_starttime);
	 if (timeleft < 0)
	   {
	     XDestroyWindow(d->dpy, d->msg_win);
	     d->msg_win = None;
	     return False;
	   }

	 sec = timeleft / 1000;
	 usec = (timeleft % 1000) * 1000;

	 if (!*tvp 
	     || tv->tv_sec > sec 
	     || (tv->tv_sec == sec && tv->tv_usec > usec) )
	   {
	     tv->tv_usec = usec;
	     tv->tv_sec = sec;
	     *tvp = tv;
	   }

	 return True;
       }
   }

   return False;
}

static MBLayout*
msg_calc_win_size(MBPanel *panel, MBPanelMessageQueue *m, int *w, int *h)
{
  MBLayout *layout;

  int title_width = 0, context_width = 0;
  
  *w = 0;
  *h = 0;
   
  layout = mb_layout_new ();
  
  mb_layout_set_font(layout, panel->msg_font);
  mb_layout_set_text(layout, m->data, MB_ENCODING_UTF8);
 
  mb_layout_get_geometry(layout, w, h); 

  /* title + line */
  *h += (mb_font_get_height(panel->msg_font) + MSG_LINE_SPC) ;

  title_width = mb_font_get_txt_width(panel->msg_font,  
				      m->sender->name, strlen((char*)m->sender->name),
				      MB_ENCODING_UTF8) 
    + (2 * mb_font_get_height(panel->msg_font));

  if (title_width > *w) *w = title_width;

   if (m->extra_context_data)
     {
       context_width = mb_font_get_txt_width(panel->msg_font,
					     m->extra_context_data, 
					     strlen((char*)m->extra_context_data),
					     MB_ENCODING_UTF8);

       if (context_width > *w) *w = context_width;

       *h += mb_font_get_height(panel->msg_font);
     }

   *w +=  ( 2 * MSG_TEXT_MARGIN );

   return layout;
}


