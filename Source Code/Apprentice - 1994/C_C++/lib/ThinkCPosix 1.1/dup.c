/* $Id: $ */

/*
 * This implementation of dup() disables fd;
 * in effect, it assumes that fd is closed
 * immediately after dup() (as is almost always the case).
 * The problem is that if __file[fd]->func() is left unchanged
 * then close(fd) will also close dup(fd).
 */
 
#include "ThinkCPosix.h"

int dup(int fd)
{
	FILE *fp1 = fdopen(fd, "");
	FILE *fp2;
	
	if (fp1 == NULL)
		return -1;
	fp2 = __getfile();
	memcpy((char*)fp2, (char*)fp1, sizeof(FILE));
	memset((char*)fp1, 0, sizeof(FILE));
	return fileno(fp2);
}
