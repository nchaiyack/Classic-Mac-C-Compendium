/**********************************************************************\

File:		msg integrity.c

Purpose:	This module implements a quick-and-dirty integrity check;
			compare the resource fork and map length to stored values.
			(Drop the completed application on "Prepare" to store
			these values in the right place.)

Chef -=- convert text to Swedish chef talk
Copyright �1994, Mark Pilgrim

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

#include "msg integrity.h"
#include "msg graphics.h"
#include "msg dialogs.h"
#include "program globals.h"

void DoIntegrityCheck(void)
{
	int				thisFile;
	long			count;
	long			resDataLength, checkData;
	long			resMapLength, checkMap;
	Boolean			problem;
	
	problem=FALSE;
	FlushVol(0L, 0);
	OpenRF(CurApName, 0, &thisFile);
	if (!problem)
	{
		SetFPos(thisFile, 1, 8L);
		count=4L;
		problem=(FSRead(thisFile, &count, (Ptr)(&resDataLength))!=noErr);
	}
	if (!problem)
	{
		SetFPos(thisFile, 1, 12L);
		count=4L;
		problem=(FSRead(thisFile, &count, (Ptr)(&resMapLength))!=noErr);
	}
	if (!problem)
	{
		SetFPos(thisFile, 1, 144L);
		count=4L;
		problem=(FSRead(thisFile, &count, (Ptr)(&checkData))!=noErr);
	}
	if (!problem)
	{
		SetFPos(thisFile, 1, 148L);
		count=4L;
		problem=(FSRead(thisFile, &count, (Ptr)(&checkMap))!=noErr);
	}
	
	if (!problem)
		problem=((resDataLength!=checkData) || (resMapLength!=checkMap));

	if (problem)
	{
		ParamText(APPLICATION_NAME, "\p", "\p", "\p");
		PositionDialog('ALRT', integrityCheckFailAlert);
		StopAlert(integrityCheckFailAlert,0L);
		ExitToShell();
	}
}
