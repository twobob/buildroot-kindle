/*
 *  mbpanel
 *
 *  A 'system tray' - Matthew Allum <mallum@handhelds.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include "panel.h"
#include "msg.h"

MBPanel *G_panel = NULL; 

#define DEFAULT_MSG_FGCOL "black"
#define DEFAULT_MSG_BGCOL "yellow"
#define DEFAULT_MSG_BGURGCOL "orange"

#ifdef USE_XSETTINGS

#define XSET_UNKNOWN  0
#define XSET_GTK_FONT 1

static void
panel_xsettings_notify_cb (const char       *name,
			   XSettingsAction   action,
			   XSettingsSetting *setting,
			   void             *data)
{
  MBPanel *panel = (MBPanel *)data;
  int i = 0;
  int key = XSET_UNKNOWN;
  
  struct _mb_xsettings { char *name; int value; } mb_xsettings[] = {
    { "Gtk/FontName",     XSET_GTK_FONT   },
    { NULL,       -1 } 
  };

  while(  mb_xsettings[i].name != NULL )
    {
      if (!strcmp(name, mb_xsettings[i].name)
	  && setting != NULL 	/* XXX set to NULL when action deleted */
	  && setting->type == XSETTINGS_TYPE_STRING )
	{
	  key = mb_xsettings[i].value;
	  break;
	}
      i++;
    }
    
  if (key == XSET_UNKNOWN) return;

  switch (action)
    {
    case XSETTINGS_ACTION_NEW:
    case XSETTINGS_ACTION_CHANGED:
      switch (key)
	{
	case XSET_GTK_FONT:
	  if (setting->data.v_string && strlen(setting->data.v_string))
	    {
	      /* This will be overidden ( hopefully ) by any theme.desktop
               * setting.
               */
	      DBG("%s() setting  XSET_GTK_FONT: '%s'\n", __func__, 
		  setting->data.v_string);
	      mb_font_set_from_string(panel->msg_font, setting->data.v_string);
	      
	    }
	  break;

	}
    case XSETTINGS_ACTION_DELETED:
      /* Do nothing for now */
      break;
    }
}

#endif


void
panel_handle_full_panel (MBPanel *panel, MBPanelApp *bad_papp)
{

  /* For now we just killoff the client */

  DBG("%s() killing winbow id %li\n", __func__, bad_papp->win);

  XKillClient(panel->dpy, bad_papp->win); /* XXX should probably make this
				             kill more effectively    */
  panel_app_destroy (panel, bad_papp); 
}

void  /* XXX maybe better to add this to panel_menu_add_remove_item */
panel_update_client_list_prop (MBPanel *panel)
{
  MBPanelApp *papp = NULL;
  MBPanelApp *papp_heads[] = { panel->apps_start_head, 
			       panel->apps_end_head,
			       NULL };
  int i = 0, app_cnt = 0, cnt = 0;
  Window  *wins = NULL;

  DBG("%s() called\n", __func__);

  /* Count number of applets docked */
  while (i < 2)
    {
      papp = papp_heads[i];
      while( papp != NULL)
	{
	  if (!papp->ignore) app_cnt++;
	  papp = papp->next;
	}
      i++;
    }

  if (app_cnt == 0)
    {
      /* delete prop ?? */
      XChangeProperty(panel->dpy, panel->win, 
		      panel->atoms[ATOM_NET_CLIENT_LIST] ,
		      XA_WINDOW, 32, PropModeReplace,
		      (unsigned char *)NULL, 0);
      return;
    }

  i = 0;
  wins = malloc(sizeof(Window)*app_cnt);

  while (i < 2)
    {
      papp = papp_heads[i];

      if (i == 1)
	papp = panel_app_list_get_last(panel, panel->apps_end_head);

      while( papp != NULL)
	{
	  if (!papp->ignore)
	    {
	      wins[cnt++] = papp->win;
	    }

	  if (i == 1)
	    {
	      papp = panel_app_list_get_prev (panel, papp, 
					      &panel->apps_end_head);
	    }
	  else papp = papp->next;
	}
      i++;
    }

  
  XChangeProperty(panel->dpy, panel->win, 
		  panel->atoms[ATOM_NET_CLIENT_LIST],
		  XA_WINDOW, 32, PropModeReplace,
		  (unsigned char *)wins, app_cnt);

  free(wins);

}


void 				/* show/hiden the panel */
panel_toggle_visibilty(MBPanel *d)
{

#define PANEL_HIDDEN_SIZE 6 

  
  static int  panel_orig_size;
  MBPanelApp *papp = NULL;
  MBPanelApp *papp_heads[] = { d->apps_start_head, 
			       d->apps_end_head,
			       NULL };
  int i = 0;

  DBG("%s() called, x: %i, y: %i, w: %i, h: %i\n", __func__,
      d->x, d->y, d->w, d->h);

  if (d->is_hidden)
    {

      if (PANEL_IS_VERTICAL(d))
	{
	  XMoveResizeWindow(d->dpy, d->win, d->x, d->y, 
			    panel_orig_size, d->h );
	  d->w = panel_orig_size;
	} else {
	  XMoveResizeWindow(d->dpy, d->win, d->x, d->y, d->w, 
			    panel_orig_size);
	  d->h = panel_orig_size;
	}

      while (i < 2)
	{
	  papp = papp_heads[i];
	  while( papp != NULL)
	    {
	      XMapWindow(d->dpy, papp->win);
	      panel_app_move_to (d, papp,  
				 panel_app_get_offset (d, papp));
	      papp->ignore_unmap--;
	      papp = papp->next;
	    }
	  i++;
	}

      d->is_hidden = False;

    } else {
      while (i < 2)
	{
	  papp = papp_heads[i];
	  while( papp != NULL)
	    {
	      XUnmapWindow(d->dpy, papp->win);
	      papp->ignore_unmap++;
	      papp = papp->next;
	    }
	  i++;
	}

      if (PANEL_IS_VERTICAL(d))
	{
	  XMoveResizeWindow(d->dpy, d->win, d->x, d->y, 
			    PANEL_HIDDEN_SIZE, d->h );
	  panel_orig_size = d->w;
	} else {
	  XMoveResizeWindow(d->dpy, d->win, d->x, d->y, 
			    d->w, PANEL_HIDDEN_SIZE);
	  panel_orig_size = d->h;
	}
      d->is_hidden = True;
    }
}

void
panel_set_bg(MBPanel *panel, int bg_type, char *bg_spec)
{
  MBPixbufImage *img_tmp, *img_bg;
  int dx, dy, dw, dh;

  Pixmap tmp_pxm;
  char xprop_def[32] = { 0 };
  char *tmp_path = NULL;

  if (panel->bg_spec) free(panel->bg_spec);

  panel->bg_spec = strdup(bg_spec);
  panel->bg_type = bg_type;

  DBG("%s() bg_spec: %s, type %i\n", __func__, panel->bg_spec, panel->bg_type);

  XGrabServer(panel->dpy);

  switch (bg_type)
    {
    case BG_PIXMAP:
      if (bg_spec[0] != '/' && panel->theme_path != NULL)
	{
	  tmp_path = alloca( sizeof(char) * (strlen(panel->theme_path) + strlen(bg_spec) + 2));
	  sprintf(tmp_path, "%s/%s", panel->theme_path, bg_spec);
	}
      else tmp_path = bg_spec;
      
      if ((img_tmp = mb_pixbuf_img_new_from_file(panel->pb, tmp_path)) == NULL)
	{
	  fprintf(stderr, "matchbox-panel: failed to load %s\n", bg_spec);
	  panel_set_bg(panel, BG_SOLID_COLOR, DEFAULT_COLOR_SPEC);
	  return;
	}
      
      img_bg = mb_pixbuf_img_new(panel->pb, panel->w, panel->h);

      /* scale for panel height ? */
      if ( (img_tmp->height != panel->h && !PANEL_IS_VERTICAL(panel))
	   || (img_tmp->width != panel->w && PANEL_IS_VERTICAL(panel)) )
	{
	  MBPixbufImage *img_scaled;
	  img_scaled =  mb_pixbuf_img_scale(panel->pb, img_tmp,
					    img_tmp->width,
					    PANEL_IS_VERTICAL(panel) ? 
					    panel->w : panel->h);
	  mb_pixbuf_img_free(panel->pb, img_tmp);
	  img_tmp = img_scaled;
	}

      /* rotate  */
      if (PANEL_IS_VERTICAL(panel))
	{
	  MBPixbufImage *img_rot;
	  img_rot =  mb_pixbuf_img_transform (panel->pb, img_tmp,
					      MBPIXBUF_TRANS_ROTATE_90);
	  
	  mb_pixbuf_img_free(panel->pb, img_tmp);
	  img_tmp = img_rot;
	}
      
      for (dy=0; dy < panel->h; dy += img_tmp->height)
	for (dx=0; dx < panel->w; dx += img_tmp->width)
	  {
	    if ( (dx + img_tmp->width) > panel->w )
	      dw = img_tmp->width - ((dx + img_tmp->width)-panel->w);
	    else
	      dw = img_tmp->width;
	    
	    if ( (dy + img_tmp->height) > panel->h )
	      dh = img_tmp->height-((dy + img_tmp->height)-panel->h);
	    else
	      dh = img_tmp->height;
	    mb_pixbuf_img_copy(panel->pb, img_bg, img_tmp,
			       0, 0, dw, dh, dx, dy);
	  }
      
      mb_pixbuf_img_free(panel->pb, img_tmp);
      
      if (panel->bg_pxm != None) 
	XFreePixmap(panel->dpy, panel->bg_pxm);

      panel->bg_pxm = XCreatePixmap(panel->dpy, panel->win_root, 
				    panel->w, panel->h,
				    panel->pb->depth ); 

      mb_pixbuf_img_render_to_drawable(panel->pb, img_bg, panel->bg_pxm, 0, 0);
      
      mb_pixbuf_img_free(panel->pb, img_bg);
      
      XSetWindowBackgroundPixmap(panel->dpy, panel->win, panel->bg_pxm);
      XClearWindow(panel->dpy, panel->win);
      
      snprintf(xprop_def, 32, "pxm:%li", panel->bg_pxm);
      panel->root_pixmap_id = 0;
      break;
    case BG_SOLID_COLOR:
      if (XParseColor(panel->dpy, 
		      DefaultColormap(panel->dpy, panel->screen),
		      bg_spec,
		      &panel->xcol ))
	{

	  /*
	  XAllocColor(panel->dpy, 
		      DefaultColormap(panel->dpy, panel->screen),
		      &panel->xcol);
	  */

	  XSetWindowBackground(panel->dpy, panel->win, 
			       mb_pixbuf_lookup_x_pixel(panel->pb, 
							panel->xcol.red >> 8,
							panel->xcol.green >> 8,
							panel->xcol.blue >> 8, 0));

	  //			       panel->xcol.pixel);
	  XClearWindow(panel->dpy, panel->win);

	  if (panel->bg_pxm != None) 
	    XFreePixmap(panel->dpy, panel->bg_pxm);
	  panel->bg_pxm = None;
	  
	  snprintf(xprop_def, 32, "rgb:%li", mb_pixbuf_lookup_x_pixel(panel->pb, 
								      panel->xcol.red >> 8,
								      panel->xcol.green >> 8,
								      panel->xcol.blue >> 8, 0));
	}
      panel->root_pixmap_id = 0;
      break;
    case BG_TRANS:
      tmp_pxm = util_get_root_pixmap(panel);
      if (tmp_pxm != None)
	{
	  int trans_level = 0;

	  if (bg_spec) trans_level = atoi(bg_spec);

	  DBG("%s() Getting root pixmap\n", __func__);

	  if (panel->bg_pxm != None) XFreePixmap(panel->dpy, panel->bg_pxm);

	  panel->bg_pxm = XCreatePixmap(panel->dpy, panel->win_root, 
					panel->w, panel->h,
					panel->pb->depth ); 
	  
	  img_tmp = mb_pixbuf_img_new_from_drawable(panel->pb, 
						    tmp_pxm, None,
						    panel->x, panel->y, 
						    panel->w, panel->h); 
	  if (img_tmp == NULL)
	    {
	      XFreePixmap(panel->dpy, panel->bg_pxm);
	      fprintf(stderr, "Failed to get root pixmap id\n");
	      panel_set_bg(panel, BG_SOLID_COLOR, DEFAULT_COLOR_SPEC);
	      return;
	    }

	  /*
	  DBG("%s() First pixel of root looks like %i, %i, %i, %i\n", 
	      __func__, img_tmp->rgba[0], img_tmp->rgba[1],
	      img_tmp->rgba[2], img_tmp->rgba[3]);
	  */

	  if (trans_level > 0)
	    for (dx = 0; dx < panel->w; dx++)
	      for (dy = 0; dy < panel->h; dy++)
		mb_pixbuf_img_plot_pixel_with_alpha(panel->pb,
						    img_tmp, dx, dy, 
						    255, 255, 255,
						    trans_level);
	  
	  
	  mb_pixbuf_img_render_to_drawable(panel->pb, img_tmp, 
					   panel->bg_pxm, 0, 0);
	  
	  mb_pixbuf_img_free(panel->pb, img_tmp);
	  
	  
	  XSetWindowBackgroundPixmap(panel->dpy, panel->win, 
				     panel->bg_pxm);
	  XClearWindow(panel->dpy, panel->win);
	  
	  snprintf(xprop_def, 32, "pxm:%li", panel->bg_pxm);
	  
	} else {
	  fprintf(stderr, "Failed to get root pixmap id\n");
	  panel->root_pixmap_id = -1;
	  panel_set_bg(panel, BG_SOLID_COLOR, DEFAULT_COLOR_SPEC);
	  return;
	}
      
      break;
    }

  if (xprop_def)
    {
      DBG("setting _MB_PANEL_BG to %s\n", xprop_def);
      
      XChangeProperty(panel->dpy, panel->win, 
		      panel->atoms[ATOM_MB_PANEL_BG],
		      XA_STRING, 8,
		      PropModeReplace, xprop_def, 
		      strlen(xprop_def));
    }

  XUngrabServer(panel->dpy);

  XSync(panel->dpy, False);

}

Bool
panel_set_theme_from_root_prop(MBPanel *panel)
{
  Atom realType;
  unsigned long n;
  unsigned long extra;
  int format;
  int status;
  char * value;
  struct stat stat_info;
  char panel_cfg[256];

  DBG("%s() called\n", __func__);
		
  if ( panel->use_themes == False )
    {
      DBG("%s() panel themeing disabled by command line options\n", __func__ );
      return False;
    }
  
  status = XGetWindowProperty(panel->dpy, panel->win_root,
			      panel->atoms[ATOM_MB_THEME], 
			      0L, 512L, False,
			      AnyPropertyType, &realType,
			      &format, &n, &extra,
			      (unsigned char **) &value);
	    
  if (status != Success || value == 0
      || *value == 0 || n == 0)
    {
      DBG("%s() no _MB_THEME set on root window\n", __func__ );
      return False;
    } else {

      int i = 0;
      MBPanelApp *papp = NULL;
      MBPanelApp *papp_heads[] = { panel->apps_start_head, 
				   panel->apps_end_head,
				   NULL };

      strcpy(panel_cfg, value);
      strcat(panel_cfg, "/theme.desktop");

      if (stat(panel_cfg, &stat_info) != -1)
	{
	  MBDotDesktop *theme  = NULL;
	  theme = mb_dotdesktop_new_from_file(panel_cfg);
	  if (theme)
	    {
	      if (panel->theme_path) free(panel->theme_path);
	      panel->theme_path = strdup(value);

	      /* Different theme values for panel in titlebar */
	      if (panel->want_titlebar_dest)
		{
		  if (mb_dotdesktop_get(theme, "TitlebarDockBgColor"))
		    {
		      panel_set_bg(panel, BG_SOLID_COLOR, 
				   mb_dotdesktop_get(theme, "TitlebarDockBgColor"));
		    }

		  if (mb_dotdesktop_get(theme, "TitlebarDockBgPixmap"))
		    {
		      panel_set_bg(panel, BG_PIXMAP, 
				   mb_dotdesktop_get(theme, "TitlebarDockBgPixmap")); 
		    }

		  /* Newer settings */

		  if (mb_dotdesktop_get(theme, "TitlebarPanelBgColor"))
		    {
		      panel_set_bg(panel, BG_SOLID_COLOR, 
				   mb_dotdesktop_get(theme, "TitlebarPanelBgColor"));
		    }

		  if (mb_dotdesktop_get(theme, "TitlebarPanelBgPixmap"))
		    {
		      panel_set_bg(panel, BG_PIXMAP, 
				   mb_dotdesktop_get(theme, "TitlebarPanelBgPixmap")); 
		    }


		}
	      else
		{

		  /* 
                   * FIXME: Need to phase out the Dock prefix to panel
		   */

		  if (mb_dotdesktop_get(theme, "DockBgColor"))
		    {
		      panel_set_bg(panel, BG_SOLID_COLOR, 
				   mb_dotdesktop_get(theme, "DockBgColor"));
		    }
		  if (mb_dotdesktop_get(theme, "DockBgTrans"))
		    {
		      panel_set_bg(panel, BG_TRANS, 
				   mb_dotdesktop_get(theme, "DockBgTrans")); 
		    }
		  if (mb_dotdesktop_get(theme, "DockBgPixmap"))
		    {
		      panel_set_bg(panel, BG_PIXMAP, 
				   mb_dotdesktop_get(theme, "DockBgPixmap")); 
		    }

		  /* Newer setting below */

		  if (mb_dotdesktop_get(theme, "PanelBgColor"))
		    {
		      panel_set_bg(panel, BG_SOLID_COLOR, 
				   mb_dotdesktop_get(theme, "PanelBgColor"));
		    }
		  if (mb_dotdesktop_get(theme, "PanelBgTrans"))
		    {
		      panel_set_bg(panel, BG_TRANS, 
				   mb_dotdesktop_get(theme, "PanelBgTrans")); 
		    }
		  if (mb_dotdesktop_get(theme, "PanelBgPixmap"))
		    {
		      panel_set_bg(panel, BG_PIXMAP, 
				   mb_dotdesktop_get(theme, "PanelBgPixmap")); 
		    }


		}

	      if (mb_dotdesktop_get(theme, "PanelMsgFont"))
		mb_font_set_from_string(panel->msg_font, 
					mb_dotdesktop_get(theme, "PanelMsgFont"));

	      if (mb_dotdesktop_get(theme, "PanelMsgBgCol"))
		util_xcol_from_spec(panel, panel->msg_col, 
				    mb_dotdesktop_get(theme, "PanelMsgBgCol"));
	      else
		util_xcol_from_spec(panel, panel->msg_col, 
				    DEFAULT_MSG_BGCOL);




	      if (mb_dotdesktop_get(theme, "PanelMsgBgUrgentCol"))
		util_xcol_from_spec(panel, panel->msg_urgent_col, 
				    mb_dotdesktop_get(theme, 
						      "PanelMsgBgUrgentCol"));
	      else
		util_xcol_from_spec(panel, panel->msg_urgent_col, 
				    DEFAULT_MSG_BGURGCOL);


	      if (mb_dotdesktop_get(theme, "PanelMsgFgCol"))
		util_xcol_from_spec(panel, panel->msg_fg_col, 
				    mb_dotdesktop_get(theme, 
						      "PanelMsgFgCol"));
	      else
		util_xcol_from_spec(panel, panel->msg_fg_col, 
				    DEFAULT_MSG_FGCOL);
	      mb_dotdesktop_free(theme);

	    } 
	}

      if (value) XFree(value);

      status = XGetWindowProperty(panel->dpy, panel->win_root,
				  panel->atoms[ATOM_MB_THEME_NAME], 
				  0L, 512L, False,
				  AnyPropertyType, &realType,
				  &format, &n, &extra,
				  (unsigned char **) &value);

      if (status && value)
	{
	  if (panel->theme_name) free(panel->theme_name);
	  panel->theme_name = strdup(value);
	}

      if (value) XFree(value);

      /* Now retheme the panel menu */
      panel_menu_init(panel);

      while (i < 2)
	{
	  papp = papp_heads[i];
	  while( papp != NULL )
	    {
	      panel_menu_update_remove_items(panel);
	      papp = papp->next;
	    }
	  i++;
	}

      return True;
    }
  return False;
}

void 
panel_usage(char *bin_name)
{
  fprintf(stderr, "%s (%s) usage: %s [Options...]\n"
	  "Where options are;\n"
          "-display, -d         <X11 Display name>\n"
          "-geometry, -g        Use --size / --orientation instead.\n"
	  "--id                 <int> Panel ID\n"
          "--size, -s           <int> width/height of dock in pixels\n"
          "--orientation        <north|east|south|west>\n"
          "--default-apps, -da  <app list> comma seperated list of apps to\n"
          "                     add to a tray when no session exists\n"
          "--margins            <int>[,<int>]  specify left+right[, top+bottom ] panel\n"
	  "                     margins in pixels ( default: 2,2 )\n"
	  "--padding            Specify padding betweeen applets in pixels ( default: 2)\n"
	  "--titlebar           Request panel in titlebar - see docs for limitations\n"
          "--no-session, -ns    No session saving.\n"
          "--no-menu, -nm       No popup menu\n"   
          "--no-flip, -nf       On a display rotation, stop the panel from\n"
          "                     From rotating itself too\n"
          "--overide-bubbles, -o\n\n"
	  "Background options:\n"
          "--bgcolor,  -c       <color spec>\n"
	  "--bgpixmap, -b       <image file>\n"
	  "--bgtrans,  -bt      <'yes'|transparency percentage>\n"
	  "*NOTE* setting the background here will disable the effect\n"
	  "       of any external theme changes. \n"
	  , bin_name, VERSION, bin_name);
  exit(1);
}

/* XEMBED */

int
panel_get_map_state(MBPanel *d, MBPanelApp *c)
{
  Atom realType;
  unsigned long n;
  unsigned long extra;
  int format;
  int status;
  CARD32 * value = NULL;
  int result = -1; 

  status = XGetWindowProperty(d->dpy, c->win,
			      d->atoms[ATOM_XEMBED_INFO],
			      0L, 10L,
			      0, XA_ATOM, &realType, &format,
			      &n, &extra, (unsigned char **) &value);
  if (status == Success)
    {
      if (realType == XA_CARDINAL && format == 32)
	{
	  /*
	    printf("VERSION: %i\n", value[0]);
	    printf("MAPPED:  %i\n", value[1]);
	  */
	  
	  result = (int)value[1];

	}
    }

  if (value) XFree(value);

  return result;
}

void panel_send_xembed_message(
			      MBPanel *d,
			      MBPanelApp *c,
			      long message, /* message opcode */
			      long detail,  /* message detail */
			      long data1,  /* message data 1 */
			      long data2  /* message data 2 */
			      ){
  XEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = c->win;
  ev.xclient.message_type = d->atoms[ATOM_XEMBED_MESSAGE];
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = message;
  ev.xclient.data.l[2] = detail;
  ev.xclient.data.l[3] = data1;
  ev.xclient.data.l[4] = data2;
  XSendEvent(d->dpy, c->win, False, NoEventMask, &ev);
  XSync(d->dpy, False);
}

void panel_send_manage_message( MBPanel *d )
{
  XEvent ev;
  memset(&ev, 0, sizeof(ev));
  ev.xclient.type = ClientMessage;
  ev.xclient.window = d->win_root;
  ev.xclient.message_type = d->atoms[ATOM_MANAGER];
  ev.xclient.format = 32;
  ev.xclient.data.l[0] = CurrentTime;
  ev.xclient.data.l[1] = d->atoms[ATOM_SYSTEM_TRAY];
  ev.xclient.data.l[2] = d->win;

  XSendEvent(d->dpy, d->win_root, False, StructureNotifyMask, &ev);
  XSync(d->dpy, False);
}



/* Events */

void
panel_handle_button_event(MBPanel *panel, XButtonEvent *e)
{
  XEvent         ev;
  int            done = 0;
  struct timeval then, now;

  DBG("%s() called, subwindow : %li\n", __func__, e->subwindow );

  if (!panel->use_menu) return; /* menu disabled */

  if (e->window != panel->win) return;

  if (panel->is_hidden)
    {
      panel_toggle_visibilty(panel);
      return;
    }

  gettimeofday(&then, NULL);
  while (!done) {
    if (XCheckMaskEvent(panel->dpy,ButtonReleaseMask, &ev))
      if (ev.type == ButtonRelease) done=1;
    gettimeofday(&now, NULL);
    if ( (now.tv_usec-then.tv_usec) > (panel->click_time*1000) )
      done=2;
  }
  
  if (done == 2 && !mb_menu_is_active(panel->mbmenu))
    {
      int dpy_h = DisplayHeight(panel->dpy, panel->screen);
      util_get_mouse_position(panel, &panel->click_x, &panel->click_y);
      mb_menu_activate(panel->mbmenu,
		       (panel->click_x-5 < 0) ? 2 : panel->click_x-5,
		       (panel->click_y+5 > dpy_h) ? dpy_h-2 : panel->click_y+5);
    }
}

void
panel_handle_expose(MBPanel *panel, XExposeEvent *e)
{
  ;
}

void
panel_handle_dock_request(MBPanel *panel, Window win)
{
  int app_origin_dist = 0;
  char *cmd_str = NULL;
  MBPanelApp *new_papp = NULL;

  util_get_command_str_from_win(panel, win, &cmd_str); /* cmd_str freed l8r */

  if (session_preexisting_restarting(panel))
    {
      if (session_preexisting_win_matches_wanted(panel, win, cmd_str))
	{
	  app_origin_dist = panel->session_init_offset;
	  session_preexisting_clear_current(panel);
	}
      else
	{
	  DBG("%s() defering winid %li ( %s) \n", __func__, win, cmd_str );
	  panel->session_defered_wins[panel->n_session_defered_wins++] = win;
	  return;
	}
    }

  new_papp = panel_app_new(panel, win, cmd_str);

  if (new_papp) 
    {
      XSelectInput(panel->dpy, new_papp->win, PropertyChangeMask );
  
      /* tell app its docked - panel app should now map its self */
      panel_send_xembed_message(panel, new_papp,
				XEMBED_EMBEDDED_NOTIFY,
				0, panel->win, 0);
  
      /* sent when it gets focus */
      panel_send_xembed_message(panel, new_papp,
				XEMBED_WINDOW_ACTIVATE,
				0,0,0);
  
      XMapWindow(panel->dpy, new_papp->win);	
      new_papp->mapped = True;

      panel_menu_update_remove_items(panel);
    }

  session_save(panel);

  session_preexisting_start_next(panel);
}

void
panel_handle_client_message(MBPanel *panel, XClientMessageEvent *e)
{
  DBG("%s() called\n", __func__ );
  if (e->message_type
      == panel->atoms[ATOM_SYSTEM_TRAY_OPCODE])
    {
      DBG("%s() got system tray message\n", __func__ );
      switch (e->data.l[1])
	{
	case SYSTEM_TRAY_REQUEST_DOCK:
	  DBG("%s() is SYSTEM_TRAY_REQUEST_DOCK\n", __func__ );
	  panel_handle_dock_request(panel, e->data.l[2]);
	  break;
	case SYSTEM_TRAY_BEGIN_MESSAGE:
	  DBG("%s() is SYSTEM_TRAY_BEGIN_MESSAGE\n", __func__ );
	  msg_new(panel, e);
	  break;
	case SYSTEM_TRAY_CANCEL_MESSAGE:
	  DBG("%s() is SYSTEM_TRAY_CANCEL_MESSAGE\n", __func__ );
	  msg_cancel(panel, e);
	  break;
	}
      return;
    }

  if (e->message_type
      == panel->atoms[ATOM_MB_COMMAND])
    {
      switch (e->data.l[0])
	{
	case MB_CMD_PANEL_TOGGLE_VISIBILITY:
	  panel_toggle_visibilty(panel);
	  break;
	case MB_CMD_PANEL_SIZE:
	case MB_CMD_PANEL_ORIENTATION:
	default:
	  break;
	}

    }

  if (e->message_type
      == panel->atoms[ATOM_NET_SYSTEM_TRAY_MESSAGE_DATA])
    {
      DBG("%s() got system tray message _data_\n", __func__ );
      msg_add_data(panel, e);
      return;
    }

  if (e->message_type
      == panel->atoms[ATOM_WM_DELETE_WINDOW])
    {
      util_cleanup_children(0);
    }
   
  return;
}

void
panel_handle_property_notify(MBPanel *panel, XPropertyEvent *e)
{
  MBPanelApp *papp;

  if (e->atom == panel->atoms[ATOM_XEMBED_INFO])
    {
      int i; 
      papp = panel_app_get_from_window(panel, e->window); 
      DBG("%s() got XEMBED_INFO property notify for %s\n", 
	  __func__, papp->name );
      if (papp != NULL)
	{
	  i = panel_get_map_state(panel, papp);
	  if (i == 1)
	    {
	      XMapRaised(panel->dpy, papp->win);
	      papp->mapped = True;
	    }
	  if (i == 0)
	    {
	      XUnmapWindow(panel->dpy, papp->win);
	      papp->mapped = False;
	    }
	}
      return;
    }

  if (e->atom == panel->atoms[ATOM_MB_THEME])
    {
      panel_set_theme_from_root_prop(panel);
      return;
    }
  
  if (e->atom == panel->atoms[ATOM_XROOTPMAP_ID] 
      && panel->root_pixmap_id != 0)
    {
      panel_set_bg(panel, BG_TRANS, panel->bg_trans); 
      return;
    }

  if (e->atom == panel->atoms[ATOM_MB_REQ_CLIENT_ORDER])
    {
      panel_reorder_apps(panel);
    }
}

static Bool
get_xevent_timed(Display* dpy, XEvent* event_return, struct timeval *tv)
{
  if (tv == NULL) 
    {
      XNextEvent(dpy, event_return);
      return True;
    }

  XFlush(dpy);

  if (XPending(dpy) == 0) 
    {
      int fd = ConnectionNumber(dpy);
      fd_set readset;
      FD_ZERO(&readset);
      FD_SET(fd, &readset);
      if (select(fd+1, &readset, NULL, NULL, tv) == 0) 
	{
	  return False;
	} else {
	  XNextEvent(dpy, event_return);
	  return True;
	}
    } else {
      XNextEvent(dpy, event_return);
      return True;
    }
}

void 
panel_main(MBPanel *panel)
{
  MBPanelApp *papp;
  XEvent an_event;
  int xfd;
  Bool had_rotation = False;


  XSelectInput (panel->dpy, panel->win_root, 
		PropertyChangeMask|StructureNotifyMask);

  xfd = ConnectionNumber (panel->dpy);

  XFlush(panel->dpy);

  while(1)
    {
      struct timeval tvt, *tvp = NULL;

      session_preexisting_set_timeout (panel, &tvt, &tvp);
      msg_set_timeout (panel, &tvt, &tvp);
      
      if (get_xevent_timed(panel->dpy, &an_event, tvp))
	{
#ifdef USE_XSETTINGS
	  if (panel->xsettings_client != NULL)
	    xsettings_client_process_event(panel->xsettings_client, &an_event);
#endif
	  mb_menu_handle_xevent(panel->mbmenu, &an_event);
	  switch (an_event.type)
	    {
	    case SelectionClear:
	      if (an_event.xselectionclear.selection == panel->atoms[ATOM_SYSTEM_TRAY])
		{
		  fprintf(stderr, "matchbox-panel: Another system tray has started and taken tray ID. Exiting.\n");
		  exit(-1);
		}
	      break;
	    case ButtonPress:
	      panel_handle_button_event(panel, &an_event.xbutton);
	      break;
	    case ClientMessage:
	      panel_handle_client_message(panel, &an_event.xclient);
	      break;
	    case PropertyNotify:
	      panel_handle_property_notify(panel, &an_event.xproperty);
	      break;
	    case MapRequest:
	      break;
	    case UnmapNotify:
	      /* window should unmap ... we destroy it */
	      papp = panel_app_get_from_window(panel, 
					       an_event.xunmap.window);
	      if (papp && !papp->ignore_unmap)
		{
		  panel_app_destroy(panel, papp);

		  /* remove any connected message windows */
		  if (panel->msg_win && panel->msg_win_sender == papp)
		    {
		      XDestroyWindow(panel->dpy, panel->msg_win);
		      panel->msg_win = None;
		    }

		  /* 
		   *  We set an alarm here, so the session file is only
		   *  updated after a couple fo seconds. 
		   *  This is done so if the xserver is closing and bringing 
		   *  down all the clients. Its very likely it'll kill 
		   *  a panelapp before the panel, and we dont really want it
		   *  to be refmoved from the session file.
		   */
		  alarm(3);
		}
	      break;
	    case Expose:
	      panel_handle_expose(panel, &an_event.xexpose);
	      break;
	    case DestroyNotify:
	      break;
	    case ConfigureRequest:
	      panel_app_handle_configure_request(panel, 
						 &an_event.xconfigurerequest);
	      break;
	    case ConfigureNotify:
	      DBG("%s(): configureNotify\n", __func__);
	      if (an_event.xconfigure.window == panel->win_root)
		{
		  had_rotation = True;
		  DBG("%s() **** HAD ROTATION ***\n", __func__);
		  break;
		}	 
     
	      if (an_event.xconfigure.window == panel->win)
		{
		  DBG("%s(): configureNotify on panel\n", __func__);
		  /* These can be confused by a flip */
		  if (an_event.xconfigure.send_event)
		    break;

		  if (panel->w != an_event.xconfigure.width
		      || panel->h != an_event.xconfigure.height)
		    {
		      int diff = 0;
		      MBPanelApp *papp = NULL;

		      DBG("mark %i\n", __LINE__);

		      if (panel->ignore_next_config)
			{
			  panel->ignore_next_config = False;
			  break;
			}

		      if (panel->use_flip && had_rotation) 
			{
			  /* Flip if are length is changed 
			     XXX a little hacky XXXX
			  */
			  int dpy_w, dpy_h;
			  XWindowAttributes root_attr;
			  
			  XGetWindowAttributes(panel->dpy, 
					       panel->win_root, 
					       &root_attr);
			  
			  dpy_w = root_attr.width;
			  dpy_h = root_attr.height;
			  
			  had_rotation = False;

			  DBG("mark %i\n", __LINE__);

			  if ((PANEL_IS_VERTICAL(panel)
			       && (an_event.xconfigure.width == panel->w)
			       )
			      ||
			      (!PANEL_IS_VERTICAL(panel)
			       && (an_event.xconfigure.height == panel->h)
			       /* && (an_event.xconfigure.width  == dpy_w) 
				  && dpy_w != panel->w */ )
			      )
			    { 

			      DBG("%s() flipping ....\n", __func__);

			      panel->ignore_next_config = True;

			      switch (panel->orientation)
				{
				case South:
				  panel_change_orientation(panel, East,
							   dpy_w, dpy_h);
							   break;
				case North:
				  panel_change_orientation(panel, West,
							   dpy_w, dpy_h);
				  break;
				case West:
				  panel_change_orientation(panel, North,
							   dpy_w, dpy_h);
				  break;
				case East:
				  panel_change_orientation(panel, South,
							   dpy_w, dpy_h);
				  break;
				}
			      break;
			    }
			}

		      if (PANEL_IS_VERTICAL(panel))
			{
			  diff = an_event.xconfigure.height - panel->h;
			  if (an_event.xconfigure.y > panel->y)
			    papp = panel->apps_start_head;
			  else
			    papp = panel->apps_end_head;
			} else {	
			  DBG("mark %i\n", __LINE__);
			  diff = an_event.xconfigure.width - panel->w;
			  if (an_event.xconfigure.x > panel->x)
			    papp = panel->apps_start_head;
			  else
			    papp = panel->apps_end_head;
			}

		      panel->w = an_event.xconfigure.width;
		      panel->h = an_event.xconfigure.height;

		      panel_apps_nudge (panel, papp, diff); 

		      /* Nake sure bg gets updated */
		      if (!(panel->use_flip && had_rotation))
			{
			  char *tmp_str = NULL;
			  if (panel->bg_spec)
			    {
			      tmp_str = strdup(panel->bg_spec) ;
			      panel_set_bg(panel, panel->bg_type, tmp_str);
			    }
			  if (tmp_str) free(tmp_str);
			}

		      // panel_apps_rescale (panel, panel->apps_start_head);
		      // panel_apps_rescale (panel, panel->apps_end_head);
		    }

		  panel->x = an_event.xconfigure.x;
		  panel->y = an_event.xconfigure.y;

		  DBG("%s() config notify, got x: %i , y: %i w: %i h: %i \n", 
		      __func__,
		      an_event.xconfigure.x,
		      an_event.xconfigure.y,
		      an_event.xconfigure.width,
		      an_event.xconfigure.height);

		  DBG("%s() panel is now  x: %i , y: %i w: %i h: %i \n", 
		      __func__, panel->x, panel->y,
		      panel->w, panel->h  );
		} 
	      break;
	    }
	  msg_handle_events(panel, &an_event);
	}
    }
}

static void
panel_orientation_set_hint (MBPanel *panel)
{
  int is_vertical[1] = { 0 };

  if (PANEL_IS_VERTICAL(panel)) is_vertical[0] = 1;

  XChangeProperty(panel->dpy, panel->win, 
		  panel->atoms[ATOM_NET_SYSTEM_TRAY_ORIENTATION], 
		  XA_CARDINAL, 32, PropModeReplace,
		  (unsigned char *)is_vertical , 1);
}


void
panel_change_orientation(MBPanel *panel, 
			 MBPanelOrientation new_orientation,
			 int dpy_w, int dpy_h)
{
  char *tmp_str = NULL;

  XUnmapWindow(panel->dpy, panel->win);

  if ( ( (panel->orientation == East || panel->orientation == West)
	 && (new_orientation == North || new_orientation == South) )
       || 
       ( (panel->orientation == North || panel->orientation == South)
	 && (new_orientation == East || new_orientation == West ) )
       )
    {
      MBPanelApp *papp_heads[] = { panel->apps_start_head, 
				   panel->apps_end_head,
				   NULL };
      MBPanelApp *papp_cur = NULL;
      int i = 0;

      panel->x = 0;
      panel->y = 0;
      panel->h = panel->default_panel_size;
      panel->w = dpy_w;

      switch (new_orientation)
	{
	case South:
	  panel->y = dpy_h - panel->h; 
	  break;
	case North:
	  break;
	case West:
	  panel->w = panel->default_panel_size;
	  panel->h = dpy_h;
	  break;
	case East:
	  panel->w = panel->default_panel_size;
	  panel->h = dpy_h;
	  panel->x = dpy_w - panel->w; 
	  break;
	}

      panel->orientation = new_orientation;

      panel_orientation_set_hint(panel);

      DBG("%s() setting panel  x: %i , y: %i w: %i h: %i \n", 
	  __func__, panel->x, panel->y,
			 panel->w, panel->h  );

      XMoveResizeWindow( panel->dpy, panel->win, panel->x, panel->y,
			 panel->w, panel->h );

      /* move_to() will reposition each app */
      while (i < 2)
	{
	  papp_cur = papp_heads[i];
	  DBG("%s() moveing to, cur is i: %i ( %p )\n",
	      __func__, i, papp_cur);

	  while (papp_cur != NULL)
	    {
	      int tmp;
	      tmp = papp_cur->w;
	      papp_cur->w = papp_cur->h;
	      papp_cur->h = tmp;
	  
	      if (panel->orientation == North || panel->orientation == South)
		{
		  papp_cur->x = papp_cur->offset;
		  papp_cur->y = (panel->h - papp_cur->h) / 2;
		}
	      else
		{
		  papp_cur->y = papp_cur->offset;
		  papp_cur->x = (panel->w - papp_cur->w) / 2;
		}

	      XMoveWindow(panel->dpy, papp_cur->win, 
			  papp_cur->x, papp_cur->y);      

	      panel_app_deliver_config_event(panel, papp_cur);
	      papp_cur = papp_cur->next;
	    }
	  i++;
	}
    }
  else
    {
      switch (new_orientation)
	{
	case South:
	  panel->y = dpy_h - panel->h; 
	  break;
	case North:
	  panel->y = 0;
	  break;
	case West:
	  panel->x = 0;
	  break;
	case East:
	  panel->x = dpy_w - panel->w; 
	  break;
	}

      panel->orientation = new_orientation;

      panel_orientation_set_hint(panel);

      XMoveResizeWindow( panel->dpy, panel->win, panel->x, panel->y,
			 panel->w, panel->h );
    }

  if (panel->bg_spec) tmp_str = strdup(panel->bg_spec) ;
  panel_set_bg(panel, panel->bg_type, tmp_str);
  if (tmp_str) free(tmp_str);

  XMapWindow(panel->dpy, panel->win);
  XMapSubwindows(panel->dpy, panel->win);

}



void
panel_reorder_apps(MBPanel *panel)
{
  MBPanelApp *papp, *papp_tmp;
  int i, watermark = 0, offset = 0;

  Atom           actual_type;
  int            actual_format;
  unsigned long  nitems, bytes_after = 0;
  unsigned char *prop = NULL;
  int n_wins;
  Window *wins;
  
  if (XGetWindowProperty (panel->dpy, panel->win, 
			  panel->atoms[ATOM_MB_REQ_CLIENT_ORDER],
			  0, 1000L, False, XA_WINDOW, 
			  &actual_type, &actual_format,
			  &nitems, &bytes_after, &prop) != Success)
    return;

  wins   = (Window *)prop;
  n_wins = (int)nitems;

  papp = panel->apps_start_head;
  
  while( papp != NULL)
    {
      watermark++;
      papp = papp->next;
    }

  DBG("%s() watermark is %i\n", __func__, watermark);

  for (i =0; i<n_wins; i++)
    {
      papp = panel_app_get_from_window(panel, wins[i]);
      
      if (papp)
	{
	  MBPanelApp *found_head = NULL;
	  /* find where it currently is */
	  papp_tmp = panel->apps_start_head;

	  DBG("%s() %i looping for %s\n", __func__, i, papp->name);

	  while( papp_tmp != NULL)
	    {
	      if (papp == papp_tmp)
		{
		  found_head = panel->apps_start_head;
		  break;
		}
		
	      papp_tmp = papp_tmp->next;
	    }

	  if (!found_head)
	    {
	      papp_tmp = panel->apps_end_head;

	      while( papp_tmp != NULL)
		{
		  if (papp == papp_tmp)
		    {
		      found_head = panel->apps_end_head;
		      break;
		    }
		
		  papp_tmp = papp_tmp->next;
		}
	    }

	  if (found_head)
	    {
	      if (found_head == panel->apps_start_head)
		{
		  DBG("%s() %i found head is start\n", __func__, i );
		  panel_app_list_remove (panel, papp, &panel->apps_start_head);
		}
	      else
		{
		  DBG("%s() %i found head is end\n", __func__, i );
		  panel_app_list_remove (panel, papp, &panel->apps_end_head);

		}

	      papp->next = NULL;

	      if (!watermark || (watermark && i >= watermark))
		{
		  DBG("%s() %i prepending at end\n", __func__, i );

		  /*
		  if (found_head != panel->apps_end_head) switched lists 
		    panel_app_list_append(panel, &panel->apps_end_head, papp);
		    else
		  */
		    panel->apps_end_head = panel_app_list_prepend(panel, panel->apps_end_head, papp);
		    papp->gravity = PAPP_GRAVITY_END;
		  //panel_app_add_end(panel, papp);
		}
	      else
		{
		  DBG("%s() %i appending at start\n", __func__, i );
		  panel_app_list_append(panel, &panel->apps_start_head, papp);
		  //panel_app_add_start(panel, papp);
		  papp->gravity = PAPP_GRAVITY_START;
		}


	    }
	  else DBG("%s() %i not found head !!!!\n", __func__, i );
	}
    }

  papp = panel->apps_start_head;

  offset = panel->margin_sides;

  DBG("%s() start list:\n", __func__ );

  while( papp != NULL)
    {

      DBG("%s() %s moving to %i\n", __func__, papp->name, offset );

      if (PANEL_IS_VERTICAL(panel))
	  papp->y = offset;
      else
	  papp->x = offset;

      papp->offset = offset;

      XMoveWindow(panel->dpy, papp->win, papp->x, papp->y);      

      panel_app_deliver_config_event(panel, papp);

      offset += ( panel_app_get_size(panel, papp) + panel->padding );

      papp = papp->next;
    }

  papp = panel->apps_end_head;

  DBG("%s() end list:\n", __func__ );

  if (papp)
    {
      offset = ( PANEL_IS_VERTICAL(panel) ? panel->h : panel->w ) - panel->margin_sides;
      
      while( papp != NULL)
	{
	  offset -= ( panel_app_get_size(panel, papp) + panel->padding );
	  
	  DBG("%s() %s moving to %i\n", __func__, papp->name, offset );

	  if (PANEL_IS_VERTICAL(panel))
	      papp->y = offset;
	  else
	      papp->x = offset;
	  
	  papp->offset = offset;
	  
	  XMoveWindow(panel->dpy, papp->win, papp->x, papp->y);      

	  panel_app_deliver_config_event(panel, papp);
	  
	  papp = papp->next;
	}
     }

  session_save(panel);
  panel_menu_update_remove_items(panel);
  panel_update_client_list_prop(panel);

}

MBPanel 
*panel_init(int argc, char *argv[])
{
  int                  panel_length;
  XGCValues            gv;
  XSetWindowAttributes dattr;
  unsigned long        dattr_flags = CWBackPixel;
  XSizeHints           size_hints;
  XWMHints            *wm_hints;

  unsigned long wm_struct_vals[4];

  char *geometry_str = NULL;
  char *color_def = NULL;
  char *bg_pixmap_def = NULL;
  char *display_name = (char *)getenv("DISPLAY");

  char tray_atom_spec[128] = { 0 }; 
  char tray_id_env_str[16] = { 0 };

  /* MBPanelApp *papp_menu_button = NULL; */
  MBPanel    *panel;

  int panel_border_sz = 0;
  char *want_trans = NULL;

  char win_name[64] = { 0 };
  /*
#ifdef USE_XFT  
  XRenderColor colortmp;
#endif
  */

  int i = 0, j = 0;

  struct {
    char *name;
    MBPanelOrientation orientation;
  } orientation_lookup[] = {
    { "north", North },
    { "south", South },
    { "east",  East  },
    { "west",  West  },
    {  NULL,   North }
  };

  XSetErrorHandler(util_handle_xerror);
   
  panel = NEW(MBPanel);
  memset(panel, sizeof(MBPanel), 0);

  /* defualts */

  panel->padding             = 2;
  panel->use_session         = True;
  panel->use_alt_session_defaults = False;
  panel->use_menu            = True;
  panel->click_time          = 400;
  panel->use_overide_wins    = False;
  panel->orientation         = South;
  panel->system_tray_id      = 0;
  panel->use_flip            = True;
  panel->default_panel_size  = 0;
  panel->session_cur_gravity = PAPP_GRAVITY_START;
  panel->theme_name          = NULL;
  panel->theme_path          = NULL;
  panel->apps_start_head     = NULL;
  panel->apps_end_head       = NULL;
  panel->ignore_next_config  = False;
  panel->margin_sides        = 2;
  panel->margin_topbottom    = 2;
  panel->bg_spec             = NULL;
  panel->want_titlebar_dest  = False;

  for (i = 1; i < argc; i++) {
    if (!strcmp ("-display", argv[i]) || !strcmp ("-d", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      display_name = argv[i];
      setenv("DISPLAY", display_name, 1);
      continue;
    }
    if (!strcmp ("--no-session", argv[i]) || !strcmp ("-ns", argv[i])) {
      panel->use_session  = False;
      continue;
    }
    if (!strcmp ("--titlebar", argv[i]) || !strcmp ("-tb", argv[i])) {
      panel->want_titlebar_dest  = True;
      continue;
    }

    if (!strcmp ("--default-apps", argv[i]) || !strcmp ("-da", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      if (!strcmp(argv[i], "none"))
	session_set_defaults(panel, strdup(""));
      else
	session_set_defaults(panel, argv[i]);
      continue;
    }

    if (!strcmp ("--no-menu", argv[i]) || !strcmp ("-nm", argv[i])) {
      panel->use_menu  = False;
      continue;
    }
    if (!strcmp ("--overide-bubbles", argv[i]) || !strcmp ("-o", argv[i])) {
      panel->use_overide_wins = True;
      continue;
    }
    if (!strcmp ("--no-flip", argv[i])) {
      panel->use_flip = False;
      continue;
    }
    if (!strcmp ("--bgcolor", argv[i]) || !strcmp ("-c", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      color_def = argv[i];
      continue;
    }
    if (strstr (argv[i], "-geometry") || !strcmp ("-g", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      fprintf(stderr,"matchbox-panel: -geometry is depreciated, please consider --size  and --orientation instead\n");
      geometry_str = argv[i];
      continue;
    }
    if (strstr (argv[i], "--size") || !strcmp ("-s", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      panel->default_panel_size = atoi(argv[i]);
      if (panel->default_panel_size < 1) panel_usage (argv[0]);
      continue;
    }
    if (strstr (argv[i], "--padding") || !strcmp ("-s", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      panel->padding = atoi(argv[i]);
      if (panel->padding < 0) panel_usage (argv[0]);
      continue;
    }
    if (strstr (argv[i], "--margins")) {
      char *p = NULL;

      if (++i>=argc) 
	panel_usage (argv[0]);

      p = argv[i];

      if ((p = strchr(argv[i], ',')) != NULL)
	{
	  *p = '\0'; p++;
	}

      panel->margin_sides = atoi(argv[i]);
      if (panel->margin_sides < 0) panel_usage (argv[0]);

      if (p != NULL)
	{
	  panel->margin_topbottom = atoi(p);
	  if (panel->margin_topbottom < 0) panel_usage (argv[0]);
	}

      continue;
    }
    if (!strcmp ("--bgpixmap", argv[i]) || !strcmp ("-b", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      bg_pixmap_def = argv[i];
      continue;
    }
    if (!strcmp ("--bgtrans", argv[i]) || !strcmp ("-bt", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      want_trans = argv[i];
      continue;
    }
    if (!strcmp ("--id", argv[i]) || !strcmp ("-i", argv[i])) {
      if (++i>=argc) panel_usage (argv[0]);
      panel->system_tray_id = atoi(argv[i]);
      continue;

    }
    if (!strcmp ("--orientation", argv[i])) {
      Bool found = False;
      if (++i>=argc) panel_usage (argv[0]);
      j = 0;
      while (orientation_lookup[j].name != NULL)
	{
	  if (!strcasecmp(orientation_lookup[j].name, argv[i]))
	    {
	      panel->orientation = orientation_lookup[j].orientation;
	      found = True;
	    }
	  j++;
	}
      if (found) continue;
    }
    panel_usage(argv[0]);
  }

  if ((panel->dpy = XOpenDisplay(display_name)) == NULL)
    {
      fprintf(stderr, "%s: failed to open display", argv[0]);
      exit(1);
    }

   if (getenv("MB_SYNC")) 
     XSynchronize (panel->dpy, True);


  panel->screen = DefaultScreen(panel->dpy);
  panel->win_root = RootWindow(panel->dpy, panel->screen);

  if (panel->default_panel_size == 0)
    panel->default_panel_size 
      = ( DisplayHeight(panel->dpy, panel->screen) > 320 ) ? 36 : 20;

  panel->x = 0;
  panel->y = 0;
  panel->h = panel->default_panel_size;
  panel->w = DisplayWidth(panel->dpy, panel->screen); 

  switch (panel->orientation)
    {
    case South:
      panel->y = DisplayHeight(panel->dpy, panel->screen) - panel->h; 
      break;
    case North:
      break;
    case West:
      panel->w = panel->default_panel_size;
      panel->h = DisplayHeight(panel->dpy, panel->screen);
      break;
    case East:
      panel->w = panel->default_panel_size;
      panel->h = DisplayHeight(panel->dpy, panel->screen);
      panel->x = DisplayWidth(panel->dpy, panel->screen) - panel->w; 
      break;
    }

  panel->bg_pxm = None;

  if ( PANEL_IS_VERTICAL(panel) )
      panel_length = panel->h;
  else
      panel_length = panel->w;

  if (geometry_str)
    XParseGeometry(geometry_str, &panel->x, &panel->y, &panel->w, &panel->h);  

  /* XXX 
   * Lots of atoms now, move to xinternatoms call ...   
   */
  panel->atoms[0] = XInternAtom(panel->dpy, "_NET_WM_WINDOW_TYPE"     ,False);
   
  panel->atoms[1] = XInternAtom(panel->dpy, "_NET_WM_WINDOW_TYPE_DOCK",False);


  panel->atoms[3] = XInternAtom(panel->dpy, "_NET_SYSTEM_TRAY_OPCODE", False);
  panel->atoms[4] = XInternAtom(panel->dpy, "_XEMBED_INFO",            False);
  panel->atoms[5] = XInternAtom(panel->dpy, "_XEMBED",                 False);
  panel->atoms[6] = XInternAtom(panel->dpy, "MANAGER",                 False);

  panel->atoms[7] = XInternAtom(panel->dpy, "_MB_DOCK_ALIGN",          False);
  panel->atoms[8] = XInternAtom(panel->dpy, "_MB_DOCK_ALIGN_EAST",     False);

  panel->atoms[9] = XInternAtom(panel->dpy, "_NET_SYSTEM_TRAY_MESSAGE_DATA", False);
  panel->atoms[10] = XInternAtom(panel->dpy, "_NET_WM_WINDOW_TYPE_SPLASH", False);

  panel->atoms[11] = XInternAtom(panel->dpy, "WM_PROTOCOLS", False);
  panel->atoms[12] = XInternAtom(panel->dpy, "WM_DELETE_WINDOW", False);

  panel->atoms[13] = XInternAtom(panel->dpy, "_MB_THEME", False);

  panel->atoms[14] = XInternAtom(panel->dpy, "_MB_PANEL_TIMESTAMP", False);

  panel->atoms[15] = XInternAtom(panel->dpy, "_NET_WM_STRUT", False);

  panel->atoms[16] = XInternAtom(panel->dpy, "_MB_PANEL_BG", False);

  panel->atoms[17] = XInternAtom(panel->dpy, "WM_CLIENT_LEADER", False);

  panel->atoms[18] = XInternAtom(panel->dpy, "_NET_WM_ICON", False);

  panel->atoms[ATOM_NET_WM_PID] 
    = XInternAtom(panel->dpy, "_NET_WM_PID", False); 

  panel->atoms[ATOM_XROOTPMAP_ID] 
    = XInternAtom(panel->dpy, "_XROOTPMAP_ID", False);

  panel->atoms[ATOM_NET_SYSTEM_TRAY_ORIENTATION]
    = XInternAtom(panel->dpy, "_NET_SYSTEM_TRAY_ORIENTATION", False);

  panel->atoms[ATOM_MB_THEME_NAME] 
    = XInternAtom(panel->dpy, "_MB_THEME_NAME", False);

  panel->atoms[ATOM_MB_COMMAND] 
    = XInternAtom(panel->dpy, "_MB_COMMAND", False);

  panel->atoms[ATOM_NET_WM_NAME] 
    = XInternAtom(panel->dpy, "_NET_WM_NAME", False);

  panel->atoms[ATOM_UTF8_STRING] 
    = XInternAtom(panel->dpy, "UTF8_STRING", False);

  panel->atoms[ATOM_NET_CLIENT_LIST]
    = XInternAtom(panel->dpy, "_NET_CLIENT_LIST", False);

  panel->atoms[ATOM_NET_WM_STATE]
    = XInternAtom(panel->dpy, "_NET_WM_STATE", False);

  panel->atoms[ATOM_NET_WM_STATE_TITLEBAR]
    = XInternAtom(panel->dpy, "_MB_WM_STATE_DOCK_TITLEBAR", False);

  panel->atoms[ATOM_MB_DOCK_TITLEBAR_SHOW_ON_DESKTOP]
    = XInternAtom(panel->dpy, "_MB_DOCK_TITLEBAR_SHOW_ON_DESKTOP", False);

  panel->atoms[ATOM_MB_SYSTEM_TRAY_CONTEXT]
    = XInternAtom(panel->dpy, "_MB_SYSTEM_TRAY_CONTEXT", False);

  panel->atoms[ATOM_MB_REQ_CLIENT_ORDER]
    = XInternAtom(panel->dpy, "_MB_REQ_CLIENT_ORDER", False);


  /* Set selection atom */
  snprintf(tray_atom_spec, 128,"_NET_SYSTEM_TRAY_S%i", panel->system_tray_id); 

  panel->atoms[ATOM_SYSTEM_TRAY] 
    = XInternAtom(panel->dpy, tray_atom_spec, False);

  snprintf(tray_id_env_str, 16, "%i", panel->system_tray_id);
  setenv("SYSTEM_TRAY_ID", tray_id_env_str, 1);

  snprintf(win_name, 64, "Panel %i", panel->system_tray_id);

  panel->pb = mb_pixbuf_new(panel->dpy, panel->screen);
   
  gv.graphics_exposures = False;
  gv.function   = GXcopy;
  gv.foreground = WhitePixel(panel->dpy, panel->screen);

  panel->gc = XCreateGC(panel->dpy, panel->win_root,
		    GCGraphicsExposures|GCFunction|GCForeground, &gv);

  dattr.background_pixel = panel->xcol.pixel;

  if (geometry_str)
    {
      /* Make the window overide redirect - kind of evil hack for now */
      dattr_flags = CWBackPixel|CWOverrideRedirect;
      dattr.override_redirect = True;
    }
   
  panel->win = XCreateWindow(panel->dpy, panel->win_root,
			 panel->x, panel->y, 
			 panel->w, panel->h,
			 panel_border_sz,
			 CopyFromParent,
			 CopyFromParent,
			 CopyFromParent,
			 dattr_flags,
			 &dattr);

  XSelectInput (panel->dpy, panel->win, StructureNotifyMask|ExposureMask|
		SubstructureRedirectMask|SubstructureNotifyMask|
		ButtonPressMask|ButtonReleaseMask|PointerMotionMask|
		PropertyChangeMask);

  size_hints.flags      = PPosition | PSize | PMinSize;
  size_hints.x          = panel->x;
  size_hints.y          = panel->y;
  size_hints.width      = panel->w;
  size_hints.height     = panel->h;
  size_hints.min_width  = panel->w;
  size_hints.min_height = panel->h;
    
  XSetStandardProperties(panel->dpy, panel->win, win_name, 
			 win_name, 0, argv, argc, &size_hints);

  XChangeProperty(panel->dpy, panel->win, 
		  panel->atoms[ATOM_WM_WINDOW_TYPE], XA_ATOM, 32, 
		  PropModeReplace,
		  (unsigned char *) &panel->atoms[ATOM_WM_WINDOW_TYPE_DOCK], 
		  1);

  wm_hints = XAllocWMHints();
  wm_hints->input = False;
  wm_hints->flags = InputHint;
  XSetWMHints(panel->dpy, panel->win, wm_hints );
  XFree(wm_hints);

  if (panel->want_titlebar_dest)
    {
      Atom state_list[] 
	= { panel->atoms[ATOM_NET_WM_STATE_TITLEBAR],
	    panel->atoms[ATOM_MB_DOCK_TITLEBAR_SHOW_ON_DESKTOP] };

      int n_states = 1;

      if (getenv("MB_PANEL_NO_DESKTOP_HIDE")) n_states = 2;

      panel->use_flip = False;

      /* 
       *   We need someway ( selection ? ) to check if the 	 
       *   wm is running and only map then - not before 
       *   
       */

      XChangeProperty(panel->dpy, panel->win, 
		      panel->atoms[ATOM_NET_WM_STATE], XA_ATOM, 32, 
		      PropModeReplace, 
		      (unsigned char *) &state_list, n_states);

      panel->orientation = North;
    }

  /* Set our ewmh reserved space XXX reset this when we hide */
  wm_struct_vals[0] = ( panel->orientation == West ) ? panel->w : 0;
  wm_struct_vals[1] = ( panel->orientation == East ) ? panel->w : 0;
  wm_struct_vals[2] = ( panel->orientation == North ) ? panel->h : 0;
  wm_struct_vals[3] = ( panel->orientation == South ) ? panel->h : 0;

  XChangeProperty(panel->dpy, panel->win, panel->atoms[ATOM_NET_WM_STRUT], 
		  XA_CARDINAL, 32, PropModeReplace,
		  (unsigned char *)wm_struct_vals, 4);
  
  panel->msg_queue_start = NULL;
  panel->msg_queue_end   = NULL;
  panel->msg_win = None;   

  panel->msg_col 
    = mb_col_new_from_spec(panel->pb, DEFAULT_MSG_BGCOL);
  panel->msg_urgent_col 
    = mb_col_new_from_spec(panel->pb, DEFAULT_MSG_BGURGCOL);
  panel->msg_fg_col 
    = mb_col_new_from_spec(panel->pb, DEFAULT_MSG_FGCOL);
  panel->msg_link_col 
    = mb_col_new_from_spec(panel->pb, "blue");

  panel->msg_gc = XCreateGC(panel->dpy, panel->win_root,
			GCGraphicsExposures|GCFunction|GCForeground, &gv);

  panel->msg_font = mb_font_new_from_string(panel->dpy, MB_MSG_FONT);

  panel->next_click_is_not_double = True;
  panel->is_hidden = False;
  
  XSetWMProtocols(panel->dpy, panel->win , &panel->atoms[11], 2);

  panel->mbmenu = NULL;
  panel_menu_init(panel); 
  
  G_panel = panel;                /* global for sig handlers :(  */
  panel->reload_pending = False;

  util_install_signal_handlers();

  /* Set the theme etc */

  panel->use_themes = True;

  if (color_def != NULL) 
    { 
      panel_set_bg(panel, BG_SOLID_COLOR, color_def); 
      mb_menu_set_col(panel->mbmenu, MBMENU_SET_BG_COL, color_def);
      panel->use_themes = False;
    }
  else if (bg_pixmap_def) 
    {  
      panel_set_bg(panel, BG_PIXMAP, bg_pixmap_def); 
      panel->use_themes = False;
    }
  else if (want_trans)
    {
      panel->bg_trans = strdup(want_trans);
      
      panel_set_bg(panel, BG_TRANS,  want_trans ); 
      panel->use_themes = False;
    }
  else
    {  
      if (!panel_set_theme_from_root_prop(panel)) 
	{
	  panel_set_bg(panel, BG_SOLID_COLOR, DEFAULT_COLOR_SPEC );
	  mb_menu_set_col(panel->mbmenu, MBMENU_SET_BG_COL, DEFAULT_COLOR_SPEC );
	}
    }

  panel->click_x = 0;
  panel->click_y = 0;

#ifdef USE_XSETTINGS

  /* This will trigger callbacks instantly so called last */

  panel->xsettings_client = xsettings_client_new(panel->dpy, panel->screen,
						 panel_xsettings_notify_cb,
						 NULL,
						 (void *)panel );
#endif


  /* Below line is there to get the menu btton on the right side */
  panel->session_preexisting_lock = True;

  return panel;
}

int main(int argc, char *argv[])
{
  MBPanel *panel;

#if ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, DATADIR "/locale");
  bind_textdomain_codeset (PACKAGE, "UTF-8"); 
  textdomain (PACKAGE);
#endif

  panel = panel_init(argc, argv);

  /* Attempt to own the system tray selection  */
  if (!XGetSelectionOwner(panel->dpy, panel->atoms[ATOM_SYSTEM_TRAY]))
    {
      XSetSelectionOwner(panel->dpy, panel->atoms[ATOM_SYSTEM_TRAY],
			 panel->win, CurrentTime);
    } else {
      fprintf(stderr, "Panel already exists. aborting. Try running matchbox-panel with the --id switch.\n"); 
      exit(0);
    }

  /* Announce to any clients that are interested that we have it */
  panel_send_manage_message( panel ); 

  panel_orientation_set_hint (panel);

  XMapWindow (panel->dpy, panel->win);

  XSync(panel->dpy, False);

  session_init (panel);

  panel_main(panel);

  return 0;
}

