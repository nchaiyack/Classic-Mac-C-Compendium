/* $Id: $ */

#include "ThinkCPosix.h"

char *getcwd(char *buf, size_t buflen)
{
	CInfoPBRec cipbr;
	HFileInfo *fpb = (HFileInfo*)&cipbr;
	DirInfo *dpb = (DirInfo*)&cipbr;
	char dirname[256];
	short err;
	int depth = 0;
	int dirlen, pathlen = 0;
	
	if (buf == NULL) {
		buf = (char*)malloc(buflen);
		if (buf == NULL) {
			errno = ENOSPC;
			return NULL;
		}
	}

	buf[0] = 0;
	dpb->ioDrParID = 0;
	while (1) {
		dirname[0] = 0;
		fpb->ioDirID = dpb->ioDrParID;
		fpb->ioNamePtr = (unsigned char*)dirname;
		fpb->ioVRefNum = 0;
		fpb->ioFDirIndex = -1;
		fpb->ioFVersNum = 0;
		err = PBGetCatInfo(&cipbr, FALSE);
		if (err != noErr)
			break;
		p2cstr(dirname);
		if (depth > 1)
			strcat(dirname, ":");
		dirlen = strlen(dirname);
		if (dirlen + pathlen + 1 >= buflen) {
			errno = ERANGE;
			return NULL;
		}
		memmove(buf + dirlen, buf, pathlen + 1);
		memcpy(buf, dirname, dirlen);
		pathlen += dirlen;
	}
	if (err == fnfErr)
		return buf;
	else {
		errno = ENODEV;
		return NULL;
	}
}

