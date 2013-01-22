#ifndef _MBDOTDESKTOP_H_
#define _MBDOTDESKTOP_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <locale.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>

#include "libmb/mbconfig.h"
#include "libmb/mbutil.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @defgroup freedesktop Freedesktop Standards utilitys
 * @brief Micro implementations of Freedesktop.org standards for .desktop files, icon themeing and vfolders. 
 *
 * @{
 */

/**
 * @typedef MBDotDesktop
 *
 * Opaque type used for representing a parsed .desktop file
 */ 
typedef struct MBDotDesktop MBDotDesktop;

typedef struct _mbdotdesktopfolderentry
{
  unsigned char *name;
  unsigned char *icon;
  unsigned char *match;
  
  struct _mbdotdesktopfolderentry *parent_entry;
  struct _mbdotdesktopfolderentry *next_entry;

} MBDotDesktopFolderEntry;


typedef struct _mbdotdesktopfolders
{
  struct _mbdotdesktopfolderentry *entries;
  int n_entries;

} MBDotDesktopFolders;

/**
 * Parses a .desktop file and creates a localised #MBDotDesktop instance.
 *
 * @param filename full path to .desktop file
 * @returns #MBDotDesktop instance, NULL on fail
 */
MBDotDesktop *
mb_dotdesktop_new_from_file(const char *filename);

/**
 * Gets the localised value for a specified key in #MBDotDesktop instance
 *
 * @param dd #MBDotDesktop instance
 * @param key requested key.
 * @returns values, NULL on fail
 */
unsigned char *
mb_dotdesktop_get(MBDotDesktop *dd, char *key);

/**
  * Gets the filename from #MBDotDesktop instance
  *
  * @param dd #MBDotDesktop instance
  * @returns filename of #MBDotDesktop, NULL on fail
  */
char *
mb_dotdesktop_get_filename(MBDotDesktop *dd);

/**
 * Get the Exec key from a #MBDotDesktop instance, with the %-escapes expanded.
 * Unlike mb_dotdesktop_get(), this string needs to be free()d.
 *
 * @param dd #MBDotDesktop instance
 * @returns file name, NULL on fail
 */
char *
mb_dotdesktop_get_exec (MBDotDesktop *dd);

/**
 * Free's a #MBDotDesktop instance
 *
 * @param dd #MBDotDesktop instance
 */
void
mb_dotdesktop_free(MBDotDesktop *dd);

/**
 * Gets the full path for an specified icon.  The function allocates memory
 * for the returned data, this should be freed by the caller.
 *
 * @param theme_name name of theme to get the icon for
 * @param size_wanted icon dimention wanted in pixels 
 * @param icon_name   the filename
 * @returns the full path to the found icon, or NULL on fail. 
 */
char*
mb_dot_desktop_icon_get_full_path (char* theme_name, 
				   int   size_wanted, 
				   char* icon_name);


/**
 * Parses a 'vfolder style' directory of .directory entrys used for
 * building simple hireachies of .desktop files. 
 * 
 * Expect a directory containing a root.order file, which lists 
 * a .directory file per line. Each of these entry in then parsed 
 * in the specified order as a #MBDotDesktopFolderEntry instance. 
 *
 * @param vfolder_path path to .directory files
 * @returns A dotdesktopfolders object instance
 */
MBDotDesktopFolders *mb_dot_desktop_folders_new(const char *vfolder_path);

/**
 * Frees a dotdesktopfolders instance
 *
 * @param folders dotdesktopfolders instance
 */
void mb_dot_desktop_folders_free(MBDotDesktopFolders* folders);

/**
 * @def mb_dot_desktop_folders_get_cnt
 * Gets a count of .directory entrys ( folders ) in an #MBDotDesktopFolder .
 */
#define mb_dot_desktop_folders_get_cnt(f)        (f)->n_entries

/**
 * @def mb_dot_desktop_folders_enumerate
 * Enumerates the #MBDotDesktopFolderEntry entrys in a #MBDotDesktopFolders
 * struct. 
 */
#define mb_dot_desktop_folders_enumerate(ddfolders, ddentry) \
     for ( (ddentry) = (ddfolders)->entries;                 \
	   (ddentry) != NULL;                                \
	   (ddentry) = (ddentry)->next_entry )               \

/**
 * @def mb_dot_desktop_folder_entry_get_name
 * Gets the name of a #MBDotDesktopFolderEntry
 */
#define mb_dot_desktop_folder_entry_get_name(f)  (f)->name 

/**
 * @def mb_dot_desktop_folder_entry_get_icon
 * Gets the icon filename ( not path ) of a #MBDotDesktopFolderEntry
 */
#define mb_dot_desktop_folder_entry_get_icon(f)  (f)->icon

/**
 * @def mb_dot_desktop_folder_entry_get_match
 * Gets the categorie match string of a #MBDotDesktopFolderEntry
 */
#define mb_dot_desktop_folder_entry_get_match(f) (f)->match

#ifdef __cplusplus
}
#endif


/** @} */


#endif
