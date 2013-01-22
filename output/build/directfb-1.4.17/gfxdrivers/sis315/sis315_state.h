/*
 * $Id: sis315_state.h,v 1.2 2006-10-29 23:24:50 dok Exp $
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

#ifndef _SIS315_STATE_H
#define _SIS315_STATE_H

void sis_validate_color(SiSDriverData *drv, SiSDeviceData *dev, CardState *state);
void sis_validate_dst(SiSDriverData *drv, SiSDeviceData *dev, CardState *state);
void sis_validate_src(SiSDriverData *drv, SiSDeviceData *dev, CardState *state);
void sis_set_dst_colorkey(SiSDriverData *drv, SiSDeviceData *dev, CardState *state);
void sis_set_src_colorkey(SiSDriverData *drv, SiSDeviceData *dev, CardState *state);
void sis_set_clip(SiSDriverData *drv, DFBRegion *clip);


#endif /* _SIS315_STATE_H */
