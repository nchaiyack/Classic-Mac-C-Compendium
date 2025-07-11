/* $Id: $ */

#include "ThinkCPosix.h"

/* Create a directory. */

int
mkdir(path, mode)
	char *path;
	int mode; /* Ignored */
{
	HFileParam pb;
	char name[FILENAME_MAX];
		
	strncpy(name, path, sizeof(name)-1);
	c2pstr(name);
	pb.ioNamePtr= (unsigned char*)name;
	pb.ioVRefNum= 0;
	pb.ioDirID= 0;
	if (PBDirCreate((HParmBlkPtr)&pb, FALSE) != noErr) {
		errno= EACCES;
		return -1;
	}
	return 0;
}
