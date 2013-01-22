#!/bin/sh

# This just wraps rxvt and xterm so a term gets launched ok.
# rxvt is the preference. This could be much improved
#

if [ -x /usr/X11R6/bin/rxvt ]; then
   exec /usr/X11R6/bin/rxvt;
else
   if [ -x /usr/bin/rxvt ]; then
      exec /usr/bin/rxvt;
   else
      exec xterm; 
   fi
fi 