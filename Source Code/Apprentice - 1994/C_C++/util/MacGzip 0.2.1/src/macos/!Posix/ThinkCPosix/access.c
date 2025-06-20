/* $Id: access.c,v 1.1 1992/09/14 14:27:36 TimothyMurphy Exp TimothyMurphy $ */

#include "ThinkCPosix.h"

/*
 * If mode includes check for write permission,
 * and file is locked, return -1 (failure).
 */

#define LOCKBIT 0x1
 
int
access(path, mode)
	char *path;
	int mode;	/* ignored */
{
	CInfoPBRec cipbr;
	HFileInfo *fpb = (HFileInfo*)&cipbr;
	DirInfo *dpb = (DirInfo*)&cipbr;
	short err;
	char name[FILENAME_MAX];
	
	strncpy(name, path, sizeof(name)-1);
	c2pstr(name);
	dpb->ioDrDirID= 0L;
	fpb->ioNamePtr= (unsigned char*)name;
	fpb->ioVRefNum= 0;
	fpb->ioFDirIndex= 0;
	fpb->ioFVersNum= 0;
	err= PBGetCatInfo(&cipbr, FALSE);
	if (err != noErr) {
		errno = ENOENT;
		return -1;
	}
	/* check for write-permission = not locked */
	if ((mode & 02) && (fpb->ioFlAttrib & LOCKBIT)) {
		errno = EACCES;
		return -1;
	}
	return 0;
}

