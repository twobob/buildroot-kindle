/*
 * File: include/asm-arm/arch-omap/omapfb.h
 *
 * Framebuffer driver for TI OMAP boards
 *
 * Copyright (C) 2004 Nokia Corporation
 * Author: Imre Deak <imre.deak@nokia.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef __OMAPFB_H
#define __OMAPFB_H

#include <asm/ioctl.h>
#include <asm/types.h>

/* IOCTL commands. */

#define OMAP_IOW(num, dtype)	_IOW('O', num, dtype)
#define OMAP_IOR(num, dtype)	_IOR('O', num, dtype)
#define OMAP_IOWR(num, dtype)	_IOWR('O', num, dtype)
#define OMAP_IO(num)		_IO('O', num)

#define OMAPFB_MIRROR		OMAP_IOW(31, int)
#define OMAPFB_SYNC_GFX		OMAP_IO(37)
#define OMAPFB_VSYNC		OMAP_IO(38)
#define OMAPFB_SET_UPDATE_MODE	OMAP_IOW(40, int)
#define OMAPFB_GET_CAPS		OMAP_IOR(42, struct omapfb_caps)
#define OMAPFB_GET_UPDATE_MODE	OMAP_IOW(43, int)
#define OMAPFB_LCD_TEST		OMAP_IOW(45, int)
#define OMAPFB_CTRL_TEST	OMAP_IOW(46, int)
#define OMAPFB_UPDATE_WINDOW_OLD OMAP_IOW(47, struct omapfb_update_window_old)
#define OMAPFB_SET_COLOR_KEY	OMAP_IOW(50, struct omapfb_color_key)
#define OMAPFB_GET_COLOR_KEY	OMAP_IOW(51, struct omapfb_color_key)
#define OMAPFB_SETUP_PLANE	OMAP_IOW(52, struct omapfb_plane_info)
#define OMAPFB_QUERY_PLANE	OMAP_IOW(53, struct omapfb_plane_info)
#define OMAPFB_UPDATE_WINDOW	OMAP_IOW(54, struct omapfb_update_window)
#define OMAPFB_SETUP_MEM	OMAP_IOW(55, struct omapfb_mem_info)
#define OMAPFB_QUERY_MEM	OMAP_IOW(56, struct omapfb_mem_info)

#define OMAPFB_CAPS_GENERIC_MASK	0x00000fff
#define OMAPFB_CAPS_LCDC_MASK		0x00fff000
#define OMAPFB_CAPS_PANEL_MASK		0xff000000

#define OMAPFB_CAPS_MANUAL_UPDATE	0x00001000
#define OMAPFB_CAPS_TEARSYNC		0x00002000
#define OMAPFB_CAPS_PLANE_RELOCATE_MEM	0x00004000
#define OMAPFB_CAPS_PLANE_SCALE		0x00008000
#define OMAPFB_CAPS_WINDOW_PIXEL_DOUBLE	0x00010000
#define OMAPFB_CAPS_WINDOW_SCALE	0x00020000
#define OMAPFB_CAPS_WINDOW_OVERLAY	0x00040000
#define OMAPFB_CAPS_SET_BACKLIGHT	0x01000000

/* Values from DSP must map to lower 16-bits */
#define OMAPFB_FORMAT_MASK		0x00ff
#define OMAPFB_FORMAT_FLAG_DOUBLE	0x0100
#define OMAPFB_FORMAT_FLAG_TEARSYNC	0x0200
#define OMAPFB_FORMAT_FLAG_FORCE_VSYNC	0x0400
#define OMAPFB_FORMAT_FLAG_ENABLE_OVERLAY	0x0800
#define OMAPFB_FORMAT_FLAG_DISABLE_OVERLAY	0x1000

#define OMAPFB_EVENT_READY	1
#define OMAPFB_EVENT_DISABLED	2

#define OMAPFB_MEMTYPE_SDRAM		0
#define OMAPFB_MEMTYPE_SRAM		1
#define OMAPFB_MEMTYPE_MAX		1

enum omapfb_color_format {
	OMAPFB_COLOR_RGB565 = 0,
	OMAPFB_COLOR_YUV422,
	OMAPFB_COLOR_YUV420,
	OMAPFB_COLOR_CLUT_8BPP,
	OMAPFB_COLOR_CLUT_4BPP,
	OMAPFB_COLOR_CLUT_2BPP,
	OMAPFB_COLOR_CLUT_1BPP,
	OMAPFB_COLOR_RGB444,
	OMAPFB_COLOR_YUY422,
};

struct omapfb_update_window {
	u32 x, y;
	u32 width, height;
	u32 format;
	u32 out_x, out_y;
	u32 out_width, out_height;
	u32 reserved[8];
};

struct omapfb_update_window_old {
	u32 x, y;
	u32 width, height;
	u32 format;
};

enum omapfb_plane {
	OMAPFB_PLANE_GFX = 0,
	OMAPFB_PLANE_VID1,
	OMAPFB_PLANE_VID2,
};

enum omapfb_channel_out {
	OMAPFB_CHANNEL_OUT_LCD = 0,
	OMAPFB_CHANNEL_OUT_DIGIT,
};

struct omapfb_plane_info {
	u32 pos_x;
	u32 pos_y;
	u8  enabled;
	u8  channel_out;
	u8  mirror;
	u8  reserved1;
	u32 out_width;
	u32 out_height;
	u32 reserved2[12];
};

struct omapfb_mem_info {
	u32 size;
	u8  type;
	u8  reserved[3];
};

struct omapfb_caps {
	u32 ctrl;
	u32 plane_color;
	u32 wnd_color;
};

enum omapfb_color_key_type {
	OMAPFB_COLOR_KEY_DISABLED = 0,
	OMAPFB_COLOR_KEY_GFX_DST,
	OMAPFB_COLOR_KEY_VID_SRC,
};

struct omapfb_color_key {
	u8  channel_out;
	u32 background;
	u32 trans_key;
	u8  key_type;
};

enum omapfb_update_mode {
	OMAPFB_UPDATE_DISABLED = 0,
	OMAPFB_AUTO_UPDATE,
	OMAPFB_MANUAL_UPDATE
};

#endif /* __OMAPFB_H */
