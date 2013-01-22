/*
 *  miniwave - Tiny 820.11 wireless 
 *
 *  Note: you can use themes from http://www.eskil.org/wavelan-applet/  
 *
 *  originally based on wmwave
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Street #330, Boston, MA 02111-1307, USA.
 *
 */

#include <libmb/mb.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef ENABLE_NLS
# include <libintl.h>
# define _(text) gettext(text)
#else
# define _(text) (text)
#endif

#include <sys/types.h>
#include <sys/ioctl.h>

#include <netdb.h>      /* gethostbyname, getnetbyname */
#if 0
#include <linux/if_arp.h>   /* For ARPHRD_ETHER */
#include <linux/socket.h>   /* For AF_INET & struct sockaddr */
#endif
#include <sys/socket.h>       /* For struct sockaddr_in */
#if 0
#include <linux/wireless.h>
#else
#include <iwlib.h>

#endif

#ifdef MB_HAVE_PNG
#define IMG_EXT "png"
#else
#define IMG_EXT "xpm"
#endif

enum {
  MW_BROKE = 0,
  MW_NO_LINK,
  MW_SIG_1_40,
  MW_SIG_41_60,
  MW_SIG_61_80,
  MW_SIG_80_100,
};

#define  MW_BROKE_IMG      "broken-0."      IMG_EXT
#define  MW_NO_LINK_IMG    "no-link-0."     IMG_EXT
#define  MW_SIG_1_40_IMG   "signal-1-40."   IMG_EXT
#define  MW_SIG_41_60_IMG  "signal-41-60."  IMG_EXT
#define  MW_SIG_61_80_IMG  "signal-61-80."  IMG_EXT
#define  MW_SIG_80_100_IMG "signal-81-100." IMG_EXT

static char *ImgLookup[64] = {
  MW_BROKE_IMG,      
  MW_NO_LINK_IMG,    
  MW_SIG_1_40_IMG,   
  MW_SIG_41_60_IMG,  
  MW_SIG_61_80_IMG,  
  MW_SIG_80_100_IMG, 

};

static char          *ThemeName = NULL;
static MBPixbuf      *pb;
static MBPixbufImage *Imgs[6] = { 0,0,0,0,0,0 }, 
                     *ImgsScaled[6] = { 0,0,0,0,0,0 };
static int            CurImg = MW_BROKE;
static int            LastImg = -1;



struct {
   char *iface;			/* Interface name */
   char *essid;			/* ESSID */
   char *mode;			/* Mode */
   int   quality;		/* Quality (%) */
   int   level, noise;		/* Signal level, noise (dBm) */
} Mwd;

/* iwlib stuff  */

int  Wfd; /* file descriptor for socket */
static struct wireless_info WInfo;

Bool
update_wireless(void) 
{
  /* urg, iwlib api :/ */

  if (Wfd == -1) 
    {
      fprintf(stderr, "mb-applet-wireless: Kernel lacks wireless support?\n" );
      return False;
    }

  if (Mwd.iface == NULL)
      return False;

  if (iw_get_basic_config(Wfd, Mwd.iface, &WInfo.b) < 0)
    {
      fprintf(stderr, "mb-applet-wireless: unable to read wireless config\n" );
      return False;
    }

  if(iw_get_range_info(Wfd, Mwd.iface, &(WInfo.range)) >= 0)
    WInfo.has_range = 1;  

  if (iw_get_stats(Wfd, Mwd.iface, 
		   &(WInfo.stats),
                   &(WInfo.range), WInfo.has_range) >= 0)
    WInfo.has_stats = 1;
    
  Mwd.essid = ( WInfo.b.has_essid ? WInfo.b.essid : NULL );
  Mwd.mode  = ( WInfo.b.has_mode ? (char *)iw_operation_mode[WInfo.b.mode] : NULL );

  if (WInfo.has_stats) 
    {
      /* via http://www.snorp.net/files/patches/wireless-applet.c */
      Mwd.quality = (int)rint ((log (WInfo.stats.qual.qual) / log (94)) * 100);

      if (Mwd.quality > 100) 
	Mwd.quality = 100;
      else 
	if (Mwd.quality < 0) 
	  Mwd.quality = 0;

      Mwd.level = (int)WInfo.stats.qual.level;
      Mwd.noise = (int)WInfo.stats.qual.noise;
    } 
  else 
    {
      Mwd.quality = -1;
      Mwd.level = -1;
      Mwd.noise = -1;
    }

  return True;
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{

  MBPixbufImage *img_backing = NULL;

  if (update_wireless())
    {
      if (Mwd.quality != -1)
	{
	  if (Mwd.quality >= 0 && Mwd.quality <= 40)
	    CurImg = MW_SIG_1_40;
	  else if (Mwd.quality > 40 && Mwd.quality <= 60)
	    CurImg = MW_SIG_41_60;
	  else if (Mwd.quality > 60 && Mwd.quality <= 80)
	    CurImg = MW_SIG_61_80;
	  else if (Mwd.quality > 80)
	    CurImg = MW_SIG_80_100;
	  else
	    CurImg = MW_NO_LINK;
	}
      else CurImg = MW_NO_LINK;
    } 
  else CurImg = MW_BROKE;

  if (LastImg == CurImg) return;
  
  img_backing = mb_tray_app_get_background (app, pb);

  mb_pixbuf_img_copy_composite(pb, img_backing, 
			       ImgsScaled[CurImg], 0, 0,
			       mb_pixbuf_img_get_width(ImgsScaled[0]),
			       mb_pixbuf_img_get_height(ImgsScaled[0]),
			       mb_tray_app_tray_is_vertical(app) ? 
			       (mb_pixbuf_img_get_width(img_backing)-mb_pixbuf_img_get_width(ImgsScaled[0]))/2 : 0,
			       mb_tray_app_tray_is_vertical(app) ? 0 : 
			       (mb_pixbuf_img_get_height(img_backing)-mb_pixbuf_img_get_height(ImgsScaled[0]))/2 );

  mb_pixbuf_img_render_to_drawable(pb, img_backing, drw, 0, 0);

  mb_pixbuf_img_free( pb, img_backing );

  LastImg = CurImg;
}


void
load_icons(MBTrayApp *app)
{
 int   i;
 char *icon_path;

  for (i=0; i<6; i++)
    {
      if (Imgs[i] != NULL) mb_pixbuf_img_free(pb, Imgs[i]);
      icon_path = mb_dot_desktop_icon_get_full_path (ThemeName, 
						     32, 
						     ImgLookup[i]);
      
      if (icon_path == NULL 
	  || !(Imgs[i] = mb_pixbuf_img_new_from_file(pb, icon_path)))
	{
	  fprintf(stderr, "mb-applet-wireless: failed to load icon\n" );
	  exit(1);
	}

      free(icon_path);
    }
}

void
resize_callback (MBTrayApp *app, int w, int h )
{
  int  i;
  int  base_width  = mb_pixbuf_img_get_width(Imgs[0]);
  int  base_height = mb_pixbuf_img_get_height(Imgs[0]);
  int  scale_width = base_width, scale_height = base_height;
  Bool want_resize = True;

  if (mb_tray_app_tray_is_vertical(app) && w < base_width)
    {

      scale_width = w;
      scale_height = ( base_height * w ) / base_width;

      want_resize = False;
    }
  else if (!mb_tray_app_tray_is_vertical(app) && h < base_height)
    {
      scale_height = h;
      scale_width = ( base_width * h ) / base_height;
      want_resize = False;
    }

  if (w < base_width && h < base_height
      && ( scale_height > h || scale_width > w))
    {
       /* Something is really wrong to get here  */
      scale_height = h; scale_width = w;
      want_resize = False;
    }

  if (want_resize)  /* we only request a resize is absolutely needed */
    {
      LastImg = -1;
      mb_tray_app_request_size (app, scale_width, scale_height);
    }

  for (i=0; i<6; i++)
    {
      if (ImgsScaled[i] != NULL) 
	mb_pixbuf_img_free(pb, ImgsScaled[i]);

      ImgsScaled[i] = mb_pixbuf_img_scale(pb, 
					  Imgs[i], 
					  scale_width, 
					  scale_height);
    }
}

void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  char tray_msg[256];
  char quality[10];
  char level[10];
  char noise[10];

  update_wireless();
  
  if (Mwd.quality != -1)
    snprintf (quality, 10, "%u%%", Mwd.quality);
  else
  	strncpy (quality, "Unknown", 10);
  if (Mwd.level != -1)
    snprintf (level, 10, "%udBm", Mwd.level);
  else
  	strncpy (level, "Unknown", 10);
  if (Mwd.noise != -1)
    snprintf (noise, 10, "%udBm", Mwd.noise);
  else
  	strncpy (noise, "Unknown", 10);

  if (!is_released) return;

  snprintf(tray_msg, 256,
	  "%s:\n"
	  "  Mode: %s\n" 
	  "  ESSID: %s\n"
	  "  Quality: %s\n"
	  "  Level: %s\n"
	  "  Noise: %s\n",
	  Mwd.iface, 
	  Mwd.mode ? Mwd.mode : "Unknown",
	  Mwd.essid ? Mwd.essid  : "Unknown",
	  quality, 
	  level, 
	  noise );

  mb_tray_app_tray_send_message(app, tray_msg, 5000);
}

void 
theme_callback (MBTrayApp *app, char *theme_name)
{
  if (!theme_name) return;
  if (ThemeName) free(ThemeName);

  LastImg = -1; 			/* Make sure paint gets updated */

  ThemeName = strdup(theme_name);

  load_icons(app);

  resize_callback (app, mb_tray_app_width(app), mb_tray_app_width(app) );
}

void
timeout_callback ( MBTrayApp *app )
{
  mb_tray_app_repaint (app);
}

int  				/* repeatadly call via enum_devices */
find_iwface(int Wfd, char *ifname, char *args[], int count)
{

  /* is it a wireless if */
 if (iw_get_basic_config(Wfd, ifname, &WInfo.b) < 0)
   return 0;

 /* dont stop check interfaces till we find one that supports stats 
  * works round odd issues on Z with host AP.
 */
 if (Mwd.iface != NULL && WInfo.has_stats == 1) 
   return 0;

 if(iw_get_range_info(Wfd, Mwd.iface, &(WInfo.range)) >= 0)
   WInfo.has_range = 1;  

 if (iw_get_stats(Wfd, Mwd.iface, 
		  &(WInfo.stats),
		  &(WInfo.range), WInfo.has_range) >= 0)
   WInfo.has_stats = 1;

 /* mark first found as one to monitor */
 if (Mwd.iface)
   free(Mwd.iface);

 Mwd.iface = strdup(ifname);
 
 return 0;
}


int
main( int argc, char *argv[])
{
  MBTrayApp *app = NULL;
  struct timeval tv;

#if ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, DATADIR "/locale");
  bind_textdomain_codeset (PACKAGE, "UTF-8"); 
  textdomain (PACKAGE);
#endif

  memset(&WInfo, 0, sizeof(struct wireless_info));
  Wfd = iw_sockets_open();

  if (Wfd != -1)
    iw_enum_devices(Wfd, find_iwface, NULL, 0);

  app = mb_tray_app_new ( _("Wireless Monitor"),
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  
  
   pb = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
		      mb_tray_app_xscreen(app));
   
   memset(&tv,0,sizeof(struct timeval));

   tv.tv_sec = 2;

   load_icons(app);

   mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 
   
   mb_tray_app_set_button_callback (app, button_callback );
   
   mb_tray_app_set_theme_change_callback (app, theme_callback );

   mb_tray_app_set_icon(app, pb, Imgs[3]);
   
   mb_tray_app_main (app);

   if (Mwd.iface != NULL)
      free(Mwd.iface);

   return 1;
}



