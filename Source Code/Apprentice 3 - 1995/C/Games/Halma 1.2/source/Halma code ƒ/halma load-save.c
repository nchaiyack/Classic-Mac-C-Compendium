/**********************************************************************\

File:		halma load-save.c

Purpose:	This module handles loading and saving games on disk.

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

#include "error.h"
#include "halma load-save.h"
#include "file interface.h"
#include "halma.h"
#include "dialogs.h"
#include "environment.h"
#include "util.h"
#include "Script.h"

FSSpec			gameFile;
Boolean			deleteTheThing;

typedef struct
{
	unsigned short	version;			/* for forward compatibility */
	short			numRows;			/* number of rows in saved board */
	short			numColumns;			/* number of columns in saved board */
	short			theboard[9][9];		/* actual board */
	short			numMoves;			/* number of moves so far */
	short			numJumps;			/* number of jumps in current move so far */
	short			stickyButtonRow;	/* which piece is down (row) */
	short			stickyButtonColumn;	/* which piece is down (column) */
	short			thisStartRow;		/* start of current jump chain (row) */
	short			thisStartColumn;	/* start of current jump chain (column) */
	Str255			thisJumpString;		/* current jump chain */
	unsigned long	fullJumpPtrSize;	/* GetHandleSize(gTheFullJumpHandle) */
	unsigned short	crc;				/* checksum */
} SaveStruct;

void InitLoadSave(void)
{
	gameFile.name[0]=0x00;
	deleteTheThing=FALSE;
}

void LoadSaveDispatch(Boolean isLoad, Boolean useOldGame)
{
	if (isLoad)
	{
		if (GetSourceFile(&gameFile, FALSE, FALSE))
			HandleError(GetSavedGame(&gameFile), FALSE);
	}
	else
	{
		useOldGame=useOldGame&&(gameFile.name[0]!=0x00);
		if (useOldGame ? TRUE : GetDestFile(&gameFile, &deleteTheThing, "\pSave Halma game as..."))
			HandleError(SaveGame(gameFile), FALSE);
	}
}

enum ErrorTypes SaveGame(FSSpec gameFile)
{
	OSErr			theError;
	short			thisFile;
	long			count;
	SaveStruct		data;
	short			i,j;
	
	data.numRows=gNumRows;
	data.numColumns=gNumColumns;
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			data.theboard[i][j]=Board[i][j];
	data.numMoves=gNumMoves;
	data.numJumps=gNumJumps;
	data.stickyButtonRow=gStickyButtonRow;
	data.stickyButtonColumn=gStickyButtonColumn;
	data.thisStartRow=gThisStartRow;
	data.thisStartColumn=gThisStartColumn;
	Mymemcpy((Ptr)data.thisJumpString, (Ptr)gThisJumpString, 256);
	data.fullJumpPtrSize=GetHandleSize((Handle)gTheFullJumpHandle);
	data.version=SAVE_VERSION;
	data.crc=0;
	data.crc=TheChecksum((Ptr)(&data), sizeof(SaveStruct));
	
	if (deleteTheThing)
	{
		if (gHasFSSpecs)
			FSpDelete(&gameFile);
		else
			HDelete(gameFile.vRefNum, gameFile.parID, gameFile.name);
	}
	FlushVol(0L, gameFile.vRefNum);
	
	if (gHasFSSpecs)
		theError=FSpCreate(&gameFile, CREATOR, SAVE_TYPE, smSystemScript);
	else
		theError=HCreate(gameFile.vRefNum, gameFile.parID,
						gameFile.name, CREATOR, SAVE_TYPE);
	FlushVol(0L, gameFile.vRefNum);
	
	if (theError!=noErr)
		return kCantCreateGame;
	
	if (gHasFSSpecs)
		theError=FSpOpenDF(&gameFile, fsRdWrPerm, &thisFile);
	else
		theError=HOpen(gameFile.vRefNum, gameFile.parID,
						gameFile.name, fsRdWrPerm, &thisFile);
	
	if (theError!=noErr)
		return kCantOpenGameToSave;

	count=sizeof(SaveStruct);
	SetEOF(thisFile, count+data.fullJumpPtrSize);
	SetFPos(thisFile, 1, 0L);
	theError=FSWrite(thisFile, &count, &data);
	
	if (theError==noErr)
	{
		count=data.fullJumpPtrSize;
		HLock((Handle)gTheFullJumpHandle);
		theError=FSWrite(thisFile, &count, *gTheFullJumpHandle);
		HUnlock((Handle)gTheFullJumpHandle);
	}
	
	FSClose(thisFile);
	FlushVol(0L, gameFile.vRefNum);

	if (theError!=noErr)
	{
		if (gHasFSSpecs)
			FSpDelete(&gameFile);
		else
			HDelete(gameFile.vRefNum, gameFile.parID, gameFile.name);
		
		gameFile.name[0]=0x00;
		return kCantWriteGame;
	}
	else deleteTheThing=TRUE;
	
	return allsWell;
}

enum ErrorTypes GetSavedGame(FSSpec *gameFile)
{
	short			thisFile;
	long			count;
	short			i,j;
	SaveStruct		data;
	OSErr			theError;
	unsigned short	checksum, savedChecksum;
	Ptr				temp;
	
	if (gHasFSSpecs)
		theError=FSpOpenDF(gameFile, fsRdPerm, &thisFile);
	else
		theError=HOpen(gameFile->vRefNum, gameFile->parID, gameFile->name, fsRdPerm, &thisFile);
	
	if (theError!=noErr)
		return kCantOpenGameToLoad;
	
	count=sizeof(SaveStruct);
	SetFPos(thisFile, 1, 0L);
	theError=FSRead(thisFile, &count, &data);

	if (theError!=noErr)
	{
		FSClose(thisFile);
		return kCantLoadGame;
	}
	
	deleteTheThing=TRUE;
	
	if (data.version!=SAVE_VERSION)
	{
		FSClose(thisFile);
		return kSaveVersionNotSupported;
	}
	
	savedChecksum=data.crc;
	data.crc=0;
	checksum=TheChecksum((Ptr)(&data), sizeof(SaveStruct));
	if (checksum!=savedChecksum)
	{
		gameFile->name[0]=0x00;
		FSClose(thisFile);
		return kBadChecksum;
	}
	
	count=data.fullJumpPtrSize;
	temp=NewPtr(count);
	theError=FSRead(thisFile, &count, temp);
	FSClose(thisFile);
	
	if (theError!=noErr)
	{
		DisposePtr(temp);
		return kCantLoadGame;
	}
	
	gNumRows=data.numRows;
	gNumColumns=data.numColumns;
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			Board[i][j]=data.theboard[i][j];
	gNumMoves=data.numMoves;
	gNumJumps=data.numJumps;
	gStickyButtonRow=data.stickyButtonRow;
	gStickyButtonColumn=data.stickyButtonColumn;
	gThisStartRow=data.thisStartRow;
	gThisStartColumn=data.thisStartColumn;
	Mymemcpy((Ptr)gThisJumpString, (Ptr)(data.thisJumpString), 256);
	SetHandleSize((Handle)gTheFullJumpHandle, data.fullJumpPtrSize);
	HLock((Handle)gTheFullJumpHandle);
	Mymemcpy((Ptr)(*gTheFullJumpHandle), (Ptr)temp, data.fullJumpPtrSize);
	HUnlock((Handle)gTheFullJumpHandle);
	DisposePtr(temp);
	
	StartGame();
	
	return allsWell;
}

unsigned short TheChecksum(Ptr input, unsigned short len)
{
	unsigned short	i;
	Boolean			shiftedOut;
	unsigned short	checksum;
	
	checksum=0;
	for (i=0; i<len; i++)
	{
		shiftedOut=checksum&0x8000;
		checksum+=*((unsigned char*)((long)input+i));
		checksum<<=1;
		if (shiftedOut)
			checksum^=0xdead;
	}
	
	return checksum;
}
