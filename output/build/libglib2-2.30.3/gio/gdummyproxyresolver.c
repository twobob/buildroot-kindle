/* GIO - GLib Input, Output and Streaming Library
 *
 * Copyright (C) 2010 Collabora, Ltd.
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
 *
 * Author: Nicolas Dufresne <nicolas.dufresne@collabora.co.uk>
 */

#include "config.h"

#include "gdummyproxyresolver.h"

#include <glib.h>

#include "gasyncresult.h"
#include "gcancellable.h"
#include "gproxyresolver.h"
#include "gsimpleasyncresult.h"

#include "giomodule.h"
#include "giomodule-priv.h"

struct _GDummyProxyResolver {
  GObject parent_instance;
};

static void g_dummy_proxy_resolver_iface_init (GProxyResolverInterface *iface);

#define g_dummy_proxy_resolver_get_type _g_dummy_proxy_resolver_get_type
G_DEFINE_TYPE_WITH_CODE (GDummyProxyResolver, g_dummy_proxy_resolver, G_TYPE_OBJECT,
			 G_IMPLEMENT_INTERFACE (G_TYPE_PROXY_RESOLVER,
						g_dummy_proxy_resolver_iface_init)
			 _g_io_modules_ensure_extension_points_registered ();
			 g_io_extension_point_implement (G_PROXY_RESOLVER_EXTENSION_POINT_NAME,
							 g_define_type_id,
							 "dummy",
							 -100))

static void
g_dummy_proxy_resolver_finalize (GObject *object)
{
  /* must chain up */
  G_OBJECT_CLASS (g_dummy_proxy_resolver_parent_class)->finalize (object);
}

static void
g_dummy_proxy_resolver_init (GDummyProxyResolver *resolver)
{
}

static gboolean
g_dummy_proxy_resolver_is_supported (GProxyResolver *resolver)
{
  return TRUE;
}

static gchar **
g_dummy_proxy_resolver_lookup (GProxyResolver  *resolver,
			       const gchar     *uri,
			       GCancellable    *cancellable,
			       GError         **error)
{
  gchar **proxies;

  if (g_cancellable_set_error_if_cancelled (cancellable, error))
    return NULL;

  proxies = g_new0 (gchar *, 2);
  proxies[0] = g_strdup ("direct://");

  return proxies;
}

static void
g_dummy_proxy_resolver_lookup_async (GProxyResolver      *resolver,
				     const gchar         *uri,
				     GCancellable        *cancellable,
				     GAsyncReadyCallback  callback,
				     gpointer             user_data)
{
  GError *error = NULL;
  GSimpleAsyncResult *simple;
  gchar **proxies;

  proxies = g_dummy_proxy_resolver_lookup (resolver, uri, cancellable, &error);

  
  simple = g_simple_async_result_new (G_OBJECT (resolver),
				      callback, user_data,
				      g_dummy_proxy_resolver_lookup_async);

  if (proxies == NULL)
    {
      g_simple_async_result_take_error (simple, error);
    }
  else
    {
      g_simple_async_result_set_op_res_gpointer (simple,
						 proxies,
						 NULL);
    }

  g_simple_async_result_complete_in_idle (simple);
  g_object_unref (simple);
}

static gchar **
g_dummy_proxy_resolver_lookup_finish (GProxyResolver     *resolver,
				      GAsyncResult       *result,
				      GError            **error)
{
  if (G_IS_SIMPLE_ASYNC_RESULT (result))
    {
      GSimpleAsyncResult *simple = G_SIMPLE_ASYNC_RESULT (result);

      if (g_simple_async_result_propagate_error (simple, error))
        return NULL;

      return g_simple_async_result_get_op_res_gpointer (simple);
    }

  return NULL;
}

static void
g_dummy_proxy_resolver_class_init (GDummyProxyResolverClass *resolver_class)
{
  GObjectClass *object_class;
  
  object_class = G_OBJECT_CLASS (resolver_class);
  object_class->finalize = g_dummy_proxy_resolver_finalize;
}

static void
g_dummy_proxy_resolver_iface_init (GProxyResolverInterface *iface)
{
  iface->is_supported = g_dummy_proxy_resolver_is_supported;
  iface->lookup = g_dummy_proxy_resolver_lookup;
  iface->lookup_async = g_dummy_proxy_resolver_lookup_async;
  iface->lookup_finish = g_dummy_proxy_resolver_lookup_finish;
}
