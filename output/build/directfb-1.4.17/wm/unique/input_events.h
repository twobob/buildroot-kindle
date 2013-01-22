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

#ifndef __UNIQUE__INPUT_EVENTS_H__
#define __UNIQUE__INPUT_EVENTS_H__

#include <directfb.h>

#include <unique/types.h>
#include <unique/device.h>

typedef enum {
     UIET_NONE      = 0x00000000,

     UIET_MOTION    = 0x00000001,
     UIET_BUTTON    = 0x00000002,

     UIET_WHEEL     = 0x00000010,

     UIET_KEY       = 0x00000100,

     UIET_CHANNEL   = 0x00001000,

     UIET_ALL       = 0x00001113
} UniqueInputEventType;


typedef struct {
     UniqueInputEventType               type;

     DFBInputDeviceID                   device_id;

     bool                               press;

     int                                x;
     int                                y;

     DFBInputDeviceButtonIdentifier     button;

     DFBInputDeviceButtonMask           buttons;
} UniqueInputPointerEvent;

typedef struct {
     UniqueInputEventType               type;

     DFBInputDeviceID                   device_id;

     int                                value;
} UniqueInputWheelEvent;

typedef enum {
     UIKF_NONE      = 0x00000000,

     UIKF_REPEAT    = 0x00000001,

     UIKF_ALL       = 0x00000001
} UniqueInputKeyboardEventFlags;

typedef struct {
     UniqueInputEventType               type;

     DFBInputDeviceID                   device_id;

     bool                               press;
     UniqueInputKeyboardEventFlags      flags;

     int                                key_code;
     DFBInputDeviceKeyIdentifier        key_id;
     DFBInputDeviceKeySymbol            key_symbol;

     DFBInputDeviceModifierMask         modifiers;
     DFBInputDeviceLockState            locks;
} UniqueInputKeyboardEvent;

typedef struct {
     UniqueInputEventType               type;

     bool                               selected;

     UniqueDeviceClassIndex             index;

     int                                x;
     int                                y;
} UniqueInputChannelEvent;


union __UniQuE_UniqueInputEvent {
     UniqueInputEventType               type;

     UniqueInputPointerEvent            pointer;
     UniqueInputWheelEvent              wheel;
     UniqueInputKeyboardEvent           keyboard;
     UniqueInputChannelEvent            channel;
};


#endif

