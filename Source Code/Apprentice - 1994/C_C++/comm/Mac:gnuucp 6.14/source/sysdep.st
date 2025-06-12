/*
 * @(#)sysdep.st 1.3 87/09/23	Copyright 1987 Free Software Foundation, Inc.
 *
 * Copying and use of this program are controlled by the terms of the
 * GNU Emacs General Public License.
 */

#ifndef lint
char sysdep_version[] = "@(#)sysdep.st gnuucp Version hoptoad-1.3";
#endif

#include "includes.h"
#include "uucp.h"
#include "sysdep.h"

#define AUX		1	/* rs232 port */
#define CON		2	/* console */
#define CTRL(X)	(X & 037)

/* Our variables */
jmp_buf alarming;			/* For read timeouts */
void sigint();				/* Forward declaration */


/*
 * Open the serial line for an incoming call.
 * Argument of NULL or empty string means stdin.
 */
int
openline(ttynam, baud)
	char	*ttynam;
	int	baud;
{
	/* FIXME, honor specified baudrate */
	/* set 1200 baud, no flow ctrl. */
	Rsconf(7, 0, -1, -1, -1, -1);
	/* FIXME, these numbers (at least the 7!) should be replaced
	 * with symbolic constants, preferably from a system header
	 * file */

	/* setup user abort */
	if (setjmp(abortenv))
		exit(0);
}

/*
 * Basement level I/O routines
 *
 * xwrite() writes a character string to the serial port
 * xgetc() returns a character from the serial port, or an EOF for timeout.
 * sigint() restores the state of the serial port on exit.
 */

void
sigint()
{
	/* Restore terminal settings on dialout line */

	/* No need to do anything here? */
	exit(0);
}


/*
 * xwrite(dummy, buf, n) - write "n" bytes from buffer "buf" to rs232 port.
 */
xwrite(dummy, buf, n)
	int dummy, n;
	char *buf;
{
	register char *c = buf;

	while (n) {
		/* wait for rs232 to be ready */
		while (Bcostat(AUX) == 0)
			;

		/* write next character in buffer */
		Bconout(AUX, c++);
		n--;
	}
}

/*
 * Serial port reading routines
 */

/*
 * Atari ST routines for reading the comm port.
 *
 * The following routines come by way of J. R. Bammi
 */

long prtime = 0L;			/* present time			*/
long alrmtime = 0L;			/* alarm time			*/
long *hz200 = (long *)0x0004ba;		/* address of system 200 hz clk	*/
jmp_buf abortenv;			/* used to catch user aborts	*/

/*
 * read200hz() - read the system 200 hz clock.
 */
void read200hz()
{
	prtime = *hz200;
}

/*
 * alarm(n) - set the alarm time to n seconds.
 */
void alarm(n)
	unsigned int n;
{
	/* if n != 0, then set the alarm time */
	if (n) {
		Supexec(read200hz);
		alrmtime = prtime + (long)(200 * n);

	/* n == 0, reset alarm time */
	} else
		alrmtime = 0L;
}

/*
 * xgetc() - get a character from the rs232 port, catch timeouts & aborts.
 */
xgetc()
{
	if (setjmp(alarming))
		return(EOF);
	alarm(BYTE_TIMEOUT);
	while (1) {
		/* catch ^C (user abort) */
		if (Bconstat(CON))
			if ((int)Bconin(CON) == CTRL('C'))
				longjmp(abortenv, 1);

		/* check for a char at rs232 port */
		if (Bconstat(AUX)) {
			alarm(0);
			return((int)Bconin(AUX));

		/* no char, check for timeout */
		} else if (alrmtime) {
			Supexec(read200hz);
			if (prtime >= alrmtime)
				longjmp(alarming, 1);
		}
	}
}

/* CP/M and MSDOS and ST need these routines.  Probably should use
 * the new names, but for now...
 */
bzero(s, cnt)
register char	*s;
register int	cnt;
{
	register int	i;
	for (i = 0; i < cnt; i++) {
		*s++ = '\0';
	}
}

bcopy(from, to, cnt)
register char	*from;
register char	*to;
register int	cnt;
{
	register int	i;
	for (i = 0; i < cnt; i++) {
		*to++ = *from++;
	}
}

/*
 * Create a temporary file name for receiving a file into.
 * "name" is the name we will actually eventually want to use for the file.
 * We currently ignore it, but some OS's that can't move files around
 * easily might want to e.g. put the temp file into the same directory
 * that this file is going into.
 *
 * FIXME:
 * This interface should be able to return a "possible" filename, and
 * be re-called if the name is already in use, to get another.
 * This avoids checking here whether the name is good -- saving system calls.
 */
char *
temp_filename(name)
	register char *name;
{
	static char tname[NAMESIZE];

	FIXME;
	DEBUG(6,"Using temp file %s\n", tname);
	return tname;
}


/*
 * Transform a filename from a uucp packet (in Unix format) into a local
 * filename that will work in the local file system.
 */
char *
munge_filename(name)
	register char *name;
{
	register char *p;

#ifdef DEBUG
	printf("Munge_filename  input: %s\n", name);
#endif
	FIXME;
#ifdef DEBUG
	printf("Munge_filename output: %s\n", p);
#endif
	return p;
}
