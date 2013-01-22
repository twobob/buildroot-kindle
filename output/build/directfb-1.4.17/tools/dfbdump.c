/*
   (c) Copyright 2001-2010  The world wide DirectFB Open Source Community (directfb.org)
   (c) Copyright 2000-2004  Convergence (integrated media) GmbH

   All rights reserved.

   Written by Denis Oliver Kropp <dok@directfb.org>,
              Andreas Hundt <andi@fischlustig.de>,
              Sven Neumann <neo@directfb.org>,
              Ville Syrjälä <syrjala@sci.fi> and
              Claudio Ciccani <klan@users.sf.net>.

   This file is subject to the terms and conditions of the MIT License:

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without restriction,
   including without limitation the rights to use, copy, modify, merge,
   publish, distribute, sublicense, and/or sell copies of the Software,
   and to permit persons to whom the Software is furnished to do so,
   subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
   CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
   TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
   SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <directfb.h>
#include <directfb_strings.h>

#include <direct/clock.h>
#include <direct/debug.h>

#include <fusion/build.h>
#include <fusion/fusion.h>
#include <fusion/object.h>
#include <fusion/ref.h>
#include <fusion/shmalloc.h>
#include <fusion/shm/shm.h>
#include <fusion/shm/shm_internal.h>

#include <core/CoreLayer.h>

#include <core/core.h>
#include <core/layer_control.h>
#include <core/layer_context.h>
#include <core/layers.h>
#include <core/layers_internal.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_pool.h>
#include <core/windows.h>
#include <core/windowstack.h>
#include <core/windows_internal.h>
#include <core/wm.h>

static DirectFBPixelFormatNames( format_names );

/**********************************************************************************************************************/

typedef struct {
     int video;
     int system;
     int presys;
} MemoryUsage;

/**********************************************************************************************************************/

static IDirectFB *dfb;

static MemoryUsage mem;

static bool loop_mode;
static bool show_shm;
static bool show_pools;
static bool show_allocs;
static int  dump_layer;       /* ref or -1 (all) or 0 (none) */
static int  dump_surface;     /* ref or -1 (all) or 0 (none) */

/**********************************************************************************************************************/

static DFBBoolean parse_command_line( int argc, char *argv[] );

/**********************************************************************************************************************/

static inline int
buffer_size( CoreSurface *surface, CoreSurfaceBuffer *buffer, bool video )
{
     int                    i, mem = 0;
     CoreSurfaceAllocation *allocation;

     fusion_vector_foreach (allocation, i, buffer->allocs) {
          int size = allocation->size;

          if (video) {
               if (allocation->access[CSAID_GPU])
                    mem += size;
          }
          else if (!allocation->access[CSAID_GPU])
               mem += size;
     }

     return mem;
}

static int
buffer_sizes( CoreSurface *surface, bool video )
{
     int i, mem = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          mem += buffer_size( surface, buffer, video );
     }

     return mem;
}

static int
buffer_locks( CoreSurface *surface, bool video )
{
     int i, j, locks = 0;

     for (i=0; i<surface->num_buffers; i++) {
          CoreSurfaceBuffer *buffer = surface->buffers[i];

          for (j=0; j<buffer->allocs.count; j++) {
               CoreSurfaceAllocation *allocation = fusion_vector_at( &buffer->allocs, j );

               locks += dfb_surface_allocation_locks( allocation );
          }
     }

     return locks;
}

static bool
surface_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult ret;
     int          i;
     int          refs;
     CoreSurface *surface = (CoreSurface*) object;
     MemoryUsage *mem     = ctx;
     int          vmem;
     int          smem;

     if (object->state != FOS_ACTIVE)
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }

     if (dump_surface && ((dump_surface < 0 && surface->type & CSTF_SHARED) ||
                          (dump_surface == object->ref.multi.id)) && surface->num_buffers)
     {
          char buf[32];

          snprintf( buf, sizeof(buf), "dfb_surface_0x%08x", object->ref.multi.id );

          dfb_surface_dump_buffer( surface, CSBR_FRONT, ".", buf );
     }

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               printf( "%8s ", format_names[i].name );
     }

     printf( "%5d  ", surface->object.id );

     vmem = buffer_sizes( surface, true );
     smem = buffer_sizes( surface, false );

     mem->video += vmem;

     /* FIXME: assumes all buffers have this flag (or none) */
     /*if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
          mem->presys += smem;
     else*/
          mem->system += smem;

     if (vmem && vmem < 1024)
          vmem = 1024;

     if (smem && smem < 1024)
          smem = 1024;

     printf( "%5dk%c  ", vmem >> 10, buffer_locks( surface, true ) ? '*' : ' ' );
     printf( "%5dk%c  ", smem >> 10, buffer_locks( surface, false ) ? '*' : ' ' );

     /* FIXME: assumes all buffers have this flag (or none) */
//     if (surface->front_buffer->flags & SBF_FOREIGN_SYSTEM)
//          printf( "preallocated " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          printf( "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          printf( "video only   " );

     if (surface->config.caps & DSCAPS_INTERLACED)
          printf( "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          printf( "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          printf( "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          printf( "premultiplied" );

     printf( "\n" );

     return true;
}

static void
dump_surfaces( void )
{
     printf( "\n"
             "-----------------------------[ Surfaces ]--------------------------------------------\n" );
     printf( "Reference   FID  . Refs  Width Height  Format     ID     Video   System  Capabilities\n" );
     printf( "-------------------------------------------------------------------------------------\n" );

     dfb_core_enum_surfaces( NULL, surface_callback, &mem );

     printf( "                                                       ------   ------\n" );
     printf( "                                                      %6dk  %6dk   -> %dk total\n",
             mem.video >> 10, (mem.system + mem.presys) >> 10,
             (mem.video + mem.system + mem.presys) >> 10);
}

/**********************************************************************************************************************/

static DFBEnumerationResult
alloc_callback( CoreSurfaceAllocation *alloc,
                void                  *ctx )
{
     int                i, index;
     CoreSurface       *surface;
     CoreSurfaceBuffer *buffer;

     D_MAGIC_ASSERT( alloc, CoreSurfaceAllocation );

     buffer  = alloc->buffer;
     D_MAGIC_ASSERT( buffer, CoreSurfaceBuffer );

     surface = buffer->surface;
     D_MAGIC_ASSERT( surface, CoreSurface );

     printf( "%9lu %8d  ", alloc->offset, alloc->size );

     printf( "%4d x %4d   ", surface->config.size.w, surface->config.size.h );

     for (i=0; format_names[i].format; i++) {
          if (surface->config.format == format_names[i].format)
               printf( "%8s ", format_names[i].name );
     }

     index = dfb_surface_buffer_index( alloc->buffer );

     printf( " %-5s ",
             (dfb_surface_get_buffer( surface, CSBR_FRONT ) == buffer) ? "front" :
             (dfb_surface_get_buffer( surface, CSBR_BACK  ) == buffer) ? "back"  :
             (dfb_surface_get_buffer( surface, CSBR_IDLE  ) == buffer) ? "idle"  : "" );

     printf( direct_serial_check(&alloc->serial, &buffer->serial) ? " * " : "   " );

     printf( "%d  %2lu  ", fusion_vector_size( &buffer->allocs ), surface->resource_id );

     if (surface->type & CSTF_SHARED)
          printf( "SHARED  " );
     else
          printf( "PRIVATE " );

     if (surface->type & CSTF_LAYER)
          printf( "LAYER " );

     if (surface->type & CSTF_WINDOW)
          printf( "WINDOW " );

     if (surface->type & CSTF_CURSOR)
          printf( "CURSOR " );

     if (surface->type & CSTF_FONT)
          printf( "FONT " );

     printf( " " );

     if (surface->type & CSTF_INTERNAL)
          printf( "INTERNAL " );

     if (surface->type & CSTF_EXTERNAL)
          printf( "EXTERNAL " );

     printf( " " );

     if (surface->config.caps & DSCAPS_SYSTEMONLY)
          printf( "system only  " );

     if (surface->config.caps & DSCAPS_VIDEOONLY)
          printf( "video only   " );

     if (surface->config.caps & DSCAPS_INTERLACED)
          printf( "interlaced   " );

     if (surface->config.caps & DSCAPS_DOUBLE)
          printf( "double       " );

     if (surface->config.caps & DSCAPS_TRIPLE)
          printf( "triple       " );

     if (surface->config.caps & DSCAPS_PREMULTIPLIED)
          printf( "premultiplied" );

     printf( "\n" );

     return DFENUM_OK;
}

static DFBEnumerationResult
surface_pool_callback( CoreSurfacePool *pool,
                       void            *ctx )
{
     int length;

     printf( "\n" );
     printf( "--------------------[ Surface Buffer Allocations in %s ]-------------------%n\n", pool->desc.name, &length );
     printf( "Offset    Length   Width Height     Format  Role  Up nA ID  Usage   Type / Storage / Caps\n" );

     while (length--)
          putc( '-', stdout );

     printf( "\n" );

     dfb_surface_pool_enumerate( pool, alloc_callback, NULL );

     return DFENUM_OK;
}

static void
dump_surface_pools( void )
{
     dfb_surface_pools_enumerate( surface_pool_callback, NULL );
}

/**********************************************************************************************************************/

static DFBEnumerationResult
surface_pool_info_callback( CoreSurfacePool *pool,
                       void            *ctx )
{
     int                    i;
     unsigned long          total = 0;
     CoreSurfaceAllocation *alloc;

     fusion_vector_foreach (alloc, i, pool->allocs)
          total += alloc->size;

     printf( "%-20s ", pool->desc.name );

     switch (pool->desc.priority) {
          case CSPP_DEFAULT:
               printf( "DEFAULT  " );
               break;

          case CSPP_PREFERED:
               printf( "PREFERED " );
               break;

          case CSPP_ULTIMATE:
               printf( "ULTIMATE " );
               break;

          default:
               printf( "unknown  " );
               break;
     }

     printf( "%6lu/%6luk  ", total / 1024, pool->desc.size / 1024 );

     if (pool->desc.types & CSTF_SHARED)
          printf( "* " );
     else
          printf( "  " );


     if (pool->desc.types & CSTF_INTERNAL)
          printf( "INT " );

     if (pool->desc.types & CSTF_EXTERNAL)
          printf( "EXT " );

     if (!(pool->desc.types & (CSTF_INTERNAL | CSTF_EXTERNAL)))
          printf( "    " );


     if (pool->desc.types & CSTF_LAYER)
          printf( "LAYER " );
     else
          printf( "      " );

     if (pool->desc.types & CSTF_WINDOW)
          printf( "WINDOW " );
     else
          printf( "       " );

     if (pool->desc.types & CSTF_CURSOR)
          printf( "CURSOR " );
     else
          printf( "       " );

     if (pool->desc.types & CSTF_FONT)
          printf( "FONT " );
     else
          printf( "     " );


     for (i=CSAID_CPU; i<=CSAID_GPU; i++) {
          printf( " %c%c%c",
                  (pool->desc.access[i] & CSAF_READ)   ? 'r' : '-',
                  (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '-',
                  (pool->desc.access[i] & CSAF_SHARED) ? 's' : '-' );
     }

     for (i=CSAID_LAYER0; i<=CSAID_LAYER15; i++) {
          printf( " %c%c%c",
                  (pool->desc.access[i] & CSAF_READ)   ? 'r' : '-',
                  (pool->desc.access[i] & CSAF_WRITE)  ? 'w' : '-',
                  (pool->desc.access[i] & CSAF_SHARED) ? 's' : '-' );
     }

     printf( "\n" );

     return DFENUM_OK;
}

static void
dump_surface_pool_info( void )
{
     printf( "\n" );
     printf( "-------------------------------------[ Surface Buffer Pools ]------------------------------------\n" );
     printf( "Name                 Priority   Used/Capacity S I/E Resource Type Support     CPU GPU Layer 0 - 2\n" );
     printf( "-------------------------------------------------------------------------------------------------\n" );

     dfb_surface_pools_enumerate( surface_pool_info_callback, NULL );
}

/**********************************************************************************************************************/

static bool
context_callback( FusionObjectPool *pool,
                  FusionObject     *object,
                  void             *ctx )
{
     DirectResult       ret;
     int                i;
     int                refs;
     int                level;
     CoreLayer         *layer   = (CoreLayer*) ctx;
     CoreLayerContext  *context = (CoreLayerContext*) object;
     CoreLayerRegion   *region  = NULL;
     CoreSurface       *surface = NULL;

     if (object->state != FOS_ACTIVE)
          return true;

     if (context->layer_id != dfb_layer_id( layer ))
          return true;

     ret = fusion_ref_stat( &object->ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return false;
     }

     if (dump_layer && (dump_layer < 0 || dump_layer == object->ref.multi.id)) {
          if (dfb_layer_context_get_primary_region( context, false, &region ) == DFB_OK) {
               if (dfb_layer_region_get_surface( region, &surface ) == DFB_OK) {
                    if (surface->num_buffers) {
                         char buf[32];
                         
                         snprintf( buf, sizeof(buf), "dfb_layer_context_0x%08x", object->ref.multi.id );

                         dfb_surface_dump_buffer( surface, CSBR_FRONT, ".", buf );
                    }

                    dfb_surface_unref( surface );
               }
          }
     }

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", object->ref.multi.id, object->ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d x %4d  ", context->config.width, context->config.height );

     for (i=0; format_names[i].format; i++) {
          if (context->config.pixelformat == format_names[i].format) {
               printf( "%-8s ", format_names[i].name );
               break;
          }
     }

     if (!format_names[i].format)
          printf( "unknown  " );

     printf( "%.1f, %.1f -> %.1f, %.1f   ",
             context->screen.location.x,  context->screen.location.y,
             context->screen.location.x + context->screen.location.w,
             context->screen.location.y + context->screen.location.h );

     printf( "%2d     ", fusion_vector_size( &context->regions ) );

     printf( context->active ? "(*)    " : "       " );

     if (context == layer->shared->contexts.primary)
          printf( "SHARED   " );
     else
          printf( "PRIVATE  " );

     if (context->rotation)
          printf( "ROTATED %d ", context->rotation);

     if (dfb_layer_get_level( layer, &level ))
          printf( "N/A" );
     else
          printf( "%3d", level );

     printf( "\n" );

     return true;
}

static void
dump_contexts( CoreLayer *layer )
{
     if (fusion_vector_size( &layer->shared->contexts.stack ) == 0)
          return;

     printf( "\n"
             "----------------------------------[ Contexts of Layer %d ]----------------------------------------\n", dfb_layer_id( layer ));
     printf( "Reference   FID  . Refs  Width Height Format   Location on screen  Regions  Active  Info    Level\n" );
     printf( "-------------------------------------------------------------------------------------------------\n" );

     dfb_core_enum_layer_contexts( NULL, context_callback, layer );
}

static DFBEnumerationResult
window_callback( CoreWindow *window,
                 void       *ctx )
{
     DirectResult      ret;
     int               refs;
     CoreWindowConfig *config = &window->config;
     DFBRectangle     *bounds = &config->bounds;

     ret = fusion_ref_stat( &window->object.ref, &refs );
     if (ret) {
          printf( "Fusion error %d!\n", ret );
          return DFENUM_OK;
     }

#if FUSION_BUILD_MULTI
     printf( "0x%08x [%3lx] : ", window->object.ref.multi.id, window->object.ref.multi.creator );
#else
     printf( "N/A              : " );
#endif

     printf( "%3d   ", refs );

     printf( "%4d, %4d   ", bounds->x, bounds->y );

     printf( "%4d x %4d    ", bounds->w, bounds->h );

     printf( "0x%02x ", config->opacity );

     printf( "%5d  ", window->id );

     switch (config->stacking) {
          case DWSC_UPPER:
               printf( "^  " );
               break;
          case DWSC_MIDDLE:
               printf( "-  " );
               break;
          case DWSC_LOWER:
               printf( "v  " );
               break;
          default:
               printf( "?  " );
               break;
     }

     if (window->caps & DWCAPS_ALPHACHANNEL)
          printf( "alphachannel   " );

     if (window->caps & DWCAPS_INPUTONLY)
          printf( "input only     " );

     if (window->caps & DWCAPS_DOUBLEBUFFER)
          printf( "double buffer  " );

     if (config->options & DWOP_GHOST)
          printf( "GHOST          " );

     if (DFB_WINDOW_FOCUSED( window ))
          printf( "FOCUSED        " );

     if (DFB_WINDOW_DESTROYED( window ))
          printf( "DESTROYED      " );

     if (window->config.rotation)
          printf( "ROTATED %d     ", window->config.rotation);

     printf( "\n" );

     return DFENUM_OK;
}

static void
dump_windows( CoreLayer *layer )
{
     DFBResult         ret;
     CoreLayerContext *context;
     CoreWindowStack  *stack;

     if (!layer->shared->contexts.primary)
          return;

     ret = CoreLayer_GetPrimaryContext( layer, false, &context );
     if (ret)
          return;

     stack = dfb_layer_context_windowstack( context );
     if (!stack) {
          dfb_layer_context_unref( context );
          return;
     }

     dfb_windowstack_lock( stack );

     if (stack->num) {
          printf( "\n"
                  "-----------------------------------[ Windows of Layer %d ]-----------------------------------------\n", dfb_layer_id( layer ) );
          printf( "Reference   FID  . Refs     X     Y   Width Height Opacity   ID     Capabilities   State & Options\n" );
          printf( "--------------------------------------------------------------------------------------------------\n" );

          dfb_wm_enum_windows( stack, window_callback, NULL );
     }

     dfb_windowstack_unlock( stack );

     dfb_layer_context_unref( context );
}

static DFBEnumerationResult
layer_callback( CoreLayer *layer,
                void      *ctx)
{
     dump_windows( layer );
     dump_contexts( layer );

     return DFENUM_OK;
}

static void
dump_layers( void )
{
     dfb_layers_enumerate( layer_callback, NULL );
}

/**********************************************************************************************************************/

#if FUSION_BUILD_MULTI
static DirectEnumerationResult
dump_shmpool( FusionSHMPool *pool,
              void          *ctx )
{
     DFBResult     ret;
     SHMemDesc    *desc;
     unsigned int  total = 0;
     int           length;
     FusionSHMPoolShared *shared = pool->shared;

     printf( "\n" );
     printf( "----------------------------[ Shared Memory in %s ]----------------------------%n\n", shared->name, &length );
     printf( "      Size          Address      Offset      Function                     FusionID\n" );

     while (length--)
          putc( '-', stdout );

     putc( '\n', stdout );

     ret = fusion_skirmish_prevail( &shared->lock );
     if (ret) {
          D_DERROR( ret, "Could not lock shared memory pool!\n" );
          return DFENUM_OK;
     }

     if (shared->allocs) {
          direct_list_foreach (desc, shared->allocs) {
               printf( " %9zu bytes at %p [%8lu] in %-30s [%3lx] (%s: %u)\n",
                       desc->bytes, desc->mem, (ulong)desc->mem - (ulong)shared->heap,
                       desc->func, desc->fid, desc->file, desc->line );

               total += desc->bytes;
          }

          printf( "   -------\n  %7dk total\n", total >> 10 );
     }

     printf( "\nShared memory file size: %dk\n", shared->heap->size >> 10 );

     fusion_skirmish_dismiss( &shared->lock );

     return DFENUM_OK;
}

static void
dump_shmpools( void )
{
     fusion_shm_enum_pools( dfb_core_world(NULL), dump_shmpool, NULL );
}
#endif

/**********************************************************************************************************************/

int
main( int argc, char *argv[] )
{
     DFBResult ret;
     long long millis;
     long int  seconds, minutes, hours, days;

     char *buffer = malloc( 0x100000 );

     setvbuf( stdout, buffer, _IOFBF, 0x100000 );

     /* Initialize DirectFB. */
     ret = DirectFBInit( &argc, &argv );
     if (ret) {
          DirectFBError( "DirectFBInit", ret );
          return -1;
     }

     /* Parse the command line. */
     if (!parse_command_line( argc, argv ))
          return -2;

     /* Create the super interface. */
     ret = DirectFBCreate( &dfb );
     if (ret) {
          DirectFBError( "DirectFBCreate", ret );
          return -3;
     }

     while (true) {
          millis = direct_clock_get_millis();

          seconds  = millis / 1000;
          millis  %= 1000;

          minutes  = seconds / 60;
          seconds %= 60;

          hours    = minutes / 60;
          minutes %= 60;

          days     = hours / 24;
          hours   %= 24;

          switch (days) {
               case 0:
                    printf( "\nDirectFB uptime: %02ld:%02ld:%02ld\n",
                            hours, minutes, seconds );
                    break;

               case 1:
                    printf( "\nDirectFB uptime: %ld day, %02ld:%02ld:%02ld\n",
                            days, hours, minutes, seconds );
                    break;

               default:
                    printf( "\nDirectFB uptime: %ld days, %02ld:%02ld:%02ld\n",
                            days, hours, minutes, seconds );
                    break;
          }

          dump_surfaces();
          fflush( stdout );

          dump_layers();
          fflush( stdout );

     #if FUSION_BUILD_MULTI
          if (show_shm) {
               printf( "\n" );
               dump_shmpools();
               fflush( stdout );
          }
     #endif

          if (show_pools) {
               printf( "\n" );
               dump_surface_pool_info();
               fflush( stdout );
          }

          if (show_allocs) {
               printf( "\n" );
               dump_surface_pools();
               fflush( stdout );
          }

          printf( "\n" );


          if (loop_mode) {
               usleep( 2000000 );

               /* Clear surface memory totals */
               memset( &mem, 0, sizeof(mem) );
          }
          else
               break;
     }

     /* DirectFB deinitialization. */
     if (dfb)
          dfb->Release( dfb );

     return ret;
}

/**********************************************************************************************************************/

static void
print_usage (const char *prg_name)
{
     fprintf (stderr, "\nDirectFB Dump (version %s)\n\n", DIRECTFB_VERSION);
     fprintf (stderr, "Usage: %s [options]\n\n", prg_name);
     fprintf (stderr, "Options:\n");
     fprintf (stderr, "   -l,  --loop         Run in loop mode, periodically dumping status (useful as secure master for debug)\n");
     fprintf (stderr, "   -s,  --shm          Show shared memory pool content (if debug enabled)\n");
     fprintf (stderr, "   -p,  --pools        Show information about surface pools\n");
     fprintf (stderr, "   -a,  --allocs       Show surface buffer allocations in surface pools\n");
     fprintf (stderr, "   -dl, --dumplayer    Dump surfaces of layer contexts into files (dfb_layer_context_REFID...)\n");
     fprintf (stderr, "   -ds, --dumpsurface  Dump surfaces (front buffers) into files (dfb_surface_REFID...)\n");
     fprintf (stderr, "   -h,  --help         Show this help message\n");
     fprintf (stderr, "   -v,  --version      Print version information\n");
     fprintf (stderr, "\n");
}

static DFBBoolean
parse_command_line( int argc, char *argv[] )
{
     int n;

     for (n = 1; n < argc; n++) {
          const char *arg = argv[n];

          if (strcmp (arg, "-h") == 0 || strcmp (arg, "--help") == 0) {
               print_usage (argv[0]);
               return DFB_FALSE;
          }

          if (strcmp (arg, "-v") == 0 || strcmp (arg, "--version") == 0) {
               fprintf (stderr, "dfbdump version %s\n", DIRECTFB_VERSION);
               return DFB_FALSE;
          }

          if (strcmp (arg, "-l") == 0 || strcmp (arg, "--loop") == 0) {
               loop_mode = true;
               continue;
          }

          if (strcmp (arg, "-s") == 0 || strcmp (arg, "--shm") == 0) {
               show_shm = true;
               continue;
          }

          if (strcmp (arg, "-p") == 0 || strcmp (arg, "--pools") == 0) {
               show_pools = true;
               continue;
          }

          if (strcmp (arg, "-a") == 0 || strcmp (arg, "--allocs") == 0) {
               show_allocs = true;
               continue;
          }

          if (strcmp (arg, "-dl") == 0 || strcmp (arg, "--dumplayer") == 0) {
               dump_layer = -1;
               continue;
          }

          if (strcmp (arg, "-ds") == 0 || strcmp (arg, "--dumpsurface") == 0) {
               dump_surface = -1;
               continue;
          }

          print_usage (argv[0]);

          return DFB_FALSE;
     }

     return DFB_TRUE;
}

