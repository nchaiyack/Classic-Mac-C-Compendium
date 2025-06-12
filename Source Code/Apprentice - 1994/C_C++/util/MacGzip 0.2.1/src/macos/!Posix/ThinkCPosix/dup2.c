/* $Id: $ */

/*
 * This implementation of dup2() disables oldfd;`
 * in effect, it assumes that oldfd is closed
 * immediately after dup2() (as is almost always the case).
 * The problem is that if __file[oldfd]->func() is left unchanged
 * then close(oldfd) will also close newfd.
 */
 
#include "ThinkCPosix.h"

int dup2(int oldfd, int newfd)
{
	FILE *oldfp = fdopen(oldfd, "");

	if (oldfp == NULL || (unsigned)newfd >= FOPEN_MAX) {
		errno = EBADF;
		return -1;
	}
	close(newfd);
	memcpy(&__file[newfd], (char*)oldfp, sizeof(FILE));
	memset((char*)oldfp, 0, sizeof(FILE));
	return newfd;
}
