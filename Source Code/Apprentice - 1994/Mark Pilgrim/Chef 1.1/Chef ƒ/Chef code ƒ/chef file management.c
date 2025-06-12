/**********************************************************************\

File:		chef file management.c

Purpose:	This module handles creating, setting up, and deleting
			temp files, opening and reading the source file, and
			opening and writing the dest file.


Chef -=- convert text to Swedish chef talk
Copyright ©1994, Mark Pilgrim

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

#include "Script.h"
#include "program globals.h"
#include "chef file management.h"
#include "msg environment.h"
#include "util.h"

FSSpec			inputFS, outputFS, tempFS;
Boolean			deleteTheThing;
int				inputRefNum, outputRefNum;
unsigned long	gWhatsReallyInInputBuffer;
unsigned long	gTotalOutputLength;

void InitFiles(void)
{
	inputRefNum=outputRefNum=0;
	gWhatsReallyInInputBuffer=gTotalOutputLength=0L;
}

int OpenInputFile(void)
{
	OSErr			theError;
	
	if (gHasFSSpecs)
		theError=FSpOpenDF(&inputFS, fsRdPerm, &inputRefNum);
	else
		theError=HOpen(inputFS.vRefNum, inputFS.parID, inputFS.name, fsRdPerm, &inputRefNum);
	
	if (theError==noErr)
		GetEOF(inputRefNum, &gInputLength);
	
	return (theError==noErr) ? allsWell : kCantOpenInputFile;
}

int CreateTempFile(void)
{
	int					i;
	Str255				timeStr;
	OSErr				isHuman;
	unsigned long		fileType;
	int					resultCode;
	
	tempFS=outputFS;
	tempFS.name[0]=0x05;
	tempFS.name[1]='b';
	tempFS.name[2]='o';
	tempFS.name[3]='r';
	tempFS.name[4]='k';
	tempFS.name[5]='.';
	NumToString(Time&0x7fffffff, timeStr);
	for (i=1; i<=timeStr[0]; i++)
		tempFS.name[++(tempFS.name[0])]=timeStr[i];
	
	if (gHasFSSpecs)
		isHuman=FSpCreate(&tempFS, CREATOR, 'TeMp', smSystemScript);
	else
		isHuman=HCreate(tempFS.vRefNum, tempFS.parID, tempFS.name, CREATOR, 'TeMp');
	
	return (isHuman==noErr) ? allsWell : kCantCreateTempFile;
}

int SetupTempFile(void)
{
	OSErr				isHuman;
	unsigned long		chefLen;
	
	if (gHasFSSpecs)
		isHuman=FSpOpenDF(&tempFS, fsRdWrPerm, &outputRefNum);
	else
		isHuman=HOpen(tempFS.vRefNum, tempFS.parID, tempFS.name, fsRdWrPerm, &outputRefNum);
	
	if (isHuman!=noErr)
		return kCantOpenTempFile;
		
	SetEOF(outputRefNum, 0L);
	SetFPos(outputRefNum, 1, 0L);

	return allsWell;
}

void FinalizeFiles(Boolean good)
{
	FInfo				theInfo;
	
	if (inputRefNum)
		FSClose(inputRefNum);
	if (outputRefNum)
	{
		SetEOF(outputRefNum, gTotalOutputLength);
		FSClose(outputRefNum);
	}
	FlushVol(0L, outputRefNum);
	
	if (!good)
	{
		if(gHasFSSpecs)
			FSpDelete(&tempFS);
		else
			HDelete(tempFS.vRefNum, tempFS.parID, tempFS.name);
		
		return;
	}
	
	if (deleteTheThing)
	{
		if (gHasFSSpecs)
			FSpDelete(&outputFS);
		else
			HDelete(outputFS.vRefNum, outputFS.parID, outputFS.name);
	}
	FlushVol(0L, outputRefNum);
		
	if (gHasFSSpecs)
		FSpGetFInfo(&inputFS, &theInfo);
	else
		HGetFInfo(inputFS.vRefNum, inputFS.parID, inputFS.name, &theInfo);

	theInfo.fdFlags&=~0x0100;	/* clear Inited bit */
	
	if (gHasFSSpecs)
		FSpSetFInfo(&tempFS, &theInfo);
	else
		HSetFInfo(tempFS.vRefNum, tempFS.parID, tempFS.name, &theInfo);
	FlushVol(0L, outputRefNum);
	
	if (gHasFSSpecs)
		FSpRename(&tempFS, outputFS.name);
	else
		HRename(tempFS.vRefNum, tempFS.parID, tempFS.name, outputFS.name);
	FlushVol(0L, outputRefNum);
}

int ReadInputFile(int thisFile, unsigned char* buffer, unsigned long count)
{
	OSErr			isHuman;
	unsigned long	temp;
	
	temp=count;
	isHuman=FSRead(thisFile, &temp, buffer);
	return ((isHuman==eofErr) || (isHuman==noErr)) ? allsWell : kCantReadInputFile;
}

int WriteTempFile(int thatFile, unsigned char* buffer, unsigned long theLength)
{
	unsigned long	oldFPos;
	
	gTotalOutputLength+=theLength;
	GetFPos(thatFile, &oldFPos);
	SetEOF(thatFile, gTotalOutputLength);
	SetFPos(thatFile, 1, oldFPos);
	return (FSWrite(thatFile, &theLength, buffer)==noErr ? allsWell : kCantWriteTempFile);
}
