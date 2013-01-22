#include "panel_menu.h"

#ifdef MB_HAVE_PNG
#define FOLDER_IMG  "mbfolder.png"
#define ADD_IMG     "mbadd.png"
#define REMOVE_IMG  "mbremove.png"
#define HIDE_IMG    "mbdown.png"
#else
#define FOLDER_IMG  "mbfolder.xpm"
#define ADD_IMG     "mbadd.xpm"
#define REMOVE_IMG  "mbremove.xpm"
#define HIDE_IMG    "mbdown.xpm"
#endif

void panel_menu_exec_cb(MBMenuItem *item)
{
  char *cmd = strdup(item->info);

  util_fork_exec(cmd);
  free(cmd);
}

void panel_menu_exit_cb(MBMenuItem *item)
{
  /* This should only be called by the menu. 
     As we now delete the session file. So its not used again. 
  */
  MBPanel *panel = (MBPanel *)item->cb_data;
  session_destroy(panel);

  util_cleanup_children(0);
}

void panel_menu_hide_cb(MBMenuItem *item)
{
  MBPanel *panel = (MBPanel *)item->cb_data;
  panel_toggle_visibilty(panel);
}

void panel_menu_kill_cb(MBMenuItem *item)
{
  MBPanelApp *papp = (MBPanelApp *)item->cb_data;

  XGrabServer(papp->panel->dpy);
  XKillClient(papp->panel->dpy, papp->win);
  session_save(papp->panel);
  XUngrabServer(papp->panel->dpy);
}

void
panel_menu_update_remove_items(MBPanel *panel)
{
  int *icon_data = NULL;
  MBMenuItem *menu_item;
  MBPanelApp *papp = NULL;
  MBPanelApp *papp_heads[] = { panel->apps_start_head, 
			       panel->apps_end_head,
			       NULL };
  int i = 0;

  if (panel->remove_menu == NULL)
    {
      char *icon_path = NULL;

      icon_path = mb_dot_desktop_icon_get_full_path (panel->theme_name, 
						     16, REMOVE_IMG  );

      panel->remove_menu = mb_menu_add_path(panel->mbmenu, _("Remove"), 
					    icon_path, 
					    MBMENU_PREPEND);
      if (icon_path) free(icon_path);
    } 

  /* Remove all items then readd so order matches panel */

  if (panel->remove_menu->items)
    {
      MBMenuItem *tmp_item = NULL;

      menu_item = panel->remove_menu->items;
   
      while (menu_item != NULL)
	{
	  tmp_item = menu_item->next_item;
	  mb_menu_item_remove(panel->mbmenu, panel->remove_menu, menu_item);
	  menu_item = tmp_item;
	}
    }


  while (i < 2)
    {
      papp = papp_heads[i];

      if (i == 1)
	papp = panel_app_list_get_last(panel, panel->apps_end_head);

      while( papp != NULL)
	{
	  if (!papp->ignore)
	    {
	      menu_item = mb_menu_add_item_to_menu(panel->mbmenu, 
						   panel->remove_menu, 
						   papp->name, NULL, 
						   papp->name, 
						   panel_menu_kill_cb, 
						   (void *)papp, 
						   MBMENU_NO_SORT);

	      if (!papp->icon)
		{
		  if ((icon_data = panel_app_icon_prop_data_get(panel, papp)) 
		      != NULL )
		    {
		      DBG("%s() Got icon data (size: %i x %i)\n", __func__,
			  icon_data[0], icon_data[1] );

		      papp->icon 
			= mb_pixbuf_img_new_from_int_data(panel->pb,
							  &icon_data[2],
							  icon_data[0], 
							  icon_data[1]);

		      XFree(icon_data);
		    }
		}
	      
	      if (papp->icon)
		mb_menu_item_icon_set(panel->mbmenu, menu_item, papp->icon);
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

}

void 
panel_menu_init(MBPanel *panel)
{
#ifndef MAXPATHLEN
#define MAXPATHLEN 1024
#endif

  MBMenuMenu *m, *menu_launchers;
  char orig_wd[MAXPATHLEN] = { 0 };
  struct dirent *dir_entry;
  char *icon_path = NULL;
  DIR *dp;

  if (panel->mbmenu == NULL)
    {
      panel->mbmenu = mb_menu_new(panel->dpy, panel->screen ); 
      mb_menu_set_icon_size(panel->mbmenu, 16);
    }
  else mb_menu_free(panel->mbmenu); /* XXX should be mb_menu_empty */

  icon_path = mb_dot_desktop_icon_get_full_path (panel->theme_name, 
						 16, ADD_IMG  );

  m = mb_menu_add_path(panel->mbmenu, _("Add"), icon_path, 
		       MBMENU_NO_SORT );

  if (icon_path) free(icon_path);
  
  icon_path = mb_dot_desktop_icon_get_full_path (panel->theme_name, 
						 16, FOLDER_IMG  );

  menu_launchers = mb_menu_add_path(panel->mbmenu, "Add/Launchers", 
				    icon_path, MBMENU_NO_SORT );

  if (icon_path) free(icon_path);

  if (getcwd(orig_wd, MAXPATHLEN) == (char *)NULL)
    {
      printf("Cant get current directory\n");
      exit(0);
    }

  if ((dp = opendir(DATADIR "/applications")) != NULL)
    {
      chdir(DATADIR "/applications");
      
      while((dir_entry = readdir(dp)) != NULL)
	{
	  struct stat stat_info;
	  if (strcmp(dir_entry->d_name+strlen(dir_entry->d_name)-8,".desktop"))
	    continue;
	  stat(dir_entry->d_name, &stat_info);
	  if (!(stat_info.st_mode & S_IFDIR))
	    {
	      MBDotDesktop *ddentry  = NULL;
	      ddentry = mb_dotdesktop_new_from_file(dir_entry->d_name);
	      if (ddentry
		  && mb_dotdesktop_get(ddentry, "Type")
		  && mb_dotdesktop_get(ddentry, "Name")
		  && mb_dotdesktop_get(ddentry, "Exec")
		  )
		{

		  char *png_path = NULL;
		  unsigned char *icon_str = mb_dotdesktop_get(ddentry, "Icon");
		  
		  png_path = mb_dot_desktop_icon_get_full_path (
								panel->theme_name, 
								16, icon_str  );

		  if (!strcmp(mb_dotdesktop_get(ddentry, "Type"), "PanelApp"))
		    {
		      mb_menu_add_item_to_menu(panel->mbmenu, 
					       m, 
					       mb_dotdesktop_get(ddentry, 
								 "Name"), 
					       png_path, 
					       mb_dotdesktop_get_exec(ddentry), 
					       panel_menu_exec_cb, 
					       (void *)panel, 0);
		    } else {
		      char launcher_exec_str[256] = { 0 };
		      snprintf(launcher_exec_str, 256, 
			       "mb-applet-launcher --desktop %s/%s",
			       DATADIR "/applications", dir_entry->d_name);
		      mb_menu_add_item_to_menu(panel->mbmenu, 
					       menu_launchers, 
					       mb_dotdesktop_get(ddentry, 
								 "Name"), 
					       png_path, 
					       launcher_exec_str,
					       panel_menu_exec_cb, 
					       (void *)panel, 0);

		    }
		  if (png_path) free(png_path);
		  mb_dotdesktop_free(ddentry);
		}
	    }
	}
      closedir(dp);
    }
  else fprintf(stderr, "failed to open %s\n", DATADIR "/applications");

  chdir(orig_wd);

  panel->remove_menu = NULL;

  icon_path = mb_dot_desktop_icon_get_full_path (panel->theme_name, 
						 16, HIDE_IMG  );

  mb_menu_add_item_to_menu(panel->mbmenu, panel->mbmenu->rootmenu, _("Hide"), 
			  icon_path, NULL , 
			  panel_menu_hide_cb, (void *)panel, MBMENU_NO_SORT); 


  if (panel->system_tray_id > 0)
    mb_menu_add_item_to_menu(panel->mbmenu, panel->mbmenu->rootmenu, 
			     "Exit", 
			     NULL, NULL , 
			     panel_menu_exit_cb, 
			     (void *)panel, MBMENU_NO_SORT); 


  if (icon_path) free(icon_path);

  return;
}

