/**********************************************************************\

File:		ghost dictionary.c

Purpose:	This module handles dictionary maintenance -- loading the
			words from disk and making sure the dictionaries haven't
			been tampered with.


Ghost -=- a classic word-building challenge
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

#include "ghost globals.h"
#include "ghost dictionary.h"
#include "ghost files.h"
#include "msg environment.h"

Ptr				gTheDictionary[2];
long			gIndex[2][27];
unsigned char	gUseFullDictionary;

static int		partialRefNum;
static int		fullRefNum;

void DisposeDictionary(void)
{
	if (gTheDictionary[kPartial])
	{
		DisposePtr(gTheDictionary[kPartial]);
		gTheDictionary[kPartial]=0L;
	}
	
	if (gTheDictionary[kFull])
	{
		DisposePtr(gTheDictionary[kFull]);
		gTheDictionary[kFull]=0L;
	}
}

int LoadDictionary(void)
{
	long			thisDirID;
	OSErr			isHuman;
	long			thisEOF;
	FSSpec			fs;
	int				i;
	Handle			tempHandle;
	long			count;
	
	tempHandle=GetResource('Indx', 128);
	if (tempHandle==0L)
		LoadResource(tempHandle);
	if (tempHandle==0L)
		return kNoIndex;
	HLock(tempHandle);
	BlockMove(*tempHandle, &(gIndex[0][0]), 216);	
	ReleaseResource(tempHandle);
	
	thisDirID=GetApplicationParID();
	if (thisDirID==-1L)
		return kNoDictionaries;
	
	partialRefNum=fullRefNum=0;
	gTheDictionary[kPartial]=gTheDictionary[kFull]=0L;
	
	MyMakeFSSpec(0, thisDirID, "\p:Ghost dict (small)", &fs);
	if (gHasFSSpecs)
		isHuman=FSpOpenDF(&fs, fsRdPerm, &partialRefNum);
	else
		isHuman=HOpen(0, thisDirID, fs.name, fsRdPerm, &partialRefNum);
	
	if (isHuman!=noErr)
		return kCantFindSmallDict;
	
	GetEOF(partialRefNum, &thisEOF);
	if (thisEOF!=gIndex[kPartial][26])
	{
		CloseDictionary();
		return kSmallDictDamaged;
	}
		
	gTheDictionary[kPartial]=NewPtr(thisEOF);
	if (gTheDictionary==0L)
	{
		CloseDictionary();
		return kNoMemory;
	}
	
	count=thisEOF;
	SetFPos(partialRefNum, 1, 0L);
	isHuman=FSRead(partialRefNum, &count, gTheDictionary[kPartial]);
	CloseDictionary();
	if (isHuman!=noErr)
		return kCantReadSmallDict;
	
	
	MyMakeFSSpec(0, thisDirID, "\p:Ghost dict (large)", &fs);
	if (gHasFSSpecs)
		isHuman=FSpOpenDF(&fs, fsRdPerm, &fullRefNum);
	else
		isHuman=HOpen(0, thisDirID, fs.name, fsRdPerm, &fullRefNum);
	
	if (isHuman!=noErr)
		return kCantFindLargeDict;
	
	GetEOF(fullRefNum, &thisEOF);
	if (thisEOF!=gIndex[kFull][26])
	{
		CloseDictionary();
		return kLargeDictDamaged;
	}
		
	gTheDictionary[kFull]=NewPtr(thisEOF);
	if (gTheDictionary==0L)
	{
		CloseDictionary();
		return kNoMemory;
	}
	
	count=thisEOF;
	SetFPos(fullRefNum, 1, 0L);
	isHuman=FSRead(fullRefNum, &count, gTheDictionary[kFull]);
	CloseDictionary();
	if (isHuman!=noErr)
		return kCantReadLargeDict;

	for (i=0; i<26; i++)
	{
		if (*((char*)((long)gTheDictionary[kPartial]+gIndex[kPartial][i]))!=('A'+i))
			return kSmallDictDamaged;
		if (*((char*)((long)gTheDictionary[kFull]+gIndex[kFull][i]))!=('A'+i))
			return kLargeDictDamaged;
	}
	
	return allsWell;
}

void CloseDictionary(void)
{
	if (partialRefNum)
	{
		FSClose(partialRefNum);
		partialRefNum=0;
	}
	
	if (fullRefNum)
	{
		FSClose(fullRefNum);
		fullRefNum=0;
	}
}

long GetApplicationParID(void)
{
	CInfoPBRec		pb;
	int				err;
	
	pb.hFileInfo.ioCompletion=0L;
	pb.hFileInfo.ioNamePtr=CurApName;
	pb.hFileInfo.ioVRefNum=0;
	pb.hFileInfo.ioFDirIndex=0;
	pb.hFileInfo.ioDirID=0;
	err=PBGetCatInfo(&pb, FALSE);
	if (err!=noErr)
		return -1L;
	
	return pb.hFileInfo.ioFlParID;
}
