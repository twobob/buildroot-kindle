#include "mbdesktop_item.h"
#include "mbdesktop_module.h"
#include "md5.h"

#define WARN(txt) fprintf(stderr, "%s:%i %s %s", __FILE__, __LINE__, __func__, txt )

static char*
md5sum(char *str)
{
  md5_state_t state;
  md5_byte_t digest[16];
  char *hex_output = (char *)malloc(sizeof(char)*((16*2) + 1));
  int di;

  md5_init(&state);
  md5_append(&state, (const md5_byte_t *)str, strlen(str));
  md5_finish(&state, digest);

  for (di = 0; di < 16; ++di)
    sprintf(hex_output + di * 2, "%02x", digest[di]);

  return hex_output;
}

MBDesktopItem *
mbdesktop_item_new()
{
  MBDesktopItem *ditem;
  ditem = malloc(sizeof(MBDesktopItem));
  memset(ditem, 0, sizeof(MBDesktopItem));
  
  ditem->type = ITEM_TYPE_UNKNOWN;

  return ditem;
}

void
mbdesktop_item_free(MBDesktop     *mb, 
		    MBDesktopItem *item)
{
  /* XXX callback to free item->data */

  if (item->name)      free(item->name);
  if (item->name_extended)      free(item->name_extended);
  if (item->icon_name) free(item->icon_name);
  if (item->icon)      mb_pixbuf_img_free(mb->pixbuf, item->icon);

  /* XXX check for children to free ? */

  free(item);

}

MBDesktopItem*
mbdesktop_item_folder_get_open(MBDesktop     *mb) 
{
  return mb->current_folder_item;
}

MBDesktopItem*
mbdesktop_item_folder_set_view(MBDesktop     *mb,
			       MBDesktopItem *folder,
			       int            view) 
{
  folder->view = view;
  return folder; 		/* XXX ? */
}

int
mbdesktop_item_folder_get_view(MBDesktop     *mb,
			       MBDesktopItem *folder)	
{
  return folder->view;  
}

Bool
mbdesktop_item_is_folder (MBDesktop *mb, MBDesktopItem *item)	
{
  return (item->type == ITEM_TYPE_FOLDER 
	  || item->type == ITEM_TYPE_ROOT
	  || item->type == ITEM_TYPE_PREVIOUS);
}


Bool
mbdesktop_item_folder_has_contents(MBDesktop     *mb, 
				   MBDesktopItem *folder)
{
  if (folder->item_child && folder->item_child->item_next_sibling)
    return True;
  return False;
}

void
mbdesktop_item_folder_contents_free(MBDesktop     *mb, 
				    MBDesktopItem *item)
{
  MBDesktopItem *item_tmp = NULL, *item_cur = NULL;

  if (item->item_child && item->item_child->item_next_sibling)
    {
      if (item == mb->top_head_item )
	{
	  item_cur = item->item_child;
	  mb->top_head_item->item_child = NULL;
	}
      else item_cur = item->item_child->item_next_sibling;

      while (item_cur != NULL)
	{
	  item_tmp = item_cur->item_next_sibling;

	  /* XXX free up any children - check this !! XX */
	  if (item_cur->item_child)
	    mbdesktop_item_folder_contents_free(mb, item_cur->item_child);

	  /* Stop possible segv of focus_item pointing to non-existant */
	  if (mb->kbd_focus_item == item_cur)
	    mb->kbd_focus_item = item->item_child;

	  mbdesktop_item_free(mb, item_cur); 
	  item_cur = item_tmp;
	}
      if ( item != mb->top_head_item)
	item->item_child->item_next_sibling = NULL;
    }
}

MBDesktopItem *
mbdesktop_item_new_with_params (MBDesktop     *mb,
			        const char    *name, 
			        const char    *icon_name,
			        void          *data,
			        int            type)
{
  MBDesktopItem *ditem;
  ditem = mbdesktop_item_new();

  if (name) ditem->name           = strdup(name);
  if (icon_name)
    {
      if (strlen(icon_name) > 5
	  && icon_name[strlen(icon_name)-4] != '.'
	  && icon_name[strlen(icon_name)-5] != '.')
	{
	  /* Tag .png onto the end of icon names with appear 
	     to have no extension.                            */
	  ditem->icon_name = malloc(sizeof(char)*(strlen(icon_name)+5));
	  sprintf(ditem->icon_name, "%s.png", icon_name); 
	}
      else
	{
	  ditem->icon_name = strdup(icon_name);
	}
    }

  mbdesktop_item_set_icon_from_theme(mb, ditem);

  if (data) ditem->data           = data;

  /*
  if (exec_str) ditem->exec_str   = strdup(exec_str);
  if (exec_cb) ditem->exec_cb     = exec_cb;
  if (populate_cb) ditem->populate_cb  = populate_cb;
  */

  if (type) ditem->type           = type;

  ditem->view = VIEW_ICONS;

  return ditem;
}

void
mbdesktop_item_set_icon_from_theme (MBDesktop     *mb,
				    MBDesktopItem *item)
{
#ifdef MB_HAVE_PNG
  char *default_icon_name = "mbnoapp.png";
#else
  char *default_icon_name = "mbnoapp.xpm";
#endif

  char *icon_path = NULL;
  icon_path = mb_dot_desktop_icon_get_full_path (mb->theme_name, 
						 mb->icon_size, 
						 item->icon_name);

  if (icon_path == NULL)
    icon_path = mb_dot_desktop_icon_get_full_path (mb->theme_name, 
						   mb->icon_size, 
						   default_icon_name);

  if (icon_path == NULL) 	/* Still NULL, something is wrong */
    {
      fprintf(stderr, "matchbox-desktop: could not load default icon. Is matchbox-common installed ?\n"); 
      exit(1); 
    }


  if (item->icon) mb_pixbuf_img_free(mb->pixbuf, item->icon);

  if ((item->icon = mb_pixbuf_img_new_from_file(mb->pixbuf, icon_path)) 
      == NULL)
    {
      fprintf(stderr, "matchbox-desktop: could not load %s for %s\n", 
	      icon_path, item->name);
      /* exit(1); */
    }

  if (icon_path) free(icon_path);

  return;
}

void
mbdesktop_items_append (MBDesktop     *mb,
			MBDesktopItem *item_head,
			MBDesktopItem *item )
{
  MBDesktopItem *item_tmp = NULL;

  item_tmp = item_head;

  while ( item_tmp->item_next_sibling != NULL )
    item_tmp = item_tmp->item_next_sibling;

  item_tmp->item_next_sibling = item;
  item->item_prev_sibling = item_tmp;

}

void
mbdesktop_items_insert_after (MBDesktop     *mb,
			      MBDesktopItem *suffix_item,
			      MBDesktopItem *item )
{
  if (!suffix_item->item_next_sibling)
    {
      mbdesktop_items_append (mb, suffix_item, item);
      return;
    }

  item->item_next_sibling = suffix_item->item_next_sibling;

  suffix_item->item_next_sibling->item_prev_sibling = item;

  suffix_item->item_next_sibling = item;

  item->item_prev_sibling = suffix_item;  
}

void
mbdesktop_items_append_to_folder (MBDesktop      *mb,
				  MBDesktopItem  *item_folder,
				  MBDesktopItem  *item )
{
  /* -- need to fix this warning --
  if (item_folder->type != ITEM_TYPE_FOLDER && item->type != ITEM_TYPE_ROOT)
    WARN("Passed folder item is not folder\n");
  */

  if (!item_folder->item_child) return;

  item->module_handle = item_folder->module_handle;

  mbdesktop_items_append (mb, item_folder->item_child, item);

}

void
mbdesktop_items_append_to_top_level (MBDesktop      *mb,
				     MBDesktopItem  *item )
{
  MBDesktopItem *top_level = mbdesktop_get_top_level_folder(mb);

  /* item->module_handle = item_folder->module_handle; ???  */ 

  item->item_parent = top_level;

  if (top_level->item_child == NULL)
    top_level->item_child = item;
  else
    mbdesktop_items_append (mb, top_level->item_child, item); 
}

void
mbdesktop_items_prepend (MBDesktop      *mb,
			 MBDesktopItem **item_head,
			 MBDesktopItem  *item )
{
  MBDesktopItem *item_tmp = NULL;

  /*
  if ((*item_head)->type == ITEM_TYPE_PREVIOUS)
    item_tmp = (*item_head)->item_next_sibling;
  else
  */

  item_tmp = *item_head;
  item->item_next_sibling = item_tmp;
  item_tmp->item_prev_sibling = item;

  *item_head = item;


}

MBDesktopItem *
mbdesktop_item_get_next_sibling(MBDesktopItem *item)
{
  return item->item_next_sibling;
}

MBDesktopItem *
mbdesktop_item_get_prev_sibling(MBDesktopItem *item)
{
  return item->item_prev_sibling;
}

MBDesktopItem *
mbdesktop_item_get_parent(MBDesktopItem *item)
{
  MBDesktopItem *result = mbdesktop_item_get_first_sibling(item);

  if (result && result->item_parent)
    return result->item_parent;

  return NULL;
}

MBDesktopItem *
mbdesktop_item_get_child(MBDesktopItem *item)
{
  /* XXX Need to fix warnings
  if (item->type != ITEM_TYPE_FOLDER && item->type != ITEM_TYPE_ROOT)
    WARN("Passed folder item is not folder\n");
  */

  return item->item_child;
}


MBDesktopItem *
mbdesktop_item_get_first_sibling(MBDesktopItem *item)
{
  while (item->item_prev_sibling != NULL )
    item = item->item_prev_sibling;
  return item;
}

MBDesktopItem *
mbdesktop_item_get_last_sibling(MBDesktopItem *item)
{
  while (item->item_next_sibling != NULL )
    item = item->item_next_sibling;
  return item;
}

void
mbdesktop_item_set_icon_data (MBDesktop     *mb, 
			      MBDesktopItem *item, 
			      MBPixbufImage *img)
{
  if (item->icon) mb_pixbuf_img_free(mb->pixbuf, item->icon);
  item->icon = mb_pixbuf_img_clone(mb->pixbuf, img);
}

void
mbdesktop_item_set_name (MBDesktop     *mb, 
			 MBDesktopItem *item, 
			 char          *name)
{
  if (item->name) free(item->name);
  item->name = strdup(name);
}

char *
mbdesktop_item_get_name (MBDesktop     *mb, 
			 MBDesktopItem *item)
{
  return item->name;
}

void
mbdesktop_item_set_comment (MBDesktop     *mb, 
			    MBDesktopItem *item, 
			    char          *comment)
{
  if (item->comment) free(item->comment);
  item->comment = strdup(comment);
}

char *
mbdesktop_item_get_comment (MBDesktop     *mb, 
			    MBDesktopItem *item)
{
  return item->comment;
}


void
mbdesktop_item_set_extended_name (MBDesktop     *mb, 
				       MBDesktopItem *item, 
				       char          *name)
{
  if (item->name_extended) free(item->name_extended);
  item->name_extended = strdup(name);
}

char *
mbdesktop_item_get_extended_name (MBDesktop     *mb, 
				  MBDesktopItem *item)
{
  return item->name_extended;
}

void
mbdesktop_item_set_user_data (MBDesktop     *mb, 
			      MBDesktopItem *item, 
			      void          *data)
{
  if (item->data) free(item->data); /* XXX callback ? */
  item->data = data;
}

void *
mbdesktop_item_get_user_data (MBDesktop     *mb, 
			      MBDesktopItem *item)
{
  return item->data;
}


void
mbdesktop_item_set_image (MBDesktop     *mb, 
			  MBDesktopItem *item, 
			  char          *full_img_path)
{
  if (item->icon) mb_pixbuf_img_free(mb->pixbuf, item->icon);
  item->icon = mb_pixbuf_img_new_from_file(mb->pixbuf, full_img_path);

  if (item->icon == NULL)
    fprintf(stderr, "matchbox-desktop: *warning* failed to load '%s' \n",
	    full_img_path);
}

void
mbdesktop_item_set_activate_callback (MBDesktop     *mb, 
				      MBDesktopItem *item, 
				      MBDesktopCB    activate_cb)
{
  item->activate_cb = activate_cb;
}

void
mbdesktop_item_set_type (MBDesktop     *mb, 
			 MBDesktopItem *item,
			 int            type)
{
  item->subtype = type;
}

int
mbdesktop_item_get_type (MBDesktop     *mb, 
			 MBDesktopItem *item)
{
  return item->subtype;
}

void
mbdesktop_item_cache (MBDesktop     *mb, 
		      MBDesktopItem *item,
		      char          *ident )
{
  struct stat st;
  gzFile cachefile;
  char path[1024];
  char new_ident[1024];
  char *md5 = NULL;
  int name_len = 0, name_e_len =0, comment_len = 0;

  snprintf(new_ident, 1024, "%s-%i", ident, mb->icon_size);
  
  md5 = md5sum(new_ident);

  snprintf(path, 1024, "%s/.thumbnails", mb_util_get_homedir());
  
  /* Check if ~/.matchbox exists and create if not */
  if (stat(path, &st) != 0)
      mkdir(path, 0755);

  snprintf(path, 1024, "%s/.thumbnails/%s", mb_util_get_homedir(), md5);
  
  if ((cachefile = gzopen (path, "wb")) == NULL )
    {
      fprintf(stderr, "%s() failed to open : %s", __func__, path);
      free(md5);
      return;
    }

  name_len = item->name != NULL ? strlen(item->name) : 0;
  name_e_len = item->name_extended != NULL ? strlen(item->name_extended) : 0;
  comment_len = item->comment != NULL ? strlen(item->comment) : 0;

  gzwrite (cachefile, (void *)&name_len, sizeof(int));
  gzwrite (cachefile, (void *)&name_e_len, sizeof(int));
  gzwrite (cachefile, (void *)&comment_len, sizeof(int));

  if (item->name)
    gzwrite (cachefile, (void *)item->name, 
	     strlen(item->name));

  if (item->name_extended)
    gzwrite (cachefile, (void *)item->name_extended, 
	     strlen(item->name_extended));

  if (item->comment)
    gzwrite (cachefile, (void *)item->comment, 
	     strlen(item->comment));

  gzwrite (cachefile, (void *)&item->icon->width, sizeof(int)); 
  gzwrite (cachefile, (void *)&item->icon->height, sizeof(int)); 
  gzwrite (cachefile, (void *)&item->icon->has_alpha, sizeof(int)); 
  gzwrite (cachefile, (void *)item->icon->rgba, 
	   item->icon->width * item->icon->height * ( 3 + item->icon->has_alpha));
  gzclose (cachefile);
  free(md5);

}

MBDesktopItem *
mbdesktop_item_from_cache (MBDesktop *mb, 
			   char      *ident,
			   time_t     age)
{
  MBDesktopItem *item;
  char path[1024];
  char new_ident[1024];
  char *md5 = NULL;
  char buf[1024]; 	    
  struct stat stat_info;
  gzFile cache_file;
  int name_len = 0, name_e_len =0, comment_len = 0;
  int img_w, img_h, has_alpha;
  unsigned char *img_data = NULL, *tmp_ptr = NULL;

  snprintf(new_ident, 1024, "%s-%i", ident, mb->icon_size);

  md5 = md5sum(new_ident);

  snprintf(path, 1024, "%s/.thumbnails/%s", mb_util_get_homedir(), md5);

  if (stat(path, &stat_info) == -1) 
    return NULL;

  if (age > stat_info.st_mtime)
    return NULL;

  cache_file = gzopen (path, "rb");

  gzread (cache_file, (void *)&name_len, sizeof(int));
  gzread (cache_file, (void *)&name_e_len, sizeof(int));
  gzread (cache_file, (void *)&comment_len, sizeof(int));

  item = mbdesktop_item_new();

  item->type = ITEM_TYPE_MODULE_ITEM;

  if (name_len)
    {
      gzread (cache_file, (void *)buf, name_len);
      buf[name_len] = '\0';
      item->name = strdup(buf);
      printf("cache gives %s\n", item->name);
    }

  if (name_e_len)
    {
      gzread (cache_file, (void *)buf, name_e_len);
      buf[name_e_len] = '\0';
      item->name_extended = strdup(buf);
      printf("cache gives %s\n", item->name_extended);
    }

  if (comment_len)
    {
      gzread (cache_file, (void *)buf, comment_len);
      buf[comment_len] = '\0';
      item->comment = strdup(buf);
      printf("cache gives %s\n", item->comment);
    }
  
  gzread (cache_file, (void *)&img_w, sizeof(int));
  gzread (cache_file, (void *)&img_h, sizeof(int));
  gzread (cache_file, (void *)&has_alpha, sizeof(int));

  printf("cache gives img %ix%i has alpha: %i expecting %i bytes\n", img_w, img_h, has_alpha, img_w * img_h * ( 3 + has_alpha));

  img_data = malloc(sizeof(unsigned char) * img_w * img_h * ( 3 + has_alpha));

  tmp_ptr = img_data;

  printf("gzread gives %i\n", gzread (cache_file, (void *)img_data, img_w * img_h * ( 3 + has_alpha)));

  item->icon = mb_pixbuf_img_new_from_data(mb->pixbuf, 
					   tmp_ptr, img_w, img_h,
					   has_alpha);
  free(tmp_ptr);
  gzclose (cache_file);
  free(md5);

  return item;
}



/* Should these live here ?? */

void
mbdesktop_item_folder_activate_cb(void *data1, void *data2)
{
  MBDesktop *mb = (MBDesktop *)data1; 
  MBDesktopItem *item = (MBDesktopItem *)data2; 

  /* Save scroll position if relevant */
  if (mb->scroll_offset_item)
    item->saved_scroll_offset_item = mb->scroll_offset_item; 

  mb->scroll_offset_item = mb->kbd_focus_item 
    = mb->current_head_item = item->item_child;

  mb->current_folder_item = item; 

  if (mb->kbd_focus_item->item_next_sibling)
    mb->kbd_focus_item = mb->kbd_focus_item->item_next_sibling;

  mbdesktop_view_paint(mb, False);
}

void
mbdesktop_item_folder_prev_activate_cb(void *data1, void *data2)
{
  MBDesktop *mb = (MBDesktop *)data1; 
  MBDesktopItem *item = (MBDesktopItem *)data2; 

  if (item && item->type == ITEM_TYPE_PREVIOUS && item->item_parent)
    {
      mb->kbd_focus_item = item->item_parent;

      mb->scroll_offset_item  = mb->current_head_item 
	= mbdesktop_item_get_first_sibling(item->item_parent);

      /* Get saved scroll position if exists */
      if (item->item_parent->saved_scroll_offset_item)
	mb->scroll_offset_item = item->item_parent->saved_scroll_offset_item;

      item->item_parent->saved_scroll_offset_item = NULL;
      
      mb->current_folder_item = item->item_parent->item_parent; 
      
      mbdesktop_view_paint(mb, False);
    }
}

MBDesktopItem *
mbdesktop_item_get_from_coords(MBDesktop *mb, int x, int y)
{
  MBDesktopItem *item;
  for(item = mb->scroll_offset_item; 
      (item != NULL && item != mb->last_visible_item); 
      item = item->item_next_sibling)
    {
      if (x > item->x && x < (item->x + item->width)
	  && y > item->y && y < (item->y + item->height))
	{
	  return item;
	} 
    }
  return NULL;
}



