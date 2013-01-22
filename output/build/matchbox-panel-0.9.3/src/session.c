#include "session.h"

#define DEFAULT_SESSIONS "mb-applet-menu-launcher,mb-applet-clock"

void 
session_set_defaults(MBPanel *panel, char *defaults)
{
  panel->session_defaults_cur_pos = strdup(defaults);
  panel->use_alt_session_defaults = True;
}

void
session_destroy(MBPanel *panel)
{
  char sessionfile[512] = { 0 };

  if (!panel->use_session) return;

  if (panel->system_tray_id)
    snprintf(sessionfile, 512, "%s/.matchbox/%s.%i", getenv("HOME"), 
	     PANELFILE, panel->system_tray_id );   
  else
    snprintf(sessionfile, 512, "%s/.matchbox/%s", 
	     getenv("HOME"), PANELFILE );   
  
  unlink(sessionfile);
}

void 
session_init(MBPanel *panel)
{
  char sessionfile[512] = { 0 };
  struct stat st;

  DBG("%s() called\n", __func__);

  panel->session_preexisting_lock   = False;
  panel->session_run_first_time     = False;

  if (!panel->use_session && !panel->use_alt_session_defaults) return;

  /*
  if (panel->use_alt_session_defaults)
    {
      panel->session_cur_gravity = PAPP_GRAVITY_END;
      panel->session_run_first_time = True;
    }
  */ 

  if (getenv("HOME") == NULL)
    {
      fprintf(stderr, "matchbox-panel: unable to get home directory, is HOME set?\n");
      panel->session_preexisting_lock = False;
      return;
    }

  snprintf(sessionfile, 512, "%s/.matchbox", getenv("HOME"));
  
  /* Check if ~/.matchbox exists and create if not */
  if (stat(sessionfile, &st) != 0 /* || !S_ISDIR(st.st_mode) */
      || !panel->use_session)
    {
      if (panel->use_session)
	{
	  fprintf(stderr, "matchbox-panel: creating %s directory for session files\n",
		  sessionfile);
	  mkdir(sessionfile, 0755);
	}
      panel->session_cur_gravity = PAPP_GRAVITY_END;
      panel->session_run_first_time = True;
      if (!panel->use_alt_session_defaults)
	panel->session_defaults_cur_pos = strdup(DEFAULT_SESSIONS);
    }
  else
    {
      /* We have a ~/.matchbox , see if we have a session file 
	 and set defualts if not. 
      */
      if (panel->system_tray_id)
	snprintf(sessionfile, 512, "%s/.matchbox/%s.%i", getenv("HOME"), 
		 PANELFILE, panel->system_tray_id );   
      else
	snprintf(sessionfile, 512, "%s/.matchbox/%s", 
		 getenv("HOME"), PANELFILE );   

      if ((panel->session_fp = fopen(sessionfile, "r")) == NULL)
	{
	  fprintf(stderr,
		  "mbpanel: Session file does not exist ( tryed %s )\n", 
		  sessionfile);
	  if (!panel->use_alt_session_defaults)
	    {
	      panel->session_defaults_cur_pos = strdup(DEFAULT_SESSIONS);
	    }
	  panel->session_cur_gravity = PAPP_GRAVITY_START;
	  panel->session_run_first_time = True;
	}
      else
	{
	    DBG("%s() opened %s\n", __func__, sessionfile);
	}
    }

  panel->session_preexisting_lock = True;  /* we are loading session data  */
  panel->session_entry_cur[0] = 0; 

  session_preexisting_start_next(panel);
   
}

void session_save(MBPanel *panel)
{
  char *sessionfile = alloca(sizeof(char)*255);

  int i = 0;
  MBPanelApp *papp = NULL;
  MBPanelApp *papp_heads[] = { panel->apps_start_head, 
			       panel->apps_end_head    };
  
  DBG("%s() called\n", __func__);

  if (!panel->use_session 
      || session_preexisting_restarting(panel))
    return;

  if (getenv("HOME") == NULL)
    {
      fprintf(stderr, "matchbox-panel: unable to get home directory, is HOME set?\n");
      return;
    }

  if (panel->system_tray_id)
    snprintf(sessionfile, 255, "%s/.matchbox/%s.%i", getenv("HOME"), 
	     PANELFILE, panel->system_tray_id );   
  else
    snprintf(sessionfile, 255, "%s/.matchbox/%s", 
	     getenv("HOME"), PANELFILE );   
  
  if ((panel->session_fp = fopen(sessionfile, "w")) == NULL)
    { 
      fprintf(stderr,"matchbox-panel: Unable to create Session file ( %s )\n", 
	      sessionfile); 
      return; 
    }

  DBG("%s() still called\n", __func__);
  
  while (i < 2)
    {
      papp = papp_heads[i];
      while( papp != NULL )
	{
	  DBG("%s() writing %s\n", __func__, papp->cmd_str);
	  if (papp->cmd_str)
	    {
	      DBG("%s() writing %s\n", __func__, papp->cmd_str);
	      fprintf(panel->session_fp, "%s\n", papp->cmd_str);  
	    }
	  papp = papp->next;
	}
      fprintf(panel->session_fp, "\t\n" );
      i++;
    }
  
  fclose(panel->session_fp);
}

Bool 
session_preexisting_restarting(MBPanel *panel)
{
  return panel->session_preexisting_lock;
}

Bool
session_preexisting_start_next(MBPanel *panel)
{
  int i = 0;

  if (!session_preexisting_restarting(panel)) return False;

  if (panel->session_entry_cur[0] == '\0' 
      && session_preexisting_get_next(panel))
    {
      DBG("%s() starting %s\n", __func__, panel->session_entry_cur);
      panel->session_needed_pid = util_fork_exec(panel->session_entry_cur);
      return True;
    }

  /* Nothing left in session, dock any defered */

  DBG("%s() Now launching defered apps\n", __func__);

  for (i=0; i<panel->n_session_defered_wins; i++)
    panel_handle_dock_request(panel, panel->session_defered_wins[i]);
  

  return False;
}

Bool 
session_preexisting_win_matches_wanted(MBPanel *panel, Window win, 
				       char *win_cmd)
{
  pid_t win_pid = 0;

  if (!session_preexisting_restarting(panel)) return False;

  DBG("%s() called\n", __func__);

  if (panel->session_entry_cur) /* what were waiting on */
    {
      /* Check if its got the pid we expect */
      win_pid = util_get_window_pid_from_prop(panel, win);

      DBG("%s() win pid is %i\n", __func__, win_pid);

      if (win_pid && win_pid == panel->session_needed_pid)
	return True;

      DBG("%s() pid failed, comparing '%s' vs '%s'\n", __func__, 
	  win_cmd, panel->session_entry_cur );

      /* check cmd str */
      if (win_cmd && !strncmp(win_cmd, panel->session_entry_cur, 
			      strlen(win_cmd)))
	return True;
    }

  return False;
}

void
session_preexisting_clear_current(MBPanel *panel)
{
  panel->session_entry_cur[0] = '\0';
}

Bool
session_preexisting_set_timeout (MBPanel        *panel, 
				 struct timeval *tv, 
				 struct timeval **tvp)
{
  int timeleft;

  for (;;)
    {
      if (!session_preexisting_restarting(panel)) 
	return False;

      if (panel->session_entry_cur[0] == '\0')
	return False;

      timeleft = SESSION_TIMEOUT - (time (NULL) - panel->session_start_time);

      if (timeleft <= 0)
	{
	  fprintf(stderr, "Session timeout on %s\n", panel->session_entry_cur);
	  session_preexisting_clear_current(panel);
	  session_preexisting_start_next(panel);
	  continue;
	}

      break;
    }

  if (!*tvp || tv->tv_sec > timeleft)
    {
      tv->tv_usec = 0;
      tv->tv_sec = timeleft;
      *tvp = tv;
    }

  return True;
}

Bool 
session_preexisting_get_next(MBPanel *panel) /* session_restarting_get_next  */
{
  char *tmp;

  if (!session_preexisting_restarting(panel)) return False;

  panel->session_init_offset = 10;

  if (panel->session_run_first_time)
    {
      if (*panel->session_defaults_cur_pos != '\0')
	{
	  char *prev_pos  = panel->session_defaults_cur_pos;
	  while ( *panel->session_defaults_cur_pos != '\0')
	    {
	      if ( *panel->session_defaults_cur_pos == ',')
	       {
		 *panel->session_defaults_cur_pos = '\0';
		 panel->session_defaults_cur_pos++;
		 break;
	       }
	      panel->session_defaults_cur_pos++;
	    }

	  strncpy(panel->session_entry_cur, prev_pos, 512);
         }
      else
	{
	  panel->session_run_first_time   = False;
	  panel->session_preexisting_lock = False;
	  session_save(panel);
	  return False;
	}
    }
  else
    {

      if (fgets(panel->session_entry_cur, 512, panel->session_fp) == NULL)
	{
	  fclose(panel->session_fp); /* All sessions done */
	  panel->session_preexisting_lock = False;
	  session_save(panel);
	  return False;
	}
      
      /* tab + newline -> change the session gravity */
      if (!strcmp(panel->session_entry_cur, "\t\n"))
	{
	  panel->session_cur_gravity = PAPP_GRAVITY_END;
	  return session_preexisting_get_next(panel);
	}

      if ( panel->session_entry_cur[strlen(panel->session_entry_cur)-1] == '\n')
	panel->session_entry_cur[strlen(panel->session_entry_cur)-1] = '\0';

      if ( (tmp = strstr(panel->session_entry_cur, "\t\t")) != NULL )
	{
	  panel->session_init_offset = atoi(tmp);
	  *tmp = '\0';
	}
    }

  panel->session_start_time = time(NULL);
  return True;
}
