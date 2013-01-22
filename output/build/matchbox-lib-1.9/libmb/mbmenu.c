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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "mbmenu.h"

#define MBMAX(x,y) ((x>y)?(x):(y))

#ifdef DEBUG
#define MENUDBG(txt, args... ) fprintf(stderr, "MENU DEBUG: " txt , ##args )
#else
#define MENUDBG(txt, args... ) /* nothing */
#endif

#define WPAD 3  /* Window padding */

#define SCROLL_BUTT_H 10

#define WBW(a) ((a)->border_width) 	/* Window border width */


static MBMenuMenu *new_menu(MBMenu *mb, char *title, int depth);

static MBMenuItem *new_menu_seperator(MBMenu *mb);

static MBMenuItem *new_menu_item(MBMenu *mb, char *title, char *icon, char *info,
			       void (* cmd)( MBMenuItem *item ),
			       void *cb_data);

static MBMenuItem* menu_add_item(MBMenu *mb,MBMenuMenu *menu, MBMenuItem *item, 
			       int flags);

static void mb_menu_xmenu_paint_active_item(MBMenu *mb,MBMenuMenu *menu);

static void mb_menu_create_xmenu(MBMenu *mb,MBMenuMenu *menu, int x, int y);

static void xmenu_destroy(MBMenu *mb,MBMenuMenu *menu);

static void mb_menu_xmenu_show(MBMenu *mb,MBMenuMenu *menu);

static void xmenu_scroll_up(MBMenu *mb,MBMenuMenu *menu );

static void xmenu_scroll_down(MBMenu *mb,MBMenuMenu *menu);

static void mb_menu_xmenu_paint(MBMenu *mb,MBMenuMenu *menu);

static void remove_xmenus(MBMenu *mb,MBMenuMenu *active[]);

static void menu_set_theme_from_root_prop(MBMenu *mb);

#ifdef USE_XSETTINGS
/*
 Gtk/FontName
*/

#define XSET_UNKNOWN  0
#define XSET_GTK_FONT 1

static void
mbmenu_xsettings_notify_cb (const char       *name,
			    XSettingsAction   action,
			    XSettingsSetting *setting,
			    void             *data)
{
  MBMenu *mbmenu = (MBMenu *)data;
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
	      mb_menu_set_font (mbmenu, setting->data.v_string);
	    }
	  break;

	}
    case XSETTINGS_ACTION_DELETED:
      /* Do nothing for now */
      break;
    }
}

#endif

MBMenu *
mb_menu_new(Display *dpy, int screen) 
{
  XGCValues gv;
  XWindowAttributes root_attr;

  char *fontname = "Sans bold 14px"; /* will fall back to fixed */

  MBMenu *mbmenu = (MBMenu *)malloc(sizeof(MBMenu));
  memset(mbmenu, 0, sizeof(MBMenu));

  mbmenu->dpy    = dpy;
  mbmenu->screen = screen;
  mbmenu->root   = RootWindow(mbmenu->dpy, mbmenu->screen);
  mbmenu->pb     = mb_pixbuf_new(mbmenu->dpy, mbmenu->screen);
  mbmenu->active_depth    = 0;
  mbmenu->xmenu_is_active = False;


  mbmenu->fg_col = mb_col_new_from_spec (mbmenu->pb, "#000000");
  mbmenu->bg_col = mb_col_new_from_spec (mbmenu->pb, "#e2e2de");
  mbmenu->hl_col = mb_col_new_from_spec (mbmenu->pb, "#999999");
  mbmenu->bd_col = mb_col_new_from_spec (mbmenu->pb, "#999999");

  mbmenu->font = mb_font_new(dpy, NULL);
  mb_font_set_color(mbmenu->font, mbmenu->fg_col);

  gv.graphics_exposures = False;
  gv.function   = GXcopy;
  gv.foreground = mb_col_xpixel(mbmenu->fg_col);
  mbmenu->gc = XCreateGC(mbmenu->dpy, mbmenu->root,
			 GCGraphicsExposures|GCFunction|GCForeground, 
			       &gv);

  mbmenu->icon_dimention     = 0;     /* default: no icons */
  mbmenu->img_default_folder = NULL;
  mbmenu->img_default_app    = NULL;
  mbmenu->inner_border_width = 1;     /* bevel size */
  mbmenu->border_width       = 0;     /* 1 pixel border */
  mbmenu->trans              = 0;     /* no transparancy */
  mbmenu->img_bg             = NULL;  /* no image background */

  mbmenu->have_highlight_col = False;

  if (!mb_menu_set_font (mbmenu, fontname)) return NULL;



  mbmenu->atom_mbtheme = XInternAtom(mbmenu->dpy, "_MB_THEME", False);

  XGetWindowAttributes(mbmenu->dpy, mbmenu->root, &root_attr);

  XSelectInput(mbmenu->dpy, mbmenu->root, 
	       root_attr.your_event_mask
	       |PropertyChangeMask
	       |StructureNotifyMask);


  mbmenu->rootmenu = new_menu(mbmenu, "root", 0);

  menu_set_theme_from_root_prop(mbmenu);

#ifdef USE_XSETTINGS

  /* This will trigger callbacks instantly so called last */

  mbmenu->xsettings_client = xsettings_client_new(mbmenu->dpy, mbmenu->screen,
						  mbmenu_xsettings_notify_cb,
						  NULL,
						  (void *)mbmenu );
#endif

  return mbmenu;
}


MBMenuMenu*
mb_menu_get_root_menu(MBMenu *mbmenu)
{
  return mbmenu->rootmenu;
}

Bool
mb_menu_set_default_icons(MBMenu *mbmenu, char *folder, char *app) 
{
  MBPixbufImage *img_tmp = NULL;

  if (!mbmenu->icon_dimention) mbmenu->icon_dimention = 16;

  if (app)
    {
      if (mbmenu->img_default_app != NULL) 
	mb_pixbuf_img_free(mbmenu->pb, mbmenu->img_default_app);

      if ((mbmenu->img_default_app
	   = mb_pixbuf_img_new_from_file(mbmenu->pb, app)) == NULL)
	    {
	      if (mb_want_warnings())
		fprintf(stderr, "libmb: failed to get load image: %s\n", 
			app);
	      mbmenu->img_default_app = NULL;
	    }
      else
	{
	  if (mbmenu->img_default_app->width != mbmenu->icon_dimention 
	      || mbmenu->img_default_app->height != mbmenu->icon_dimention)
	    {
	      img_tmp = mb_pixbuf_img_scale(mbmenu->pb, 
					    mbmenu->img_default_app, 
					    mbmenu->icon_dimention, 
					    mbmenu->icon_dimention);
	      mb_pixbuf_img_free(mbmenu->pb, mbmenu->img_default_app);
	      mbmenu->img_default_app = img_tmp;
	    }
	}
    }


  if (folder)
    {
      if (mbmenu->img_default_folder != NULL) 
	mb_pixbuf_img_free(mbmenu->pb, mbmenu->img_default_folder);

      if ((mbmenu->img_default_folder
	   = mb_pixbuf_img_new_from_file(mbmenu->pb, 
					 folder)) == NULL)
	{
	  if (mb_want_warnings())
	    fprintf(stderr, "libmb: failed to get load image: %s\n", 
		    folder);
	  mbmenu->img_default_folder = NULL;
	}
      else
	{
	  if (mbmenu->img_default_folder->width != mbmenu->icon_dimention 
	      || mbmenu->img_default_folder->height != mbmenu->icon_dimention )
	    {
	      img_tmp = mb_pixbuf_img_scale(mbmenu->pb, 
					    mbmenu->img_default_folder, 
					    mbmenu->icon_dimention, 
					    mbmenu->icon_dimention);
	      mb_pixbuf_img_free(mbmenu->pb, mbmenu->img_default_folder);
	      mbmenu->img_default_folder = img_tmp;
	    }
	}
    }
  return True;
}

Bool
mb_menu_set_font (MBMenu *mbmenu, 
		  char   *font_desc)
{
  mb_font_set_from_string(mbmenu->font, font_desc);

  return True;
}

void
mb_menu_set_icon_size(MBMenu *mbmenu, int size)
{
  mbmenu->icon_dimention = size;
}

void
mb_menu_set_trans(MBMenu *mbmenu, int trans)
{
  mbmenu->trans = trans;
}

void
mb_menu_set_col(MBMenu            *mbmenu, 
		MBMenuColorElement element, 
		char              *col_spec)
{
  
  switch (element)
    {
    case MBMENU_SET_BG_COL:
      mb_col_set (mbmenu->bg_col, col_spec);
      break;
    case MBMENU_SET_FG_COL:
      mb_col_set (mbmenu->fg_col, col_spec);
      break;
    case MBMENU_SET_HL_COL:
      mb_col_set (mbmenu->hl_col, col_spec);
      break;
    case MBMENU_SET_BD_COL:
      mb_col_set (mbmenu->bd_col, col_spec);
      break;
    }
}

void
mb_menu_item_icon_set(MBMenu *mb, MBMenuItem *item, MBPixbufImage *img)
{
  if (!mb->icon_dimention) return;

  if (item->img) 
    mb_pixbuf_img_free(mb->pb, item->img); 

  item->img = mb_pixbuf_img_scale(mb->pb, img, 
				  mb->icon_dimention, 
				  mb->icon_dimention);
}

static void
mb_menu_item_free(MBMenu   *mb, 
		  MBMenuItem *item)
{
  if (item->child) 
    mb_menu_remove_menu(mb, item->child);
  
  if (item->title)   free(item->title);
  if (item->info)    free(item->info);
  if (item->icon_fn) free(item->icon_fn);
  if (item->img)     mb_pixbuf_img_free(mb->pb, item->img);

  free(item);
}

void
mb_menu_item_remove(MBMenu   *mb, 
		    MBMenuMenu     *menu, 
		    MBMenuItem *item)
{
  MENUDBG("%s() called \n", __func__);
  if (menu->items == item) 	/* first item */
    {
      MENUDBG("%s() looks like first item\n", __func__);
      if (item->next_item == NULL)
	menu->items = NULL;
      else
	menu->items = item->next_item;
    }
  else
    {
      MBMenuItem *item_tmp = menu->items;
      while( item_tmp->next_item != item && item_tmp->next_item != NULL) 
	item_tmp = item_tmp->next_item;
      
      if (item_tmp->next_item == NULL) return; /* Something gone wrong */

      item_tmp->next_item = item->next_item;
    }  
  mb_menu_item_free(mb, item); 
}

MBMenuItem *
mb_menu_new_item (MBMenu *mb, 
		 MBMenuMenu   *menu, 
		  char   *title, 
		  void  (*activate_callback)( MBMenuItem *item ),
		  void   *user_data,
		  MBMenuItemAddFlags flags
)
{
  return menu_add_item (mb, menu, 
			new_menu_item(mb, title, NULL, NULL, 
				      activate_callback, user_data),
			flags );
}


MBMenuItem * 			/* XXX TOGO replaced by above */
mb_menu_add_item_to_menu(MBMenu *mb, 
			MBMenuMenu *menu, 
			 char *title, 
			 char *icon, 
			 char *info,
			 void (* cmd)( MBMenuItem *item ),
			 void *cb_data,
			 int flags
)
{
  return menu_add_item(mb, menu, new_menu_item(mb,title,icon,info,cmd,cb_data),flags);
}


void
mb_menu_add_seperator_to_menu(MBMenu *mb,MBMenuMenu *menu, int flags)
{
  menu_add_item(mb, menu, new_menu_seperator(mb), flags);
}


MBMenuMenu *
mb_menu_add_path(MBMenu *mb, char *path, char *icon_path, int flags)
{
   char *path_cpy = strdup(path);
   char *s, *p;
   MBMenuMenu *found;
   MBMenuMenu *current = mb->rootmenu;
   MBMenuItem *item = NULL;
   MBMenuItem *new = NULL;

   p = path_cpy;

   while(*p != '\0')
   {
      s = p;
      found   = NULL;
      while(strchr("/\0", *p) == NULL) p++;
      if (*p != '\0') { *p = '\0'; p++; };

      item = current->items;
      while(item != NULL)
      {
	 if (item->child)
	    if (!strcmp(item->child->title, s))
	       found = item->child;
	 item = item->next_item;
      }

      if (found)
	{
	  current = found;
	}
      else
	{

	  new = menu_add_item(mb, current, 
			      new_menu_item(mb,s,icon_path,NULL,NULL,NULL),
			      flags);
	  new->type = MBMENU_ITEM_FOLDER;
	  new->child = new_menu(mb, s, current->depth+1);
	  new->child->parent_item = new;
	  current = new->child;
	}
   }

   if (icon_path && mb->icon_dimention)
     {
       if (new->icon_fn) free(new->icon_fn);
       new->icon_fn = strdup(icon_path); /* XXX fix */
     }

   if (path_cpy) free(path_cpy);

   return current;
}


void /* remove a menu and its children from the structure */
mb_menu_remove_menu(MBMenu *mb,MBMenuMenu *menu)
{
  MBMenuItem *item, *nxt_item;

  nxt_item = menu->items;

  while (nxt_item != NULL)
    {
      item = nxt_item;
      nxt_item = item->next_item;

      if (item->child) 
	mb_menu_remove_menu(mb, item->child);
      
      if (item->title)   free(item->title);
      if (item->info)    free(item->info);
      if (item->icon_fn) free(item->icon_fn);

      free(item);
    }

  if (menu != mb->rootmenu) 
    {
      menu->parent_item->child = NULL;
      if (menu->title)
	free(menu->title);
      free(menu);
    }
  else
    {
      menu->items=NULL;
    }
}


void
mb_menu_free(MBMenu *mb)
{
  mb_menu_remove_menu(mb, mb->rootmenu);
}


Bool
mb_menu_is_active(MBMenu *mb)
{
  return ( mb->xmenu_is_active ? True : False );
}


void
mb_menu_dump(MBMenu *mb,MBMenuMenu *menu)
{
   MBMenuItem *tmp;
   if (menu == NULL)
     menu = mb->rootmenu;

   if (menu->items == NULL) return;
   for(tmp = menu->items; tmp != NULL; tmp = tmp->next_item)
   {
      if (tmp->child != NULL) mb_menu_dump(mb, tmp->child);
   }
}


void
mb_menu_activate(MBMenu *mb, int x, int y)
{
  XGrabPointer(mb->dpy, mb->root, True,
	       (ButtonPressMask|ButtonReleaseMask),
	       GrabModeAsync,
	       GrabModeAsync, None, None, CurrentTime);
  XGrabKeyboard(mb->dpy, mb->root, True, GrabModeAsync, 
		GrabModeAsync, CurrentTime);

  mb_menu_create_xmenu(mb, mb->rootmenu, x, y);
  mb_menu_xmenu_show(mb, mb->rootmenu);
  mb->active[0] = mb->rootmenu;
  mb->active[1] = (MBMenuMenu *)NULL;

  mb->keyboard_focus_menu = mb->rootmenu;

}

void 
mb_menu_deactivate(MBMenu *mb)
{
  if (mb->xmenu_is_active)
    {
      mb->xmenu_is_active = False;
      XUngrabKeyboard(mb->dpy, CurrentTime);
      XUngrabPointer(mb->dpy, CurrentTime);
      remove_xmenus(mb, &mb->active[0]);
      mb->active_depth = 0;
      mb->keyboard_focus_menu = NULL;
    }
}

#define GET_MENU_FROM_WIN(mb,w,m) for( i=0; (mb)->active[i] != NULL; i++) \
                                   if ((mb)->active[i]->win == (w)) \
                                       (m) = (mb)->active[i]; 

static MBMenuItem *
mb_menu_item_from_coords(MBMenu *mb,MBMenuMenu *m, int x, int y)
{
  MBMenuItem *im;
  for(im = m->items; im != NULL; im=im->next_item)
    if (y > im->y && y < (im->y+im->h))
      return im;

  return NULL;
}

static void
mb_menu_open_child_menu(MBMenu *mb,MBMenuMenu *m, MBMenuItem *im)
{
  if (mb->active[im->child->depth] != NULL)
    remove_xmenus(mb, &mb->active[im->child->depth]);
  
  mb->active_depth = im->child->depth;
  mb->active[mb->active_depth] = im->child;
  mb->active[mb->active_depth+1] = NULL; 
  
  mb_menu_create_xmenu(mb, im->child,
		       m->x + m->width + WBW(mb),
		       m->y + im->y - mb->inner_border_width - WPAD );

  mb_menu_xmenu_paint(mb,im->child);
  mb_menu_xmenu_show(mb,im->child);

  if (m->active_item)
    mb_menu_xmenu_paint_active_item(mb, m);
}

static void
mb_menu_activate_item(MBMenu *mb,MBMenuMenu *m, MBMenuItem *im)
{
  if (im == NULL)
    {
      m->active_item = NULL;
      mb_menu_xmenu_paint_active_item(mb, m);
      return;
    }

  if (im->type == MBMENU_ITEM_SEPERATOR) return;

  // if (m->active_item == im) return; /* Already active */

  m->active_item = im;
  mb_menu_xmenu_paint_active_item(mb, m);

}


static void
mb_menu_active_item_execute(MBMenu *mb,MBMenuMenu *m)
{
  if (!m->active_item) return;

  /* XXX probably call mb_menu_activate_item */
  if (m->active_item->child && m->active_item->child->items) return; 

  MENUDBG("launching %s\n", m->active_item->title);
  mb_menu_deactivate(mb);
  if (m->active_item->cb != NULL)
    m->active_item->cb(m->active_item); 
}

static MBMenuItem *
mb_menu_get_item_prev(MBMenu *mb,MBMenuMenu *m, MBMenuItem *mi)
{
  MBMenuItem *tmp;
  for (tmp = m->items; tmp != NULL; tmp = tmp->next_item)
    {
      if (tmp->next_item && tmp->next_item == mi)
	return tmp;
    }

  return mi;
}

#define WANT_SCROLL_UP   1
#define WANT_SCROLL_DOWN 2

static int
mb_menu_check_scroll_button(MBMenu *mb,MBMenuMenu *m, int y_pos)
{

  MENUDBG("%s() called\n", __func__);

  if (!m->too_big) return 0;

  if (y_pos <= SCROLL_BUTT_H)
    {
      MENUDBG("%s() retruning want scroll down\n", __func__);
      return WANT_SCROLL_DOWN;
    }

  if (m->too_big_end_item
      && y_pos > (m->too_big_end_item->y+m->too_big_end_item->h))
    return WANT_SCROLL_UP;

  return 0 ;

}

static void
menu_set_theme_from_root_prop(MBMenu *mb)
{
  Atom realType;
  unsigned long n;
  unsigned long extra;
  int format;
  int status;
  char * value;
  struct stat stat_info;
  char app_cfg[256];

  status = XGetWindowProperty(mb->dpy,
			      mb->root,
			      mb->atom_mbtheme,
			      0L, 512L, False,
			      AnyPropertyType, &realType,
			      &format, &n, &extra,
			      (unsigned char **) &value);
	    
  if (status != Success || value == 0
      || *value == 0 || n == 0)
    {
      if (mb_want_warnings())
	fprintf(stderr, "mbmenu: no _MB_THEME set on root window\n");
    } else {
      strcpy(app_cfg, value);
      strcat(app_cfg, "/theme.desktop");
      if (stat(app_cfg, &stat_info) != -1)
	{
	  MBDotDesktop *theme  = NULL;
	  theme = mb_dotdesktop_new_from_file(app_cfg);
	  if (theme)
	    {

	      if (mb_dotdesktop_get(theme, "MenuBgColor"))
		{
		  mb_menu_set_col(mb, MBMENU_SET_BG_COL, 
				  (char*)mb_dotdesktop_get(theme, "MenuBgColor"));
		}

	      if (mb_dotdesktop_get(theme, "MenuFont"))
		{
		  mb_menu_set_font (mb, 
				    (char*)mb_dotdesktop_get(theme, "MenuFont"));
		}

	      if (mb_dotdesktop_get(theme, "MenuFgColor"))
		{
		  mb_menu_set_col(mb, MBMENU_SET_FG_COL, 
				  (char*)mb_dotdesktop_get(theme, "MenuFgColor"));
		}

	      if (mb_dotdesktop_get(theme, "MenuHlColor"))
		{
		  mb_menu_set_col(mb, MBMENU_SET_HL_COL, 
				  (char*)mb_dotdesktop_get(theme, "MenuHlColor"));
		    mb->have_highlight_col = True;
		}
	      else mb->have_highlight_col = False;

	      if (mb_dotdesktop_get(theme, "MenuBdColor"))
		{
		  mb_menu_set_col(mb, MBMENU_SET_BD_COL, 
				  (char*)mb_dotdesktop_get(theme, "MenuBdColor"));
		}

	      /* xxx currently broke xxx
	      if (mb_dotdesktop_get(theme, "MenuTransparency"))
		{
		  mb_menu_set_trans(mb,  
				    atoi(mb_dotdesktop_get(theme, 
						      "MenuTransparency")));
		}
	      */
	      mb_dotdesktop_free(theme);
	    }
	}
    }

  if (value) XFree(value);
  return;
}


void
mb_menu_handle_xevent(MBMenu *mb, XEvent *an_event)
{
 MBMenuMenu *m = NULL;
  int i, scroll_state = 0;
  KeySym key;
  MBMenuItem *im;

  static Bool button_pressed = False;
  static Bool had_cancel_press = False;


  /* First handle any keyremaps - ie cursor keys changing on rotation */
  if (an_event->type == MappingNotify)
    {
      XRefreshKeyboardMapping((XMappingEvent *)an_event);
      return;
    }

#ifdef USE_XSETTINGS
  if (mb->xsettings_client != NULL)
    xsettings_client_process_event(mb->xsettings_client, an_event);
#endif

  if (an_event->type == PropertyNotify 
      && an_event->xproperty.atom == mb->atom_mbtheme) 
    {
      menu_set_theme_from_root_prop(mb);
      return;
    }

  if (!mb->xmenu_is_active) return;

  switch (an_event->type)
    {
    case KeyPress:
      MENUDBG("%s() Keyevent recieved\n", __func__ );
      switch (key = XKeycodeToKeysym (mb->dpy, an_event->xkey.keycode, 0))
	{
	case XK_Left:
	  if (mb->active_depth > 0)
	    {
	      remove_xmenus(mb, &mb->active[mb->active_depth]);
	      mb->keyboard_focus_menu = mb->active[--mb->active_depth];
	      mb_menu_xmenu_paint_active_item(mb, mb->keyboard_focus_menu);

	    }
	  break;
	case XK_Right:
	    if (mb->keyboard_focus_menu->active_item
		&& mb->keyboard_focus_menu->active_item->child 
		&& mb->keyboard_focus_menu->active_item->child->items)
	      {
		mb_menu_open_child_menu(mb, mb->keyboard_focus_menu, 
				      mb->keyboard_focus_menu->active_item);
		mb->keyboard_focus_menu = mb->active[mb->active_depth];
	      mb->keyboard_focus_menu->active_item 
		= mb->keyboard_focus_menu->items;
	      mb_menu_xmenu_paint_active_item(mb, mb->keyboard_focus_menu);

	      }

	  break;
	case XK_Up:
	  im = NULL;
	  if (mb->keyboard_focus_menu->active_item)
	    im = mb_menu_get_item_prev(mb, 
				       mb->keyboard_focus_menu,
				       mb->keyboard_focus_menu->active_item);


	  if (im == NULL)  
	    im = (mb->keyboard_focus_menu->too_big_start_item) ? mb->keyboard_focus_menu->too_big_start_item : mb->keyboard_focus_menu->items;

	  if (mb->keyboard_focus_menu->too_big_start_item
	      && im->next_item == mb->keyboard_focus_menu->too_big_start_item)
	    {
	      MENUDBG("keyboard scroll up\n");
	      xmenu_scroll_down(mb, mb->keyboard_focus_menu);

	      if (im->type == MBMENU_ITEM_SEPERATOR)
		{
		  xmenu_scroll_down(mb, mb->keyboard_focus_menu);
		  im = mb_menu_get_item_prev(mb, mb->keyboard_focus_menu, im);
		}

	      mb->keyboard_focus_menu->active_item = im;
	      mb_menu_xmenu_paint(mb, mb->keyboard_focus_menu);
	    }
	  else
	    {
	      if (im && im->type == MBMENU_ITEM_SEPERATOR)
		im = mb_menu_get_item_prev(mb, mb->keyboard_focus_menu, im);
	    }

	  mb->keyboard_focus_menu->active_item = im;
	  mb_menu_xmenu_paint_active_item(mb, mb->keyboard_focus_menu);
	  break;
	case XK_Down:
	  im = NULL;
	  if (mb->keyboard_focus_menu->active_item)
	    {
	      im = mb->keyboard_focus_menu->active_item->next_item;

	      /* We dont loop to beginning on scrolled menu */
	      if (mb->keyboard_focus_menu->active_item->next_item == NULL
		  && mb->keyboard_focus_menu->too_big)
		break;
	    }

	  if (im == NULL)  
	    im = (mb->keyboard_focus_menu->too_big_start_item) ? mb->keyboard_focus_menu->too_big_start_item : mb->keyboard_focus_menu->items;

	  if (mb->keyboard_focus_menu->too_big_end_item
	      && mb->keyboard_focus_menu->too_big_end_item->next_item == im)
	    {
	      MENUDBG("keyboard scroll down\n");
	      xmenu_scroll_up(mb, mb->keyboard_focus_menu);
	      
	      if (im->type == MBMENU_ITEM_SEPERATOR)
		{
		  xmenu_scroll_up(mb, mb->keyboard_focus_menu);
		  im = im->next_item;
		}

	      mb->keyboard_focus_menu->active_item = im;
	      mb_menu_xmenu_paint(mb, mb->keyboard_focus_menu);
	    }
	  else
	    {
	      if (im->type == MBMENU_ITEM_SEPERATOR)
		im = im->next_item;
	    }


	  mb->keyboard_focus_menu->active_item = im;
	  mb_menu_xmenu_paint_active_item(mb, mb->keyboard_focus_menu);

	  /* mb_menu_activate_item(mb, mb->keyboard_focus_menu, im); */
	  break;
	case XK_Return:
	case XK_KP_Enter:
	  mb_menu_active_item_execute(mb, mb->keyboard_focus_menu);
	  break;
	case XK_Escape:
	  mb_menu_deactivate(mb);
	  button_pressed = False;
	  break;
	}
      break;
    case LeaveNotify:

      if (!button_pressed) break; /* For touchscreens */

      MENUDBG("got leave\n");

      GET_MENU_FROM_WIN(mb, an_event->xcrossing.window, m);

      if (m == NULL) break;

      /* Only remove highligh if pointer has moved off an actual menu */
      if (!XCheckTypedEvent(mb->dpy, EnterNotify, an_event))
	mb_menu_activate_item(mb, m, NULL);

      break;

    case MotionNotify:
      
      if (!button_pressed) break; /* For touchscreens */

      /* Compress motion notifys a little */
      while ( XEventsQueued(mb->dpy, QueuedAfterReading) > 0 )
	{
	  XEvent ahead;
	  XPeekEvent( mb->dpy, &ahead);
	  if ( ahead.type != MotionNotify ) break;
	  if ( ahead.xmotion.window != an_event->xmotion.window ) break;
	  XNextEvent ( mb->dpy, an_event );
	}


      GET_MENU_FROM_WIN(mb, an_event->xmotion.window, m);

      if (m == NULL) break;

      im = mb_menu_item_from_coords(mb, m, 
				    an_event->xmotion.x, 
				    an_event->xmotion.y);

      scroll_state = mb_menu_check_scroll_button(mb, m, an_event->xmotion.y);
      
      if (scroll_state) return;

      mb_menu_activate_item(mb, m, im);
      mb->keyboard_focus_menu = mb->active[mb->active_depth];

      break;
    case ButtonRelease:
      button_pressed = False;

      GET_MENU_FROM_WIN(mb, an_event->xmotion.window, m);
      
      if (m == NULL)
	{
	  if (had_cancel_press) 
	    {
	      mb_menu_deactivate(mb);
	      had_cancel_press = False;
	    }
	  break;
	}

      scroll_state = mb_menu_check_scroll_button(mb, m, an_event->xmotion.y);
      
      if (scroll_state == WANT_SCROLL_DOWN) 
	{
	  xmenu_scroll_down(mb,m);
	  mb_menu_xmenu_paint(mb, m);
	  return;
	}

      if (scroll_state == WANT_SCROLL_UP) 
	{
	  xmenu_scroll_up(mb,m);
	  mb_menu_xmenu_paint(mb, m);
	  return;
	}

      mb_menu_active_item_execute(mb, m);

      if (m->active_item
	  && m->active_item->child 
	  && m->active_item->child->items)    
	{
	  mb_menu_open_child_menu(mb, m, m->active_item);
	}

      mb->keyboard_focus_menu = mb->active[mb->active_depth];

      break;
    case ButtonPress:
      button_pressed = True;

      MENUDBG("button down %i, %i\n", an_event->xmotion.x,
	  an_event->xmotion.y);
      if (mb->xmenu_is_active) {

	GET_MENU_FROM_WIN(mb, an_event->xmotion.window, m);

      if (m == NULL)
	{ 
	  had_cancel_press = True;
	  //mb_menu_deactivate(mb);
	  // button_pressed = False;
	  break;
	}


      scroll_state = mb_menu_check_scroll_button(mb, m, an_event->xmotion.y);
      
      if (scroll_state) return;
      
      im = mb_menu_item_from_coords(mb, m, 
				    an_event->xmotion.x, 
				    an_event->xmotion.y);

      mb_menu_activate_item(mb, m, im);
      mb->keyboard_focus_menu = mb->active[mb->active_depth];

      }
      break;
    case Expose:
      if (an_event->xexpose.count != 0 )
	break;
      if (mb->xmenu_is_active)
	{
	  GET_MENU_FROM_WIN(mb, an_event->xexpose.window, m);
	  if (m)
	    mb_menu_xmenu_paint(mb, m);
	}
      break;

    }
  
}

void*
mb_menu_item_get_user_data(MBMenuItem *item)
{
  return item->cb_data;
}

/* ------------------------ Private Calls ---------------------------- */

static MBMenuMenu *
new_menu(MBMenu *mb, char *title, int depth)
{
  MBMenuMenu *menu = (MBMenuMenu *)malloc(sizeof(MBMenuMenu));
   memset(menu, 0, sizeof(menu));
   menu->items = NULL;

   MENUDBG("adding menu -> %s, (%i) \n", title, depth);

   menu->depth = depth;
   menu->too_big = False;
   menu->too_big_start_item = NULL;
   menu->too_big_end_item = NULL;

   menu->title = (char *)malloc(sizeof(char)*(strlen(title)+1));
   strcpy(menu->title, title);

   menu->active_item_drw = NULL;
   return menu;
}

static MBMenuItem *
new_menu_seperator(MBMenu *mb)
{
   MBMenuItem *menu_item = new_menu_item(mb, "--", NULL, NULL, 
				       NULL, NULL );
   menu_item->type = MBMENU_ITEM_SEPERATOR; 
   return menu_item;
}

static MBMenuItem *
new_menu_item(MBMenu *mb, char *title, char *icon, char *info,
	      void (* cmd)( MBMenuItem *item ),
	      void *cb_data
	      )
{
  MBPixbufImage *img_tmp;
  MBMenuItem *menu_item = (MBMenuItem *)malloc(sizeof(MBMenuItem));
   
   menu_item->type      = MBMENU_ITEM_APP; 
   menu_item->next_item = NULL;
   menu_item->child     = NULL;
   menu_item->info      = NULL;
   menu_item->cb        = NULL;
   menu_item->cb_data   = NULL;
   menu_item->icon_fn   = NULL;
   menu_item->img       = NULL;

   if (title == NULL) 
     {
       if (mb_want_warnings())
	 fprintf(stderr, "Cant create menu with no title\n"); 
       exit(0);
     }

   MENUDBG("adding menu item -> %s\n", title);
   
   menu_item->title = strdup(title);

   if (info != NULL)
   {
      menu_item->info = (char *)malloc(sizeof(char)*(strlen(info)+1));
      strcpy(menu_item->info, info);
   }

   if (cmd != NULL)
     {
       menu_item->cb = cmd;
       if (cb_data != NULL)
	 menu_item->cb_data = cb_data;
     }

   if (icon != NULL && mb->icon_dimention)
   {
      menu_item->icon_fn = strdup(icon);

      if ((menu_item->img = mb_pixbuf_img_new_from_file(mb->pb, 
							menu_item->icon_fn)) 
	  != NULL)
	{
	  if (menu_item->img->width != mb->icon_dimention 
	      || menu_item->img->height != mb->icon_dimention )
	    {
	      img_tmp = mb_pixbuf_img_scale(mb->pb, menu_item->img, 
					    mb->icon_dimention, 
					    mb->icon_dimention);
	      mb_pixbuf_img_free(mb->pb, menu_item->img);
	      menu_item->img = img_tmp;
	    }
	}
      else
	{
	  if (mb_want_warnings())
	    fprintf(stderr, "failed to load image: %s \n", menu_item->icon_fn);
	  free(menu_item->icon_fn);
	  menu_item->icon_fn = (char *)NULL;
	}
   }

   return menu_item;
}

static MBMenuItem*
menu_add_item(MBMenu *mb,MBMenuMenu *menu, MBMenuItem *item, int flags)
{
   MBMenuItem *tmp = NULL;
   if (menu->items == NULL) 
   {
      menu->items = item;
   } else {
      MBMenuItem *prev = NULL;
      if (item->type == MBMENU_ITEM_SEPERATOR
	  || flags & MBMENU_NO_SORT || flags & MBMENU_PREPEND)
	{
	  if (flags & MBMENU_PREPEND)
	    {
	      if (menu->items)
		item->next_item = menu->items;
	      menu->items = item;
	    }
	  else
	    {
	      for(tmp = menu->items; 
		  tmp->next_item != NULL; 
		  tmp = tmp->next_item);
	      tmp->next_item = item;
	    }
	} else {
	  for(tmp = menu->items;
	      tmp->next_item != NULL;
	      tmp = tmp->next_item)
	    {
	      if (strcoll(tmp->title, item->title) > 0)
		{
		  if (prev == NULL)
		    {
		      item->next_item = menu->items;
		      menu->items = item;
		    } else {
		      item->next_item = tmp;
	             prev->next_item = item;
		    }
		  return item;
		}
	      prev = tmp;
	    }
	  tmp->next_item = item;
	}
   }
   return item;
}


static void
_mb_menu_get_x_menu_geom(MBMenu     *mb, 
			 MBMenuMenu *menu,
			 int *width_ret, int *height_ret)
{
   MBMenuItem *item, *item_tmp = NULL;
   int   maxw      = 0;
   int   height    = WPAD + mb->inner_border_width;
   char *tmp_title;

   /* 
      XXX Hack to remove any empty folders from being rendered.
      This could be very dangerous and needs improving. 
   */
   item_tmp = menu->items;
   while (item_tmp != NULL)
     {
       if (item_tmp->child 
	   && item_tmp->child->items == NULL )
	 {
	   MBMenuItem *prev = mb_menu_get_item_prev(mb, menu, item_tmp);
	   if (prev)
	     {
	       prev->next_item = item_tmp->next_item;
	     }
	   else menu->items = item_tmp->next_item;
	 }
       item_tmp = item_tmp->next_item;
     }

   for(item = menu->items; item != NULL; item = item->next_item)
   {
     int text_width = 0;
      tmp_title = item->title;
     
      if (item->type == MBMENU_ITEM_SEPERATOR)
	{
	  item->y = height;
	  item->h = 6;
	  height += item->h;
	  continue;
	}

      text_width = mb_font_get_txt_width (mb->font, 
					  (unsigned char *) tmp_title,
					  strlen(tmp_title),
					  MB_ENCODING_UTF8);

      if ((text_width + mb->icon_dimention + (2*WPAD)) > maxw)
	maxw = (text_width + mb->icon_dimention + (2*WPAD));

      item->y = height;
      item->h = MBMAX(mb_font_get_height(mb->font) + 2,
		    mb->icon_dimention);

      height += item->h;
   }

   height += (WPAD + mb->inner_border_width);
   if (mb->icon_dimention) maxw += 2;    /* space between icon & text */
   maxw += (WPAD + (2*mb->inner_border_width)+8); /* 8 is width of arrow xpm */

   *height_ret = height;
   *width_ret   = maxw;

}

Bool
mb_menu_get_root_menu_size(MBMenu *mb, int *w, int *h)
{

  if (mb->rootmenu->items == NULL)
    {
      *w = 0;
      *h = 0;
      return False;
    }

  _mb_menu_get_x_menu_geom(mb, mb->rootmenu, w, h);
  return True;
}

static void
mb_menu_create_xmenu(MBMenu     *mb, 
		     MBMenuMenu *menu, 
		     int x, int y)
{

   XSetWindowAttributes attr;
   XWindowAttributes root_attr;
   MBMenuItem *item;
   int maxw = 0;
   int height;

   menu->active_item = (MBMenuItem*)NULL;
   menu->backing = NULL;

   if (menu->items == NULL) return;

   _mb_menu_get_x_menu_geom(mb, menu, &maxw, &height); 

   XGetWindowAttributes(mb->dpy, mb->root, &root_attr);
   
   attr.override_redirect = True;
   attr.background_pixel  = mb_col_xpixel(mb->bg_col);
   attr.event_mask        = ButtonPressMask| ButtonReleaseMask|ExposureMask|
                           EnterWindowMask|LeaveWindowMask|PointerMotionMask;

   if (height <= root_attr.height)  /* Does menu fit ? */
   {
     if (menu == mb->rootmenu) // && (y - height) <= 0 )
       {
	 //if ((y - height) > 0)
	   y = y - height;
	   if (y < 0) {
	     y = 0;
	   }
       } else {
	 if ( (y+height) > root_attr.height)
	   {
	     y = ( root_attr.height - height - (2*WBW(mb)) 
		   - mb->inner_border_width);
	   } else {
	     if (y < 0) y = (mb->rootmenu->y);
	   }
       }
      menu->too_big = False;
      
   } else {
 
     y = 0;
     height = root_attr.height - 2*WBW(mb);
     menu->too_big            = True;
     menu->too_big_start_item = menu->items;
     menu->too_big_end_item   = NULL;

     for(item = menu->items; item != NULL; item = item->next_item)
       {
	 item->y += SCROLL_BUTT_H;

	 if (menu->too_big_end_item == NULL
	     && item->next_item
	     && (item->next_item->y+item->next_item->h+SCROLL_BUTT_H) > (height - SCROLL_BUTT_H))
	   {
	     menu->too_big_end_item = item;
	     MENUDBG("too big item is : '%s'\n",
		     menu->too_big_end_item->title);

	   }

       }
   }

   if ((x+maxw) > root_attr.width)
     {
       if (mb->active_depth)
	 {
	   x = mb->active[mb->active_depth-1]->x - maxw - (2*WBW(mb));
	   if (x < 0) x = root_attr.width - maxw - (2*WBW(mb));
	 } else {
	   x = root_attr.width - maxw - WBW(mb);
	 }
       if (x < 0) x = 0;
     }

   /* XXX */

   menu->x = x;
   menu->y = y;
   menu->height = height;
   menu->width  = maxw;
   
   MENUDBG("creating menu ay %i x %i", menu->x, menu->y);
   
   menu->win = XCreateWindow(mb->dpy, mb->root, menu->x, menu->y,
			     maxw, menu->height, mb->border_width,
			     CopyFromParent,
			     CopyFromParent,
			     CopyFromParent,
			     CWOverrideRedirect|CWBackPixel|CWEventMask
			     |CWEventMask, &attr);
}

static void
xmenu_destroy(MBMenu *mb,MBMenuMenu *menu)
{
   MBMenuItem *item;
   for(item = menu->items; item != NULL; item = item->next_item)
   {
      if (item->img && mb->icon_dimention)
      {
	 if (menu->backing != NULL) 
	     mb_drawable_unref(menu->backing);
	 menu->backing = NULL;
	 /* if (item->img) mb_pixbuf_img_free(mb->pb, item->img); */
      }
   }
   XDestroyWindow(mb->dpy, menu->win);
}

static void
mb_menu_xmenu_show(MBMenu *mb, MBMenuMenu *menu)
{
  mb->xmenu_is_active = True;
  XMapWindow(mb->dpy, menu->win);
}

static void 
xmenu_scroll_up(MBMenu *mb, MBMenuMenu *menu )
{
   MBMenuItem *item;
   int         height = 0;

   MENUDBG("%s() called, height is %i\n", __func__, height);

   if (menu->too_big_end_item->next_item == NULL) return;

   height = menu->too_big_start_item->h;

   for(item = menu->items; item->next_item != NULL; item = item->next_item);
   
   MENUDBG("%s() item->y : %i less than menu->height: %i ? too bi : '%s'\n",
	   __func__, item->y, menu->height, menu->too_big_end_item->title );
   
   // if (item->y < menu->height) return;
   
   // if (menu->too_big_start_item->next_item == NULL) return;
       
   menu->too_big_start_item = menu->too_big_start_item->next_item;
       
   menu->too_big_end_item = menu->too_big_end_item->next_item;
   
   for(item = menu->items; item != NULL; item = item->next_item)
     item->y -= height;
       

   if (menu->backing != NULL) 
     mb_drawable_unref(menu->backing);

   menu->backing = NULL;
}

static void 
xmenu_scroll_down(MBMenu *mb,MBMenuMenu *menu)
{
   MBMenuItem *item;
   int height = menu->too_big_start_item->h;

   MENUDBG("%s() called\n", __func__);

   if (menu->items == menu->too_big_start_item)
     {
       MENUDBG("%s() nothing to scroll\n", __func__);
      return;  /* nothing to scroll down  */
     }

   for(item = menu->items; item != NULL; item = item->next_item)
     if (item->next_item == menu->too_big_start_item)
       {
	 menu->too_big_start_item = item;
	 break;
       }
   
   for(item = menu->items; item != NULL; item = item->next_item)
     if (item->next_item == menu->too_big_end_item)
       {
	 menu->too_big_end_item = item;
	 break;
       }
       

   for(item = menu->items; item != NULL; item = item->next_item)
      item->y += height;

   if (menu->backing != NULL) 
     mb_drawable_unref(menu->backing);

   menu->backing = NULL;

}

static void
mb_menu_xmenu_paint_active_item(MBMenu *mb,MBMenuMenu *menu)
{
  MBPixbufImage *img = NULL;
  static struct _menuitem *last_item = NULL;

  if (menu->active_item != last_item)
    XClearWindow(mb->dpy, menu->win);

  if (menu->active_item_drw != NULL)
    {
      mb_drawable_unref(menu->active_item_drw);
      menu->active_item_drw = NULL;
    }

  if (menu->active_item == NULL) return;

  last_item = menu->active_item;

  menu->active_item_drw = mb_drawable_new( mb->pb, menu->width - 4, 
					   menu->active_item->h);

  img = mb_pixbuf_img_new( mb->pb, menu->width - 4, menu->active_item->h);

  if (mb->have_highlight_col)
    {
      mb_pixbuf_img_fill(mb->pb, img,
			 mb_col_red(mb->hl_col),  
			 mb_col_green(mb->hl_col),
			 mb_col_blue(mb->hl_col), 0);

    } else {
      mb_pixbuf_img_fill(mb->pb, img,
			 mb_col_red(mb->bd_col),  
			 mb_col_green(mb->bd_col),
			 mb_col_blue(mb->bd_col), 0);

    }


  if (img)
    {
      /* Curve edges - not for image bg ? */
      mb_pixbuf_img_plot_pixel(mb->pb, img, 0, 0, 
			       mb_col_red(mb->bg_col),  
			       mb_col_green(mb->bg_col),
			       mb_col_blue(mb->bg_col));

      mb_pixbuf_img_plot_pixel(mb->pb, img, 0, menu->active_item->h-1, 
			       mb_col_red(mb->bg_col),  
			       mb_col_green(mb->bg_col),
			       mb_col_blue(mb->bg_col));

      mb_pixbuf_img_plot_pixel(mb->pb, img, menu->width-5, 0, 
			       mb_col_red(mb->bg_col),  
			       mb_col_green(mb->bg_col),
			       mb_col_blue(mb->bg_col));

      mb_pixbuf_img_plot_pixel(mb->pb, img, 
			       menu->width-5, menu->active_item->h-1, 
			       mb_col_red(mb->bg_col),  
			       mb_col_green(mb->bg_col),
			       mb_col_blue(mb->bg_col));


     if (mb->icon_dimention)
       {
	 if (menu->active_item->img != NULL)
	   {
	     mb_pixbuf_img_composite(mb->pb, img, menu->active_item->img,
				     WPAD + mb->inner_border_width - 2, 0);
	   } else {
	     if (mb->img_default_folder && menu->active_item->child != NULL)
	       {
		 mb_pixbuf_img_composite(mb->pb, img, 
					 mb->img_default_folder,
					 WPAD + mb->inner_border_width - 2, 
					 0);
	       }
	     else if ( mb->img_default_app 
		       && menu->active_item->child == NULL )
	       {
		 mb_pixbuf_img_composite(mb->pb, img, 
					 mb->img_default_app,
					 WPAD + mb->inner_border_width - 2, 
					 0);
	       }
	   }
       }

     if (menu->active_item->child)
       {
	 int y;
	 for (y = (menu->active_item->h/2) - 2; 
	      y < (menu->active_item->h/2) + 3; y++)
	   {
	     mb_pixbuf_img_plot_pixel(mb->pb,img, 
				      menu->width - 10, y,
				      mb_col_red(mb->bg_col),  
				      mb_col_green(mb->bg_col),
				      mb_col_blue(mb->bg_col));
	   }
	 for (y = (menu->active_item->h/2) - 1; 
	      y < (menu->active_item->h/2) + 2; y++)
	   {
	     mb_pixbuf_img_plot_pixel(mb->pb, img, 
				     menu->width - 9, y,
				     mb_col_red(mb->bg_col),  
				     mb_col_green(mb->bg_col),
				     mb_col_blue(mb->bg_col));

	   }
	 mb_pixbuf_img_plot_pixel(mb->pb, img, 
				 menu->width - 8, (menu->active_item->h/2),
				 mb_col_red(mb->bg_col),  
				 mb_col_green(mb->bg_col),
				 mb_col_blue(mb->bg_col));

       }


     mb_pixbuf_img_render_to_drawable(mb->pb, img, 
				      mb_drawable_pixmap(menu->active_item_drw), 
				      0, 0);

     /* XXX set color */

     mb_font_render_simple (mb->font, 
			    menu->active_item_drw,
			    WPAD + mb->inner_border_width + mb->icon_dimention,
			    0,
			    menu->width,
			    (unsigned char *) menu->active_item->title,
			    MB_ENCODING_UTF8,
			    0);

      XCopyArea(mb->dpy,  mb_drawable_pixmap(menu->active_item_drw), 
		menu->win, mb->gc, 0, 0,
		menu->width, menu->active_item->h, 
		2, menu->active_item->y);

      mb_pixbuf_img_free(mb->pb, img);
    }
}

static void
xmenu_paint_arrow(MBMenu *mb,MBMenuMenu *menu, int direction)
{
   XPoint pts[3];
   int mid = (menu->width/2);
   if (direction == 1) 	 /* UP */
   {
      pts[0].x = mid - 5;
      pts[0].y = 10;
      pts[1].x = mid;
      pts[1].y = 5;
      pts[2].x = mid + 5;
      pts[2].y = 10;
   } else {
      pts[0].x = mid - 5;
      pts[0].y = menu->height - 10;
      pts[1].x = mid;
      pts[1].y = menu->height - 5;
      pts[2].x = mid + 5;
      pts[2].y = menu->height - 10;
   }

   XSetForeground(mb->dpy, mb->gc, BlackPixel(mb->dpy, mb->screen));
   XSetLineAttributes(mb->dpy, mb->gc, 2, LineSolid, CapRound, JoinRound);
   XDrawLines(mb->dpy, mb_drawable_pixmap(menu->backing), mb->gc, pts, 3, CoordModeOrigin);
   XSetLineAttributes(mb->dpy, mb->gc, 1, LineSolid, CapRound, JoinRound);
}


static void
mb_menu_xmenu_paint(MBMenu *mb,MBMenuMenu *menu)
{

   MBMenuItem *item;
   MBMenuItem *start_item = menu->items;
   
   MBPixbufImage *img_dest;

   char *tmp_title;
   int x,y;
   int sx;

   if (menu->items == NULL) return;

   if (menu->backing != NULL) return; /* Cached */

   menu->backing = mb_drawable_new(mb->pb, menu->width, menu->height);

   if (mb->trans)
     {
       MENUDBG("calling mb_pixbuf_img_new_from_drawable with %i,%i - %ix%i\n",
	       menu->x + mb->border_width, 
	       menu->y + mb->border_width, 
	       menu->width, 
	       menu->height);
       img_dest = mb_pixbuf_img_new_from_drawable(mb->pb, mb->root, 
						  None, 
						  menu->x + mb->border_width, 
						  menu->y + mb->border_width, 
						  menu->width, 
						  menu->height);

     } else {
       img_dest = mb_pixbuf_img_new(mb->pb, menu->width, menu->height);
      } 

   if (img_dest == NULL) return; /* Something has gone wrong */

   /* Background  */
   if (mb->img_bg)  
     {
       int dx, dy, dw, dh;
       for (dy=0; dy < menu->height;  dy += mb->img_bg->height)
	 for (dx=0; dx < menu->width; dx += mb->img_bg->width)
	   {
	     if ( (dx + mb->img_bg->width) > menu->width )
	       dw = mb->img_bg->width - ((dx + mb->img_bg->width)-menu->width);
	     else
	       dw = mb->img_bg->width;
	     
	     if ( (dy + mb->img_bg->height) > menu->height )
	       dh = mb->img_bg->height-((dy+mb->img_bg->height)-menu->height);
	     else
	       dh = mb->img_bg->height;

	     if (mb->trans)
	       mb_pixbuf_img_copy_composite(mb->pb, img_dest, mb->img_bg, 
					    0, 0, dw, dh, dx, dy);
	     else
	       mb_pixbuf_img_copy(mb->pb, img_dest, mb->img_bg, 
				  0, 0, dw, dh, dx, dy);
	  }
     }
   else
     {
       if (mb->trans)
	 {
	   for (x = 0; x < menu->width; x++)
	     for (y = 0; y < menu->height; y++)
	       mb_pixbuf_img_plot_pixel_with_alpha(mb->pb, img_dest, x, y, 
						   mb_col_red(mb->bg_col),  
						   mb_col_green(mb->bg_col),
						   mb_col_blue(mb->bg_col),
						   mb->trans);
	 } else {
	   mb_pixbuf_img_fill(mb->pb, img_dest,
			      mb_col_red(mb->bg_col),  
			      mb_col_green(mb->bg_col),
			      mb_col_blue(mb->bg_col),
			      0);
	 }
     }

   
   if (menu->too_big)
      start_item = menu->too_big_start_item;

   for(item = start_item; item != NULL; item = item->next_item)
   {
      if (menu->too_big && ( (item->y+item->h)
			    >= (menu->height - SCROLL_BUTT_H)) )
      {
	 /* leave space for scroll down button */
	 break;
      }

     if (item->type == MBMENU_ITEM_SEPERATOR)
       {
	 for ( x = WPAD + mb->inner_border_width; 
	       x < menu->width - (WPAD + mb->inner_border_width);
	       x++)
	   {
	     mb_pixbuf_img_plot_pixel(mb->pb, img_dest, 
				     x, item->y+(item->h/2),
				     mb_col_red(mb->bd_col),  
				     mb_col_green(mb->bd_col),
				     mb_col_blue(mb->bd_col));
	   }
	 continue;
       }

     if (mb->icon_dimention)
       {
	 if (item->img != NULL)
	   {
	     mb_pixbuf_img_composite(mb->pb, img_dest, item->img,
				     WPAD + mb->inner_border_width, item->y);
	   } else {
	     if (mb->img_default_folder && item->child != NULL)
	       {
		 mb_pixbuf_img_composite(mb->pb, img_dest, 
					 mb->img_default_folder,
					 WPAD + mb->inner_border_width, 
					 item->y);
	       }
	     else if ( mb->img_default_app && item->child == NULL )
	       {
		 mb_pixbuf_img_composite(mb->pb, img_dest, 
					 mb->img_default_app,
					 WPAD + mb->inner_border_width, 
					 item->y);
	       }
	   }
       }
     
     /* Child Arrow */
     if (item->child)
       {
	 int y;
	 for (y = item->y+(item->h/2) - 2; y < item->y+(item->h/2) + 3; y++)
	   {
	     mb_pixbuf_img_plot_pixel(mb->pb, img_dest, 
				     menu->width - 8, y,
				     mb_col_red(mb->fg_col),  
				     mb_col_green(mb->fg_col),
				     mb_col_blue(mb->fg_col));
	   }
	 for (y = item->y+(item->h/2) - 1; y < item->y+(item->h/2) + 2; y++)
	   {
	     mb_pixbuf_img_plot_pixel(mb->pb, img_dest, 
				     menu->width - 7, y,
				     mb_col_red(mb->fg_col),  
				     mb_col_green(mb->fg_col),
				     mb_col_blue(mb->fg_col));
	   }
	 mb_pixbuf_img_plot_pixel(mb->pb, img_dest, 
				 menu->width - 6, item->y+(item->h/2),
				 mb_col_red(mb->fg_col),  
				 mb_col_green(mb->fg_col),
				 mb_col_blue(mb->fg_col));
       }
   }

   /* border */

   if (mb->inner_border_width)
     {
       unsigned char r,g,b;

       r = mb_col_red(mb->bd_col);
       g = mb_col_green(mb->bd_col);
       b = mb_col_blue(mb->bd_col);
       
       for(y = 0; y < mb->inner_border_width; y++) 
	 for(x = 0; x < menu->width; x++)
	   mb_pixbuf_img_plot_pixel(mb->pb, img_dest, x, y, r, g, b);
       
       for(x = 1; x <= mb->inner_border_width; x++)
	 for(y = 0; y < menu->height; y++) 
	   mb_pixbuf_img_plot_pixel(mb->pb, img_dest, menu->width-x, y, r, g, b);
       for(y = 1; y <= mb->inner_border_width; y++) 
	 for(x = 0; x < menu->width; x++)
	   mb_pixbuf_img_plot_pixel(mb->pb, img_dest, x, menu->height-y, r, g, b);
       for(x = 0; x < mb->inner_border_width; x++)
	 for(y = 0; y < menu->height; y++) 
	   mb_pixbuf_img_plot_pixel(mb->pb, img_dest, x, y, r, g, b);
     }
   
   mb_pixbuf_img_render_to_drawable(mb->pb, img_dest, 
				    mb_drawable_pixmap(menu->backing), 0, 0);

   /* Text */

   for(item = start_item; item != NULL; item = item->next_item)
     {
      if (menu->too_big && ( (item->y+item->h)
			    >= (menu->height - SCROLL_BUTT_H)) )
      {
	 /* leave space for scroll down button */
	 break;
      }

      if (item->type == MBMENU_ITEM_SEPERATOR)
	continue;
       
       tmp_title = item->title;
       
       sx = WPAD + mb->inner_border_width + mb->icon_dimention;
       
       /*
       if (mb->icon_dimention)
       	 {
	   sx += 2 ;
	 }
       */
       sx += 2 ;

       /*
       if (item == menu->active_item)
	 mb_font_set_color (mb->font, mb->hl_col);
       else
       */
	 
       mb_font_set_color (mb->font, mb->fg_col);
       
       mb_font_render_simple (mb->font, 
			      menu->backing,
			      sx,
			      item->y,
			      menu->width,
			      (unsigned char *) tmp_title,
			      MB_ENCODING_UTF8, 0);
       
     }
   /* Scroll butts */

   if (menu->too_big)
     {
       xmenu_paint_arrow(mb, menu, 1); /* UP */
       xmenu_paint_arrow(mb, menu, 0); /* DOWN */
     }

   XSetWindowBackgroundPixmap(mb->dpy, menu->win, 
			      mb_drawable_pixmap(menu->backing));
   XClearWindow(mb->dpy, menu->win);

   mb_pixbuf_img_free(mb->pb, img_dest);

   XFlush(mb->dpy);
}

static void
remove_xmenus(MBMenu *mb,MBMenuMenu *active[])
{
   while( *active != NULL)
   {
      MENUDBG("destroying %s\n", (*active)->title); 
      xmenu_destroy(mb, *active);
      *active = (MBMenuMenu *)NULL;
      active++;
   }

   XSync(mb->dpy, False);  /* should help with compositer ? */
}


