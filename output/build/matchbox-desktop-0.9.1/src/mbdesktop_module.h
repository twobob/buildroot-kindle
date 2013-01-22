#ifndef _HAVE_MBDESKTOP_MODULE_H
#define _HAVE_MBDESKTOP_MODULE_H

#include "mbdesktop.h"
#include "mbdesktop_item.h"


#define MBDESKTOP_MODULE_REGISTER_FOLDER(module_name); \
MBDesktopFolderModule *__mb_desktop_module_handle = &(module_name);

MBDesktopItem *
mbdesktop_get_top_level_folder(MBDesktop     *mb);

MBDesktopItem*
mbdesktop_module_folder_create ( MBDesktop *mb,
				 char      *name,
				 char      *icon_name );

int
mbdesktop_module_get_register_type ( MBDesktop *mb );

void
mbdesktop_module_set_userdata (MBDesktop             *mb, 
			       MBDesktopFolderModule *folder_module,
			       void                  *data);
void *
mbdesktop_module_get_userdata_from_item (MBDesktop     *mb, 
					 MBDesktopItem *item);       

#endif
