/* 
 *  Matchbox Keyboard - A lightweight software keyboard.
 *
 *  Authored By Matthew Allum <mallum@o-hand.com>
 *
 *  Copyright (c) 2005 OpenedHand Ltd - http://o-hand.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#ifndef HAVE_MB_KEYBOARD_CAIRO_BACKEND_XFT_H
#define HAVE_MB_KEYBOARD_CAIRO_BACKEND_XFT_H

#include "matchbox-keyboard.h"

#include <cairo/cairo.h>
#include <cairo/cairo-xlib.h>

MBKeyboardUIBackend*
mb_kbd_ui_cairo_init(MBKeyboardUI *ui);

#define MB_KBD_UI_BACKEND_INIT_FUNC(ui)  mb_kbd_ui_cairo_init((ui))

#endif
