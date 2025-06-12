/**********************************************************************\

File:		ghost end.c

Purpose:	This module handles ending a player's turn (adding a
			letter), stepping to the next player, and ending a round
			(when appropriate).


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
#include "ghost end.h"
#include "ghost strategy.h"
#include "ghost challenge.h"
#include "msg graphics.h"
#include "msg dialogs.h"
#include "msg environment.h"
#include "msg timing.h"
#include "msg sounds.h"

void AddLetter(char theChar)
{
	if (theChar==' ')
	{
		TimeCorrection(1+30*gGameSpeed);
		StartTiming();
		ChallengeWord();
	}
	else
	{
		gTheWord[++gTheWord[0]]=theChar;
		TimeCorrection(1+30*gGameSpeed);
		StartTiming();
		DoSound(sound_play_letter, TRUE);
		gStatus=kNewPlayer;
		UpdateBoard();
		gStatus=kNoStatus;
		NextPlayer();
	}
}

void NextPlayer(void)
{
	FindStartPtr(TRUE);
	if (gTheWord[0]>=0x04)
	{
		GetNextWord(FALSE);
		if (thisWord[0]==gTheWord[0])
			EndRound();
	}
	
	GoToNextPlayer();
	
	UpdateBoard();
}

void GoToNextPlayer(void)
{
	Boolean			keepgoing;
	
	do
	{
		gCurrentPlayer++;
		if (gCurrentPlayer==gNumPlayers)
			gCurrentPlayer=0;
		if (gPlayOrderIndex[gCurrentPlayer]>=gNumHumanPlayers)
			keepgoing=(gComputerPlayerScore[gPlayOrderIndex[gCurrentPlayer]-gNumHumanPlayers]==5);
		else
			keepgoing=(gHumanPlayerScore[gPlayOrderIndex[gCurrentPlayer]]==5);
	}
	while (keepgoing);
}

void GoToPreviousPlayer(void)
{
	Boolean			keepgoing;
	
	do
	{
		gCurrentPlayer--;
		if (gCurrentPlayer<0)
			gCurrentPlayer=gNumPlayers-1;
		if (gPlayOrderIndex[gCurrentPlayer]>=gNumHumanPlayers)
			keepgoing=(gComputerPlayerScore[gPlayOrderIndex[gCurrentPlayer]-gNumHumanPlayers]==5);
		else
			keepgoing=(gHumanPlayerScore[gPlayOrderIndex[gCurrentPlayer]]==5);
	}
	while (keepgoing);
}

void EndRound(void)
{
	int				index;
	
	index=gPlayOrderIndex[gCurrentPlayer];
	if (index>=gNumHumanPlayers)
		gComputerPlayerScore[index-gNumHumanPlayers]++;
	else
		gHumanPlayerScore[gPlayOrderIndex[gCurrentPlayer]]++;
	
	gStatus=kJustGotALetter;
	UpdateBoard();
	gStatus=kNoStatus;
	
	if (gPlayOrderIndex[gCurrentPlayer]>=gNumHumanPlayers)
		TimeCorrection(1+30*gGameSpeed);
	StartTiming();

	if (gPlayOrderIndex[gCurrentPlayer]>=gNumHumanPlayers)
	{
		if (gComputerPlayerScore[gPlayOrderIndex[gCurrentPlayer]-gNumHumanPlayers]==5)
			ComputerLoses();
	}
	else
	{
		if (gHumanPlayerScore[gPlayOrderIndex[gCurrentPlayer]]==5)
			HumanLoses();
	}
	
	if (gInProgress)
	{
		if (gPlayOrderIndex[gCurrentPlayer]<gNumHumanPlayers)
		{
			TimeCorrection(1+30*gGameSpeed);
			StartTiming();
		}
		gTheWord[0]=0x00;
		oldStartPtr=startPtr=listPtr=0L;
		gCurrentPlayer--;
	}
}

void HumanLoses(void)
{
	gStatus=kJustLost;
	UpdateBoard();
	gStatus=kNoStatus;
	
	TimeCorrection(1+30*gGameSpeed);
	StartTiming();

	gActualHumanPlayers--;
	if (gActualHumanPlayers+gActualComputerPlayers==1)
	{
		DoSound(sound_joy, TRUE);
		gInProgress=FALSE;
	}
	else DoSound(sound_raz, FALSE);
}

void ComputerLoses(void)
{
	int				iconIndex;
	
	gStatus=kJustLost;
	UpdateBoard();
	gStatus=kNoStatus;
	
	gActualComputerPlayers--;
	if (gActualHumanPlayers+gActualComputerPlayers==1)
	{
		DoSound(sound_joy, TRUE);
		gInProgress=FALSE;
	}
	else
	{
		iconIndex=gComputerIconIndex[gPlayOrderIndex[gCurrentPlayer]-gNumHumanPlayers];
		if ((iconIndex==0) || (iconIndex==2) || (iconIndex==22))
			DoSound(sound_female_shoot, FALSE);
		else
			DoSound(sound_male_shoot, FALSE);
	}

	TimeCorrection(1+30*gGameSpeed);
	StartTiming();
}
