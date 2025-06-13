/*
 *  sys/stat.h
 *
 *	Copyright (c) 1995, Christopher E. Hyde.  All rights reserved.
 *
 *	Updated: 95-06-30
 */

#ifndef _H_SYS_STAT_
#define	_H_SYS_STAT_

#include <time.h>
#include <sys/types.h>
#include <stdio.h>

struct stat {				// "inode" information returned by stat/fstat
	dev_t	st_dev;				// device of the inode
	ino_t	st_ino;				// inode number
	short	st_mode;			// mode bits
	short	st_nlink;			// number of links to file
	int		st_uid;				// owner's user id
	int		st_gid;				// owner's group id
	dev_t	st_rdev;			// for special files [ignored]
	off_t	st_size;			// file size in characters
	time_t	st_atime;			// time last accessed
	time_t	st_mtime;			// time last modified
	time_t	st_ctime;			// time originally created
};

#define	S_IFMT		0xFFFF		// type of file:
#define	S_IFDIR		0x0000		//		directory
#define	S_IFCHR		0x0001		//		character special	[n/a]
#define	S_IFBLK		0x0002		//		block special		[n/a]
#define	S_IFREG		0x0003		//		regular

#define S_ISDIR(m)	(((m) & (S_IFMT)) == (S_IFDIR))
#define S_ISREG(m)	(((m) & (S_IFMT)) == (S_IFREG))

enum { kUserID = 24, kGroupID = 31 };

#define getuid()		((int) kUserID)
#define getgid()		((int) kGroupID)

int		stat	(const char* filename, struct stat* sr);
int		fstat	(int fd, struct stat* sr);

#endif
