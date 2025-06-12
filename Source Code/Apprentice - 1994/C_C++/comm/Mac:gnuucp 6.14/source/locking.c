/*
 * @(#)locking.c 1.2 87/09/29	Copyright 1987 Free Software Foundation, Inc.
 *
 * Copying and use of this program are controlled by the terms of the
 * GNU Emacs General Public License.
 */

#ifndef lint
char locking_version[] = "@(#)locking.c gnuucp Version Fort Pond Research-6.14";
#endif

#include "includes.h"
#include "uucp.h"

extern int errno;

/*
 * Lock the serial line, if on a multitasking system.
 */
static char	lockname[NAMESIZE];	/* Name of lock file */
static int	lockfile;		/* File descriptor of lock file */

int
ttylock(ttynam)
	char	*ttynam;
{

#ifdef MULTITASK
	sprintf(lockname, "LCK.%s", ttynam);
	strcpy(lockname, munge_filename(lockname));
	/* FIXME, next line is nonportable */
	lockfile = open(lockname, O_RDWR|O_BINARY|O_CREAT|O_EXCL, 0444);
	if (lockfile == -1) {
		DEBUG(9, "Can't create tty lock %s", lockname);
		/* FIXME, check for expired lock (pid not running) here. */
		lockname[0] = '\0';	/* We don't hold a lock */
		return FAIL;
	}

	if (ourpid == 0)
		ourpid = getpid();

	if (sizeof ourpid != write(lockfile, (char *)&ourpid, sizeof ourpid)) {
		DEBUG(0, "Write of pid to %s failed\n", lockname);
		(void) close(lockfile);
		ttyunlock(ttynam);
		return FAIL;
	}
#endif

	return SUCCESS;
}

int
ttyunlock()
{

#ifdef MULTITASK
	if (lockname[0] == '\0')
		return SUCCESS;

	if (lockfile != -1 && 0 != close(lockfile)) {
		DEBUG(0, "Lock file close failed\n", 0);
	}
	lockfile = -1;

	if (0 != remove(lockname)) {
		DEBUG(0, "Removing lock file %s failed", lockname);
		DEBUG(0, " with errno %d\n", errno);
		lockname[0] = '\0';
		return FAIL;
	}
	lockname[0] = '\0';
#endif

	return SUCCESS;
}
