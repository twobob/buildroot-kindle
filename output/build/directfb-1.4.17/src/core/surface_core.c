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
#include <direct/mem.h>
#include <direct/messages.h>

#include <fusion/conf.h>
#include <fusion/shmalloc.h>

#include <core/core_parts.h>
#include <core/surface.h>
#include <core/surface_buffer.h>
#include <core/surface_core.h>
#include <core/surface_pool.h>
#include <core/surface_pool_bridge.h>


#if FUSION_BUILD_MULTI
extern SurfacePoolFuncs sharedSurfacePoolFuncs;
extern SurfacePoolFuncs sharedSecureSurfacePoolFuncs;
#else
extern SurfacePoolFuncs localSurfacePoolFuncs;
#endif
extern SurfacePoolFuncs preallocSurfacePoolFuncs;

extern const SurfacePoolBridgeFuncs *preallocSurfacePoolBridgeFuncs;

D_DEBUG_DOMAIN( Core_Surface, "Core/SurfaceCore", "DirectFB Surface Core" );

/**********************************************************************************************************************/

DFB_CORE_PART( surface_core, SurfaceCore );

/**********************************************************************************************************************/

static DFBResult
dfb_surface_core_initialize( CoreDFB              *core,
                             DFBSurfaceCore       *data,
                             DFBSurfaceCoreShared *shared )
{
     DFBResult ret;

     D_DEBUG_AT( Core_Surface, "dfb_surface_core_initialize( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_ASSERT( shared != NULL );

     data->core   = core;
     data->shared = shared;

#if FUSION_BUILD_MULTI
     if (fusion_config->secure_fusion) {
          ret = dfb_surface_pool_initialize2( core, &sharedSecureSurfacePoolFuncs, data, &shared->surface_pool );
          if (ret) {
               D_DERROR( ret, "Core/Surface: Could not register 'shared' surface pool!\n" );
               return ret;
          }
     }
     else {
          ret = dfb_surface_pool_initialize2( core, &sharedSurfacePoolFuncs, data, &shared->surface_pool );
          if (ret) {
               D_DERROR( ret, "Core/Surface: Could not register 'shared' surface pool!\n" );
               return ret;
          }
     }
#else
     ret = dfb_surface_pool_initialize2( core, &localSurfacePoolFuncs, data, &shared->surface_pool );
     if (ret) {
          D_DERROR( ret, "Core/Surface: Could not register 'local' surface pool!\n" );
          return ret;
     }
#endif

     ret = dfb_surface_pool_initialize2( core, &preallocSurfacePoolFuncs, data, &shared->prealloc_pool );
     if (ret) {
          D_DERROR( ret, "Core/Surface: Could not register 'prealloc' surface pool!\n" );
          dfb_surface_pool_destroy( shared->surface_pool );
          return ret;
     }

     ret = dfb_surface_pool_bridge_initialize( core, preallocSurfacePoolBridgeFuncs, data, &shared->prealloc_pool_bridge );
     if (ret) {
          D_DERROR( ret, "Core/Surface: Could not register 'prealloc' surface pool bridge!\n" );
          dfb_surface_pool_destroy( shared->prealloc_pool );
          dfb_surface_pool_destroy( shared->surface_pool );
          return ret;
     }

     D_MAGIC_SET( data, DFBSurfaceCore );
     D_MAGIC_SET( shared, DFBSurfaceCoreShared );

     return DFB_OK;
}

static DFBResult
dfb_surface_core_join( CoreDFB              *core,
                       DFBSurfaceCore       *data,
                       DFBSurfaceCoreShared *shared )
{
     D_DEBUG_AT( Core_Surface, "dfb_surface_core_join( %p, %p, %p )\n", core, data, shared );

     D_ASSERT( data != NULL );
     D_MAGIC_ASSERT( shared, DFBSurfaceCoreShared );

     data->core   = core;
     data->shared = shared;

#if FUSION_BUILD_MULTI
     if (fusion_config->secure_fusion)
          dfb_surface_pool_join2( core, shared->surface_pool, &sharedSecureSurfacePoolFuncs, data );
     else
          dfb_surface_pool_join2( core, shared->surface_pool, &sharedSurfacePoolFuncs, data );
#else
     dfb_surface_pool_join2( core, shared->surface_pool, &localSurfacePoolFuncs, data );
#endif

     dfb_surface_pool_join2( core, shared->prealloc_pool, &preallocSurfacePoolFuncs, data );

     dfb_surface_pool_bridge_join( core, shared->prealloc_pool_bridge, preallocSurfacePoolBridgeFuncs, data );

     D_MAGIC_SET( data, DFBSurfaceCore );

     return DFB_OK;
}

static DFBResult
dfb_surface_core_shutdown( DFBSurfaceCore *data,
                           bool            emergency )
{
     DFBSurfaceCoreShared *shared;

     D_DEBUG_AT( Core_Surface, "dfb_surface_core_shutdown( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBSurfaceCore );
     D_MAGIC_ASSERT( data->shared, DFBSurfaceCoreShared );

     shared = data->shared;

     dfb_surface_pool_bridge_destroy( shared->prealloc_pool_bridge );

     dfb_surface_pool_destroy( shared->prealloc_pool );

#if FUSION_BUILD_MULTI
     dfb_surface_pool_destroy( shared->surface_pool );
#else
     dfb_surface_pool_destroy( shared->surface_pool );
#endif

     D_MAGIC_CLEAR( data );
     D_MAGIC_CLEAR( shared );

     return DFB_OK;
}

static DFBResult
dfb_surface_core_leave( DFBSurfaceCore *data,
                        bool            emergency )
{
     DFBSurfaceCoreShared *shared;

     D_DEBUG_AT( Core_Surface, "dfb_surface_core_leave( %p, %semergency )\n", data, emergency ? "" : "no " );

     D_MAGIC_ASSERT( data, DFBSurfaceCore );
     D_MAGIC_ASSERT( data->shared, DFBSurfaceCoreShared );

     shared = data->shared;

     dfb_surface_pool_bridge_leave( shared->prealloc_pool_bridge );

     dfb_surface_pool_leave( shared->prealloc_pool );

#if FUSION_BUILD_MULTI
     dfb_surface_pool_leave( shared->surface_pool );
#else
     dfb_surface_pool_leave( shared->surface_pool );
#endif

     D_MAGIC_CLEAR( data );

     return DFB_OK;
}

static DFBResult
dfb_surface_core_suspend( DFBSurfaceCore *data )
{
     DFBSurfaceCoreShared *shared;

     D_DEBUG_AT( Core_Surface, "dfb_surface_core_suspend( %p )\n", data );

     D_MAGIC_ASSERT( data, DFBSurfaceCore );
     D_MAGIC_ASSERT( data->shared, DFBSurfaceCoreShared );

     shared = data->shared;

     return DFB_OK;
}

static DFBResult
dfb_surface_core_resume( DFBSurfaceCore *data )
{
     DFBSurfaceCoreShared *shared;

     D_DEBUG_AT( Core_Surface, "dfb_surface_core_resume( %p )\n", data );

     D_MAGIC_ASSERT( data, DFBSurfaceCore );
     D_MAGIC_ASSERT( data->shared, DFBSurfaceCoreShared );

     shared = data->shared;

     return DFB_OK;
}

