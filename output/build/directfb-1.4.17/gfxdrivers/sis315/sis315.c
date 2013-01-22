/*
 * $Id: sis315.c,v 1.20 2007-01-29 01:00:45 dok Exp $
 *
 * Copyright (C) 2003 by Andreas Oberritter <obi@saftware.de>
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
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 *
 */

#include <config.h>

#include <stdio.h>
#include <sys/ioctl.h>

#include <fbdev/fbdev.h>  /* FIXME: Needs to be included before dfb_types.h to work around a type clash with asm/types.h */

#include <directfb.h>

#include <direct/mem.h>
#include <direct/messages.h>

#include <core/gfxcard.h>
#include <core/graphics_driver.h>
#include <core/state.h>
#include <core/surface.h>

#include <fbdev/fbdev.h>

#include "sis315.h"
#include "sis315_accel.h"
#include "sis315_compat.h"
#include "sis315_state.h"

DFB_GRAPHICS_DRIVER(sis315);

#define SIS_SUPPORTED_DRAWING_FUNCTIONS		\
	(DFXL_FILLRECTANGLE | DFXL_DRAWRECTANGLE | DFXL_DRAWLINE)
#define SIS_SUPPORTED_DRAWING_FLAGS		\
	(DSDRAW_NOFX)
#define SIS_SUPPORTED_BLITTING_FUNCTIONS	\
	(DFXL_BLIT | DFXL_STRETCHBLIT)
#define SIS_SUPPORTED_BLITTING_FLAGS		\
	(DSBLIT_SRC_COLORKEY)

static DFBResult sis_engine_sync(void *driver_data, void *device_data)
{
	(void)driver_data;
	(void)device_data;

	/*
	 * this driver syncs after every command,
	 * so this function can be left empty
	 */

    return DFB_OK;
}

static void sis_check_state(void *driver_data, void *device_data,
			    CardState *state, DFBAccelerationMask accel)
{
	(void)driver_data;
	(void)device_data;

	switch (state->destination->config.format) {
	case DSPF_LUT8:
	case DSPF_ARGB1555:
	case DSPF_RGB16:
	case DSPF_RGB32:
	case DSPF_ARGB:
		break;
	default:
		return;
	}

	if (DFB_DRAWING_FUNCTION(accel)) {
		if (state->drawingflags & ~SIS_SUPPORTED_DRAWING_FLAGS)
			return;
		if (accel & DFXL_FILLTRIANGLE) {
			/* this is faster. don't know why. */
			state->accel = 0;
			return;
		}
		state->accel |= SIS_SUPPORTED_DRAWING_FUNCTIONS;
	}
	else {
		if (state->blittingflags & ~SIS_SUPPORTED_BLITTING_FLAGS)
			return;

		switch (state->source->config.format) {
		case DSPF_LUT8:
		case DSPF_ARGB1555:
		case DSPF_RGB16:
		case DSPF_RGB32:
		case DSPF_ARGB:
			break;
		default:
			return;
		}

		if (state->source->config.format != state->destination->config.format)
			return;

		state->accel |= SIS_SUPPORTED_BLITTING_FUNCTIONS;
	}
}

static void sis_set_state(void *driver_data, void *device_data,
			  GraphicsDeviceFuncs *funcs, CardState *state,
			  DFBAccelerationMask accel)
{
	SiSDriverData *drv = (SiSDriverData *)driver_data;
	SiSDeviceData *dev = (SiSDeviceData *)device_data;

	(void)funcs;

	if (state->mod_hw) {
		if (state->mod_hw & SMF_SOURCE)
			dev->v_source = 0;

		if (state->mod_hw & SMF_DESTINATION)
			dev->v_color = dev->v_destination = 0;
		else if (state->mod_hw & SMF_COLOR)
			dev->v_color = 0;

		if (state->mod_hw & SMF_SRC_COLORKEY)
			dev->v_src_colorkey = 0;

//		if (state->mod_hw & SMF_BLITTING_FLAGS)
//			dev->v_blittingflags = 0;
	}

	switch (accel) {
	case DFXL_FILLRECTANGLE:
	case DFXL_DRAWRECTANGLE:
	case DFXL_DRAWLINE:
		sis_validate_dst(drv, dev, state);
		sis_validate_color(drv, dev, state);
		state->set = SIS_SUPPORTED_DRAWING_FUNCTIONS;
		break;
	case DFXL_BLIT:
		sis_validate_src(drv, dev, state);
		sis_validate_dst(drv, dev, state);
		if (state->blittingflags & DSBLIT_DST_COLORKEY)
			sis_set_dst_colorkey(drv, dev, state);
		if (state->blittingflags & DSBLIT_SRC_COLORKEY)
			sis_set_src_colorkey(drv, dev, state);
		state->set = SIS_SUPPORTED_BLITTING_FUNCTIONS;
		break;
	case DFXL_STRETCHBLIT:
		sis_validate_src(drv, dev, state);
		sis_validate_dst(drv, dev, state);
		if (state->blittingflags & DSBLIT_DST_COLORKEY)
			sis_set_dst_colorkey(drv, dev, state);
		if (state->blittingflags & DSBLIT_SRC_COLORKEY)
			sis_set_src_colorkey(drv, dev, state);
		state->set = DFXL_STRETCHBLIT;
		break;
	default:
		D_BUG("unexpected drawing or blitting function");
		break;
	}

	if ((state->mod_hw & SMF_CLIP) && (accel!=DFXL_STRETCHBLIT))
		sis_set_clip(drv, &state->clip);

	state->mod_hw = 0;
}

static void check_sisfb_version(SiSDriverData *drv, const struct sisfb_info *i)
{
	u32 sisfb_version = SISFB_VERSION(i->sisfb_version,
					  i->sisfb_revision,
					  i->sisfb_patchlevel);

	if (sisfb_version < SISFB_VERSION(1, 6, 23)) {
		fprintf(stderr, "*** Warning: sisfb version < 1.6.23 detected, "
				"please update your driver! ***\n");
		drv->has_auto_maximize = false;
	}
	else {
		drv->has_auto_maximize = true;
	}
}

/*
 * exported symbols...
 */

static int driver_probe(CoreGraphicsDevice *device)
{
	switch (dfb_gfxcard_get_accelerator(device)) {
	case FB_ACCEL_SIS_GLAMOUR_2:
	case FB_ACCEL_SIS_XABRE:
	case FB_ACCEL_XGI_VOLARI_Z:
		return 1;
	default:
		return 0;
	}
}

static void driver_get_info(CoreGraphicsDevice *device,
			    GraphicsDriverInfo *info)
{
	(void)device;

	snprintf(info->name, DFB_GRAPHICS_DRIVER_INFO_NAME_LENGTH,
			"SiS 315 Driver");
	snprintf(info->vendor, DFB_GRAPHICS_DRIVER_INFO_VENDOR_LENGTH,
			"Andreas Oberritter <obi@saftware.de>");

	info->version.major = 0;
	info->version.minor = 1;

	info->driver_data_size = sizeof(SiSDriverData);
	info->device_data_size = sizeof(SiSDeviceData);
}

static DFBResult driver_init_driver(CoreGraphicsDevice *device,
				    GraphicsDeviceFuncs *funcs,
				    void *driver_data,
				    void *device_data,
                    CoreDFB *core)
{
	SiSDriverData *drv = (SiSDriverData *)driver_data;
	FBDev *dfb_fbdev;
	struct sisfb_info *fbinfo;
	u32 fbinfo_size;
	u32 zero = 0;

	(void)device_data;

	dfb_fbdev = dfb_system_data();
	if (!dfb_fbdev)
		return DFB_IO;

	if (ioctl(dfb_fbdev->fd, SISFB_GET_INFO_SIZE, &fbinfo_size) == 0) {
		fbinfo = D_MALLOC(fbinfo_size);
		drv->get_info = SISFB_GET_INFO | (fbinfo_size << 16);
		drv->get_automaximize = SISFB_GET_AUTOMAXIMIZE;
		drv->set_automaximize = SISFB_SET_AUTOMAXIMIZE;
	}
	else {
		fbinfo = D_MALLOC(sizeof(struct sisfb_info));
		drv->get_info = SISFB_GET_INFO_OLD;
		drv->get_automaximize = SISFB_GET_AUTOMAXIMIZE_OLD;
		drv->set_automaximize = SISFB_SET_AUTOMAXIMIZE_OLD;
	}

	if (fbinfo == NULL)
		return DFB_NOSYSTEMMEMORY;

	if (ioctl(dfb_fbdev->fd, drv->get_info, fbinfo) == -1) {
		D_FREE(fbinfo);
		return DFB_IO;
	}

	check_sisfb_version(drv, fbinfo);

	D_FREE(fbinfo);

	if (drv->has_auto_maximize) {
		if (ioctl(dfb_fbdev->fd, drv->get_automaximize, &drv->auto_maximize))
			return DFB_IO;
		if (drv->auto_maximize)
			if (ioctl(dfb_fbdev->fd, drv->set_automaximize, &zero))
				return DFB_IO;
	}

	drv->mmio_base = dfb_gfxcard_map_mmio(device, 0, -1);
	if (!drv->mmio_base)
		return DFB_IO;

	/* base functions */
	funcs->EngineSync = sis_engine_sync;
	funcs->CheckState = sis_check_state;
	funcs->SetState = sis_set_state;

	/* drawing functions */
	funcs->FillRectangle = sis_fill_rectangle;
	funcs->DrawRectangle = sis_draw_rectangle;
	funcs->DrawLine = sis_draw_line;

	/* blitting functions */
	funcs->Blit = sis_blit;
	funcs->StretchBlit = sis_stretchblit;

	/* allocate buffer for stretchBlit with colorkey */
	drv->buffer_offset = dfb_gfxcard_reserve_memory( device, 1024*768*4 );

	return DFB_OK;
}

static DFBResult driver_init_device(CoreGraphicsDevice *device,
				    GraphicsDeviceInfo *device_info,
				    void *driver_data,
				    void *device_data)
{
	(void)device;
	(void)driver_data;
	(void)device_data;

	snprintf(device_info->name,
			DFB_GRAPHICS_DEVICE_INFO_NAME_LENGTH, "315");
	snprintf(device_info->vendor,
			DFB_GRAPHICS_DEVICE_INFO_VENDOR_LENGTH, "SiS");

	device_info->caps.flags = CCF_CLIPPING;
	device_info->caps.accel = SIS_SUPPORTED_DRAWING_FUNCTIONS |
				SIS_SUPPORTED_BLITTING_FUNCTIONS;
	device_info->caps.drawing = SIS_SUPPORTED_DRAWING_FLAGS;
	device_info->caps.blitting = SIS_SUPPORTED_BLITTING_FLAGS;

	device_info->limits.surface_byteoffset_alignment = 32 * 4;
	device_info->limits.surface_pixelpitch_alignment = 32;

	return DFB_OK;
}

static void driver_close_device(CoreGraphicsDevice *device,
				void *driver_data,
				void *device_data)
{
	(void)device;
	(void)driver_data;
	(void)device_data;
}

static void driver_close_driver(CoreGraphicsDevice *device,
				void *driver_data)
{
	SiSDriverData *drv = (SiSDriverData *)driver_data;

	dfb_gfxcard_unmap_mmio(device, drv->mmio_base, -1);

	if ((drv->has_auto_maximize) && (drv->auto_maximize)) {
		FBDev *dfb_fbdev = dfb_system_data();
		if (!dfb_fbdev)
			return;
		ioctl(dfb_fbdev->fd, drv->set_automaximize, &drv->auto_maximize);
	}
}

