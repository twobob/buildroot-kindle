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

#include <core/CoreWindow.h>

#include <directfb_util.h>

#include <direct/debug.h>
#include <direct/mem.h>
#include <direct/messages.h>

#include <core/windows.h>
#include <core/windowstack.h>
#include <core/wm.h>

D_DEBUG_DOMAIN( Core_Window, "Core/Window", "DirectFB Core Window" );

/*********************************************************************************************************************/


DFBResult
IWindow_Real__Repaint( CoreWindow          *obj,
                       const DFBRegion     *left,
                       const DFBRegion     *right,
                       DFBSurfaceFlipFlags  flags )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );
     D_ASSERT( left != NULL );
     D_ASSERT( right != NULL );

     return dfb_window_repaint( obj, left, flags );
}

DFBResult
IWindow_Real__Restack( CoreWindow *obj,
                       CoreWindow *relative,
                       int         relation )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     DFBResult        ret;
     CoreWindowStack *stack;

     D_MAGIC_ASSERT( obj, CoreWindow );
     D_ASSERT( obj->stack != NULL );

     stack = obj->stack;

     /* Lock the window stack. */
     if (dfb_windowstack_lock( stack ))
          return DFB_FUSION;

     /* Never call WM after destroying the window. */
     if (DFB_WINDOW_DESTROYED( obj )) {
          dfb_windowstack_unlock( stack );
          return DFB_DESTROYED;
     }

     /* Let the window manager do its work. */
     ret = dfb_wm_restack_window( obj, relative, relation );

     /* Unlock the window stack. */
     dfb_windowstack_unlock( stack );

     return ret;
}

DFBResult
IWindow_Real__SetConfig( CoreWindow                    *obj,
                         const CoreWindowConfig        *config,
                         const DFBInputDeviceKeySymbol *keys,
                         u32                            num_keys,
                         CoreWindow                    *parent,
                         CoreWindowConfigFlags          flags )
{
     CoreWindowConfig config_copy;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );
     D_ASSERT( config != NULL );

     config_copy = *config;

     config_copy.keys        = (DFBInputDeviceKeySymbol*) keys;
     config_copy.num_keys    = num_keys;
     config_copy.association = parent ? parent->object.id : 0;

     return dfb_window_set_config( obj, &config_copy, flags );
}

DFBResult
IWindow_Real__Bind( CoreWindow *obj,                         
                    CoreWindow *source,
                    int         x,
                    int         y )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );
     D_MAGIC_ASSERT( source, CoreWindow );

     return dfb_window_bind( obj, source, x, y );
}

DFBResult
IWindow_Real__Unbind( CoreWindow *obj,
                      CoreWindow *source )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );
     D_MAGIC_ASSERT( source, CoreWindow );

     return dfb_window_unbind( obj, source );
}

DFBResult
IWindow_Real__RequestFocus( CoreWindow *obj )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_request_focus( obj );
}

DFBResult
IWindow_Real__ChangeGrab( CoreWindow          *obj,
                          CoreWMGrabTarget     target,
                          bool                 grab )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_change_grab( obj, target, grab );
}

DFBResult
IWindow_Real__GrabKey( CoreWindow                  *obj,
                       DFBInputDeviceKeySymbol     symbol,
                       DFBInputDeviceModifierMask  modifiers )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_grab_key( obj, symbol, modifiers );
}

DFBResult
IWindow_Real__UngrabKey( CoreWindow                  *obj,
                         DFBInputDeviceKeySymbol     symbol,
                         DFBInputDeviceModifierMask  modifiers )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_ungrab_key( obj, symbol, modifiers );
}

DFBResult
IWindow_Real__Move( CoreWindow *obj,
                    int         dx,
                    int         dy )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_move( obj, dx, dy, true );
}

DFBResult
IWindow_Real__MoveTo( CoreWindow *obj,
                      int         x,
                      int         y )
{
     DFBResult ret;
     DFBInsets insets;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     dfb_windowstack_lock( obj->stack );

     dfb_wm_get_insets( obj->stack, obj, &insets );

     ret = dfb_window_move( obj, x + insets.l, y + insets.t, false );

     dfb_windowstack_unlock( obj->stack );

     return ret;
}

DFBResult
IWindow_Real__Resize( CoreWindow *obj,
                      int         width,
                      int         height )
{
     DFBResult ret;
     DFBInsets insets;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     dfb_windowstack_lock( obj->stack );

     dfb_wm_get_insets( obj->stack, obj, &insets );

     ret = dfb_window_resize( obj, width + insets.l+insets.r, height + insets.t+insets.b );

     dfb_windowstack_unlock( obj->stack );

     return ret;
}

DFBResult
IWindow_Real__Destroy( CoreWindow *obj )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     dfb_window_destroy( obj );

     return DFB_OK;
}

DFBResult
IWindow_Real__BeginUpdates( CoreWindow      *obj,
                            const DFBRegion *update )
{
     DFBResult ret;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     dfb_windowstack_lock( obj->stack );

     ret = dfb_wm_begin_updates( obj, update );

     dfb_windowstack_unlock( obj->stack );

     return ret;
}

DFBResult
IWindow_Real__SetCursorPosition( CoreWindow *obj,
                                 int         x,
                                 int         y )
{
     DFBResult ret;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     dfb_windowstack_lock( obj->stack );

     ret = dfb_wm_set_cursor_position( obj, x, y );

     dfb_windowstack_unlock( obj->stack );

     return ret;
}

DFBResult
IWindow_Real__ChangeEvents( CoreWindow         *obj,
                            DFBWindowEventType  disable,
                            DFBWindowEventType  enable )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_change_events( obj, disable, enable );
}

DFBResult
IWindow_Real__ChangeOptions( CoreWindow       *obj,
                             DFBWindowOptions  disable,
                             DFBWindowOptions  enable )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_change_options( obj, disable, enable );
}

DFBResult
IWindow_Real__SetColor( CoreWindow     *obj,
                        const DFBColor *color )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_color( obj, *color );
}

DFBResult
IWindow_Real__SetColorKey( CoreWindow *obj,
                           u32         key )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_colorkey( obj, key );
}

DFBResult
IWindow_Real__SetOpaque( CoreWindow      *obj,
                         const DFBRegion *opaque )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_opaque( obj, opaque );
}

DFBResult
IWindow_Real__SetOpacity( CoreWindow *obj,
                          u8          opacity )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_opacity( obj, opacity );
}

DFBResult
IWindow_Real__SetStacking( CoreWindow             *obj,
                           DFBWindowStackingClass  stacking )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_change_stacking( obj, stacking );
}

DFBResult
IWindow_Real__SetBounds( CoreWindow         *obj,
                         const DFBRectangle *bounds )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_bounds( obj, DFB_RECTANGLE_VALS( bounds ) );
}

DFBResult
IWindow_Real__SetKeySelection( CoreWindow                    *obj,
                               DFBWindowKeySelection          selection,
                               const DFBInputDeviceKeySymbol *keys,
                               u32                            num_keys )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_key_selection( obj, selection, keys, num_keys );
}

DFBResult
IWindow_Real__SetRotation( CoreWindow *obj,
                           int         rotation )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_rotation( obj, rotation );
}

DFBResult
IWindow_Real__GetSurface(
                    CoreWindow                                *obj,
                    CoreSurface                              **ret_surface
)
{
     DFBResult ret;

     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_ASSERT( ret_surface != NULL );

     if (!obj->surface)
          return DFB_UNSUPPORTED;

     ret = (DFBResult) dfb_surface_ref( obj->surface );
     if (ret)
          return ret;

     *ret_surface = obj->surface;

     return DFB_OK;
}

DFBResult
IWindow_Real__SetCursorShape(
                    CoreWindow                                *obj,
                    CoreSurface                               *shape,
                    const DFBPoint                            *hotspot )
{
     D_DEBUG_AT( Core_Window, "%s( %p )\n", __FUNCTION__, obj );

     D_MAGIC_ASSERT( obj, CoreWindow );

     return dfb_window_set_cursor_shape( obj, shape, hotspot->x, hotspot->y );
}

