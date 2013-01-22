/*
   TI Davinci driver - 2D Acceleration

   (c) Copyright 2007  Telio AG

   Written by Denis Oliver Kropp <dok@directfb.org>

   Code is derived from VMWare driver.

   (c) Copyright 2001-2009  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

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

//#define DIRECT_ENABLE_DEBUG

#include <config.h>

#include <asm/types.h>

#include <directfb.h>

#include <direct/debug.h>
#include <direct/memcpy.h>
#include <direct/messages.h>

#include <core/state.h>
#include <core/surface.h>

#include <gfx/convert.h>

#include "davinci_2d.h"
#include "davinci_gfxdriver.h"


D_DEBUG_DOMAIN( Davinci_2D, "Davinci/2D", "Davinci 2D Acceleration" );

/*
 * State validation flags.
 *
 * There's no prefix because of the macros below.
 */
enum {
     DESTINATION    = 0x00000001,
     FILLCOLOR      = 0x00000002,

     SOURCE         = 0x00000010,
     SOURCE_MULT    = 0x00000020,

     BLIT_BLEND_SUB = 0x00010000,
     DRAW_BLEND_SUB = 0x00020000,

     ALL            = 0x00030033
};

/*
 * State handling macros.
 */

#define DAVINCI_VALIDATE(flags)        do { ddev->v_flags |=  (flags); } while (0)
#define DAVINCI_INVALIDATE(flags)      do { ddev->v_flags &= ~(flags); } while (0)

#define DAVINCI_CHECK_VALIDATE(flag)   do {                                               \
                                            if (! (ddev->v_flags & flag))                 \
                                                davinci_validate_##flag( ddev, state );   \
                                       } while (0)

/**************************************************************************************************/

static bool davinciFillRectangle16( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *rect );

static bool davinciFillRectangle32( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *rect );

static bool davinciFillRectangleBlend32( void                *drv,
                                         void                *dev,
                                         DFBRectangle        *rect );

static bool davinciBlit16         ( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *srect,
                                    int                  dx,
                                    int                  dy );

static bool davinciBlit32to16     ( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *srect,
                                    int                  dx,
                                    int                  dy );

static bool davinciBlit32         ( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *srect,
                                    int                  dx,
                                    int                  dy );

static bool davinciBlitKeyed16    ( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *srect,
                                    int                  dx,
                                    int                  dy );

static bool davinciBlitKeyed32    ( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *srect,
                                    int                  dx,
                                    int                  dy );

static bool davinciBlitBlend32    ( void                *drv,
                                    void                *dev,
                                    DFBRectangle        *srect,
                                    int                  dx,
                                    int                  dy );

/**************************************************************************************************/

static inline int
get_blit_blend_sub_function( const CardState *state )
{
     DFBSurfaceBlittingFlags flags = state->blittingflags & ~DSBLIT_COLORIZE;

     if (state->dst_blend == DSBF_INVSRCALPHA) {
          switch (state->src_blend) {
               case DSBF_SRCALPHA:
                    if (flags == DSBLIT_BLEND_ALPHACHANNEL)
                         return C64X_BLEND_SRC_INVSRC;
                    break;

               case DSBF_ONE:
                    switch (flags) {
                         case DSBLIT_BLEND_ALPHACHANNEL:
                              return C64X_BLEND_ONE_INVSRC;

                         case DSBLIT_BLEND_ALPHACHANNEL |
                              DSBLIT_SRC_PREMULTIPLY:
                              return C64X_BLEND_ONE_INVSRC_PREMULT_SRC;

                         case DSBLIT_BLEND_ALPHACHANNEL |
                              DSBLIT_BLEND_COLORALPHA |
                              DSBLIT_SRC_PREMULTCOLOR:
                              return C64X_BLEND_ONE_INVSRC_PREMULT_ALPHA;

                         default:
                              break;
                    }
                    break;

               default:
                    break;
          }
     }

     return -1;
}

static inline int
get_draw_blend_sub_function( const CardState *state )
{
     DFBSurfaceDrawingFlags flags = state->drawingflags;

     if (state->dst_blend == DSBF_INVSRCALPHA) {
          switch (state->src_blend) {
               case DSBF_SRCALPHA:
                    if (flags == DSDRAW_BLEND)
                         return C64X_BLEND_SRC_INVSRC;
                    break;

               case DSBF_ONE:
                    switch (flags) {
                         case DSDRAW_BLEND:
                              return C64X_BLEND_ONE_INVSRC;

                         case DSDRAW_BLEND |
                              DSDRAW_SRC_PREMULTIPLY:
                              return C64X_BLEND_ONE_INVSRC_PREMULT_SRC;

                         default:
                              break;
                    }
                    break;

               default:
                    break;
          }
     }

     return -1;
}

/**************************************************************************************************/

/*
 * Called by davinciSetState() to ensure that the destination registers are properly set
 * for execution of rendering functions.
 */
static inline void
davinci_validate_DESTINATION( DavinciDeviceData *ddev,
                              CardState         *state )
{
     /* Remember destination parameters for usage in rendering functions. */
     ddev->dst_addr   = state->dst.addr;
     ddev->dst_phys   = state->dst.phys;
     ddev->dst_size   = state->dst.allocation->size;
     ddev->dst_pitch  = state->dst.pitch;
     ddev->dst_format = state->dst.buffer->format;
     ddev->dst_bpp    = DFB_BYTES_PER_PIXEL( ddev->dst_format );

     D_DEBUG_AT( Davinci_2D, "  => DESTINATION: 0x%08lx\n", ddev->dst_phys );

     /* Set the flag. */
     DAVINCI_VALIDATE( DESTINATION );
}

/*
 * Called by davinciSetState() to ensure that the color register is properly set
 * for execution of rendering functions.
 */
static inline void
davinci_validate_FILLCOLOR( DavinciDeviceData *ddev,
                            CardState         *state )
{
     switch (ddev->dst_format) {
          case DSPF_ARGB:
          case DSPF_RGB32:
               ddev->fillcolor = ddev->color_argb;
               break;

          case DSPF_RGB16:
               ddev->fillcolor = PIXEL_RGB16( state->color.r,
                                              state->color.g,
                                              state->color.b );

               ddev->fillcolor |= ddev->fillcolor << 16;
               break;

          case DSPF_UYVY: {
               int y, u, v;

               RGB_TO_YCBCR( state->color.r, state->color.g, state->color.b, y, u, v );

               ddev->fillcolor = PIXEL_UYVY( y, u, v );
               break;
          }

          default:
               D_BUG( "unexpected format %s", dfb_pixelformat_name(ddev->dst_format) );
               return;
     }

     D_DEBUG_AT( Davinci_2D, "  => FILLCOLOR: 0x%08lx\n", ddev->fillcolor );

     /* Set the flag. */
     DAVINCI_VALIDATE( FILLCOLOR );
}

/*
 * Called by davinciSetState() to ensure that the source registers are properly set
 * for execution of blitting functions.
 */
static inline void
davinci_validate_SOURCE( DavinciDeviceData *ddev,
                         CardState         *state )
{
     /* Remember source parameters for usage in rendering functions. */
     ddev->src_addr   = state->src.addr;
     ddev->src_phys   = state->src.phys;
     ddev->src_pitch  = state->src.pitch;
     ddev->src_format = state->src.buffer->format;
     ddev->src_bpp    = DFB_BYTES_PER_PIXEL( ddev->src_format );

     D_DEBUG_AT( Davinci_2D, "  => SOURCE: 0x%08lx\n", ddev->src_phys );

     /* Set the flag. */
     DAVINCI_VALIDATE( SOURCE );
}

/*
 * Called by davinciSetState() to ensure that the source ARGB modulation is properly set
 * for execution of blitting functions.
 */
static inline void
davinci_validate_SOURCE_MULT( DavinciDeviceData *ddev,
                              CardState         *state )
{
     switch (ddev->dst_format) {
          case DSPF_ARGB:
               if (state->blittingflags & DSBLIT_COLORIZE)
                    ddev->source_mult = 0xff000000 | ddev->color_argb;
               else
                    ddev->source_mult = 0xffffffff;
               break;

          default:
               D_BUG( "unexpected format %s", dfb_pixelformat_name(ddev->dst_format) );
               return;
     }

     D_DEBUG_AT( Davinci_2D, "  => SOURCE_MULT: 0x%08lx\n", ddev->source_mult );

     /* Set the flag. */
     DAVINCI_VALIDATE( SOURCE_MULT );
}

/*
 * Called by davinciSetState() to ensure that the blend sub function index is valid
 * for execution of blitting functions.
 */
static inline void
davinci_validate_BLIT_BLEND_SUB( DavinciDeviceData *ddev,
                                 CardState         *state )
{
     int index = get_blit_blend_sub_function( state );

     if (index < 0) {
          D_BUG( "unexpected state" );
          return;
     }

     /* Set blend sub function index. */
     ddev->blit_blend_sub_function = index;

     D_DEBUG_AT( Davinci_2D, "  => BLIT_BLEND_SUB: %d\n", index );

     /* Set the flag. */
     DAVINCI_VALIDATE( BLIT_BLEND_SUB );
}

/*
 * Called by davinciSetState() to ensure that the blend sub function index is valid
 * for execution of drawing functions.
 */
static inline void
davinci_validate_DRAW_BLEND_SUB( DavinciDeviceData *ddev,
                                 CardState         *state )
{
     int index = get_draw_blend_sub_function( state );

     if (index < 0) {
          D_BUG( "unexpected state" );
          return;
     }

     /* Set blend sub function index. */
     ddev->draw_blend_sub_function = index;

     D_DEBUG_AT( Davinci_2D, "  => DRAW_BLEND_SUB: %d\n", index );

     /* Set the flag. */
     DAVINCI_VALIDATE( DRAW_BLEND_SUB );
}

/**************************************************************************************************/

/*
 * Wait for the blitter to be idle.
 *
 * This function is called before memory that has been written to by the hardware is about to be
 * accessed by the CPU (software driver) or another hardware entity like video encoder (by Flip()).
 * It can also be called by applications explicitly, e.g. at the end of a benchmark loop to include
 * execution time of queued commands in the measurement.
 */
DFBResult
davinciEngineSync( void *drv, void *dev )
{
     DFBResult          ret;
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s()\n", __FUNCTION__ );

     if (!ddev->synced) {
          D_DEBUG_AT( Davinci_2D, "  -> syncing...\n" );

          ret = davinci_c64x_wait_low( &ddrv->c64x );
          if (ret) {
               D_DEBUG_AT( Davinci_2D, "  -> ERROR (%s)\n", DirectFBErrorString(ret) );
               return ret;
          }

          D_DEBUG_AT( Davinci_2D, "  => syncing done.\n" );

          ddev->synced = true;
     }
     else
          D_DEBUG_AT( Davinci_2D, "  => already synced!\n" );

     return DFB_OK;
}

/*
 * Reset the graphics engine.
 */
void
davinciEngineReset( void *drv, void *dev )
{
     D_DEBUG_AT( Davinci_2D, "%s()\n", __FUNCTION__ );
}

/*
 * Start processing of queued commands if required.
 *
 * This function is called before returning from the graphics core to the application.
 * Usually that's after each rendering function. The only functions causing multiple commands
 * to be queued with a single emition at the end are DrawString(), TileBlit(), BatchBlit(),
 * DrawLines() and possibly FillTriangle() which is emulated using multiple FillRectangle() calls.
 */
void
davinciEmitCommands( void *drv, void *dev )
{
     DFBResult          ret;
     DavinciDeviceData *ddev = dev;
     DavinciDriverData *ddrv = drv;

     D_DEBUG_AT( Davinci_2D, "%s()\n", __FUNCTION__ );

     ret = davinci_c64x_emit_tasks( &ddrv->c64x, &ddrv->tasks, C64X_TEF_RESET );
     if (ret)
          D_DERROR( ret, "Davinci/Driver: Error emitting local task buffer!\n" );

     ddev->synced = false;
}

/*
 * Invalidate the DSP's read cache.
 */
void
davinciFlushTextureCache( void *drv, void *dev )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s()\n", __FUNCTION__ );

     /* Bad workaround */
     davinci_c64x_blit_32( &ddrv->c64x, dfb_config->video_phys, 1024, dfb_config->video_phys, 1024, 256, 64 );

     /* These don't work */
//     davinci_c64x_wb_inv_range( &ddrv->c64x, dfb_config->video_phys,
//                                             dfb_config->video_length, 2 );

//     davinci_c64x_wb_inv_range( &ddrv->c64x, ddev->fix[OSD0].smem_start,
//                                             ddev->fix[OSD0].smem_len, 2 );
}

/*
 * Check for acceleration of 'accel' using the given 'state'.
 */
void
davinciCheckState( void                *drv,
                   void                *dev,
                   CardState           *state,
                   DFBAccelerationMask  accel )
{
     D_DEBUG_AT( Davinci_2D, "%s( state %p, accel 0x%08x ) <- dest %p\n",
                 __FUNCTION__, state, accel, state->destination );

     /* Return if the desired function is not supported at all. */
     if (accel & ~(DAVINCI_SUPPORTED_DRAWINGFUNCTIONS | DAVINCI_SUPPORTED_BLITTINGFUNCTIONS))
          return;

     /* Return if the destination format is not supported. */
     switch (state->destination->config.format) {
          case DSPF_UYVY:
          case DSPF_RGB16:
          case DSPF_RGB32:
          case DSPF_ARGB:
               break;

          default:
               return;
     }

     /* Check if drawing or blitting is requested. */
     if (DFB_DRAWING_FUNCTION( accel )) {
          /* Return if unsupported drawing flags are set. */
          if (state->drawingflags & ~DAVINCI_SUPPORTED_DRAWINGFLAGS)
               return;

          /* Limited blending support. */
          if (state->drawingflags & (DSDRAW_BLEND | DSDRAW_SRC_PREMULTIPLY)) {
               if (state->destination->config.format != DSPF_ARGB)
                    return;

               if (get_draw_blend_sub_function( state ) < 0)
                    return;
          }
     }
     else {
          /* Return if unsupported blitting flags are set. */
          if (state->blittingflags & ~DAVINCI_SUPPORTED_BLITTINGFLAGS)
               return;

          /* No other flags supported when color keying is used. */
          if ((state->blittingflags & DSBLIT_SRC_COLORKEY) && state->blittingflags != DSBLIT_SRC_COLORKEY)
               return;

          /* Return if the source format is not supported. */
          switch (state->source->config.format) {
               case DSPF_UYVY:
               case DSPF_RGB16:
                    /* Only color keying for these formats. */
                    if (state->blittingflags & ~DSBLIT_SRC_COLORKEY)
                         return;
                    /* No format conversion supported. */
                    if (state->source->config.format != state->destination->config.format)
                         return;
                    break;

               case DSPF_RGB32:
                    /* Only color keying for these formats. */
                    if (state->blittingflags & ~DSBLIT_SRC_COLORKEY)
                         return;
                    /* fall through */
               case DSPF_ARGB:
                    /* Only few blending combinations are valid. */
                    if ((state->blittingflags & ~DSBLIT_SRC_COLORKEY) && get_blit_blend_sub_function( state ) < 0)
                         return;
                    /* Only ARGB/RGB32 -> RGB16 conversion (without any flag). */
                    if (state->source->config.format != state->destination->config.format &&
                        (state->destination->config.format != DSPF_RGB16 || state->blittingflags))
                         return;
                    break;

               default:
                    return;
          }

          /* Checks per function. */
          switch (accel) {
               case DFXL_STRETCHBLIT:
                    /* No flags supported with StretchBlit(). */
                    if (state->blittingflags)
                         return;

                    /* Only (A)RGB at 32 bit supported. */
                    if (state->source->config.format != DSPF_ARGB && state->source->config.format != DSPF_RGB32)
                         return;

                    break;

               default:
                    break;
          }
     }

     /* Enable acceleration of the function. */
     state->accel |= accel;

     D_DEBUG_AT( Davinci_2D, "  => accel 0x%08x\n", state->accel );
}

/*
 * Make sure that the hardware is programmed for execution of 'accel' according to the 'state'.
 */
void
davinciSetState( void                *drv,
                 void                *dev,
                 GraphicsDeviceFuncs *funcs,
                 CardState           *state,
                 DFBAccelerationMask  accel )
{
     DavinciDeviceData      *ddev     = dev;
     StateModificationFlags  modified = state->mod_hw;

     D_DEBUG_AT( Davinci_2D, "%s( state %p, accel 0x%08x ) <- dest %p, modified 0x%08x\n",
                 __FUNCTION__, state, accel, state->destination, modified );

     /*
      * 1) Invalidate hardware states
      *
      * Each modification to the hw independent state invalidates one or more hardware states.
      */

     /* Simply invalidate all? */
     if (modified == SMF_ALL) {
          D_DEBUG_AT( Davinci_2D, "  <- ALL\n" );

          DAVINCI_INVALIDATE( ALL );
     }
     else if (modified) {
          /* Invalidate destination settings. */
          if (modified & SMF_DESTINATION) {
               D_DEBUG_AT( Davinci_2D, "  <- DESTINATION | FILLCOLOR\n" );

               DAVINCI_INVALIDATE( DESTINATION | FILLCOLOR );
          }
          else if (modified & SMF_COLOR) {
               D_DEBUG_AT( Davinci_2D, "  <- FILLCOLOR\n" );

               DAVINCI_INVALIDATE( FILLCOLOR );
          }

          /* Invalidate source settings. */
          if (modified & SMF_SOURCE) {
               D_DEBUG_AT( Davinci_2D, "  <- SOURCE\n" );

               DAVINCI_INVALIDATE( SOURCE );
          }

          /* Invalidate source color(ize) settings. */
          if (modified & (SMF_BLITTING_FLAGS | SMF_COLOR)) {
               D_DEBUG_AT( Davinci_2D, "  <- SOURCE_MULT\n" );

               DAVINCI_INVALIDATE( SOURCE_MULT );
          }

          /* Invalidate blend function for blitting. */
          if (modified & (SMF_BLITTING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND)) {
               D_DEBUG_AT( Davinci_2D, "  <- BLIT_BLEND_SUB\n" );

               DAVINCI_INVALIDATE( BLIT_BLEND_SUB );
          }

          /* Invalidate blend function for drawing. */
          if (modified & (SMF_DRAWING_FLAGS | SMF_SRC_BLEND | SMF_DST_BLEND)) {
               D_DEBUG_AT( Davinci_2D, "  <- DRAW_BLEND_SUB\n" );

               DAVINCI_INVALIDATE( DRAW_BLEND_SUB );
          }
     }

     /*
      * Just keep these values, no computations needed here.
      * Values used by state validation or rendering functions.
      */
     ddev->blitting_flags = state->blittingflags;
     ddev->clip           = state->clip;
     ddev->color          = state->color;
     ddev->colorkey       = state->src_colorkey;
     ddev->color_argb     = PIXEL_ARGB( state->color.a,
                                        state->color.r,
                                        state->color.g,
                                        state->color.b );

     /*
      * 2) Validate hardware states
      *
      * Each function has its own set of states that need to be validated.
      */

     /* Always requiring valid destination... */
     DAVINCI_CHECK_VALIDATE( DESTINATION );

     /* Depending on the function... */
     switch (accel) {
          case DFXL_FILLRECTANGLE:
               D_DEBUG_AT( Davinci_2D, "  -> FILLRECTANGLE\n" );

               /* Validate blend sub function index for drawing... */
               if (state->drawingflags & (DSDRAW_BLEND | DSDRAW_SRC_PREMULTIPLY))
                    DAVINCI_CHECK_VALIDATE( DRAW_BLEND_SUB );
               else
                    /* ...or just validate fill color. */
                    DAVINCI_CHECK_VALIDATE( FILLCOLOR );

               /* Choose function. */
               switch (DFB_BYTES_PER_PIXEL( state->destination->config.format )) {
                    case 2:
                         funcs->FillRectangle = davinciFillRectangle16;
                         break;

                    case 4:
                         if (state->drawingflags & (DSDRAW_BLEND | DSDRAW_SRC_PREMULTIPLY))
                              funcs->FillRectangle = davinciFillRectangleBlend32;
                         else
                              funcs->FillRectangle = davinciFillRectangle32;
                         break;

                    default:
                         D_BUG( "unexpected destination bpp %d",
                                DFB_BYTES_PER_PIXEL( state->destination->config.format ) );
                         break;
               }

               /*
                * 3) Tell which functions can be called without further validation, i.e. SetState()
                *
                * When the hw independent state is changed, this collection is reset.
                */
               state->set |= DAVINCI_SUPPORTED_DRAWINGFUNCTIONS;
               break;

          case DFXL_BLIT:
               D_DEBUG_AT( Davinci_2D, "  -> BLIT\n" );

               /* ...require valid source. */
               DAVINCI_CHECK_VALIDATE( SOURCE );

               /* Validate blend sub function index for blitting. */
               if (state->blittingflags & DSBLIT_BLEND_ALPHACHANNEL) {
                    DAVINCI_CHECK_VALIDATE( BLIT_BLEND_SUB );

                    /* Validate ARGB source modulator. */
                    DAVINCI_CHECK_VALIDATE( SOURCE_MULT );
               }

               /* Choose function. */
               switch (DFB_BYTES_PER_PIXEL( state->destination->config.format )) {
                    case 2:
                         if (state->blittingflags & DSBLIT_SRC_COLORKEY)
                              funcs->Blit = davinciBlitKeyed16;
                         else if (state->source->config.format == DSPF_ARGB ||
                                  state->source->config.format == DSPF_RGB32)
                              funcs->Blit = davinciBlit32to16;
                         else
                              funcs->Blit = davinciBlit16;
                         break;

                    case 4:
                         if (state->blittingflags & DSBLIT_SRC_COLORKEY)
                              funcs->Blit = davinciBlitKeyed32;
                         else if (state->blittingflags & DSBLIT_BLEND_ALPHACHANNEL)
                              funcs->Blit = davinciBlitBlend32;
                         else
                              funcs->Blit = davinciBlit32;
                         break;

                    default:
                         D_BUG( "unexpected destination bpp %d",
                                DFB_BYTES_PER_PIXEL( state->destination->config.format ) );
                         break;
               }

               /*
                * 3) Tell which functions can be called without further validation, i.e. SetState()
                *
                * When the hw independent state is changed, this collection is reset.
                */
               state->set |= DFXL_BLIT;
               break;

          case DFXL_STRETCHBLIT:
               D_DEBUG_AT( Davinci_2D, "  -> STRETCHBLIT\n" );

               /* ...require valid source. */
               DAVINCI_CHECK_VALIDATE( SOURCE );

               /* Choose function. */
#if 0 // only 32bit, statically set in driver_init_driver()
               switch (state->destination->config.format) {
                    case DSPF_ARGB:
                    case DSPF_RGB32:
                         funcs->StretchBlit = davinciStretchBlit32;
                         break;

                    default:
                         D_BUG( "unexpected destination format %s",
                                dfb_pixelformat_name( state->destination->config.format ) );
                         break;
               }
#endif

               /*
                * 3) Tell which functions can be called without further validation, i.e. SetState()
                *
                * When the hw independent state is changed, this collection is reset.
                */
               state->set |= DFXL_STRETCHBLIT;
               break;

          default:
               D_BUG( "unexpected drawing/blitting function" );
               break;
     }

     /*
      * 4) Clear modification flags
      *
      * All flags have been evaluated in 1) and remembered for further validation.
      * If the hw independent state is not modified, this function won't get called
      * for subsequent rendering functions, unless they aren't defined by 3).
      */
     state->mod_hw = 0;
}

/**********************************************************************************************************************/
/**********************************************************************************************************************/

/*
 * Render a filled rectangle using the current hardware state.
 */
static bool
davinciFillRectangle16( void *drv, void *dev, DFBRectangle *rect )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d )\n", __FUNCTION__, DFB_RECTANGLE_VALS(rect) );

     /* FIXME: Optimize in DSP. */
     if ((rect->x | rect->w) & 1)
          davinci_c64x_fill_16__L( &ddrv->tasks,
                                   ddev->dst_phys + ddev->dst_pitch * rect->y + ddev->dst_bpp * rect->x,
                                   ddev->dst_pitch,
                                   rect->w, rect->h,
                                   ddev->fillcolor );
     else
          davinci_c64x_fill_32__L( &ddrv->tasks,
                                   ddev->dst_phys + ddev->dst_pitch * rect->y + ddev->dst_bpp * rect->x,
                                   ddev->dst_pitch,
                                   rect->w/2, rect->h,
                                   ddev->fillcolor );

     return true;
}

static bool
davinciFillRectangle32( void *drv, void *dev, DFBRectangle *rect )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d )\n", __FUNCTION__, DFB_RECTANGLE_VALS(rect) );

     if (ddev->dst_format == DSPF_ARGB && ddev->color.a == 0xff)
          davinci_c64x_blit_blend_32__L( &ddrv->tasks,
                                         C64X_BLEND_ONE_INVSRC,
                                         ddev->dst_phys + ddev->dst_pitch * rect->y + ddev->dst_bpp * rect->x,
                                         ddev->dst_pitch,
                                         0,
                                         0,
                                         rect->w, rect->h,
                                         ddev->color_argb,
                                         0xff );
     else
          davinci_c64x_fill_32__L( &ddrv->tasks,
                                   ddev->dst_phys + ddev->dst_pitch * rect->y + ddev->dst_bpp * rect->x,
                                   ddev->dst_pitch,
                                   rect->w, rect->h,
                                   ddev->fillcolor );

     return true;
}

/**********************************************************************************************************************/

static bool
davinciFillRectangleBlend32( void *drv, void *dev, DFBRectangle *rect )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d )\n", __FUNCTION__, DFB_RECTANGLE_VALS(rect) );

     davinci_c64x_blit_blend_32__L( &ddrv->tasks,
                                    ddev->draw_blend_sub_function,
                                    ddev->dst_phys + ddev->dst_pitch * rect->y + ddev->dst_bpp * rect->x,
                                    ddev->dst_pitch,
                                    0,
                                    0,
                                    rect->w, rect->h,
                                    ddev->color_argb,
                                    0xff );

     return true;
}

/**********************************************************************************************************************/
/**********************************************************************************************************************/

/*
 * Blit a rectangle using the current hardware state.
 */
static bool
davinciBlit16( void *drv, void *dev, DFBRectangle *rect, int dx, int dy )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d <- %4d,%4d )\n",
                 __FUNCTION__, dx, dy, rect->w, rect->h, rect->x, rect->y );

     /* FIXME: Optimize in DSP. */
     if ((dx | rect->x | rect->w) & 1)
          davinci_c64x_blit_16__L( &ddrv->tasks,
                                   ddev->dst_phys + ddev->dst_pitch * dy      + ddev->dst_bpp * dx,
                                   ddev->dst_pitch,
                                   ddev->src_phys + ddev->src_pitch * rect->y + ddev->src_bpp * rect->x,
                                   ddev->src_pitch,
                                   rect->w, rect->h );
     else
          davinci_c64x_blit_32__L( &ddrv->tasks,
                                   ddev->dst_phys + ddev->dst_pitch * dy      + ddev->dst_bpp * dx,
                                   ddev->dst_pitch,
                                   ddev->src_phys + ddev->src_pitch * rect->y + ddev->src_bpp * rect->x,
                                   ddev->src_pitch,
                                   rect->w/2, rect->h );

     return true;
}

static bool
davinciBlit32( void *drv, void *dev, DFBRectangle *rect, int dx, int dy )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d <- %4d,%4d )\n",
                 __FUNCTION__, dx, dy, rect->w, rect->h, rect->x, rect->y );

     davinci_c64x_blit_32__L( &ddrv->tasks,
                              ddev->dst_phys + ddev->dst_pitch * dy      + ddev->dst_bpp * dx,
                              ddev->dst_pitch,
                              ddev->src_phys + ddev->src_pitch * rect->y + ddev->src_bpp * rect->x,
                              ddev->src_pitch,
                              rect->w, rect->h );

     return true;
}

/**********************************************************************************************************************/

static bool
davinciBlit32to16( void *drv, void *dev, DFBRectangle *rect, int dx, int dy )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d <- %4d,%4d )\n",
                 __FUNCTION__, dx, dy, rect->w, rect->h, rect->x, rect->y );

     davinci_c64x_dither_argb__L( &ddrv->tasks,
                                  ddev->dst_phys + ddev->dst_pitch * dy      + ddev->dst_bpp * dx,
                                  DAVINCI_C64X_MEM,
                                  ddev->dst_pitch,
                                  ddev->src_phys + ddev->src_pitch * rect->y + ddev->src_bpp * rect->x,
                                  ddev->src_pitch,
                                  rect->w, rect->h );

     return true;
}

/**********************************************************************************************************************/

static bool
davinciBlitKeyed16( void *drv, void *dev, DFBRectangle *rect, int dx, int dy )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d <- %4d,%4d ) <- key 0x%04lx\n",
                 __FUNCTION__, dx, dy, rect->w, rect->h, rect->x, rect->y, ddev->colorkey );

     davinci_c64x_blit_keyed_16__L( &ddrv->tasks,
                                    ddev->dst_phys + ddev->dst_pitch * dy      + ddev->dst_bpp * dx,
                                    ddev->dst_pitch,
                                    ddev->src_phys + ddev->src_pitch * rect->y + ddev->src_bpp * rect->x,
                                    ddev->src_pitch,
                                    rect->w, rect->h,
                                    ddev->colorkey,
                                    (1 << DFB_COLOR_BITS_PER_PIXEL( ddev->dst_format )) - 1 );

     return true;
}

static bool
davinciBlitKeyed32( void *drv, void *dev, DFBRectangle *rect, int dx, int dy )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d <- %4d,%4d ) <- key 0x%08lx\n",
                 __FUNCTION__, dx, dy, rect->w, rect->h, rect->x, rect->y, ddev->colorkey );

     davinci_c64x_blit_keyed_32__L( &ddrv->tasks,
                                    ddev->dst_phys + ddev->dst_pitch * dy      + ddev->dst_bpp * dx,
                                    ddev->dst_pitch,
                                    ddev->src_phys + ddev->src_pitch * rect->y + ddev->src_bpp * rect->x,
                                    ddev->src_pitch,
                                    rect->w, rect->h,
                                    ddev->colorkey,
                                    (1 << DFB_COLOR_BITS_PER_PIXEL( ddev->dst_format )) - 1 );

     return true;
}

/**********************************************************************************************************************/

static bool
davinciBlitBlend32( void *drv, void *dev, DFBRectangle *rect, int dx, int dy )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d <- %4d,%4d )\n",
                 __FUNCTION__, dx, dy, rect->w, rect->h, rect->x, rect->y );

     davinci_c64x_blit_blend_32__L( &ddrv->tasks,
                                    ddev->blit_blend_sub_function,
                                    ddev->dst_phys + ddev->dst_pitch * dy      + ddev->dst_bpp * dx,
                                    ddev->dst_pitch,
                                    ddev->src_phys + ddev->src_pitch * rect->y + ddev->src_bpp * rect->x,
                                    ddev->src_pitch,
                                    rect->w, rect->h,
                                    ddev->source_mult,
                                    ddev->color.a );

     return true;
}

/**********************************************************************************************************************/
/**********************************************************************************************************************/

bool
davinciStretchBlit32( void *drv, void *dev, DFBRectangle *srect, DFBRectangle *drect )
{
     DavinciDriverData *ddrv = drv;
     DavinciDeviceData *ddev = dev;

     DFBRegion clip = DFB_REGION_INIT_FROM_RECTANGLE( drect );

     D_DEBUG_AT( Davinci_2D, "%s( %4d,%4d-%4dx%4d <- %4d,%4d-%4dx%4d )\n",
                 __FUNCTION__, DFB_RECTANGLE_VALS(drect), DFB_RECTANGLE_VALS(srect) );

     if (!dfb_region_region_intersect( &clip, &ddev->clip ))
          return true;

     dfb_region_translate( &clip, -drect->x, -drect->y );

     davinci_c64x_stretch_32__L( &ddrv->tasks,
                                 ddev->dst_phys + ddev->dst_pitch * drect->y + ddev->dst_bpp * drect->x,
                                 ddev->dst_pitch,
                                 ddev->src_phys + ddev->src_pitch * srect->y + ddev->src_bpp * srect->x,
                                 ddev->src_pitch,
                                 drect->w, drect->h,
                                 srect->w, srect->h,
                                 &clip );

     return true;
}

