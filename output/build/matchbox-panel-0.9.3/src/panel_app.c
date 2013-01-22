#include "panel_app.h"

MBPanelApp*
panel_app_list_get_prev (MBPanel     *panel, 
			 MBPanelApp  *papp, 
			 MBPanelApp **list_head)
{
  MBPanelApp *tmp = *list_head;
  
  if (tmp == NULL || papp == tmp) return NULL;

  while (tmp->next != NULL)
    {
      if (tmp->next == papp) return tmp;
      tmp = tmp->next;
    }
  
  return NULL;
}

MBPanelApp*
panel_app_list_get_last (MBPanel    *panel, 
			 MBPanelApp *list_head)
{
  MBPanelApp *tmp = list_head;
  
  if (tmp == NULL) return NULL;

  while (tmp->next != NULL) tmp = tmp->next;

  return tmp;
}

MBPanelApp*
panel_app_list_prepend(MBPanel     *panel, 
		       MBPanelApp  *list_to_append_to,
		       MBPanelApp  *papp_new)
{
  papp_new->next = list_to_append_to;
  return papp_new;
}

void
panel_app_list_append (MBPanel     *panel, 
		       MBPanelApp **list_to_append_to,
		       MBPanelApp  *new_client)
{
  MBPanelApp *tmp = NULL;
  if (*list_to_append_to == NULL)
    {
      *list_to_append_to = new_client;
    }
  else
    {
      tmp = *list_to_append_to;
      while ( tmp->next != NULL ) tmp = tmp->next;
      tmp->next = new_client;
    }

  new_client->next = NULL;
}

void 	/* XXX Can probably go */
panel_app_list_insert_after(MBPanel    *panel, 
			    MBPanelApp *papp, 
			    MBPanelApp *new_papp)
{
  MBPanelApp *tmp;
  tmp = papp->next;
  papp->next = new_papp;
  new_papp->next = tmp;
}

void
panel_app_list_remove (MBPanel     *panel, 
		       MBPanelApp  *papp,
		       MBPanelApp **list_head)
{
  MBPanelApp *prev_papp = panel_app_list_get_prev(panel, papp, list_head);

  if (prev_papp == NULL) 
    {
      *list_head = papp->next;
    }
  else
    {
      prev_papp->next = papp->next;
    }
  return;
}

void
panel_app_name_get(MBPanel    *panel, 
		   MBPanelApp *papp)
{
  Atom type;
  int format;
  long bytes_after;
  long n_items;
  int result;

  result =  XGetWindowProperty (panel->dpy, papp->win, 
				panel->atoms[ATOM_NET_WM_NAME],
				0, 1024L,
				False, panel->atoms[ATOM_UTF8_STRING],
				&type, &format, &n_items,
				&bytes_after, (unsigned char **)&papp->name);

  if (result != Success 
      ||  papp->name == NULL 
      ||  type != panel->atoms[ATOM_UTF8_STRING] 
      || format != 8 
      || n_items == 0)
    {
      if (papp->name) XFree (papp->name);

      XFetchName(panel->dpy, papp->win, (char **)&papp->name);
      if (papp->name == NULL) {
	XStoreName(panel->dpy, papp->win, "<unnamed>");
	XFetchName(panel->dpy, papp->win, (char **)&papp->name);
	if (papp->name == NULL) 
	  papp->name = strdup("<unnamed>");
      }
    }
}

Window
panel_app_get_client_leader_win(MBPanel *panel, MBPanelApp *papp)
{
  Atom realType;
  unsigned long n;
  unsigned long extra;
  int format;
  int status;
  Window* value = NULL, win_result = None;
  
  status = XGetWindowProperty(panel->dpy, papp->win,
			      panel->atoms[ATOM_WM_CLIENT_LEADER],
			      0L, 16L,
			      0, XA_WINDOW, &realType, &format,
			      &n, &extra, (unsigned char **) &value);
  if (status == Success && realType == XA_WINDOW 
      && format == 32 && n == 1 && value != NULL)
    {
      win_result = (Window) value[0];
    }
  
  if (value) XFree(value);

  return win_result;
}

int*
panel_app_icon_prop_data_get(MBPanel *d, MBPanelApp *papp)
{
  Atom type;
  int format;
  long bytes_after;
  unsigned char *data = NULL;
  long n_items;
  int result;

  result =  XGetWindowProperty (d->dpy, papp->win, 
			        d->atoms[ATOM_NET_WM_ICON],
				0, 100000L,
				False, XA_CARDINAL,
				&type, &format, &n_items,
				&bytes_after, (unsigned char **)&data);

  if (result != Success || data == NULL)
    {
      if (data) XFree (data);
      DBG("%s() failed for %s (XID: %li)\n", __func__, papp->name, papp->win);
      return NULL;
    }

  return (int *)data;
}

void
panel_app_add_start(MBPanel *panel, MBPanelApp *papp_new)
{
  MBPanelApp *papp_prev;

  papp_prev = panel_app_list_get_last(panel, panel->apps_start_head);

  panel_app_list_append (panel, &panel->apps_start_head, papp_new);

  if (papp_prev != NULL)
    {
      papp_new->offset = panel_app_get_offset(panel, papp_prev) 
	+ panel_app_get_size(panel, papp_prev);
      DBG("%s() got %i = %i + %i\n", __func__, papp_new->offset, 
	  panel_app_get_offset(panel, papp_prev) ,
	  panel_app_get_size(panel, papp_prev) );
    }
  else
    papp_new->offset = 0;

  papp_new->gravity = PAPP_GRAVITY_START;
}

void
panel_app_add_end(MBPanel *panel, MBPanelApp *papp_new)
{
  MBPanelApp *papp_prev;

  papp_prev = panel_app_list_get_last(panel, panel->apps_end_head);

  panel_app_list_append (panel, &panel->apps_end_head, papp_new);

  if (papp_prev != NULL)
    papp_new->offset = panel_app_get_offset(panel, papp_prev) 
      - panel_app_get_size(panel, papp_new);
  else
    papp_new->offset = ( PANEL_IS_VERTICAL(panel) ? panel->h : panel->w ) 
      - panel_app_get_size(panel, papp_new);


  papp_new->gravity = PAPP_GRAVITY_END;
}

Bool
panel_app_check_for_space (MBPanel *panel, MBPanelApp *papp)
{
  MBPanelApp *papp_end = NULL, *papp_start = NULL;

  papp_end  = panel_app_list_get_last(panel, panel->apps_end_head);
  papp_start = panel_app_list_get_last(panel, panel->apps_start_head);

  if (papp_end && papp_start &&
      panel_app_get_offset(panel, papp_start) 
      + panel_app_get_size(panel, papp_start) 
      > panel_app_get_offset(panel, papp_end))
    {
      panel_handle_full_panel(panel, papp);
      return False;
    }
  return True;
}

MBPanelApp *
panel_app_new(MBPanel *panel, Window win, char *cmd_str)
{
  MBPanelApp        *papp;
  XWindowAttributes  attr;
  int                padding = 0;
  Bool               add_at_start = False;

  papp = NEW(MBPanelApp);
  
  papp->next   = NULL;
  papp->win    = win;
  papp->panel  = panel;
  papp->ignore = False;
  papp->ignore_unmap = 0;
  papp->icon   = NULL;

  /* XXX should check we actually get this */
  XGetWindowAttributes(panel->dpy, win, &attr);  

  papp->w = attr.width;
  papp->h = attr.height;

  if (session_preexisting_restarting(panel) && !panel->session_run_first_time)
    {
      if (panel->session_cur_gravity == PAPP_GRAVITY_START)
	add_at_start = True;
    }
  else if ( ((attr.x < 0) && !PANEL_IS_VERTICAL(panel))
	    || ((attr.y < 0) && PANEL_IS_VERTICAL(panel)))
    {
      add_at_start = True;
    }

  /* Assume square app - it can always change */

  if (panel->orientation == North || panel->orientation == South)
    {
      papp->h = panel->h - (2*panel->margin_topbottom);
      papp->w = papp->h;
    }
  else
    {
      papp->w = panel->w - (2*panel->margin_topbottom); 
      papp->h = papp->w;
    }


  if (add_at_start)
    {
      if (panel->apps_start_head == NULL)
	padding = panel->margin_sides;
      else
	padding = panel->padding;

      panel_app_add_start(panel, papp);
    }
  else
    {
      if (panel->apps_end_head == NULL)
	padding = -1 * panel->margin_sides;
      else
	padding = -1 * panel->padding;

      panel_app_add_end(panel, papp); 
    }



  DBG("%s() papp offset at %i\n", __func__, papp->offset );

  panel_app_name_get(panel, papp);

  papp->cmd_str = cmd_str;

  if (panel->orientation == North || panel->orientation == South)
    {
      papp->y = (panel->h - papp->h) / 2;
      papp->x = papp->offset + padding;
    }
  else
    {
      papp->x = (panel->w - papp->w) / 2;
      papp->y = papp->offset + padding;
    }

  if (!panel_app_check_for_space(panel, papp))
    return NULL;
  
  XResizeWindow(panel->dpy, papp->win, papp->w, papp->h);
  XReparentWindow(panel->dpy, papp->win, panel->win, papp->x, papp->y);
  panel_app_deliver_config_event(panel, papp);

  panel_update_client_list_prop (panel);

  return papp;
}

void
panel_app_handle_configure_request(MBPanel *panel, XConfigureRequestEvent *ev)
{
  XWindowChanges xwc;
  MBPanelApp *papp = NULL;

  papp = panel_app_get_from_window( panel, ev->window );

  if (panel->is_hidden) return;

  if (papp != NULL)
    {
      DBG("%s() config req  x: %i , y: %i w: %i h: %i for %s\n", 
	  __func__, ev->x, ev->y, ev->width, ev->height, papp->name );

      DBG("%s() panel is w: %i  %i h:\n", 
	  __func__, panel->w, panel->h );

      /*
      if (papp->x == ev->x && papp->y == ev->y 
	  && papp->h == ev->height && papp->w == ev->width)
	return;
      */

      if (panel->orientation == North || panel->orientation == South)
	{
	  xwc.width  = ev->width;
	  xwc.height = panel->h - (2*panel->margin_topbottom);

	  if (ev->width == ev->height) /* app wants to be square */
	    {
	      xwc.width  = xwc.height;
	    }

	  papp->y = (panel->h - papp->h) / 2;

	  if (xwc.width != papp->w)	/* Handle width changes */
	    {
	      if (papp == panel_app_list_get_last(panel, panel->apps_end_head))
		{
		  DBG("%s() papp %s is last at end\n", __func__, papp->name);
		  panel_app_move_to(panel, papp, 
				    papp->x - (xwc.width - papp->w));
		}
	      else if (papp == panel_app_list_get_last(panel, 
						       panel->apps_start_head))
		{
		  DBG("%s() papp %s is last at start\n", __func__, papp->name);
		  panel_app_move_to(panel, papp, 
				    papp->x /*+ (xwc.width - papp->w)*/);
		}
	      else if (papp == panel->apps_end_head)
		{
		  DBG("******** %s() papp %s panel->apps_end_head ****\n", __func__, papp->name);
		  panel_app_move_to(panel, papp, 
				    panel->w - panel->margin_sides - xwc.width);
		  panel_apps_nudge (panel, papp->next, papp->w - xwc.width);
		}
	      else
		{
		  DBG("%s() papp %s is nowhere special\n", __func__, papp->name);
		  panel_apps_nudge (panel, papp->next, xwc.width - papp->w);
		}
	      papp->w = xwc.width;
	    }

	  xwc.x = papp->x; 	      /* NOT allowed to move themselves */
	  xwc.y = papp->y;

	} else { 		/* East / West orientated dock */

	  xwc.width  = panel->w - (2*panel->margin_topbottom);
	  xwc.height = ev->height;

	  if (ev->width == ev->height) /* app wants to be square */
	    xwc.height  = xwc.width;

	  papp->x = (panel->w - papp->w) / 2;
	  xwc.x = papp->x;
	  xwc.y = papp->y;

	  if (xwc.height != papp->h)	/* Handle width changes */
	    {
	      if (papp == panel_app_list_get_last(panel, panel->apps_end_head))
		{
		  panel_app_move_to(panel, papp, 
				    papp->y - (xwc.height - papp->h));
		}
	      else if (papp == panel_app_list_get_last(panel, 
						       panel->apps_start_head))
		{
		  panel_app_move_to(panel, papp, 
				    papp->y /* + (xwc.height - papp->h) */);
		}
	      else
		{
		  panel_apps_nudge (panel, papp->next, xwc.height - papp->h);
		}
	      papp->h = xwc.height;
	    }
	}

      xwc.border_width = 0;
      xwc.sibling      = None;
      xwc.stack_mode   = None;

      DBG("%s() setting x: %i , y: %i w: %i h: %i \n", 
	  __func__, xwc.x, xwc.y, xwc.width, xwc.height  );

      XConfigureWindow(panel->dpy, papp->win, ev->value_mask, &xwc);
    }
}

void
panel_app_deliver_config_event(MBPanel *panel, MBPanelApp *papp)
{
   XConfigureEvent ce;

   if (panel->is_hidden) return;
   
   ce.type = ConfigureNotify;
   ce.event  = papp->win;
   ce.window = papp->win;

   if (PANEL_IS_VERTICAL(panel))
     {
       papp->x = (panel->w - papp->w) / 2;
       ce.x = papp->x;
       ce.y = papp->y;
       ce.width  = papp->w;
       ce.height = papp->h;
     } else {
       papp->y = (panel->h - papp->h) / 2;
       ce.x = papp->x;
       ce.y = papp->y; //  + panel->y;
       ce.width  = papp->w;
       ce.height = papp->h;
     }
   
   ce.border_width = 0;
   ce.above =  panel->win;
   ce.override_redirect = 0;

   DBG("%s() delivering x: %i , y: %i w: %i h: %i name : %s\n", 
       __func__, ce.x, ce.y, ce.width, ce.height, papp->name  );
   
   XSendEvent(panel->dpy, papp->win, False,
	      StructureNotifyMask, (XEvent *)&ce);
}

void 
panel_app_move_to (MBPanel    *panel, 
		   MBPanelApp *papp, 
		   int         origin_offset)
{
  if (panel->orientation == North || panel->orientation == South)
    {
      papp->x = origin_offset;
      papp->y = (panel->h - papp->h) / 2;
    }
  else
    {
      papp->y = origin_offset;
      papp->x = (panel->w - papp->w) / 2;
    }

  papp->offset = origin_offset;

  if (!panel_app_check_for_space(panel, papp))
    return;

  XMoveWindow(panel->dpy, papp->win, papp->x, papp->y);      
}

void
panel_apps_nudge (MBPanel    *panel, 
		  MBPanelApp *papp,
		  int         amount)
{
  MBPanelApp *papp_cur = papp;

  while (papp_cur != NULL)
    {
      panel_app_move_to (panel, papp_cur, 
			 panel_app_get_offset(panel, papp_cur) + amount);
      papp_cur = papp_cur->next;
    }
}

void
panel_apps_rescale (MBPanel    *panel, 
		    MBPanelApp *papp)
{
  MBPanelApp *papp_cur = papp;

  while (papp_cur != NULL)
    {
      papp->h = panel->h - (2*panel->margin_topbottom);
      XResizeWindow(panel->dpy, papp->win, papp->w, papp->h);
      panel_app_deliver_config_event(panel, papp);
      papp_cur = papp_cur->next;
    }
}

void
panel_app_destroy (MBPanel    *panel, 
		   MBPanelApp *papp)
{
   MBMenuItem *tmp = NULL;
   
   if (!papp) return;
      
   /* remove popup menu entry  XXX this functionaility should be in mbmenu */
   if (panel->remove_menu)
     tmp = panel->remove_menu->items;
   
   while (tmp != NULL)
     {
       if ((MBPanelApp *)tmp->cb_data == papp)
	 {
	   mb_menu_item_remove(panel->mbmenu, panel->remove_menu, tmp);
	   break;
	 }
       tmp = tmp->next_item;
     }

   if (papp->gravity ==  PAPP_GRAVITY_START)
     {
       panel_apps_nudge(panel, papp->next, 
			-1 *  panel_app_get_size(panel, papp));
       panel_app_list_remove(panel, papp, &panel->apps_start_head);
     }
   else
     {
       panel_apps_nudge(panel, papp->next, panel_app_get_size(panel, papp));
       panel_app_list_remove(panel, papp, &panel->apps_end_head);
     }

   if (papp->name) XFree(papp->name);

   if (papp->cmd_str) free(papp->cmd_str);

   if (papp->icon) mb_pixbuf_img_free(panel->pb, papp->icon);

   free(papp);

   panel_update_client_list_prop (panel);
}

/* Utilities */

int
panel_app_get_offset (MBPanel    *panel, 
		      MBPanelApp *papp)
{
  if (panel->orientation == East || panel->orientation == West)
    return papp->y;
  else
    return papp->x;
}

int
panel_app_get_size (MBPanel    *panel, 
		    MBPanelApp *papp)
{
  if (panel->orientation == East || panel->orientation == West)
    return papp->h;
  else
    return papp->w;
}

MBPanelApp*
panel_app_get_from_window (MBPanel *panel, 
			   Window   win)
{
   MBPanelApp *papp = panel->apps_start_head;
   DBG("%s() called, looking for win %li\n", __func__, win);

   while( papp != NULL)
   {
     DBG("%s() check %s ( %li )\n", __func__, papp->name, papp->win);
      if (papp->win == win) return papp;
      papp = papp->next;
   }

   papp = panel->apps_end_head;

   while( papp != NULL)
   {
     // DBG("%s() check %s ( %li )\n", __func__, papp->name, papp->win);
      if (papp->win == win) return papp;
      papp = papp->next;
   }

   return NULL;
}

