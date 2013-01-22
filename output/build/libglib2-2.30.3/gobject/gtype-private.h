/* GObject - GLib Type, Object, Parameter and Signal Library
 * Copyright (C) 1998-1999, 2000-2001 Tim Janik and Red Hat, Inc.
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
 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place, Suite 330,
 * Boston, MA 02111-1307, USA.
 */
#if !defined (__GLIB_GOBJECT_H_INSIDE__) && !defined (GOBJECT_COMPILATION)
#error "Only <glib-object.h> can be included directly."
#endif

#ifndef __G_TYPE_PRIVATE_H__
#define __G_TYPE_PRIVATE_H__

#include "gboxed.h"

G_BEGIN_DECLS

void    _g_value_c_init          (void); /* sync with gvalue.c */
void    _g_value_types_init      (void); /* sync with gvaluetypes.c */
void    _g_enum_types_init       (void); /* sync with genums.c */
void    _g_param_type_init       (void); /* sync with gparam.c */
void    _g_boxed_type_init       (void); /* sync with gboxed.c */
void    _g_object_type_init      (void); /* sync with gobject.c */
void    _g_param_spec_types_init (void); /* sync with gparamspecs.c */
void    _g_value_transforms_init (void); /* sync with gvaluetransform.c */
void    _g_signal_init           (void); /* sync with gsignal.c */

/* for gboxed.c */
gpointer        _g_type_boxed_copy      (GType          type,
                                         gpointer       value);
void            _g_type_boxed_free      (GType          type,
                                         gpointer       value);
void            _g_type_boxed_init      (GType          type,
                                         GBoxedCopyFunc copy_func,
                                         GBoxedFreeFunc free_func);

G_END_DECLS

#endif /* __G_TYPE_PRIVATE_H__ */
