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

#ifndef ___MACH64_H__
#define ___MACH64_H__

#include <dfb_types.h>
#include <core/coretypes.h>
#include <core/layers.h>

#define S13( val ) ((val) & 0x3FFF)
#define S14( val ) ((val) & 0x7FFF)

typedef enum {
     m_source       = 0x001,
     m_source_scale = 0x002,
     m_color        = 0x004,
     m_color_3d     = 0x008,
     m_color_tex    = 0x010,
     m_srckey       = 0x020,
     m_srckey_scale = 0x040,
     m_dstkey       = 0x080,
     m_disable_key  = 0x100,
     m_draw_blend   = 0x200,
     m_blit_blend   = 0x400,
} Mach64StateBits;

#define MACH64_VALIDATE(b)      (mdev->valid |= (b))
#define MACH64_INVALIDATE(b)    (mdev->valid &= ~(b))
#define MACH64_IS_VALID(b)      (mdev->valid & (b))

typedef enum {
     CHIP_UNKNOWN = 0,
     CHIP_264VT,
     CHIP_3D_RAGE,
     CHIP_264VT3,
     CHIP_3D_RAGE_II,
     CHIP_3D_RAGE_IIPLUS,
     CHIP_3D_RAGE_LT,
     CHIP_264VT4,
     CHIP_3D_RAGE_IIC,
     CHIP_3D_RAGE_PRO,
     CHIP_3D_RAGE_LT_PRO,
     CHIP_3D_RAGE_XLXC,
     CHIP_3D_RAGE_MOBILITY,
} Mach64ChipType;

typedef struct {
     Mach64ChipType chip;

     /* for fifo/performance monitoring */
     unsigned int fifo_space;
     unsigned int waitfifo_sum;
     unsigned int waitfifo_calls;
     unsigned int fifo_waitcycles;
     unsigned int idle_waitcycles;
     unsigned int fifo_cache_hits;

     Mach64StateBits valid;

     u32 hw_debug;
     u32 hw_debug_orig;

     u32 pix_width;

     u32 draw_blend;
     u32 blit_blend;

     int tex_offset;
     int tex_pitch;
     int tex_height;
     int tex_size;

     int scale_offset;
     int scale_pitch;

     CoreSurface *source;

     bool blit_deinterlace;
     int field;

     DFBRegion clip;

     bool use_scaler_3d;
} Mach64DeviceData;

typedef struct {
     int accelerator;
     volatile u8 *mmio_base;
     Mach64DeviceData *device_data;
} Mach64DriverData;

extern DisplayLayerFuncs mach64OverlayFuncs;

#endif
