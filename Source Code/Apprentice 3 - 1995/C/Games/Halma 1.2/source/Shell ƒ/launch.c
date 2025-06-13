/**********************************************************************\

File:		launch.c

Purpose:	This module handles launching another application.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "launch.h"
#include "file interface.h"

#define haveAUX() 0

/*-----------------------------------------------------------------------------------*/
/* internal stuff for launch.c                                                       */

void DoLaunch(Ptr theApp, short memCode );
void PathNameFromDirID(long dirID, short vRefNum, Str255 fullPathName);
void pstrcat(Str255 dst, Str255 src);
void pstrinsert(Str255 dst, Str255 src);


void LaunchDispatch(void)
{
	FSSpec				temp;

	if (GetSourceFile(&temp, FALSE, TRUE))
	{
		PathNameFromDirID(temp.parID, temp.vRefNum, temp.name);
		DoLaunch((Ptr)temp.name,  0 );			/* launch the program */
	}
}

void DoLaunch(Ptr theApp, short memCode )
{
	struct {				/* temporary structure */
		Ptr	appName;	/* the name of the launchee */
		short	memUse;		/* config. param. see above Notes */
		char	lc[2];			/* extended parameters */
		long	extBlocLen;	/* # bytes in extension(6) */
		short	fFlags;	/* Finder file info flags */
		long	launchFlags;	/* bit 31,30==1 for sublaunch */
	} LaunchRec;

	LaunchRec.appName = theApp;
	LaunchRec.launchFlags = memCode;
	asm {
			lea	LaunchRec, A0
			_Launch
	}
}

void pstrcat(Str255 dst, Str255 src)
{
	/* copy string in */
	BlockMove(src + 1, dst + *dst + 1, *src);
	/* adjust length byte */
	*dst += *src;
}

void pstrinsert(Str255 dst, Str255 src)
{
	/* make room for new string */
	BlockMove(dst + 1, dst + *src + 1, *dst);
	/* copy new string in */
	BlockMove(src + 1, dst + 1, *src);
	/* adjust length byte */
	*dst += *src;
}

void PathNameFromDirID(long dirID, short vRefNum, Str255 fullPathName)
{
	DirInfo	block;
	Str255	directoryName;
	OSErr	err;

	block.ioDrParID = dirID;
	block.ioNamePtr = directoryName;
	do
	{
		block.ioVRefNum = vRefNum;
		block.ioFDirIndex = -1;
		block.ioDrDirID = block.ioDrParID;
		err = PBGetCatInfo((CInfoPBPtr)&block, FALSE);
		if (haveAUX())
		{
			if (directoryName[1] != '/')
				pstrcat(directoryName, (StringPtr)"\p/");
		}
		else
		{
			pstrcat(directoryName, (StringPtr)"\p:");
		}
		pstrinsert(fullPathName, directoryName);
	}
	while (block.ioDrDirID != 2);
}
