/**********************************************************************\

File:		jotto load-save.c

Purpose:	This module handles loading and saving games on disk.


Jotto ][ -=- a simple word game, revisited
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

#include "jotto globals.h"
#include "jotto error.h"
#include "jotto load-save.h"
#include "jotto files.h"
#include "jotto.h"
#include "msg dialogs.h"
#include "msg environment.h"
#include "util.h"
#include "Script.h"

FSSpec			gameFile;
Boolean			deleteTheThing;

typedef struct
{
	unsigned int	version;					/* for forward compatibility */
	unsigned char	numLetters;					/* 5 or 6 */
	unsigned char	computerWord[6];			/* encrypted! */
	unsigned char	numRight[MAX_TRIES];		/* # letters right in n-th guess */
	unsigned char	humanWord[MAX_TRIES][6];	/* n-th guess */
	unsigned char	numTries;					/* # guesses so far */
	unsigned char	whichChar;					/* which char 1-6 of current word */
	unsigned char	checksum;					/* checksum of previous data */
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
		if (GetSourceFile(&gameFile, useOldGame))
			HandleError(GetSavedGame(&gameFile));
	}
	else
	{
		useOldGame=useOldGame&&(gameFile.name[0]!=0x00);
		if (useOldGame ? TRUE : GetDestFile(&gameFile, &deleteTheThing))
			HandleError(SaveGame(gameFile));
	}
}

int SaveGame(FSSpec gameFile)
{
	OSErr			theError;
	int				thisFile;
	long			count;
	SaveStruct		data;
	unsigned char	checksum;
	int				i,j;
	
	data.version=SAVE_VERSION;
	data.numLetters=gNumLetters;
	for (i=0; i<6; i++)
		data.computerWord[i]=gComputerWord[i]^0x42;
	for (i=0; i<MAX_TRIES; i++)
		data.numRight[i]=gNumRight[i];
	for (i=0; i<MAX_TRIES; i++)
		for (j=0; j<6; j++)
			data.humanWord[i][j]=gHumanWord[i][j];
	data.numTries=gNumTries;
	data.whichChar=gWhichChar;	
	checksum=0;
	for (i=0; i<sizeof(SaveStruct)-2; i++)
		checksum+=*((unsigned char*)((long)&data+i));
	data.checksum=checksum;
	
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
	SetEOF(thisFile, count);
	SetFPos(thisFile, 1, 0L);
	theError=FSWrite(thisFile, &count, &data);
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

int GetSavedGame(FSSpec *gameFile)
{
	int				thisFile;
	unsigned char	checksum;
	long			count;
	int				i,j;
	SaveStruct		data;
	OSErr			theError;

	if (gHasFSSpecs)
		theError=FSpOpenDF(gameFile, fsRdPerm, &thisFile);
	else
		theError=HOpen(gameFile->vRefNum, gameFile->parID, gameFile->name, fsRdPerm, &thisFile);
	
	if (theError!=noErr)
		return kCantOpenGameToLoad;
	
	count=sizeof(SaveStruct);
	SetFPos(thisFile, 1, 0L);
	theError=FSRead(thisFile, &count, &data);
	FSClose(thisFile);

	if (theError!=noErr)
		return kCantLoadGame;
	
	deleteTheThing=TRUE;
	
	if (data.version!=SAVE_VERSION)
		return kSaveVersionNotSupported;
	
	checksum=0;
	for (i=0; i<sizeof(SaveStruct)-2; i++)
			checksum+=*((unsigned char*)((long)&data+i));

	if (checksum!=data.checksum)
	{
		gameFile->name[0]=0x00;
		return kBadChecksum;
	}

	gNumLetters=data.numLetters;
	for (i=0; i<6; i++)
		gComputerWord[i]=data.computerWord[i]^0x42;
	for (i=0; i<15; i++)
		gNumRight[i]=data.numRight[i];
	for (i=0; i<15; i++)
		for (j=0; j<6; j++)
			gHumanWord[i][j]=data.humanWord[i][j];
	gNumTries=data.numTries;
	gWhichChar=data.whichChar;
	if (gNumTries==MAX_TRIES)
		Mymemcpy((Ptr)gHumanWord[gNumTries], (Ptr)gComputerWord, 6);
	
	gInProgress=(gNumRight[gNumTries]!=gNumLetters) && (gNumTries<MAX_TRIES);
	StartGame();
	
	return allsWell;
}
