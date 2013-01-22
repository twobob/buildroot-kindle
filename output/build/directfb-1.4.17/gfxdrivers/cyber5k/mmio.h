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

#ifndef  __CYBER5K_MMIO__
#define  __CYBER5K_MMIO__

#include <dfb_types.h>
#include "cyber5k.h"
#include "regs.h"

static inline void
cyber_out8(volatile u8 *mmioaddr, u32 reg, u8 value)
{
     *((volatile u8*)(mmioaddr+reg)) = value;
}

static inline void
cyber_out16(volatile u8 *mmioaddr, u32 reg, u16 value)
{
     *((volatile u16*)(mmioaddr+reg)) = value;
}

static inline void
cyber_out32(volatile u8 *mmioaddr, u32 reg, u32 value)
{
     *((volatile u32*)(mmioaddr+reg)) = value;
}

static inline u8
cyber_in8(volatile u8 *mmioaddr, u32 reg)
{
     return *((volatile u8*)(mmioaddr+reg));
}

static inline u16
cyber_in16(volatile u8 *mmioaddr, u32 reg)
{
     return *((volatile u16*)(mmioaddr+reg));
}

static inline u32
cyber_in32(volatile u8 *mmioaddr, u32 reg)
{
     return *((volatile u32*)(mmioaddr+reg));
}

/* Wait for idle accelerator */
static inline void
cyber_waitidle( CyberDriverData *cdrv, CyberDeviceData *cdev )
{
/*     while (cyber_in8(mmioaddr, COP_STAT) & (CMDFF_FULL | HOSTFF_NOTEMPTY)) {
          grodis = 0;
     }*/
     while ( cyber_in8(cdrv->mmio_base, COP_STAT) & (COP_BUSY|CMDFF_FULL|HOSTFF_NOTEMPTY) );
}

/* ------------------------------------------------------------------------ */

static inline void cyber_crtcw(int reg, int val)
{
     cyber_out8( cyber_mmio, CRTINDEX, reg );
     cyber_out8( cyber_mmio, CRTDATA, val );
}

static inline void cyber_grphw(int reg, int val)
{
     cyber_out8( cyber_mmio, GRAINDEX, reg );
     cyber_out8( cyber_mmio, GRADATA, val );
}

static inline unsigned int cyber_grphr(int reg)
{
     cyber_out8( cyber_mmio, GRAINDEX, reg );
     return cyber_in8( cyber_mmio, GRADATA );
}

static inline void cyber_attrw(int reg, int val)
{
     cyber_in8( cyber_mmio, ATTRRESET );
     cyber_out8( cyber_mmio, ATTRINDEX, reg );
     cyber_in8( cyber_mmio, ATTRDATAR );
     cyber_out8( cyber_mmio, ATTRDATAW, val );
}

static inline void cyber_seqw(int reg, int val)
{
     cyber_out8( cyber_mmio, SEQINDEX, reg );
     cyber_out8( cyber_mmio, SEQDATA, val );
}

static inline void cyber_tvw(int reg, int val)
{
     cyber_out8( cyber_mmio, 0xb0000 + reg, val );
}

static inline unsigned int cyber_tvr(int reg)
{
     return cyber_in8( cyber_mmio, 0xb0000 + reg );
}

#endif
