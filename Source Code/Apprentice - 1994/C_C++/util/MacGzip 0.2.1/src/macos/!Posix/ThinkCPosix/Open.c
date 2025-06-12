/* $Id: $ */

/*
 * This replacement for open() deals with 2 problems:
 * (1) open() is often given with 3 arguments;
 * (2) if an attempt is made to open a file :dirname:filename
 *     and there is no directory dirnam,
 *     open() returns ENOTDIR, where Unix would return ENOENT.
 */

#include "ThinkCPosix.h"

int Open(char *filename, int mode, ...)
{
	int fd;
	fd = open(filename, mode);
	if (fd < 0 && errno == ENOTDIR)
		errno = ENOENT;
	return fd;
}
