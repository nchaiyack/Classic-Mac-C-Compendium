/**********************************************************************\

File:		v file management.c

Purpose:	This module handles opening and closing files and forks.


Voyeur -- a no-frills file viewer
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

#include "v file management.h"
#include "v structs.h"
#include "msg environment.h"

FSSpec			editFile;
int				fileRefNum;
unsigned long	offset[2];
Boolean			changed;
unsigned char	buffer[256];
unsigned long	totallen[2];
int				forknum;

int OpenTheFile(FSSpec *myFSS, int *thisFile)
{
	OSErr			theError;
	long			count;
	CInfoPBRec		pb;
	
	pb.hFileInfo.ioCompletion=0L;
	pb.hFileInfo.ioNamePtr=myFSS->name;
	pb.hFileInfo.ioVRefNum=myFSS->vRefNum;
	pb.hFileInfo.ioDirID=myFSS->parID;
	pb.hFileInfo.ioFDirIndex=0;	/* very important */
	theError=PBGetCatInfo(&pb, FALSE);
	if (theError!=noErr)
		return diskReadErr;
	
	totallen[0]=pb.hFileInfo.ioFlLgLen;
	totallen[1]=pb.hFileInfo.ioFlRLgLen;
	forknum=(totallen[0]==0L) ? ((totallen[1]==0L) ? 0 : 1) : 0;
	
	if (forknum==0)
		theError=OpenTheDataFork(myFSS, thisFile);
	else
		theError=OpenTheResourceFork(myFSS, thisFile);
	if (theError!=noErr)
		return diskReadErr;
	
	changed=FALSE;
	offset[0]=0L;
	offset[1]=0L;
	
	return GetBuffer(*thisFile, offset[forknum], buffer);
}

void CloseTheFile(int fileRefNum)
{
	FSClose(fileRefNum);
}

int GetBuffer(int fileRefNum, unsigned long off, unsigned char output[256])
{
	long			count;
	OSErr			theError;
	int				i;
	
	count=totallen[forknum]-off<256L ? totallen[forknum]-off : 256L;
	SetFPos(fileRefNum, 1, off);
	theError=FSRead(fileRefNum, &count, output);
	if (count!=256L)
	{
		for (i=count; i<256; i++)
			output[i]=0x00;
	}

	return (theError==noErr) ? allsWell : diskReadErr;
}

OSErr OpenTheDataFork(FSSpec *myFSS, int *thisFile)
{
	if (gHasFSSpecs)
		return FSpOpenDF(myFSS, fsRdWrPerm, thisFile);
	else
		return HOpen(myFSS->vRefNum, myFSS->parID, myFSS->name, fsRdWrPerm, thisFile);
}

OSErr OpenTheResourceFork(FSSpec *myFSS, int *thisFile)
{
	if (gHasFSSpecs)
		return FSpOpenRF(myFSS, fsRdWrPerm, thisFile);
	else
		return HOpenRF(myFSS->vRefNum, myFSS->parID, myFSS->name, fsRdWrPerm, thisFile);
}
