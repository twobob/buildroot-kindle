/*
   (c) Copyright 2001-2011  The world wide DirectFB Open Source Community (directfb.org)
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

#include <config.h>

#include "CoreInputDevice.h"

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <core/input.h>

D_DEBUG_DOMAIN( DirectFB_CoreInputDevice, "DirectFB/CoreInputDevice", "DirectFB CoreInputDevice" );

/*********************************************************************************************************************/


DFBResult
IInputDevice_Real__SetKeymapEntry(
                    CoreInputDevice                           *obj,
                    s32                                        key_code,
                    const DFBInputDeviceKeymapEntry           *entry
)
{
    D_DEBUG_AT( DirectFB_CoreInputDevice, "%s()\n", __FUNCTION__ );

    return dfb_input_device_set_keymap_entry( obj, key_code, entry );
}

DFBResult
IInputDevice_Real__ReloadKeymap(
                    CoreInputDevice                            *obj
)
{
    D_DEBUG_AT( DirectFB_CoreInputDevice, "%s()\n", __FUNCTION__ );

    return dfb_input_device_reload_keymap( obj );
}

