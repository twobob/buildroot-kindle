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

#ifndef __UNIQUE__TYPES_H__
#define __UNIQUE__TYPES_H__


typedef struct __UniQuE_UniqueContext        UniqueContext;
typedef struct __UniQuE_UniqueDecoration     UniqueDecoration;
typedef struct __UniQuE_UniqueDecorationItem UniqueDecorationItem;
typedef struct __UniQuE_UniqueDevice         UniqueDevice;
typedef struct __UniQuE_UniqueLayout         UniqueLayout;
typedef struct __UniQuE_UniqueWindow         UniqueWindow;

typedef struct __UniQuE_UniqueInputChannel   UniqueInputChannel;
typedef union  __UniQuE_UniqueInputEvent     UniqueInputEvent;
typedef struct __UniQuE_UniqueInputFilter    UniqueInputFilter;
typedef struct __UniQuE_UniqueInputSwitch    UniqueInputSwitch;


typedef struct __UniQuE_StretRegion          StretRegion;


typedef struct __UniQuE_WMData               WMData;
typedef struct __UniQuE_WMShared             WMShared;

#endif

