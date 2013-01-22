/* miniapm - A tiny battery monitor

   Copyright 2002 Matthew Allum

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.
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

#ifdef USE_LIBSN
#define SN_API_NOT_YET_FROZEN 1
#include <libsn/sn.h>
#endif 

#ifdef HAVE_APM_H 		/* Linux */
#include <apm.h>
#endif

#ifdef HAVE_APMVAR_H		/* *BSD  */
#include <err.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <machine/apmvar.h>

#define APMDEV "/dev/apm"

enum apm_state {
    NORMAL,
    SUSPENDING,
    STANDING_BY
};

struct apm_reply {
    int vno;
    enum apm_state newstate;
    struct apm_power_info batterystate;
};

#endif

#define TIME_LEFT  0
#define PERCENTAGE 1
#define AC_POWER   2

#define WIN_WIDTH  8
#define WIN_HEIGHT 14

#define CLOCK_DISP 1
#define BATT_DISP  0

#ifndef AC_LINE_STATUS_ON
#define AC_LINE_STATUS_ON 1
#endif

#ifdef MB_HAVE_PNG
#define MINIAPM_IMG     "miniapm.png"
#define MINIAPM_PWR_IMG "miniapm-power.png"
#else
#define MINIAPM_IMG     "miniapm.xpm"
#define MINIAPM_PWR_IMG "miniapm-power.xpm"
#endif

#define CONTEXT_APP        "/usr/bin/gpe-info"
#define CONTEXT_APP_ARGS   "battery"
#define CONTEXT_APP_WANT_SN 1

#define CHECK_TIME_SLICE  2 	/* 2 seconds between battery checks */

static int apm_vals[3];
static MBPixbuf *pb;
static MBPixbufImage *img_icon = NULL, *img_icon_scaled = NULL,
  *img_icon_pwr = NULL, *img_icon_pwr_scaled = NULL; 


static int time_left_alerts[] = { 0, 2, 5, 10, 20 }; /* times to alert on */
static int time_left_idx = 4;

static Bool ac_power = False;

static char* ThemeName = NULL;

static int last_percentage, last_ac; 

#ifdef USE_LIBSN

static SnDisplay *sn_dpy;

static void 
sn_activate(char *name, char *exec_str)
{
  SnLauncherContext *context;
  pid_t child_pid = 0;

  context = sn_launcher_context_new (sn_dpy, 0);
  
  sn_launcher_context_set_name (context, name);
  sn_launcher_context_set_binary_name (context, exec_str);
  
  sn_launcher_context_initiate (context, "monoluanch launch", exec_str,
				CurrentTime);

  switch ((child_pid = fork ()))
    {
    case -1:
      fprintf (stderr, "Fork failed\n" );
      break;
    case 0:
      sn_launcher_context_setup_child_process (context);
      mb_exec(exec_str);
      fprintf (stderr, "Failed to exec %s \n", exec_str);
      _exit (1);
      break;
    }
}

#endif

#ifdef USE_ACPI_LINUX

#define ACPI_PREFIX	"/proc/acpi"
#define BAT_PREFIX	"battery/BAT0"
#define AC_PREFIX	"ac_adapter/AC0"

/* ACPI sometimes reports incorrect values, especially 
 * shortly after unplugging the AC power. Tweak this for 
 * your hardware. 
 * See /proc/acpi/battery/BAT0/state:"present rate" for
 * "normal" values on your system. */
#define	ACPI_RATE_MIN	300
#define	ACPI_RATE_MAX	5000

struct avg_consumption {
	unsigned long avg_values[100];
	int avg_index;
	int avg_records;
	int countdown;
};

static struct avg_consumption a;

static int
read_apm(int *values)
{
	enum ac_state { AC_UNKNOWN = -1, AC_OFFLINE = 0, AC_ONLINE = 1 };
	enum bat_state { BAT_UNKNOWN = -1, BAT_CHARGING = 1, BAT_DISCHARGING = 2 };

	struct entry {
		char *key;
		char *value;
		char *unit;
	};

	struct acpi_status {
		enum ac_state	ac_state;
		enum bat_state	bat_state;
		unsigned long	design_capacity;
		unsigned long	remaining_capacity;
		unsigned long	present_rate;
	};

	static int
	line_parse(char *line, struct entry *e)
	{
		char *ptr;
		
		ptr = strchr(line, ':');
		if (!ptr)
			return -1;
		e->key = line;
		*ptr = 0;
		while (*++ptr && *ptr == ' ');
		e->value = ptr;
		while (*++ptr && *ptr != '\n' && *ptr != ' ');
		*ptr = 0;
		if (*++ptr) {
			e->unit = ptr;
			while (*++ptr && *ptr != '\n');
			*ptr = 0;
		} else
			e->unit = 0;
	
		return 0;
	}

	FILE *f;
	char line[1024];
	struct entry e;
	struct acpi_status s;

	memset (&s, 0, sizeof(s));

	f = fopen(ACPI_PREFIX "/" AC_PREFIX "/state", "r");
	if (f) {
		while (fgets(line, sizeof(line), f)) {
			if (line_parse(line, &e) < 0)
				continue;
			if (strcmp(e.key, "state") == 0) {
				if(strcmp(e.value, "on-line") == 0) 
					s.ac_state = AC_ONLINE;
				else if (strcmp(e.value, "off-line") == 0)
					s.ac_state = AC_OFFLINE;
				else 
					s.ac_state = AC_UNKNOWN;
			}
		}
		fclose(f);
	}

	f = fopen(ACPI_PREFIX "/" BAT_PREFIX "/info", "r");
	if (f) {
		while (fgets(line, sizeof(line), f)) {
			if (line_parse(line, &e) < 0)
				continue;
			if (strcmp(e.key, "design capacity") == 0)
				sscanf(e.value, "%lu", &s.design_capacity);
		}
		fclose(f);
	}

	f = fopen(ACPI_PREFIX "/" BAT_PREFIX "/state", "r");
	if (f) {
		while (fgets(line, sizeof(line), f)) {
			if (line_parse(line, &e) < 0)
				continue;
			if (strcmp(e.key, "remaining capacity") == 0)
				sscanf(e.value, "%lu", &s.remaining_capacity);
			else if (strcmp(e.key, "charging state") == 0) {
				if (strcmp(e.value, "charging") == 0)
					s.bat_state = BAT_CHARGING;
				else if (strcmp(e.value, "discharging") == 0)
					s.bat_state = BAT_DISCHARGING;
				else
					s.bat_state = BAT_UNKNOWN;
			} else if (strcmp(e.key, "present rate") == 0)
				sscanf(e.value, "%lu", &s.present_rate);
		}
		fclose(f);
	}

	/* Don't estimate TIME_LEFT on current power consumption. Instead 
	 * compute some average and ignore too low/high values.  */
	values[TIME_LEFT] = 0;
	if (s.bat_state != BAT_DISCHARGING && !a.countdown) {
		memset(&a, 0, sizeof(a));
		a.countdown = 10;
	}
	else if (s.bat_state == BAT_DISCHARGING && s.present_rate) {
		int num_values = sizeof(a.avg_values)/sizeof(a.avg_values[0]);
		unsigned long sum = 0;
		int i;

		if (a.countdown) {
			a.countdown--;
			goto bail_out;
		}

		if (s.present_rate < ACPI_RATE_MIN || s.present_rate > ACPI_RATE_MAX) {
			goto bail_out;
		}

		a.avg_values[a.avg_index++] = s.present_rate;
		a.avg_index %= num_values;
		if (a.avg_records < num_values)
			a.avg_records++;
		if (a.avg_records > 10) {
			for (i=0; i<a.avg_records; i++)
				sum += a.avg_values[i];
			values[TIME_LEFT] = s.remaining_capacity*60/(sum/a.avg_records);
		}
bail_out:
		sum = 0; /* shut up gcc */
	}
	
	values[PERCENTAGE] = s.design_capacity ? s.remaining_capacity*100/s.design_capacity : 0;
	values[AC_POWER] = s.ac_state;

#if 0
	printf("AC status: %d\n", values[AC_POWER]);
	printf("Level:     %d%%\n", values[PERCENTAGE]);
	printf("Remaining: %d sec\n", values[TIME_LEFT]);
#endif
	return 1;
}
#else
#ifdef HAVE_APM_H

static int 
read_apm(int *values)
{
  /* add stat function here ? */
 
  apm_info info;
  apm_read(&info);

  values[TIME_LEFT] = info.battery_time;
  values[PERCENTAGE] = info.battery_percentage;
  values[AC_POWER] = info.ac_line_status;

  return 1;
}

#else  /* BSD */

static int 
read_apm(int *values)
{
  int fd;
  struct apm_reply reply;
  struct apm_power_info *api = &reply.batterystate;

  /* open the device directly and get status */
  fd = open(APMDEV, O_RDONLY);
  if (fd == -1) 
	return 0;
  memset(&reply, 0, sizeof(reply));
  if (ioctl(fd, APM_IOC_GETPOWER, &reply.batterystate) == -1) {
        close (fd);
  return 0;
  }
  close (fd);

  values[TIME_LEFT]  = api->minutes_left;
  values[PERCENTAGE] = api->battery_life;
  values[AC_POWER]   = api->ac_state;
 
  return 1;
}

#endif
#endif /* Use ACPI */

void 
fork_exec(char *cmd)
{
  switch (fork())
    {
    case 0:
      setpgid(0, 0); /* Stop us killing child */
      mb_exec(cmd);
      fprintf(stderr, "minitime: Failed to Launch '%s'", cmd);
      exit(1);
    case -1:
      fprintf(stderr, "minitime: Failed to Launch '%s'", cmd);
      break;
    }
}

void
paint_callback (MBTrayApp *app, Drawable drw )
{
  int      power_pixels = 0;
  unsigned char r = 0, g = 0, b = 0;
  int      x, y;
  int      bar_width, bar_height, bar_x, bar_y;

  MBPixbufImage *img_backing = NULL;

  while (!read_apm(apm_vals))
    usleep(50000L);

  if (last_percentage == apm_vals[PERCENTAGE] && last_ac == apm_vals[AC_POWER])
    return;

  img_backing = mb_tray_app_get_background (app, pb);

  /* we assume width = height */
  bar_width  = (mb_pixbuf_img_get_width(img_backing)/16) * 10;
  bar_height = (mb_pixbuf_img_get_width(img_backing)/16) * 2;
  bar_y      = (mb_pixbuf_img_get_width(img_backing)/4)*3;
  bar_x      = (mb_pixbuf_img_get_width(img_backing)/4) - (mb_pixbuf_img_get_width(img_backing)/16);

  /*
  printf("bar: %ix%i +%i+%i (%i)\n", bar_width, bar_height, bar_x, bar_y, mb_pixbuf_img_get_width(img_backing) );
  */  

  if (apm_vals[PERCENTAGE] <= 0 || apm_vals[PERCENTAGE] > 100)
    { 
      r = 0x66; g = 0xff; b = 0x33; ac_power = True; 
      apm_vals[PERCENTAGE] = -1;
    }
  else if (apm_vals[PERCENTAGE] <= 25)
    { r = 0xff; g = 0; b = 0; }
  else if (apm_vals[PERCENTAGE] <= 50)
    { r = 0xff; g = 0x99; b = 0x33; }
  else if (apm_vals[PERCENTAGE] <= 100)
    { r = 0x66; g = 0xff; b = 0x33; }

  if (apm_vals[AC_POWER] == AC_LINE_STATUS_ON) 
    ac_power = True;
  else 
    ac_power = False;

  if (ac_power)
    mb_pixbuf_img_composite(pb, img_backing, img_icon_pwr_scaled, 0, 0);
  else
    mb_pixbuf_img_composite(pb, img_backing, img_icon_scaled, 0, 0);

  /* Clear out bar first */
  for ( y = bar_y; y < bar_y + bar_height; y++) 
    for ( x = bar_x; x < bar_x + bar_width; x++)
      if (ac_power)
	{
	  mb_pixbuf_img_plot_pixel(pb, img_backing, x, y, 0xff, 0xff, 0);
	}
      else
	{
	  mb_pixbuf_img_plot_pixel(pb, img_backing, x, y, 0, 0, 0);
	}

  if (apm_vals[PERCENTAGE] > 0)
    { 
      power_pixels = (apm_vals[PERCENTAGE] * ( bar_width) )/ 100 ;

      for ( y = bar_y; y < bar_y + bar_height; y++) 
	for ( x = bar_x; x < bar_x + power_pixels + 1; x++)
	  mb_pixbuf_img_plot_pixel(pb, img_backing, x, y, r, g, b);
    }

  /*dont do this if time values arent supported by this bios*/
  if (apm_vals[TIME_LEFT] > 0 )
    {
    /* Bubble alerts  */
    if ((time_left_idx > 0) 
        && !ac_power
       && apm_vals[PERCENTAGE] > 0
        /* && apm_vals[TIME_LEFT]  > 0 XXX Less strict */ 
        && (apm_vals[TIME_LEFT] < time_left_alerts[time_left_idx]))
      {
        char tray_msg[256];
        sprintf(tray_msg, 
   	        _("Battery power very low !\n\nTime Left: %.2i minutes"), 
	        time_left_alerts[time_left_idx]);
        mb_tray_app_tray_send_message(app, tray_msg, 0);
        time_left_idx--;
      }
    else if (time_left_idx < 4 
  	     && apm_vals[TIME_LEFT] > time_left_alerts[time_left_idx+1])
      {
        time_left_idx++;
      }
    }

  mb_pixbuf_img_render_to_drawable(pb, img_backing, drw, 0, 0);

  mb_pixbuf_img_free( pb, img_backing );

  last_percentage = apm_vals[PERCENTAGE];
  last_ac         = apm_vals[AC_POWER];

}

void
resize_callback (MBTrayApp *app, int w, int h )
{

 if (img_icon_scaled) mb_pixbuf_img_free(pb, img_icon_scaled);
 if (img_icon_pwr_scaled) mb_pixbuf_img_free(pb, img_icon_pwr_scaled);

 last_percentage = -1; last_ac = -1;

 img_icon_scaled = mb_pixbuf_img_scale(pb, img_icon, w, h);
 img_icon_pwr_scaled = mb_pixbuf_img_scale(pb, img_icon_pwr, w, h);

}

void 
load_icon(void)
{
 char *icon_path = NULL;
 
 printf("%s() called", __func__);

 last_percentage = -1; last_ac = -1;

 if (img_icon) mb_pixbuf_img_free(pb, img_icon);
 if (img_icon_pwr) mb_pixbuf_img_free(pb, img_icon_pwr);

 icon_path = mb_dot_desktop_icon_get_full_path (ThemeName, 
						32, 
						MINIAPM_IMG );

 if (icon_path == NULL 
     || !(img_icon = mb_pixbuf_img_new_from_file(pb, icon_path)))
    {
      fprintf(stderr, "mb-applet-battery: failed to load icon %s\n", MINIAPM_IMG);
      exit(1);
    }

  free(icon_path);

  icon_path = mb_dot_desktop_icon_get_full_path (ThemeName, 
						 32, 
						 MINIAPM_PWR_IMG );

 if (icon_path == NULL 
     || !(img_icon_pwr = mb_pixbuf_img_new_from_file(pb, icon_path)))
    {
      fprintf(stderr, "mb-applet-battery: failed to load icon %s\n", MINIAPM_PWR_IMG);
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
button_callback (MBTrayApp *app, int x, int y, Bool is_released )
{
  char tray_msg[256];

  if (!is_released) return;

  if (apm_vals[AC_POWER] == AC_LINE_STATUS_ON)
    {
      if (apm_vals[PERCENTAGE] > 0 
	  && apm_vals[PERCENTAGE] <= 100 )
	sprintf(tray_msg, _("AC Connected\nCharging: %.2i %%\n")
		, apm_vals[PERCENTAGE]);
      else
	sprintf(tray_msg, _("AC Connected\nFully charged.\n"));
    } else {
      if (apm_vals[PERCENTAGE] > 0 
	  && apm_vals[PERCENTAGE] <= 100 )
        {
	  if (apm_vals[TIME_LEFT] > 0 )
	    {
	      sprintf(tray_msg, 
		      _("Battery Power\nJuice %.2i %%\nTime left: %.2i h. %.2i min.\n"), apm_vals[PERCENTAGE], (apm_vals[TIME_LEFT]/60), (apm_vals[TIME_LEFT]%60));
	    }
          else
	    {
	      sprintf(tray_msg, 
	   	      _("Battery Power\nJuice %.2i %%\n"), apm_vals[PERCENTAGE]);
	    }

        }
      else sprintf(tray_msg, _("Battery Power\n Device read error.\n"));
    }
  mb_tray_app_tray_send_message(app, tray_msg, 5000);

}

void
timeout_callback ( MBTrayApp *app )
{
  mb_tray_app_repaint (app);
}

void
context_callback ( MBTrayApp *app )
{
#ifdef USE_LIBSN
  if (CONTEXT_APP_WANT_SN)
    {
      sn_activate(CONTEXT_APP, CONTEXT_APP " " CONTEXT_APP_ARGS);      
      return;
    }
#endif

  fork_exec(CONTEXT_APP " " CONTEXT_APP_ARGS);
}

static Bool 
file_exists(char *filename)
{
  struct stat st; 		/* XXX Should probably check if exe too */
  if (stat(filename, &st)) return False;
  return True;
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


  app = mb_tray_app_new ( _("Battery Monitor"),
			  resize_callback,
			   paint_callback,
			  &argc,
			  &argv );  
  
   pb = mb_pixbuf_new(mb_tray_app_xdisplay(app), 
		      mb_tray_app_xscreen(app));
   
   memset(&tv,0,sizeof(struct timeval));
   tv.tv_sec = CHECK_TIME_SLICE;

   mb_tray_app_set_timeout_callback (app, timeout_callback, &tv); 
   
   mb_tray_app_set_button_callback (app, button_callback );
   
   mb_tray_app_set_theme_change_callback (app, theme_callback );
   
   load_icon();

   mb_tray_app_set_icon(app, pb, img_icon);

  if (file_exists(CONTEXT_APP))
    {
      mb_tray_app_set_context_info (app, _("More info")); 

      mb_tray_app_set_context_callback (app, context_callback); 
    }

#ifdef USE_LIBSN
  sn_dpy = sn_display_new (mb_tray_app_xdisplay(app), NULL, NULL);
#endif
   
   mb_tray_app_main (app);

   return 1;
}


