/*
 * @(#)includes.h 1.7 87/09/29	Copyright 1987 Free Software Foundation, Inc.
 *
 * Copying and use of this program are controlled by the terms of the
 * GNU Emacs General Public License.
 *
 * Include files for various supported systems:
 * Note that NAMESIZE should be the max length of a file name, including
 * all its directories, drive specifiers, extensions, and the like.
 * E.g. on a Unix with 14-char file names, NAMESIZE is several hundred
 * characters, since the 14-char names can be nested.
 */

#define MAC
#undef HAS_GETTY

#ifdef BSD
/* Unix Berserkeley systems */
#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <strings.h>

extern char *strtok();
#define	strchr	index

#define STATUS union wait
#define postmaster "postmaster"
#define EXIT_OK 0
#define EXIT_ERR 1

#define	UNIX
#define EXEDIR  "/usr/lib/uucp" /* uuxqt executables live here */
#define NULL_DEVICE "/dev/null"
#define	NAMESIZE	MAXPATHLEN
#endif

#ifdef SYSV
/* Unix System V */
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/timeb.h>
#include <sys/stat.h>
#include <sys/dirent.h>

#define	UNIX
#endif

#ifdef UNIX
/* Stuff common to all Unix systems */
#define	remove	unlink	/* Unix-ism for removing a file */
#define	BUFFEREDIO
#define	MULTITASK
#define	STDIN		0
#define	O_BINARY	0	/* No distinction between text and binary */
#endif

#ifdef CPM
/* CP/M-80 */
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>

#define	NAMESIZE	50		/* No directories... */
#endif

#ifdef MSDOS
/* Microsoft DOS */
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>
#include <dos.h>
#include <conio.h>
#include <stdlib.h>
#include <process.h>
#include <string.h>
#include <direct.h>
#include <memory.h>

/* Turn on support for the interrupt driven comm port routines */
#define	COMPORT

#ifdef COMPORT
#define	BUFFEREDIO
#include "comport.h"
int handler();
#endif

/* FIXME, these should all be uppercase. */
#define	fnamesize	sizeof("FILENAME")    /* 8 chars */
#define	NAMESIZE	128		/* full path size */
#define	ufnamesize      5               /* uux id size */
#endif

#ifdef ST
/* Atari ST */
#include <stdio.h>
#include <ctype.h>
#include <osbind.h>
#include <signal.h>
#include <setjmp.h>

#define O_RDONLY	0	/* for read only open() */
#define NAMESIZE	13	/* filename size */

#endif

#ifdef VMS

#include <ctype.h>
#include <dcdef.h>
#include <descrip.h>
#include <dvidef.h>
#include <errno.h>
#include <file.h>
#include <file>
#include <iodef.h>
#include <math.h>
#include <rmsdef.h>
#include <setjmp.h>
#include <signal.h>
#include <ssdef.h>
#include <stat.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <tt2def.h>
#include <ttdef.h>
#include <string.h>

#define NAMESIZE 255
#define CONTROL         "user2:[uucp.gnuucp]gnuucp.ctl"
#define	LOGCLOSE	/* Logfile must be closed; VMS locks it when open */
#define EXEDIR  "user2:[uucp.gnuucp]"  /* uuxqt executables live here */
#define NULL_DEVICE "NL:"
#define fork vfork
#define STATUS int
#define postmaster "system"
#define EXIT_OK 1
#define EXIT_ERR 0

#define	time_t	unsigned
#define index strchr
#define rindex strrchr
#define remove delete	/* Remove a file */
#define qsort pqsort	/* Our own version */

#endif

#ifdef MAC
/* Macintosh */
#include <Files.h>
#include <Serial.h>
/* #include <HFS.h> */
#include <Menus.h>
#include <signal.h>
#include <Packages.h>
#include <Types.h>
#include <OsUtils.h>
#include <Events.h>
#include <Windows.h>
#include <stdio.h>
#include <console.h>
#include <ctype.h>
#include <setjmp.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unix.h>
#include <errno.h>
#include <fcntl.h>
/* #include <io.h> */
#define postmaster "postmaster"
#define NULL_DEVICE ":NULL"
#define off_t long
#define time_t long
#define EXIT_OK 1
#define EXIT_ERR 0
struct STATUS 
	{ 
		long w_status;
		};

/* struct stat 
	{
		long st_size;
		long st_mtime;
		};
*/
struct DIR
	{
		long dirinfo;
		};
struct dirent
	{
		char *d_name;
		};
		
#define	NAMESIZE	512		/* No directories... */
#include "gnuucp_proto.h"
#include "rmail_proto.h"
#include "sysdep.h"
#define	BUFFEREDIO
#endif


#ifdef MAC_MPW
/* Macintosh Programmers Workshop. */
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <setjmp.h>
#include <uumac_mpw.h>

#define NAMESIZE 512 /* Hat. */

#define	abort()	exit(1);
#endif

#ifdef AMIGA
#include <stdio.h>
#include <ctype.h>
#include <fcntl.h>
#include <exec/types.h>
#include <exec/exec.h>
#include <devices/serial.h>
#include <devices/keymap.h>
#include <devices/timer.h>
#include <libraries/dos.h>
#include <signal.h>

#define NAMESIZE 50

#define	bzero(area, len)	memset(area, '\0', len)
#define	bcopy(from, to, len)	memcpy(to, from, len)
#endif
