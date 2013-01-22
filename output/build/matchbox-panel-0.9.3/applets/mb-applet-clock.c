/* 

Minitime - A mini dockable clock.

Copyright (c) 2003 Matthew Allum

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you
       must not claim that you wrote the original software. If you use
       this software in a product, an acknowledgment in the product
       documentation would be appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and
       must not be misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
       distribution.
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <libmb/mb.h>

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(text) gettext(text)
#else
# define _(text) (text)
#endif

#ifdef USE_LIBSN
#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#endif 

#define CONTEXT_APP        "/usr/bin/gpe-conf"
#define CONTEXT_APP_ARGS   "time"
#define CONTEXT_APP_WANT_SN 1

MBFont     *Fnt = NULL;
MBDrawable *Drw  = NULL;
MBColor    *Col  = NULL;
MBPixbuf   *Pixbuf;

void usage()
{
  exit(1);
}

#ifdef USE_LIBSN

static SnDisplay *sn_dpy;

static void 
sn_activate(char *name, char *exec_str)
{
  SnLauncherContext *context;
  pid_t child_pid = 0;

  context = sn_launcher_context_new (sn_dpy, 0);
  
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

void 
fork_exec(char *cmd)
{
  switch (fork())
    {
    case 0:
      setpgid(0, 0); /* Stop us killing child */
      mb_exec(cmd);
      fprintf(stderr, "minitime: Failed to Launch '%s'", cmd);
      exit(1);
    case -1:
      fprintf(stderr, "minitime: Failed to Launch '%s'", cmd);
      break;
    }
}


static void
set_fg_col(MBTrayApp *app, char* spec)
{
  mb_col_set (Col, spec);
}


void
paint_callback ( MBTrayApp *app, Drawable pxm )
{
  struct timeval   tv;
  struct timezone  tz;
  struct tm       *localTime = NULL; 
  time_t           actualTime;
  char             timestr[6] = { 0 };
  MBPixbufImage   *img_bg = NULL;
  MBDrawable      *drw;

  /* Figure out  the actual time */
  gettimeofday(&tv, &tz);
  actualTime = tv.tv_sec;
  localTime = localtime(&actualTime);

  snprintf(timestr, sizeof(timestr), _("%.2d:%.2d"),
           localTime->tm_hour, localTime->tm_min);

  img_bg = mb_tray_app_get_background (app, Pixbuf);

  drw = mb_drawable_new_from_pixmap(Pixbuf, pxm);

  if (mb_tray_app_tray_is_vertical(app))
    {
      /* 
	 - create a new HxW pixmap
	 - rotate background img +90 onto it
	 - render the text to it
	 - call drawable to pixbuf
	 - rotate the new pixbuf -90 
      */
      MBDrawable *drw_rot;
      MBPixbufImage *img_bg_rot, *img_txt, *img_txt_rot;

      int font_y = ((mb_tray_app_width(app) - (mb_font_get_height(Fnt)))/2);

      img_bg_rot = mb_pixbuf_img_transform (Pixbuf, img_bg,
					    MBPIXBUF_TRANS_ROTATE_90);

      drw_rot = mb_drawable_new(Pixbuf, 
				mb_pixbuf_img_get_width(img_bg_rot), 
				mb_pixbuf_img_get_height(img_bg_rot));

      mb_pixbuf_img_render_to_drawable (Pixbuf, img_bg_rot, 
					mb_drawable_pixmap(drw_rot), 0, 0);

      mb_font_render_simple (Fnt, 
			     drw_rot, 
			     1, font_y,
			     mb_pixbuf_img_get_width(img_bg_rot),
			     (unsigned char *) timestr,
			     MB_ENCODING_UTF8,
			     0);

      img_txt = mb_pixbuf_img_new_from_drawable (Pixbuf,
						 mb_drawable_pixmap(drw_rot),
						 None,
						 0, 0,
						 mb_pixbuf_img_get_width(img_bg_rot), 
						 mb_pixbuf_img_get_height(img_bg_rot)); 

      img_txt_rot = mb_pixbuf_img_transform (Pixbuf, img_txt,
					     MBPIXBUF_TRANS_ROTATE_90);
      
      mb_pixbuf_img_render_to_drawable (Pixbuf, img_txt_rot, 
					mb_drawable_pixmap(drw), 0, 0);

      mb_pixbuf_img_free(Pixbuf, img_bg_rot); 
      mb_pixbuf_img_free(Pixbuf, img_txt);
      mb_pixbuf_img_free(Pixbuf, img_txt_rot);

      mb_drawable_unref(drw_rot);

    } else {
      int font_y = ((mb_tray_app_height(app) - (mb_font_get_height(Fnt)))/2);

      mb_pixbuf_img_render_to_drawable (Pixbuf, img_bg, 
					mb_drawable_pixmap(drw), 0, 0);

      mb_font_render_simple (Fnt, 
			     drw, 
			     1, font_y,
			     mb_tray_app_width(app),
			     (unsigned char *) timestr,
			     MB_ENCODING_UTF8,
			     0);
    }

  mb_pixbuf_img_free(Pixbuf, img_bg);
  mb_drawable_unref(drw);

}

void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  if (is_released)
    {
      time_t now;
      struct tm tm;
      char buf[256];
      
      now = time (0);
      localtime_r(&now, &tm);
      strftime (buf, sizeof (buf), _("%a %b %e %k:%M:%S %Y"), &tm);
      
      mb_tray_app_tray_send_message(app, buf, 6000);
    }
}

void
resize_callback ( MBTrayApp *app, int w, int h )
{
  int   req_size   = 0;

  mb_font_set_size_to_pixels (Fnt, 
			      (mb_tray_app_tray_is_vertical(app) ? w : h), 
			      NULL);

  req_size = mb_font_get_txt_width (Fnt, 
				    (unsigned char *) "99999", 5,
				    MB_ENCODING_UTF8);

  if (mb_tray_app_tray_is_vertical(app))
    {
      mb_tray_app_request_size (app, w, req_size+2);
    } else {
      mb_tray_app_request_size (app, req_size+2, h);
    }
}

void 
theme_change_callback (MBTrayApp *app, char *theme_name )
{
  char *theme_path = mb_util_get_theme_full_path(theme_name);

  if (theme_path)
    {
      MBDotDesktop *theme  = NULL;
      char *theme_desktop_path = alloca(strlen(theme_path)+15);

      sprintf(theme_desktop_path, "%s/theme.desktop", theme_path);

      theme = mb_dotdesktop_new_from_file(theme_desktop_path);
      if (theme)
	{
	  /* Get the PanelFgColor key value if exists */
	  if (mb_dotdesktop_get(theme, "PanelFgColor"))
	    {
	      /* Set out font foreground color and repaint */
	      set_fg_col(app, mb_dotdesktop_get(theme, "PanelFgColor"));
	    }
	  mb_dotdesktop_free(theme);
	}
      free(theme_path);
    }
}

void
timeout_callback ( MBTrayApp *app )
{
  struct timeval tv;

  mb_tray_app_repaint (app);

  /* Make sure we get called again in 60 secs - we should get called 
   * exactly on the minute initially.
  */
  tv.tv_usec = 0;
  tv.tv_sec  = 60;

  mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 
}

void
context_callback ( MBTrayApp *app )
{
#ifdef USE_LIBSN
  if (CONTEXT_APP_WANT_SN)
    {
      sn_activate(CONTEXT_APP, CONTEXT_APP " " CONTEXT_APP_ARGS);      
      return;
    }
#endif

  fork_exec(CONTEXT_APP " " CONTEXT_APP_ARGS);
}

static Bool 
file_exists(char *filename)
{
  struct stat st; 		/* XXX Should probably check if exe too */
  if (stat(filename, &st)) return False;
  return True;
}


int 
main(int argc, char **argv)
{
  MBTrayApp *app = NULL;
  MBPixbufImage *img_icon = NULL;
  struct timeval tv;
  char *icon_path;
  struct timezone  tz;
  struct tm       *localTime = NULL; 
  time_t           actualTime;

#if ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, DATADIR "/locale");
  bind_textdomain_codeset (PACKAGE, "UTF-8"); 
  textdomain (PACKAGE);
#endif

  app = mb_tray_app_new ( _("Clock"),
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  

  if (app == NULL) usage();

  Pixbuf = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
			 mb_tray_app_xscreen(app));

  Col  = mb_col_new_from_spec(Pixbuf, "#000000");
  Fnt = mb_font_new_from_string(mb_tray_app_xdisplay(app), "Sans bold"); 

  mb_font_set_color (Fnt, Col);

  memset(&tv,0,sizeof(struct timeval));

  /* Figure out number of seconds till next minute */
  gettimeofday(&tv, &tz);
  actualTime = tv.tv_sec;
  localTime = localtime(&actualTime);
  tv.tv_usec = 0;
  tv.tv_sec  = 60 - localTime->tm_sec;

  /* This we then get reset when first called to 60 */
  mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 

  mb_tray_app_set_button_callback (app, button_callback );

  mb_tray_app_set_theme_change_callback (app, theme_change_callback );

  if (file_exists(CONTEXT_APP))
    {
      mb_tray_app_set_context_info (app, _("Set Time")); 

      mb_tray_app_set_context_callback (app, context_callback); 
    }

  if ((icon_path = mb_dot_desktop_icon_get_full_path (NULL, 
						     16, 
						      "minitime.png"))
      != NULL)
    {
      if ((img_icon = mb_pixbuf_img_new_from_file(Pixbuf, icon_path)) != NULL)
	{
	  mb_tray_app_set_icon(app, Pixbuf, img_icon);
	  mb_pixbuf_img_free(Pixbuf, img_icon);
	}
      free(icon_path);
    }

#ifdef USE_LIBSN
  sn_dpy = sn_display_new (mb_tray_app_xdisplay(app), NULL, NULL);
#endif

  mb_tray_app_main (app);

  return 1;
}
