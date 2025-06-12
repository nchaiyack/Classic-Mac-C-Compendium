/**********************************************************************\

File:		ghost.c

Purpose:	This module handles game initialization, shutdown, and
			event handling.


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
#include "ghost.h"
#include "ghost setup.h"
#include "ghost files.h"
#include "ghost load-save.h"
#include "ghost strategy.h"
#include "ghost end.h"
#include "ghost dictionary.h"
#include "ghost error.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "msg environment.h"
#include "msg timing.h"
#include "msg dialogs.h"
#include "msg sounds.h"

CIconHandle		gColorIcons[24];
Handle			gBWIcons[24];

int				gNumComputerPlayers;
StringHandle	gIconNames[24];
int				gComputerIconIndex[5];
int				gNumHumanPlayers;
StringHandle	gHumanName[5];
int				gHumanIconIndex[5];

Str255			gTheWord;
Str255			gTheMessage;
int				gCurrentPlayer;

int				gComputerPlayerScore[5];
int				gHumanPlayerScore[5];

int				gNumPlayers;
int				gPlayOrderIndex[10];
int				gActualHumanPlayers;
int				gActualComputerPlayers;

unsigned char	gShowMessageBox;
unsigned char	gGameSpeed;
unsigned char	gComputerIntelligence;

int				gStatus;

void InitGame(void)
{
	int			i,j,k;
	AppFile		myFile;
	Str255		tempStr;
	
	HandleError(LoadDictionary());
	
	if (gHasColorQD)
		for (i=0; i<24; i++)
			gColorIcons[i]=GetCIcon(128+i);
	for (i=0; i<24; i++)
		gBWIcons[i]=GetIcon(128+i);
	for (i=0; i<24; i++)
	{
		GetIndString(tempStr, 130, i+1);
		gIconNames[i]=NewString(tempStr);
	}
	for (i=0; i<10; i++)
		gHumanName[i]=0L;
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		GetAppFiles(1, &myFile);
		MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &gameFile);
		HandleError(GetSavedGame(&gameFile));
		for (k=1; k<=j; k++)
			ClrAppFiles(k);
	}
}

void NewGame(void)
{
	InitLoadSave();
	if (GetHumanPlayers())
	{
		MakeComputerPlayers();
		OrderPlayers();
		if (gNumPlayers<2)
		{
			DoSound(sound_male_shoot, TRUE);
			PositionDialog('ALRT', generalAlert);
			ParamText("\pThere must be at least two players to play Ghost.","\p","\p","\p");
			StopAlert(generalAlert, 0L);
			return;
		}
		gCurrentPlayer=-1;
		gActualComputerPlayers=gNumComputerPlayers;
		gActualHumanPlayers=gNumHumanPlayers;
		gTheWord[0]=0x00;
		gInProgress=TRUE;
		StartGame();
	}
}

void StartGame(void)
{
	StartTiming();
	oldStartPtr=startPtr=listPtr=0L;
	gTheMessage[0]=0x00;
	gStatus=kNoStatus;
	OpenMainWindow();
	AdjustMenus();
	NextPlayer();
}

void GameUndo(void)
{
}

void GameEvent(void)
{
}

void GameKeyEvent(char charPressed)
{
	ObscureCursor();
	if ((gInProgress) && (gPlayOrderIndex[gCurrentPlayer]<gNumHumanPlayers))
	{
		charPressed&=0xdf;
		if ((charPressed>='A') && (charPressed<='Z'))
			AddLetter(charPressed);
	}
}

void CheckForComputerPlayer(void)
{
	if (gInProgress)
		if (gPlayOrderIndex[gCurrentPlayer]>=gNumHumanPlayers)
			DoComputerPlayer();	
}

void ShutDownGame(void)
{
	int			i;
	
	DisposeDictionary();
	if (gHasColorQD)
		for (i=0; i<24; i++)
			DisposeCIcon(gColorIcons[i]);
	for (i=0; i<24; i++)
		ReleaseResource(gBWIcons[i]);
	for (i=0; i<24; i++)
		DisposeHandle(gIconNames[i]);
}
