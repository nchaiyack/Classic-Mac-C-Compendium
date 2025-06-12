/**********************************************************************\

File:		ghost load-save.c

Purpose:	This module handles loading and saving games on disk.


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
#include "ghost error.h"
#include "ghost load-save.h"
#include "ghost files.h"
#include "ghost.h"
#include "msg dialogs.h"
#include "msg environment.h"
#include "util.h"
#include "Script.h"

FSSpec			gameFile;
Boolean			deleteTheThing;

typedef struct
{
	unsigned int	version;					/* for forward compatibility */
	int				numComputerPlayers;
	int				computerIconIndex[5];
	int				numHumanPlayers;
	unsigned char	humanName[5][40];
	int				humanIconIndex[5];
	int				actualHumanPlayers;
	int				actualComputerPlayers;
	int				numPlayers;
	int				playOrderIndex[10];
	int				computerPlayerScore[5];
	int				humanPlayerScore[5];
	unsigned char	theWord[20];
	int				currentPlayer;
	unsigned char	showMessageBox;
	unsigned char	gameSpeed;
	unsigned char	computerIntelligence;
	unsigned char	useFullDictionary;
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
	data.numComputerPlayers=gNumComputerPlayers;
	for (i=0; i<5; i++)
		data.computerIconIndex[i]=gComputerIconIndex[i];
	data.numHumanPlayers=gNumHumanPlayers;
	for (i=0; i<gNumHumanPlayers; i++)
		BlockMove(*gHumanName[i], data.humanName[i], 40);
	for (i=0; i<5; i++)
		data.humanIconIndex[i]=gHumanIconIndex[i];
	data.actualHumanPlayers=gActualHumanPlayers;
	data.actualComputerPlayers=gActualComputerPlayers;
	data.numPlayers=gNumPlayers;
	for (i=0; i<10; i++)
		data.playOrderIndex[i]=gPlayOrderIndex[i];
	for (i=0; i<5; i++)
		data.computerPlayerScore[i]=gComputerPlayerScore[i];
	for (i=0; i<5; i++)
		data.humanPlayerScore[i]=gHumanPlayerScore[i];
	BlockMove(gTheWord, data.theWord, 20);
	data.currentPlayer=gCurrentPlayer;
	data.showMessageBox=gShowMessageBox;
	data.gameSpeed=gGameSpeed;
	data.computerIntelligence=gComputerIntelligence;
	data.useFullDictionary=gUseFullDictionary;
	
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

	gNumComputerPlayers=data.numComputerPlayers;
	for (i=0; i<5; i++)
		gComputerIconIndex[i]=data.computerIconIndex[i];
	gNumHumanPlayers=data.numHumanPlayers;
	for (i=0; i<gNumHumanPlayers; i++)
	{
		if (gHumanName[i]!=0L)
			DisposeHandle(gHumanName[i]);
		gHumanName[i]=NewString(data.humanName[i]);
	}
	for (i=0; i<5; i++)
		gHumanIconIndex[i]=data.humanIconIndex[i];
	gActualHumanPlayers=data.actualHumanPlayers;
	gActualComputerPlayers=data.actualComputerPlayers;
	gNumPlayers=data.numPlayers;
	for (i=0; i<10; i++)
		gPlayOrderIndex[i]=data.playOrderIndex[i];
	for (i=0; i<5; i++)
		gComputerPlayerScore[i]=data.computerPlayerScore[i];
	for (i=0; i<5; i++)
		gHumanPlayerScore[i]=data.humanPlayerScore[i];
	BlockMove(data.theWord, gTheWord, 20);
	gCurrentPlayer=data.currentPlayer;
	gShowMessageBox=data.showMessageBox;
	gGameSpeed=data.gameSpeed;
	gComputerIntelligence=data.computerIntelligence;
	gUseFullDictionary=data.useFullDictionary;
	
	gInProgress=TRUE;
	gCurrentPlayer--;
	StartGame();
	
	return allsWell;
}
