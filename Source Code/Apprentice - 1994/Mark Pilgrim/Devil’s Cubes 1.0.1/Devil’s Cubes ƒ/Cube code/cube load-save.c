/**********************************************************************\

File:		cube load-save.c

Purpose:	This module handles actually loading and saving games.


Devil�s Cubes -- a simple cubes puzzle
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

#include "Script.h"
#include "cube load-save.h"
#include "cube.h"
#include "cube meat.h"
#include "cube files.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "msg dialogs.h"
#include "msg sounds.h"
#include "msg environment.h"
#include "msg prefs.h"

FSSpec			gameFile;
Boolean			deleteTheThing;

void LoadSaveDispatch(Boolean isLoad, Boolean useOldGame)
{
	if (isLoad)
	{
		if (GetSourceFile(&gameFile, useOldGame))
			GetSavedGame(&gameFile);
	}
	else
	{
		useOldGame=useOldGame&&(gameFile.name[0]!=0x00);
		if (useOldGame ? TRUE : GetDestFile(&gameFile, &deleteTheThing))
			SaveGame(gameFile);
	}
}

void SaveGame(FSSpec gameFile)
{
	int				i,j;
	int				buffer[25];
	int				thisFile;
	long			count;
	int				iter;
	OSErr			theError;
	
	for (i=0; i<4; i++)
		for (j=0; j<6; j++)
			buffer[i*6+j]=Cube[i][j];
	
	buffer[24]=0;
	for (i=0; i<24; i++)
		buffer[24]+=buffer[i];
	
	if (deleteTheThing)
	{
		if (gHasFSSpecs)
			FSpDelete(&gameFile);
		else
			HDelete(gameFile.vRefNum, gameFile.parID, gameFile.name);
	}
	FlushVol(0L, gameFile.vRefNum);
	
	if (gHasFSSpecs)
		theError=FSpCreate(&gameFile, 'Dcbe', 'DcSG', smSystemScript);
	else
		theError=HCreate(gameFile.vRefNum, gameFile.parID,
						gameFile.name, 'Dcbe', 'DcSG');
	FlushVol(0L, gameFile.vRefNum);
	
	if (theError==noErr)
	{
		if (gHasFSSpecs)
			theError=FSpOpenDF(&gameFile, fsRdWrPerm, &thisFile);
		else
			theError=HOpen(gameFile.vRefNum, gameFile.parID,
							gameFile.name, fsRdWrPerm, &thisFile);
	}
	if (theError==noErr)
	{
		count=50L;
		SetEOF(thisFile, 50L);
		SetFPos(thisFile, 1, 0L);
		theError=FSWrite(thisFile, &count, buffer);
		FSClose(thisFile);
		FlushVol(0L, gameFile.vRefNum);
	}
	if (theError!=noErr)
	{
		if (gHasFSSpecs)
			FSpDelete(&gameFile);
		else
			HDelete(gameFile.vRefNum, gameFile.parID, gameFile.name);
		ParamText("\pSorry, an error occurred trying to write the game to disk.","","","");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
	}
	else
		deleteTheThing=TRUE;
}

void GetSavedGame(FSSpec *gameFile)
{
	int				thisFile;
	int				checksum;
	long			count;
	int				i,j;
	int				buffer[25];
	OSErr			theError;

	if (gHasFSSpecs)
		theError=FSpOpenDF(gameFile, fsRdPerm, &thisFile);
	else
		theError=HOpen(gameFile->vRefNum, gameFile->parID, gameFile->name, fsRdPerm, &thisFile);

	if (theError==noErr)
	{
		count=50L;
		SetFPos(thisFile, 1, 0L);
		theError=FSRead(thisFile, &count, buffer);
		FSClose(thisFile);
	}
	else
	{
		ParamText("\pSorry, an error occurred while trying to read the game from disk.","","","");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
		return;
	}
	
	if (theError==noErr)
	{				
		checksum=0;
		for (i=0; i<24; i++)
				checksum+=buffer[i];

		if (checksum!=buffer[24])
		{
			ParamText("\pSorry, this game file is damaged or has been altered.","","","");
			PositionDialog('ALRT', generalAlert);
			StopAlert(generalAlert, 0L);
		}
		else
		{
			for (i=0; i<4; i++)
				for (j=0; j<6; j++)
					Cube[i][j]=buffer[i*6+j];
			deleteTheThing=TRUE;
			OpenMainWindow();
			AdjustMenus();
		}
	}
	else
	{
		ParamText("\pSorry, an error occurred while trying to read the game from disk.","","","");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
	}
}
