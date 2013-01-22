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

#include <directfb.h>

#include <direct/interface.h>
#include <direct/mem.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <voodoo/conf.h>
#include <voodoo/interface.h>
#include <voodoo/manager.h>

#include <idirectfbeventbuffer_requestor.h>

#include "idirectfbwindow_dispatcher.h"


static DFBResult Probe( void );
static DFBResult Construct( IDirectFBWindow  *thiz,
                            IDirectFBWindow  *real,
                            VoodooManager    *manager,
                            VoodooInstanceID  super,
                            void             *arg,
                            VoodooInstanceID *ret_instance );

#include <direct/interface_implementation.h>

DIRECT_INTERFACE_IMPLEMENTATION( IDirectFBWindow, Dispatcher )


/**************************************************************************************************/

static void
IDirectFBWindow_Dispatcher_Destruct( IDirectFBWindow *thiz )
{
     IDirectFBWindow_Dispatcher_data *data;

     D_DEBUG( "%s (%p)\n", __FUNCTION__, thiz );

     data = thiz->priv;

     voodoo_manager_unregister_local( data->manager, data->self );

     data->real->Release( data->real );

     DIRECT_DEALLOCATE_INTERFACE( thiz );
}

/**************************************************************************************************/

static DirectResult
IDirectFBWindow_Dispatcher_AddRef( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     data->ref++;

     return DFB_OK;
}

static DirectResult
IDirectFBWindow_Dispatcher_Release( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     if (--data->ref == 0)
          IDirectFBWindow_Dispatcher_Destruct( thiz );

     return DFB_OK;
}

static DFBResult
IDirectFBWindow_Dispatcher_CreateEventBuffer( IDirectFBWindow       *thiz,
                                              IDirectFBEventBuffer **buffer )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_AttachEventBuffer( IDirectFBWindow       *thiz,
                                              IDirectFBEventBuffer  *buffer )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_DetachEventBuffer( IDirectFBWindow       *thiz,
                                              IDirectFBEventBuffer  *buffer )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_EnableEvents( IDirectFBWindow       *thiz,
                                         DFBWindowEventType     mask )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_DisableEvents( IDirectFBWindow    *thiz,
                                          DFBWindowEventType  mask )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GetID( IDirectFBWindow *thiz,
                                  DFBWindowID     *id )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GetPosition( IDirectFBWindow *thiz,
                                        int             *x,
                                        int             *y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GetSize( IDirectFBWindow *thiz,
                                    int             *width,
                                    int             *height )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GetSurface( IDirectFBWindow   *thiz,
                                       IDirectFBSurface **surface )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetProperty( IDirectFBWindow  *thiz,
                                        const char       *key,
                                        void             *value,
                                        void            **old_value )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GetProperty( IDirectFBWindow  *thiz,
                                        const char       *key,
                                        void            **value )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_RemoveProperty( IDirectFBWindow  *thiz,
                                           const char       *key,
                                           void            **value )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetOptions( IDirectFBWindow  *thiz,
                                       DFBWindowOptions  options )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GetOptions( IDirectFBWindow  *thiz,
                                       DFBWindowOptions *options )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetColorKey( IDirectFBWindow *thiz,
                                        u8               r,
                                        u8               g,
                                        u8               b )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetColorKeyIndex( IDirectFBWindow *thiz,
                                             unsigned int     index )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetOpaqueRegion( IDirectFBWindow *thiz,
                                            int              x1,
                                            int              y1,
                                            int              x2,
                                            int              y2 )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetOpacity( IDirectFBWindow *thiz,
                                       u8               opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GetOpacity( IDirectFBWindow *thiz,
                                       u8              *opacity )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetCursorShape( IDirectFBWindow  *thiz,
                                           IDirectFBSurface *shape,
                                           int               hot_x,
                                           int               hot_y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_RequestFocus( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GrabKeyboard( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_UngrabKeyboard( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GrabPointer( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_UngrabPointer( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_GrabKey( IDirectFBWindow            *thiz,
                                    DFBInputDeviceKeySymbol     symbol,
                                    DFBInputDeviceModifierMask  modifiers )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_UngrabKey( IDirectFBWindow            *thiz,
                                      DFBInputDeviceKeySymbol     symbol,
                                      DFBInputDeviceModifierMask  modifiers )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_Move( IDirectFBWindow *thiz, int dx, int dy )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_MoveTo( IDirectFBWindow *thiz, int x, int y )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_Resize( IDirectFBWindow *thiz,
                                   int              width,
                                   int              height )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_Raise( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetStackingClass( IDirectFBWindow        *thiz,
                                             DFBWindowStackingClass  stacking_class )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_Lower( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_RaiseToTop( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_LowerToBottom( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_PutAtop( IDirectFBWindow *thiz,
                                    IDirectFBWindow *lower )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_PutBelow( IDirectFBWindow *thiz,
                                     IDirectFBWindow *upper )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_Close( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_Destroy( IDirectFBWindow *thiz )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_SetBounds( IDirectFBWindow *thiz,
                                      int x, int y, int w, int h )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}

static DFBResult
IDirectFBWindow_Dispatcher_ResizeSurface( IDirectFBWindow *thiz,
                                          int              width,
                                          int              height )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     D_UNIMPLEMENTED();

     return DFB_UNIMPLEMENTED;
}


/**************************************************************************************************/

static DirectResult
Dispatch_Release( IDirectFBWindow *thiz, IDirectFBWindow *real,
                  VoodooManager *manager, VoodooRequestMessage *msg )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     return thiz->Release( thiz );
}

static DirectResult
Dispatch_CreateEventBuffer( IDirectFBWindow *thiz, IDirectFBWindow *real,
                            VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult                ret;
     IDirectFBEventBuffer       *buffer;
     VoodooInstanceID            instance;
     VoodooMessageParser         parser;
     void                       *requestor;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_ID( parser, instance );
     VOODOO_PARSER_END( parser );

     ret = real->CreateEventBuffer( real, &buffer );
     if (ret)
          return ret;

     ret = voodoo_construct_requestor( manager, "IDirectFBEventBuffer",
                                       instance, buffer, &requestor );
     if (ret)
          buffer->Release( buffer );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_AttachEventBuffer( IDirectFBWindow *thiz, IDirectFBWindow *real,
                            VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult                         ret;
     IDirectFBEventBuffer                *buffer;
     IDirectFBEventBuffer_Requestor_data *buffer_data;
     VoodooInstanceID                     instance;
     VoodooMessageParser                  parser;
     void                                *ptr;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_ID( parser, instance );
     VOODOO_PARSER_END( parser );

     ret = voodoo_manager_lookup_remote( manager, instance, &ptr );
     if (ret)
          return ret;

     buffer = ptr;

     DIRECT_INTERFACE_GET_DATA_FROM( buffer, buffer_data, IDirectFBEventBuffer_Requestor );

     ret = real->AttachEventBuffer( real, buffer_data->src );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_EnableEvents( IDirectFBWindow *thiz, IDirectFBWindow *real,
                       VoodooManager *manager, VoodooRequestMessage *msg )
{
     VoodooMessageParser parser;
     DFBWindowEventType  mask;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_UINT( parser, mask );
     VOODOO_PARSER_END( parser );

     return real->EnableEvents( real, mask );
}

static DirectResult
Dispatch_DetachEventBuffer( IDirectFBWindow *thiz, IDirectFBWindow *real,
                            VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult                         ret;
     IDirectFBEventBuffer                *buffer;
     IDirectFBEventBuffer_Requestor_data *buffer_data;
     VoodooInstanceID                     instance;
     VoodooMessageParser                  parser;
     void                                *ptr;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_ID( parser, instance );
     VOODOO_PARSER_END( parser );

     ret = voodoo_manager_lookup_remote( manager, instance, &ptr );
     if (ret)
          return ret;

     buffer = ptr;

     DIRECT_INTERFACE_GET_DATA_FROM( buffer, buffer_data, IDirectFBEventBuffer_Requestor );

     ret = real->DetachEventBuffer( real, buffer_data->src );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_GetID( IDirectFBWindow *thiz, IDirectFBWindow *real,
                VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult ret;
     DFBWindowID  id;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->GetID( real, &id );
     if (ret)
          return ret;

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    DFB_OK, VOODOO_INSTANCE_NONE,
                                    VMBT_ID, id,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_GetPosition( IDirectFBWindow *thiz, IDirectFBWindow *real,
                      VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult ret;
     DFBPoint     position;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->GetPosition( real, &position.x, &position.y );
     if (ret)
          return ret;

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    DFB_OK, VOODOO_INSTANCE_NONE,
                                    VMBT_DATA, sizeof(DFBPoint), &position,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_GetSize( IDirectFBWindow *thiz, IDirectFBWindow *real,
                  VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult ret;
     DFBDimension size;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->GetSize( real, &size.w, &size.h );
     if (ret)
          return ret;

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    DFB_OK, VOODOO_INSTANCE_NONE,
                                    VMBT_DATA, sizeof(DFBDimension), &size,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_GetSurface( IDirectFBWindow *thiz, IDirectFBWindow *real,
                     VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult      ret;
     IDirectFBSurface *surface;
     VoodooInstanceID  instance;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->GetSurface( real, &surface );
     if (ret)
          return ret;

     ret = voodoo_construct_dispatcher( manager, "IDirectFBSurface",
                                        surface, data->self, NULL, &instance, NULL );
     if (ret) {
          surface->Release( surface );
          return ret;
     }

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    DFB_OK, instance,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SetOptions( IDirectFBWindow *thiz, IDirectFBWindow *real,
                     VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult           ret;
     VoodooMessageParser parser;
     DFBWindowOptions    options;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_INT( parser, options );
     VOODOO_PARSER_END( parser );

     ret = real->SetOptions( real, options );

     return DFB_OK;
}

static DirectResult
Dispatch_GetOptions( IDirectFBWindow *thiz, IDirectFBWindow *real,
                     VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult     ret;
     DFBWindowOptions options;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->GetOptions( real, &options );
     if (ret)
          return ret;

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    DFB_OK, VOODOO_INSTANCE_NONE,
                                    VMBT_INT, options,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SetOpacity( IDirectFBWindow *thiz, IDirectFBWindow *real,
                     VoodooManager *manager, VoodooRequestMessage *msg )
{
     VoodooMessageParser parser;
     u8                  opacity;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_UINT( parser, opacity );
     VOODOO_PARSER_END( parser );

     real->SetOpacity( real, opacity );

     return DFB_OK;
}

static DirectResult
Dispatch_GetOpacity( IDirectFBWindow *thiz, IDirectFBWindow *real,
                     VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult     ret;
     u8               opacity;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->GetOpacity( real, &opacity );
     if (ret)
          return ret;

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    DFB_OK, VOODOO_INSTANCE_NONE,
                                    VMBT_UINT, opacity,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SetCursorShape( IDirectFBWindow *thiz, IDirectFBWindow *real,
                         VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult         ret;
     VoodooMessageParser  parser;
     VoodooInstanceID     instance;
     const DFBPoint      *hot;
     void                *surface;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_ID( parser, instance );
     VOODOO_PARSER_GET_DATA( parser, hot );
     VOODOO_PARSER_END( parser );

     ret = voodoo_manager_lookup_local( manager, instance, NULL, &surface );
     if (ret)
          return ret;

     ret = real->SetCursorShape( real, surface, hot->x, hot->y );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_RequestFocus( IDirectFBWindow *thiz, IDirectFBWindow *real,
                       VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->RequestFocus( real );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_GrabPointer( IDirectFBWindow *thiz, IDirectFBWindow *real,
                      VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->GrabPointer( real );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_UngrabPointer( IDirectFBWindow *thiz, IDirectFBWindow *real,
                        VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->UngrabPointer( real );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_Move( IDirectFBWindow *thiz, IDirectFBWindow *real,
               VoodooManager *manager, VoodooRequestMessage *msg )
{
     VoodooMessageParser  parser;
     const DFBPoint      *point;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, point );
     VOODOO_PARSER_END( parser );

     real->Move( real, point->x, point->y );

     return DFB_OK;
}

static DirectResult
Dispatch_MoveTo( IDirectFBWindow *thiz, IDirectFBWindow *real,
                 VoodooManager *manager, VoodooRequestMessage *msg )
{
     VoodooMessageParser  parser;
     const DFBPoint      *point;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, point );
     VOODOO_PARSER_END( parser );

     real->MoveTo( real, point->x, point->y );

     return DFB_OK;
}

static DirectResult
Dispatch_Resize( IDirectFBWindow *thiz, IDirectFBWindow *real,
                 VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult            ret;
     VoodooMessageParser  parser;
     const DFBDimension  *size;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, size );
     VOODOO_PARSER_END( parser );

     ret = real->Resize( real, size->w, size->h );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SetStackingClass( IDirectFBWindow *thiz, IDirectFBWindow *real,
                           VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult              ret;
     VoodooMessageParser    parser;
     DFBWindowStackingClass stacking_class;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_INT( parser, stacking_class );
     VOODOO_PARSER_END( parser );

     if (1) {
          if (voodoo_config->stacking_mask && !(voodoo_config->stacking_mask & (1 << stacking_class))) {
               D_ERROR( "Stacking class not permitted!\n" );
               return DR_ACCESSDENIED;
          }
     }

     ret = real->SetStackingClass( real, stacking_class );

     return DFB_OK;
}

static DirectResult
Dispatch_Raise( IDirectFBWindow *thiz, IDirectFBWindow *real,
                VoodooManager *manager, VoodooRequestMessage *msg )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     real->Raise( real );

     return DFB_OK;
}

static DirectResult
Dispatch_Lower( IDirectFBWindow *thiz, IDirectFBWindow *real,
                VoodooManager *manager, VoodooRequestMessage *msg )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     real->Lower( real );

     return DFB_OK;
}

static DirectResult
Dispatch_RaiseToTop( IDirectFBWindow *thiz, IDirectFBWindow *real,
                     VoodooManager *manager, VoodooRequestMessage *msg )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     real->RaiseToTop( real );

     return DFB_OK;
}

static DirectResult
Dispatch_LowerToBottom( IDirectFBWindow *thiz, IDirectFBWindow *real,
                        VoodooManager *manager, VoodooRequestMessage *msg )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     real->LowerToBottom( real );

     return DFB_OK;
}

static DirectResult
Dispatch_Close( IDirectFBWindow *thiz, IDirectFBWindow *real,
                VoodooManager *manager, VoodooRequestMessage *msg )
{
     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     real->Close( real );

     return DFB_OK;
}

static DirectResult
Dispatch_Destroy( IDirectFBWindow *thiz, IDirectFBWindow *real,
                  VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult ret;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     ret = real->Destroy( real );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SetBounds( IDirectFBWindow *thiz, IDirectFBWindow *real,
                    VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult           ret;
     VoodooMessageParser parser;
     const DFBRectangle *bounds;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, bounds );
     VOODOO_PARSER_END( parser );

     ret = real->SetBounds( real, bounds->x, bounds->y, bounds->w, bounds->h );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_ResizeSurface( IDirectFBWindow *thiz, IDirectFBWindow *real,
                        VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult           ret;
     VoodooMessageParser parser;
     const DFBDimension *size;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, size );
     VOODOO_PARSER_END( parser );

     ret = real->ResizeSurface( real, size->w, size->h );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SetSrcGeometry( IDirectFBWindow *thiz, IDirectFBWindow *real,
                         VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult                ret;
     VoodooMessageParser      parser;
     const DFBWindowGeometry *geometry;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, geometry );
     VOODOO_PARSER_END( parser );

     ret = real->SetSrcGeometry( real, geometry );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SetDstGeometry( IDirectFBWindow *thiz, IDirectFBWindow *real,
                         VoodooManager *manager, VoodooRequestMessage *msg )
{
     DFBResult                ret;
     VoodooMessageParser      parser;
     const DFBWindowGeometry *geometry;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, geometry );
     VOODOO_PARSER_END( parser );

     ret = real->SetDstGeometry( real, geometry );

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    ret, VOODOO_INSTANCE_NONE,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_GetProperty( IDirectFBWindow *thiz, IDirectFBWindow *real,
                      VoodooManager *manager, VoodooRequestMessage *msg )
{
     DirectResult         ret;
     VoodooMessageParser  parser;
     const char          *key;
     void                *value = NULL;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_STRING( parser, key );
     VOODOO_PARSER_END( parser );

     ret = real->GetProperty( real, key, &value );
     if (ret)
          return ret;

     if (!value)
          value = (void*) "";

     return voodoo_manager_respond( manager, true, msg->header.serial,
                                    DFB_OK, VOODOO_INSTANCE_NONE,
                                    VMBT_STRING, value,
                                    VMBT_NONE );
}

static DirectResult
Dispatch_SendEvent( IDirectFBWindow *thiz, IDirectFBWindow *real,
                    VoodooManager *manager, VoodooRequestMessage *msg )
{
     VoodooMessageParser   parser;
     const DFBWindowEvent *event;

     DIRECT_INTERFACE_GET_DATA(IDirectFBWindow_Dispatcher)

     VOODOO_PARSER_BEGIN( parser, msg );
     VOODOO_PARSER_GET_DATA( parser, event );
     VOODOO_PARSER_END( parser );

     return real->SendEvent( real, event );
}


static DirectResult
Dispatch( void *dispatcher, void *real, VoodooManager *manager, VoodooRequestMessage *msg )
{
     D_DEBUG( "IDirectFBWindow/Dispatcher: "
              "Handling request for instance %u with method %u...\n", msg->instance, msg->method );

     switch (msg->method) {
          case IDIRECTFBWINDOW_METHOD_ID_Release:
               return Dispatch_Release( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_CreateEventBuffer:
               return Dispatch_CreateEventBuffer( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_AttachEventBuffer:
               return Dispatch_AttachEventBuffer( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_EnableEvents:
               return Dispatch_EnableEvents( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_DetachEventBuffer:
               return Dispatch_DetachEventBuffer( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GetID:
               return Dispatch_GetID( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GetPosition:
               return Dispatch_GetPosition( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GetSize:
               return Dispatch_GetSize( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GetSurface:
               return Dispatch_GetSurface( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SetOptions:
               return Dispatch_SetOptions( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GetOptions:
               return Dispatch_GetOptions( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SetOpacity:
               return Dispatch_SetOpacity( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GetOpacity:
               return Dispatch_GetOpacity( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SetCursorShape:
               return Dispatch_SetCursorShape( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_RequestFocus:
               return Dispatch_RequestFocus( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GrabPointer:
               return Dispatch_GrabPointer( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_UngrabPointer:
               return Dispatch_UngrabPointer( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_Move:
               return Dispatch_Move( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_MoveTo:
               return Dispatch_MoveTo( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_Resize:
               return Dispatch_Resize( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SetStackingClass:
               return Dispatch_SetStackingClass( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_Raise:
               return Dispatch_Raise( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_Lower:
               return Dispatch_Lower( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_RaiseToTop:
               return Dispatch_RaiseToTop( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_LowerToBottom:
               return Dispatch_LowerToBottom( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_Close:
               return Dispatch_Close( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_Destroy:
               return Dispatch_Destroy( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SetBounds:
               return Dispatch_SetBounds( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_ResizeSurface:
               return Dispatch_ResizeSurface( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SetSrcGeometry:
               return Dispatch_SetSrcGeometry( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SetDstGeometry:
               return Dispatch_SetDstGeometry( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_GetProperty:
               return Dispatch_GetProperty( dispatcher, real, manager, msg );

          case IDIRECTFBWINDOW_METHOD_ID_SendEvent:
               return Dispatch_SendEvent( dispatcher, real, manager, msg );
     }

     return DFB_NOSUCHMETHOD;
}

/**************************************************************************************************/

static DFBResult
Probe()
{
     /* This implementation has to be loaded explicitly. */
     return DFB_UNSUPPORTED;
}

static DFBResult
Construct( IDirectFBWindow  *thiz,     /* Dispatcher interface */
           IDirectFBWindow  *real,     /* Real interface implementation */
           VoodooManager    *manager,  /* Manager of the Voodoo framework */
           VoodooInstanceID  super,    /* Instance ID of the super interface */
           void             *arg,      /* Optional arguments to constructor */
           VoodooInstanceID *ret_instance )
{
     DFBResult        ret;
     VoodooInstanceID instance;

     DIRECT_ALLOCATE_INTERFACE_DATA(thiz, IDirectFBWindow_Dispatcher)

     D_ASSERT( real != NULL );
     D_ASSERT( manager != NULL );
     D_ASSERT( super != VOODOO_INSTANCE_NONE );
     D_ASSERT( ret_instance != NULL );

     /* Register the dispatcher, getting a new instance ID that refers to it. */
     ret = voodoo_manager_register_local( manager, false, thiz, real, Dispatch, &instance );
     if (ret) {
          DIRECT_DEALLOCATE_INTERFACE( thiz );
          return ret;
     }

     /* Return the new instance. */
     *ret_instance = instance;

     /* Initialize interface data. */
     data->ref   = 1;
     data->real  = real;
     data->self  = instance;
     data->super = super;
     data->manager = manager;

     /* Initialize interface methods. */
     thiz->AddRef             = IDirectFBWindow_Dispatcher_AddRef;
     thiz->Release            = IDirectFBWindow_Dispatcher_Release;
     thiz->GetID              = IDirectFBWindow_Dispatcher_GetID;
     thiz->GetPosition        = IDirectFBWindow_Dispatcher_GetPosition;
     thiz->GetSize            = IDirectFBWindow_Dispatcher_GetSize;
     thiz->CreateEventBuffer  = IDirectFBWindow_Dispatcher_CreateEventBuffer;
     thiz->AttachEventBuffer  = IDirectFBWindow_Dispatcher_AttachEventBuffer;
     thiz->DetachEventBuffer  = IDirectFBWindow_Dispatcher_DetachEventBuffer;
     thiz->EnableEvents       = IDirectFBWindow_Dispatcher_EnableEvents;
     thiz->DisableEvents      = IDirectFBWindow_Dispatcher_DisableEvents;
     thiz->GetSurface         = IDirectFBWindow_Dispatcher_GetSurface;
     thiz->SetProperty        = IDirectFBWindow_Dispatcher_SetProperty;
     thiz->GetProperty        = IDirectFBWindow_Dispatcher_GetProperty;
     thiz->RemoveProperty     = IDirectFBWindow_Dispatcher_RemoveProperty;
     thiz->SetOptions         = IDirectFBWindow_Dispatcher_SetOptions;
     thiz->GetOptions         = IDirectFBWindow_Dispatcher_GetOptions;
     thiz->SetColorKey        = IDirectFBWindow_Dispatcher_SetColorKey;
     thiz->SetColorKeyIndex   = IDirectFBWindow_Dispatcher_SetColorKeyIndex;
     thiz->SetOpaqueRegion    = IDirectFBWindow_Dispatcher_SetOpaqueRegion;
     thiz->SetOpacity         = IDirectFBWindow_Dispatcher_SetOpacity;
     thiz->GetOpacity         = IDirectFBWindow_Dispatcher_GetOpacity;
     thiz->SetCursorShape     = IDirectFBWindow_Dispatcher_SetCursorShape;
     thiz->RequestFocus       = IDirectFBWindow_Dispatcher_RequestFocus;
     thiz->GrabKeyboard       = IDirectFBWindow_Dispatcher_GrabKeyboard;
     thiz->UngrabKeyboard     = IDirectFBWindow_Dispatcher_UngrabKeyboard;
     thiz->GrabPointer        = IDirectFBWindow_Dispatcher_GrabPointer;
     thiz->UngrabPointer      = IDirectFBWindow_Dispatcher_UngrabPointer;
     thiz->GrabKey            = IDirectFBWindow_Dispatcher_GrabKey;
     thiz->UngrabKey          = IDirectFBWindow_Dispatcher_UngrabKey;
     thiz->Move               = IDirectFBWindow_Dispatcher_Move;
     thiz->MoveTo             = IDirectFBWindow_Dispatcher_MoveTo;
     thiz->Resize             = IDirectFBWindow_Dispatcher_Resize;
     thiz->SetStackingClass   = IDirectFBWindow_Dispatcher_SetStackingClass;
     thiz->Raise              = IDirectFBWindow_Dispatcher_Raise;
     thiz->Lower              = IDirectFBWindow_Dispatcher_Lower;
     thiz->RaiseToTop         = IDirectFBWindow_Dispatcher_RaiseToTop;
     thiz->LowerToBottom      = IDirectFBWindow_Dispatcher_LowerToBottom;
     thiz->PutAtop            = IDirectFBWindow_Dispatcher_PutAtop;
     thiz->PutBelow           = IDirectFBWindow_Dispatcher_PutBelow;
     thiz->Close              = IDirectFBWindow_Dispatcher_Close;
     thiz->Destroy            = IDirectFBWindow_Dispatcher_Destroy;
     thiz->SetBounds          = IDirectFBWindow_Dispatcher_SetBounds;
     thiz->ResizeSurface      = IDirectFBWindow_Dispatcher_ResizeSurface;

     return DFB_OK;
}

