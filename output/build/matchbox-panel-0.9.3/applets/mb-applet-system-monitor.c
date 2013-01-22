/*
 *  mb-applet-system-monitor - tiny sys monitor
 *
 *  cpu reading code based on wmbubblemon
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

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

#ifdef MB_HAVE_PNG
#define IMG_EXT "png"
#else
#define IMG_EXT "xpm"
#endif

#define MINISYS_IMG "minisys." IMG_EXT

struct {

   /* cpu data  */
   int loadIndex;
   int samples;
   u_int64_t *load, *total;

   /* memory data  */
   u_int64_t mem_used;
   u_int64_t mem_max;
   u_int64_t swap_used;
   u_int64_t swap_max;
   unsigned int swap_percent;  /* swap used, in percent */
   unsigned int mem_percent;   /* memory used, in percent */

} msd;   

MBPixbuf *pb = NULL;
MBPixbufImage *ImgIcon = NULL, *ImgIconScaled = NULL, *ImgGraph = NULL;
int GraphHeight = 0, GraphWidth = 0;
char *ThemeName;
int IsKernel26 = 0;

int 
check_if_kernel_2_6(void)
{
  float v_nr=0;
  FILE  *version;
  
  if ((version = fopen("/proc/version", "r")) == NULL)
    {
      fprintf(stderr, "mb-applet-system-monitor: failed to open /proc/version. Exiting\n");
      exit(1);
    }
  fscanf(version, "%*s %*s %f", &v_nr);
  fclose(version);
  
  return (v_nr > 2.5);
}

/* returns current CPU load in percent, 0 to 100 */
int system_cpu(void)
{
    unsigned int cpuload;
    u_int64_t load, total, oload, ototal;
    u_int64_t ab, ac, ad, ae;
    int i;
    FILE *stat;

    if ((stat = fopen("/proc/stat", "r")) == NULL)
      {
	fprintf(stderr, "mb-applet-system-monitor: failed to open /proc/stat. Exiting\n");
	exit(1);
      }

    fscanf(stat, "%*s %Ld %Ld %Ld %Ld", &ab, &ac, &ad, &ae);
    fclose(stat);

    /* Find out the CPU load */
    /* user + sys = load
     * total = total */
    load = ab + ac + ad;	/* cpu.user + cpu.sys; */
    total = ab + ac + ad + ae;	/* cpu.total; */

    /* "i" is an index into a load history */
    i = msd.loadIndex;
    oload = msd.load[i];
    ototal = msd.total[i];

    msd.load[i] = load;
    msd.total[i] = total;
    msd.loadIndex = (i + 1) % msd.samples;

    if (ototal == 0)	
	cpuload = 0;
    else
	cpuload = (100 * (load - oload)) / (total - ototal);

    return cpuload;
}

int system_memory(void)
{
  u_int64_t  total, mfree, buffers, cached, used, shared,
    cache_total, cache_free, cache_used, uneeded = 0;
  
  u_int64_t my_mem_used, my_mem_max;
  u_int64_t my_swap_max;
  
  static int mem_delay = 0;
  FILE      *mem;
  
  /* put this in permanent storage instead of stack */
  static char not_needed[2048];
  
  if (mem_delay-- <= 0) 
    {
      if ((mem = fopen("/proc/meminfo", "r")) == NULL) 
	{
	  fprintf(stderr, "mb-applet-system-monitor: failed to open /proc/meminfo. Exiting.\n");
	  exit(1);
	}
      
      fgets(not_needed, 2048, mem);

      if (IsKernel26)
	{
	  rewind (mem);
      
	  fscanf (mem, "%*s %Ld %*s", &total);
	  fscanf (mem, "%*s %Ld %*s", &mfree);
	  fscanf (mem, "%*s %Ld %*s", &buffers);
	  fscanf (mem, "%*s %Ld %*s", &cached);
	  fscanf (mem, "%*s %Ld %*s", &shared);
	  fscanf (mem, "%*s %Ld %*s", &used);
	  fscanf (mem, "%*s %Ld %*s", &uneeded);
	  fscanf (mem, "%*s %Ld %*s", &uneeded);
	  fscanf (mem, "%*s %Ld %*s", &uneeded);
	  fscanf (mem, "%*s %Ld %*s", &uneeded);
	  fscanf (mem, "%*s %Ld %*s", &uneeded);
	  fscanf (mem, "%*s %Ld %*s", &cache_total);
	  fscanf (mem, "%*s %Ld %*s", &cache_free);
      
	  total = total * 1024;
	  mfree = mfree * 1024;
	  buffers = buffers * 1024;
	  cached = cached * 1024;
	  used = used * 1024;
	  shared = shared * 1024;
	  cache_total = cache_total * 1024;
	  cache_used = cache_total - (cache_free * 1024);
	}
      else
	{ /* Assume 2.4  */
	  /*
	    total:    used:    free:  shared: buffers:  cached:
	  */
	  fscanf(mem, "%*s %Ld %Ld %Ld %Ld %Ld %Ld", &total, &used, &mfree,
		 &shared, &buffers, &cached);
	  fscanf(mem, "%*s %Ld %Ld", &cache_total, &cache_used);
	}
      
      fclose(mem);
      
      mem_delay = 25;
      
      /* calculate it */
      my_mem_max      = total;
      my_swap_max     = cache_total;
      my_mem_used     = cache_used + used - cached - buffers;
      msd.mem_used    = my_mem_used;
      msd.mem_max     = my_mem_max;
      msd.mem_percent = (100 * msd.mem_used) / msd.mem_max;

    /* memory info changed - update things */
    return 1;
  }
  /* nothing new */
  return 0;
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{
  static int prev_cpu_pixels = -1, prev_mem_pixels = -1;
  int        cpu_pixels, mem_pixels;
  int        cpusize, memsize;
  int        x, y;

  int        membox_x, membox_y, membox_w, membox_h;
  int        cpubox_x, cpubox_y, cpubox_w, cpubox_h;

  MBPixbufImage *img_backing = NULL;

  system_memory(); 		/* Update reading */

  cpusize = system_cpu();
  memsize = msd.mem_percent; 

  cpubox_h = membox_h = (mb_pixbuf_img_get_width(ImgIconScaled)/16) * 10;

  cpu_pixels = (cpusize * ( cpubox_h) )/ 100 ;
  mem_pixels = (memsize * ( membox_h) )/ 100 ;

  if ((cpu_pixels == prev_cpu_pixels && mem_pixels == prev_mem_pixels))
    return;

  img_backing = mb_tray_app_get_background (app, pb);

  mb_pixbuf_img_composite(pb, img_backing, ImgIconScaled, 0, 0);

  cpubox_x = (mb_pixbuf_img_get_width(img_backing)/4) - (mb_pixbuf_img_get_width(img_backing)/16);
  cpubox_y = cpubox_x;
  cpubox_w = (mb_pixbuf_img_get_width(img_backing)/16) * 2;

  membox_x = ((mb_pixbuf_img_get_width(img_backing)/4) * 3) - (mb_pixbuf_img_get_width(img_backing)/16);
  membox_y = cpubox_y;
  membox_w = cpubox_w;

  /* clear boxes */

  for ( y = membox_y; y < membox_y + membox_h; y++) 
    for ( x = 0; x < membox_w; x++)
      {
	  mb_pixbuf_img_plot_pixel(pb, img_backing, membox_x + x, y, 
				   0x66, 0x66, 0x66);

	  mb_pixbuf_img_plot_pixel(pb, img_backing, cpubox_x + x, y, 
				   0x66, 0x66, 0x66);
      }

  if (cpusize > 0)
    { 
      for ( y = cpubox_h; y > cpubox_h - cpu_pixels;  y--) 
	for ( x = cpubox_x; x < cpubox_x + cpubox_w; x++)
	  mb_pixbuf_img_plot_pixel(pb, img_backing, x, y + cpubox_y, 
				   0, 0xff, 0);
    }

  if (memsize > 0)
    { 
      for ( y =  membox_h; y > membox_h - mem_pixels; y--) 
	for ( x = membox_x; x < membox_x + membox_w; x++)
	  mb_pixbuf_img_plot_pixel(pb, img_backing, x, y + membox_y, 
				   0xff, 0, 0);
    }

  /* XXX Alert here for low memory  */

  mb_pixbuf_img_render_to_drawable(pb, img_backing, drw, 0, 0);

  mb_pixbuf_img_free( pb, img_backing );

  prev_cpu_pixels = cpu_pixels;
  prev_mem_pixels = mem_pixels;

}

void
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  char tray_msg[256];
  int cpu = system_cpu();

  if (!is_released)
    return;

  sprintf(tray_msg, _("CPU: %i %%, MEMORY: %i %%\n"),cpu, msd.mem_percent);
  mb_tray_app_tray_send_message(app, tray_msg, 5000);
}

void
resize_callback (MBTrayApp *app, int w, int h )
{

 if (ImgIconScaled) mb_pixbuf_img_free(pb, ImgIconScaled);
 if (ImgGraph) mb_pixbuf_img_free(pb, ImgGraph);

 ImgIconScaled = mb_pixbuf_img_scale(pb, ImgIcon, w, h);

}

void 
load_icon(void)
{
 char *icon_path = NULL;
 
 if (ImgIcon) mb_pixbuf_img_free(pb, ImgIcon);

 icon_path = mb_dot_desktop_icon_get_full_path (ThemeName, 
						32, 
						MINISYS_IMG );

 if (icon_path == NULL 
     || !(ImgIcon = mb_pixbuf_img_new_from_file(pb, icon_path)))
    {
      fprintf(stderr, "miniapm: failed to load icon %s\n", MINISYS_IMG);
      exit(1);
    }

 free(icon_path);

 return;

}

void 
theme_callback (MBTrayApp *app, char *theme_name)
{
  if (!theme_name) return;
  if (ThemeName) free(ThemeName);
  ThemeName = strdup(theme_name);
  load_icon(); 	     
  resize_callback (app, mb_tray_app_width(app), mb_tray_app_width(app) );
}

void
timeout_callback ( MBTrayApp *app )
{
  mb_tray_app_repaint (app);
}

int
main( int argc, char *argv[])
{
  MBTrayApp *app = NULL;
  struct timeval tv;

  int i;
  u_int64_t load = 0, total = 0;

#if ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, DATADIR "/locale");
  bind_textdomain_codeset (PACKAGE, "UTF-8");
  textdomain (PACKAGE);
#endif

  IsKernel26 = check_if_kernel_2_6();

  app = mb_tray_app_new ( _("CPU/Mem Monitor"),
			  resize_callback,
			  paint_callback,
			  &argc,
			  &argv );  

   msd.samples = 16;
   
   if (msd.load) {
      load = msd.load[msd.loadIndex];
      free(msd.load);
   }
   
   if (msd.total) {
      total = msd.total[msd.loadIndex];
      free(msd.total);
   }
   
   msd.loadIndex = 0;
   msd.load = malloc(msd.samples * sizeof(u_int64_t));
   msd.total = malloc(msd.samples * sizeof(u_int64_t));
   for (i = 0; i < msd.samples; i++) {
      msd.load[i] = load;
      msd.total[i] = total;
   }
  
  pb = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
		     mb_tray_app_xscreen(app));
   
  memset(&tv,0,sizeof(struct timeval));
  tv.tv_usec = 400000;

  mb_tray_app_set_theme_change_callback (app, theme_callback );

  mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 

  mb_tray_app_set_button_callback (app, button_callback );
  
  load_icon();

  mb_tray_app_set_icon(app, pb, ImgIcon);
  
  mb_tray_app_main (app);
   
   return 1;
}
