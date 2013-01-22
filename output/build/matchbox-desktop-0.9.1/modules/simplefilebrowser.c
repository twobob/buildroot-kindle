#include "mbdesktop_module.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <regex.h>

#include <libmb/mb.h>

typedef struct BrowserData { 
  char* BrowserCurrentPath;
  char* BrowserPath;
  char* BrowserMatchStr;
  char* BrowserIcon;
  char* BrowserExecWith;
  char* BrowserFolderName;
  char* BrowserFolderIcon;
} BrowserData;

void
filebrowser_open_cb (void *data1, void *data2);

static void
filebrowser_file_activate_cb (void *data1, void *data2)
{
  BrowserData *mod_data;
  MBDesktop *mb = (MBDesktop *)data1; 
  MBDesktopItem *item = (MBDesktopItem *)data2; 
  char *exec_str     = NULL;
  char *current_path = NULL;

  mod_data = (BrowserData*)mbdesktop_item_get_user_data (mb, item);

  /* Items contain full path to the file  */
  current_path = mbdesktop_item_get_extended_name(mb, item);

  exec_str = alloca(strlen(mod_data->BrowserExecWith)+strlen(current_path)+strlen(item->name)+6);

  sprintf(exec_str, "%s '%s/%s'", 
	  mod_data->BrowserExecWith, 
	  current_path,
	  item->name);

  switch (fork())
    {
    case 0:
      fprintf(stderr, "mbdesktop: attempting to exec '%s'\n", exec_str);
      mb_exec(exec_str);
      fprintf(stderr, "exec failed, cleaning up child\n");
      exit(1);
    case -1:
      fprintf(stderr, "can't fork\n");
      break;
    }

  if (current_path) free(current_path);
}

int
filebrowser_init (MBDesktop             *mb, 
		  MBDesktopFolderModule *folder_module, 
		  char                  *arg_str)
{
  DIR           *dp;
  struct dirent *dir_entry;
  struct stat    stat_info;
  MBDotDesktop  *dd;
  BrowserData   *data = NULL;

  /* XXX args can be location of user def config folder */
  if (arg_str == NULL)
    arg_str = PKGDATADIR "/mbdesktop_filebrowser";

  if ((dp = opendir(arg_str)) == NULL)
    {
      fprintf(stderr, "simplefilebrowser: failed to open %s\n", arg_str);
      return -1;
    }

  while((dir_entry = readdir(dp)) != NULL)
    {
      char buf[512];

      if (strcmp(dir_entry->d_name+strlen(dir_entry->d_name)-8, ".desktop"))
	continue;

      snprintf(buf, 512, "%s/%s", arg_str, dir_entry->d_name);

      lstat(buf, &stat_info);
      if (!(S_ISDIR(stat_info.st_mode)))
	{
	  dd = mb_dotdesktop_new_from_file(buf);
	  if (dd)
	    {
	      MBDesktopItem *folder = NULL;
	      data = malloc(sizeof(BrowserData));

	      /* Defualts */
	      data->BrowserPath        = "/";
	      data->BrowserMatchStr    = "*"; 
	      data->BrowserIcon        = "mbnoapp.png"; 
	      data->BrowserExecWith    = "cat"; 
	      data->BrowserFolderName  = "files"; 
	      data->BrowserFolderIcon  = "mbfolder.png"; 

	      if (mb_dotdesktop_get(dd, "Path"))
		data->BrowserPath = strdup(mb_dotdesktop_get(dd, "Path"));

	      if (mb_dotdesktop_get(dd, "Match"))
		data->BrowserMatchStr = strdup(mb_dotdesktop_get(dd, "Match")); 
	      if (mb_dotdesktop_get(dd, "FileIcon"))
		data->BrowserIcon = strdup(mb_dotdesktop_get(dd, "FileIcon")); 

	      if (mb_dotdesktop_get(dd, "ExecWith"))
		data->BrowserExecWith = strdup(mb_dotdesktop_get(dd, "ExecWith"));
	      if (mb_dotdesktop_get(dd, "FolderName"))
		data->BrowserFolderName = strdup(mb_dotdesktop_get(dd, "FolderName"));
	      if (mb_dotdesktop_get(dd, "FolderIcon")) 
		data->BrowserFolderIcon = strdup(mb_dotdesktop_get(dd, "FolderIcon"));

	      folder = mbdesktop_module_folder_create (mb,
						       data->BrowserFolderName, 
						       data->BrowserFolderIcon);
	      mbdesktop_item_set_user_data (mb, folder, (void *)data);

	      mbdesktop_item_set_extended_name(mb, folder, data->BrowserFolderName);
	      
	      mbdesktop_items_append_to_top_level (mb, folder);
	      
	      mbdesktop_item_folder_set_view (mb, folder, VIEW_LIST);
	      
	      mbdesktop_item_set_activate_callback (mb, folder, filebrowser_open_cb);
	      mb_dotdesktop_free(dd);
	    }
	}
    }

  closedir(dp);

  return 1;
}

void
filebrowser_open_cb (void *data1, void *data2)
{
  DIR *dp;
  BrowserData   *mod_data;
  struct dirent **namelist;
  char orig_wd[512] = { 0 };
  regex_t re;
  int n, i = 0;
  char *current_path = NULL, *current_path_stripped = NULL;

  MBDesktopItem *subfolder = NULL;
  MBDesktopItem *item_new  = NULL; 

  MBDesktop     *mb          = (MBDesktop *)data1; 
  MBDesktopItem *item_folder = (MBDesktopItem *)data2; 

  Bool is_subfolder = False;

  mod_data = (BrowserData*)mbdesktop_item_get_user_data (mb, item_folder);

  if (!strcmp(mbdesktop_item_get_name(mb, item_folder), 
	      mod_data->BrowserFolderName))
    {				/* Is top level */
      current_path = strdup(mod_data->BrowserPath);
      current_path_stripped = strdup("");
    } 
  else
    {				/* Is sub folder  */
      /* Figure out the 'real' directory path from name etc */

      char *base_path = mbdesktop_item_get_extended_name(mb, item_folder) 
	+ (strlen(mod_data->BrowserFolderName) +1) ;

      int len = strlen(mod_data->BrowserPath) 
	+ strlen(mbdesktop_item_get_extended_name(mb, item_folder))
	+ strlen(mod_data->BrowserFolderName);
     
      current_path = malloc(sizeof(char)*len);
      current_path_stripped = malloc(sizeof(char)*(strlen(base_path)+3));

      snprintf(current_path, len, "%s/%s", mod_data->BrowserPath, base_path);
      sprintf(current_path_stripped, "%s/", base_path);
      
      is_subfolder = True;
    }

  if (mbdesktop_item_folder_has_contents(mb, item_folder))
      mbdesktop_item_folder_contents_free(mb, item_folder); 

  if (regcomp(&re, mod_data->BrowserMatchStr, 
	      REG_EXTENDED|REG_ICASE|REG_NOSUB) != 0) 
    {
      fprintf(stderr, "mbdesktop-filebrowser: failed to compile re: %s\n", 
	      mod_data->BrowserMatchStr);
      return;
    }

  if ((dp = opendir(current_path)) == NULL)
    {
      fprintf(stderr, "mbdesktop-filebrowser: failed to open %s\n", 
	      mod_data->BrowserPath);
      return;
    }
  
  if (getcwd(orig_wd, 255) == (char *)NULL)
    {
      fprintf(stderr, "mbdesktop-filebrowser: cant get current directory\n");
      return;
    }

  chdir(current_path);

  n = scandir(".", &namelist, 0, alphasort);
  while (i < n && n > 0)
    {
      struct stat stat_buf;

      if (!strcmp(namelist[i]->d_name, "..")
	  || !strcmp(namelist[i]->d_name, "."))
	goto end;

      /* Check for directory */
      if (stat(namelist[i]->d_name, &stat_buf) == 0 
	  && S_ISDIR(stat_buf.st_mode))
	{
	  char *subfolderlongname = NULL;


	  int l = strlen(mod_data->BrowserFolderName) 
	    + strlen(current_path) + strlen(namelist[i]->d_name);
	  
	  subfolderlongname = malloc(sizeof(char)*l);

	  sprintf(subfolderlongname,  "%s/%s%s", 
		  mod_data->BrowserFolderName, 
		  current_path_stripped, namelist[i]->d_name);

	  subfolder = mbdesktop_module_folder_create (mb,
						      namelist[i]->d_name,
						      mod_data->BrowserFolderIcon);
	  mbdesktop_item_set_extended_name (mb, subfolder, subfolderlongname);
	  mbdesktop_item_set_user_data (mb, subfolder, (void *)mod_data);

	  mbdesktop_items_append_to_folder (mb, item_folder, subfolder);
	  mbdesktop_item_folder_set_view (mb, subfolder, VIEW_LIST);
	  mbdesktop_item_set_activate_callback (mb, subfolder, 
						filebrowser_open_cb);

	  free(subfolderlongname);
	  goto end;
	}

      if (regexec(&re, namelist[i]->d_name, 0, NULL, REG_NOTBOL) == 0)
	{
	  item_new = mbdesktop_item_new_with_params( mb,
						     namelist[i]->d_name,
							 mod_data->BrowserIcon,
						     NULL,
						     ITEM_TYPE_MODULE_ITEM
						     );
	  
	  mbdesktop_item_set_user_data (mb, item_new, (void *)mod_data);

	  mbdesktop_item_set_extended_name (mb, item_new, current_path);
	      
	  mbdesktop_item_set_activate_callback (mb, item_new, 
						filebrowser_file_activate_cb); 
	  
	  mbdesktop_items_append_to_folder( mb, item_folder, item_new );
	}


    end:
      free(namelist[i]);
      ++i;
    }

  regfree(&re);

  free(namelist);

  closedir(dp);

  chdir(orig_wd);

  free(current_path);


  mbdesktop_item_folder_activate_cb(data1, data2);
}


#define MODULE_NAME         "simple file system browser"
#define MODULE_DESC         "simple file system browser"
#define MODULE_AUTHOR       "Matthew Allum"
#define MODULE_MAJOR_VER    0
#define MODULE_MINOR_VER    0
#define MODULE_MICRO_VER    2
#define MODULE_API_VERSION  0
 
MBDesktopModuleInfo filebrowser_info = 
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
    &filebrowser_info,
    filebrowser_init,
    NULL,
    NULL
  };
