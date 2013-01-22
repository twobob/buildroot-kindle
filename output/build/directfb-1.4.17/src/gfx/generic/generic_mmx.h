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


#define __aligned( n )  __attribute__ ((aligned((n))))


static void SCacc_add_to_Dacc_MMX( GenefxState *gfxs )
{
     __asm__ __volatile__ (
               "    movq     %2, %%mm0\n"
               ".align   16\n"
               "1:\n"
               "    movq     (%0), %%mm1\n"
               "    paddw    %%mm0, %%mm1\n"
               "    movq     %%mm1, (%0)\n"
               "    add      $8, %0\n"
               "    dec      %1\n"
               "    jnz      1b\n"
               "    emms"
               : /* no outputs */
               : "D" (gfxs->Dacc), "c" (gfxs->length), "m" (gfxs->SCacc)
               : "%st", "memory");
}

static void Dacc_modulate_argb_MMX( GenefxState *gfxs )
{
     __asm__ __volatile__ (
               "movq     %2, %%mm0\n\t"
               ".align   16\n"
               "1:\n\t"
               "testw    $0xF000, 6(%0)\n\t"
               "jnz      2f\n\t"
               "movq     (%0), %%mm1\n\t"
               "pmullw   %%mm0, %%mm1\n\t"
               "psrlw    $8, %%mm1\n\t"
               "movq     %%mm1, (%0)\n"
               ".align   16\n"
               "2:\n\t"
               "add      $8, %0\n\t"
               "dec      %1\n\t"
               "jnz      1b\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Dacc), "c" (gfxs->length), "m" (gfxs->Cacc)
               : "%st", "memory");
}

static void Sacc_add_to_Dacc_MMX( GenefxState *gfxs )
{
     __asm__ __volatile__ (
               ".align   16\n"
               "1:\n\t"
               "movq     (%2), %%mm0\n\t"
               "movq     (%0), %%mm1\n\t"
               "paddw    %%mm1, %%mm0\n\t"
               "movq     %%mm0, (%0)\n\t"
               "add      $8, %0\n\t"
               "add      $8, %2\n\t"
               "dec      %1\n\t"
               "jnz      1b\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Dacc), "c" (gfxs->length), "S" (gfxs->Sacc)
               : "%st", "memory");
}

__attribute__((unused))
static void Sacc_to_Aop_rgb16_MMX( GenefxState *gfxs )
{
     static const u32 preload[] = { 0xFF00FF00, 0x0000FF00 };
     static const u32 mask[]    = { 0x00FC00F8, 0x000000F8 };
     static const u32 pm[]      = { 0x01000004, 0x00000004 };

     __asm__ __volatile__ (
	       "movq     %3, %%mm7\n\t"
	       "movq     %4, %%mm5\n\t"
	       "movq     %5, %%mm4\n\t"
               ".align   16\n"
               "1:\n\t"
               "testw    $0xF000, 6(%2)\n\t"
               "jnz      2f\n\t"
               "movq     (%2), %%mm0\n\t"
               "paddusw  %%mm7, %%mm0\n\t"
               "pand     %%mm5, %%mm0\n\t"
               "pmaddwd  %%mm4, %%mm0\n\t"
               "psrlq    $5, %%mm0\n\t"
               "movq     %%mm0, %%mm1\n\t"
               "psrlq    $21, %%mm0\n\t"
               "por      %%mm1, %%mm0\n\t"
               "movd     %%mm0, %%eax\n\t"
               "movw     %%ax, (%0)\n\t"
               ".align 16\n"
               "2:\n\t"
               "add      $8, %2\n\t"
	       "add      $2, %0\n\t"
               "dec      %1\n\t"
               "jnz      1b\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Aop[0]), "c" (gfxs->length), "S" (gfxs->Sacc),
                 "m" (*preload), "m" (*mask), "m" (*pm)
               : "%eax", "%st", "memory");
}

__attribute__((unused))
static void Sacc_to_Aop_rgb32_MMX( GenefxState *gfxs )
{
     static const u32 preload[]  = { 0xFF00FF00, 0x0000FF00 };
     static const u32 postload[] = { 0x00FF00FF, 0x000000FF };
     static const u32 pm[]       = { 0x01000001, 0x00000001 };

     __asm__ __volatile__ (
	       "movq     %3, %%mm1\n\t"
	       "movq     %4, %%mm2\n\t"
	       "movq     %5, %%mm3\n\t"
               ".align   16\n"
               "1:\n\t"
               "testw    $0xF000, 6(%2)\n\t"
               "jnz      2f\n\t"
               "movq     (%2), %%mm0\n\t"
               "paddusw  %%mm1, %%mm0\n\t"
               "pand     %%mm2, %%mm0\n\t"
               "pmaddwd  %%mm3, %%mm0\n\t"
               "movq     %%mm0, %%mm4\n\t"
               "psrlq    $16, %%mm0\n\t"
               "por      %%mm0, %%mm4\n\t"
               "movd     %%mm4, (%0)\n\t"
               ".align 16\n"
               "2:\n\t"
               "add      $8, %2\n\t"
	       "add      $4, %0\n\t"
               "dec      %1\n\t"
               "jnz      1b\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Aop[0]), "c" (gfxs->length), "S" (gfxs->Sacc),
                 "m" (*preload), "m" (*postload), "m" (*pm)
               : "%st", "memory");
}

__attribute__((unused))
__attribute__((no_instrument_function))
static void Sop_argb_Sto_Dacc_MMX( GenefxState *gfxs )
{
     static const u32 zeros[]  = { 0, 0 };
     int i = 0;

     __asm__ __volatile__ (
	       "movq     %5, %%mm0\n\t"
               ".align   16\n"
               "1:\n\t"
               "movd     (%3), %%mm1\n\t"
               "punpcklbw %%mm0, %%mm1\n\t"
               ".align   16\n"
               "2:\n\t"
               "movq     %%mm1, (%1)\n\t"
               "dec      %2\n\t"
               "jz       3f\n\t"
               "add      $8, %1\n\t"
               "add      %4, %0\n\t"
               "testl    $0xFFFF0000, %0\n\t"
               "jz       2b\n\t"
               "movl     %0, %%edx\n\t"
               "andl     $0xFFFF0000, %%edx\n\t"
               "shrl     $14, %%edx\n\t"
#ifdef ARCH_X86_64
               "addq     %%rdx, %3\n\t"
#else
               "addl     %%edx, %3\n\t"
#endif
               "andl     $0xFFFF, %0\n\t"
               "jmp      1b\n"
               "3:\n\t"
               "emms"
               : "=r" (i)
               : "D" (gfxs->Dacc), "c" (gfxs->length), "S" (gfxs->Sop[0]),
                 "a" (gfxs->SperD), "m" (*zeros), "0" (i)
               : "%edx", "%st", "memory");
}

__attribute__((unused))
static void Sop_argb_to_Dacc_MMX( GenefxState *gfxs )
{
     static const u32 zeros[]  = { 0, 0 };

     __asm__ __volatile__ (
	       "movq     %3, %%mm0\n\t"
               ".align   16\n"
               "1:\n\t"
               "movd     (%2), %%mm1\n\t"
               "punpcklbw %%mm0, %%mm1\n\t"
               "movq     %%mm1, (%0)\n\t"
               "add      $4, %2\n\t"
               "add      $8, %0\n\t"
               "dec      %1\n\t"
               "jnz      1b\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Dacc), "c" (gfxs->length),
                 "S" (gfxs->Sop[0]), "m" (*zeros)
               : "%st", "memory");
}

__attribute__((unused))
static void Sop_rgb16_to_Dacc_MMX( GenefxState *gfxs )
{
     static const u32 mask[]  = { 0x07E0001F, 0x0000F800 };
     static const u32 smul[]  = { 0x00200800, 0x00000001 };
     static const u32 alpha[] = { 0x00000000, 0x00FF0000 };

     __asm__ __volatile__ (
	       "movq     %3, %%mm4\n\t"
	       "movq     %4, %%mm5\n\t"
	       "movq     %5, %%mm7\n\t"
               ".align   16\n"
               "1:\n\t"
               "movq     (%2), %%mm0\n\t"
               /* 1. Konvertierung nach 24 bit interleaved */
	       "movq     %%mm0, %%mm3\n\t"
               "punpcklwd %%mm3, %%mm3\n\t"
               "punpckldq %%mm3, %%mm3\n\t"
               "pand     %%mm4, %%mm3\n\t"
               "pmullw   %%mm5, %%mm3\n\t"
               "psrlw    $8, %%mm3\n\t"
               /* mm3 enthaelt jetzt: 0000 00rr 00gg 00bb des alten pixels */
               "por      %%mm7, %%mm3\n\t"
               "movq     %%mm3, (%0)\n\t"
               "dec      %1\n\t"
               "jz       2f\n\t"
               "psrlq    $16, %%mm0\n\t"
	       "add      $8, %0\n\t"
               /* 2. Konvertierung nach 24 bit interleaved */
	       "movq     %%mm0, %%mm3\n\t"
               "punpcklwd %%mm3, %%mm3\n\t"
               "punpckldq %%mm3, %%mm3\n\t"
               "pand     %%mm4, %%mm3\n\t"
               "pmullw   %%mm5, %%mm3\n\t"
               "psrlw    $8, %%mm3\n\t"
               /* mm3 enthaelt jetzt: 0000 00rr 00gg 00bb des alten pixels */
               "por      %%mm7, %%mm3\n\t"
               "movq     %%mm3, (%0)\n\t"
               "dec      %1\n\t"
               "jz       2f\n\t"
               "psrlq    $16, %%mm0\n\t"
	       "add      $8, %0\n\t"
               /* 3. Konvertierung nach 24 bit interleaved */
	       "movq     %%mm0, %%mm3\n\t"
               "punpcklwd %%mm3, %%mm3\n\t"
               "punpckldq %%mm3, %%mm3\n\t"
               "pand     %%mm4, %%mm3\n\t"
               "pmullw   %%mm5, %%mm3\n\t"
               "psrlw    $8, %%mm3\n\t"
               /* mm3 enthaelt jetzt: 0000 00rr 00gg 00bb des alten pixels */
               "por      %%mm7, %%mm3\n\t"
               "movq     %%mm3, (%0)\n\t"
               "dec      %1\n\t"
               "jz       2f\n\t"
               "psrlq    $16, %%mm0\n\t"
	       "add      $8, %0\n\t"
               /* 4. Konvertierung nach 24 bit interleaved */
	       "movq     %%mm0, %%mm3\n\t"
               "punpcklwd %%mm3, %%mm3\n\t"
               "punpckldq %%mm3, %%mm3\n\t"
               "pand     %%mm4, %%mm3\n\t"
               "pmullw   %%mm5, %%mm3\n\t"
               "psrlw    $8, %%mm3\n\t"
               /* mm3 enthaelt jetzt: 0000 00rr 00gg 00bb des alten pixels */
               "por      %%mm7, %%mm3\n\t"
               "movq     %%mm3, (%0)\n\t"
               "dec      %1\n\t"
               "jz       2f\n\t"
	       "add      $8, %0\n\t"
	       "add      $8, %2\n\t"
               "jmp      1b\n"
               "2:\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Dacc), "c" (gfxs->length), "S" (gfxs->Sop[0]),
                 "m" (*mask), "m" (*smul), "m" (*alpha)
               : "%st", "memory");
}

__attribute__((unused))
static void Sop_rgb32_to_Dacc_MMX( GenefxState *gfxs )
{
     static const u32 alpha[]  = { 0, 0x00FF0000 };
     static const u32 zeros[]  = { 0, 0 };

     __asm__ __volatile__ (
	       "movq     %3, %%mm7\n\t"
	       "movq     %4, %%mm6\n\t"
               ".align   16\n"
               "1:\n\t"
               "movd     (%2), %%mm0\n\t"
               "punpcklbw %%mm6, %%mm0\n\t"
               "por      %%mm7, %%mm0\n\t"
               "movq     %%mm0, (%0)\n\t"
	       "add      $4, %2\n\t"
	       "add      $8, %0\n\t"
               "dec      %1\n\t"
               "jnz      1b\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Dacc), "c" (gfxs->length), "S" (gfxs->Sop[0]),
                 "m" (*alpha), "m" (*zeros)
               : "%st", "memory");
}

static void Xacc_blend_invsrcalpha_MMX( GenefxState *gfxs )
{
     static const u32 einser[] = { 0x01000100, 0x01000100 };
     static const u32 zeros[]  = { 0, 0 };

     __asm__ __volatile__ (
	       "movq     %3, %%mm7\n\t"
               "cmp      $0, %2\n\t"
               "jne      1f\n\t"
               "movq     %4, %%mm6\n\t"
               "movd     %5, %%mm0\n\t"
               "punpcklbw %%mm6, %%mm0\n\t" /* mm0 = 00aa 00rr 00gg 00bb */
               "punpcklwd %%mm0, %%mm0\n\t" /* mm0 = 00aa 00aa xxxx xxxx */
               "movq      %%mm7, %%mm1\n\t"
               "punpckldq %%mm0, %%mm0\n\t" /* mm0 = 00aa 00aa 00aa 00aa */
               "psubw     %%mm0, %%mm1\n\t"

               ".align   16\n"
               "2:\n\t"                 /* blend from color */
               "testw    $0xF000, 6(%0)\n\t"
               "jnz      3f\n\t"
               "movq     (%0), %%mm0\n\t"
               "pmullw   %%mm1, %%mm0\n\t"
               "psrlw    $8, %%mm0\n\t"
               "movq     %%mm0, (%6)\n\t"
               "jmp      4f\n\t"
               "3:\n\t"
               "movq     (%0), %%mm0\n\t"
               "movq     %%mm0, (%6)\n\t"
               "4:\n\t"
	       "add      $8, %0\n\t"
	       "add      $8, %6\n\t"
               "dec      %1\n\t"
               "jnz      2b\n\t"
               "jmp      9f\n\t"

               ".align   16\n"
               "1:\n\t"                      /* blend from Sacc */
               "testw    $0xF000, 6(%0)\n\t"
               "jnz      5f\n\t"
               "movq     (%2), %%mm2\n\t"
               "movq     (%0), %%mm0\n\t"
	       "punpckhwd %%mm2, %%mm2\n\t" /* mm2 = 00aa 00aa xxxx xxxx */
               "movq	  %%mm7, %%mm1\n\t"
               "punpckhdq %%mm2, %%mm2\n\t" /* mm2 = 00aa 00aa 00aa 00aa */
               "psubw    %%mm2, %%mm1\n\t"
               "pmullw   %%mm1, %%mm0\n\t"
               "psrlw    $8, %%mm0\n\t"
               "movq     %%mm0, (%6)\n\t"
               "jmp      6f\n\t"
               "5:\n\t"
               "movq     (%0), %%mm0\n\t"
               "movq     %%mm0, (%6)\n\t"
               "6:\n\t"
	       "add      $8, %2\n\t"
	       "add      $8, %0\n\t"
	       "add      $8, %6\n\t"
               "dec      %1\n\t"
               "jnz      1b\n\t"
               "9:\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Yacc), "c" (gfxs->length), "S" (gfxs->Sacc),
                 "m" (*einser), "m" (*zeros), "m" (gfxs->color), "r" (gfxs->Xacc)
               : "%st", "memory");
}

static void Xacc_blend_srcalpha_MMX( GenefxState *gfxs )
{
     static const u32 ones[]  = { 0x00010001, 0x00010001 };
     static const u32 zeros[] = { 0, 0 };

     __asm__ __volatile__ (
	       "movq     %3, %%mm7\n\t"
               "cmp      $0, %2\n\t"
               "jne      3f\n\t"
               "movq     %4, %%mm6\n\t"
               "movd     %5, %%mm0\n\t"
               "punpcklbw %%mm6, %%mm0\n\t" /* mm0 = 00aa 00rr 00gg 00bb */
               "punpcklwd %%mm0, %%mm0\n\t" /* mm0 = 00aa 00aa xxxx xxxx */
               "punpckldq %%mm0, %%mm0\n\t" /* mm0 = 00aa 00aa 00aa 00aa */
               "paddw     %%mm7, %%mm0\n\t"

               ".align   16\n\t"
               "4:\n\t"                 /* blend from color */
               "testw    $0xF000, 6(%0)\n\t"
               "jnz      6f\n\t"
               "movq     (%0), %%mm1\n\t"
               "pmullw   %%mm0, %%mm1\n\t"
               "psrlw    $8, %%mm1\n\t"
               "movq     %%mm1, (%6)\n\t"
               "jmp      1f\n\t"
               "6:\n\t"
               "movq     (%0), %%mm1\n\t"
               "movq     %%mm1, (%6)\n\t"
               "1:\n\t"
	       "add      $8, %0\n\t"
	       "add      $8, %6\n\t"
               "dec      %1\n\t"
               "jnz      4b\n\t"
               "jmp      2f\n\t"

               ".align   16\n\t"
               "3:\n\t"                      /* blend from Sacc */
               "testw    $0xF000, 6(%0)\n\t"
               "jnz      5f\n\t"
               "movq     (%2), %%mm0\n\t"
               "movq     (%0), %%mm1\n\t"
	       "punpckhwd %%mm0, %%mm0\n\t" /* mm2 = 00aa 00aa xxxx xxxx */
               "punpckhdq %%mm0, %%mm0\n\t" /* mm2 = 00aa 00aa 00aa 00aa */
               "paddw    %%mm7, %%mm0\n\t"
               "pmullw   %%mm0, %%mm1\n\t"
               "psrlw    $8, %%mm1\n\t"
               "movq     %%mm1, (%6)\n\t"
               "jmp      7f\n\t"
               "5:\n\t"
               "movq     (%0), %%mm1\n\t"
               "movq     %%mm1, (%6)\n\t"
               "7:\n\t"
	       "add      $8, %2\n\t"
	       "add      $8, %0\n\t"
	       "add      $8, %6\n\t"
               "dec      %1\n\t"
               "jnz      3b\n\t"
               "2:\n\t"
               "emms"
               : /* no outputs */
               : "D" (gfxs->Yacc), "c" (gfxs->length), "S" (gfxs->Sacc),
                 "m" (*ones), "m" (*zeros), "m" (gfxs->color), "r" (gfxs->Xacc)
               : "%st", "memory");
}

static void Dacc_YCbCr_to_RGB_MMX( GenefxState *gfxs )
{  
     static const u16 __aligned(8) sub0[4] = {  16,  16,  16,  16 }; 
     static const u16 __aligned(8) sub1[4] = { 128, 128, 128, 128 };
     static const s16 __aligned(8) mul[20] = {
                     0x253F,  0x253F,  0x253F,  0x253F, // Y       Coeff.
                     0x3312,  0x3312,  0x3312,  0x3312, // V Red   Coeff.
                     0x4093,  0x4093,  0x4093,  0x4093, // U Blue  Coeff.
                    -0x1A04, -0x1A04, -0x1A04, -0x1A04, // V Green Coeff.
                    -0x0C83, -0x0C83, -0x0C83, -0x0C83  // U Green Coeff.
     };

     int                w = gfxs->length & 3;
     GenefxAccumulator *D = gfxs->Dacc;
     
     __asm__ __volatile__ (
          "shrl          $2,    %1\n\t"
          "jz            2f\n\t"
          "pxor       %%mm7, %%mm7\n\t"
               ".align 16\n"
               "1:\n\t"
               "movq        (%0), %%mm0\n\t" // 00 a0 00 y0 00 v0 00 u0
               "movq       8(%0), %%mm1\n\t" // 00 a1 00 y1 00 v1 00 u1
               "movq      16(%0), %%mm2\n\t" // 00 a2 00 y2 00 v2 00 u2
               "movq      24(%0), %%mm3\n\t" // 00 a3 00 y3 00 v3 00 u3
               "movq       %%mm0, %%mm4\n\t" // 00 a0 00 y0 00 v0 00 u0
               "movq       %%mm2, %%mm5\n\t" // 00 a2 00 y2 00 v2 00 u2
               "punpcklwd  %%mm1, %%mm0\n\t" // 00 v1 00 v0 00 u1 00 u0
               "punpcklwd  %%mm3, %%mm2\n\t" // 00 v3 00 v2 00 u3 00 u2
               "punpckhwd  %%mm1, %%mm4\n\t" // 00 a1 00 a0 00 y1 00 y0
               "punpckhwd  %%mm3, %%mm5\n\t" // 00 a3 00 a2 00 y3 00 y2
               "movq       %%mm0, %%mm1\n\t" // 00 v1 00 v1 00 u1 00 u0
               "movq       %%mm4, %%mm3\n\t" // 00 a1 00 a0 00 y1 00 y0
               "punpckldq  %%mm2, %%mm0\n\t" // 00 u3 00 u2 00 u1 00 u0
               "punpckldq  %%mm5, %%mm3\n\t" // 00 y3 00 y2 00 y1 00 y0
               "punpckhdq  %%mm2, %%mm1\n\t" // 00 v3 00 v2 00 v1 00 v0
               "punpckhdq  %%mm5, %%mm4\n\t" // 00 a3 00 a2 00 a1 00 a0
               /* mm0 = u, mm1 = v, mm3 = y, mm4 = a */
               "psubw         %2, %%mm3\n\t" // y -= 16
               "psllw         $3, %%mm3\n\t" // precision
               "pmulhw      (%4), %%mm3\n\t"
               "psubw         %3, %%mm1\n\t" // v -= 128 
               "psllw         $3, %%mm1\n\t" // precision
               "movq       %%mm1, %%mm2\n\t" // 00 v3 00 v2 00 v1 00 v0
               "pmulhw     8(%4), %%mm2\n\t" // vr
               "psubw         %3, %%mm0\n\t" // u -= 128 
               "psllw         $3, %%mm0\n\t" // precision
               "movq       %%mm0, %%mm5\n\t" // 00 u3 00 u2 00 u1 00 u0
               "pmulhw    16(%4), %%mm5\n\t" // ub
               "paddw      %%mm3, %%mm2\n\t" // 00 r3 00 r2 00 r1 00 r0
               "paddw      %%mm3, %%mm5\n\t" // 00 b3 00 b2 00 b1 00 b0
               "pmulhw    24(%4), %%mm1\n\t" // vg
               "packuswb   %%mm2, %%mm2\n\t" // r3 r2 r1 r0 r3 r2 r1 r0
               "packuswb   %%mm5, %%mm5\n\t" // b3 b2 b1 b0 b3 b2 b1 b0
               "pmulhw    32(%4), %%mm0\n\t" // ug
               "punpcklbw  %%mm7, %%mm2\n\t" // 00 r3 00 r2 00 r1 00 r0
               "punpcklbw  %%mm7, %%mm5\n\t" // 00 b3 00 b2 00 b1 00 b0
               "paddw      %%mm1, %%mm3\n\t" // y + vg
               "paddw      %%mm0, %%mm3\n\t" // 00 g3 00 g2 00 g1 00 g0
               "packuswb   %%mm3, %%mm3\n\t" // g3 g2 g1 g0 g3 g2 g1 g0
               "punpcklbw  %%mm7, %%mm3\n\t" // 00 g3 00 g2 00 g1 00 g0
               /* mm5 = b, mm3 = g, mm2 = r, mm4 = a */
               "movq       %%mm5, %%mm0\n\t" // 00 b3 00 b2 00 b1 00 b0
               "movq       %%mm3, %%mm1\n\t" // 00 g3 00 g2 00 g1 00 g0
               "punpcklwd  %%mm2, %%mm0\n\t" // 00 r1 00 b1 00 r0 00 b0
               "punpcklwd  %%mm4, %%mm1\n\t" // 00 a1 00 g1 00 a0 00 g0
               "punpckhwd  %%mm2, %%mm5\n\t" // 00 r3 00 b3 00 r2 00 b2
               "punpckhwd  %%mm4, %%mm3\n\t" // 00 a3 00 g3 00 a2 00 g2
               "movq       %%mm0, %%mm2\n\t" // 00 r1 00 b1 00 r0 00 b0
               "movq       %%mm5, %%mm4\n\t" // 00 r3 00 b3 00 r2 00 b2
               "punpcklwd  %%mm1, %%mm0\n\t" // 00 a0 00 r0 00 g0 00 b0
               "punpcklwd  %%mm3, %%mm5\n\t" // 00 a2 00 r2 00 g2 00 b2
               "punpckhwd  %%mm1, %%mm2\n\t" // 00 a1 00 r1 00 g1 00 b1
               "punpckhwd  %%mm3, %%mm4\n\t" // 00 a3 00 r3 00 g3 00 b3
               "movq       %%mm0,  (%0)\n\t"
               "movq       %%mm2, 8(%0)\n\t"
               "movq       %%mm5,16(%0)\n\t"
               "movq       %%mm4,24(%0)\n\t"
               "add          $32,    %0\n\t"
               "decl          %1\n\t"
               "jnz           1b\n\t"
          "emms\n\t"
          "2:"     
          : "=&D" (D)
          : "c" (gfxs->length), "m" (*sub0), "m" (*sub1), "r" (mul), "0" (D)
          : "memory" );

     while (w) {
          if (!(D->YUV.a & 0xF000))
               YCBCR_TO_RGB( D->YUV.y, D->YUV.u, D->YUV.v,
                             D->RGB.r, D->RGB.g, D->RGB.b );

          D++;
          w--;
     }
}

static void Dacc_RGB_to_YCbCr_MMX( GenefxState *gfxs )
{
     static const u16 __aligned(8) add0[4] = { 128, 128, 128, 128 };
     static const u16 __aligned(8) add1[4] = {  16,  16,  16,  16 };
     static const u16 __aligned(8) mul[24] = {
                    0x03A5, 0x03A5, 0x03A5, 0x03A5, // Eb
                    0x12C8, 0x12C8, 0x12C8, 0x12C8, // Eg
                    0x0991, 0x0991, 0x0991, 0x0991, // Er
                    0x0FE1, 0x0FE1, 0x0FE1, 0x0FE1, // Cb
                    0x140A, 0x140A, 0x140A, 0x140A, // Cr
                    0x1B7B, 0x1B7B, 0x1B7B, 0x1B7B  // Y
     };
      
     int                w = gfxs->length & 3;
     GenefxAccumulator *D = gfxs->Dacc;
     
     __asm__ __volatile__(
          "shrl          $2,    %1\n\t"
          "jz            2f\n\t" 
          "pxor       %%mm7, %%mm7\n\t"     
               ".align 16\n"
               "1:\n\t"
               "movq        (%0), %%mm0\n\t" // 00 a0 00 r0 00 g0 00 b0
               "movq       8(%0), %%mm1\n\t" // 00 a1 00 r1 00 g1 00 b1
               "movq      16(%0), %%mm2\n\t" // 00 a2 00 r2 00 g2 00 b2
               "movq      24(%0), %%mm3\n\t" // 00 a3 00 r3 00 g3 00 b3
               "movq       %%mm0, %%mm4\n\t" // 00 a0 00 r0 00 g0 00 b0
               "movq       %%mm2, %%mm6\n\t" // 00 a2 00 r2 00 g2 00 b2
               "punpcklwd  %%mm1, %%mm0\n\t" // 00 g1 00 g0 00 b1 00 b0
               "punpcklwd  %%mm3, %%mm2\n\t" // 00 g3 00 g2 00 b3 00 b2
               "movq       %%mm0, %%mm5\n\t" // 00 g1 00 g0 00 b1 00 b0
               "punpckldq  %%mm2, %%mm0\n\t" // 00 b3 00 b2 00 b1 00 b0
               "punpckhdq  %%mm2, %%mm5\n\t" // 00 g3 00 g2 00 g1 00 g0
               "punpckhwd  %%mm1, %%mm4\n\t" // 00 a1 00 a0 00 r1 00 r0
               "punpckhwd  %%mm3, %%mm6\n\t" // 00 a3 00 a2 00 r3 00 r2
               "movq       %%mm4, %%mm3\n\t" // 00 a1 00 a0 00 r1 00 r0
               "punpckldq  %%mm6, %%mm4\n\t" // 00 r3 00 r2 00 r1 00 r0
               "punpckhdq  %%mm6, %%mm3\n\t" // 00 a3 00 a2 00 a1 00 a0
               /* mm0 = b, mm5 = g, mm4 = r, mm3 = a */
               "movq       %%mm0, %%mm1\n\t" // save b
               "psllw         $3, %%mm0\n\t"
               "pmulhw      (%2), %%mm0\n\t"
               "movq       %%mm4, %%mm2\n\t" // save r
               "psllw         $3, %%mm5\n\t"
               "pmulhw     8(%2), %%mm5\n\t"
               "psllw         $3, %%mm4\n\t"
               "pmulhw    16(%2), %%mm4\n\t"
               "paddw      %%mm5, %%mm0\n\t"
               "paddw      %%mm4, %%mm0\n\t" // ey
               "psubw      %%mm0, %%mm1\n\t" // b - ey
               "psllw         $3, %%mm1\n\t"
               "pmulhw    24(%2), %%mm1\n\t" // 00 u3 00 u2 00 u1 00 u0
               "psubw      %%mm0, %%mm2\n\t" // r - ey
               "psllw         $3, %%mm2\n\t"
               "pmulhw    32(%2), %%mm2\n\t" // 00 v3 00 v2 00 v1 00 v0
               "paddw         %3, %%mm1\n\t" // Cb + 128
               "packuswb   %%mm1, %%mm1\n\t" // u3 u2 u1 u0 u3 u2 u1 u0
               "psllw         $3, %%mm0\n\t"
               "pmulhw    40(%2), %%mm0\n\t" // 00 y3 00 y2 00 y1 00 y0
               "paddw         %3, %%mm2\n\t" // Cr + 128
               "packuswb   %%mm2, %%mm2\n\t" // v3 v2 v1 v0 v3 v2 v1 v0  
               "paddw         %4, %%mm0\n\t" // Y + 16
               "packuswb   %%mm0, %%mm0\n\t" // y3 y2 y1 y0 y3 y2 y1 y0
               "punpcklbw  %%mm7, %%mm2\n\t" // 00 v3 00 v2 00 v1 00 v0
               "punpcklbw  %%mm7, %%mm1\n\t" // 00 u3 00 y2 00 u1 00 u0
               "punpcklbw  %%mm7, %%mm0\n\t" // 00 y3 00 y2 00 y1 00 y0 
               /* mm1 = u, mm2 = v, mm0 = y, mm3 = a */
               "movq       %%mm2, %%mm4\n\t" // 00 v3 00 v2 00 v1 00 v0
               "movq       %%mm1, %%mm5\n\t" // 00 u3 00 y2 00 u1 00 u0
               "punpcklwd  %%mm3, %%mm2\n\t" // 00 a1 00 v1 00 a0 00 v0
               "punpcklwd  %%mm0, %%mm1\n\t" // 00 y1 00 u1 00 y0 00 u0
               "punpckhwd  %%mm3, %%mm4\n\t" // 00 a3 00 v3 00 a2 00 v2
               "punpckhwd  %%mm0, %%mm5\n\t" // 00 y3 00 u3 00 y2 00 u2
               "movq       %%mm1, %%mm3\n\t" // 00 y1 00 u1 00 y0 00 u0
               "movq       %%mm5, %%mm6\n\t" // 00 y3 00 u3 00 y2 00 u2
               "punpcklwd  %%mm2, %%mm1\n\t" // 00 a0 00 y0 00 v0 00 u0
               "punpcklwd  %%mm4, %%mm5\n\t" // 00 a2 00 y2 00 v2 00 u2
               "punpckhwd  %%mm2, %%mm3\n\t" // 00 a1 00 y1 00 v1 00 u1
               "punpckhwd  %%mm4, %%mm6\n\t" // 00 a3 00 y3 00 v3 00 u3
               "movq       %%mm1,  (%0)\n\t"
               "movq       %%mm3, 8(%0)\n\t"
               "movq       %%mm5,16(%0)\n\t"
               "movq       %%mm6,24(%0)\n\t"
               "add          $32, %0\n\t"
               "decl          %1\n\t"
               "jnz           1b\n\t"
          "emms\n\t"
          "2:"
          : "=&D" (D)
          : "c" (gfxs->length), "r" (mul), "m" (*add0), "m" (*add1), "0" (D)
          : "memory" );

     while (w) {
          if (!(D->RGB.a & 0xF000))
               RGB_TO_YCBCR( D->RGB.r, D->RGB.g, D->RGB.b,
                             D->YUV.y, D->YUV.u, D->YUV.v );

          D++;
          w--;
     }
}

