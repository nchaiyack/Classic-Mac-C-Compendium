/* catinfo.h - get catalog info from full or partial path */

/*
modification history
--------------------
01a,06nov94,ejo  written.
*/

#ifndef INC_catinfo_h
#define INC_catinfo_h

#include <Files.h>

#define MAX_DIR_LEN	31		/* max length for directory and file names */
#define MAX_VOL_LEN	27		/* max length for volumes */
#define MAX_PAS_LEN	255		/* max length of a pascal string */

/*
 * DESCRIPTION
 * This function accepts a path (full or partial, IM IV-99), and returns
 * information in INFO about the directory or file specified by the path.
 *
 * If a file is specified by the path, then the ID of the directory
 * containing the file is returned in DIRID. If a directory is specified,
 * then the parent directory ID is returned in DIRID. It is OK to pass
 * NULL for DIRID if this information is not needed.
 *
 * NOTE:
 * On exit, the INFO.ioNamePtr field will point to static storage which
 * are overwritten by subsequent calls.
 *
 * RETURNS: 0 on success, otherwise an errno describing the problem.
 */
int GetCatInfo (const char *path, CInfoPBRec *info, long *dirID);

#endif /* INC_catinfo_h */
