#ifndef _MBMENU_H_
#define _MBMENU_H_

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

#include "libmb/mbconfig.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <X11/Xresource.h>
#include <X11/extensions/shape.h>
#include <X11/Xatom.h>
#include <X11/keysymdef.h>
#include <X11/keysym.h>
#include <X11/Xmd.h>

#ifdef USE_XSETTINGS
#include <xsettings-client.h>
#endif 

#include "mbpixbuf.h"
#include "mbexp.h"

/**
 * @defgroup MBMenu MBMenu - A simple popup menu widget
 * @brief a simple independent popup menu widget, used mainly for application
 *  launchers etc.
 *
 * NOTE: Its planned that one day in the future this will be superceded by matchbox-tk. 
 * @{
 */

/* 
   TODO?

   - mb_menu menu calls to mb_menu_menu ?
*/

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @enum MBMenuColorElement
 *
 * Used to set various menu elements colours
 */ 
typedef enum {
  MBMENU_SET_BG_COL,
  MBMENU_SET_FG_COL,
  MBMENU_SET_HL_COL,
  MBMENU_SET_BD_COL
} MBMenuColorElement;

/**
 * @enum MBMenuItemAddFlags
 *
 * Used to specify how an item is added to a menu
 */ 
typedef enum {
  MBMENU_NO_SORT    =     (1<<1),
  MBMENU_PREPEND    =     (1<<2)
} MBMenuItemAddFlags;

enum {
  MBMENU_ITEM_APP,
  MBMENU_ITEM_FOLDER,
  MBMENU_ITEM_SEPERATOR
};

/* for mb_menu_new */
#define MBMENU_FG_COL         (1<<1)
#define MBMENU_BG_COL         (1<<2)
#define MBMENU_HL_COL         (1<<3)
#define MBMENU_BD_COL         (1<<4)
#define MBMENU_FONT           (1<<5)
#define MBMENU_BD_SZ          (1<<6)
#define MBMENU_ICON_SZ        (1<<7)
#define MBMENU_ICON_FN        (1<<8)
#define MBMENU_ICON_FOLDER_FN (1<<9)
#define MBMENU_TRANS          (1<<10)
#define MBMENU_BG_FN          (1<<11)
#define MBMENU_BEVEL          (1<<12)

typedef struct _menu_options
{
  char *fontname;
  char *foreground_col_spec;
  char *background_col_spec;
  char *highlight_col_spec;
  char *border_col_spec;
  int   border_size;
  int   icon_dimention;
  char *default_icon_filename;
  char *default_folder_icon_filename;
  char *bg_img_filename;
  int  transparency_level;
  int  bevel_size;

} MBMenuOptions;

/**
 * @typedef MBMenuMenu
 *
 * Opaque type for a  menu
 */ 
typedef struct _menu
{

  /*
#ifdef USE_XFT
  XftDraw *xftdraw;
  XftDraw *shadow_xftdraw;
  XftDraw *active_xftdraw;
  int expose_cnt;
#endif
  */

  char *title;
  struct _menuitem *items;
  struct _menuitem *active_item;
  struct _menuitem *too_big_start_item;
  struct _menuitem *too_big_end_item;
  struct _menuitem *parent_item;
  
  int x;
  int y;
  int width;
  int height;
  int depth;
  
  Window      win;

  GC mask_gc;
  Bool too_big;

  MBDrawable *active_item_drw;
  MBDrawable *backing; 


} MBMenuMenu;


/**
 * @typedef MBMenuItem
 *
 * Opaque type for a menu item
 */ 
typedef struct _menuitem
{
  int type;

  char *title;
  void (* cb)( struct _menuitem *item );
  void *cb_data;
  char *info;
  char *icon_fn;
  
  MBPixbufImage *img;
  
  MBMenuMenu *child;
  struct _menuitem *next_item;
  
  int y;
  int h;

   
} MBMenuItem; 			/* XXX MBMenuItem */

/**
 * @typedef MBMenu
 *
 * Opaque type for a 'top level' menu
 */ 
typedef struct _mbmemu
{
  Display *dpy;
  Window   root;
  int      screen;
  MBFont  *font;

  /*
#ifdef USE_XFT
   XftFont *xftfont;
   XftColor fg_xftcol;
   XftColor bg_xftcol;
   XftColor hl_xftcol;
   XftColor bd_xftcol;
   XftColor shadow_xftcol;
#else
   XFontStruct* font;
#endif
   XColor   fg_xcol;
   XColor   bg_xcol;
   XColor   hl_xcol;
   XColor   bd_xcol;
  */

  MBColor   *fg_col;
  MBColor   *bg_col;
  MBColor   *hl_col;
  MBColor   *bd_col;


  GC gc;

  Bool have_highlight_col;
  
  int options;

  int border_width;		/* X window border */
  int inner_border_width; 	/* Non X border */
  XColor border_cols[3];
  int trans;

  int icon_dimention;  /* 0 - no icons, else icon size after scale */
  
  MBPixbuf    *pb;
  MBPixbufImage *img_default_folder;
  MBPixbufImage *img_default_app;
  MBPixbufImage *img_bg;

  Pixmap arrow_icon, arrow_mask; /* XXX Togo */
  Pixmap bg_pixmap, bg_pixmap_mask;

  struct _menu *rootmenu;
  Bool xmenu_is_active;
  struct _menu *active[10];
  int active_depth;

  Atom atom_mbtheme;

  struct _menu *keyboard_focus_menu;

#ifdef USE_XSETTINGS
  XSettingsClient *xsettings_client;
#endif 

} MBMenu;

/**
 * @typedef MBMenuActivateCB
 *
 * Callback for an activated menu item.
 */ 
typedef void (*MBMenuActivateCB)( MBMenuItem *item ) ;

/**
 * Creates a new toplevel mbmenu instance
 *
 * @param dpy X Display
 * @param screen X Screen
 * @returns an mbmenu instance, NULL on failure. 
 */
MBMenu *
mb_menu_new(Display *dpy, int screen);

/**
 * Sets the font used by the referenced menu
 *
 * @param mbmenu mb menu instance
 * @param font_desc font to load
 * @returns True if font loaded, False on failiure
 */
Bool
mb_menu_set_font (MBMenu *mbmenu, 
		  char   *font_desc);

/**
 * Sets the default icons to be uses when not supplied by an individual item
 *
 * @param mbmenu mb menu instance
 * @param folder icon filename for folders
 * @param app    icon filename for items
 * @returns True if icons loaded ok, False on failiure
 */
Bool
mb_menu_set_default_icons(MBMenu *mbmenu, char *folder, char *app);


/**
 * Sets the icon dimention in pixels used by the referenced menu instance.
 * If set to zero, icons will not be used by the menu. 
 *
 * @param mbmenu mb menu instance
 * @param size Icon dimention in pixels
 */
void
mb_menu_set_icon_size(MBMenu *mbmenu, int size);

/**
 * Sets the font used by the referenced menu instance
 *
 * @param mbmenu mb menu instance
 * @param element Which part of the menu to set color
 * @param col_spec Color specification in the form #RRGGBB
 */
void
mb_menu_set_col(MBMenu             *mbmenu, 
		MBMenuColorElement  element, 
		char               *col_spec);

/**
 * Sets the menu's transparency level. The Transparency is a HACK!, use at 
 * your own risk, its unsupported. 
 *
 * @param mbmenu mb menu instance
 * @param trans Transparency level
 */
void
mb_menu_set_trans(MBMenu *mbmenu, int trans);

/**
 * Gets the top level MBMenu menu.
 * This menu is automatically created on initialisation
 *
 * @param mbmenu mb menu instance
 * @returns root menu instance. 
 */
MBMenuMenu* mb_menu_get_root_menu(MBMenu *mbmenu);


/**
 * Gets the top level MBMenu menu size.
 *
 * @param mbmenu mb menu instance
 * @param width pointer to populate width int
 * @param height pointer to populate height int
 * @returns True on success, False on fail ( 0x0 menu ). 
 */
Bool
mb_menu_get_root_menu_size(MBMenu *mbmenu, int *width, int *height);


/**
 * Adds a seperator to a menu.
 *
 * @param mbmenu MBMenu instance
 * @param menu The menu to add the seperator too. 
 * @param flags can be 0 or MBMENU_PREPEND to prepend the seperator rather
 *        than append the seperator to the menu. 
 */
void 
mb_menu_add_seperator_to_menu(MBMenu     *mbmenu, 
			      MBMenuMenu *menu, 
			      int         flags);

/**
 * Adds single or multiple new menus to an mbmenu instance. 
 *
 * @param mbmenu MBMenu instance
 * @param path 
 * @param icon_path 
 * @param flags can be 0 or MBMENU_PREPEND to prepend the seperator rather
 *        than append the seperator to the menu. 
 * @returns new created menu 
 */
MBMenuMenu 
*mb_menu_add_path(MBMenu *mbmenu, 
		  char   *path, 
		  char   *icon_path, 
		  int     flags);

/**
 * Removes a menu and all of its sub menus
 *
 * @param mbmenu mb menu instance
 * @param menu menu to remove
 */
void mb_menu_remove_menu(MBMenu     *mbmenu, 
			 MBMenuMenu *menu);


/**
 * Free's a mbmenu toplevel instance
 *
 * @param mbmenu mbmenu instance
 */
void mb_menu_free(MBMenu *mbmenu);

/**
 * Checks to see if specified menu intance is active ( ie popped up )
 *
 * @param mbmenu mb menu instance
 * @returns True if menu is active, False otherwise
 */
Bool mb_menu_is_active(MBMenu *mbmenu);

/**
 * Checks to see if specified menu intance is active ( ie popped up )
 *
 * @param mbmenu mb menu instance
 * @param x x co-ord ( relative to root window origin ) to activate menu
 * @param y y co-ord ( relative to root window origin ) to activate menu
 *
 */
void mb_menu_activate(MBMenu *mbmenu, 
		      int     x, 
		      int     y);

/**
 * Deactivates ( hides ) a mbmenu instance.
 *
 * @param mbmenu mb menu instance
 */
void mb_menu_deactivate(MBMenu *mbmenu);


/**
 * Processes an X Event.
 *
 * @param mbmenu mb menu instance
 * @param xevent Xevent to process
 *
 */
void mb_menu_handle_xevent(MBMenu *mbmenu, XEvent *xevent);

/**
 * Adds a new menu item to a menu. 
 *
 * @param mbmenu mb menu instance
 * @param menu menu to add the item too
 * @param title menu item title
 * @param activate_callback function to call when menu is clicked.
 * @param user_data  user data to attach to item.
 * @param flags specify how the item is added to the menu
 * @returns the created menu item or NULL on failiure. 
 *
 */
MBMenuItem *
mb_menu_new_item (MBMenu *mbmenu, 
		  MBMenuMenu   *menu, 
		  char   *title,
		  MBMenuActivateCB activate_callback ,
		  void   *user_data,
		  MBMenuItemAddFlags flags
		  );


MBMenuItem * 			/* XXX TOGO */
mb_menu_add_item_to_menu(MBMenu *mbmenu, 
			 MBMenuMenu *menu, 
			 char *title, 
			 char *icon, 
			 char *info,
			 void (* cmd)( MBMenuItem *item ),
			 void *cb_data,
			 int flags);

/**
 * Adds a new menu item to a menu. 
 *
 * @param mbmenu mb menu instance
 * @param item menu item 
 * @param img mbpixbuf image to set use as item image
 */
void
mb_menu_item_icon_set(MBMenu *mbmenu, MBMenuItem *item, MBPixbufImage *img);

/**
 * Gets any user data attatched to a menu item
 *
 * @param item menu item 
 * @returns void pointer to set user data.
 */
void*
mb_menu_item_get_user_data(MBMenuItem *item);

/**
 * Removes a menu item
 *
 * @param mbmenu mbmenu instance
 * @param menu the menu to remove from
 * @param item the menu item
 */
void mb_menu_item_remove(MBMenu *mbmenu, MBMenuMenu *menu, MBMenuItem *item);

/**
 * Dumps an mbmenu menu structure to stdout
 *
 * @param mbmenu mb menu instance
 * @param menu menu to dump 
 * 
 */
void mb_menu_dump(MBMenu *mbmenu, MBMenuMenu *menu);

#ifdef __cplusplus
}
#endif



/** @} */


#endif
