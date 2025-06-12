/* $Id: $ */

#include "ThinkCPosix.h"

/* Rmdir for the Macintosh.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987).
   Pathnames must be Macintosh paths, with colons as separators. */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ThinkCPosix.h"

int
rmdir(path)
	char *path;
{
	IOParam pb;
	char name[FILENAME_MAX];
	
	strncpy(name, path, sizeof(name) - 1);
	c2pstr(name);
	pb.ioNamePtr= (unsigned char*)name;
	pb.ioVRefNum= 0;
	if (PBDelete((ParmBlkPtr)&pb, FALSE) != noErr) {
		errno= EACCES;
		return -1;
	}
	return 0;
}
