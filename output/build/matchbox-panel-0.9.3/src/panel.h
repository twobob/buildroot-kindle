#ifndef _DOCK_H_
#define _DOCK_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include <X11/Xmd.h>
#include <X11/extensions/shape.h>
#include <X11/cursorfont.h>

#include <libmb/mb.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_XSETTINGS
#include <xsettings-client.h>
#endif 

#define NEW(OBJ) ((OBJ *)(malloc(sizeof(OBJ))))

#ifdef DEBUG
#define DBG(txt, args... ) fprintf(stderr, "MB-PANEL-DEBUG: " txt , ##args )
#else
#define DBG(txt, args... ) /* nothing */
#endif

#define SWAP(a,b) { \
        (a)^=(b);   \
        (b)^=(a);   \
        (a)^=(b);   \
        }

#define PANEL_IS_VERTICAL(p) ((p)->orientation == East || (p)->orientation == West) 

#define SYSTEM_TRAY_REQUEST_DOCK    0
#define SYSTEM_TRAY_BEGIN_MESSAGE   1
#define SYSTEM_TRAY_CANCEL_MESSAGE  2

#define ATOM_WM_WINDOW_TYPE      0
#define ATOM_WM_WINDOW_TYPE_DOCK 1
#define ATOM_WM_WINDOW_TYPE_SPLASH 10
#define ATOM_SYSTEM_TRAY         2
#define ATOM_SYSTEM_TRAY_OPCODE  3
#define ATOM_XEMBED_INFO         4
#define ATOM_XEMBED_MESSAGE      5
#define ATOM_MANAGER             6
#define ATOM_MB_DOCK_ALIGN       7
#define ATOM_MB_DOCK_ALIGN_EAST  8

#define ATOM_NET_SYSTEM_TRAY_MESSAGE_DATA 9

#define ATOM_WM_PROTOCOLS 11
#define ATOM_WM_DELETE_WINDOW  12

#define ATOM_MB_THEME  13
#define ATOM_MB_PANEL_BG  16
#define ATOM_MB_DOCK_TIMESTAMP  14
#define ATOM_NET_WM_STRUT       15
#define ATOM_WM_CLIENT_LEADER   17
#define ATOM_NET_WM_ICON        18
#define ATOM_NET_WM_PID         19
#define ATOM_XROOTPMAP_ID       20
#define ATOM_NET_SYSTEM_TRAY_ORIENTATION       21
#define ATOM_MB_THEME_NAME      22
#define ATOM_MB_COMMAND      23
#define ATOM_NET_WM_NAME     24
#define ATOM_UTF8_STRING     25
#define ATOM_NET_CLIENT_LIST 26

#define ATOM_NET_WM_STATE          27
#define ATOM_NET_WM_STATE_TITLEBAR 28

#define ATOM_MB_SYSTEM_TRAY_CONTEXT 29
#define ATOM_MB_REQ_CLIENT_ORDER 30

#define ATOM_MB_DOCK_TITLEBAR_SHOW_ON_DESKTOP 31

#define XEMBED_EMBEDDED_NOTIFY  0
#define XEMBED_WINDOW_ACTIVATE  1

/* ID's for various MB COMMAND X Messages */

#define MB_CMD_PANEL_TOGGLE_VISIBILITY 1
#define MB_CMD_PANEL_SIZE              2
#define MB_CMD_PANEL_ORIENTATION       3

#define MB_PANEL_ORIENTATION_NORTH     1
#define MB_PANEL_ORIENTATION_EAST      2
#define MB_PANEL_ORIENTATION_SOUTH     3
#define MB_PANEL_ORIENTATION_WEST      4


#define SESSION_TIMEOUT  10    /* 5 second session timeout  */

#define DBL_CLICK_TIME 200


#define DEFAULT_COLOR_SPEC "#e2e2de" /* Same as gnome ? */
#define MB_MSG_FONT        "Sans 14px"   

#define MAX_DEFERED_WINS 32 	/* *very* Unlikely as much as 32 */

enum {
  BG_SOLID_COLOR,
  BG_PIXMAP,
  BG_TRANS
};

typedef enum {
  PAPP_GRAVITY_START,
  PAPP_GRAVITY_END,
} PanelAppGravity; 
 
typedef enum  { 
  North = 1, 
  East, 
  South, 
  West 
} MBPanelOrientation;


typedef struct _panel_app {
   
  Window             win;
  unsigned char     *name;
  int                x;
  int                y;
  int                w;
  int                h;

  int                offset;
  
  Bool               mapped;
  
  struct _panel_app *next;
  
  char              *cmd_str;

  struct _panel     *panel;

  Bool               ignore; 	/* set so client cant be removed */
  int                ignore_unmap;

  PanelAppGravity    gravity;

  MBPixbufImage     *icon;

} MBPanelApp;

typedef struct _message_queue {

  MBPanelApp             *sender;
  unsigned long           starttime;
  int                     timeout; 
  int                     total_msg_length;
  int                     current_msg_length;
  int                     id;
  unsigned char          *data;

  Bool                    has_extra_context;
  unsigned char          *extra_context_data;

  Bool                    pending;  /* Set to true if all data */ 
   
  struct _message_queue  *next;
   
} MBPanelMessageQueue;


typedef struct _panel {

  /* General */

  Display                *dpy;
  int                     screen;
  MBPixbuf               *pb;
  Window                  win, win_root;
  GC                      gc, band_gc;
  XColor                  xcol;
  
  int                     x,y,w,h;

  MBPanelApp             *apps_start_head;
  MBPanelApp             *apps_end_head;

  Atom                    atoms[32];
  int                     padding;
  int                     margin_topbottom;
  int                     margin_sides;  

  /* Message windows */

  struct _message_queue  *msg_queue_start;
  struct _message_queue  *msg_queue_end;
  
  Window                  msg_win;
  unsigned long           msg_starttime;
  int                     msg_timeout; 
  MBPanelApp             *msg_win_sender;
  Bool                    msg_has_context;
  unsigned long		  msg_sender_id;
  int                     msg_context_y1, msg_context_y2;

  GC                      msg_gc;

  MBColor                *msg_col;
  MBColor                *msg_urgent_col;
  MBColor                *msg_fg_col;
  MBColor                *msg_link_col;

  MBFont                 *msg_font;
  

  /* Various state bits */

  Bool                    use_flip;
  Bool                    use_session;
  Bool                    use_alt_session_defaults;

  Bool                    use_overide_wins;
  Bool                    reload_pending;

  MBPanelOrientation      orientation;

  int                     system_tray_id;

  Bool                    use_themes; 
  char                   *theme_name;
  char                   *theme_path;

  Bool                    want_titlebar_dest;

  Bool                    ignore_next_config;

  int                     default_panel_size;

  /* Session */

  Bool                    session_preexisting_lock;
  int                     session_init_offset;
  char                    session_entry_cur[512];
  pid_t                   session_needed_pid;
  time_t                  session_start_time;
  FILE                   *session_fp;
  PanelAppGravity         session_cur_gravity; 
  Bool                    session_run_first_time;
  char                   *session_defaults_cur_pos;
  
  Window                  last_click_window;
  Time                    last_click_time;
  Bool                    next_click_is_not_double;
  Bool                    is_hidden;

  /* Background */
  
  XColor                  bg_col;
  Pixmap                  bg_tile;
  Pixmap                  bg_pxm;
  char                   *bg_spec;
  int                     bg_type;

  /* Popup menu */

  MBMenu*                 mbmenu;
  Bool                    use_menu;
  MBMenuMenu             *remove_menu;

  /* co-ords of where mouse click happened for menu  */
  int                     click_x; 
  int                     click_y;
  Time                    click_time;

  char                   *bg_trans;
  long                    root_pixmap_id;

  /* Defered apps list - win ids that start while session is
   * starting but *arnt* in the session, so they are docked
   * after starting. 
   *
   * XXX: should really use a list here.
  */
  
  Window                  session_defered_wins[MAX_DEFERED_WINS];
  int                     n_session_defered_wins;

#ifdef USE_XSETTINGS
  XSettingsClient        *xsettings_client;
#endif 

} MBPanel;

void
panel_handle_full_panel (MBPanel *panel, MBPanelApp *bad_papp);

void 
panel_toggle_visibilty(MBPanel *panel);

void 
panel_change_orientation (MBPanel            *panel, 
			  MBPanelOrientation  new_orientation,
			  int                 dpy_w, 
			  int                 dpy_h);

void
panel_update_client_list_prop (MBPanel *panel);

void
panel_reorder_apps(MBPanel *panel);

void
panel_handle_dock_request(MBPanel *panel, Window win);


#include "panel_menu.h"
#include "panel_util.h"  
#include "panel_app.h"  
#include "session.h"

#endif
