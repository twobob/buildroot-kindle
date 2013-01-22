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

#include <direct/debug.h>
#include <direct/messages.h>
#include <direct/util.h>

#include <core/gfxcard.h>
#include <core/state.h>
#include <core/surface.h>
#include <core/windows_internal.h>

#include <misc/util.h>

#include <unique/context.h>
#include <unique/internal.h>
#include <unique/stret.h>
#include <unique/window.h>


D_DEBUG_DOMAIN( UniQuE_Window, "UniQuE/WindowReg", "UniQuE's Window Region Class" );

static DFBResult
window_get_input( StretRegion         *region,
                  void                *region_data,
                  unsigned long        arg,
                  int                  index,
                  int                  x,
                  int                  y,
                  UniqueInputChannel **ret_channel )
{
     UniqueWindow *window = region_data;

     D_MAGIC_ASSERT( region, StretRegion );
     D_MAGIC_ASSERT( window, UniqueWindow );

     D_ASSERT( ret_channel != NULL );

     D_DEBUG_AT( UniQuE_Window, "window_get_input( region %p, window %p, index %d, x %d, y %d )\n",
                 region, window, index, x, y );

     *ret_channel = window->channel;

     return DFB_OK;
}

static void
window_update( StretRegion     *region,
               void            *region_data,
               void            *update_data,
               unsigned long    arg,
               int              x,
               int              y,
               const DFBRegion *updates,
               int              num )
{
     int                      i;
     DFBSurfaceBlittingFlags  flags  = DSBLIT_NOFX;
     UniqueWindow            *window = region_data;
     CardState               *state  = update_data;
     bool                     alpha  = arg;
     bool                     visible;

     D_ASSERT( updates != NULL );

     D_MAGIC_ASSERT( region, StretRegion );
     D_MAGIC_ASSERT( window, UniqueWindow );
     D_MAGIC_ASSERT( state, CardState );

     D_ASSERT( window->surface != NULL );

     visible = D_FLAGS_IS_SET( window->flags, UWF_VISIBLE );

     D_DEBUG_AT( UniQuE_Window, "window_update( region %p, window %p, visible %s, num %d )\n",
                 region, window, visible ? "yes" : "no", num );
#if D_DEBUG_ENABLED
     for (i=0; i<num; i++) {
          D_DEBUG_AT( UniQuE_Window, "    (%d)  %4d,%4d - %4dx%4d\n",
                      i, DFB_RECTANGLE_VALS_FROM_REGION( &updates[i] ) );
     }
#endif

     if (!visible)
          return;

     /* Use per pixel alpha blending. */
     if (alpha && (window->options & DWOP_ALPHACHANNEL))
          flags |= DSBLIT_BLEND_ALPHACHANNEL;

     /* Use global alpha blending. */
     if (window->opacity != 0xFF) {
          flags |= DSBLIT_BLEND_COLORALPHA;

          /* Set opacity as blending factor. */
          if (state->color.a != window->opacity) {
               state->color.a   = window->opacity;
               state->modified |= SMF_COLOR;
          }
     }

     /* Use source color keying. */
     if (window->options & DWOP_COLORKEYING) {
          flags |= DSBLIT_SRC_COLORKEY;

          /* Set window color key. */
          dfb_state_set_src_colorkey( state, window->color_key );
     }

     /* Use automatic deinterlacing. */
     if (window->surface->config.caps & DSCAPS_INTERLACED)
          flags |= DSBLIT_DEINTERLACE;

     /* Set blitting flags. */
     dfb_state_set_blitting_flags( state, flags );

     /* Set blitting source. */
     state->source    = window->surface;
     state->modified |= SMF_SOURCE;

     for (i=0; i<num; i++) {
          DFBRectangle src = DFB_RECTANGLE_INIT_FROM_REGION( &updates[i] );

          /* Blit from the window to the region being updated. */
          dfb_gfxcard_blit( &src, x + src.x, y + src.y, state );
     }

     /* Reset blitting source. */
     state->source    = NULL;
     state->modified |= SMF_SOURCE;
}

const StretRegionClass unique_window_region_class = {
     .GetInput = window_get_input,
     .Update   = window_update,
};

