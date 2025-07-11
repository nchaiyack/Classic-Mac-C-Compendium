/**********************************************************************\

File:		MSG Prepare.c

Purpose:	This module handles storing the resource fork and map
			length to the application space of the resource fork of
			a file (to be checked when that file is run as a
			minimal integrity check).


MSG Prepare 1.0 -- minimal integrity check preparation program
Copyright (C) 1993 Mark Pilgrim

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

#include "MSG Prepare.h"

Boolean			gDone;

void CheckDropKick(void)
{
	int			i,j,k;
	AppFile		myFile;
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		k=1;
		while (k<=j)
		{
			GetAppFiles(k, &myFile);
			OpenFile(myFile.fName, myFile.vRefNum);
			ClrAppFiles(k++);
		}
		gDone=TRUE;
	}
}

void OpenFile(Str255 fName, int vRefNum)
{
	int				thisFile;
	long			count;
	long			resMapOffset;
	int				resAttributes;
	long			resDataLength;
	long			resMapLength;
	
	OpenRF(fName, vRefNum, &thisFile);
	count=4L;
	SetFPos(thisFile, 1, 4L);
	FSRead(thisFile, &count, (Ptr)(&resMapOffset));
	resMapOffset+=22L;
	count=2L;
	SetFPos(thisFile, 1, resMapOffset);
	FSRead(thisFile, &count, (Ptr)(&resAttributes));
	resAttributes|=0x8000;
	count=2L;
	SetFPos(thisFile, 1, resMapOffset);
	FSWrite(thisFile, &count, (Ptr)(&resAttributes));
	SetFPos(thisFile, 1, 8L);
	count=4L;
	FSRead(thisFile, &count, (Ptr)(&resDataLength));
	SetFPos(thisFile, 1, 12L);
	count=4L;
	FSRead(thisFile, &count, (Ptr)(&resMapLength)); 
	SetFPos(thisFile, 1, 144L);
	count=4L;
	FSWrite(thisFile, &count, (Ptr)(&resDataLength));
	SetFPos(thisFile, 1, 148L);
	count=4L;
	FSWrite(thisFile, &count, (Ptr)(&resMapLength));
	FSClose(thisFile);
	FlushVol(0L, vRefNum);
}
