#ifndef _HAVE_MBDESKTOP_VIEW_H
#define _HAVE_MBDESKTOP_VIEW_H

#include "mbdesktop.h"

enum {
  ALIGN_LEFT = 0,
  ALIGN_CENTER,
  ALIGN_RIGHT
};

void
mbdesktop_view_init_bg(MBDesktop *mb);

void
mbdesktop_view_set_root_pixmap(MBDesktop *mb, MBPixbufImage *img);

void 
mbdesktop_view_configure(MBDesktop *mb);

void 
mbdesktop_set_view(MBDesktop *mb, int view);

void 
mbdesktop_view_paint(MBDesktop *mb, Bool use_cache);

void
mbdesktop_view_paint_list(MBDesktop *mb, MBPixbufImage *dest_img);

void 
mbdesktop_view_paint_icons(MBDesktop *mb, MBPixbufImage *img_dest);

void
mbdesktop_view_item_highlight (MBDesktop     *mb, 
			       MBDesktopItem *item,
			       int            highlight_style);


#endif
