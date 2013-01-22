#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <check.h>
#include <stdlib.h>
#include <libmb/mb.h>

/**
 * Check that test.desktop, a perfectly valid file, can be loaded in the C locale
 * and the strings are as expected.
 */
START_TEST (dotdesktop_valid)
{
  MBDotDesktop *dd = NULL;
  char *exec;
  unsetenv("LC_MESSAGES");
  dd = mb_dotdesktop_new_from_file("test1.desktop");
  fail_unless(dd != NULL, "mb_dotdesktop_new_from_file returned NULL");
  fail_unless(strcmp(mb_dotdesktop_get_filename(dd), "test1.desktop") == 0, NULL);
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Encoding"), "UTF-8") == 0, NULL);
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Type"), "Application") == 0, NULL);
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Name"), "Test 1") == 0, NULL);
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Comment"), "Test Entry 1") == 0, NULL);
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Exec"), "test %f") == 0, NULL);

  exec = mb_dotdesktop_get_exec(dd);
  fail_unless(strcmp(exec, "test ") == 0, NULL);
  free (exec);

  mb_dotdesktop_free (dd);
}
END_TEST

/**
 * Check that the French translation in test1.desktop is loaded correctly when
 * the locale is fr_FR.
 */
START_TEST(dotdesktop_l10n_present)
{
  MBDotDesktop *dd;
  setenv("LC_MESSAGES", "fr_FR", 1);
  dd = mb_dotdesktop_new_from_file("test1.desktop");
  fail_unless(dd != NULL, "mb_dotdesktop_new_from_file returned NULL");
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Name"), "La Test Un") == 0, NULL);
  mb_dotdesktop_free (dd);
}
END_TEST

/**
 * Check that the C locale is selected correctly when a locale which is not
 * present in the .desktop file is active.
 */
START_TEST(dotdesktop_l10n_absent)
{
  MBDotDesktop *dd;
  setenv("LC_MESSAGES", "es", 1);
  dd = mb_dotdesktop_new_from_file("test1.desktop");
  fail_unless(dd != NULL, "mb_dotdesktop_new_from_file returned NULL");
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Name"), "Test 1") == 0, NULL);
  mb_dotdesktop_free (dd);
}
END_TEST

/**
 * Check UTF-8 characters are parsed correctly.
 */
START_TEST(dotdesktop_utf8_valid)
{
  MBDotDesktop *dd;
  dd = mb_dotdesktop_new_from_file("test1.desktop");
  fail_unless(dd != NULL, "mb_dotdesktop_new_from_file returned NULL");
  fail_unless(strcmp(mb_dotdesktop_get(dd, "X-Some-Key"), "\330\252\330\253\330\254\330\266\330\265\330\263\330\270\331\205") == 0, NULL);
  mb_dotdesktop_free (dd);
}
END_TEST

/**
 * Check invalid files are parsed in a somewhat sane manner.
 */
START_TEST(dotdesktop_invalid)
{
  MBDotDesktop *dd;
  dd = mb_dotdesktop_new_from_file("test2.desktop");
  fail_unless(dd != NULL, "mb_dotdesktop_new_from_file returned NULL");
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Name"), "Test 1") == 0, NULL);
  fail_unless(strcmp(mb_dotdesktop_get(dd, "Comment"), "A test entry") == 0, NULL);
  fail_unless(mb_dotdesktop_get(dd, "Type") == 0, NULL);
  fail_unless(mb_dotdesktop_get(dd, "TypeApplication") == 0, NULL);
  mb_dotdesktop_free (dd);
}
END_TEST

START_TEST(dotdesktop_menu_parse)
{
  MBDotDesktopFolders *folders = NULL;
  MBDotDesktopFolderEntry *entry;
  folders = mb_dot_desktop_folders_new("menu");
  fail_unless(folders != NULL, NULL);
  fail_unless(mb_dot_desktop_folders_get_cnt(folders) == 4, NULL);
  entry = folders->entries;
  fail_unless(strcmp(entry->name, "Utilities") == 0, NULL);
  entry = entry->next_entry;
  fail_unless(strcmp(entry->name, "Games") == 0, NULL);
  entry = entry->next_entry;
  fail_unless(strcmp(entry->name, "Settings") == 0, NULL);
  entry = entry->next_entry;
  fail_unless(strcmp(entry->name, "Other") == 0, NULL);
  entry = entry->next_entry;
  fail_unless(entry == NULL, NULL);
  mb_dot_desktop_folders_free(folders);
}
END_TEST


Suite *dotdesktop_suite(void)
{
  Suite *s = suite_create("DotDesktop");
  TCase *tc_desktop = tcase_create("DesktopParser");
  suite_add_tcase (s, tc_desktop);
  tcase_add_test(tc_desktop, dotdesktop_valid);
  tcase_add_test(tc_desktop, dotdesktop_l10n_present);
  tcase_add_test(tc_desktop, dotdesktop_l10n_absent);
  tcase_add_test(tc_desktop, dotdesktop_utf8_valid);
  tcase_add_test(tc_desktop, dotdesktop_invalid);

  TCase *tc_menu = tcase_create("MenuParser");
  suite_add_tcase (s, tc_menu);
  tcase_add_test(tc_menu, dotdesktop_menu_parse);

  return s;
}

int main(void)
{
  int nf;
  Suite *s = dotdesktop_suite();
  SRunner *sr = srunner_create(s);
  srunner_run_all(sr, CK_NORMAL);
  nf = srunner_ntests_failed(sr);
  srunner_free(sr);
  return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

