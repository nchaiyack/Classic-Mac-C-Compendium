/* $Id: $ */

#include "ThinkCPosix.h"

int SetType (char *filename, FInfo FlFndrInfo)
{
	CInfoPBRec cipbr;
	HFileInfo *fpb = (HFileInfo*)&cipbr;
	DirInfo *dpb = (DirInfo*)&cipbr;
	unsigned char pname[256];
	short err;

	strcpy((char*)pname, filename);
	c2pstr(pname);

	dpb->ioDrDirID = 0L;
	fpb->ioNamePtr = pname;
	fpb->ioVRefNum = 0;
	fpb->ioFDirIndex = 0;
	fpb->ioFVersNum = 0;
	err = PBGetCatInfo(&cipbr, FALSE);
	if (err != noErr) {
		errno = ENOENT;
		return -1;
	}
	dpb->ioDrDirID = 0L;
	fpb->ioFlFndrInfo = FlFndrInfo;
	err = PBSetCatInfo(&cipbr, FALSE);
	if (err != noErr) {
		errno = EACCES;
		return -1;
	}
	return 0;
}

