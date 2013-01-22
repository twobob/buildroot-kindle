#include "panel_util.h"

extern MBPanel* G_panel;

void
util_cleanup_children(int s)
{
  DBG("DIE DIE\n");
  kill(-getpid(), 15);  /* kill every one in our process group  */
  exit(0);
}

void 
util_install_signal_handlers(void)
{

  signal (SIGCHLD, SIG_IGN);  /* kernel can deal with zombies  */
  signal (SIGINT, util_cleanup_children);
  signal (SIGQUIT, util_cleanup_children);
  signal (SIGTERM, util_cleanup_children);
  signal (SIGHUP, util_handle_hup);
  signal (SIGALRM, util_handle_alarm);
  //  signal (SIGSEGV, cleanup_children);

}

int 
util_handle_xerror(Display *dpy, XErrorEvent *e)
{
  char msg[255];
  XGetErrorText(dpy, e->error_code, msg, sizeof msg);
  fprintf(stderr, "Panel X error (%#lx):\n %s (opcode: %i)\n",
	  e->resourceid, msg, e->request_code);
  return 0;
}

pid_t 
util_fork_exec(char *cmd)
{
  pid_t pid, mypid;
  mypid = getpid();
  pid = fork();

  switch (pid) {
  case 0:
    setpgid (0, mypid); /* set pgid to parents pid  */
    mb_exec(cmd);
    fprintf(stderr, "exec failed, cleaning up child\n");
    exit(1);
  case -1:
    fprintf(stderr, "can't fork\n"); break;
  }
  return pid;
}


void 
util_handle_alarm(int s)
{
  MBPanel *p = G_panel;
  DBG("%s() called", __func__);
  session_save(p);
}

void 
util_handle_hup(int s)
{
  MBPanel *p = G_panel;
  DBG("%s() called", __func__);

  if (p != NULL)
    {
      p->reload_pending = True;
    }
}

void
util_get_mouse_position(MBPanel *panel, int *x, int *y)
{
  Window mouse_root, mouse_win;
  int win_x, win_y;
  unsigned int mask;

  XQueryPointer(panel->dpy, panel->win_root, &mouse_root, &mouse_win,
		x, y, &win_x, &win_y, &mask);
}

pid_t
util_get_window_pid_from_prop(MBPanel *panel, Window win)
{
  Atom type;
  int format;
  long bytes_after;
  unsigned int *data = NULL;
  long n_items;
  int result;
  pid_t pid_result = 0; 

  result =  XGetWindowProperty (panel->dpy, win, 
				panel->atoms[ATOM_NET_WM_PID],
				0, 16L,
				False, XA_CARDINAL,
				&type, &format, &n_items,
				&bytes_after, (unsigned char **)&data);

  if (result == Success && n_items) 
    pid_result = *data;

  if (data) XFree(data);

  return pid_result;
}

Bool
util_get_command_str_from_win(MBPanel *panel, Window win, char **result)
{
   int i, bytes_needed = 0; 
   char *p = NULL, *cmd = NULL, **argv_win;
   int argc_win;

   if (!XGetCommand(panel->dpy, win, &argv_win, &argc_win))
     return False;

   bytes_needed = strlen(argv_win[0])+2;

   for(i=1;i<argc_win;i++)
   {
     bytes_needed += strlen(argv_win[i])+2;
     for (p = argv_win[i]; *p != '\0'; p++)
       if (*p == ' ' || *p == '\t')
	 bytes_needed++;
   }

   *result = malloc(sizeof(char)*bytes_needed);
   cmd = *result;

   strcpy(cmd, argv_win[0]);
   while(*cmd != '\0') cmd++;

   for(i=1;i<argc_win;i++)
   {
     p = argv_win[i];
     *cmd++ = ' ';

     if (strpbrk(p, " \t") == NULL)
       {
	 while (*p) *cmd++ = *p++;
       } else {
	 *cmd++ = '\'';
	 while (*p)
	   {
	     if (*p == '\'')
	       *cmd++ = '\\';
	     *cmd++ = *p++;
	   }
	 *cmd++ = '\'';
       }
   }

   *cmd = '\0';

   XFreeStringList(argv_win);

   return True;
}

Bool
util_xcol_from_spec(MBPanel *panel, MBColor *col, char *spec)
{
  mb_col_set (col, spec);
  return True;
}

Pixmap
util_get_root_pixmap(MBPanel *panel)
{
  Pixmap root_pxm = None;

  Atom type;
  int format;
  long bytes_after;
  Pixmap *data = NULL;
  long n_items;
  int result;

  result =  XGetWindowProperty (panel->dpy, panel->win_root, 
				panel->atoms[ATOM_XROOTPMAP_ID],
				0, 16L,
				False, XA_PIXMAP,
				&type, &format, &n_items,
				&bytes_after, (unsigned char **)&data);

  if (result == Success && n_items) 
    root_pxm = *data;

  if (data) XFree(data);

  panel->root_pixmap_id = root_pxm;

  return root_pxm;
}

unsigned char *
util_get_utf8_prop(MBPanel *panel, Window win, Atom req_atom)
{
  Atom type;
  int format;
  long bytes_after;
  unsigned char *str = NULL;
  long n_items;
  int result;

  result =  XGetWindowProperty (panel->dpy, win, 
				req_atom,
				0, 1024L,
				False, panel->atoms[ATOM_UTF8_STRING],
				&type, &format, &n_items,
				&bytes_after, (unsigned char **)&str);

  if (result != Success || str == NULL)
    {
      if (str) XFree (str);
      return NULL;
    }

  if (type != panel->atoms[ATOM_UTF8_STRING] || format != 8 || n_items == 0)
    {
      XFree (str);
      return NULL;
    }

  return str;
}
