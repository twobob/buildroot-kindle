#include <glib.h>

static void
test_environment (void)
{
  GHashTable *table;
  gchar **list;
  gint i;

  table = g_hash_table_new_full (g_str_hash, g_str_equal,
                                 g_free, g_free);

  list = g_get_environ ();
  for (i = 0; list[i]; i++)
    {
      gchar **parts;

      parts = g_strsplit (list[i], "=", 2);
      g_assert (g_hash_table_lookup (table, parts[0]) == NULL);
      g_hash_table_insert (table, parts[0], parts[1]);
      g_free (parts);
    }
  g_strfreev (list);

  g_assert_cmpint (g_hash_table_size (table), >, 0);

  list = g_listenv ();
  for (i = 0; list[i]; i++)
    {
      const gchar *expected;
      const gchar *value;

      expected = g_hash_table_lookup (table, list[i]);
      value = g_getenv (list[i]);
      g_assert_cmpstr (value, ==, expected);
      g_hash_table_remove (table, list[i]);
    }
  g_assert_cmpint (g_hash_table_size (table), ==, 0);
  g_hash_table_unref (table);
  g_strfreev (list);
}

int
main (int argc, char **argv)
{
  g_test_init (&argc, &argv, NULL);

  g_test_add_func ("/glib/environment", test_environment);

  return g_test_run ();
}
