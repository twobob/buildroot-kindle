SUBDIRS = dotdesktop icons small-icons

EXTRA_DIST = mb-applet-battery.c mb-applet-xterm-wrapper.sh

PREFIXDIR  = $(prefix)
PKGDATADIR = $(datadir)/matchbox
DATADIR    = $(datadir)

INCLUDES = -DDATADIR=\"$(DATADIR)\" -DPKGDATADIR=\"$(PKGDATADIR)\" -DPREFIX=\"$(PREFIXDIR)\" $(LIBMB_CFLAGS) $(SN_CFLAGS)

if WANT_APM
bin_PROGRAMS =                                                    \
  mb-applet-clock    mb-applet-menu-launcher  mb-applet-battery   \
  mb-applet-launcher mb-applet-system-monitor 

  mb_applet_battery_SOURCES   =      mb-applet-battery.c
  mb_applet_battery_LDADD   =        $(LIBMB_LIBS) $(SN_LIBS) -lapm  

else
bin_PROGRAMS =                                 \
  mb-applet-clock    mb-applet-menu-launcher    \
  mb-applet-launcher mb-applet-system-monitor
endif

if WANT_WIFI
  bin_PROGRAMS += mb-applet-wireless
endif

if WANT_ACPI
  bin_PROGRAMS += mb-applet-battery
  mb_applet_battery_SOURCES   =      mb-applet-battery.c
  mb_applet_battery_LDADD   =        $(LIBMB_LIBS) $(SN_LIBS)
endif


LDADD = $(LIBMB_LIBS) $(SN_LIBS)

mb_applet_clock_SOURCES =          mb-applet-clock.c
mb_applet_menu_launcher_SOURCES =  mb-applet-menu-launcher.c
mb_applet_wireless_SOURCES =       mb-applet-wireless.c
mb_applet_wireless_LDADD   =       $(LIBMB_LIBS) $(SN_LIBS) $(WIRELESS_LIBS)
mb_applet_launcher_SOURCES =       mb-applet-launcher.c  
mb_applet_system_monitor_SOURCES = mb-applet-system-monitor.c 


bin_SCRIPTS = mb-applet-xterm-wrapper.sh
