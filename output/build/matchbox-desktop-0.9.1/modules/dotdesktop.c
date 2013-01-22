#include "mbdesktop_module.h"

#ifdef USE_LIBSN
#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#endif 

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif 

static void
item_activate_cb(void *data1, void *data2);

static char* RootMatchStr = NULL;
static int ItemTypeDotDesktop = 0;

#ifdef USE_LIBSN

static void
item_activate_sn_cb(void *data1, void *data2);

static void
item_activate_si_cb(void *data1, void *data2);

static SnDisplay *SnDpy;

#endif 

static MBDesktopItem *
get_folder_from_name ( MBDesktop *mb, char *name )
{
  MBDesktopItem *item, *item_top;

  item_top = mbdesktop_get_top_level_folder(mb);

  if (!strcasecmp(name, "root"))
    return item_top; 

  mbdesktop_items_enumerate_siblings(mbdesktop_item_get_child(item_top), item)
    {
      if (!strcmp(name, mbdesktop_item_get_name (mb, item)))
	return item;
    }
  
  return NULL;
}

static MBDesktopItem *
match_folder ( MBDesktop *mb, char *category )
{
  MBDesktopItem *item, *item_fallback = NULL, *item_top;
  char          *match_str;  

  /* We dont want 'action' entrys */
  if (category && strstr(category, "Action")) 
    return NULL;

  item_top = mbdesktop_get_top_level_folder(mb);

  /* Add to root window */
  if (RootMatchStr)
    {
      if (!strcmp("fallback", RootMatchStr))
	{
	  item_fallback = item_top;
	}
      else if (category && strstr(category, RootMatchStr))
	{
	  return item_top;
	}
    }

  /* Each root child folder */
  mbdesktop_items_enumerate_siblings(mbdesktop_item_get_child(item_top), item)
    {
      if (mbdesktop_item_get_type (mb, item) == ItemTypeDotDesktop)
	{
	  match_str = (char *)mbdesktop_item_get_user_data(mb, item);
	  if (match_str != NULL)
	    {
	      if (item_fallback == NULL && !strcmp("fallback", match_str))
		{
		  item_fallback = item;
		  continue;
		}
	      if (category && strstr(category, match_str))
		{
		  return item;
		}
	    }
	}
    }
  return item_fallback; 
}

static void
add_a_dotdesktop_item (MBDesktop     *mb, 
		       MBDotDesktop  *dd,
		       MBDesktopItem *folder)
{
  MBDesktopItem  *item_new = NULL, *item_before, *found_folder_item = NULL;
  Bool            have_attached = False;

  if (folder)
    found_folder_item = folder;
  else
    found_folder_item = match_folder( mb, mb_dotdesktop_get(dd, "Categories"));

  if ( found_folder_item == NULL) return;
      
  item_new = mbdesktop_item_new_with_params( mb,
					     mb_dotdesktop_get(dd, "Name"),
					     mb_dotdesktop_get(dd, "Icon"),
					     (void *)mb_dotdesktop_get_exec(dd),
					     ITEM_TYPE_DOTDESKTOP_ITEM
					     );
  if (item_new == NULL ) return;

#ifdef USE_LIBSN
  if (mb_dotdesktop_get(dd, "SingleInstance")
      && !strcasecmp(mb_dotdesktop_get(dd, "SingleInstance"), 
		     "true"))
    {
      mbdesktop_item_set_activate_callback (mb, item_new, 
					    item_activate_si_cb); 
    }
  else if (mb_dotdesktop_get(dd, "StartupNotify")
	   && !strcasecmp(mb_dotdesktop_get(dd, "StartupNotify"), "true"))
    mbdesktop_item_set_activate_callback (mb, item_new, 
					  item_activate_sn_cb); 
  else
#endif
    mbdesktop_item_set_activate_callback (mb, item_new, 
					  item_activate_cb); 

  item_before = mbdesktop_item_get_child(found_folder_item);

  do
    {
      MBDesktopItem *item_next = NULL;
      if ((item_next = mbdesktop_item_get_next_sibling(item_before)) != NULL)
	{
	  if (item_next->type == ITEM_TYPE_FOLDER
	      || item_next->type == ITEM_TYPE_PREVIOUS)
	    continue;
	  
	  if ( (strcasecmp(item_before->name, item_new->name) < 0
		|| item_before->type == ITEM_TYPE_FOLDER
		|| item_before->type == ITEM_TYPE_PREVIOUS )
	      && strcasecmp(item_next->name, item_new->name) > 0)
	    {
	      /*
	      printf("addind '%s' after '%s' before '%s'\n",
		     item_new->name,
		     item_before->name,
		     item_next->name);
	      */
	      have_attached = True;
	      mbdesktop_items_insert_after (mb, item_before, item_new);
	      break;
	    }
	}
    }
  while ((item_before = mbdesktop_item_get_next_sibling(item_before)) != NULL);

  if (!have_attached)
    {
      /* printf("appending '%s' \n", item_new->name); */
      
      mbdesktop_items_append_to_folder( mb, found_folder_item, item_new);
    }

}



int
dotdesktop_init (MBDesktop             *mb, 
		 MBDesktopFolderModule *folder_module, 
		 char                  *arg_str)
{
#define APP_PATHS_N 4

  DIR *dp;
  struct stat    stat_info;

  char vfolder_path_root[512];
  char vfolder_path[512];
  char orig_wd[256];

  int   desktops_dirs_n  = APP_PATHS_N;

  int   i = 0;

  MBDotDesktopFolders     *ddfolders;
  MBDotDesktopFolderEntry *ddentry;
  MBDesktopItem           *item_new = NULL;
  MBDotDesktop            *dd, *user_overides = NULL;

  char                     app_paths[APP_PATHS_N][256];
  struct dirent          **namelist;

#ifdef USE_LIBSN
  SnDpy = sn_display_new (mb->dpy, NULL, NULL);
#endif

  ItemTypeDotDesktop = mbdesktop_module_get_register_type ( mb );
  
  snprintf( vfolder_path_root, 512, "%s/.matchbox/vfolders/Root.directory", 
	    mb_util_get_homedir());
  snprintf( vfolder_path, 512, "%s/.matchbox/vfolders", 
	    mb_util_get_homedir());


  if (stat(vfolder_path_root, &stat_info))
    {
      snprintf(vfolder_path_root, 512, PKGDATADIR "/vfolders/Root.directory");
      snprintf(vfolder_path, 512, PKGDATADIR "/vfolders" );
    }

  dd = mb_dotdesktop_new_from_file(vfolder_path_root);

  if (!dd) 			/* XXX improve */
    { 
      fprintf( stderr, "mb-desktop-dotdesktop: cant open %s\n", vfolder_path ); 
      return -1; 
    }

  RootMatchStr = mb_dotdesktop_get(dd, "Match");

  /* XXX Below is potentially evil 
     - need to figure out a safe way so only one module can
       access the 'root props' at once. 
  */
  mbdesktop_item_set_name (mb, mb->top_head_item, 
			   mb_dotdesktop_get(dd, "Name"));  

  /* Now grab the vfolders */
  ddfolders = mb_dot_desktop_folders_new(vfolder_path);

  mb_dot_desktop_folders_enumerate(ddfolders, ddentry)
    {
      item_new
	= mbdesktop_module_folder_create ( mb,
					   mb_dot_desktop_folder_entry_get_name(ddentry),
					   mb_dot_desktop_folder_entry_get_icon(ddentry));

      mbdesktop_item_set_user_data (mb, item_new, 
				    (void *)mb_dot_desktop_folder_entry_get_match(ddentry));

      mbdesktop_item_set_type (mb, item_new, ItemTypeDotDesktop);

      mbdesktop_items_append_to_top_level (mb, item_new);
    }

  /* Now see if theres a user file overiding any folder mappings */


  /* hmm, just reuse vfolder_path var :/ */
  snprintf(vfolder_path, 512, "%s/.matchbox/desktop/dd-folder-overides",
	   mb_util_get_homedir());
  
  /* 
   *  Format of the .desktop file is ;
   *           path/to/dot/desktop/file=category 
   *
   */
  user_overides = mb_dotdesktop_new_from_file(vfolder_path);

  printf("user_overides is %p\n", user_overides);

  /* Now grep all the .desktop files */

  if (arg_str)
    { 				/* hack to allow just one dir to be searched */
      desktops_dirs_n = 1;	/* Need to figure better way */
      strncpy(app_paths[0], arg_str, 256);
    }
  else
    {
      snprintf(app_paths[0], 256, "%s/applications", DATADIR);
      snprintf(app_paths[1], 256, "/usr/share/applications");
      snprintf(app_paths[2], 256, "/usr/local/share/applications");
      snprintf(app_paths[3], 256, "%s/.applications", mb_util_get_homedir());

    }

  if (getcwd(orig_wd, 255) == (char *)NULL)
    {
      fprintf(stderr, "Cant get current directory\n");
      return -1;
    }

  for (i = 0; i < desktops_dirs_n; i++)
    {
#ifdef USE_DNOTIFY
      int fd;
#endif
      
      int   n = 0, j = 0;

      /* Dont reread default */
      if (i > 0 && !strcmp(app_paths[0], app_paths[i]))
	continue;

      if ((dp = opendir(app_paths[i])) == NULL)
	{
	  fprintf(stderr, "mb-desktop-dotdesktop: failed to open %s\n", app_paths[i]);
	  continue;
	}

#ifdef USE_DNOTIFY
      fd = open(app_paths[i], O_RDONLY);
      fcntl(fd, F_SETSIG, SIGRTMIN);
      fcntl(fd, F_NOTIFY, DN_RENAME|DN_MODIFY|DN_CREATE|DN_DELETE|DN_MULTISHOT);
#endif // USE_DNOTIFY

  
      chdir(app_paths[i]);

      n = scandir(".", &namelist, 0, alphasort);
      /*      while((dir_entry = readdir(dp)) != NULL) */
      while (j < n && n > 0)
	{

	  if (namelist[j]->d_name[0] ==  '.')
	    goto end;

	  if (strcmp(namelist[j]->d_name+strlen(namelist[j]->d_name)-8,".desktop"))
	    goto end;

	  lstat(namelist[j]->d_name, &stat_info);
	  if (!(S_ISDIR(stat_info.st_mode)))
	    {
	      MBDotDesktop *dd;
	      dd = mb_dotdesktop_new_from_file(namelist[j]->d_name);
	      if (dd)
		{
		  if (mb_dotdesktop_get(dd, "Type") 
		      && !strcmp(mb_dotdesktop_get(dd, "Type"), "Application")
		      && mb_dotdesktop_get(dd, "Name")
		      && mb_dotdesktop_get(dd, "Exec"))
		    {
		      MBDesktopItem *folder = NULL;
		      char           full_path[512];
		      char          *folder_name = NULL;

		      if (user_overides)
			{
			  snprintf(full_path, 512, "%s/%s", 
				   app_paths[i], namelist[j]->d_name);
			  if ((folder_name = mb_dotdesktop_get(user_overides, 
							       full_path)) 
			      != NULL )
			    {
			      folder = get_folder_from_name(mb, folder_name);
			    }
			}

		      add_a_dotdesktop_item (mb, dd, folder);
		    }
		  mb_dotdesktop_free(dd);
		}
	    }
	end:
	  free(namelist[j]);
	  ++j;

	}
      
      closedir(dp);
      free(namelist);
    }
  chdir(orig_wd);

  if (user_overides) mb_dotdesktop_free(user_overides);

  return 1;
}

/* Activate callbacks */


#ifdef USE_LIBSN
static void
item_activate_sn_cb(void *data1, void *data2)
{
  MBDesktop *mb = (MBDesktop *)data1;
  MBDesktopItem *item = (MBDesktopItem *)data2;

  SnLauncherContext *context;
  pid_t child_pid = 0;

  context = sn_launcher_context_new (SnDpy, mb->scr);

  sn_launcher_context_set_name (context, item->name);
  if (item->comment)
    sn_launcher_context_set_description (context, item->comment);
  sn_launcher_context_set_binary_name (context, (char *)item->data);

  sn_launcher_context_initiate (context, "mbdesktop launch", 
				(char *)item->data, CurrentTime);

  switch ((child_pid = fork ()))
    {
    case -1:
      fprintf (stderr, "Fork failed\n" );
      break;
    case 0:
      sn_launcher_context_setup_child_process (context);
      mb_exec((char *)item->data);
      // execlp(item->exec_str, item->exec_str, NULL);
      fprintf (stderr, "Failed to exec %s \n", (char *)item->data);
      _exit (1);
      break;
    }
  mb_util_animate_startup(mb->dpy, item->x, item->y, 
			  item->width, item->height); 

}
#endif

#ifdef USE_LIBSN
static void
item_activate_si_cb(void *data1, void *data2)
{
  MBDesktop *mb = (MBDesktop *)data1;
  MBDesktopItem *item = (MBDesktopItem *)data2;
  Window win_found;

  if (mb_single_instance_is_starting(mb->dpy, (char *)item->data))
    return;

  win_found = mb_single_instance_get_window(mb->dpy, (char *)item->data);

  if (win_found != None)
    {
      mb_util_animate_startup(mb->dpy, item->x, item->y, 
			      item->width, item->height); 
      mb_util_window_activate(mb->dpy, win_found);
    }
  else item_activate_sn_cb((void *)mb, (void *)item);

}
#endif

static void
item_activate_cb(void *data1, void *data2)
{
  MBDesktop *mb = (MBDesktop *)data1;
  MBDesktopItem *item = (MBDesktopItem *)data2;

  switch (fork())
    {
    case 0:
      mb_exec((char *)item->data);
      fprintf(stderr, "exec failed, cleaning up child\n");
      exit(1);
    case -1:
      fprintf(stderr, "can't fork\n");
      break;
    }

  mb_util_animate_startup(mb->dpy, item->x, item->y, item->width, item->height); 

}

#define MODULE_NAME         "DotDesktop App Launcher"
#define MODULE_DESC         "DotDesktop App Launcher"
#define MODULE_AUTHOR       "Matthew Allum"
#define MODULE_MAJOR_VER    0
#define MODULE_MINOR_VER    0
#define MODULE_MICRO_VER    1
#define MODULE_API_VERSION  0
 
MBDesktopModuleInfo dotdesktop_info = 
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
    &dotdesktop_info,
    dotdesktop_init,
    NULL,
    NULL
  };
