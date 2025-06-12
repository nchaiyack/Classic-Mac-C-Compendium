/* $Id: $ */

/* Include file belonging to stat emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

#pragma once

struct stat {
	unsigned short st_dev;
	unsigned short st_ino;
	unsigned short st_mode;
	unsigned short st_nlink;
    unsigned int   st_uid;
    unsigned int   st_gid;
	unsigned short st_rdev;
	unsigned long st_size;
	unsigned long st_rsize; /* Resource size -- nonstandard */
	unsigned long st_atime;
	unsigned long st_mtime;
	unsigned long st_ctime;
	FInfo st_FlFndrInfo;		/* File type, creator, etc */
};

/* from Borland-C
struct  stat
{
    short st_dev;
    short st_ino;
    short st_mode;
    short st_nlink;
    int   st_uid;
    int   st_gid;
    short st_rdev;
    long  st_size;
    long  st_atime;
    long  st_mtime;
    long  st_ctime;
};
*/

int fstat (int __handle, struct stat *__statbuf);
int stat  (char *__path, struct stat *__statbuf);


#define S_IFMT	0170000L
#define S_IFDIR	0040000L
#define S_IFREG 0100000L
#define S_IREAD    0400
#define S_IWRITE   0200
#define S_IEXEC    0100

#define S_IFIFO 010000  /* FIFO special */
#define S_IFCHR 020000  /* character special */
#define S_IFBLK 030000  /* block special */
