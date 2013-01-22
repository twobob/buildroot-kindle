/* glib-unix.h - Unix specific integration
 * Copyright (C) 2011 Red Hat, Inc.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef __G_UNIX_H__
#define __G_UNIX_H__

/* We need to include the UNIX headers needed to use the APIs below,
 * but we also take this opportunity to include a wide selection of
 * other UNIX headers.  If one of the headers below is broken on some
 * system, work around it here (or better, fix the system or tell
 * people to use a better one).
 */
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <fcntl.h>

#include <glib.h>

#ifndef G_OS_UNIX
#error "This header may only be used on UNIX"
#endif

/**
 * G_UNIX_ERROR:
 *
 * Error domain for API in the "g_unix_" namespace.  Note that there is
 * no exported enumeration mapping %errno.  Instead, all functions
 * ensure that %errno is relevant.  The code for all #G_UNIX_ERROR is
 * always <literal>0</literal>, and the error message is always
 * generated via g_strerror().
 *
 * It is expected that most code will not look at %errno from these
 * APIs. Important cases where one would want to differentiate between
 * errors are already covered by existing cross-platform GLib API,
 * such as e.g. #GFile wrapping %ENOENT.  However, it is provided for
 * completeness, at least.
 */
#define G_UNIX_ERROR (g_unix_error_quark())

GQuark g_unix_error_quark (void);

gboolean g_unix_open_pipe (gint    *fds,
                           gint     flags,
                           GError **error);

gboolean g_unix_set_fd_nonblocking (gint       fd,
                                    gboolean   nonblock,
                                    GError   **error);

GSource *g_unix_signal_source_new  (gint signum);

guint    g_unix_signal_add_full    (gint           priority,
                                    gint           signum,
                                    GSourceFunc    handler,
                                    gpointer       user_data,
                                    GDestroyNotify notify);

guint    g_unix_signal_add         (gint        signum,
                                    GSourceFunc handler,
                                    gpointer    user_data);

#endif
