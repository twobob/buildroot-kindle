#ifndef _HAVE_MBDESKTOP_WIN_PLUGIN_H
#define _HAVE_MBDESKTOP_WIN_PLUGIN_H

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

void
mbdesktop_win_plugin_init (MBDesktop *mb);

Bool
mbdesktop_win_plugin_load (MBDesktop *mb, char *cmd);

void
mbdesktop_win_plugin_reparent (MBDesktop *mb);

void
mbdesktop_win_plugin_config_event (MBDesktop *mb);

#endif
