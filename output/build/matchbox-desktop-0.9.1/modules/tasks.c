#include "mbdesktop_module.h"

static int trapped_error_code = 0;
static int (*old_error_handler) (Display *d, XErrorEvent *e);


static MBDesktopItem *ThisFolder = NULL;

#ifdef USE_PNG
#define NOAPP_FN    "mbnoapp.png"
#else
#define NOAPP_FN    "mbnoapp.xpm"
#endif

#define WIN_CACHE_SIZE 256  	/* 256 apps *should* be enough */

static Atom  atom_client_list;
static Atom atom_net_name;
static Atom atom_net_win_type;
static Atom atom_net_wm_icon; 
static Atom atom_net_win_type_normal; 
static Atom atom_utf8_str;    

static void
tasks_activate_cb (void *data1, void *data2);

static void
tasks_open_cb (void *data1, void *data2);


static int
error_handler(Display     *display,
	      XErrorEvent *error)
{
   trapped_error_code = error->error_code;
   return 0;
}

static void
trap_errors(void)
{
   trapped_error_code = 0;
   old_error_handler = XSetErrorHandler(error_handler);
}

static int
untrap_errors(void)
{
   XSetErrorHandler(old_error_handler);
   return trapped_error_code;
}


static void *
get_win_prop_data (MBDesktop *mb, 
                   Window     win, 
                   Atom       prop, 
                   Atom       type, 
                   int       *items)
{
	Atom type_ret;
	int format_ret;
	unsigned long items_ret;
	unsigned long after_ret;
	unsigned char *prop_data;

	prop_data = 0;

	XGetWindowProperty (mbdesktop_xdisplay(mb), win, prop, 0, 0x7fffffff, False,
			    type, &type_ret, &format_ret, &items_ret,
			    &after_ret, &prop_data);
	if (items)
		*items = items_ret;

	return prop_data;
}


int
tasks_init (MBDesktop *mb, MBDesktopFolderModule *folder_module, char *arg_str)
{
  MBDesktopItem *tasks_folder;

  tasks_folder = mbdesktop_module_folder_create ( mb, 
						  "Active Tasks", 
						  "mbfolder.png" );
  
  mbdesktop_item_set_activate_callback (mb, tasks_folder, tasks_open_cb); 

  mbdesktop_items_append_to_top_level (mb, tasks_folder);

  atom_net_name    
    = XInternAtom(mbdesktop_xdisplay(mb), "_NET_WM_NAME",    False);
  atom_net_win_type 
    = XInternAtom(mbdesktop_xdisplay(mb), "_NET_WM_WINDOW_TYPE", False);
  atom_net_wm_icon 
    = XInternAtom(mbdesktop_xdisplay(mb), "_NET_WM_ICON", False);
  atom_net_win_type_normal 
    = XInternAtom(mbdesktop_xdisplay(mb), "_NET_WM_WINDOW_TYPE_NORMAL", False);
  atom_utf8_str    
    = XInternAtom(mbdesktop_xdisplay(mb), "UTF8_STRING",    False);
  atom_client_list 
    = XInternAtom(mbdesktop_xdisplay(mb), "_NET_CLIENT_LIST",False);

  ThisFolder = tasks_folder;

  return 1;
}


static void
tasks_populate (MBDesktop *mb, MBDesktopItem *item_folder)
{
  MBDesktopItem *item = NULL, *item_new; 

  MBPixbufImage *img = NULL;
  Window *wins, win_trans_for;
  XWMHints *wmhints;
  XWindowAttributes winattr; 
  int i, num;
  char *icon_name = NULL;

  Atom *atom_tmp;
  
  int *wm_icon_data;

  if (mbdesktop_item_folder_has_contents(mb, item_folder))
    mbdesktop_item_folder_contents_free(mb, item_folder); 

  wins = get_win_prop_data (mb, mb->root, atom_client_list, XA_WINDOW, &num);

  item = item_folder->item_child;

  if (!wins) return;

  for (i = 0; i < num; i++)
    {
      unsigned char *win_name = NULL;
      img = NULL;
      
      trap_errors();
      
      if (!XGetWindowAttributes(mbdesktop_xdisplay(mb), wins[i], &winattr))
	continue;
      
      if (winattr.map_state != IsViewable || winattr.override_redirect)
	{
	  continue;
	}


      atom_tmp = get_win_prop_data (mb, wins[i], atom_net_win_type, 
				    XA_ATOM, NULL);

      if (untrap_errors())
	continue;

      if (atom_tmp && atom_tmp[0] != atom_net_win_type_normal)
	{
	  continue;
	}

      if (atom_tmp) XFree(atom_tmp);
      
      XGetTransientForHint(mbdesktop_xdisplay(mb), wins[i], &win_trans_for);
      
      if ( win_trans_for && (win_trans_for != wins[i]))
	{
	  continue;
	}
      
      if ((win_name = get_win_prop_data(mb, wins[i], 
					atom_net_name, atom_utf8_str, 
					NULL)) == NULL)
	{
	  XFetchName(mbdesktop_xdisplay(mb), wins[i], (char **)&win_name); 
	  if (!win_name) win_name = (unsigned char *)strdup("<unnamed>");
	}
      
      wmhints = XGetWMHints(mbdesktop_xdisplay(mb), wins[i]);
      
      if ((wm_icon_data = get_win_prop_data(mb, wins[i], 
					    atom_net_wm_icon, XA_CARDINAL, 
					    NULL)) != NULL)
	{
	  unsigned char *p;
	  int i;
	  MBPixbufImage *tmp_img;

	  img = mb_pixbuf_img_new_from_int_data(mb->pixbuf,
						wm_icon_data+2,
						wm_icon_data[0], 
						wm_icon_data[1]);

	  if (img)
	    {
	      if (wm_icon_data[0] != 32 || wm_icon_data[1] != 32) 
		{
		  tmp_img = mb_pixbuf_img_scale(mb->pixbuf, img, 32, 32); 
		  mb_pixbuf_img_free(mb->pixbuf, img);
		  img = tmp_img;
		}
	    }
	  
	  XFree(wm_icon_data);
	}
      else if (wmhints && wmhints->flags & IconPixmapHint
	       && wmhints->flags & IconMaskHint)
	{
	  Window duh;
	  int x,y,w,h,b,d;
	  MBPixbufImage *tmp_img;
	  
	  if (XGetGeometry(mbdesktop_xdisplay(mb), wmhints->icon_pixmap, &duh, 
			   &x, &y, &w, &h, &b, &d))
	    {
	      img = mb_pixbuf_img_new_from_drawable(mb->pixbuf, 
						    (Drawable)wmhints->icon_pixmap, 
						    (Drawable)wmhints->icon_mask,
						    0, 0, w, h);
	      
	      if ( (img) && (w != 32 || h != 32))
		{
		  tmp_img = mb_pixbuf_img_scale(mb->pixbuf, img, 32, 32); 
		  
		  mb_pixbuf_img_free(mb->pixbuf, img);
		  img = tmp_img;
		}
	    }
	  
	} 

      
      item_new = mbdesktop_item_new_with_params( mb,
						 win_name,
						 NULL,
						 (void *)wins[i],
						 ITEM_TYPE_MODULE_ITEM
						 );


      if (img != NULL && !untrap_errors())
	mbdesktop_item_set_icon_data(mb, item_new, img);

      mbdesktop_item_set_activate_callback (mb, item_new, tasks_activate_cb); 
	
      mbdesktop_items_append_to_folder (mb, item_folder, item_new);

      if (img) mb_pixbuf_img_free(mb->pixbuf, img);

      if (win_name) XFree(win_name);
      
      if (icon_name) free(icon_name);

   }

 XFree(wins);

}

static void
tasks_open_cb (void *data1, void *data2)
{
  MBDesktop *mb = (MBDesktop *)data1; 
  MBDesktopItem *item_folder = (MBDesktopItem *)data2; 

  tasks_populate (mb, item_folder);

  mbdesktop_item_folder_activate_cb(data1, data2);
}


void
tasks_xevent (MBDesktop             *mb, 
	      MBDesktopFolderModule *folder_module,
	      XEvent                *xevent)
{
  if (mbdesktop_item_folder_get_open(mb) == ThisFolder
      && xevent->xproperty.atom == atom_client_list)
    {
      tasks_populate (mb, ThisFolder);

      /* XXX This probably gets called a little to much */
      // tasks_open_cb((void *)mb, (void *)ThisFolder);
      mbdesktop_view_paint(mb, False);
    }

}

static void
tasks_activate_cb (void *data1, void *data2)
{
  XEvent ev;
  MBDesktop *mb = (MBDesktop *)data1; 
  MBDesktopItem *item = (MBDesktopItem *)data2; 
  Atom atom_net_active 
    = XInternAtom(mbdesktop_xdisplay(mb), "_NET_ACTIVE_WINDOW", False);

  memset(&ev, 0, sizeof ev);
  ev.xclient.type = ClientMessage;
  ev.xclient.window = (Window)item->data;
  ev.xclient.message_type = atom_net_active;
  ev.xclient.format = 32;

  ev.xclient.data.l[0] = 0; 

  XSendEvent(mbdesktop_xdisplay(mb), mb->root, False, 
	     SubstructureRedirectMask, &ev);
}


#define MODULE_NAME         "Active Tasks"
#define MODULE_DESC         "Simple task switcher for desktop"
#define MODULE_AUTHOR       "Matthew Allum"
#define MODULE_MAJOR_VER    0
#define MODULE_MINOR_VER    0
#define MODULE_MICRO_VER    1
#define MODULE_API_VERSION  0
 
MBDesktopModuleInfo tasks_info = 
  {
    MODULE_NAME         ,
    MODULE_DESC         ,
    MODULE_AUTHOR       ,
    MODULE_MAJOR_VER    ,
    MODULE_MINOR_VER    ,
    MODULE_MICRO_VER    ,
    MODULE_API_VERSION
  };

MBDesktopFolderModule folder_module =
  {
    &tasks_info,
    tasks_init,
    tasks_xevent,
    NULL
  };


