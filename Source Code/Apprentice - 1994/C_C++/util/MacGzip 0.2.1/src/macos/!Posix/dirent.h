/* $Id: $ */

#pragma once

#include "sys/dir.h"

/* from Mips posix/sysv/sys/dirent.h */

struct dirent				/* data from readdir() */
	{
	long		d_ino;		/* inode number of entry */
	off_t		d_off;		/* offset of disk directory entry */
	unsigned short	d_reclen;	/* length of this record */
	char		d_name[1];	/* name of file */
	};

/*
#define	DIRENTBASESIZE \
	(((struct dirent *) 0)->d_name - (char *) 0)
#define	DIRENTSIZE(namelen) \
	((DIRENTBASESIZE + (namelen) + NBPW) & ~(NBPW - 1))
*/

/* from Mips posix/dirent.h */

/*
#undef rewinddir
*/

extern DIR		*opendir();
extern struct dirent	*readdir();
extern void		rewinddir();
extern int		closedir();

