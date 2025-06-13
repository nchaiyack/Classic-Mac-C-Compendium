/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/*
	fopenMAC.c
*/

#include <stdio.h>

char *GetPathName(char *s,char *name,short vRefNum,long dirID)
{
	CInfoPBRec	block;
	Str255 directoryName;
	OSErr err;
	char pname[64];
	
	*s = 0;
	block.dirInfo.ioNamePtr = directoryName;
	block.dirInfo.ioDrParID = dirID;
	
	do {
		block.dirInfo.ioVRefNum = vRefNum;
		block.dirInfo.ioFDirIndex = -1;
		block.dirInfo.ioDrDirID = block.dirInfo.ioDrParID;
		
		err = PBGetCatInfo(&block,false);
		if (err) break;
		
		ConcatPStrings(directoryName,"\p:");
		ConcatPStrings(directoryName,s);
		CopyPString(directoryName,s);
	} while (block.dirInfo.ioDrDirID != fsRtDirID);
	
	if (name) {
		strcpy(pname,name);
		c2pstr(pname);
		ConcatPStrings(s,pname);
	}
	return s;
}

/** PathNameFromWD ************************************************************/
/*
/*	Given an HFS working directory, this routine returns the full pathname
/*	that corresponds to it. It does this by calling PBGetWDInfo to get the
/*	VRefNum and DirID of the real directory. It then calls PathNameFromDirID,
/*	and returns its result.
/*
/******************************************************************************/

char *GetPathNameFromWD(char *s,char *name,short vRefNum)
{

	WDPBRec	myBlock;

	/*
	/* PBGetWDInfo has a bug under A/UX 1.1.  If vRefNum is a real vRefNum
	/* and not a wdRefNum, then it returns garbage.  Since A/UX has only 1
	/* volume (in the Macintosh sense) and only 1 root directory, this can
	/* occur only when a file has been selected in the root directory (/).
	/* So we look for this and hardcode the DirID and vRefNum. */

	myBlock.ioNamePtr = NULL;
	myBlock.ioVRefNum = vRefNum;
	myBlock.ioWDIndex = 0;
	myBlock.ioWDProcID = 0;

	/* Change the Working Directory number in vRefnum into a real vRefnum */
	/* and DirID. The real vRefnum is returned in ioVRefnum, and the real */
	/* DirID is returned in ioWDDirID. */

	PBGetWDInfo(&myBlock,false);

	return(GetPathName(s,name,myBlock.ioWDDirID,myBlock.ioWDVRefNum));
}

FILE *fopenMAC(char *name,short vRefNum,long dirID,char *mode)
{
	char path[512];
	GetPathName(path,name,vRefNum,dirID);
	p2cstr(path);
	return fopen(path,mode);
}

char *GetDirPathFromWD(char *s,short vRefNum, int ftype)
{

	WDPBRec	myBlock;

	/*
	/* PBGetWDInfo has a bug under A/UX 1.1.  If vRefNum is a real vRefNum
	/* and not a wdRefNum, then it returns garbage.  Since A/UX has only 1
	/* volume (in the Macintosh sense) and only 1 root directory, this can
	/* occur only when a file has been selected in the root directory (/).
	/* So we look for this and hardcode the DirID and vRefNum. */

	myBlock.ioNamePtr = NULL;
	myBlock.ioVRefNum = vRefNum;
	myBlock.ioWDIndex = 0;
	myBlock.ioWDProcID = 0;

	/* Change the Working Directory number in vRefnum into a real vRefnum */
	/* and DirID. The real vRefnum is returned in ioVRefnum, and the real */
	/* DirID is returned in ioWDDirID. */

	PBGetWDInfo(&myBlock,false);

	return(GetPathName(s,NULL,myBlock.ioWDDirID,myBlock.ioWDVRefNum));
}

