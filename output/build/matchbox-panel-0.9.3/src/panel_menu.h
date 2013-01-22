#ifndef _HAVE_PANEL_MENU_H
#define _HAVE_PANEL_MENU_H

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(text) gettext(text)
#else
# define _(text) (text)
#endif

#include "panel.h"

void panel_menu_exec_cb(MBMenuItem *item);

void panel_menu_exit_cb(MBMenuItem *item);

void panel_menu_hide_cb(MBMenuItem *item);

void panel_menu_move_app_cb(MBMenuItem *item);

void panel_menu_kill_cb(MBMenuItem *item);

void panel_menu_update_remove_items(MBPanel *panel);

void panel_menu_init(MBPanel *panel);

#endif
