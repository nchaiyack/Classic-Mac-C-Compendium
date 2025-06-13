/**********************************************************************\

File:		halma endgame.c

Purpose:	This module handles drawing the endgame board.

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

#include "halma endgame.h"
#include "halma meat.h"
#include "program globals.h"
#include "sounds.h"
#include "graphics.h"

static	Boolean				gShowingEndGame;
static	PicHandle			gCongratsColorPict, gCongratsBWPict;
static	PicHandle			gWinnerColorPict, gWinnerBWPict;

enum
{
	congratsColorID=400,
	congratsBWID,
	winnerColorID,
	winnerBWID
};

Boolean GameOverQQ(void)
{
	short			theRow, theColumn;
	
	for (theRow=0; theRow<3; theRow++)
	{
		for (theColumn=gNumColumns-3; theColumn<gNumColumns; theColumn++)
		{
			if (Board[theRow][theColumn]==kNoPiece)
				return FALSE;
		}
	}
	
	return TRUE;
}

void CheckEndGame(WindowDataHandle theData, short theRow, short theColumn)
{
	if (GameOverQQ())
	{
		DoAlreadyHighlighted(theData, theRow, theColumn);
		gShowingEndGame=TRUE;
		DoSound(sound_endgame, TRUE);
		(**theData).offscreenNeedsUpdate=TRUE;
		UpdateTheWindow((ExtendedWindowDataHandle)theData);
	}
}

Boolean ShowingEndGameQQ(void)
{
	return gShowingEndGame;
}

void DontShowEndGame(WindowDataHandle theData)
{
	gShowingEndGame=FALSE;
	(**theData).offscreenNeedsUpdate=TRUE;
	UpdateTheWindow((ExtendedWindowDataHandle)theData);
}

void InitTheEndGame(void)
{
	gCongratsColorPict=gCongratsBWPict=gWinnerColorPict=gWinnerBWPict=0L;
	gShowingEndGame=FALSE;
}

void ShutDownTheEndGame(void)
{
	gCongratsColorPict=ReleaseThePict(gCongratsColorPict);
	gCongratsBWPict=ReleaseThePict(gCongratsBWPict);
	gWinnerColorPict=ReleaseThePict(gWinnerColorPict);
	gWinnerBWPict=ReleaseThePict(gWinnerBWPict);
}

void EndTheGame(WindowDataHandle theData, int theDepth)
{
	GrafPtr			curPort;
	short			theX, theY;
	Boolean			isColor;
	short			best;
	
	isColor=(theDepth>2);
	GetPort(&curPort);
	theX=gNumColumns*15-42;
	theY=(curPort->portRect.bottom-curPort->portRect.top)/3;
	best=BestSolution();
	if ((gNumMoves<=best) && (best>0))
	{
		if (isColor)
			gWinnerColorPict=DrawThePicture(gWinnerColorPict, winnerColorID, theX, theY);
		else
			gWinnerBWPict=DrawThePicture(gWinnerBWPict, winnerBWID, theX, theY);
	}
	else
	{
		if (isColor)
			gCongratsColorPict=DrawThePicture(gCongratsColorPict, congratsColorID, theX, theY);
		else
			gCongratsBWPict=DrawThePicture(gCongratsBWPict, congratsBWID, theX, theY);
	}
}
