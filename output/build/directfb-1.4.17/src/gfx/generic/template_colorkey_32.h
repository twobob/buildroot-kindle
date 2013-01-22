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

/*
 * Example:
 * #define RGB_MASK 0x00ffffff
 * #define Cop_OP_Aop_PFI( op ) Cop_##op##_Aop_32
 * #define Bop_PFI_OP_Aop_PFI( op ) Bop_32_##op##_Aop_32
 * #include "template_colorkey_32.h"
 */

/********************************* Cop_toK_Aop_PFI ****************************/

static void Cop_OP_Aop_PFI(toK)( GenefxState *gfxs )
{
     int  l    = gfxs->length+1;
     u32 *D    = gfxs->Aop[0];
     u32  Dkey = gfxs->Dkey;
     u32  Cop  = gfxs->Cop;

     while (--l) {
          if ((*D & RGB_MASK) == Dkey)
               *D = Cop;

          ++D;
     }
}

/********************************* Bop_PFI_Kto_Aop_PFI ************************/

static void Bop_PFI_OP_Aop_PFI(Kto)( GenefxState *gfxs )
{
     int  l     = gfxs->length+1;
     int  Sstep = gfxs->Bstep;
     int  Dstep = gfxs->Astep;
     u32 *S     = gfxs->Bop[0];
     u32 *D     = gfxs->Aop[0];
     u32  Skey  = gfxs->Skey;

     if (Sstep < 0) {
          S += gfxs->length - 1;
          D += (gfxs->length - 1) * gfxs->Astep;
     }

     while (--l) {
          u32 s = *S;

          if ((s & RGB_MASK) != Skey)
               *D = s;

          S += Sstep;
          D += Dstep;
     }
}

/********************************* Bop_PFI_toK_Aop_PFI ************************/

static void Bop_PFI_OP_Aop_PFI(toK)( GenefxState *gfxs )
{
     int  l     = gfxs->length+1;
     int  Sstep = gfxs->Bstep;
     int  Dstep = gfxs->Astep;
     u32 *S     = gfxs->Bop[0];
     u32 *D     = gfxs->Aop[0];
     u32  Dkey  = gfxs->Dkey;

     if (Sstep < 0) {
          S += gfxs->length - 1;
          D += (gfxs->length - 1) * gfxs->Astep;
     }

     while (--l) {
          if ((*D & RGB_MASK) == Dkey)
               *D = *S;

          S += Sstep;
          D += Dstep;
     }
}

/********************************* Bop_PFI_KtoK_Aop_PFI ***********************/

static void Bop_PFI_OP_Aop_PFI(KtoK)( GenefxState *gfxs )
{
     int  l     = gfxs->length+1;
     int  Sstep = gfxs->Bstep;
     int  Dstep = gfxs->Astep;
     u32 *S     = gfxs->Bop[0];
     u32 *D     = gfxs->Aop[0];
     u32  Skey  = gfxs->Skey;
     u32  Dkey  = gfxs->Dkey;

     if (Sstep < 0) {
          S += gfxs->length - 1;
          D += (gfxs->length - 1) * gfxs->Astep;
     }

     while (--l) {
          u32 s = *S;

          if ((s & RGB_MASK) != Skey && (*D & RGB_MASK) == Dkey)
               *D = s;

          S += Sstep;
          D += Dstep;
     }
}

/********************************* Bop_PFI_SKto_Aop_PFI ***********************/

static void Bop_PFI_OP_Aop_PFI(SKto)( GenefxState *gfxs )
{
     int  l     = gfxs->length+1;
     int  i     = gfxs->Xphase;
     int  SperD = gfxs->SperD;
     u32 *S     = gfxs->Bop[0];
     u32 *D     = gfxs->Aop[0];
     int  Dstep = gfxs->Astep;
     u32  Skey  = gfxs->Skey;

     while (--l) {
          u32 s = S[i>>16];

          if ((s & RGB_MASK) != Skey)
               *D = s;

          D += Dstep;
          i += SperD;
     }
}

/********************************* Bop_PFI_StoK_Aop_PFI ***********************/

static void Bop_PFI_OP_Aop_PFI(StoK)( GenefxState *gfxs )
{
     int  l     = gfxs->length+1;
     int  i     = gfxs->Xphase;
     int  SperD = gfxs->SperD;
     u32 *S     = gfxs->Bop[0];
     u32 *D     = gfxs->Aop[0];
     u32  Dkey  = gfxs->Dkey;
     int  Dstep = gfxs->Astep;

     while (--l) {
          if ((*D & RGB_MASK) == Dkey)
               *D = S[i>>16];

          D += Dstep;
          i += SperD;
     }
}

/********************************* Bop_PFI_SKtoK_Aop_PFI **********************/

static void Bop_PFI_OP_Aop_PFI(SKtoK)( GenefxState *gfxs )
{
     int  l     = gfxs->length+1;
     int  i     = gfxs->Xphase;
     int  SperD = gfxs->SperD;
     u32 *S     = gfxs->Bop[0];
     u32 *D     = gfxs->Aop[0];
     u32  Skey  = gfxs->Skey;
     u32  Dkey  = gfxs->Dkey;
     int  Dstep = gfxs->Astep;

     while (--l) {
          u32 s = S[i>>16];

          if ((s & RGB_MASK) != Skey && (*D & RGB_MASK) == Dkey)
               *D = s;

          D += Dstep;
          i += SperD;
     }
}

/******************************************************************************/

#undef RGB_MASK
#undef Cop_OP_Aop_PFI
#undef Bop_PFI_OP_Aop_PFI
