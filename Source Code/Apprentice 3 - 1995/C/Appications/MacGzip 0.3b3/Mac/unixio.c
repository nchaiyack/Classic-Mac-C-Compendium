/* unixio.c - a mini version of Symantec unixio.c */

/* Copyright (c) 1993-4 Symantec Corporation.  All rights reserved. */

/*
modification history
--------------------
01a,28may95,ejo  derived from Symantec unixio.c to remove shortcomings.
*/

#include <stdio.h>
#include <errno.h>
#include <ansi_private.h>
#include <fcntl.h>

int _fmode = O_BINARY;

static FILE *file(int);

int
open(const char *filename, int mode, ...)
{
	int omode = fsRdPerm, oflag = 0;
	
		/*  set read/write permission  */
		
	if (mode & O_WRONLY)
		omode = fsWrPerm;
	if (mode & O_RDWR)
		omode = fsRdWrPerm;
		
		/*  set open flags  */
		
	if (mode & O_CREAT)
		oflag |= F_CREAT;
	if (mode & O_TRUNC)
		oflag |= F_TRUNC;
	if (mode & O_EXCL)
		oflag |= F_EXCL;
	if (mode & O_APPEND)
		oflag |= F_APPEND;
		
		/*  set text/binary mode  */
	
	if (!(mode & (O_TEXT|O_BINARY)))
		mode = _fmode;
	if (!(mode & O_TEXT))
		oflag |= F_BINARY;
		
		/*  open file  */
		
	return(fileno(__open(filename, omode, oflag, __getfile())));
}

int
fileno(FILE *fp)
{
	if (fp)
		return(fp - __file);
	return(-1);
}

int
read(int fd, char *buf, unsigned n)
{
	FILE *fp;

	if (fp = file(fd)) {
		if (n == 0)
			return(0);
		if (fp->dirty)
			fflush(fp);
		n = fread(buf, 1, n, fp);
		if (n || feof(fp))
			return(n);
	}
	return(-1);
}


int
write(int fd, char *buf, unsigned n)
{
	FILE *fp;

	if (fp = file(fd)) {
		if (n == 0)
			return(0);
		if (!fp->dirty)
			fflush(fp);
		n = fwrite(buf, 1, n, fp);
		if (n)
			return(n);
	}
	return(-1);
}


int
close(int fd)
{
	FILE *fp;
	
	if (fp = file(fd))
		return(fclose(fp));
	return(-1);
}


long
lseek(int fd, long ofs, int whence)
{
	FILE *fp;
	
	if (fp = file(fd)) {
		if (fseek(fp, ofs, whence) == 0)
			return(ftell(fp));
	}
	return(-1);
}

int
unlink(char *filename)
{
	return(remove(filename));
}

/*
 *  file - translate "file descriptor" to "file pointer"
 *
 */

static FILE *
file(int fd)
{
	register FILE *fp;
	
	if ((unsigned) fd < FOPEN_MAX) {
		fp = &__file[fd];
		if (fp->refnum || fp->std)
			return(fp);
	}
	errno = EBADF;
	return(NULL);
}

