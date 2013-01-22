#ifndef _HAVE_PANEL_SESSION_H
#define _HAVE_PANEL_SESSION_H

#include "panel.h"

#ifdef DEBUG
#define PANELFILE "mbdock.session.debug"
#else
#define PANELFILE "mbdock.session"
#endif

void
session_destroy(MBPanel *panel);

void 
session_set_defaults(MBPanel *panel, char *defaults);

void session_init(MBPanel *panel);

void session_save(MBPanel *panel);

Bool session_preexisting_restarting(MBPanel *panel);

Bool session_preexisting_start_next(MBPanel *panel);;

Bool session_preexisting_win_matches_wanted(MBPanel *panel, Window win, 
					    char *win_cmd);

void session_preexisting_clear_current(MBPanel *panel);

Bool session_preexisting_set_timeout(MBPanel *panel, struct timeval *tv, struct timeval **tvp);

Bool session_preexisting_get_next(MBPanel *panel);



#endif
