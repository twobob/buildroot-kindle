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

#include <fusion/shmalloc.h>

#include <core/coredefs.h>
#include <core/coretypes.h>

#include <core/CorePalette.h>

#include <core/core.h>
#include <core/surface.h>
#include <core/gfxcard.h>
#include <core/palette.h>
#include <core/colorhash.h>

#include <gfx/convert.h>

#include <misc/util.h>

D_DEBUG_DOMAIN( Core_Palette, "Core/Palette", "DirectFB Palette Core" );

/**********************************************************************************************************************/

static const u8 lookup3to8[] = { 0x00, 0x24, 0x49, 0x6d, 0x92, 0xb6, 0xdb, 0xff };
static const u8 lookup2to8[] = { 0x00, 0x55, 0xaa, 0xff };

static const ReactionFunc dfb_palette_globals[] = {
/* 0 */   _dfb_surface_palette_listener,
          NULL
};

/**********************************************************************************************************************/

static void palette_destructor( FusionObject *object, bool zombie, void *ctx )
{
     CorePaletteNotification  notification;
     CorePalette             *palette = (CorePalette*) object;

     D_MAGIC_ASSERT( palette, CorePalette );

     D_DEBUG_AT( Core_Palette, "destroying %p (%d)%s\n", palette,
                 palette->num_entries, zombie ? " (ZOMBIE)" : "");

     D_ASSERT( palette->entries != NULL );
     D_ASSERT( palette->entries_yuv != NULL );

     notification.flags   = CPNF_DESTROY;
     notification.palette = palette;

     dfb_palette_dispatch( palette, &notification, dfb_palette_globals );

     // FIXME-SECUREFUSION: invalidate in other processes as well
     dfb_colorhash_invalidate( NULL, palette );

     SHFREE( palette->shmpool, palette->entries_yuv );
     SHFREE( palette->shmpool, palette->entries );

     CorePalette_Deinit_Dispatch( &palette->call );

     D_MAGIC_CLEAR( palette );

     fusion_object_destroy( object );
}

FusionObjectPool *
dfb_palette_pool_create( const FusionWorld *world )
{
     FusionObjectPool *pool;

     pool = fusion_object_pool_create( "Palette Pool",
                                       sizeof(CorePalette),
                                       sizeof(CorePaletteNotification),
                                       palette_destructor, NULL, world );

     return pool;
}

/**********************************************************************************************************************/

DFBResult
dfb_palette_create( CoreDFB       *core,
                    unsigned int   size,
                    CorePalette  **ret_palette )
{
     CorePalette *palette;

     D_DEBUG_AT( Core_Palette, "%s( %d )\n", __FUNCTION__, size );

     D_ASSERT( ret_palette );

     palette = dfb_core_create_palette( core );
     if (!palette)
          return DFB_FUSION;

     palette->shmpool = dfb_core_shmpool( core );

     if (size) {
          palette->entries = SHCALLOC( palette->shmpool, size, sizeof(DFBColor) );
          if (!palette->entries) {
               fusion_object_destroy( &palette->object );
               return D_OOSHM();
          }

          palette->entries_yuv = SHCALLOC( palette->shmpool, size, sizeof(DFBColorYUV) );
          if (!palette->entries_yuv) {
               SHFREE( palette->shmpool, palette->entries );
               fusion_object_destroy( &palette->object );
               return D_OOSHM();
          }
     }

     palette->num_entries = size;

#if 0 //FIXME-SECUREFUSION
     /* reset cache */
     palette->search_cache.index = -1;
#endif

     CorePalette_Init_Dispatch( core, palette, &palette->call );

     D_MAGIC_SET( palette, CorePalette );

     /* activate object */
     fusion_object_activate( &palette->object );

     /* return the new palette */
     *ret_palette = palette;

     D_DEBUG_AT( Core_Palette, "  -> %p\n", palette );

     return DFB_OK;
}

void
dfb_palette_generate_rgb332_map( CorePalette *palette )
{
     unsigned int i;
     DFBColor     entries[256];

     D_DEBUG_AT( Core_Palette, "%s( %p )\n", __FUNCTION__, palette );

     D_MAGIC_ASSERT( palette, CorePalette );

     if (!palette->num_entries)
          return;

     for (i=0; i<palette->num_entries; i++) {
          entries[i].a = i ? 0xff : 0x00;
          entries[i].r = lookup3to8[ (i & 0xE0) >> 5 ];
          entries[i].g = lookup3to8[ (i & 0x1C) >> 2 ];
          entries[i].b = lookup2to8[ (i & 0x03) ];
     }

     CorePalette_SetEntries( palette, entries, palette->num_entries, 0 );
}

void
dfb_palette_generate_rgb121_map( CorePalette *palette )
{
     unsigned int i;
     DFBColor     entries[256];

     D_DEBUG_AT( Core_Palette, "%s( %p )\n", __FUNCTION__, palette );

     D_MAGIC_ASSERT( palette, CorePalette );

     if (!palette->num_entries)
          return;

     for (i=0; i<palette->num_entries; i++) {
          entries[i].a = i ? 0xff : 0x00;
          entries[i].r = (i & 0x8) ? 0xff : 0x00;
          entries[i].g = lookup2to8[ (i & 0x6) >> 1 ];
          entries[i].b = (i & 0x1) ? 0xff : 0x00;
     }

     CorePalette_SetEntries( palette, entries, palette->num_entries, 0 );
}

unsigned int
dfb_palette_search( CorePalette *palette,
                    u8           r,
                    u8           g,
                    u8           b,
                    u8           a )
{
     unsigned int index;

     D_MAGIC_ASSERT( palette, CorePalette );

#if 0 //FIXME-SECUREFUSION
     /* check local cache first */
     if (palette->search_cache.index != -1 &&
         palette->search_cache.color.a == a &&
         palette->search_cache.color.r == r &&
         palette->search_cache.color.g == g &&
         palette->search_cache.color.b == b)
          return palette->search_cache.index;
#endif

     index = dfb_colorhash_lookup( NULL, palette, r, g, b, a );

#if 0 //FIXME-SECUREFUSION
     /* write into local cache */
     palette->search_cache.index = index;
     palette->search_cache.color.a = a;
     palette->search_cache.color.r = r;
     palette->search_cache.color.g = g;
     palette->search_cache.color.b = b;
#endif

     return index;
}

void
dfb_palette_update( CorePalette *palette, int first, int last )
{
     CorePaletteNotification notification;

     D_DEBUG_AT( Core_Palette, "%s( %p, %d, %d )\n", __FUNCTION__, palette, first, last );

     D_MAGIC_ASSERT( palette, CorePalette );
     D_ASSERT( first >= 0 );
     D_ASSERT( first < palette->num_entries );
     D_ASSERT( last >= 0 );
     D_ASSERT( last < palette->num_entries );
     D_ASSERT( first <= last );

     notification.flags   = CPNF_ENTRIES;
     notification.palette = palette;
     notification.first   = first;
     notification.last    = last;

#if 0 //FIXME-SECUREFUSION
     /* reset cache */
     if (palette->search_cache.index >= first &&
         palette->search_cache.index <= last)
          palette->search_cache.index = -1;
#endif

     /* invalidate entries in colorhash */
     // FIXME-SECUREFUSION: invalidate in other processes as well
     dfb_colorhash_invalidate( NULL, palette );

     /* post message about palette update */
     dfb_palette_dispatch( palette, &notification, dfb_palette_globals );
}

bool
dfb_palette_equal( CorePalette *palette1, CorePalette *palette2 )
{
     u32 *entries1;
     u32 *entries2;
     int    i;
     
     D_DEBUG_AT( Core_Palette, "%s( %p, %p )\n", __FUNCTION__, palette1, palette2 );

     D_ASSERT( palette1 != NULL );
     D_ASSERT( palette2 != NULL );

     if (palette1 == palette2) {
          D_DEBUG_AT( Core_Palette, "  -> SAME\n" );
          return true;
     }

     if (palette1->num_entries != palette2->num_entries) {
          D_DEBUG_AT( Core_Palette, "  -> NOT EQUAL (%d/%d)\n", palette1->num_entries, palette2->num_entries );
          return false;
     }

     entries1 = (u32*)palette1->entries;
     entries2 = (u32*)palette2->entries;

     for (i = 0; i < palette1->num_entries; i++) {
          if (entries1[i] != entries2[i]) {
               D_DEBUG_AT( Core_Palette, "  -> NOT EQUAL (%d)\n", i );
               return false;
          }
     }

     D_DEBUG_AT( Core_Palette, "  -> EQUAL\n" );

     return true;
}

