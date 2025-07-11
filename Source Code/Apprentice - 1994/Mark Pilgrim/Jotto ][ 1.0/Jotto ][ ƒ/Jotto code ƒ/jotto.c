/**********************************************************************\

File:		jotto.c

Purpose:	This module handles Jotto-specific initialization/shutdown
			and event handling.


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
#include "jotto.h"
#include "jotto meat.h"
#include "jotto keys.h"
#include "jotto graphics.h"
#include "jotto load-save.h"
#include "jotto files.h"
#include "jotto dictionary.h"
#include "jotto error.h"
#include "msg graphics.h"
#include "msg environment.h"
#include "msg menus.h"

char			gComputerWord[6];		/* what it really is */
char			gHumanWord[MAX_TRIES+1][6];	/* n-th word guess */
char			gNumRight[MAX_TRIES];	/* # right in n-th try */
unsigned char	gNumTries;				/* # tries so far */
int				gNumComputerWords[2];	/* # total words in computer (5- or 6-letter) dict */
int				gNumHumanWords[2];		/* # total words in human (5- or 6-letter) dict */
unsigned char	gWhichChar;				/* which char in current word the cursor is at */
unsigned char	gNumLetters;			/* 5 or 6 */
char			gAllowDup;				/* allow duplicate letters */
char			gNonWordsCount;			/* non-words count against you */
char			gAnimation;				/* animation? */

void InitGame(void)
{
	int			i,j,k;
	AppFile		myFile;
	
	InitLoadSave();
	gWhichWipe=0;
	gIsEndGame=FALSE;
	InitJottoGraphics();
	HandleError(OpenTheFiles());

	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		GetAppFiles(1, &myFile);
		MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &gameFile);
		GetSavedGame(&gameFile);
		for (k=1; k<=j; k++)
			ClrAppFiles(k);
	}
}

void NewGame(void)
{
	int				resultCode;
	int				i;
	
	gameFile.name[0]=0x00;
	gNumTries=0;
	NewWord();
	for (i=0; i<MAX_TRIES; i++)
		gNumRight[i]=0;
	
	resultCode=GetComputerWord();
	if (resultCode!=allsWell)
		HandleError(resultCode);
	else
	{
		gInProgress=TRUE;
		StartGame();
	}
}

void StartGame(void)
{
	gWhichWipe=gLastWipe+1;
	if (gWhichWipe>NUMWIPES)
		gWhichWipe=1;
	OpenMainWindow();
	ObscureCursor();
	AdjustMenus();
}

void GameUndo(void)
{
}

void GameEvent(void)
{
	Point		mouseLoc;
	Rect		theRect;
	int			i,target;
	
	if (gInProgress)
	{
		GetMouse(&mouseLoc);
		theRect.top=31;
		theRect.bottom=theRect.top+38;
		theRect.left=gWindowWidth/2-99-((gNumLetters==6) ? 20 : 0);
		theRect.right=theRect.left+38;
		target=-1;
		for (i=0; i<gNumLetters; i++)
		{
			if (PtInRect(mouseLoc, &theRect))
			{
				target=i;
				i=gNumLetters;
			}
			theRect.left+=40;
			theRect.right+=40;
		}
		
		if (target!=-1)
		{
			DrawOneChar();
			gWhichChar=target;
			HighlightChar();
		}
	}
}

void GameKeyEvent(char charPressed)
{
	if (gInProgress)
	{
		ObscureCursor();
		if ((charPressed==0x0d) || (charPressed==0x03))
			UserHitReturn();
		else if ((charPressed==0x1c) || (charPressed==0x08))
			UserHitLeftArrow(charPressed);
		else if (charPressed==0x1d)
			UserHitRightArrow();
		else if ((((charPressed&0xdf)>='A') && ((charPressed&0xdf)<='Z')) || (charPressed==' '))
			UserHitLetter(charPressed);
	}
}

void ShutDownGame(void)
{
	ShutdownJottoGraphics();
	CloseTheFiles();
}
