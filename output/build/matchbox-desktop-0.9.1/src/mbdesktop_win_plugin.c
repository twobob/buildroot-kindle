#include "mbdesktop.h"
#include "mbdesktop_win_plugin.h"

void
mbdesktop_win_plugin_init (MBDesktop *mb)
{
  mb->win_plugin_rect.x      = 0;
  mb->win_plugin_rect.y      = 0;
  mb->win_plugin_rect.width  = 0;
  mb->win_plugin_rect.height = 0;

  mb->win_plugin             = None;
}

Bool
mbdesktop_win_plugin_load (MBDesktop *mb, char *cmd)
{
  pid_t pid;
  int   fd[2];
  
  pipe(fd);
  pid = fork();

  if (pid == 0)
    {
      close (fd[0]);

      if (dup2 (fd[1], 1) < 0)
        perror ("dup2");

      close(fd[0]);

      if (fcntl (1, F_SETFD, 0))
        perror ("fcntl");

      mb_exec (cmd);
      
      perror (cmd);
      
      exit (1);
    }

  close(fd[1]);

  {
    char buf[256];
    char c;
    int a = 0;
    size_t n;

    do {
      n = read (fd[0], &c, 1);
      if (n)
        {
          buf[a++] = c;
        }
    } while (n && (c != 10) && (a < (sizeof (buf) - 1)));

    if (a)
      {   
        buf[a] = 0;
        mb->win_plugin = atoi (buf);

	printf("%s(): got winid %li\n", __func__, mb->win_plugin);
      }
  }

  if (mb->win_plugin) return True;
  return False;

}

void
mbdesktop_win_plugin_reparent (MBDesktop *mb)
{
  XWindowAttributes  attr;

  XGetWindowAttributes(mb->dpy, mb->win_plugin, &attr);  

  mb->win_plugin_rect.x      = mb->workarea_x; 
  mb->win_plugin_rect.y      = mb->workarea_y + mb->title_offset; 
  mb->win_plugin_rect.width  = mb->workarea_width;
  mb->win_plugin_rect.height = attr.height;

  XResizeWindow(mb->dpy, mb->win_plugin, 
		mb->win_plugin_rect.width, mb->win_plugin_rect.height);

  /* XFlush needed or toplevel background dont get pained ....  */
  XFlush(mb->dpy);

  XReparentWindow(mb->dpy, mb->win_plugin, mb->win_top_level, 
  		  mb->win_plugin_rect.x, mb->win_plugin_rect.y);

  printf("%s() win_plugin rect is +%i+%i, %ix%i\n", __func__,
	 mb->win_plugin_rect.x, mb->win_plugin_rect.y,
	 mb->win_plugin_rect.width, mb->win_plugin_rect.height);
	 

  //XSelectInput(mb->dpy, mb->win_plugin, PropertyChangeMask );

  mbdesktop_win_plugin_config_event (mb);

  XMapWindow(mb->dpy, mb->win_plugin);

}


void
mbdesktop_win_plugin_configure_request (MBDesktop              *mb, 
				        XConfigureRequestEvent *ev)
{
  XWindowChanges xwc;

  printf("%s() called\n", __func__);

  xwc.width  = mb->win_plugin_rect.width;
  xwc.height = mb->win_plugin_rect.height;
  
  xwc.x = mb->win_plugin_rect.x;
  xwc.y = mb->win_plugin_rect.y;

  xwc.border_width = 0;
  xwc.sibling      = None;
  xwc.stack_mode   = None;

  XConfigureWindow(mb->dpy, mb->win_plugin, ev->value_mask, &xwc);
}


void
mbdesktop_win_plugin_config_event (MBDesktop *mb)
{
   XConfigureEvent ce;

   ce.type = ConfigureNotify;
   ce.event  = mb->win_plugin;
   ce.window = mb->win_plugin;

   ce.x = mb->win_plugin_rect.x;
   ce.y = mb->win_plugin_rect.y;
   ce.width  = mb->win_plugin_rect.width;
   ce.height = mb->win_plugin_rect.height;
   
   ce.border_width = 0;
   ce.above = mb->win_top_level;
   ce.override_redirect = 0;

   XSendEvent(mb->dpy, mb->win_plugin, False,
	      StructureNotifyMask, (XEvent *)&ce);

   XSync(mb->dpy, False);
}
