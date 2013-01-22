/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2010 Red Hat, Inc.
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

#include "config.h"

#include <errno.h>

#include "gpollableinputstream.h"
#include "gasynchelper.h"
#include "glibintl.h"

/**
 * SECTION:gpollableinputstream
 * @short_description: Interface for pollable input streams
 * @include: gio/gio.h
 * @see_also: #GInputStream, #GPollableOutputStream, #GFileDescriptorBased
 *
 * #GPollableInputStream is implemented by #GInputStream<!-- -->s that
 * can be polled for readiness to read. This can be used when
 * interfacing with a non-GIO API that expects
 * UNIX-file-descriptor-style asynchronous I/O rather than GIO-style.
 *
 * Since: 2.28
 */

G_DEFINE_INTERFACE (GPollableInputStream, g_pollable_input_stream, G_TYPE_INPUT_STREAM)

static gboolean g_pollable_input_stream_default_can_poll         (GPollableInputStream *stream);
static gssize   g_pollable_input_stream_default_read_nonblocking (GPollableInputStream  *stream,
								  void                  *buffer,
								  gsize                  size,
								  GError               **error);

static void
g_pollable_input_stream_default_init (GPollableInputStreamInterface *iface)
{
  iface->can_poll         = g_pollable_input_stream_default_can_poll;
  iface->read_nonblocking = g_pollable_input_stream_default_read_nonblocking;
}

static gboolean
g_pollable_input_stream_default_can_poll (GPollableInputStream *stream)
{
  return TRUE;
}

/**
 * g_pollable_input_stream_can_poll:
 * @stream: a #GPollableInputStream.
 *
 * Checks if @stream is actually pollable. Some classes may implement
 * #GPollableInputStream but have only certain instances of that class
 * be pollable. If this method returns %FALSE, then the behavior of
 * other #GPollableInputStream methods is undefined.
 *
 * For any given stream, the value returned by this method is constant;
 * a stream cannot switch from pollable to non-pollable or vice versa.
 *
 * Returns: %TRUE if @stream is pollable, %FALSE if not.
 *
 * Since: 2.28
 */
gboolean
g_pollable_input_stream_can_poll (GPollableInputStream *stream)
{
  g_return_val_if_fail (G_IS_POLLABLE_INPUT_STREAM (stream), FALSE);

  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE (stream)->can_poll (stream);
}

/**
 * g_pollable_input_stream_is_readable:
 * @stream: a #GPollableInputStream.
 *
 * Checks if @stream can be read.
 *
 * Note that some stream types may not be able to implement this 100%
 * reliably, and it is possible that a call to g_input_stream_read()
 * after this returns %TRUE would still block. To guarantee
 * non-blocking behavior, you should always use
 * g_pollable_input_stream_read_nonblocking(), which will return a
 * %G_IO_ERROR_WOULD_BLOCK error rather than blocking.
 *
 * Returns: %TRUE if @stream is readable, %FALSE if not. If an error
 *   has occurred on @stream, this will result in
 *   g_pollable_input_stream_is_readable() returning %TRUE, and the
 *   next attempt to read will return the error.
 *
 * Since: 2.28
 */
gboolean
g_pollable_input_stream_is_readable (GPollableInputStream *stream)
{
  g_return_val_if_fail (G_IS_POLLABLE_INPUT_STREAM (stream), FALSE);

  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE (stream)->is_readable (stream);
}

/**
 * g_pollable_input_stream_create_source: (skip)
 * @stream: a #GPollableInputStream.
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 *
 * Creates a #GSource that triggers when @stream can be read, or
 * @cancellable is triggered or an error occurs. The callback on the
 * source is of the #GPollableSourceFunc type.
 *
 * As with g_pollable_input_stream_is_readable(), it is possible that
 * the stream may not actually be readable even after the source
 * triggers, so you should use g_pollable_input_stream_read_nonblocking()
 * rather than g_input_stream_read() from the callback.
 *
 * Returns: (transfer full): a new #GSource
 *
 * Since: 2.28
 */
GSource *
g_pollable_input_stream_create_source (GPollableInputStream *stream,
				       GCancellable         *cancellable)
{
  g_return_val_if_fail (G_IS_POLLABLE_INPUT_STREAM (stream), NULL);

  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE (stream)->
	  create_source (stream, cancellable);
}

static gssize
g_pollable_input_stream_default_read_nonblocking (GPollableInputStream  *stream,
						  void                  *buffer,
						  gsize                  size,
						  GError               **error)
{
  if (!g_pollable_input_stream_is_readable (stream))
    {
      g_set_error_literal (error, G_IO_ERROR, G_IO_ERROR_WOULD_BLOCK,
                           g_strerror (EAGAIN));
      return -1;
    }

  return g_input_stream_read (G_INPUT_STREAM (stream), buffer, size,
			      NULL, error);
}

/**
 * g_pollable_input_stream_read_nonblocking:
 * @stream: a #GPollableInputStream
 * @buffer: a buffer to read data into (which should be at least @size
 *     bytes long).
 * @size: the number of bytes you want to read
 * @cancellable: (allow-none): a #GCancellable, or %NULL
 * @error: #GError for error reporting, or %NULL to ignore.
 *
 * Attempts to read up to @size bytes from @stream into @buffer, as
 * with g_input_stream_read(). If @stream is not currently readable,
 * this will immediately return %G_IO_ERROR_WOULD_BLOCK, and you can
 * use g_pollable_input_stream_create_source() to create a #GSource
 * that will be triggered when @stream is readable.
 *
 * Note that since this method never blocks, you cannot actually
 * use @cancellable to cancel it. However, it will return an error
 * if @cancellable has already been cancelled when you call, which
 * may happen if you call this method after a source triggers due
 * to having been cancelled.
 *
 * Virtual: read_nonblocking
 * Return value: the number of bytes read, or -1 on error (including
 *   %G_IO_ERROR_WOULD_BLOCK).
 */
gssize
g_pollable_input_stream_read_nonblocking (GPollableInputStream  *stream,
					  void                  *buffer,
					  gsize                  size,
					  GCancellable          *cancellable,
					  GError               **error)
{
  g_return_val_if_fail (G_IS_POLLABLE_INPUT_STREAM (stream), -1);

  if (g_cancellable_set_error_if_cancelled (cancellable, error))
    return -1;

  return G_POLLABLE_INPUT_STREAM_GET_INTERFACE (stream)->
    read_nonblocking (stream, buffer, size, error);
}

/* GPollableSource */

typedef struct {
  GSource       source;

  GObject      *stream;
} GPollableSource;

static gboolean
pollable_source_prepare (GSource *source,
			 gint    *timeout)
{
  *timeout = -1;
  return FALSE;
}

static gboolean
pollable_source_check (GSource *source)
{
  return FALSE;
}

static gboolean
pollable_source_dispatch (GSource     *source,
			  GSourceFunc  callback,
			  gpointer     user_data)
{
  GPollableSourceFunc func = (GPollableSourceFunc)callback;
  GPollableSource *pollable_source = (GPollableSource *)source;

  return (*func) (pollable_source->stream, user_data);
}

static void
pollable_source_finalize (GSource *source)
{
  GPollableSource *pollable_source = (GPollableSource *)source;

  g_object_unref (pollable_source->stream);
}

static gboolean
pollable_source_closure_callback (GObject  *stream,
				  gpointer  data)
{
  GClosure *closure = data;

  GValue param = { 0, };
  GValue result_value = { 0, };
  gboolean result;

  g_value_init (&result_value, G_TYPE_BOOLEAN);

  g_value_init (&param, G_TYPE_OBJECT);
  g_value_set_object (&param, stream);

  g_closure_invoke (closure, &result_value, 1, &param, NULL);

  result = g_value_get_boolean (&result_value);
  g_value_unset (&result_value);
  g_value_unset (&param);

  return result;
}

static GSourceFuncs pollable_source_funcs =
{
  pollable_source_prepare,
  pollable_source_check,
  pollable_source_dispatch,
  pollable_source_finalize,
  (GSourceFunc)pollable_source_closure_callback,
  (GSourceDummyMarshal)g_cclosure_marshal_generic,
};

/**
 * g_pollable_source_new: (skip)
 * @pollable_stream: the stream associated with the new source
 *
 * Utility method for #GPollableInputStream and #GPollableOutputStream
 * implementations. Creates a new #GSource that expects a callback of
 * type #GPollableSourceFunc. The new source does not actually do
 * anything on its own; use g_source_add_child_source() to add other
 * sources to it to cause it to trigger.
 *
 * Return value: (transfer full): the new #GSource.
 *
 * Since: 2.28
 */
GSource *
g_pollable_source_new (GObject *pollable_stream)
{
  GSource *source;
  GPollableSource *pollable_source;

  source = g_source_new (&pollable_source_funcs, sizeof (GPollableSource));
  g_source_set_name (source, "GPollableSource");
  pollable_source = (GPollableSource *)source;
  pollable_source->stream = g_object_ref (pollable_stream);

  return source;
}
