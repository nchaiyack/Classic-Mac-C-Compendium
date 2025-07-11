/* @(#)ndir.h	1.4	4/16/85 */
#define _H_ndir

#ifndef DEV_BSIZE
#define	DEV_BSIZE	512
#endif
#define DIRBLKSIZ	DEV_BSIZE
#define	MAXNAMLEN	255

struct	direct {
	long	d_ino;			/* inode number of entry */
	short	d_reclen;		/* length of this record */ 
	short	d_namlen;		/* length of string in d_name */
	char	d_name[MAXNAMLEN + 1];	/* name must be no longer than this */
};

/*
 * The DIRSIZ macro gives the minimum record length which will hold
 * the directory entry.  This requires the amount of space in struct direct
 * without the d_name field, plus enough space for the name with a terminating
 * null byte (dp->d_namlen+1), rounded up to a 4 byte boundary.
 */

#ifdef DIRSIZ
#undef DIRSIZ
#endif /* DIRSIZ */
#define DIRSIZ(dp) \
    ((sizeof (struct direct) - (MAXNAMLEN+1)) + (((dp)->d_namlen+1 + 3) &~ 3))

/*
 * Definitions for library routines operating on directories.
 */
/*
typedef struct _dirdesc {
	int	dd_fd;
	long	dd_loc;
	long	dd_size;
	char	dd_buf[DIRBLKSIZ];
} DIR;
*/
typedef struct _dirdesc {
	int 	ioVRefNum;
	long	ioDrDirID;
	int		ioFDirIndex;
	int		currdir;
} DIR;
#ifndef NULL
#define NULL 0L
#endif
extern	DIR *opendir();
extern	struct direct *readdir();
extern	void closedir();
extern char * getcwd();

