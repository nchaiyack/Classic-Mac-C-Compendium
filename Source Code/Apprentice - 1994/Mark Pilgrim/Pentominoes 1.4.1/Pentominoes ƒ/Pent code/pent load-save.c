/**********************************************************************\

File:		pent load-save.c

Purpose:	This module handles actually loading and saving games
			and solutions.


Pentominoes - a 2-D geometry board game
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
#include "pent load-save.h"
#include "pent.h"
#include "pent meat.h"
#include "pent files.h"
#include "pent setup.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "msg dialogs.h"
#include "msg sounds.h"
#include "msg environment.h"
#include "msg prefs.h"

FSSpec			gameFile;
FSSpec			solutionFile;
Boolean			deleteTheThing;

void LoadSaveDispatch(Boolean isLoad, Boolean isRealGame, Boolean useOldGame)
{
	if (isLoad)
	{
		if (isRealGame)
		{
			if (GetSourceFile(&gameFile, !isRealGame, useOldGame))
				GetSavedGame(&gameFile);
		}
		else
		{
			if (GetSourceFile(&solutionFile, !isRealGame, useOldGame))
				GetSolution(&solutionFile);
		}
	}
	else
	{
		if (isRealGame)
		{
			useOldGame=useOldGame&&(gameFile.name[0]!=0x00);
			if (useOldGame ? TRUE : GetDestFile(&gameFile, &deleteTheThing, isRealGame))
				SaveGame(gameFile);
		}
		else
		{
			if (GetDestFile(&solutionFile, &deleteTheThing, isRealGame))
				SaveSolution(solutionFile);
		}
	}
}

void SaveGame(FSSpec gameFile)
{
	int				i,j;
	int				buffer[192];
	int				thisFile;
	long			count;
	int				iter;
	OSErr			theError;
	
	for (i=0; i<8; i++)
		for (j=0; j<20; j++)
			buffer[i*20+j]=Board[i][j];
	for (i=0; i<12; i++)
		buffer[160+i]=PiecesPlayed[i];
	buffer[172]=gNumRows;
	buffer[173]=gNumCols;
	buffer[174]=gNumPlayed;
	buffer[175]=gWindowWidth;
	buffer[176]=gWindowHeight;
	buffer[177]=gWhichBoard;
	for (i=0; i<12; i++)
		buffer[178+i]=(int)(PieceUsed[i]);
	buffer[190]=0;
	buffer[191]=0;
	for (i=0; i<191; i++)
		buffer[191]+=buffer[i];

	if (deleteTheThing)
	{
		if (gHasFSSpecs)
			FSpDelete(&gameFile);
		else
			HDelete(gameFile.vRefNum, gameFile.parID, gameFile.name);
	}
	FlushVol(0L, gameFile.vRefNum);
	
	if (gHasFSSpecs)
		theError=FSpCreate(&gameFile, 'PNT5', 'PBrd', smSystemScript);
	else
		theError=HCreate(gameFile.vRefNum, gameFile.parID,
						gameFile.name, 'PNT5', 'PBrd');
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
		count=384L;
		SetEOF(thisFile, 384L);
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

void SaveSolution(FSSpec gameFile)
{
	char			solution[161];
	OSErr			theError;
	int				thisFile;
	long			count;
	int				iter;
	int				i,j;

	iter=1;
	solution[0]=gWhichBoard+0x20;
	for (i=0; i<gNumRows; i++)
		for (j=0; j<gNumCols; j++)
			solution[iter++]=SolutionLetters[Board[i][j]+1];
	
	
	if (deleteTheThing)
	{
		if (gHasFSSpecs)
			FSpDelete(&gameFile);
		else
			HDelete(gameFile.vRefNum, gameFile.parID, gameFile.name);
	}
	FlushVol(0L, gameFile.vRefNum);
	
	if (gHasFSSpecs)
		theError=FSpCreate(&gameFile, 'ttxt', 'TEXT', smSystemScript);
	else
		theError=HCreate(gameFile.vRefNum, gameFile.parID,
						gameFile.name, 'ttxt', 'TEXT');
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
		count=iter;
		SetEOF(thisFile, count);
		SetFPos(thisFile, 1, 0L);
		theError=FSWrite(thisFile, &count, solution);
		FSClose(thisFile);
		FlushVol(0L, gameFile.vRefNum);
	}
	if (theError!=noErr)
	{
		if (gHasFSSpecs)
			FSpDelete(&gameFile);
		else
			HDelete(gameFile.vRefNum, gameFile.parID, gameFile.name);
		ParamText("\pSorry, an error occurred trying to write the solution to disk.","","","");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
	}
	else
		deleteTheThing=TRUE;
}

void GetSavedGame(FSSpec *gameFile)
{
	int				thisFile;
	int				checksum,checkFile;
	long			count;
	int				i,j;
	int				tempWhich;
	Boolean			tempInteractive;
	int				buffer[192];
	OSErr			theError;

	if (gHasFSSpecs)
		theError=FSpOpenDF(gameFile, fsRdPerm, &thisFile);
	else
		theError=HOpen(gameFile->vRefNum, gameFile->parID, gameFile->name, fsRdPerm, &thisFile);

	if (theError==noErr)
	{
		count=384L;
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
		for (i=0; i<8; i++)
			for (j=0; j<20; j++)
				Board[i][j]=buffer[i*20+j];
		for (i=0; i<12; i++)
			PiecesPlayed[i]=buffer[160+i];
		gNumRows=buffer[172];
		gNumCols=buffer[173];
		gNumPlayed=buffer[174];
		gWindowWidth=buffer[175];
		gWindowHeight=buffer[176];
		tempWhich=buffer[177];
		for (i=0; i<12; i++)
			PieceUsed[i]=(Boolean)(buffer[178+i]);
		tempInteractive=(Boolean)(buffer[190]);
		checkFile=buffer[191];
		
		checksum=0;
		for (i=0; i<191; i++)
				checksum+=buffer[i];

		if (checksum!=checkFile)
		{
			ParamText("\pSorry, this game file is damaged or has been altered.","","","");
			PositionDialog('ALRT', generalAlert);
			StopAlert(generalAlert, 0L);
		}
		else
		{
			gNumHilited=0;
			for (i=0; i<gNumRows; i++)
				for (j=0; j<gNumCols; j++)
					if (Board[i][j]==-2)
						gNumHilited++;
			deleteTheThing=TRUE;
			gWhichBoard=tempWhich;
			SaveThePrefs();
			OpenNewGame();
		}
	}
	else
	{
		ParamText("\pSorry, an error occurred while trying to read the game from disk.","","","");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
	}
}

void GetSolution(FSSpec *solutionFile)
{
	int				thisFile;
	Boolean			badfile;
	long			count;
	int				i,j;
	char			solution[161];
	OSErr			theError;
	int				iter,thisone;
	int				oldBoard;
	
	if (gHasFSSpecs)
		theError=FSpOpenDF(solutionFile, fsRdPerm, &thisFile);
	else
		theError=HOpen(solutionFile->vRefNum, solutionFile->parID, solutionFile->name, fsRdPerm, &thisFile);

	if (theError==noErr)
	{
		oldBoard=gWhichBoard;
		badfile=FALSE;
		count=1L;
		SetFPos(thisFile, 1, 0L);
		theError=FSRead(thisFile, &count, solution);
	}
	else
	{
		ParamText("\pSorry, an error occurred trying to read the solution from disk.","","","");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
		gWhichBoard=oldBoard;
		return;
	}
	
	if (theError==noErr)
	{
		gWhichBoard=solution[0]-0x20;
		if ((gWhichBoard<1) || (gWhichBoard>31))
		{
			badfile=TRUE;
			theError=noErr;
		}
		else
		{
			CalculateRowsandCols();
			count=(long)(gNumRows*gNumCols);
			SetFPos(thisFile, 1, 1L);
			theError=FSRead(thisFile, &count, &(solution[1]));
		}
	}
	
	FSClose(thisFile);
	
	if (theError==noErr)
	{
		if (!badfile)
		{
			iter=1;
			for (i=0; i<gNumRows; i++)
				for (j=0; j<gNumCols; j++)
				{
					thisone=0;
					while ((thisone<14) && (SolutionLetters[thisone]!=solution[iter]))
						thisone++;
					badfile=(thisone==14);
					if (badfile)
					{
						j=gNumCols;
						i=gNumRows;
					}
					else
					{
						Board[i][j]=thisone-1;
						iter++;
					}
				}
		}
		
		if (badfile)
		{
			ParamText("\pSorry, this file is not a solution file.","","","");
			PositionDialog('ALRT', generalAlert);
			StopAlert(generalAlert, 0L);
			gWhichBoard=oldBoard;
		}
		else
		{
			for (i=0; i<12; i++)
			{
				PiecesPlayed[i]=i;
				PieceUsed[i]=TRUE;
			}
			gNumPlayed=12;
			gWindowWidth = (gNumCols*25) + 10;
			gWindowHeight = (gNumRows*25) + 95;
			gNumHilited=0;
			
			deleteTheThing=TRUE;
			SaveThePrefs();
			gameFile.name[0]=0x00;
			OpenNewGame();
		}
	}
	else
	{
		ParamText("\pSorry, an error occurred trying to read the solution from disk.","","","");
		PositionDialog('ALRT', generalAlert);
		StopAlert(generalAlert,0L);
		gWhichBoard=oldBoard;
	}
}
