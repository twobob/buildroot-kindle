
dfiles =                                                        \
      mb-applet-battery.desktop         mb-applet-wireless.desktop \
      mb-applet-clock.desktop           mb-launcher-term.desktop   \
      mb-applet-menu-launcher.desktop   mb-applet-system-monitor.desktop

files =                                                         \
      mb-applet-clock.desktop           mb-launcher-term.desktop   \
      mb-applet-menu-launcher.desktop   mb-applet-system-monitor.desktop

if WANT_APM
   files += mb-applet-battery.desktop
endif

if WANT_WIFI
   files +=  mb-applet-wireless.desktop 
endif

EXTRA_DIST = $(dfiles)

desktopdir = $(datadir)/applications/
desktop_DATA = $(files)

