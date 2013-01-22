/*
   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with this library; if not, write to the
   Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.
*/

#ifndef __SYSTEMS_XWINDOW_H__
#define __SYSTEMS_XWINDOW_H__

#include <X11/Xlib.h>    /* fundamentals X datas structures */
#include <X11/Xutil.h>   /* datas definitions for various functions */
#include <X11/keysym.h>  /* for a perfect use of keyboard events */

#include <X11/extensions/XShm.h>
#include <sys/ipc.h>
#include <sys/shm.h>

#include "x11types.h"


typedef struct {
     Display*            display;
     Window              window;
     Screen*             screenptr;
     int                 screennum;
     Visual*             visual;
     GC                  gc;
     XImage*             ximage;
     int                 ximage_offset;
     Colormap            colormap;

     XShmSegmentInfo*    shmseginfo;
     unsigned char*      videomemory;

     char*               virtualscreen;
     int                 videoaccesstype;

     int                 width;
     int                 height;
     int                 depth;
     int                 bpp;

     /* (Null) cursor stuff*/
     Cursor              NullCursor;
} XWindow;

Bool dfb_x11_open_window ( DFBX11 *x11, XWindow** ppXW, int iXPos, int iYPos, int iWidth, int iHeight, DFBSurfacePixelFormat format );
void dfb_x11_close_window( DFBX11 *x11, XWindow* pXW );



#endif /* __SYSTEMS_XWINDOW_H__ */

