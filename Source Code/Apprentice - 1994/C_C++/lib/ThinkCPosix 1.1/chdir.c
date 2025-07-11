/* $Id: $ */

#include "ThinkCPosix.h"

int
chdir(path)
	char *path;
{
	WDPBRec pb;
	char name[FILENAME_MAX];
	
	strncpy(name, path, sizeof(name));
	name[FILENAME_MAX-1]= '\0';
	pb.ioNamePtr= (StringPtr) c2pstr(name);
	pb.ioVRefNum= 0;
	pb.ioWDDirID= 0;
	if (PBHSetVol(&pb, FALSE) != noErr) {
		errno= ENOENT;
		return -1;
	}
	return 0;
}

