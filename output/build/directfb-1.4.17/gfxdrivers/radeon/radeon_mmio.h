/*
 * Copyright (C) 2006 Claudio Ciccani <klan@users.sf.net>
 *
 * Graphics driver for ATI Radeon cards written by
 *             Claudio Ciccani <klan@users.sf.net>.  
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#ifndef __RADEON_MMIO_H__
#define __RADEON_MMIO_H__

#include <unistd.h>
#include <dfb_types.h>

#include "radeon.h"


static __inline__ void
radeon_out8( volatile u8 *mmioaddr, u32 reg, u8 value )
{
     *((volatile u8*)(mmioaddr+reg)) = value;
}

static __inline__ void
radeon_out16( volatile u8 *mmioaddr, u32 reg, u32 value )
{
#ifdef __powerpc__
     asm volatile( "sthbrx %0,%1,%2;eieio"
                  :: "r" (value), "b"(reg), "r" (mmioaddr) : "memory" );
#else
     *((volatile u16*)(mmioaddr+reg)) = value;
#endif
}

static __inline__ void
radeon_out32( volatile u8 *mmioaddr, u32 reg, u32 value )
{
#ifdef __powerpc__
     asm volatile( "stwbrx %0,%1,%2;eieio" 
                   :: "r" (value), "b"(reg), "r" (mmioaddr) : "memory" );
#else
     *((volatile u32*)(mmioaddr+reg)) = value;
#endif
}

static __inline__ u8
radeon_in8( volatile u8 *mmioaddr, u32 reg )
{
     return *((volatile u8*)(mmioaddr+reg));
}

static __inline__ u16
radeon_in16( volatile u8 *mmioaddr, u32 reg )
{
#ifdef __powerpc__
     u32 value;
     asm volatile( "lhbrx %0,%1,%2;eieio"
                   : "=r" (value) : "b" (reg), "r" (mmioaddr) );
     return value;
#else
     return *((volatile u16*)(mmioaddr+reg));
#endif
}

static __inline__ u32
radeon_in32( volatile u8 *mmioaddr, u32 reg )
{
#ifdef __powerpc__
     u32 value;
     asm volatile( "lwbrx %0,%1,%2;eieio"
                  : "=r" (value) : "b" (reg), "r" (mmioaddr) );
     return value;
#else
     return *((volatile u32*)(mmioaddr+reg));
#endif
}


static __inline__ void
radeon_outpll( volatile u8 *mmioaddr, u32 addr, u32 value )
{
     radeon_out8( mmioaddr, CLOCK_CNTL_INDEX, (addr & 0x3f) | PLL_WR_EN );
     radeon_out32( mmioaddr, CLOCK_CNTL_DATA, value );
}

static __inline__ u32
radeon_inpll( volatile u8 *mmioaddr, u32 addr )
{
     radeon_out8( mmioaddr, CLOCK_CNTL_INDEX, addr & 0x3f );
     return radeon_in32( mmioaddr, CLOCK_CNTL_DATA );
}


static inline bool
radeon_waitfifo( RadeonDriverData *rdrv,
                 RadeonDeviceData *rdev,
                 unsigned int    space )
{
     int waitcycles = 0;

     rdev->waitfifo_sum += space;
     rdev->waitfifo_calls++;

     if (rdev->fifo_space < space ) {
          do {
               rdev->fifo_space  = radeon_in32( rdrv->mmio_base, RBBM_STATUS );
               rdev->fifo_space &= RBBM_FIFOCNT_MASK;
               if (++waitcycles > 10000000) {
                    radeon_reset( rdrv, rdev );
                    D_BREAK( "FIFO timed out" );
                    return false;
               }
          } while (rdev->fifo_space < space);
          
          rdev->fifo_waitcycles += waitcycles;
     } else
          rdev->fifo_cache_hits++;
	    
    rdev->fifo_space -= space;
    
    return true;
}

static inline bool
radeon_waitidle( RadeonDriverData *rdrv, RadeonDeviceData *rdev )
{
     int waitcycles = 0;
     int status;

     if (!radeon_waitfifo( rdrv, rdev, 64 ))
     	return false;
     
     do {
          status = radeon_in32( rdrv->mmio_base, RBBM_STATUS );
          if (++waitcycles > 10000000) {
               radeon_reset( rdrv, rdev );
               D_BREAK( "Engine timed out" );
               return false;
          }
     } while (status & RBBM_ACTIVE);
     
     rdev->fifo_space = status & RBBM_FIFOCNT_MASK;
     rdev->idle_waitcycles += waitcycles;
     
     return true;
}


#endif /* __RADEON_MMIO_H__ */
