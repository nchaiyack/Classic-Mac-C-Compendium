/* fcntl.h - a mini version of POSIX fcntl.h */

/*
Revision history
----------------
01a,20may95,ejo  written to replace Apple's fcntl.h (nonstandard trouble maker).
*/

#ifndef INC_fcntl_h
#define INC_fcntl_h

/*
 *	For use by lseek():
 */

#ifndef __STDIO__			/* these defns exactly paralled in StdIO.h for fseek() */
#define SEEK_CUR	1
#define SEEK_END	2
#define SEEK_SET	0
#endif

/*
 * Mode values accessible to open()
 */

#ifdef MPW
#define O_RDONLY		 0 		/* Bits 0 and 1 are used internally */
#define O_WRONLY		 1 		/* Values 0..2 are historical */
#define O_RDWR 			 2		/* NOTE: it goes 0, 1, 2, *!* 8, 16, 32, ... */
#define O_APPEND	(1<< 3)		/* append (writes guaranteed at the end) */
#define O_CREAT		(1<< 8)		/* Open with file create */
#define O_TRUNC		(1<< 9)		/* Open with truncation */
#define O_EXCL 		(1<<10) 	/* w/ O_CREAT:  Exclusive "create-only" */
#define O_ACCMODE 	(O_RDONLY | O_WRONLY | O_RDWR)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __STDC__

/* non-POSIX creat, we ignore second argument */
int creat (const char *, ...);

/* these are POSIX */
int fcntl (int, int, ...);
int open (const char *, int, ...);

#else

int creat ();
int fcntl ();
int open ();

#endif

#ifdef __cplusplus
}
#endif

/*
 * fcntl() commands
 */
#define F_DUPFD 0	   /* Duplicate files (file descriptor) */

#endif	/*	INC_fcntl_h	*/
