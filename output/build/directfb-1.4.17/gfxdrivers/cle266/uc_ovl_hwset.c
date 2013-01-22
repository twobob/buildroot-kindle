/*
   Copyright (c) 2003 Andreas Robinson, All rights reserved.

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.
*/

#include <config.h>

#include <sys/io.h>

#include "unichrome.h"
#include "uc_overlay.h"
#include "vidregs.h"
#include "mmio.h"

#include <direct/messages.h>

#include <core/system.h>

/**
 * Set up the extended video FIFO.
 * @note It will be turned on if ucovl->scrwidth > 1024.
 */

void uc_ovl_setup_fifo(UcOverlayData* ucovl, int scrwidth)
{
     u8* mclk_save = ucovl->mclk_save;

     if (!iopl(3)) {
          if (scrwidth <= 1024) { // Disable
               if (ucovl->extfifo_on) {

                    dfb_layer_wait_vsync(dfb_layer_at(DLID_PRIMARY));

                    outb(0x16, 0x3c4); outb(mclk_save[0], 0x3c5);
                    outb(0x17, 0x3c4); outb(mclk_save[1], 0x3c5);
                    outb(0x18, 0x3c4); outb(mclk_save[2], 0x3c5);
                    ucovl->extfifo_on = false;
               }
          }
          else { // Enable
               if (!ucovl->extfifo_on) {

                    dfb_layer_wait_vsync(dfb_layer_at(DLID_PRIMARY));

                    // Save current setting
                    outb(0x16, 0x3c4); mclk_save[0] = inb(0x3c5);
                    outb(0x17, 0x3c4); mclk_save[1] = inb(0x3c5);
                    outb(0x18, 0x3c4); mclk_save[2] = inb(0x3c5);
                    // Enable extended FIFO
                    outb(0x17, 0x3c4); outb(0x2f, 0x3c5);
                    outb(0x16, 0x3c4); outb((mclk_save[0] & 0xf0) | 0x14, 0x3c5);
                    outb(0x18, 0x3c4); outb(0x56, 0x3c5);
                    ucovl->extfifo_on = true;
               }
          }
     }
     else {
          printf("cle266: could set io perissons\n");
     }
     ucovl->scrwidth = scrwidth;
}

void uc_ovl_vcmd_wait(volatile u8* vio)
{
     while ((VIDEO_IN(vio, V_COMPOSE_MODE)
             & (V1_COMMAND_FIRE | V3_COMMAND_FIRE)));
}

/**
 * Update the video overlay.
 *
 * @param action        = UC_OVL_CHANGE: update everything
 *                      = UC_OVL_FLIP: only flip to the front surface buffer.
 * @param surface       source surface
 *
 * @note: Derived from ddmpeg.c, Upd_Video()
 */

DFBResult uc_ovl_update(UcDriverData* ucdrv,
                        UcOverlayData* ucovl,
                        int action,
                        CoreSurface* surface,
                        CoreSurfaceBufferLock* lock)
{
     int sw, sh, sp, sfmt;   // Source width, height, pitch and format
     int dx, dy;             // Destination position
     int dw, dh;             // Destination width and height
     VideoMode *videomode;
     DFBRectangle scr;       // Screen size

     bool write_buffers = false;
     bool write_settings = false;

     volatile u8* vio = ucdrv->hwregs;

     u32 win_start, win_end;     // Overlay register settings
     u32 zoom, mini;
     u32 dcount, falign, qwpitch;
     u32 y_start, u_start, v_start;
     u32 v_ctrl, fifo_ctrl;

     int offset = lock->offset;


     if (!ucovl->v1.isenabled) return DFB_OK;

     qwpitch = 0;

     // Get screen size
     videomode = dfb_system_current_mode();
     scr.w = videomode ? videomode->xres : 720;
     scr.h = videomode ? videomode->yres : 576;
     scr.x = 0;
     scr.y = 0;

     if (ucovl->scrwidth != scr.w) {
          // FIXME: fix uc_ovl_setup_fifo()
          //    uc_ovl_setup_fifo(ucovl, scr.w);
          action |= UC_OVL_CHANGE;
     }

     D_ASSERT(surface);

     sw = surface->config.size.w;
     sh = surface->config.size.h;
     sp = lock->pitch;
     sfmt = surface->config.format;

     if (ucovl->deinterlace) {
          /*if (ucovl->field)
               offset += sp;*/

          sh /= 2;
          //sp *= 2;
     }

     if (action & UC_OVL_CHANGE) {

          if ((sw > 4096) || (sh > 4096) ||
              (sw < 32) || (sh < 1) || (sp > 0x1fff)) {
               D_DEBUG("Layer surface size is out of bounds.");
               return DFB_INVAREA;
          }

          dx = ucovl->v1.win.x;
          dy = ucovl->v1.win.y;
          dw = ucovl->v1.win.w;
          dh = ucovl->v1.win.h;

          // Get image format, FIFO size, etc.

          uc_ovl_map_v1_control(sfmt, sw, ucovl->hwrev, ucovl->extfifo_on,
                                &v_ctrl, &fifo_ctrl);

          if (ucovl->deinterlace) {
               v_ctrl |= /*V1_BOB_ENABLE |*/ V1_FRAME_BASE;
          }

          // Get layer window.
          // The parts that fall outside the screen are clipped.

          uc_ovl_map_window(scr.w, scr.h, &(ucovl->v1.win), sw, sh,
                            &win_start, &win_end, &ucovl->v1.ox, &ucovl->v1.oy);

          // Get scaling and data-fetch parameters

          // Note: the *_map_?zoom() functions return false if the scaling
          // is out of bounds. We don't act on it for now, because it only
          // makes the display look strange.

          zoom = 0;
          mini = 0;

          uc_ovl_map_vzoom(sh, dh, &zoom, &mini);
          uc_ovl_map_hzoom(sw, dw, &zoom, &mini, &falign, &dcount);
          qwpitch = uc_ovl_map_qwpitch(falign, sfmt, sw);

          write_settings = true;
     }

     if (action & (UC_OVL_FIELD | UC_OVL_FLIP | UC_OVL_CHANGE)) {
          int field = 0;
          // Update the buffer pointers

          if (ucovl->deinterlace) {
               field = ucovl->field;
          }

          uc_ovl_map_buffer(sfmt, offset,
                            ucovl->v1.ox, ucovl->v1.oy, sw, surface->config.size.h, sp, 0/*field*/, &y_start,
                            &u_start, &v_start);

          if (field) {
               y_start |= 0x08000000;
          }

          write_buffers = true;
     }

     // Write to the hardware

/*    if (write_settings || write_buffers)
        uc_ovl_vcmd_wait(vio);*/

     if (write_settings) {

          VIDEO_OUT(vio, V1_CONTROL, v_ctrl);
          VIDEO_OUT(vio, V_FIFO_CONTROL, fifo_ctrl);

          VIDEO_OUT(vio, V1_WIN_START_Y, win_start);
          VIDEO_OUT(vio, V1_WIN_END_Y, win_end);

          VIDEO_OUT(vio, V1_SOURCE_HEIGHT, (sh << 16) | dcount);
          VIDEO_OUT(vio, V12_QWORD_PER_LINE, qwpitch);
          VIDEO_OUT(vio, V1_STRIDE, sp | ((sp >> 1) << 16));

          VIDEO_OUT(vio, V1_MINI_CONTROL, mini);
          VIDEO_OUT(vio, V1_ZOOM_CONTROL, zoom);
     }

     if (write_buffers) {

          VIDEO_OUT(vio, V1_STARTADDR_0, y_start);
          VIDEO_OUT(vio, V1_STARTADDR_CB0, u_start);
          VIDEO_OUT(vio, V1_STARTADDR_CR0, v_start);
     }

     if (write_settings || write_buffers) {
          VIDEO_OUT(vio, V_COMPOSE_MODE, V1_COMMAND_FIRE);
     }

     return DFB_OK;
}

DFBResult uc_ovl_set_adjustment(CoreLayer            *layer,
                                void                 *driver_data,
                                void                 *layer_data,
                                DFBColorAdjustment   *adj)
{
    UcOverlayData* ucovl = (UcOverlayData*) layer_data;
    UcDriverData*  ucdrv = (UcDriverData*) driver_data;
    DFBColorAdjustment* ucadj;
    u32 a1, a2;

    ucadj = &ucovl->v1.adj;

    if (adj->flags & DCAF_BRIGHTNESS)
        ucadj->brightness = adj->brightness;
    if (adj->flags & DCAF_CONTRAST)
        ucadj->contrast = adj->contrast;
    if (adj->flags & DCAF_HUE)
        ucadj->hue = adj->hue;
    if (adj->flags & DCAF_SATURATION)
        ucadj->saturation = adj->saturation;

    uc_ovl_map_adjustment(ucadj, &a1, &a2);

    VIDEO_OUT(ucdrv->hwregs, V1_ColorSpaceReg_1, a1);
    VIDEO_OUT(ucdrv->hwregs, V1_ColorSpaceReg_2, a2);

    return DFB_OK;
}
