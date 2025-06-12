/*
 * @(#)sysdep.h 1.4 87/09/23	Copyright 1987 Free Software Foundation, Inc.
 *
 * Copying and use of this program are controlled by the terms of the
 * GNU Emacs General Public License.
 *
 * Declarations of gnuucp's system-dependent routines, which
 * are in the file sysdep.c (a link to one of many system dependent
 * implementations).
 */

/*
 * Returns a temp filename, in a static buffer, to use while
 * receiving the file whose name is passed as argument.
 */
char *	temp_filename();

/*
 * Returns a filename, in a static buffer, which should be used
 * on the local system to access the file named in a uucp control
 * file (the argument).  E.g. it might turn D.foobarX1234 into
 * foobar\X1234.D on MSDOS, or to D./D.foobarX1234 on Unix.
 */
char *	munge_filename();

/* Should be related to MAX packet size we expect */
#define SER_BUF_SIZE 4096

/*
 * Basement level I/O routines
 *
 * xwrite() writes a character string to the serial port
 * xgetc() returns a character from the serial port, or an EOF for timeout.
 * sigint() restores the state of the serial port on exit.
 * openline() opens a serial port for an incoming call, waiting for carrier.
 * openout() opens a serial port for an outgoing call.
 */
int xwrite();		/* buffer, count */
extern long xgetc_buf_pos;
extern long xgetc_buf_count;
extern unsigned char xgetc_buf[SER_BUF_SIZE];

int xgetc();		/* No arg */
void sigint();		/* No arg */

extern int inRefNum;
extern int outRefNum;
int openline();		/* ttyname, baud */
int openout();		/* ttyname, baud */

/*
 * Uucp work queue scan.
 *
 * gotsome = work_scan(hostname, type);
 *
 * Hostname is a string, e.g. "hoptoad", or (char *)NULL for all hostnames.
 * Type is a string, indicating the type of file:
 *	"C"	control file, holding file transfer commands for host
 *	"D"	data file, holding data to go to host
 *	"X"	execute file, holding shell commands to execute here (host??)
 *	"LCK"	lock files, holding execlusive access to dialers and systems
 *	"STST"	system status files, holding status info about systems
 * Result is 1 if there is work, 0 if none.  If result is 1, and work_scan
 * is called a second time without an intervening work_done, it won't
 * rescan the directory but simply returns a 1.
 *
 * workfile = work_next();
 *
 * Result is char * to static filename; or NULL if no more.
 * We only read the directory once.  If callers want more to rescan it
 * in case more work is here, they should call work_scan again.
 *
 * void work_done();
 * Clean up a work scan.  No need to call this if work_next returned NULL;
 * it has cleaned up.
 */
extern int work_scan();
extern char *work_next();
extern void work_done();

/*
 * System-dependent default control file name.
 */
extern char sysdep_control[];

/*
 * Routine to return a string that gives the current date and time, and
 * identifies the current process, if on a multiprocess system.
 */
extern char *time_and_pid();

/******************************************************************************
 OSChecks.h

		Declarations for Operating System Checks Module
		
		Copyright © 1989 Symantec Corporation. All rights reserved.
		
 ******************************************************************************/

#define _H_OSChecks

		/** Exported Functions **/

 	Boolean		TrapAvailable(short trapNum, short tType);
 	Boolean		WNEIsImplemented(void);
 	Boolean		TempMemCallsAvailable(void);
 	Boolean		ColorQDIsPresent(void);