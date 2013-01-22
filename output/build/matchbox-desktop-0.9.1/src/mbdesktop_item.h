#ifndef _HAVE_MBDESKTOP_ITEM_H
#define _HAVE_MBDESKTOP_ITEM_H

#include "mbdesktop.h"


#define mbdesktop_items_enumerate_siblings(item_head, item)  \
     for ( (item) = (item_head);                             \
	   (item) != NULL;                                   \
	   (item) = (item)->item_next_sibling )              \

/**
 * Constructs a new blank mbpixbuf image without an alpha channel.
 *
 * @returns a MBPixbufImage object
 */
MBDesktopItem *
mbdesktop_item_new();

/**
 * Constructs a new blank mbpixbuf image without an alpha channel.
 *
 * @param mbdesktop
 * @param item
 */
void
mbdesktop_item_free(MBDesktop     *mbdesktop, 
		    MBDesktopItem *item);

MBDesktopItem*
mbdesktop_item_folder_get_open(MBDesktop     *mb);

Bool
mbdesktop_item_folder_has_contents(MBDesktop     *mb, 
				   MBDesktopItem *folder);


void
mbdesktop_item_folder_contents_free(MBDesktop     *mb, 
				    MBDesktopItem *item_folder);

MBDesktopItem*
mbdesktop_item_folder_set_view (MBDesktop     *mb,
			        MBDesktopItem *folder,
			        int            view);
int
mbdesktop_item_folder_get_view (MBDesktop     *mb,
				MBDesktopItem *folder);

Bool
mbdesktop_item_is_folder (MBDesktop *mb, MBDesktopItem *folder)	;

MBDesktopItem *
mbdesktop_item_new_with_params (MBDesktop     *mb,
			        const char    *name, 
			        const char    *icon_name,
			        void          *data,
			        int            type);

void
mbdesktop_item_set_icon_from_theme (MBDesktop     *mb,
				    MBDesktopItem *item);

void
mbdesktop_items_append (MBDesktop     *mb,
			MBDesktopItem *item_head,
			MBDesktopItem *item );

void
mbdesktop_items_insert_after (MBDesktop     *mb,
			      MBDesktopItem *suffix_item,
			      MBDesktopItem *item );


void
mbdesktop_items_append_to_folder (MBDesktop      *mb,
				  MBDesktopItem  *item_folder,
				  MBDesktopItem  *item );

void
mbdesktop_items_append_to_top_level (MBDesktop      *mb,
				     MBDesktopItem  *item );

void
mbdesktop_items_prepend (MBDesktop     *mb,
			 MBDesktopItem **item_head,
			 MBDesktopItem *item );

MBDesktopItem *
mbdesktop_item_get_first_sibling (MBDesktopItem *item) ;

MBDesktopItem *
mbdesktop_item_get_last_sibling(MBDesktopItem *item);

MBDesktopItem *
mbdesktop_item_get_next_sibling(MBDesktopItem *item);


MBDesktopItem *
mbdesktop_item_get_prev_sibling(MBDesktopItem *item);

MBDesktopItem *
mbdesktop_item_get_parent(MBDesktopItem *item);

MBDesktopItem *
mbdesktop_item_get_child(MBDesktopItem *item);

void
mbdesktop_item_set_icon_data (MBDesktop     *mb, 
			      MBDesktopItem *item, 
			      MBPixbufImage *img);

void
mbdesktop_item_set_image (MBDesktop     *mb, 
			  MBDesktopItem *item, 
			  char          *full_img_path);

void
mbdesktop_item_set_name (MBDesktop     *mb, 
			 MBDesktopItem *item, 
			 char          *name);

char *
mbdesktop_item_get_name (MBDesktop     *mb, 
			 MBDesktopItem *item);

void
mbdesktop_item_set_comment (MBDesktop     *mb, 
			    MBDesktopItem *item, 
			    char          *comment);

char *
mbdesktop_item_get_comment (MBDesktop     *mb, 
			    MBDesktopItem *item);

void
mbdesktop_item_set_extended_name (MBDesktop     *mb, 
				  MBDesktopItem *item, 
				  char          *name);

char *
mbdesktop_item_get_extended_name (MBDesktop     *mb, 
				  MBDesktopItem *item);


void
mbdesktop_item_set_user_data (MBDesktop     *mb, 
			      MBDesktopItem *item, 
			      void          *data);
void *
mbdesktop_item_get_user_data (MBDesktop     *mb, 
			      MBDesktopItem *item);


void
mbdesktop_item_set_type (MBDesktop     *mb, 
			 MBDesktopItem *item,
			 int            type);

int
mbdesktop_item_get_type (MBDesktop     *mb, 
			 MBDesktopItem *item);

void
mbdesktop_item_cache (MBDesktop     *mb, 
		      MBDesktopItem *item,
		      char          *ident );

MBDesktopItem *
mbdesktop_item_from_cache (MBDesktop *mb, 
			   char      *ident,
			   time_t     age);

void
mbdesktop_item_set_activate_callback (MBDesktop     *mb, 
				      MBDesktopItem *item, 
				      MBDesktopCB    activate_cb);

void
mbdesktop_item_folder_activate_cb(void *data1, void *data2);

void
mbdesktop_item_folder_prev_activate_cb(void *data1, void *data2);

MBDesktopItem *
mbdesktop_item_get_from_coords(MBDesktop *mb, int x, int y);


#endif
