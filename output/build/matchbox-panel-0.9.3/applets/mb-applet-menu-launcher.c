/*
   mb-applet-menu-launcher - a small application launcher

   Copyright 2002 Matthew Allum

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
*/

/*
  TODO
  o fix crash on menu's bigger than display.
  o add logout / lock buttons on root menu.
     - add use_gpe defines for this !
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef USE_DNOTIFY
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/wait.h>

#include <libmb/mb.h>


#ifdef USE_LIBSN
#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#endif

#ifndef MB_HAVE_PNG
#include <setjmp.h>
#endif

typedef struct _app
{
  MBTrayApp     *tray_app; 
  MBMenu        *mbmenu;
  MBPixbuf      *pb;

  MBPixbufImage *img_tray;
  MBPixbufImage *img_tray_scaled;
  MBPixbufImage *img_tray_active;
  MBPixbufImage *img_tray_active_scaled;

  Atom           mbtheme_atom;
  Atom           mbcommand_atom;

  char          *theme_name;
  Bool           button_is_down;

#ifdef USE_LIBSN
  SnDisplay  *sn_display;
#endif
   
} AppData;

AppData     *app_data;

MBMenuMenu  *root;
Bool         Update_Pending  = False;
jmp_buf      Jbuf;
MBMenuMenu  *active[10];
int          MenuWasActive   = False;

Bool         WantDebianMenus = False;
volatile Bool WantReload     = False;


static void reap_children (int signum);
static void fork_exec (char *cmd);
static void usage ();
static int  parse_menu_file (char *data);
static void build_menu (void);
static void catch_sigsegv (int sig);
static void load_icon(void);

#ifdef USE_LIBSN
static void sn_exec(char* name, char* bin_name, char *desc);
#endif

#ifdef USE_DNOTIFY
static void reload_menu(int signum, siginfo_t *siginfo, void *data);
#endif

#ifdef MB_HAVE_PNG
#define TRAY_IMG "mbmenu.png"
#define TRAY_IMG_ACTIVE "mbmenu-active.png"
#else
#define TRAY_IMG "mbmenu.xpm"
#define TRAY_IMG_ACTIVE "mbmenu-active.xpm"
#endif

#define SINGLE_INSTANCE 1
#define STARTUP_NOTIFY  2

#define UP   0
#define DOWN 1

#define MENUDIR "/usr/lib/menu"

#define MBMAX(x,y) ((x>y)?(x):(y))

#define NEW(OBJ) ((OBJ *)(malloc(sizeof(OBJ))))

#define IS_WHITESPACE(c) ((c) == ' ' || (c) == '\\' \
                          || (c) == '\t' || (c)=='\n' || (c)=='?')

#ifdef DEBUG
#define DBG(txt, args... ) fprintf(stderr, "DEBUG: " txt , ##args )
#else
#define DBG(txt, args... ) /* nothing */
#endif

static void usage()
{
  printf("usage: mbmenu [options ....]\n"
	  "Where options are\n"
	  "  -display <display> Display to connect to\n"
	  "\nAlso set MB_USE_DEB_MENUS env var to enable parsing /usr/lib/menu"
	  "\n"

	  );
   exit(1);
}

static void catch_sigsegv(int sig)
{
   DBG("ouch\n");
   signal(SIGSEGV, SIG_DFL);
   longjmp(Jbuf, 1);

}

static void reap_children(int signum)
{
    pid_t pid;

    do {
	pid = waitpid(-1, NULL, WNOHANG);
    } while (pid > 0);
}

#ifdef USE_LIBSN

static void 
sn_exec(char* name, char* bin_name, char *desc)
{
  SnLauncherContext *context;
  pid_t child_pid = 0;

  context = sn_launcher_context_new (app_data->sn_display, 
				     mb_tray_app_xscreen(app_data->tray_app));
  
  if (name)     sn_launcher_context_set_name (context, name);
  if (desc)     sn_launcher_context_set_description (context, desc);
  if (bin_name) sn_launcher_context_set_binary_name (context, bin_name);
  
  sn_launcher_context_initiate (context, "mbmenu launch", bin_name,
				CurrentTime);

  switch ((child_pid = fork ()))
    {
    case -1:
      fprintf (stderr, "Fork failed\n" );
      break;
    case 0:
      sn_launcher_context_setup_child_process (context);
      mb_exec(bin_name);
      fprintf (stderr, "mb-applet-menu-launcher: Failed to exec %s \n", bin_name);
      _exit (1);
      break;
    }
}

#endif

static void fork_exec(char *cmd)
{
    pid_t pid;
    
    switch (pid = fork()) 
      {
      case 0:
	mb_exec(cmd);
	/* execlp("/bin/sh", "sh", "-c", cmd, NULL); */
	fprintf(stderr, "mb-applet-menu-launcher: exec of '%s' failed, cleaning up child\n", cmd);
	exit(1);
      case -1:
	fprintf(stderr, "mb-applet-menu-launcher: can't fork '%s'\n", cmd); break;
      }
}

#ifdef USE_LIBSN
static
void menu_clicked_sn_cb(MBMenuItem *item)
{
  sn_exec(item->title, item->info, item->info);
}

static
void menu_clicked_si_cb(MBMenuItem *item)
{
  Window win_found;

  win_found = mb_single_instance_get_window(mb_tray_app_xdisplay(app_data->tray_app), item->info);

  if (win_found != None)
    {
      mb_util_window_activate(mb_tray_app_xdisplay(app_data->tray_app), win_found);
    }
  else menu_clicked_sn_cb(item);
}
#endif

static
void menu_clicked_cb(MBMenuItem *item)
{
  fork_exec(item->info);
}

static int
parse_menu_file(char *data)
{
   char *p, *key, *val;

   char *tmp_title;
   char *tmp_section;
   char *tmp_cmd;
   char *tmp_icon;
   char *tmp_needs;

   signal(SIGSEGV, catch_sigsegv);
   if (setjmp(Jbuf)) return 0;  /* catch possible parse segfualt  */

   tmp_title = NULL;
   tmp_section = NULL;
   tmp_cmd = NULL;
   tmp_icon = NULL;
   tmp_needs = NULL;
   p = data;
   
   if (*p != '?') return 0;   /* check is an actual menu entry  */
   
   while(*(++p) != ':'); *p = ' '; /* skip to first entry */
   
   while(*p != '\0')
   {
      if ((!IS_WHITESPACE(*p))
	  || (IS_WHITESPACE(*p) && *(p+1) != '\0'
	      && (!IS_WHITESPACE(*(p+1)))))
      {
	 /* process key=pair */
         char *lc = " \t\n\\";
	 char *sc = "\"";
	 char *tc = lc; 
         if (IS_WHITESPACE(*p)) p++;
	 key = p;
	 while(*p != '=') p++; *p = '\0';
	 DBG("\tkey %s ", key);
         if (*(++p) == '"') { p++; tc = sc; } /* skip "'s */
	 val = p;
	 while(strchr(tc,*p) == NULL)
	 {
	    if (*p == '\\' && *(p+1) == '"') p++;  /* skip \" */
	    p++;
	 } *p = '\0';
	 DBG("value %s \n", val);

	 if(!strcmp(key,"title"))        { tmp_title = val;  }
	 else if(!strcmp(key,"section")) { tmp_section = val; }
	 else if(!strcmp(key,"command")) { tmp_cmd = val; }
	 else if(!strcmp(key,"icon"))    { tmp_icon = val; }
	 else if(!strcmp(key,"icon16"))    { tmp_icon = val; }
	 else if(!strcmp(key,"icon32"))    { tmp_icon = val; }
	 else if(!strcmp(key,"icon48"))    { tmp_icon = val; }
	 else if(!strcmp(key,"needs"))    { tmp_needs = val; }

      }
      p++;

      if (tmp_section && (*p == '?' || *p == '\0'))
      {
	 if ( (!strcmp(tmp_needs,"x11"))
	      || (!strcmp(tmp_needs,"X11"))
	      || (!strcmp(tmp_needs,"text"))
	      || tmp_needs == NULL )
	 {
	  MBMenuMenu *m = NULL;

	   char *tmpstr = 
	     (char *)malloc(sizeof(char)*(strlen(tmp_section)+
					  strlen(tmp_title)+12));
	   sprintf(tmpstr, "Other/%s", tmp_section);

	   m = mb_menu_add_path(app_data->mbmenu, tmpstr, NULL, 0);
	   mb_menu_add_item_to_menu(app_data->mbmenu, m, tmp_title, 
				       tmp_icon, tmp_cmd, 
				    menu_clicked_cb, NULL, 0); 
	 }
      }
   }	 
	 /* reset all */
      tmp_title = NULL;
      tmp_section = NULL;
      tmp_cmd = NULL;
      tmp_icon = NULL;
      
      /* new menu entry */
      
      if ( *p == '?')
	 {
	   DBG("new entry, igonoring package(foo) data\n");
	   while(*(++p) != ':');
	   *p = ' ';
	 }
      
   
   signal(SIGSEGV, SIG_DFL);
   return 1;
}

#ifdef USE_DNOTIFY
static
void really_reload_menu(int signum)
{
  DBG("Really Reload menu callback\n");

  WantReload = True;

}


static 
void reload_menu(int signum, siginfo_t *siginfo, void *data)
{
  DBG("Reload menu callback\n");

  /* To avoid the barrage of signals dnotify likes to send when
   * a file is updated we set an alarm.
   *
   */
  signal (SIGALRM, really_reload_menu);
  alarm(3);

}
#endif

static void
build_menu(void)
{

#define APP_PATHS_N 4

  struct menu_lookup_t {
    char       *match_str;
    MBMenuMenu *item;
  };

  int i = 0, j = 0;
  DIR *dp;
  struct dirent *dir_entry;
  struct stat stat_info;

#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

  char orig_wd[MAXPATHLEN];

  char dirs[2][256] = { MENUDIR };
  FILE *fp;
  char *buf;
  int len;
  MBMenuMenu *menu_panel;
  char *tmp_path = NULL, *tmp_path2 = NULL ;

  char vfolder_path_root[512];
  char vfolder_path[512];
  MBDotDesktop            *dd;
  MBDotDesktopFolders     *ddfolders;
  MBDotDesktopFolderEntry *ddentry;
  char                    *root_match_str;

  /* List of paths to check for .desktop files */
  char                     app_paths[APP_PATHS_N][256];
  Bool got_root_items        = False;
  Bool had_action_sepearator = False;

  struct menu_lookup_t *menu_lookup = NULL;

  snprintf( vfolder_path_root, 512, "%s/.matchbox/vfolders/Root.directory", getenv("HOME"));
  snprintf( vfolder_path, 512, "%s/.matchbox/vfolders", getenv("HOME"));

  if (stat(vfolder_path_root, &stat_info))
    {
      snprintf(vfolder_path_root, 512, PKGDATADIR "/vfolders/Root.directory");
      snprintf(vfolder_path, 512, PKGDATADIR "/vfolders" );
    }
  
  dd = mb_dotdesktop_new_from_file(vfolder_path_root);

  if (!dd) 	
    { 
      fprintf( stderr, 
	       "mb-applet-menu-launcher: cant open " PKGDATADIR "/vfolders/Root.desktop\n"
	       "                         Did you install matchbox-common ?" ); 
      exit(1); 
    }

  root_match_str = mb_dotdesktop_get(dd, "Match");

  /* Now grab the vfolders */
  ddfolders = mb_dot_desktop_folders_new(vfolder_path);

  menu_lookup = malloc(sizeof(struct menu_lookup_t)*mb_dot_desktop_folders_get_cnt(ddfolders));

  mb_dot_desktop_folders_enumerate(ddfolders, ddentry)
    {
      char *icon_path = NULL;
      char *folder_name = NULL;

      /* Check Name is valid for entry */
      if ((folder_name = mb_dot_desktop_folder_entry_get_name(ddentry)) == NULL)
	continue;

      if (mb_dot_desktop_folder_entry_get_icon(ddentry))
	icon_path = mb_dot_desktop_icon_get_full_path (app_data->theme_name, 
						       16, 
						       mb_dot_desktop_folder_entry_get_icon(ddentry) );

      menu_lookup[i].item = mb_menu_add_path(app_data->mbmenu, 
					     folder_name,
					     icon_path, MBMENU_NO_SORT );

      menu_lookup[i].match_str = mb_dot_desktop_folder_entry_get_match(ddentry);
      i++;

    }

  menu_panel = mb_menu_add_path(app_data->mbmenu, "Utilities/Panel" , NULL, MBMENU_NO_SORT );


  tmp_path = mb_dot_desktop_icon_get_full_path (app_data->theme_name, 
						16, 
						"mbfolder.png" );

  tmp_path2 = mb_dot_desktop_icon_get_full_path (app_data->theme_name, 
						16, 
						"mbnoapp.png" );

  mb_menu_set_default_icons(app_data->mbmenu, tmp_path, tmp_path2);

  if (tmp_path) free(tmp_path);
  if (tmp_path2) free(tmp_path2);

  if (getcwd(orig_wd, MAXPATHLEN) == (char *)NULL)
    {
      fprintf(stderr, "mb-applet-menu-launcher: cant get current directory\n");
      exit(0);
    }

  snprintf(app_paths[0], 256, "%s/applications", DATADIR);  
  snprintf(app_paths[1], 256, "/usr/share/applications");
  snprintf(app_paths[2], 256, "/usr/local/share/applications");
  snprintf(app_paths[3], 256, "%s/.applications", getenv("HOME"));

  
  for (j = 0; j < APP_PATHS_N; j++)
    {

      /* Dont reread the prefix path if matches */
      if (j > 0 && !strcmp(app_paths[0], app_paths[j]))
	continue;

      if ((dp = opendir(app_paths[j])) == NULL)
	{
	  fprintf(stderr, "mb-applet-menu-launcher: failed to open %s\n", 
		  app_paths[j]);
	  continue;
	}
      
      chdir(app_paths[j]);

      while((dir_entry = readdir(dp)) != NULL)
	{
	  if (strcmp(dir_entry->d_name+strlen(dir_entry->d_name)-8,".desktop"))
	    continue;
	  
	  lstat(dir_entry->d_name, &stat_info);
	  if (!(S_ISDIR(stat_info.st_mode)))
	    {
	      MBDotDesktop *dd;
	      int flags = 0;
	      
	      dd = mb_dotdesktop_new_from_file(dir_entry->d_name);
	      if (dd)
		{
		  char *png_path = NULL;
		  MBMenuActivateCB activate_callback = NULL;
		  
		  if (mb_dotdesktop_get(dd, "Icon") 
		      && mb_dotdesktop_get(dd, "Name")
		      && mb_dotdesktop_get(dd, "Exec"))
		    {
		      MBMenuMenu *m = NULL, *fallback = NULL;
		      char *category;
		      
		      png_path = mb_dot_desktop_icon_get_full_path(app_data->theme_name, 16, mb_dotdesktop_get(dd, "Icon"));
		      
		      category = mb_dotdesktop_get(dd, "Categories");
		      
		      if (png_path && category && strstr(category, "Action"))
			{
			  m = app_data->mbmenu->rootmenu;

			  if (!had_action_sepearator)
			    {
			        mb_menu_add_seperator_to_menu(app_data->mbmenu,
							      app_data->mbmenu->rootmenu, 
							      MBMENU_NO_SORT);
				had_action_sepearator = True;
			    }

			  flags = MBMENU_NO_SORT;
			}
		      else
			{
			  if (root_match_str)
			    {
			      if (!strcmp("fallback", root_match_str))
				{
				  fallback = app_data->mbmenu->rootmenu;
				}
			      else if (category 
				       && strstr(category, root_match_str))
				{
				  m = app_data->mbmenu->rootmenu;
				}
			    }
			  
			  if (m == NULL 
			      && category != NULL)
			    {
			      for (i=0; 
				   i<mb_dot_desktop_folders_get_cnt(ddfolders);
				   i++)
				{
				  if (!strcmp(menu_lookup[i].match_str,
					      "fallback"))
				    {
				      fallback = menu_lookup[i].item;
				    }
				  if (strstr(category, 
					     menu_lookup[i].match_str))
				    {
				      m = menu_lookup[i].item;
				    }
				}
			    }
			  
			  if (m == NULL) m = fallback;
			  
			}
		      activate_callback = menu_clicked_cb;
#ifdef USE_LIBSN
		      
		      if (mb_dotdesktop_get(dd, "SingleInstance")
			  && !strcasecmp(mb_dotdesktop_get(dd, 
							   "SingleInstance"),
					 "true"))
			{
			  activate_callback = menu_clicked_si_cb;
			}
		      else if (mb_dotdesktop_get(dd, "StartupNotify")
			       && !strcasecmp(mb_dotdesktop_get(dd, 
								"StartupNotify"),
					      "true"))
			{
			  activate_callback = menu_clicked_sn_cb;
			}
#endif
		      if (mb_dotdesktop_get(dd, "Type") 
			  && !strcmp(mb_dotdesktop_get(dd, "Type"), 
				     "PanelApp"))
			{
			  m = menu_panel;
			}
		      
		      if (png_path && m)
			{
			  
			  if (!flags && m == app_data->mbmenu->rootmenu)
			    {
			      if (got_root_items == False)
				{
				  mb_menu_add_seperator_to_menu(app_data->mbmenu, 
								app_data->mbmenu->rootmenu, 
								MBMENU_PREPEND);
				  got_root_items = True;
				}
			      
			      flags = MBMENU_PREPEND;
			    }
			  
			  mb_menu_add_item_to_menu(app_data->mbmenu, m, 
						   mb_dotdesktop_get(dd, "Name"),
						   png_path,
						   mb_dotdesktop_get_exec(dd),
						   activate_callback, 
						   (void *)app_data, flags); 
			  /* mb_menu_add_seperator_to_menu(app_data-> mbmenu, m); */
			  
			  free(png_path);
			}
		    }
		  else fprintf(stderr, 
			       "mb-applet-menu-launcher: %s has no icon, png or name\n", 
			       dir_entry->d_name);
		  mb_dotdesktop_free(dd);
		}
	      else
		fprintf(stderr, "mb-applet-menu-launcher: failed to parse %s :( \n", dir_entry->d_name);
	    }
	}
  
      closedir(dp);
    }
  /* Now parse old Debian / Familiar Menu entrys */
  chdir(orig_wd);

  if (WantDebianMenus)
    {
      strcpy(dirs[1], (char *)getenv("HOME"));
      strcat(dirs[1], "/.menu");
      
      for(i=0; i<2; i++)
	{
	  if ((dp = opendir(dirs[i])) == NULL)
	    {
	      fprintf(stderr, "mb-applet-menu-launcher: failed to open %s\n", dirs[i]);
	      continue;
	    }
	  
	  chdir(dirs[i]);
	  while((dir_entry = readdir(dp)) != NULL)
	    {
	      lstat(dir_entry->d_name, &stat_info);
	      if (!(S_ISDIR(stat_info.st_mode)))
		{
		  DBG("file %s \n", dir_entry->d_name);
		  
		  fp = fopen(dir_entry->d_name, "r");
		  buf = malloc(sizeof(char) * (stat_info.st_size + 1));
		  len = fread(buf, 1, stat_info.st_size, fp);
		  if (len >= 0) buf[len] = '\0';
		  if (!(parse_menu_file(buf)))
		    fprintf(stderr, "mb-applet-menu-launcher: had problems parsing %s. Ignoring. \n",
			    dir_entry->d_name);
		  DBG("done\n\n");
		  fclose(fp);
		  free(buf);
		}
	    }

	  closedir(dp);

	}
      chdir(orig_wd);
    }

}

static void
menu_get_popup_pos (MBTrayApp *app, int *x, int *y)
{
  int abs_x, abs_y, menu_h, menu_w;
  mb_tray_app_get_absolute_coords (app, &abs_x, &abs_y);
  mb_menu_get_root_menu_size(app_data->mbmenu, &menu_w, &menu_h);

  if (mb_tray_app_tray_is_vertical (app))
    {
      /* XXX need to figure out menu size before its mapped 
	     so we can figure out offset for east panel
      */
      *y = abs_y + mb_tray_app_height(app);

      if (abs_x > (DisplayWidth(mb_tray_app_xdisplay(app), mb_tray_app_xscreen(app)) /2))
	*x = abs_x - menu_w - 2;
      else
	*x = abs_x + mb_tray_app_width(app) + 2;
    }
  else
    {
      *x = abs_x;
      if (abs_y > (DisplayHeight(mb_tray_app_xdisplay(app), mb_tray_app_xscreen(app)) /2))
	*y = abs_y - 2;
      else
	*y = abs_y + mb_tray_app_height(app) + menu_h;
    }
}

void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  int abs_x, abs_y;
  static Bool next_cancels;

#ifdef USE_DNOTIFY
  sigset_t block_sigset;
#endif

  app_data->button_is_down = True;
  if (is_released) app_data->button_is_down = False;

  if (is_released && !next_cancels)
    {

#ifdef USE_DNOTIFY 		/* block any reloads while active */
      sigemptyset(&block_sigset);
      sigaddset(&block_sigset, SIGRTMIN);
      sigprocmask(SIG_BLOCK, &block_sigset, NULL); 
#endif
	  
      menu_get_popup_pos (app, &abs_x, &abs_y);
      mb_menu_activate (app_data->mbmenu, abs_x, abs_y);
    }
  else
    if (mb_menu_is_active(app_data->mbmenu))
      next_cancels = True;
    else
      next_cancels = False;

  mb_tray_app_repaint (app);
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{
  MBPixbufImage *img_bg = NULL;
  MBPixbufImage *img_button = NULL;
  
  img_button = ( app_data->button_is_down ? app_data->img_tray_active_scaled : app_data->img_tray_scaled ); 

  img_bg = mb_tray_app_get_background (app, app_data->pb);

  mb_pixbuf_img_copy_composite (app_data->pb, img_bg, 
				img_button, 
				0, 0, 
				mb_pixbuf_img_get_width(app_data->img_tray_scaled), 
				mb_pixbuf_img_get_height(app_data->img_tray_scaled), 
				0, 0 );
  
  mb_pixbuf_img_render_to_drawable (app_data->pb, img_bg, drw, 0, 0);

  mb_pixbuf_img_free( app_data->pb, img_bg );
}

void
resize_callback (MBTrayApp *app, int w, int h)
{
  if (app_data->img_tray_scaled) 
    mb_pixbuf_img_free(app_data->pb, 
		       app_data->img_tray_scaled);

  app_data->img_tray_scaled = mb_pixbuf_img_scale(app_data->pb, 
						  app_data->img_tray, 
						  w, h);
  if (app_data->img_tray_active_scaled) 
    mb_pixbuf_img_free(app_data->pb, 
		       app_data->img_tray_active_scaled);

  app_data->img_tray_active_scaled 
    = mb_pixbuf_img_scale(app_data->pb, 
			  app_data->img_tray_active, 
			  w, h);
}

void 
theme_callback (MBTrayApp *app, char *theme_name)
{
  if (!theme_name) return;
  if (app_data->theme_name) free(app_data->theme_name);
  app_data->theme_name = strdup(theme_name);

  if (app_data->mbmenu != NULL)
    {
      mb_menu_free(app_data->mbmenu);
      build_menu();
      load_icon();
      resize_callback (app, mb_tray_app_width(app), mb_tray_app_width(app) );
      mb_tray_app_repaint (app_data->tray_app);
    }
}

void
xevent_callback (MBTrayApp *app, XEvent *ev)
{
#ifdef USE_DNOTIFY
  sigset_t block_sigset;
#endif

  mb_menu_handle_xevent (app_data->mbmenu, ev);  

#ifdef USE_DNOTIFY
  if (!mb_menu_is_active(app_data->mbmenu))
    { 				/* Unblock any dnotify signals */
      sigemptyset(&block_sigset);
      sigaddset(&block_sigset, SIGRTMIN);
      sigprocmask(SIG_UNBLOCK, &block_sigset, NULL); 
    }
#endif

#define MB_CMD_SHOW_EXT_MENU 6

  if (ev->type == ClientMessage)
    {
      if (ev->xclient.message_type == app_data->mbcommand_atom
	  && ev->xclient.data.l[0] == MB_CMD_SHOW_EXT_MENU )
	{
#ifdef USE_DNOTIFY
	  sigemptyset(&block_sigset);
	  sigaddset(&block_sigset, SIGRTMIN);
#endif
	  if (!mb_menu_is_active(app_data->mbmenu)) 
	    {
	      int abs_x, abs_y;
#ifdef USE_DNOTIFY
	      sigprocmask(SIG_BLOCK, &block_sigset, NULL); 
#endif
	      menu_get_popup_pos (app, &abs_x, &abs_y);
	      mb_menu_activate(app_data->mbmenu, abs_x, abs_y);
	    } else {
	      mb_menu_deactivate(app_data->mbmenu);
#ifdef USE_DNOTIFY
	      sigprocmask(SIG_UNBLOCK, &block_sigset, NULL); 
#endif
	    }
	}
    }

  if (WantReload && !mb_menu_is_active(app_data->mbmenu))
    {
      DBG("reloading menu\n");
      mb_menu_free(app_data->mbmenu);
      build_menu();
      WantReload = False;
    }
}

void 
load_icon(void)
{
 char *icon_path = NULL;
 
 if (app_data->img_tray) 
    mb_pixbuf_img_free(app_data->pb, app_data->img_tray);

 if (app_data->img_tray_active) 
    mb_pixbuf_img_free(app_data->pb, app_data->img_tray_active);
 
 icon_path = mb_dot_desktop_icon_get_full_path (app_data->theme_name, 
						16, 
						TRAY_IMG );

  if (icon_path == NULL 
      || !(app_data->img_tray 
	   = mb_pixbuf_img_new_from_file(app_data->pb, icon_path)))
    {
      fprintf(stderr, "mb-applet-menu-launcher: failed to load panel icon\n");
      exit(1);
    }

  free(icon_path);

  icon_path = mb_dot_desktop_icon_get_full_path (app_data->theme_name, 
						 16, 
						 TRAY_IMG_ACTIVE );

  if (icon_path == NULL 
      || !(app_data->img_tray_active
	   = mb_pixbuf_img_new_from_file(app_data->pb, icon_path)))
    {
      int x, y;

      app_data->img_tray_active 
	= mb_pixbuf_img_clone(app_data->pb, app_data->img_tray);

      for (x=0; x<mb_pixbuf_img_get_width(app_data->img_tray_active); x++)
	for (y=0; y<mb_pixbuf_img_get_height(app_data->img_tray_active); y++)
	  {
	    int aa;
	    unsigned char r,g,b,a;
	    mb_pixbuf_img_get_pixel(app_data->pb, app_data->img_tray_active,
				    x, y, &r, &g, &b, &a);

	    aa = (int)a;
	    aa -=  0x80; if (aa < 0) aa = 0;

	    mb_pixbuf_img_set_pixel_alpha(app_data->img_tray_active, 
					  x, y, aa);
	  }
    }

  if (icon_path) free(icon_path);
}

int
main( int argc, char *argv[])
{
  MBTrayApp *app = NULL;
  struct sigaction act;

#ifdef USE_DNOTIFY
  int fd;
#endif

  app_data = NEW(AppData);
  memset(app_data, 0, sizeof(AppData));

  app = mb_tray_app_new ( "App Launcher",
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  

  if (app == NULL) usage();
  
  if (argc > 1 && strstr(argv[1], "-h"))
    usage();

  if (getenv("MB_USE_DEB_MENUS"))
    WantDebianMenus = True;

  app_data->tray_app = app;

  app_data->mbtheme_atom 
    = XInternAtom(mb_tray_app_xdisplay(app), "_MB_THEME", False);
  app_data->mbcommand_atom 
    = XInternAtom(mb_tray_app_xdisplay(app), "_MB_COMMAND", False);
  
  app_data->mbmenu = mb_menu_new(mb_tray_app_xdisplay(app), 
				 mb_tray_app_xscreen(app));
  
#ifdef USE_LIBSN
  
  app_data->sn_display = sn_display_new (mb_tray_app_xdisplay(app), 
					  NULL, NULL); 

#endif

   app_data->pb = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
				mb_tray_app_xscreen(app));

   XSelectInput (mb_tray_app_xdisplay(app), mb_tray_app_xrootwin(app), 
		 PropertyChangeMask|SubstructureNotifyMask);

   mb_tray_app_set_button_callback (app, button_callback );

   mb_tray_app_set_xevent_callback (app, xevent_callback );

   mb_tray_app_set_theme_change_callback (app, theme_callback );

   /* Make the tray app end up on right of mb panel */
   mb_tray_app_request_offset (app, -1); 

   load_icon();

   build_menu();

   /* Set up signals */

   act.sa_flags = 0;
   sigemptyset(&act.sa_mask);
   act.sa_handler = reap_children;
   sigaction(SIGCHLD, &act, NULL);

#ifdef USE_DNOTIFY

#define DD_DIR DATADIR "/applications"

   act.sa_sigaction = reload_menu;
   sigemptyset(&act.sa_mask);
   act.sa_flags = SA_SIGINFO;
   sigaction(SIGRTMIN, &act, NULL);
   
   fd = open(DD_DIR, O_RDONLY);
   fcntl(fd, F_SETSIG, SIGRTMIN);
   fcntl(fd, F_NOTIFY, DN_MODIFY|DN_CREATE|DN_DELETE|DN_MULTISHOT);
   
#endif 

  mb_tray_app_set_icon(app, app_data->pb, app_data->img_tray);

  mb_tray_app_main (app);
   
   return 1;
}


