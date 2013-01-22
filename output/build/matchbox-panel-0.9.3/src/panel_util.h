#ifndef _HAVE_PANEL_UTIL_H
#define _HAVE_PANEL_UTIL_H

#include "panel.h"

void util_cleanup_children(int s);

void util_install_signal_handlers(void);

int  util_handle_xerror(Display *dpy, XErrorEvent *e);

pid_t util_fork_exec(char *cmd);

void util_handle_alarm(int s);

void util_handle_hup(int s);

void util_get_mouse_position(MBPanel *panel, int *x, int *y);

pid_t util_get_window_pid_from_prop(MBPanel *panel, Window win);

Bool util_get_command_str_from_win(MBPanel *panel, Window win, char **result);

Bool
util_xcol_from_spec(MBPanel *panel, MBColor *col, char *spec);

Pixmap util_get_root_pixmap(MBPanel *panel);

unsigned char *
util_get_utf8_prop(MBPanel *panel, Window win, Atom req_atom);


#endif 
