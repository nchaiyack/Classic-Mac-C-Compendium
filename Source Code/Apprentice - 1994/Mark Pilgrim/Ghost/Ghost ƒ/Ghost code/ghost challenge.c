/**********************************************************************\

File:		ghost challenge.c

Purpose:	This module handles one player challenging another --
			look up the word and see who's right.


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
#include "ghost challenge.h"
#include "ghost strategy.h"
#include "ghost end.h"
#include "msg graphics.h"
#include "msg timing.h"
#include "msg sounds.h"

void ChallengeWord(void)
{
	int				iconIndex;
	Boolean			isFemale;
	
	gStatus=kIsChallenging;
	UpdateBoard();
	gStatus=kNoStatus;
	
	StartTiming();
	if (FindOneOccurrence())
	{
		BlockMove(thisWord, gTheWord, 256);
		TimeCorrection(1+30*gGameSpeed);
		StartTiming();
		gStatus=kLostChallenge;
		UpdateBoard();
		gStatus=kNoStatus;
		DoSound(sound_raz, FALSE);
		TimeCorrection(60+30*gGameSpeed);
		StartTiming();
		NextPlayer();
	}
	else
	{
		TimeCorrection(1+30*gGameSpeed);
		StartTiming();
		gStatus=kWonChallenge;
		UpdateBoard();
		gStatus=kNoStatus;
		TimeCorrection(1+30*gGameSpeed);
		if (gPlayOrderIndex[gCurrentPlayer]>=gNumHumanPlayers)
		{
			iconIndex=gComputerIconIndex[gPlayOrderIndex[gCurrentPlayer]-gNumHumanPlayers];
			isFemale=((iconIndex==0) || (iconIndex==2) || (iconIndex==22));
		}
		else isFemale=FALSE;
		
		if (isFemale)
			DoSound(sound_female_drat, FALSE);
		else
			DoSound(sound_male_drat, FALSE);
		
		StartTiming();
		GoToPreviousPlayer();
		EndRound();
		NextPlayer();
	}
}
