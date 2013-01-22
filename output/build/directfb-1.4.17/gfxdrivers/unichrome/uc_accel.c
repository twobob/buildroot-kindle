/*
   Copyright (c) 2003 Andreas Robinson, All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <config.h>

#include <direct/messages.h>

#include <gfx/convert.h>

#include "unichrome.h"
#include "uc_accel.h"
#include "uc_fifo.h"
#include "mmio.h"

#define UC_ACCEL_BEGIN()                        \
    UcDriverData *ucdrv = (UcDriverData*) drv;  \
    UcDeviceData *ucdev = (UcDeviceData*) dev;  \
    struct uc_fifo *fifo = ucdrv->fifo;         \
    /*printf("entering %s\n", __PRETTY_FUNCTION__)*/

#define UC_ACCEL_END()                                      \
    UC_FIFO_CHECK(fifo);                                    \
    /*printf("leaving %s\n", __PRETTY_FUNCTION__)*/

// Private functions ---------------------------------------------------------

/** Wait until a new command can be set up. */

static inline void uc_waitcmd(UcDriverData* ucdrv, UcDeviceData* ucdev)
{
    int loop = 0;

    if (!ucdev->must_wait)
        return;

    //printf("waitcmd ");

    while (VIA_IN(ucdrv->hwregs, VIA_REG_STATUS) & VIA_CMD_RGTR_BUSY) {
        if (++loop > MAXLOOP) {
            D_ERROR("DirectFB/Unichrome: Timeout waiting for idle command regulator!\n");
            break;
        }
    }

    //printf("waited for %d (0x%x) cycles.\n", loop, loop);

    ucdev->cmd_waitcycles += loop;
    ucdev->must_wait = 0;
}

/** Send commands to 2D/3D engine. */

void uc_emit_commands(void* drv, void* dev)
{
    UC_ACCEL_BEGIN()

    uc_waitcmd(ucdrv, ucdev);

    UC_FIFO_FLUSH(fifo);

    ucdev->must_wait = 1;
}

void uc_flush_texture_cache(void* drv, void* dev)
{
    UC_ACCEL_BEGIN()

    (void) ucdev;

    UC_FIFO_PREPARE(fifo, 16);

    UC_FIFO_ADD_HDR(fifo, (HC_ParaType_Tex << 16) | (HC_SubType_TexGeneral << 24));
    UC_FIFO_ADD (fifo, 0x00000002);

    UC_FIFO_ADD (fifo, 0x0113000d);
    UC_FIFO_ADD (fifo, 0x02ed1316);
    UC_FIFO_ADD (fifo, 0x03071000);

    UC_FIFO_CHECK(fifo);
}

/**
 * Draw a horizontal or vertical line.
 *
 * @param fifo          command FIFO
 *
 * @param x             start x position
 * @param y             start y position
 * @param len           length
 * @param hv            if zero: draw from left to right
 *                      if nonzero: draw from top to bottom.
 *
 * @note This is actually a 1-pixel high or wide rectangular color fill.
 */

static inline void uc_draw_hv_line(struct uc_fifo* fifo,
                                   int x, int y, int len, int hv, int rop)
{
    UC_FIFO_ADD_2D(fifo, VIA_REG_DSTPOS, ((RS16(y) << 16) | RS16(x)));
    UC_FIFO_ADD_2D(fifo, VIA_REG_DIMENSION, len << (hv ? 16 : 0));
    UC_FIFO_ADD_2D(fifo, VIA_REG_GECMD, VIA_GEC_BLT | VIA_GEC_FIXCOLOR_PAT
        | rop | VIA_GEC_CLIP_ENABLE);
}

// DirectFB interfacing functions --------------------------------------------

// Functions using the 2D engine ---

bool uc_fill_rectangle(void* drv, void* dev, DFBRectangle* r)
{
    UC_ACCEL_BEGIN()

    //printf("%s: r = {%d, %d, %d, %d}, c = 0x%08x\n", __PRETTY_FUNCTION__,
    //  r->x, r->y, r->w, r->h, ucdev->color);

    if (r->w == 0 || r->h == 0) return true;

    UC_FIFO_PREPARE(fifo, 8);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_NotTex << 16);

    UC_FIFO_ADD_2D(fifo, VIA_REG_DSTPOS, ((RS16(r->y) << 16) | RS16(r->x)));
    UC_FIFO_ADD_2D(fifo, VIA_REG_DIMENSION,
        (((RS16(r->h - 1)) << 16) | RS16((r->w - 1))));
    UC_FIFO_ADD_2D(fifo, VIA_REG_GECMD, VIA_GEC_BLT | VIA_GEC_FIXCOLOR_PAT
        | ucdev->draw_rop2d | VIA_GEC_CLIP_ENABLE);

    UC_ACCEL_END();
    return true;
}

bool uc_draw_rectangle(void* drv, void* dev, DFBRectangle* r)
{
    UC_ACCEL_BEGIN()

    //printf("%s: r = {%d, %d, %d, %d}, c = 0x%08x\n", __PRETTY_FUNCTION__,
    //  r->x, r->y, r->w, r->h, ucdev->color);

    int rop = ucdev->draw_rop2d;

    // Draw lines, in this order: top, bottom, left, right

    UC_FIFO_PREPARE(fifo, 26);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_NotTex << 16);

    uc_draw_hv_line(fifo, r->x, r->y, r->w - 1, 0, rop);
    uc_draw_hv_line(fifo, r->x, r->y + r->h - 1, r->w - 1, 0, rop);
    uc_draw_hv_line(fifo, r->x, r->y, r->h - 1, 1, rop);
    uc_draw_hv_line(fifo, r->x + r->w - 1, r->y, r->h - 1, 1, rop);

    UC_ACCEL_END();
    return true;
}

bool uc_draw_line(void* drv, void* dev, DFBRegion* line)
{
    UC_ACCEL_BEGIN()

    //printf("%s: l = (%d, %d) - (%d, %d), c = 0x%08x\n", __PRETTY_FUNCTION__,
    //  line->x1, line->y1, line->x2, line->y2, ucdev->color);

    int cmd;
    int dx, dy, tmp, error;

    error = 1;

    cmd = VIA_GEC_LINE | VIA_GEC_FIXCOLOR_PAT | ucdev->draw_rop2d
        | VIA_GEC_CLIP_ENABLE;

    dx = line->x2 - line->x1;
    if (dx < 0)
    {
        dx = -dx;
        cmd |= VIA_GEC_DECX;        // line will be drawn from right
        error = 0;
    }

    dy = line->y2 - line->y1;
    if (dy < 0)
    {
        dy = -dy;
        cmd |= VIA_GEC_DECY;        // line will be drawn from bottom
    }

    if (dy > dx)
    {
        tmp  = dy;
        dy = dx;
        dx = tmp;                   // Swap 'dx' and 'dy'
        cmd |= VIA_GEC_Y_MAJOR;     // Y major line
    }

    UC_FIFO_PREPARE(fifo, 12);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_NotTex << 16);

    UC_FIFO_ADD_2D(fifo, VIA_REG_LINE_K1K2,
        ((((dy << 1) & 0x3fff) << 16)| (((dy - dx) << 1) & 0x3fff)));
    UC_FIFO_ADD_2D(fifo, VIA_REG_LINE_XY,
        ((RS16(line->y1) << 16) | RS16(line->x1)));
    UC_FIFO_ADD_2D(fifo, VIA_REG_DIMENSION, dx);
    UC_FIFO_ADD_2D(fifo, VIA_REG_LINE_ERROR,
        (((dy << 1) - dx - error) & 0x3fff));
    UC_FIFO_ADD_2D(fifo, VIA_REG_GECMD, cmd);

    UC_ACCEL_END();
    return true;
}

static bool uc_blit_one_plane(void* drv, void* dev, DFBRectangle* rect, int dx, int dy)
{
    UC_ACCEL_BEGIN()

    //printf("%s: r = (%d, %d, %d, %d) -> (%d, %d)\n", __PRETTY_FUNCTION__,
    //  rect->x, rect->y, rect->h, rect->w, dx, dy);

    int cmd = VIA_GEC_BLT | VIA_ROP_S | VIA_GEC_CLIP_ENABLE;

    int sx = rect->x;
    int sy = rect->y;
    int w = rect->w;
    int h = rect->h;

    if (!w || !h) return true;

    (void) ucdev; // Kill 'unused variable' compiler warning.

    if (sx < dx) {
        cmd |= VIA_GEC_DECX;
        sx += w - 1;
        dx += w - 1;
    }

    if (sy < dy) {
        cmd |= VIA_GEC_DECY;
        sy += h - 1;
        dy += h - 1;
    }

    UC_FIFO_PREPARE(fifo, 10);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_NotTex << 16);

    UC_FIFO_ADD_2D(fifo, VIA_REG_SRCPOS, (RS16(sy) << 16) | RS16(sx));
    UC_FIFO_ADD_2D(fifo, VIA_REG_DSTPOS, (RS16(dy) << 16) | RS16(dx));
    UC_FIFO_ADD_2D(fifo, VIA_REG_DIMENSION, (RS16(h - 1) << 16) | RS16(w - 1));
    UC_FIFO_ADD_2D(fifo, VIA_REG_GECMD, cmd);

    UC_ACCEL_END();
    return true;
}

static bool uc_blit_planar(void* drv, void* dev, DFBRectangle* rect, int dx, int dy)
{
    UC_ACCEL_BEGIN()

    int uv_dst_offset = ucdev->dst_offset + (ucdev->dst_pitch * ucdev->dst_height);
    int uv_src_offset = ucdev->src_offset + (ucdev->src_pitch * ucdev->src_height);
    
    int uv_dst_pitch = ucdev->dst_pitch / 2;
    int uv_src_pitch = ucdev->src_pitch / 2;
    int uv_pitch = ((uv_src_pitch >> 3) & 0x7fff) | (((uv_dst_pitch >> 3) & 0x7fff) << 16);
    
    DFBRectangle rect2 = *rect;
    rect2.h /= 2;
    rect2.w /= 2;
    rect2.x /= 2;
    rect2.y /= 2;
    
    // first blit the Y plane
    
    uc_blit_one_plane(drv, dev, rect, dx, dy);
    
    // now modify the offsets and clip region for the first chrominance plane
    
    UC_FIFO_PREPARE ( fifo, 12 );
    UC_FIFO_ADD_HDR( fifo, HC_ParaType_NotTex << 16 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_PITCH, VIA_PITCH_ENABLE | uv_pitch );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_SRCBASE, uv_src_offset >> 3 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_DSTBASE, uv_dst_offset >> 3 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_CLIPTL,
                      (RS16(ucdev->clip.y1/2) << 16) | RS16(ucdev->clip.x1/2) );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_CLIPBR,
                      (RS16(ucdev->clip.y2/2) << 16) | RS16(ucdev->clip.x2/2) );
    UC_FIFO_CHECK ( fifo );
    
    uc_blit_one_plane(drv, dev, &rect2, dx/2, dy/2);
    
    // now for the second chrominance plane
    
    uv_src_offset += uv_src_pitch * ucdev->src_height/2;
    uv_dst_offset += uv_dst_pitch * ucdev->dst_height/2;
    
    UC_FIFO_PREPARE ( fifo, 6 );
    UC_FIFO_ADD_HDR( fifo, HC_ParaType_NotTex << 16 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_SRCBASE, uv_src_offset >> 3 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_DSTBASE, uv_dst_offset >> 3 );
    UC_FIFO_CHECK ( fifo );

    uc_blit_one_plane(drv, dev, &rect2, dx/2, dy/2);
    
    // restore the card state to how we found it

    UC_FIFO_PREPARE ( fifo, 12 );
    UC_FIFO_ADD_HDR( fifo, HC_ParaType_NotTex << 16 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_PITCH, VIA_PITCH_ENABLE | ucdev->pitch );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_SRCBASE, ucdev->src_offset >> 3 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_DSTBASE, ucdev->dst_offset >> 3 );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_CLIPTL,
                      (RS16(ucdev->clip.y1) << 16) | RS16(ucdev->clip.x1) );
    UC_FIFO_ADD_2D ( fifo, VIA_REG_CLIPBR,
                      (RS16(ucdev->clip.y2) << 16) | RS16(ucdev->clip.x2) );
    UC_FIFO_CHECK ( fifo );
     
    UC_ACCEL_END();
    return true;
}

bool uc_blit(void* drv, void* dev, DFBRectangle* rect, int dx, int dy)
{
    DFBSurfacePixelFormat format = ((UcDeviceData *)dev)->dst_format;
    if (format == DSPF_YV12 || format == DSPF_I420)
        return uc_blit_planar(drv, dev, rect, dx, dy);
    else
        return uc_blit_one_plane(drv, dev, rect, dx, dy);
}

// Functions using the 3D engine ---

bool uc_fill_rectangle_3d(void* drv, void* dev, DFBRectangle* r)
{
    UC_ACCEL_BEGIN()

    //printf("%s: r = {%d, %d, %d, %d}, c = 0x%08x\n", __PRETTY_FUNCTION__,
    //  r->x, r->y, r->w, r->h, ucdev->color3d);

    int cmdB = HC_ACMD_HCmdB | HC_HVPMSK_X | HC_HVPMSK_Y | HC_HVPMSK_Cd;
    int cmdA = HC_ACMD_HCmdA | HC_HPMType_Tri | HC_HVCycle_AFP |
        HC_HVCycle_AA | HC_HVCycle_BB | HC_HVCycle_NewC | HC_HShading_FlatC;
    int cmdA_End = cmdA | HC_HPLEND_MASK | HC_HPMValidN_MASK | HC_HE3Fire_MASK;

    if (r->w == 0 || r->h == 0) return true;

    UC_FIFO_PREPARE(fifo, 18);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_CmdVdata << 16);
    UC_FIFO_ADD(fifo, cmdB);
    UC_FIFO_ADD(fifo, cmdA);

    UC_FIFO_ADD_XYC(fifo, r->x, r->y, 0);
    UC_FIFO_ADD_XYC(fifo, r->x + r->w, r->y + r->h, 0);
    UC_FIFO_ADD_XYC(fifo, r->x + r->w, r->y, ucdev->color3d);
    UC_FIFO_ADD_XYC(fifo, r->x, r->y + r->h, ucdev->color3d);

    UC_FIFO_ADD(fifo, cmdA_End);

    UC_FIFO_PAD_EVEN(fifo);

    UC_ACCEL_END();
    return true;
}

bool uc_draw_rectangle_3d(void* drv, void* dev, DFBRectangle* r)
{
    UC_ACCEL_BEGIN()

    int cmdB = HC_ACMD_HCmdB | HC_HVPMSK_X | HC_HVPMSK_Y | HC_HVPMSK_Cd;
    int cmdA = HC_ACMD_HCmdA | HC_HPMType_Line | HC_HVCycle_AFP | HC_HShading_FlatA;
    int cmdA_End = cmdA | HC_HPLEND_MASK | HC_HPMValidN_MASK | HC_HE3Fire_MASK;

    UC_FIFO_PREPARE(fifo, 20);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_CmdVdata << 16);
    UC_FIFO_ADD(fifo, cmdB);
    UC_FIFO_ADD(fifo, cmdA);

    UC_FIFO_ADD_XYC(fifo, r->x, r->y, ucdev->color3d);
    UC_FIFO_ADD_XYC(fifo, r->x + r->w - 1, r->y, ucdev->color3d);
    UC_FIFO_ADD_XYC(fifo, r->x + r->w - 1, r->y + r->h - 1, ucdev->color3d);
    UC_FIFO_ADD_XYC(fifo, r->x, r->y + r->h - 1, ucdev->color3d);
    UC_FIFO_ADD_XYC(fifo, r->x, r->y, ucdev->color3d);

    UC_FIFO_ADD(fifo, cmdA_End);

    UC_ACCEL_END();
    return true;
}

bool uc_draw_line_3d(void* drv, void* dev, DFBRegion* line)
{
    UC_ACCEL_BEGIN()

    int cmdB = HC_ACMD_HCmdB | HC_HVPMSK_X | HC_HVPMSK_Y | HC_HVPMSK_Cd;
    int cmdA = HC_ACMD_HCmdA | HC_HPMType_Line | HC_HVCycle_Full | HC_HShading_FlatA;
    int cmdA_End = cmdA | HC_HPLEND_MASK | HC_HPMValidN_MASK | HC_HE3Fire_MASK;

    UC_FIFO_PREPARE(fifo, 12);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_CmdVdata << 16);
    UC_FIFO_ADD(fifo, cmdB);
    UC_FIFO_ADD(fifo, cmdA);

    UC_FIFO_ADD_XYC(fifo, line->x1, line->y1, ucdev->color3d);
    UC_FIFO_ADD_XYC(fifo, line->x2, line->y2, 0);

    UC_FIFO_ADD(fifo, cmdA_End);

    UC_FIFO_PAD_EVEN(fifo);

    UC_ACCEL_END();
    return true;
}

bool uc_fill_triangle(void* drv, void* dev, DFBTriangle* tri)
{
    UC_ACCEL_BEGIN()

    int cmdB = HC_ACMD_HCmdB | HC_HVPMSK_X | HC_HVPMSK_Y | HC_HVPMSK_Cd;
    int cmdA = HC_ACMD_HCmdA | HC_HPMType_Tri | HC_HVCycle_Full | HC_HShading_FlatA;
    int cmdA_End = cmdA | HC_HPLEND_MASK | HC_HPMValidN_MASK | HC_HE3Fire_MASK;

    UC_FIFO_PREPARE(fifo, 14);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_CmdVdata << 16);
    UC_FIFO_ADD(fifo, cmdB);
    UC_FIFO_ADD(fifo, cmdA);

    UC_FIFO_ADD_XYC(fifo, tri->x1, tri->y1, ucdev->color3d);
    UC_FIFO_ADD_XYC(fifo, tri->x2, tri->y2, 0);
    UC_FIFO_ADD_XYC(fifo, tri->x3, tri->y3, 0);

    UC_FIFO_ADD(fifo, cmdA_End);

    UC_ACCEL_END();
    return true;
}

bool uc_blit_3d(void* drv, void* dev,
                DFBRectangle* rect, int dx, int dy)
{
    DFBRectangle dest = {dx, dy, rect->w, rect->h};
    return uc_stretch_blit(drv, dev, rect, &dest);
}

bool uc_stretch_blit(void* drv, void* dev,
                     DFBRectangle* sr, DFBRectangle* dr)
{
    UC_ACCEL_BEGIN()

    float w = ucdev->hwtex.l2w;
    float h = ucdev->hwtex.l2h;

    float dy = dr->y;

    float s1 = (sr->x        ) / w;
    float t1 = (sr->y        ) / h;
    float s2 = (sr->x + sr->w) / w;
    float t2 = (sr->y + sr->h) / h;

    int cmdB = HC_ACMD_HCmdB | HC_HVPMSK_X | HC_HVPMSK_Y | HC_HVPMSK_W |
               HC_HVPMSK_Cd  | HC_HVPMSK_S | HC_HVPMSK_T;

    int cmdA = HC_ACMD_HCmdA | HC_HPMType_Tri | HC_HShading_FlatC |
               HC_HVCycle_AFP | HC_HVCycle_AA | HC_HVCycle_BB | HC_HVCycle_NewC;

    int cmdA_End = cmdA | HC_HPLEND_MASK | HC_HPMValidN_MASK | HC_HE3Fire_MASK;

    if (ucdev->bflags & DSBLIT_DEINTERLACE) {
         t1 *= 0.5f;
         t2 *= 0.5f;

         if (ucdev->field)
             dy += 0.5f;
         else
             dy -= 0.5f;
    }

    UC_FIFO_PREPARE(fifo, 30);

    UC_FIFO_ADD_HDR(fifo, HC_ParaType_CmdVdata << 16);
    UC_FIFO_ADD(fifo, cmdB);
    UC_FIFO_ADD(fifo, cmdA);

    UC_FIFO_ADD_XYWCST(fifo, dr->x+dr->w, dy,       1, 0,              s2, t1);
    UC_FIFO_ADD_XYWCST(fifo, dr->x,       dy+dr->h, 1, 0,              s1, t2);
    UC_FIFO_ADD_XYWCST(fifo, dr->x,       dy,       1, ucdev->color3d, s1, t1);
    UC_FIFO_ADD_XYWCST(fifo, dr->x+dr->w, dy+dr->h, 1, ucdev->color3d, s2, t2);

    UC_FIFO_ADD(fifo, cmdA_End);

    UC_FIFO_PAD_EVEN(fifo);

    UC_ACCEL_END();

    return true;
}

#define DFBCOLOR_TO_ARGB(c)   PIXEL_ARGB( (c).a, (c).r, (c).g, (c).b )

bool uc_texture_triangles( void *drv, void *dev,
                           DFBVertex *vertices, int num,
                           DFBTriangleFormation formation )
{
     UC_ACCEL_BEGIN()

     int i;

     int cmdB = HC_ACMD_HCmdB |
                HC_HVPMSK_X   | HC_HVPMSK_Y | HC_HVPMSK_Z | HC_HVPMSK_W |
                HC_HVPMSK_Cd  | HC_HVPMSK_S | HC_HVPMSK_T;

     int cmdA = HC_ACMD_HCmdA | HC_HPMType_Tri | HC_HShading_Gouraud |
                HC_HVCycle_Full;

     int cmdA_End = cmdA | HC_HPLEND_MASK | HC_HPMValidN_MASK | HC_HE3Fire_MASK;


     switch (formation) {
          case DTTF_LIST:
               cmdA |= HC_HVCycle_NewA | HC_HVCycle_NewB | HC_HVCycle_NewC;
               break;
          case DTTF_STRIP:
               cmdA |= HC_HVCycle_AB | HC_HVCycle_BC | HC_HVCycle_NewC;
               break;
          case DTTF_FAN:
               cmdA |= HC_HVCycle_AA | HC_HVCycle_BC | HC_HVCycle_NewC;
               break;
          default:
               D_ONCE( "unknown triangle formation" );
               return false;
     }

     UC_FIFO_PREPARE(fifo, 6 + num * 7);

     UC_FIFO_ADD_HDR(fifo, HC_ParaType_CmdVdata << 16);
     UC_FIFO_ADD(fifo, cmdB);
     UC_FIFO_ADD(fifo, cmdA);

     for (i=0; i<num; i++) {
          UC_FIFO_ADD_XYZWCST(fifo,
                              vertices[i].x, vertices[i].y,
                              vertices[i].z, vertices[i].w, ucdev->color3d,
                              vertices[i].s, vertices[i].t);
     }

     UC_FIFO_ADD(fifo, cmdA_End);

     UC_FIFO_PAD_EVEN(fifo);

     UC_ACCEL_END();

     return true;
}

    // Blit profiling

    //struct timeval tv_start, tv_stop;
    //gettimeofday(&tv_start, NULL);

    // Run test here

    //gettimeofday(&tv_stop, NULL);

    //tv_stop.tv_sec -= tv_start.tv_sec;
    //tv_stop.tv_usec -= tv_start.tv_usec;
    //if (tv_stop.tv_usec < 0) {
    //  tv_stop.tv_sec--;
    //  tv_stop.tv_usec += 1000000;
    //}

    //printf("elapsed time: %d us\n", tv_stop.tv_usec);
