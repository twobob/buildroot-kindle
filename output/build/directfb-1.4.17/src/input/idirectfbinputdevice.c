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

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

#include <fusion/reactor.h>

#include <directfb.h>

#include <core/CoreInputDevice.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/input.h>

#include <misc/util.h>
#include <direct/interface.h>
#include <direct/mem.h>

#include "idirectfbinputdevice.h"
#include "idirectfbinputbuffer.h"


/*
 * processes an event, updates device state
 * (funcion is added to the event listeners)
 */
static ReactionResult
IDirectFBInputDevice_React( const void *msg_data,
                            void       *ctx );

/*
 * private data struct of IDirectFBInputDevice
 */
typedef struct {
     int                         ref;               /* reference counter */
     CoreInputDevice            *device;            /* pointer to input core
                                                       device struct*/

     int                         axis[DIAI_LAST+1]; /* position of all axes */
     DFBInputDeviceKeyState      keystates[DIKI_NUMBER_OF_KEYS];
                                                    /* state of all keys */
     DFBInputDeviceModifierMask  modifiers;         /* bitmask reflecting the
                                                       state of the modifier
                                                       keys */
     DFBInputDeviceLockState     locks;             /* bitmask reflecting the
						       state of the key locks */
     DFBInputDeviceButtonMask    buttonmask;        /* bitmask reflecting the
                                                       state of the buttons */

     DFBInputDeviceDescription   desc;              /* device description */

     Reaction                    reaction;
} IDirectFBInputDevice_data;



static void
IDirectFBInputDevice_Destruct( IDirectFBInputDevice *thiz )
{
     IDirectFBInputDevice_data *data = (IDirectFBInputDevice_data*)thiz->priv;

     dfb_input_detach( data->device, &data->reaction );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

static DirectResult
IDirectFBInputDevice_AddRef( IDirectFBInputDevice *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     data->ref++;

     return DR_OK;
}

static DirectResult
IDirectFBInputDevice_Release( IDirectFBInputDevice *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (--data->ref == 0)
          IDirectFBInputDevice_Destruct( thiz );

     return DR_OK;
}

static DFBResult
IDirectFBInputDevice_GetID( IDirectFBInputDevice *thiz,
                            DFBInputDeviceID     *id )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!id)
          return DFB_INVARG;

     *id = dfb_input_device_id( data->device );

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_CreateEventBuffer( IDirectFBInputDevice  *thiz,
                                        IDirectFBEventBuffer **buffer )
{
     IDirectFBEventBuffer *b;

     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     DIRECT_ALLOCATE_INTERFACE( b, IDirectFBEventBuffer );

     IDirectFBEventBuffer_Construct( b, NULL, NULL );

     IDirectFBEventBuffer_AttachInputDevice( b, data->device );

     *buffer = b;

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_AttachEventBuffer( IDirectFBInputDevice  *thiz,
                                        IDirectFBEventBuffer  *buffer )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     return IDirectFBEventBuffer_AttachInputDevice( buffer, data->device );
}

static DFBResult
IDirectFBInputDevice_DetachEventBuffer( IDirectFBInputDevice  *thiz,
                                        IDirectFBEventBuffer  *buffer )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     return IDirectFBEventBuffer_DetachInputDevice( buffer, data->device );
}

static DFBResult
IDirectFBInputDevice_GetDescription( IDirectFBInputDevice      *thiz,
                                     DFBInputDeviceDescription *desc )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!desc)
          return DFB_INVARG;

     *desc = data->desc;

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_GetKeymapEntry( IDirectFBInputDevice      *thiz,
                                     int                        keycode,
                                     DFBInputDeviceKeymapEntry *entry )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!entry)
          return DFB_INVARG;

     if (data->desc.min_keycode < 0 || data->desc.max_keycode < 0)
          return DFB_UNSUPPORTED;

     if (keycode < data->desc.min_keycode ||
         keycode > data->desc.max_keycode)
          return DFB_INVARG;

     return dfb_input_device_get_keymap_entry( data->device, keycode, entry );
}

static DFBResult
IDirectFBInputDevice_SetKeymapEntry( IDirectFBInputDevice      *thiz,
                                     int                        keycode,
                                     DFBInputDeviceKeymapEntry *entry )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!entry)
          return DFB_INVARG;

     if (data->desc.min_keycode < 0 || data->desc.max_keycode < 0)
          return DFB_UNSUPPORTED;

     if (keycode < data->desc.min_keycode ||
         keycode > data->desc.max_keycode)
          return DFB_INVARG;

     return CoreInputDevice_SetKeymapEntry( data->device, keycode, entry );
}

static DFBResult
IDirectFBInputDevice_LoadKeymap ( IDirectFBInputDevice          *thiz,
                                  char                          *filename )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!filename)
          return DFB_INVARG;

     return dfb_input_device_load_keymap( data->device, filename );
}

static DFBResult
IDirectFBInputDevice_GetKeyState( IDirectFBInputDevice        *thiz,
                                  DFBInputDeviceKeyIdentifier  key_id,
                                  DFBInputDeviceKeyState      *state )
{
     unsigned int index = key_id - DFB_KEY(IDENTIFIER, 0);
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!state || index >= DIKI_NUMBER_OF_KEYS)
          return DFB_INVARG;

     *state = data->keystates[index];

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_GetModifiers( IDirectFBInputDevice       *thiz,
                                   DFBInputDeviceModifierMask *modifiers )
{
     DFBResult            ret;
     CoreInputDeviceState state;

     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!modifiers)
          return DFB_INVARG;

     ret = dfb_input_device_get_state( data->device, &state );
     if (ret)
          return ret;

     *modifiers = state.modifiers_l | state.modifiers_r;

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_GetLockState( IDirectFBInputDevice    *thiz,
                                   DFBInputDeviceLockState *locks )
{
     DFBResult            ret;
     CoreInputDeviceState state;

     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!locks)
          return DFB_INVARG;

     ret = dfb_input_device_get_state( data->device, &state );
     if (ret)
          return ret;

     *locks = state.locks;

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_GetButtons( IDirectFBInputDevice     *thiz,
                                 DFBInputDeviceButtonMask *buttons )
{
     DFBResult            ret;
     CoreInputDeviceState state;

     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!buttons)
          return DFB_INVARG;

     ret = dfb_input_device_get_state( data->device, &state );
     if (ret)
          return ret;

     *buttons = state.buttons;

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_GetButtonState( IDirectFBInputDevice           *thiz,
                                     DFBInputDeviceButtonIdentifier  button,
                                     DFBInputDeviceButtonState      *ret_state)
{
     DFBResult            ret;
     CoreInputDeviceState state;

     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     ret = dfb_input_device_get_state( data->device, &state );
     if (ret)
          return ret;

     if (!ret_state || (int)button < DIBI_FIRST || button > DIBI_LAST)
          return DFB_INVARG;

     *ret_state = (state.buttons & (1 << button)) ? DIBS_DOWN : DIBS_UP;

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_GetAxis( IDirectFBInputDevice         *thiz,
                              DFBInputDeviceAxisIdentifier  axis,
                              int                          *pos )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!pos || (int)axis < DIAI_FIRST || axis > DIAI_LAST)
          return DFB_INVARG;

     *pos = data->axis[axis];

     return DFB_OK;
}

static DFBResult
IDirectFBInputDevice_GetXY( IDirectFBInputDevice *thiz,
                            int                  *x,
                            int                  *y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBInputDevice)

     if (!x && !y)
          return DFB_INVARG;

     if (x)
          *x = data->axis[DIAI_X];

     if (y)
          *y = data->axis[DIAI_Y];

     return DFB_OK;
}

DFBResult
IDirectFBInputDevice_Construct( IDirectFBInputDevice *thiz,
                                CoreInputDevice      *device )
{
     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBInputDevice)

     data->ref    = 1;
     data->device = device;

     dfb_input_device_description( device, &data->desc );

     dfb_input_attach( data->device, IDirectFBInputDevice_React,
                       data, &data->reaction );

     thiz->AddRef = IDirectFBInputDevice_AddRef;
     thiz->Release = IDirectFBInputDevice_Release;
     thiz->GetID = IDirectFBInputDevice_GetID;
     thiz->GetDescription = IDirectFBInputDevice_GetDescription;
     thiz->GetKeymapEntry = IDirectFBInputDevice_GetKeymapEntry;
     thiz->SetKeymapEntry = IDirectFBInputDevice_SetKeymapEntry;
     thiz->LoadKeymap = IDirectFBInputDevice_LoadKeymap;
     thiz->CreateEventBuffer = IDirectFBInputDevice_CreateEventBuffer;
     thiz->AttachEventBuffer = IDirectFBInputDevice_AttachEventBuffer;
     thiz->DetachEventBuffer = IDirectFBInputDevice_DetachEventBuffer;
     thiz->GetKeyState = IDirectFBInputDevice_GetKeyState;
     thiz->GetModifiers = IDirectFBInputDevice_GetModifiers;
     thiz->GetLockState = IDirectFBInputDevice_GetLockState;
     thiz->GetButtons = IDirectFBInputDevice_GetButtons;
     thiz->GetButtonState = IDirectFBInputDevice_GetButtonState;
     thiz->GetAxis = IDirectFBInputDevice_GetAxis;
     thiz->GetXY = IDirectFBInputDevice_GetXY;

     return DFB_OK;
}


/* internals */

static ReactionResult
IDirectFBInputDevice_React( const void *msg_data,
                            void       *ctx )
{
     const DFBInputEvent       *evt  = msg_data;
     IDirectFBInputDevice_data *data = ctx;
     unsigned int               index;

     if (evt->flags & DIEF_MODIFIERS)
          data->modifiers = evt->modifiers;
     if (evt->flags & DIEF_LOCKS)
          data->locks = evt->locks;
     if (evt->flags & DIEF_BUTTONS)
          data->buttonmask = evt->buttons;

     switch (evt->type) {
          case DIET_KEYPRESS:
               index = evt->key_id - DFB_KEY(IDENTIFIER, 0);
               if (index < DIKI_NUMBER_OF_KEYS)
                    data->keystates[index] = DIKS_DOWN;
	           break;

          case DIET_KEYRELEASE:
               index = evt->key_id - DFB_KEY(IDENTIFIER, 0);
               if (index < DIKI_NUMBER_OF_KEYS)
                    data->keystates[index] = DIKS_UP;
               break;

          case DIET_AXISMOTION:
               if (evt->flags & DIEF_AXISREL)
                    data->axis[evt->axis] += evt->axisrel;
               if (evt->flags & DIEF_AXISABS)
                    data->axis[evt->axis] = evt->axisabs;
               break;

          default:
               D_DEBUG( "DirectFB/IDirectFBInputDevice: Unknown event type detected (0x%x), skipping!\n", evt->type );
     }

     return RS_OK;
}

