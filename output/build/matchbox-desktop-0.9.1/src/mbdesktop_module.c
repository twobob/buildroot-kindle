#include "mbdesktop_module.h"

MBDesktopItem *
mbdesktop_get_top_level_folder(MBDesktop     *mb)
{
  return mb->top_head_item;
}

MBDesktopItem*
mbdesktop_module_folder_create ( MBDesktop *mb,
				 char      *name,
				 char      *icon_name )
{
  MBDesktopItem* item_folder = NULL;

#ifdef MB_HAVE_PNG
  char *folder_prev_name = "mbfolderprev.png";
#else
  char *folder_prev_name = "mbfolderprev.xpm";
#endif

  item_folder
    = mbdesktop_item_new_with_params( mb, 
				      name,
				      icon_name,
				      NULL,
				      ITEM_TYPE_FOLDER
				      );

  mbdesktop_item_set_activate_callback (mb, item_folder, 
					mbdesktop_item_folder_activate_cb); 

  item_folder->item_child 
    = mbdesktop_item_new_with_params(mb,
				     "Back", 
				     folder_prev_name,
				     NULL,
				     ITEM_TYPE_PREVIOUS
				     );
  
  item_folder->item_child->item_parent = item_folder;
  

  mbdesktop_item_set_activate_callback (mb, item_folder->item_child, 
					mbdesktop_item_folder_prev_activate_cb); 

  /* XXXX Below should go in append ! 
  item_folder->item_parent = mb->top_head_item; 

  if (mb->top_head_item->item_child == NULL)
    mb->top_head_item->item_child = item_folder;
  else
    mbdesktop_items_prepend( mb, &mb->top_head_item->item_child, item_folder);
  */

  return item_folder; /* ->item_child; */
}

void
mbdesktop_module_set_userdata (MBDesktop             *mb, 
			       MBDesktopFolderModule *folder_module,
			       void                  *data)
{
  folder_module->user_data = data;
}

void *
mbdesktop_module_get_userdata_from_item (MBDesktop     *mb, 
					 MBDesktopItem *item)
{
  /* XXXX broke - see above
  if (item->module_handle)
    return (void *)item->module_handle->user_data;

  if (item->item_parent && item->item_parent->module_handle)
    return (void *)item->item_parent->module_handle->user_data;

  return NULL;
  */
  return NULL;
}


int
mbdesktop_module_get_register_type (MBDesktop *mb )
{
  return mb->type_register_cnt++;
}
