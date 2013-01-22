/*
 * mb-applet-launcher - A docked applicatication launcher
 *
 * Originally based on xsingleinstance by Merle F. McClelland for CompanionLink
 *
 *
 *  Copyright 2002 Matthew Allum
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 **/

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>

#ifdef USE_LIBSN
#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#endif 

#include <libmb/mb.h>

#define BUTTON_UP   0
#define BUTTON_DOWN 1

#define LAUNCH_TIMEOUT 15 /* 15 seconds to start */

enum {
  ACTION_KILL,
  ACTION_TOGGLE_WIN_STATE,
  ACTION_NONE,
  ACTION_MESSAGE_DOCK,
#ifdef USE_LIBSN
  ACTION_SI, 		/* Single instance */
  ACTION_SN,		/* Startup notification */
#endif
};

static Display *dpy;
static int screen;
static MBPixbuf *pb;
static MBPixbufImage *img_icon = NULL, *img_icon_active = NULL; 
static Atom atom_wm_state, atom_wm_delete, atom_wm_protos;

static MBTrayApp   *TrayApp; 
static Bool ButtonIsDown;
static Bool DoAnimation = True;

#ifdef USE_LIBSN

SnDisplay *sn_dpy;

static void sn_activate(char *name, char *exec_str);
static void si_activate(char *name, char *exec_str);

static void 
si_activate(char *name, char *exec_str)
{
  Window win_found;

  if (mb_single_instance_is_starting(dpy, exec_str))
    return;

  win_found = mb_single_instance_get_window(dpy, exec_str);

  if (win_found != None)
    {
      mb_util_window_activate(dpy, win_found);
    }
  else sn_activate(name, exec_str);

}


static void 
sn_activate(char *name, char *exec_str)
{

  SnLauncherContext *context;
  pid_t child_pid = 0;

  context = sn_launcher_context_new (sn_dpy, screen);
  
  sn_launcher_context_set_name (context, name);
  sn_launcher_context_set_binary_name (context, exec_str);
  
  sn_launcher_context_initiate (context, "monoluanch launch", exec_str,
				CurrentTime);

  switch ((child_pid = fork ()))
    {
    case -1:
      fprintf (stderr, "Fork failed\n" );
      break;
    case 0:
      sn_launcher_context_setup_child_process (context);
      mb_exec(exec_str);
      fprintf (stderr, "Failed to exec %s \n", exec_str);
      _exit (1);
      break;
    }
}
#endif

static char*
arr_to_str(char **args, int argc)
{
  int i = 0, n_bytes = 0;
  char *cmd = NULL;
  
  while(i < argc)
    {
      n_bytes += (strlen(args[i]) + 2);
      i++;
    }

  if (n_bytes == 0) return NULL;

  cmd = malloc(sizeof(char)*n_bytes);

  i = 0; *cmd = '\0';

  while(i < argc)
    {
      strcat(cmd, args[i]);
      strcat(cmd, " ");
      i++;
    }

  return cmd;
}

static int 
win_exists(Window win, Window top)
{
  Window *children, dummy;
  unsigned int nchildren;
  int i, w = 0;
  
  if (top == win) return 1;
  
  if (!XQueryTree(dpy, top, &dummy, &dummy, &children, &nchildren))
    return 0;
  
  for (i=0; i<nchildren; i++) 
    {
      w = win_exists(win, children[i]);
      if (w) break;
    }

  if (children) XFree ((char *)children);
  return(w);

}

static void
send_panel_message(char *args)
{
#define MAX_MESSAGE_BYTES 1024

  char buf[256];
  char msg[MAX_MESSAGE_BYTES] = "";
  FILE *ptr = NULL;
  int bytes = 0;

  if ((ptr = popen(args, "r")) != NULL)
    {
      while (fgets(buf, 256, ptr) != NULL)
	{
	  bytes += strlen(buf);
	  if (bytes > MAX_MESSAGE_BYTES) break;
	  strcat(msg, buf);
	}

      mb_tray_app_tray_send_message(TrayApp, msg, 5000);
      pclose(ptr);
    }
  else
    {
      fprintf(stderr, "failed to popen\n");
    }
}

int
get_win_state(Window win)
{
    Atom real_type; int real_format;
    unsigned long items_read, items_left;
    long *data, state = WithdrawnState;

    if (XGetWindowProperty(dpy, win,
			   atom_wm_state, 0L, 2L, False,
			   atom_wm_state, &real_type, &real_format,
			   &items_read, &items_left,
			   (unsigned char **) &data) 
	== Success && items_read) 
      {
	state = *data;
	XFree(data);
      }
    return state;
}

void
kill_launched_win(Window win)
{
  int i, n, found = 0;
  Atom *protocols;
  XEvent e;
			  
  if (XGetWMProtocols(dpy, win,
		      &protocols, &n))
    {
      for (i=0; i<n; i++)
	if (protocols[i] == atom_wm_delete) found++;
      XFree(protocols);
    }
			  
  if (found)
    {
      e.type = ClientMessage;
      e.xclient.window = win;
      e.xclient.message_type = atom_wm_protos;
      e.xclient.format = 32;
      e.xclient.data.l[0] = atom_wm_delete;
      e.xclient.data.l[1] = CurrentTime;
      XSendEvent(dpy, win, False, NoEventMask, &e);
    }
  else
    {
      XKillClient(dpy, win);
    }
}

void 
fork_exec(char *cmd)
{
  switch (fork())
    {
    case 0:
      setpgid(0, 0); /* Stop us killing child */
      mb_exec(cmd);
      fprintf(stderr, "monolaunch: Failed to Launch '%s'", cmd);
      exit(1);
    case -1:
      fprintf(stderr, "monolaunch: Failed to Launch '%s'", cmd);
      break;
    }
}

Window 
get_launch_window()
{
  XEvent xevent;
  time_t stime = time(NULL);

  while (1) 
    {
      XNextEvent(dpy, &xevent);
      switch (xevent.type) 
	{
	case CreateNotify:
	  if (xevent.xcreatewindow.send_event) {
	    break;
	  }
	  if (xevent.xcreatewindow.override_redirect) {
	    break;
	  }
	  XSelectInput(dpy, xevent.xcreatewindow.window, PropertyChangeMask);
	  break;
	case PropertyNotify:
	  if (xevent.xproperty.atom != atom_wm_state) break;
	  return xevent.xproperty.window;
	  break;
	}
      if ((time(NULL)-stime) > LAUNCH_TIMEOUT) return None;
    }
}


void 
usage(char *program)
{
  fprintf(stderr, 
	  "Usage %s [Options..] <%s> <app>\n"
	  "Where options are;\n"
	  "\t-display, -d <display>      X11 Display to connect to.\n"
	  "\t--title, -n <title>         Set Panels Title.\n"
	  "\t--no-animation,-na          Disable Animations, if relevant.\n"
	  "\t--start, -s                 Starts app instance immediatly\n"
	  "The defualt action is to (un)iconize selected app. Alternate actions;\n"
	  "\t--kill, -k                  Destroy app\n"
	  "\t--respawn, -l               Respawn multiple instances\n"
	  "\t--message, -m               Pipe apps stdout to the panel as a message.\n"
	  "Alternatively just a valid .desktop file can be given;\n"
	  "\t%s --desktop <.desktop file>\n\n"
	  "However with this option the -k,-l and -m will have no effect.\n"
	  "The prence of a SingleInstance=True key/pair in the .desktop file\n"
	  "will achieve similar functionality\n"
	  , program,
#ifdef MB_HAVE_PNG
	  "png|xpm",
#else
	  "xpm",
#endif
	  program
	  );

  exit(1);

}

void
paint_callback ( MBTrayApp *app, Drawable drw )
{
  MBPixbufImage *img_bg = NULL, *img_button = NULL;

  img_button = (( ButtonIsDown ) ? img_icon_active : img_icon);

  img_bg = mb_tray_app_get_background (app, pb);
  mb_pixbuf_img_copy_composite (pb, img_bg, img_button, 
				0, 0, 
				mb_pixbuf_img_get_width(img_icon), 
				mb_pixbuf_img_get_height(img_icon), 
				0, 0);

  mb_pixbuf_img_render_to_drawable (pb, img_bg, drw, 0, 0);
  mb_pixbuf_img_free( pb, img_bg );
}

void
xevent_callback (MBTrayApp *app, XEvent *ev)
{
  ;
}

void
resize_callback ( MBTrayApp *app, int w, int h )
{
  if ((w) != mb_pixbuf_img_get_width(img_icon) 
      || (h) != mb_pixbuf_img_get_width(img_icon) ) 
    {
      MBPixbufImage *tmp_img = NULL;

      tmp_img = mb_pixbuf_img_scale(pb, img_icon, w, h);
      mb_pixbuf_img_free(pb, img_icon);
      img_icon = tmp_img;

      tmp_img = mb_pixbuf_img_scale(pb, img_icon_active, w, h);
      mb_pixbuf_img_free(pb, img_icon_active);
      img_icon_active = tmp_img;
    }
}

int action            = ACTION_TOGGLE_WIN_STATE;
Window win_launched   = None;
char *cmd_str         = NULL;
char* win_panel_title = NULL;

void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  int abs_x, abs_y;
  Bool do_anim = False;

  ButtonIsDown = True;
  if (is_released)
    {
      ButtonIsDown = False;
      mb_tray_app_repaint (app);
      switch (action)
	{
	case ACTION_NONE:
	  fork_exec(cmd_str);
	  do_anim = DoAnimation;
	  break;
	case ACTION_KILL:
	  if (win_launched && win_exists(win_launched, mb_tray_app_xrootwin(app)))
	    {
	      kill_launched_win(win_launched);
	      win_launched = None;
	    } else {
	      fork_exec(cmd_str);
	      win_launched = get_launch_window();
	    }
	  break;
	case ACTION_TOGGLE_WIN_STATE:
	  if (win_launched && win_exists(win_launched, mb_tray_app_xrootwin(app)))
	    {
	      XWindowAttributes win_attrib;
	      XGetWindowAttributes(dpy, win_launched, &win_attrib);
	      
	      if (win_attrib.map_state == IsUnmapped ||
		  get_win_state(win_launched) != NormalState)
		XMapRaised(dpy, win_launched);
	      else
		XIconifyWindow(dpy, win_launched, screen);
	    }
	  else
	    {
	      fork_exec(cmd_str);
	      win_launched = get_launch_window();
	    }
	  break;
	case ACTION_MESSAGE_DOCK:
	  send_panel_message(cmd_str);
	  break;
#ifdef USE_LIBSN
	case ACTION_SN:
	  do_anim = DoAnimation;
	  sn_activate(win_panel_title, cmd_str);
	  break;
	case ACTION_SI:
	  do_anim = DoAnimation;
	  si_activate(win_panel_title, cmd_str);
	  break;
#endif
	}

      if (do_anim)
	{
	  mb_tray_app_get_absolute_coords (app, &abs_x, &abs_y); 

	  mb_util_animate_startup(mb_tray_app_xdisplay (app), 
				  abs_x,
				  abs_y,
				  mb_tray_app_width (app),
				  mb_tray_app_height (app));
	}
    } else mb_tray_app_repaint (app);


}


int 
main(int argc, char **argv)
{
  int i, x, y;

  /* Config Parameters */
  int switch_count      = 1;
  char *img_file        = NULL;
  char *dotdesktop_file = NULL;
  MBDotDesktop *dd      = NULL;
  Bool start_app        = False;
  char png_path[256]    = { 0 };

  TrayApp = mb_tray_app_new ( "mb-applet-launcher",
			      resize_callback,
			      paint_callback,
			      &argc,
			      &argv );  

  for (i = 1; i < argc; i++) {
    if (argv[i][0] == '-')
      {

	if (!strcmp ("--title", argv[i]) || !strcmp ("-n", argv[i])) {
	  if (++i>=argc) usage (argv[0]);
	  win_panel_title = argv[i];
	  switch_count += 2;
	  continue;
	}

	if (!strcmp ("--kill", argv[i]) || !strcmp ("-k", argv[i])) {
	  action = ACTION_KILL;
	  switch_count++;
	  continue;
	}

	if (!strcmp ("--start", argv[i]) || !strcmp ("-s", argv[i])) {
	  start_app = True;
	  switch_count++;
	  continue;
	}
	
	if (!strcmp ("--kill", argv[i]) || !strcmp ("-k", argv[i])) {
	  action = ACTION_KILL;
	  switch_count++;
	  continue;
	}
	
	if (!strcmp ("--relaunch", argv[i]) || !strcmp ("-l", argv[i])) {
	  action = ACTION_NONE;
	  switch_count++;
	  continue;
	}
	
	if (!strcmp ("--message", argv[i]) || !strcmp ("-m", argv[i])) {
	  action = ACTION_MESSAGE_DOCK;
	  switch_count++;
	  continue;
	}

	if (!strcmp ("--no-animation", argv[i]) || !strcmp ("-na", argv[i])) {
	  DoAnimation = False;
	  switch_count++;
	  continue;
	}

	if (!strcmp ("--desktop", argv[i])) {
	  if (++i>=argc) usage (argv[0]);
	  dotdesktop_file = argv[i];
	  switch_count += 2;
	  continue;
	}
	usage(argv[0]);
      }
    else break;
  }

  if (argc-switch_count < 2 && dotdesktop_file == NULL) usage(argv[0]);

  dpy    = mb_tray_app_xdisplay(TrayApp);
  screen = mb_tray_app_xscreen(TrayApp);

  atom_wm_state  = XInternAtom(dpy, "WM_STATE", False);
  atom_wm_delete = XInternAtom(dpy, "WM_DELETE_WINDOW", False);
  atom_wm_protos = XInternAtom(dpy, "WM_PROTOCOLS", False);

  pb = mb_pixbuf_new(dpy, mb_tray_app_xscreen(TrayApp));

  if (dotdesktop_file != NULL)
    {
      if ((dd = mb_dotdesktop_new_from_file(dotdesktop_file)) != NULL
	  && mb_dotdesktop_get(dd, "Name")
	  && mb_dotdesktop_get(dd, "Icon")
	  && mb_dotdesktop_get(dd, "Exec") )
	{

	  img_file = mb_dotdesktop_get(dd, "Icon");

	  if (img_file[0] != '/')
	    {
	      snprintf(png_path, 256, "%s/pixmaps/%s", DATADIR,  
		       mb_dotdesktop_get(dd, "Icon") );
	      img_file = strdup(png_path);
	    }

	  cmd_str  = mb_dotdesktop_get_exec(dd);
	  if (!win_panel_title) 
	    win_panel_title = mb_dotdesktop_get(dd, "Name");
#ifdef USE_LIBSN

	  if (mb_dotdesktop_get(dd, "SingleInstance")
	      && !strcasecmp(mb_dotdesktop_get(dd, "SingleInstance"), 
			     "true"))
	    {
	      action = ACTION_SI;
	    }
	  else if (mb_dotdesktop_get(dd, "StartupNotify")
		   && !strcasecmp(mb_dotdesktop_get(dd, "StartupNotify"), 
				  "true"))
	    {
	      action = ACTION_SN;
	    }
	  else   
#endif
	    if (mb_dotdesktop_get(dd, "X-MB-NoWindow")
		&& !strcasecmp(mb_dotdesktop_get(dd, "X-MB-NoWindow"), 
			       "true"))
	      {
		DoAnimation = False;
		action = ACTION_NONE;
	      }	
	}
      else
	{
	  fprintf(stderr,"%s: failed to parse %s\n", 
		  argv[0], dotdesktop_file); 
	  exit(1);
	}

    } else {
      img_file = argv[switch_count];

      if (img_file[0] != '/')
	{
	  /* FIXME: should really get from theme */
	  snprintf(png_path, 256, "%s/pixmaps/%s", DATADIR, img_file);
	  img_file = strdup(png_path);
	}

      cmd_str  = arr_to_str(&argv[switch_count+1], argc - switch_count - 1);
    }

  if (!(img_icon = mb_pixbuf_img_new_from_file(pb, img_file)))
    {
      fprintf(stderr, "%s: failed to load image %s \n", 
	      argv[0], img_file );
      exit(1);
    }

  /* make active button image */
  img_icon_active = mb_pixbuf_img_clone(pb, img_icon);

  for (x=0; x<mb_pixbuf_img_get_width(img_icon); x++)
    for (y=0; y<mb_pixbuf_img_get_height(img_icon); y++)
      {
	int aa;
	unsigned char r,g,b,a;
	mb_pixbuf_img_get_pixel (pb, img_icon_active, x, y, &r, &g, &b, &a);

	aa = (int)a;
	aa -=  0x80; if (aa < 0) aa = 0;

	mb_pixbuf_img_set_pixel_alpha(img_icon_active, x, y, aa);
      }


#ifdef USE_LIBSN
  if (action == ACTION_SN || action == ACTION_SI)
    sn_dpy = sn_display_new (dpy, NULL, NULL);
#endif

  mb_tray_app_set_xevent_callback (TrayApp, xevent_callback );

  mb_tray_app_set_button_callback (TrayApp, button_callback );


  if (win_panel_title == NULL) 	/* XXX UTF8 naming */
    {
      win_panel_title = malloc( strlen(argv[1+switch_count]) +
				strlen(" Launcher") + 1 ); 
      strcpy(win_panel_title, argv[1+switch_count]);
      strcat(win_panel_title, " Launcher");
    }
  
  mb_tray_app_set_name (TrayApp, win_panel_title);

  XSelectInput(dpy, mb_tray_app_xrootwin(TrayApp), SubstructureNotifyMask);

  signal(SIGCHLD, SIG_IGN);

  mb_tray_app_set_icon(TrayApp, pb, img_icon);

  /* make sure we always end up on the left of the panel */
  mb_tray_app_request_offset (TrayApp, -1); 

  if (start_app)
    {
      switch(action)
	{
#ifdef USE_LIBSN
	case ACTION_SN:
	  sn_activate(win_panel_title, cmd_str);
	  break;
	case ACTION_SI:
	  si_activate(win_panel_title, cmd_str);
	  break;
#endif
	case ACTION_NONE:
	  fork_exec(cmd_str);
	  break;
	case ACTION_KILL:
	case ACTION_TOGGLE_WIN_STATE:
	  fork_exec(cmd_str);
	  win_launched = get_launch_window();
	  break;
	}
    }

  mb_tray_app_main (TrayApp);

  XCloseDisplay(dpy);
  exit(0);
}
