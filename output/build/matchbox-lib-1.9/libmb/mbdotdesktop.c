/* mbpixbuf.c libmb
 *
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

#include "mbdotdesktop.h"
#include "hash.h"

enum {
  DD_SUCCESS,
  DD_ERROR_FILE_OPEN_FAILED,
  DD_ERROR_NOT_DESKTOP_FILE
};

struct MBDotDesktop
{
  char *filename;
  char *lang;
  char *lang_country;
  struct hash *h;
}; 

static int
_parse_desktop_entry(MBDotDesktop *dd);

static char *
_mystrndup(const char *src, size_t n);

static int
_parse_desktop_entry(MBDotDesktop *dd)
{
  FILE *fp;
  char data[256];
  char *key = NULL, *val = NULL, *str = NULL;
  
  if (!(fp = fopen(dd->filename, "r"))) return DD_ERROR_FILE_OPEN_FAILED;

  if (fgets(data,256,fp) != NULL)
    {
      /* FIXME: this check should be moved elsewhere if exist at all */
      if (strncasecmp("[desktop entry]", data, 15))
	{
	  if (mb_want_warnings())
	    fprintf(stderr, "libmb: dont look like a desktop entry? %s\n", 
		    data);
	  fclose(fp);
	  return DD_ERROR_NOT_DESKTOP_FILE;
	}
    } else { fclose(fp); return DD_ERROR_NOT_DESKTOP_FILE; }

  while(fgets(data,256,fp) != NULL)
    {
      if (data[0] == '#' || data[0] == '[') 
	continue;
      str = strdup(data);
      if ( (val = strchr(str, '=')) != NULL)
	{
	  *val++ = '\0'; key = str;
	  if (*val != '\0')
	    {
	      char new_key[65], locale[17];
 
	      /* Trim ... 	      */
	      while (isspace(*key)) key++;
	      while (isspace(key[strlen(key)-1])) key[strlen(key)-1] = '\0'; 
	      while (isspace(*val)) val++;
	      while (isspace(val[strlen(val)-1])) val[strlen(val)-1] = '\0'; 


	      /* Handle [l10n] .. */
	      if (sscanf (key, "%64[^[][%16[^][]]", new_key, locale) == 2)
		{
		  /* Ignore C or no locale*/
		  if (dd->lang == NULL && dd->lang_country == NULL) goto END;
 
		  /* foo[lang_country] match */
		  if (dd->lang_country && !strcmp(dd->lang_country, locale)) 
		    key = new_key;
		  /* foo[lang] match */
		  else if (dd->lang && !strcmp(dd->lang, locale)) 
		    key = new_key;
		  else goto END;
		}
	      if (val[strlen(val)-1] == '\n') val[strlen(val)-1] = '\0';

	      if (*val != '\0')
		hash_add(dd->h, key, val);
	    }
	}
    END:
      free(str);
    }
  
  fclose(fp);
  
  return DD_SUCCESS;
}

static char *
_mystrndup(const char *src, size_t n) /* For NetBSD mainly */
{
  char *result = malloc(n+1);
  strncpy(result, src, n);
  result[n] = '\0';
  return result;
}

MBDotDesktop *
mb_dotdesktop_new_from_file(const char *filename)
{
  MBDotDesktop *dd;
  char *locale;

  dd = malloc(sizeof(MBDotDesktop));
  dd->filename = strdup(filename);

  locale = setlocale (LC_MESSAGES, "");
  
  if ((locale == NULL) || (locale != NULL && !strcmp(locale, "C")))
    { 
      dd->lang = NULL; 
      dd->lang_country = NULL;
    }
  else
    {
      const char *uscore_pos;
      const char *at_pos;
      const char *dot_pos;
      const char *end_pos;
      const char *start_lang;
      const char *end_lang;
      const char *end_country;
      
      /* lang_country.encoding@modifier */
      
      uscore_pos = strchr (locale, '_');
      dot_pos = strchr (uscore_pos ? uscore_pos : locale, '.');
      at_pos = strchr (dot_pos ? dot_pos : 
		       (uscore_pos ? uscore_pos : locale), '@');
      end_pos = locale + strlen (locale);
      
      start_lang = locale;
      end_lang = (uscore_pos ? uscore_pos :
		  (dot_pos ? dot_pos :
		   (at_pos ? at_pos : end_pos)));
      
      end_country = (dot_pos ? dot_pos :
		     (at_pos ? at_pos : end_pos));
      
      if (uscore_pos == NULL) 
	{
	  dd->lang = _mystrndup (start_lang, end_lang - start_lang);
	  dd->lang_country = NULL;
	} else {
	  dd->lang = _mystrndup (start_lang, end_lang - start_lang);
	  dd->lang_country = _mystrndup (start_lang,
					 end_country - start_lang);
	}
    }

  dd->h = hash_new(81);

  if (_parse_desktop_entry(dd) == DD_SUCCESS)
    return dd;
  else
    {
      mb_dotdesktop_free(dd);
      return NULL;
    }
}

char *
mb_dotdesktop_get_filename(MBDotDesktop *dd)
{
  if (dd)
    return dd->filename;
  else
    return NULL;
}

char *
mb_dotdesktop_get_exec (MBDotDesktop *dd)
{
  /* Source string, destination string */
  char *source, *dest;
  /* Source iterator, destination iterator */
  char *s, *d;
  
  s = source = (char*)mb_dotdesktop_get (dd, "Exec");
  if (source == NULL)
    return NULL;
  
  d = dest = malloc (strlen(source) + 1);
  while (*s) {
    if (*s == '%') {
      if (*++s == '%') {
        *d++ = '%';
      } else {
        /* Otherwise ignore. TODO: should handle some of the escapes */
        s++;
      }
    } else {
      *d++ = *s++;
    }
  }
  *d = '\0';
  return dest;
}

unsigned char *
mb_dotdesktop_get(MBDotDesktop *dd, char *field)
{
  struct nlist *n;
  n = hash_lookup(dd->h, field);
  if (n)
    return (unsigned char *)n->value;
  else
    return NULL;
}

void
mb_dotdesktop_free(MBDotDesktop *dd)
{
  free(dd->filename);
  free(dd->lang);
  free(dd->lang_country);

  hash_destroy(dd->h);

  free(dd);
}

/* Mini Icon theme spec implementation */

static int 
_file_exists(char *filename)
{
  struct stat st;
  if (stat(filename, &st)) return 0;
  return 1;
}


char*
mb_dot_desktop_icon_get_full_path (char* theme_name, 
				   int   size_wanted, 
				   char* icon_name)
{
  int i = 0;
  char *path = malloc(sizeof(char)*512);

  char *theme_name_cur = alloca(sizeof(char)*512);
  char tmp_path[512] = { 0 };
  int sizes[]  = { 0, 48, 36, 32, 24, 16, 0 }; 
  int size_index = 0;

  char *icon_dirs[2] = { NULL, NULL };

  icon_dirs[0] = alloca(sizeof(char) * (strlen(mb_util_get_homedir()) + 8));
  sprintf(icon_dirs[0], "%s/.icons", mb_util_get_homedir());
 
  icon_dirs[1] = alloca(sizeof(char) * (strlen(DATADIR) + 8));
  sprintf(icon_dirs[1], DATADIR "/icons");
  
  snprintf(path, 512, "%s/%s", icon_dirs[0], icon_name);
  if (_file_exists(path)) return path;
  
  if (theme_name) 
    strncpy(theme_name_cur, theme_name, 512);  
  else
    theme_name_cur = NULL;

  while (theme_name_cur != NULL)
    {
      i = 0;
      while (i < 2)
	{
	  snprintf(path, 512, "%s/%s",icon_dirs[i], theme_name) ;
	  if (_file_exists(path))
	    {
	      MBDotDesktop *dd = NULL;
	      char dd_filename[512] = { 0 };
	      size_index = 0;

	      if (size_wanted)
		sizes[0] = size_wanted;
	      else 
		size_index++;
	      
	      snprintf(dd_filename, 512, "%s/index.theme", path);
	  
	      while ( sizes[size_index] )
		{
		  snprintf(tmp_path, 512, "%s/%s/%ix%i/", 
			   icon_dirs[i],
			   theme_name_cur, 
			   sizes[size_index], 
			   sizes[size_index]);

		  if (_file_exists(tmp_path)) 
		    {
		      DIR *dp;
		      struct dirent *dir_entry;
		      struct stat stat_info;
		  
		      if ((dp = opendir(tmp_path)) != NULL)
			{
			  while((dir_entry = readdir(dp)) != NULL)
			    {
			      lstat(dir_entry->d_name, &stat_info);
			      if (S_ISDIR(stat_info.st_mode)
				  && strcmp(dir_entry->d_name, ".") != 0
				  && strcmp(dir_entry->d_name, "..") != 0)
				{
				  snprintf(path, 512, "%s/%s/%s", 
					   tmp_path, dir_entry->d_name, icon_name);
				  if (_file_exists(path)) 
				    {
				      closedir(dp);
				      return path;
				    }
				}
			    }
			  closedir(dp);
			}
		    }
		  size_index++;
		}
	      
	      /* XXX not found, not check for inherits key */
	      if ((dd = mb_dotdesktop_new_from_file(dd_filename)) != NULL)
		{
		  if (mb_dotdesktop_get(dd, "Inherits"))
		    {
		      theme_name_cur = NULL;
		      strncpy(theme_name_cur, 
			      (char*)mb_dotdesktop_get(dd, "Inherits"), 512);
		      i = 2;
		    }
		  mb_dotdesktop_free(dd);
		} 
	      else if ( i == 1 ) theme_name_cur = NULL;
	    }
	  else if ( i == 1 ) theme_name_cur = NULL;

	  i++;
	}
    }
      
  snprintf(path, 512, DATADIR "/pixmaps/%s", icon_name);
  if (_file_exists(path)) return path;

  if (strcmp(DATADIR, "/usr/share")) /* XXX potentially bad */
    {
      snprintf(path, 512, "/usr/share/pixmaps/%s", icon_name);
      if (_file_exists(path)) return path;
    }

  /* Still nothing - have we been passed a full path ! */
  if (_file_exists(icon_name))
    {
      snprintf(path, 512, "%s", icon_name);
      return path;
    }

  free(path);
  return NULL;
}

/* micro Vfolders */

MBDotDesktopFolders*
mb_dot_desktop_folders_new(const char *vfolder_path)
{
  MBDotDesktopFolders* folders;

  FILE *fp;
  //unsigned char *c;
  char data[512];
  char order_path[256];
  int folder_cnt = 0;
  MBDotDesktop *dd;
  MBDotDesktopFolderEntry* entry_cur = NULL;

  snprintf(order_path, 256, "%s/Root.order", vfolder_path);

  if (!(fp = fopen(order_path, "r"))) 
    { 
      if (mb_want_warnings())
	fprintf(stderr, "libmb: failed to open %s\n", order_path); 
      return NULL;
    }

  while(fgets(data,512,fp) != NULL) 
    if (data[0] != '#' && !isspace(data[0]))
      folder_cnt++;

  /* XXX TO FIX
  while((c = (char *)fgetc(fp)) != EOF)
    if ( *c == '\n' ) 
      folder_cnt++;
  */

  if (!folder_cnt) 
    { 
      if (mb_want_warnings())
	fprintf(stderr, "libmb: no vfolders defined\n"); 
       fclose(fp); 
       return NULL; 
    }

  folders = malloc(sizeof(MBDotDesktopFolders));
  folders->entries = NULL;
  folders->n_entries = 0;

  rewind(fp);

  while (fgets(data,256,fp) != NULL)
    {
      char tmp_path[512] = { 0 };

      if (data[strlen(data)-1] == '\n') data[strlen(data)-1] = '\0';
      snprintf(tmp_path, 512, "%s/%s.directory", vfolder_path, data);

      if ((dd = mb_dotdesktop_new_from_file(tmp_path)) != NULL)
	{
	  if (mb_dotdesktop_get(dd, "Name") && mb_dotdesktop_get(dd, "Match"))
	    {
	      if (entry_cur == NULL)
		{
		  folders->entries = malloc(sizeof(MBDotDesktopFolderEntry));
		  entry_cur = folders->entries;
		}
	      else
		{
		  entry_cur->next_entry = malloc(sizeof(MBDotDesktopFolderEntry));
		  entry_cur = entry_cur->next_entry;
		}
	      memset(entry_cur, 0, sizeof(MBDotDesktopFolderEntry));
	  
	      entry_cur->name = (unsigned char*)strdup((char*)mb_dotdesktop_get(dd, "Name"));
	      entry_cur->match = (unsigned char*)strdup((char*)mb_dotdesktop_get(dd, "Match"));

	      if (mb_dotdesktop_get(dd, "Icon"))
		{
		  entry_cur->icon 
		    = (unsigned char*)strdup((char*)mb_dotdesktop_get(dd, "Icon"));
		}

	      folders->n_entries++;
	    }
	  mb_dotdesktop_free(dd);
	}
    }

  fclose(fp);

  return folders;
}


void
mb_dot_desktop_folders_free(MBDotDesktopFolders* folders)
{
  MBDotDesktopFolderEntry *cur = NULL, *next = NULL;

  next = cur = folders->entries;

  while (next != NULL)
    {
      next = cur->next_entry;
      if (cur->name) free(cur->name);
      if (cur->match) free(cur->match);
      if (cur->icon) free(cur->icon);
      free(cur);
      cur = next;
    }
  free(folders);
}
