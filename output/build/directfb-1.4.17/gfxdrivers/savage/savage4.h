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

#ifndef __SAVAGE4_H__
#define __SAVAGE4_H__

#include <core/gfxcard.h>

#include "mmio.h"

typedef struct {
     SavageDeviceData s;

     /* state validation */
     int v_gbd;      /* destination */
     int v_pbd;      /* source */
     int v_color;    /* opaque fill color */

     /* saved values */
     u32 Cmd_Src_Transparent;
     u32 Fill_Color;
     u32 src_colorkey;
} Savage4DeviceData;

typedef struct {
     SavageDriverData s;
} Savage4DriverData;


void
savage4_get_info( CoreGraphicsDevice *device,
                  GraphicsDriverInfo *info );

DFBResult
savage4_init_driver( CoreGraphicsDevice  *device,
                     GraphicsDeviceFuncs *funcs,
                     void                *driver_data );

DFBResult
savage4_init_device( CoreGraphicsDevice *device,
                     GraphicsDeviceInfo *device_info,
                     void               *driver_data,
                     void               *device_data );

void
savage4_close_device( CoreGraphicsDevice *device,
                      void               *driver_data,
                      void               *device_data );

void
savage4_close_driver( CoreGraphicsDevice *device,
                      void               *driver_data );


#define CR_MEMCONF                              0x31
#define CR_MEMCONF_ENABLE_VGA_16BIT_IO_ACCESS   0x04
#define CR_MEMCONF_ENHANCED_MODE_MEMORY_MAPPING 0x08

#define CR_SYSCONF                              0x40
#define CR_SYSCONF_ENABLE_2D_ENGINE_IO_ACCESS   0x01


#define SAVAGE_2D_WRITE_MASK                    0x8128
#define SAVAGE_2D_READ_MASK                     0x812C
#define SAVAGE_2D_BACKGROUND_MIX                0x8134
#define SAVAGE_2D_FOREGROUND_MIX                0x8136


/* Configuration/Status Registers */

#define SAVAGE_STATUS_WORD0                     0x48C00
#define SAVAGE_STATUS_WORD1                     0x48C04
#define SAVAGE_STATUS_WORD2                     0x48C08
#define SAVAGE_SHADOW_STATUS_ADDRESS            0x48C0C
#define SAVAGE_COMMAND_BUFFER_THRESHOLDS        0x48C10
#define SAVAGE_COMMAND_OVERFLOW_BUFFER          0x48C14
#define SAVAGE_COMMAND_OVERFLOW_BUFFER_POINTERS 0x48C18
#define SAVAGE_VERTEX_BUFFER_ADDRESS            0x48C20
#define SAVAGE_BCI_POWER_MANAGEMENT             0x48C24
#define SAVAGE_TILED_SURFACE0                   0x48C40
#define SAVAGE_TILED_SURFACE1                   0x48C44
#define SAVAGE_TILED_SURFACE2                   0x48C48
#define SAVAGE_TILED_SURFACE3                   0x48C4C
#define SAVAGE_TILED_SURFACE4                   0x48C50
#define SAVAGE_ALTERNATE_STATUS_WORD0           0x48C60
#define SAVAGE_ALTERNATE_STATUS_WORD1           0x48C64


/* Wait for fifo space */
static inline void
savage4_waitfifo(Savage4DriverData *sdrv, Savage4DeviceData *sdev, int space)
{
     uint32         slots = MAXFIFO - space;
     volatile u8   *mmio  = sdrv->s.mmio_base;

     sdev->s.waitfifo_sum += space;
     sdev->s.waitfifo_calls++;
     
     if ((savage_in32(mmio, SAVAGE_ALTERNATE_STATUS_WORD0) & 0x001fffff) > slots) {
          do {
               sdev->s.fifo_waitcycles++;
          } while ((savage_in32(mmio, SAVAGE_ALTERNATE_STATUS_WORD0) & 0x001fffff) > slots);
     }
     else {
          sdev->s.fifo_cache_hits++;
     }
}

/* Wait for idle accelerator */
static inline void
savage4_waitidle(Savage4DriverData *sdrv, Savage4DeviceData *sdev)
{
     sdev->s.waitidle_calls++;

     while ((savage_in32(sdrv->s.mmio_base, SAVAGE_ALTERNATE_STATUS_WORD0) & 0x00a00000) != 0x00a00000) {
          sdev->s.idle_waitcycles++;
     }
}


#endif
