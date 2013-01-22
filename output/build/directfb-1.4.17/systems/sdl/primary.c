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

#include <directfb.h>

#include <fusion/fusion.h>
#include <fusion/shmalloc.h>

#include <core/core.h>
#include <core/coredefs.h>
#include <core/coretypes.h>
#include <core/layers.h>
#include <core/palette.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/system.h>

#include <gfx/convert.h>

#include <misc/conf.h>
#include <misc/util.h>

#include <direct/debug.h>
#include <direct/memcpy.h>
#include <direct/messages.h>
#include <direct/thread.h>

#include <SDL.h>

#include "sdl.h"
#include "primary.h"

D_DEBUG_DOMAIN( SDL_Screen,  "SDL/Screen",  "SDL System Screen" );
D_DEBUG_DOMAIN( SDL_Updates, "SDL/Updates", "SDL System Screen Updates" );

/******************************************************************************/

static DFBResult update_screen( int x, int y, int w, int h );

/******************************************************************************/

static DFBResult
primaryInitScreen( CoreScreen           *screen,
                   CoreGraphicsDevice   *device,
                   void                 *driver_data,
                   void                 *screen_data,
                   DFBScreenDescription *description )
{
     /* Set the screen capabilities. */
     description->caps = DSCCAPS_NONE;

     /* Set the screen name. */
     snprintf( description->name,
               DFB_SCREEN_DESC_NAME_LENGTH, "SDL Primary Screen" );

     return DFB_OK;
}

static DFBResult
primaryGetScreenSize( CoreScreen *screen,
                      void       *driver_data,
                      void       *screen_data,
                      int        *ret_width,
                      int        *ret_height )
{
     D_ASSERT( dfb_sdl != NULL );

     if (dfb_sdl->primary) {
          *ret_width  = dfb_sdl->primary->config.size.w;
          *ret_height = dfb_sdl->primary->config.size.w;
     }
     else {
          if (dfb_config->mode.width)
               *ret_width  = dfb_config->mode.width;
          else
               *ret_width  = 640;

          if (dfb_config->mode.height)
               *ret_height = dfb_config->mode.height;
          else
               *ret_height = 480;
     }

     return DFB_OK;
}

static ScreenFuncs primaryScreenFuncs = {
     .InitScreen    = primaryInitScreen,
     .GetScreenSize = primaryGetScreenSize,
};

ScreenFuncs *sdlPrimaryScreenFuncs = &primaryScreenFuncs;

/******************************************************************************/

static void * ScreenUpdateLoop( DirectThread *thread, void *arg );

/******************************************************************************/

static int
primaryLayerDataSize( void )
{
     return 0;
}

static int
primaryRegionDataSize( void )
{
     return 0;
}

static DFBResult
primaryInitLayer( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  DFBDisplayLayerDescription *description,
                  DFBDisplayLayerConfig      *config,
                  DFBColorAdjustment         *adjustment )
{
     /* set capabilities and type */
     description->caps = DLCAPS_SURFACE;
     description->type = DLTF_GRAPHICS;

     /* set name */
     snprintf( description->name,
               DFB_DISPLAY_LAYER_DESC_NAME_LENGTH, "SDL Primary Layer" );

     /* fill out the default configuration */
     config->flags       = DLCONF_WIDTH       | DLCONF_HEIGHT |
                           DLCONF_PIXELFORMAT | DLCONF_BUFFERMODE;
     config->buffermode  = DLBM_FRONTONLY;

     if (dfb_config->mode.width)
          config->width  = dfb_config->mode.width;
     else
          config->width  = 640;

     if (dfb_config->mode.height)
          config->height = dfb_config->mode.height;
     else
          config->height = 480;

     if (dfb_config->mode.format != DSPF_UNKNOWN)
          config->pixelformat = dfb_config->mode.format;
     else if (dfb_config->mode.depth > 0)
          config->pixelformat = dfb_pixelformat_for_depth( dfb_config->mode.depth );
     else
          config->pixelformat = DSPF_RGB16;

     /* Initialize update lock and condition. */
     pthread_mutex_init( &dfb_sdl->update.lock, NULL );
     pthread_cond_init( &dfb_sdl->update.cond, NULL );

     /* Start update thread. */
     dfb_sdl->update.thread = direct_thread_create( DTT_OUTPUT, ScreenUpdateLoop, NULL, "Screen Update" );
     if (!dfb_sdl->update.thread)
          return DFB_FAILURE;

     return DFB_OK;
}

static DFBResult
primaryTestRegion( CoreLayer                  *layer,
                   void                       *driver_data,
                   void                       *layer_data,
                   CoreLayerRegionConfig      *config,
                   CoreLayerRegionConfigFlags *failed )
{
     CoreLayerRegionConfigFlags fail = 0;

     switch (config->buffermode) {
          case DLBM_FRONTONLY:
          case DLBM_BACKSYSTEM:
          case DLBM_BACKVIDEO:
               break;

          default:
               fail |= CLRCF_BUFFERMODE;
               break;
     }

     if (config->options)
          fail |= CLRCF_OPTIONS;

     if (failed)
          *failed = fail;

     if (fail)
          return DFB_UNSUPPORTED;

     return DFB_OK;
}

static DFBResult
primaryAddRegion( CoreLayer             *layer,
                  void                  *driver_data,
                  void                  *layer_data,
                  void                  *region_data,
                  CoreLayerRegionConfig *config )
{
     return DFB_OK;
}

static DFBResult
primarySetRegion( CoreLayer                  *layer,
                  void                       *driver_data,
                  void                       *layer_data,
                  void                       *region_data,
                  CoreLayerRegionConfig      *config,
                  CoreLayerRegionConfigFlags  updated,
                  CoreSurface                *surface,
                  CorePalette                *palette,
                  CoreSurfaceBufferLock      *lock )
{
     if (surface) {
          pthread_mutex_lock( &dfb_sdl->update.lock );
          dfb_sdl->primary = surface;
          dfb_sdl->update.pending = false;
          pthread_mutex_unlock( &dfb_sdl->update.lock );
     }

     if (palette)
          dfb_sdl_set_palette( palette );

     return DFB_OK;
}

static DFBResult
primaryRemoveRegion( CoreLayer             *layer,
                     void                  *driver_data,
                     void                  *layer_data,
                     void                  *region_data )
{
     D_DEBUG_AT( SDL_Updates, "%s( %p )\n", __FUNCTION__, layer );

     D_DEBUG_AT( SDL_Updates, "  -> locking sdl lock...\n" );

     fusion_skirmish_prevail( &dfb_sdl->lock );

     D_DEBUG_AT( SDL_Updates, "  -> setting primary to NULL...\n" );

     dfb_sdl->primary = NULL;

     D_DEBUG_AT( SDL_Updates, "  -> unlocking sdl lock...\n" );

     fusion_skirmish_dismiss( &dfb_sdl->lock );

     D_DEBUG_AT( SDL_Updates, "  -> done.\n" );

     return DFB_OK;
}

static DFBResult
primaryFlipRegion( CoreLayer             *layer,
                   void                  *driver_data,
                   void                  *layer_data,
                   void                  *region_data,
                   CoreSurface           *surface,
                   DFBSurfaceFlipFlags    flags,
                   CoreSurfaceBufferLock *lock )
{
     dfb_surface_flip( surface, false );

     return dfb_sdl_update_screen( dfb_sdl_core, NULL );
}

static DFBResult
primaryUpdateRegion( CoreLayer             *layer,
                     void                  *driver_data,
                     void                  *layer_data,
                     void                  *region_data,
                     CoreSurface           *surface,
                     const DFBRegion       *update,
                     CoreSurfaceBufferLock *lock )
{
     if (update) {
          DFBRegion region = *update;

          return dfb_sdl_update_screen( dfb_sdl_core, &region );
     }

     return dfb_sdl_update_screen( dfb_sdl_core, NULL );
}

static DisplayLayerFuncs primaryLayerFuncs = {
     .LayerDataSize     = primaryLayerDataSize,
     .RegionDataSize    = primaryRegionDataSize,
     .InitLayer         = primaryInitLayer,

     .TestRegion        = primaryTestRegion,
     .AddRegion         = primaryAddRegion,
     .SetRegion         = primarySetRegion,
     .RemoveRegion      = primaryRemoveRegion,
     .FlipRegion        = primaryFlipRegion,
     .UpdateRegion      = primaryUpdateRegion,
};

DisplayLayerFuncs *sdlPrimaryLayerFuncs = &primaryLayerFuncs;

/******************************************************************************/

static DFBResult
update_screen( int x, int y, int w, int h )
{
#if 0
     int                    i, n;
     void                  *dst;
     void                  *src;
     DFBResult              ret;
     CoreSurface           *surface;
     CoreSurfaceBuffer     *buffer;
     CoreSurfaceBufferLock  lock;
     u16                   *src16, *dst16;
     u8                    *src8;
#endif

     D_DEBUG_AT( SDL_Updates, "%s( %d, %d, %d, %d )\n", __FUNCTION__, x, y, w, h );

     D_DEBUG_AT( SDL_Updates, "  -> locking sdl lock...\n" );

     fusion_skirmish_prevail( &dfb_sdl->lock );
#if 0
     surface = dfb_sdl->primary;
     D_MAGIC_ASSERT_IF( surface, CoreSurface );

     D_DEBUG_AT( SDL_Updates, "  -> primary is %p\n", surface );

     if (!surface) {
          D_DEBUG_AT( SDL_Updates, "  -> unlocking sdl lock...\n" );
          fusion_skirmish_dismiss( &dfb_sdl->lock );
          D_DEBUG_AT( SDL_Updates, "  -> done.\n" );
          return DFB_OK;
     }

     buffer = dfb_surface_get_buffer( surface, CSBR_FRONT );

     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     D_DEBUG_AT( SDL_Updates, "  -> locking sdl surface...\n" );

     if (SDL_LockSurface( screen ) < 0) {
          D_ERROR( "DirectFB/SDL: "
                   "Couldn't lock the display surface: %s\n", SDL_GetError() );
          fusion_skirmish_dismiss( &dfb_sdl->lock );
          return DFB_FAILURE;
     }

     D_DEBUG_AT( SDL_Updates, "  -> locking dfb surface...\n" );

     ret = dfb_surface_buffer_lock( buffer, CSAF_CPU_READ, &lock );
     if (ret) {
          D_ERROR( "DirectFB/SDL: Couldn't lock layer surface: %s\n",
                   DirectFBErrorString( ret ) );
          SDL_UnlockSurface(screen);
          fusion_skirmish_dismiss( &dfb_sdl->lock );
          return ret;
     }

     src = lock.addr;
     dst = screen->pixels;

     src += DFB_BYTES_PER_LINE( surface->config.format, x ) + y * lock.pitch;
     dst += DFB_BYTES_PER_LINE( surface->config.format, x ) + y * screen->pitch;

     D_DEBUG_AT( SDL_Updates, "  -> copying pixels...\n" );

     switch (screen->format->BitsPerPixel) {
          case 16:
               dfb_convert_to_rgb16( surface->config.format,
                                     src, lock.pitch, surface->config.size.h,
                                     dst, screen->pitch, w, h );
               break;

          default:
               direct_memcpy( dst, src, DFB_BYTES_PER_LINE( surface->config.format, w ) );
     }

     D_DEBUG_AT( SDL_Updates, "  -> unlocking dfb surface...\n" );

     dfb_surface_buffer_unlock( &lock );

     D_DEBUG_AT( SDL_Updates, "  -> unlocking sdl surface...\n" );

     SDL_UnlockSurface( screen );
#endif
     D_DEBUG_AT( SDL_Updates, "  -> calling SDL_UpdateRect()...\n" );

     SDL_UpdateRect( dfb_sdl->screen, x, y, w, h );

     D_DEBUG_AT( SDL_Updates, "  -> unlocking sdl lock...\n" );

     fusion_skirmish_dismiss( &dfb_sdl->lock );

     D_DEBUG_AT( SDL_Updates, "  -> done.\n" );

     return DFB_OK;
}

static void *
ScreenUpdateLoop( DirectThread *thread, void *arg )
{
     pthread_mutex_lock( &dfb_sdl->update.lock );

     D_DEBUG_AT( SDL_Updates, "Entering %s()...\n", __FUNCTION__ );

     while (!dfb_sdl->update.quit) {
          if (dfb_sdl->update.pending) {
               DFBRectangle update = DFB_RECTANGLE_INIT_FROM_REGION( &dfb_sdl->update.region );

               dfb_sdl->update.pending = false;

               D_DEBUG_AT( SDL_Updates, "Got update %d,%d - %dx%d...\n", DFB_RECTANGLE_VALS( &update ) );

               pthread_mutex_unlock( &dfb_sdl->update.lock );


               update_screen( update.x, update.y, update.w, update.h );


               pthread_mutex_lock( &dfb_sdl->update.lock );
          }
          else
               pthread_cond_wait( &dfb_sdl->update.cond, &dfb_sdl->update.lock );
     }

     D_DEBUG_AT( SDL_Updates, "Returning from %s()...\n", __FUNCTION__ );

     pthread_mutex_unlock( &dfb_sdl->update.lock );

     return NULL;
}

/******************************************************************************/

typedef enum {
     SDL_SET_VIDEO_MODE,
     SDL_UPDATE_SCREEN,
     SDL_SET_PALETTE
} DFBSDLCall;

static inline int
get_pixelformat_target_depth( DFBSurfacePixelFormat format )
{
     switch (format) {
          case DSPF_NV16:
               return 16;

          default:
               break;
     }

     return DFB_BITS_PER_PIXEL( format );
}

static DFBResult
dfb_sdl_set_video_mode_handler( CoreSurfaceConfig *config )
{
     int          depth = get_pixelformat_target_depth( config->format );
     Uint32       flags = SDL_HWSURFACE | SDL_RESIZABLE;// | SDL_ASYNCBLIT | SDL_FULLSCREEN;
     SDL_Surface *screen;

     if (config->caps & DSCAPS_FLIPPING)
          flags |= SDL_DOUBLEBUF;

     fusion_skirmish_prevail( &dfb_sdl->lock );

     D_DEBUG_AT( SDL_Screen, "  -> SDL_SetVideoMode( %dx%d, %d, 0x%08x )\n",
                 config->size.w, config->size.h, DFB_BITS_PER_PIXEL(config->format), flags );

     /* Set video mode */
     screen = SDL_SetVideoMode( config->size.w, config->size.h, depth, flags );
     if (!screen) {
          D_ERROR( "DirectFB/SDL: Couldn't set %dx%dx%d video mode: %s\n",
                   config->size.w, config->size.h, depth, SDL_GetError());

          fusion_skirmish_dismiss( &dfb_sdl->lock );

          return DFB_FAILURE;
     }

     dfb_sdl->screen = screen;

     /* Hide SDL's cursor */
     SDL_ShowCursor( SDL_DISABLE );

     fusion_skirmish_dismiss( &dfb_sdl->lock );

     return DFB_OK;
}

static DFBResult
dfb_sdl_update_screen_handler( const DFBRegion *region )
{
     DFBRegion    update;
     CoreSurface *surface = dfb_sdl->primary;

     DFB_REGION_ASSERT_IF( region );

     if (region)
          update = *region;
     else {
          update.x1 = 0;
          update.y1 = 0;
          update.x2 = surface->config.size.w - 1;
          update.y2 = surface->config.size.h - 1;
     }

#if 0
     pthread_mutex_lock( &dfb_sdl->update.lock );

     if (dfb_sdl->update.pending)
          dfb_region_region_union( &dfb_sdl->update.region, &update );
     else {
          dfb_sdl->update.region  = update;
          dfb_sdl->update.pending = true;
     }

     pthread_cond_signal( &dfb_sdl->update.cond );

     pthread_mutex_unlock( &dfb_sdl->update.lock );
#else
     if (surface->config.caps & DSCAPS_FLIPPING)
          SDL_Flip( dfb_sdl->screen );
     else
          SDL_UpdateRect( dfb_sdl->screen, DFB_RECTANGLE_VALS_FROM_REGION(&update) );
#endif

     return DFB_OK;
}

static DFBResult
dfb_sdl_set_palette_handler( CorePalette *palette )
{
     unsigned int i;
     SDL_Color    colors[palette->num_entries];

     for (i=0; i<palette->num_entries; i++) {
          colors[i].r = palette->entries[i].r;
          colors[i].g = palette->entries[i].g;
          colors[i].b = palette->entries[i].b;
     }

     fusion_skirmish_prevail( &dfb_sdl->lock );

     SDL_SetColors( dfb_sdl->screen, colors, 0, palette->num_entries );

     fusion_skirmish_dismiss( &dfb_sdl->lock );

     return DFB_OK;
}

FusionCallHandlerResult
dfb_sdl_call_handler( int           caller,
                      int           call_arg,
                      void         *call_ptr,
                      void         *ctx,
                      unsigned int  serial,
                      int          *ret_val )
{
     switch (call_arg) {
          case SDL_SET_VIDEO_MODE:
               *ret_val = dfb_sdl_set_video_mode_handler( call_ptr );
               break;

          case SDL_UPDATE_SCREEN:
               *ret_val = dfb_sdl_update_screen_handler( call_ptr );
               break;

          case SDL_SET_PALETTE:
               *ret_val = dfb_sdl_set_palette_handler( call_ptr );
               break;

          default:
               D_BUG( "unknown call" );
               *ret_val = DFB_BUG;
               break;
     }

     return FCHR_RETURN;
}

DFBResult
dfb_sdl_set_video_mode( CoreDFB *core, CoreSurfaceConfig *config )
{
     int                ret;
     CoreSurfaceConfig *tmp = NULL;

     D_ASSERT( config != NULL );

     if (dfb_core_is_master( core ))
          return dfb_sdl_set_video_mode_handler( config );

     if (!fusion_is_shared( dfb_core_world(core), config )) {
          tmp = SHMALLOC( dfb_core_shmpool(core), sizeof(CoreSurfaceConfig) );
          if (!tmp)
               return D_OOSHM();

          direct_memcpy( tmp, config, sizeof(CoreSurfaceConfig) );
     }

     fusion_call_execute( &dfb_sdl->call, FCEF_NONE, SDL_SET_VIDEO_MODE,
                          tmp ? tmp : config, &ret );

     if (tmp)
          SHFREE( dfb_core_shmpool(core), tmp );

     return ret;
}

DFBResult
dfb_sdl_update_screen( CoreDFB *core, DFBRegion *region )
{
     int        ret;
     DFBRegion *tmp = NULL;

     if (dfb_core_is_master( core ))
          return dfb_sdl_update_screen_handler( region );

     if (region) {
          tmp = SHMALLOC( dfb_core_shmpool(core), sizeof(DFBRegion) );
          if (!tmp)
               return D_OOSHM();

          direct_memcpy( tmp, region, sizeof(DFBRegion) );
     }

     fusion_call_execute( &dfb_sdl->call, FCEF_NONE, SDL_UPDATE_SCREEN, tmp ? tmp : region, &ret );

     if (tmp)
          SHFREE( dfb_core_shmpool(core), tmp );

     return DFB_OK;
}

DFBResult
dfb_sdl_set_palette( CorePalette *palette )
{
     int ret;

     fusion_call_execute( &dfb_sdl->call, FCEF_NONE, SDL_SET_PALETTE, palette, &ret );

     return ret;
}

