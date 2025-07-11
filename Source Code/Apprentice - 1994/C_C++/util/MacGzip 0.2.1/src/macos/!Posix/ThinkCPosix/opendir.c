/* $Id: $ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ThinkCPosix.h"

DIR *
opendir(path)
	char *path;
{
	CInfoPBRec cipbr;
	HFileInfo *fpb = (HFileInfo*)&cipbr;
	DirInfo *dpb = (DirInfo*)&cipbr;
	DIR *dp;
	char name[FILENAME_MAX];
	short err;
	
	strncpy(name, path, sizeof(name)-1);
	c2pstr(name);
	fpb->ioNamePtr= (unsigned char*)name;
	fpb->ioVRefNum= 0;
	fpb->ioFDirIndex= 0;
	fpb->ioFVersNum= 0;
	dpb->ioDrDirID= 0;
	err= PBGetCatInfo(&cipbr, FALSE);
	if (err != noErr) {
		errno = ENOENT;
		return NULL;
	}
	dp = (DIR*)calloc(1, sizeof(DIR));
	if (dp != NULL) {
		dp->dirid = fpb->ioDirID;
		dp->nextfile = 1;
	}
	return dp;
}

/*
 * Close a directory.
 */

int
closedir(dp)
	DIR *dp;
{
	free(dp);
	return 0;
}

/*
 * Read the next directory entry.
 */

struct dirent *
readdir(dp)
	DIR *dp;
{
	CInfoPBRec cipbr;
	HFileInfo *fpb = (HFileInfo*)&cipbr;
	DirInfo *dpb = (DirInfo*)&cipbr;
	short err;
	static struct dirent dir;

	fpb->ioDirID= dp->dirid;
	fpb->ioNamePtr= (unsigned char*)dir.d_name;
	fpb->ioVRefNum= 0;
	fpb->ioFDirIndex= dp->nextfile++;
	fpb->ioFVersNum= 0;
	err= PBGetCatInfo(&cipbr, FALSE);
	if (err != noErr) {
		errno = EIO;
		return NULL;
	}
	p2cstr(dir.d_name);
	dir.d_ino = fpb->ioDirID;
	return &dir;
}

void rewinddir(dp)
DIR *dp;
{
		dp->nextfile = 1;
}
