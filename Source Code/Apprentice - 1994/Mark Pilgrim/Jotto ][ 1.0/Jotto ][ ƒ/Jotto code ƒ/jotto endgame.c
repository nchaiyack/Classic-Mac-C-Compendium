/**********************************************************************\

File:		jotto endgame.c

Purpose:	This module handles the win-game and lose-game sequences.


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
#include "jotto endgame.h"
#include "jotto graphics.h"
#include "msg environment.h"
#include "msg menus.h"
#include "msg sounds.h"
#include "msg dialogs.h"
#include "msg graphics.h"
#include "util.h"

Boolean			gIsEndGame;

void WinGame(void)
{
	
	HighlightChar();
	gIsEndGame=TRUE;
	UpdateBoard();
	gIsEndGame=FALSE;
	DoSound(sound_wingame);
	while (!Button());
	while (Button());
	FlushEvents(mDownMask+mUpMask+keyDownMask+keyUpMask+autoKeyMask, 0);
	gInProgress=FALSE;
	UpdateBoard();
	AdjustMenus();
}

void LoseGame(void)
{
	Str255			tempStr;
	
	HighlightChar();
	DoSound(sound_losegame);
	PositionDialog('ALRT', loseAlert);
	Mymemcpy((Ptr)gHumanWord[gNumTries], gComputerWord, gNumLetters);
	Mymemcpy((Ptr)((long)tempStr+1), (Ptr)gComputerWord, gNumLetters);
	tempStr[0]=gNumLetters;
	ParamText(tempStr,"\p","\p","\p");
	Alert(loseAlert, 0L);
	gInProgress=FALSE;
	UpdateBoard();
	AdjustMenus();
	DrawMenuBar();
}
