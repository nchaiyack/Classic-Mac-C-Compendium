/**********************************************************************\

File:		jotto keys.c

Purpose:	This module handles all the different things that can
			happen when you press different keys during a game --
			that includes playing a guessed word, typing letters
			or arrow keys.


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
#include "jotto keys.h"
#include "jotto meat.h"
#include "jotto graphics.h"
#include "jotto endgame.h"
#include "jotto dictionary.h"
#include "msg graphics.h"
#include "msg sounds.h"
#include "msg dialogs.h"
#include "util.h"

void UserHitReturn(void)
{
	int				i;
	
	for (i=0; i<gNumLetters; i++)
		if ((gHumanWord[gNumTries][i]<'A') || (gHumanWord[gNumTries][i]>'Z'))
			return;

	if (ValidWord(gHumanWord[gNumTries]))
		UserGuessedValidWord();
	else
		UserGuessedInvalidWord();
}

void UserGuessedValidWord(void)
{
	CalculateNumRight();
	DrawWordInList();
	if (gNumRight[gNumTries]==gNumLetters)
		WinGame();
	else
	{
		gNumTries++;
		if (gNumTries!=MAX_TRIES)
		{
			DoSound(sound_playword_good);
			NewWord();
			SetPort(gMainWindow);
			DrawCurrentWord();
			HighlightChar();
		}
		else LoseGame();
	}
}

void UserGuessedInvalidWord(void)
{
	int			alertResult;
	Str255		tempStr;
	
	if ((gNumLetters==5) ? (gFiveLetterCustomSaveOK) : (gSixLetterCustomSaveOK))
		DoSound(sound_badword);
	Mymemcpy((Ptr)((long)tempStr+1), (Ptr)gHumanWord[gNumTries], gNumLetters);
	tempStr[0]=gNumLetters;
	ParamText(tempStr, "\p", "\p", "\p");
	if ((gNumLetters==5) ? (gFiveLetterCustomSaveOK) : (gSixLetterCustomSaveOK))
	{
		PositionDialog('ALRT', badWordAlert);
		alertResult=NoteAlert(badWordAlert, 0L);
		if (alertResult==1)
		{
			DoSound(sound_bluffing);
			AdmitBluffing();
		}
		else
			LearnWord();
	}
	else
	{
		DoSound(sound_bluffing);
		PositionDialog('ALRT', badWordNoCustomAlert);
		NoteAlert(badWordNoCustomAlert, 0L);
		AdmitBluffing();
	}
}

void AdmitBluffing(void)
{
	if (gNonWordsCount)
		gNumRight[gNumTries]=-1;
	
	UpdateBoard();
	
	if (gNonWordsCount)
	{
		DrawWordInList();
		gNumTries++;
	}
	
	if (gNumTries!=MAX_TRIES)
	{
		NewWord();
		SetPort(gMainWindow);
		DrawCurrentWord();
		HighlightChar();
	}
	else
		LoseGame();
}

void LearnWord(void)
{
	SaveCustomWordToDisk(gHumanWord[gNumTries]);
	UpdateBoard();
	UserGuessedValidWord();
}

void UserHitLeftArrow(char charPressed)
{
	HighlightChar();
	if (gWhichChar!=0x00)
		gWhichChar--;
	else if (charPressed==0x1c)
		gWhichChar=gNumLetters-1;
	HighlightChar();
}

void UserHitRightArrow(void)
{
	HighlightChar();
	gWhichChar++;
	if (gWhichChar==gNumLetters)
		gWhichChar=0x00;
	HighlightChar();
}

void UserHitLetter(char charPressed)
{
	DoSound(sound_keyclick);
	gHumanWord[gNumTries][gWhichChar]=charPressed&0xdf;
	DrawOneChar();
	if ((gWhichChar!=gNumLetters-1) || (charPressed==' '))
	{
		gWhichChar++;
		if (gWhichChar==gNumLetters)
			gWhichChar=0x00;
	}
	HighlightChar();
}
