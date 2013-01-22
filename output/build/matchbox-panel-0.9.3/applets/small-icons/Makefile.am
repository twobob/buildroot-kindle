pngs = \
broken-0.png  minisys.png        no-link-0.png     signal-61-80.png  \
mbmenu.png    minitime.png       signal-1-40.png   signal-81-100.png \
mbterm.png    signal-41-60.png   miniapm-power.png miniapm.png

xpms = \
mbterm.xpm    minisys.xpm        no-link-0.xpm      signal-41-60.xpm   \
broken-0.xpm  signal-61-80.xpm   miniapm-power.xpm  miniapm.xpm       \
mbmenu.xpm    signal-1-40.xpm    signal-81-100.xpm  minitime.xpm

EXTRA_DIST = $(xpms) $(pngs)

if WANT_PNGS
imgs = $(pngs)
else
imgs = $(xpms)
endif

pixmapsdir = $(datadir)/pixmaps/

if WANT_SMALL_ICONS
pixmaps_DATA =  $(imgs)
else
pixmaps_DATA =
endif