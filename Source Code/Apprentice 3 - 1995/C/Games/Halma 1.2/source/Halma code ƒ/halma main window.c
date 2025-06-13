/**********************************************************************\

File:		halma main window.c

Purpose:	This module handles initializing/drawing/dealing with the
			main window.

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

#include "halma main window.h"
#include "halma meat.h"
#include "halma snow.h"
#include "halma endgame.h"
#include "buttons.h"
#include "environment.h"
#include "graphics.h"
#include "util.h"
#include "program globals.h"
#include "sounds.h"

/* internal procedures for halma main window.c only */
void SetupTheMainWindow(WindowDataHandle theData);
void ShutDownTheMainWindow(WindowDataHandle theData);
void InitializeTheMainWindow(WindowDataHandle theData);
void OpenTheMainWindow(WindowDataHandle theData);
void CloseTheMainWindow(WindowDataHandle theData);
void IdleInMainWindow(WindowDataHandle theData);
void KeyPressedInMainWindow(WindowDataHandle theData, unsigned char theChar);
void MouseClickedInMainWindow(WindowDataHandle theData, Point thePoint);
void DisposeTheMainWindow(WindowDataHandle theData);
void ActivateTheMainWindow(WindowDataHandle theData);
void DeactivateTheMainWindow(WindowDataHandle theData);
void DrawTheMainWindow(WindowDataHandle theData, short theDepth);

static	short			gOldForegroundTime;		/* stored foreground wait time */
static	short			gDisplayedScore;

CIconHandle		gPieceIconColor;
Handle			gPieceIconBW;
Rect			gButtonRect[NUM_BUTTONS];

short MainWindowDispatch(WindowDataHandle theData, short theMessage,
	unsigned long misc)
{
	short			theDepth;
	unsigned char	theChar;
	Point			thePoint;
	GrafPtr			curPort;
	
	switch (theMessage)
	{
		case kNull:
			IdleInMainWindow(theData);
			return kSuccess;
			break;
		case kUpdate:
			theDepth=misc&0x7fff;
			DrawTheMainWindow(theData, theDepth);
			return kSuccess;
			break;
		case kInitialize:
			InitializeTheMainWindow(theData);
			return kFailure;
		case kOpen:
			OpenTheMainWindow(theData);
			return kSuccess;
			break;
		case kClose:
			CloseTheMainWindow(theData);
			return kSuccess;
			break;
		case kKeydown:
			theChar=misc&charCodeMask;
			KeyPressedInMainWindow(theData, theChar);
			return kSuccess;
			break;
		case kMousedown:
 			thePoint.h=(misc>>16)&0x7fff;
 			thePoint.v=misc&0x7fff;
 			MouseClickedInMainWindow(theData, thePoint);
 			return kSuccess;
 			break;
 		case kActivate:
 			ActivateTheMainWindow(theData);
 			return kSuccess;
 			break;
 		case kDeactivate:
 			DeactivateTheMainWindow(theData);
 			return kSuccess;
 			break;
		case kStartup:
			SetupTheMainWindow(theData);
			return kSuccess;
			break;
		case kDispose:
			DisposeTheMainWindow(theData);
			return kSuccess;
			break;
		case kShutdown:
			ShutDownTheMainWindow(theData);
			return kSuccess;
			break;
		case kUndo:
			if (!ShowingEndGameQQ())
			{
				if (gThisJumpString[0]>0x00)
					UndoOneJump(theData);
				else
					UndoWholeMove(theData);
			}
			return kSuccess;
			break;
		case kChangeDepth:
			gDisplayedScore=-1;
			ShutDownTheSnow();
			InitTheSnow();
			GetPort(&curPort);
			FillRect(&(curPort->portRect), black);
			break;
	}
	
	return kFailure;
}

void SetupTheMainWindow(WindowDataHandle theData)
{
	unsigned char	*titleStr="\pHalma";
	short			row, col;
	
	(**theData).maxDepth=8;
	(**theData).windowType=noGrowDocProc;
	(**theData).hasCloseBox=TRUE;
	Mymemcpy((Ptr)((**theData).windowTitle), (Ptr)titleStr, titleStr[0]+1);
	
	if (gHasColorQD)
		gPieceIconColor=GetCIcon(128);
	gPieceIconBW=GetIcon(128);
	
	for (row=0; row<9; row++)
	{
		for (col=0; col<9; col++)
		{
			SetRect(&gButtonRect[row*9+col], 10+col*30, 10+row*30, 40+col*30, 40+row*30);
		}
	}
}

void ShutDownTheMainWindow(WindowDataHandle theData)
{
	if (gHasColorQD)
		DisposeCIcon(gPieceIconColor);
	ReleaseResource(gPieceIconBW);
}

void InitializeTheMainWindow(WindowDataHandle theData)
{
	(**theData).windowWidth=155+gNumColumns*30;
	(**theData).windowHeight=20+gNumRows*30;
	gDisplayedScore=-1;
	GetDateTime((unsigned long*)&randSeed);
	InitTheSnow();
	InitTheEndGame();
	DoSound(sound_startgame, TRUE);
}

void OpenTheMainWindow(WindowDataHandle theData)
{
	GrafPtr			curPort;
	
	(**theData).offscreenNeedsUpdate=TRUE;
	if (SetPortToOffscreen(theData))
	{
		GetPort(&curPort);
		FillRect(&(curPort->portRect), black);
		RestorePortToScreen(theData);
	}
}

void CloseTheMainWindow(WindowDataHandle theData)
{
	ShutDownTheSnow();
}

void IdleInMainWindow(WindowDataHandle theData)
{
	SnowIdle(theData);
}

void KeyPressedInMainWindow(WindowDataHandle theData, unsigned char theChar)
{
	short			theRow, theColumn;
	
	if (ShowingEndGameQQ())
	{
		DontShowEndGame(theData);
		return;
	}
	
	if (GameOverQQ())
		return;
	
	if ((gStickyButtonRow==-1) && (gStickyButtonColumn==-1))
		return;
		
	theRow=theColumn=-1;
	if ((theChar>='A') && (theChar<='Z'))
		theChar|=0x20;
	
	switch (theChar)
	{
		case 'u':
		case '7':
		case 'i':
		case '8':
		case 'o':
		case '9':
			theRow=gStickyButtonRow-2;
			break;
		case 'j':
		case '4':
		case 'l':
		case '6':
		case 'a':
		case 'd':
			theRow=gStickyButtonRow;
			break;
		case 'm':
		case ',':
		case '.':
		case '1':
		case '2':
		case '3':
			theRow=gStickyButtonRow+2;
			break;
		case 'q':
		case 'w':
		case 'e':
			theRow=gStickyButtonRow-1;
			break;
		case 'z':
		case 'x':
		case 'c':
			theRow=gStickyButtonRow+1;
			break;
	}
	switch (theChar)
	{
		case 'u':
		case 'j':
		case 'm':
		case '7':
		case '4':
		case '1':
			theColumn=gStickyButtonColumn-2;
			break;
		case 'i':
		case ',':
		case '8':
		case '2':
		case 'w':
		case 'x':
			theColumn=gStickyButtonColumn;
			break;
		case 'o':
		case 'l':
		case '.':
		case '9':
		case '6':
		case '3':
			theColumn=gStickyButtonColumn+2;
			break;
		case 'q':
		case 'a':
		case 'z':
			theColumn=gStickyButtonColumn-1;
			break;
		case 'e':
		case 'd':
		case 'c':
			theColumn=gStickyButtonColumn+1;
			break;
	}
	if ((theRow<0) || (theRow>=gNumRows) || (theColumn<0) || (theColumn>=gNumColumns))
		return;
	
	switch (GetMoveType(theRow, theColumn))
	{
		case kValidJump:
			DoValidJump(theData, theRow, theColumn);
			break;
		case kValidFirstMove:
			DoValidFirstMove(theData, theRow, theColumn, TRUE);
			break;
		case kValidSingleMove:
			DoValidSingleMove(theData, theRow, theColumn);
			break;
		case kValidSingleMoveNewMove:
			DoValidSingleMoveNewMove(theData, theRow, theColumn);
			break;
		case kOffBoard:
			DoOffBoard(theData);
			break;
		case kCantJumpThere:
			DoCantJumpThere(theData);
			break;
		case kMustJumpOverPiece:
			DoMustJumpOverPiece(theData);
			break;
		case kCantSelectNonPiece:
			DoCantSelectNonPiece(theData);
			break;
		case kSpaceNotEmpty:
			DoSpaceNotEmpty(theData, theRow, theColumn);
			break;
		case kAlreadyHighlighted:
			DoAlreadyHighlighted(theData, theRow, theColumn);
			break;
	}
	CheckEndGame(theData, theRow, theColumn);
}

void MouseClickedInMainWindow(WindowDataHandle theData, Point thePoint)
{
	short			theRow, theColumn;
	
	if (ShowingEndGameQQ())
	{
		DontShowEndGame(theData);
		return;
	}
	
	if (GameOverQQ())
		return;
	
	switch (GetMoveType(theRow=(thePoint.v-10)/30, theColumn=(thePoint.h-10)/30))
	{
		case kValidJump:
			DoValidJump(theData, theRow, theColumn);
			break;
		case kValidFirstMove:
			DoValidFirstMove(theData, theRow, theColumn, TRUE);
			break;
		case kValidSingleMove:
			DoValidSingleMove(theData, theRow, theColumn);
			break;
		case kValidSingleMoveNewMove:
			DoValidSingleMoveNewMove(theData, theRow, theColumn);
			break;
		case kOffBoard:
			DoOffBoard(theData);
			break;
		case kCantJumpThere:
			DoCantJumpThere(theData);
			break;
		case kMustJumpOverPiece:
			DoMustJumpOverPiece(theData);
			break;
		case kCantSelectNonPiece:
			DoCantSelectNonPiece(theData);
			break;
		case kSpaceNotEmpty:
			DoSpaceNotEmpty(theData, theRow, theColumn);
			break;
		case kAlreadyHighlighted:
			DoAlreadyHighlighted(theData, theRow, theColumn);
			break;
	}
	CheckEndGame(theData, theRow, theColumn);
}

void DisposeTheMainWindow(WindowDataHandle theData)
{
}

void ActivateTheMainWindow(WindowDataHandle theData)
{
	gOldForegroundTime=gForegroundWaitTime;
	gForegroundWaitTime=0;
}

void DeactivateTheMainWindow(WindowDataHandle theData)
{
	gForegroundWaitTime=gOldForegroundTime;
	GetRidOfSnowflake(theData);
}

void DrawTheMainWindow(WindowDataHandle theData, short theDepth)
{
	RGBColor		oldForeColor, oldBackColor;
	GrafPtr			curPort;
	short			i,j;
	Str31			theStr;
	Rect			tempRect;
	
	if (theDepth>2)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
	}
	
	GetPort(&curPort);
	tempRect=curPort->portRect;
	tempRect.bottom=10;
	FillRect(&tempRect, black);
	tempRect=curPort->portRect;
	tempRect.right=gNumColumns*30+20;
	FillRect(&tempRect, black);
	tempRect=curPort->portRect;
	tempRect.top=gNumRows*30-25;
	FillRect(&tempRect, black);
	MoveTo(gNumColumns*30+20, gNumRows*30-25);
	ForeColor(whiteColor);
	Line(125, 0);
	ForeColor(blackColor);
	TextFont(geneva);
	TextSize(9);
	TextMode(srcXor);
	MoveTo(gNumColumns*30+20, gNumRows*30-9);
	DrawString("\pSolution so far:");
	MoveTo(gNumColumns*30+20, gNumRows*30+6);
	DrawString("\pMinimal solution:");
	MoveTo(gNumColumns*30+101, gNumRows*30+6);
	i=BestSolution();
	if (i>0)
	{
		NumToString(BestSolution(), theStr);
		DrawString(theStr);
		DrawString("\p moves");
	}
	else
		DrawString("\punknown");
	
	gDisplayedScore=-1;
	DrawScoreAndStuff(theData, theDepth);
	
	if (ShowingEndGameQQ())
		EndTheGame(theData, theDepth);
	else
	{
		for (i=0; i<gNumRows; i++)
		{
			for (j=0; j<gNumColumns; j++)
			{
				switch (Board[i][j])
				{
					case kPiece:
						Draw3DButton(&gButtonRect[i*9+j], 0L, (**theData).windowDepth>2 ?
							(Handle)gPieceIconColor : gPieceIconBW, theDepth, FALSE, FALSE);
						break;
					case kHighlightedPiece:
						Draw3DButton(&gButtonRect[i*9+j], 0L, (**theData).windowDepth>2 ?
							(Handle)gPieceIconColor : gPieceIconBW, theDepth, TRUE, FALSE);
						break;
					default:
						Draw3DButton(&gButtonRect[i*9+j], 0L, 0L, theDepth, FALSE, FALSE);
						break;
				}
			}
		}
	}
	
	DrawScoreAndStuff(theData, theDepth);
	
	if (theDepth>2)
	{
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);
	}
}

void DrawScoreAndStuff(WindowDataHandle theData, short theDepth)
{
	Rect			tempRect;
	GrafPtr			curPort;
	Str255			ts;
	
	if (gNumMoves==gDisplayedScore)
		return;
	
	GetPort(&curPort);
	SetRect(&tempRect, gNumColumns*30+101, gNumRows*30-20, gNumColumns*30+125,
		gNumRows*30-9);
	MoveTo(gNumColumns*30+101, gNumRows*30-9);
	NumToString(gNumMoves, ts);
	FillRect(&tempRect, black);
	TextMode(srcXor);
	DrawString(ts);
	TextMode(srcCopy);
	gDisplayedScore=gNumMoves;
}
