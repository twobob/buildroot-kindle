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

#ifndef __NEOMAGIC_H__
#define __NEOMAGIC_H__

#include <dfb_types.h>
#include <sys/io.h>

#include <core/gfxcard.h>
#include <core/layers.h>

typedef struct {
     unsigned int waitfifo_sum;
     unsigned int waitfifo_calls;
     unsigned int fifo_waitcycles;
     unsigned int idle_waitcycles;
     unsigned int fifo_cache_hits;
} NeoDeviceData;

typedef struct {
     volatile u8 *mmio_base;
} NeoDriverData;

extern DisplayLayerFuncs neoOverlayFuncs;

void
neo2200_get_info( CoreGraphicsDevice *device,
                  GraphicsDriverInfo *info );

DFBResult
neo2200_init_driver( CoreGraphicsDevice  *device,
                     GraphicsDeviceFuncs *funcs,
                     void                *driver_data );

DFBResult
neo2200_init_device( CoreGraphicsDevice *device,
                     GraphicsDeviceInfo *device_info,
                     void               *driver_data,
                     void               *device_data );

void
neo2200_close_device( CoreGraphicsDevice *device,
                      void               *driver_data,
                      void               *device_data );

void
neo2200_close_driver( CoreGraphicsDevice *device,
                      void               *driver_data );


#define NEO_BS0_BLT_BUSY        0x00000001
#define NEO_BS0_FIFO_AVAIL      0x00000002
#define NEO_BS0_FIFO_PEND       0x00000004

#define NEO_BC0_DST_Y_DEC       0x00000001
#define NEO_BC0_X_DEC           0x00000002
#define NEO_BC0_SRC_TRANS       0x00000004
#define NEO_BC0_SRC_IS_FG       0x00000008
#define NEO_BC0_SRC_Y_DEC       0x00000010
#define NEO_BC0_FILL_PAT        0x00000020
#define NEO_BC0_SRC_MONO        0x00000040
#define NEO_BC0_SYS_TO_VID      0x00000080

#define NEO_BC1_DEPTH8          0x00000100
#define NEO_BC1_DEPTH16         0x00000200
#define NEO_BC1_X_320           0x00000400
#define NEO_BC1_X_640           0x00000800
#define NEO_BC1_X_800           0x00000c00
#define NEO_BC1_X_1024          0x00001000
#define NEO_BC1_X_1152          0x00001400
#define NEO_BC1_X_1280          0x00001800
#define NEO_BC1_X_1600          0x00001c00
#define NEO_BC1_DST_TRANS       0x00002000
#define NEO_BC1_MSTR_BLT        0x00004000
#define NEO_BC1_FILTER_Z        0x00008000

#define NEO_BC2_WR_TR_DST       0x00800000

#define NEO_BC3_SRC_XY_ADDR     0x01000000
#define NEO_BC3_DST_XY_ADDR     0x02000000
#define NEO_BC3_CLIP_ON         0x04000000
#define NEO_BC3_FIFO_EN         0x08000000
#define NEO_BC3_BLT_ON_ADDR     0x10000000
#define NEO_BC3_SKIP_MAPPING    0x80000000

#define NEO_MODE1_DEPTH8        0x0100
#define NEO_MODE1_DEPTH16       0x0200
#define NEO_MODE1_DEPTH24       0x0300
#define NEO_MODE1_X_320         0x0400
#define NEO_MODE1_X_640         0x0800
#define NEO_MODE1_X_800         0x0c00
#define NEO_MODE1_X_1024        0x1000
#define NEO_MODE1_X_1152        0x1400
#define NEO_MODE1_X_1280        0x1800
#define NEO_MODE1_X_1600        0x1c00
#define NEO_MODE1_BLT_ON_ADDR   0x2000


static inline void OUTGR (u8 index, u8 data)
{
     outb (index, 0x3ce);
     outb (data, 0x3cf);
}

static inline void OUTSR (u8 index, u8 data)
{
     outb (index, 0x3c4);
     outb (data, 0x3c5);
}

static inline void neo_lock( void )
{
     OUTGR(0x09, 0x00);
}

static inline void neo_unlock( void )
{
     OUTGR(0x09, 0x26);
}

#endif
