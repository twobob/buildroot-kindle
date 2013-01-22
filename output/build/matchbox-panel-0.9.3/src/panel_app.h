#ifndef _PANEL_APP_H_
#define _PANEL_APP_H_

#include "panel.h"

MBPanelApp*
panel_app_list_get_prev (MBPanel     *panel, 
			 MBPanelApp  *papp, 
			 MBPanelApp **list_head);
MBPanelApp*
panel_app_list_get_last (MBPanel    *panel, 
			 MBPanelApp *list_head);


MBPanelApp  *
panel_app_list_prepend(MBPanel     *panel, 
		       MBPanelApp  *list_to_append_to,
		       MBPanelApp  *papp_new);

/*
void 
panel_app_list_prepend(MBPanel     *panel, 
		       MBPanelApp **list_to_append_to,
		       MBPanelApp  *papp_new);
*/

void
panel_app_list_append (MBPanel     *panel, 
		       MBPanelApp **list_to_append_to,
		       MBPanelApp  *new_client);


void panel_app_list_insert_after(MBPanel *panel, MBPanelApp *papp, 
				 MBPanelApp *new_papp);

void panel_app_list_remove (MBPanel     *panel, 
			    MBPanelApp  *papp,
			    MBPanelApp **list_head );

void
panel_app_add_start(MBPanel *panel, MBPanelApp *papp_new);

void
panel_app_add_end(MBPanel *panel, MBPanelApp *papp_new);


void panel_app_list_add(MBPanel *panel, MBPanelApp *papp_new);

void panel_app_name_get(MBPanel *panel, MBPanelApp *papp);

Window panel_app_get_client_leader_win(MBPanel *panel, MBPanelApp *papp);

int* panel_app_icon_prop_data_get(MBPanel *d, MBPanelApp *papp);

void panel_app_command_prop_get(MBPanel *panel, MBPanelApp *papp);

Bool panel_app_get_command_str(MBPanel *panel, MBPanelApp *papp, 
			       char **result);

MBPanelApp* panel_app_get_from_window(MBPanel *panel, Window win);

MBPanelApp* panel_app_new(MBPanel *panel, 
			  Window   win, 
			  char    *cmd );

void panel_app_handle_configure_request(MBPanel *panel, 
					XConfigureRequestEvent *ev);

void panel_app_deliver_config_event(MBPanel *panel, MBPanelApp *papp);

void
panel_apps_rescale (MBPanel    *panel, 
		    MBPanelApp *papp);

void
panel_apps_nudge (MBPanel    *panel, 
		  MBPanelApp *papp,
		  int         amount);

void 
panel_app_move_to(MBPanel *panel, MBPanelApp *papp, int origin_offset);

void 
panel_app_destroy(MBPanel *panel, MBPanelApp *papp);

int
panel_app_get_offset(MBPanel *panel, MBPanelApp *papp);

int
panel_app_get_size(MBPanel *panel, MBPanelApp *papp);

#endif
