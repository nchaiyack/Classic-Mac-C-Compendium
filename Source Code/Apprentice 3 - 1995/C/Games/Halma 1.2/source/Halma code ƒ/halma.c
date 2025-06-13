/**********************************************************************\

File:		halma.c

Purpose:	This module handles demo initialization/shutdown and a
			dispatch for the graphic effects and fades.

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

#include "halma.h"
#include "program globals.h"
#include "graphics.h"
#include "file interface.h"
#include "error.h"
#include "halma load-save.h"
#include "halma endgame.h"
#include "halma main window.h"
#include "halma board size.h"
#include "halma meat.h"

short			gNumRows;
short			gNumColumns;
short			Board[9][9];
short			gNumMoves;
short			gNumJumps;
short			gThisStartRow, gThisStartColumn;
Str255			gThisJumpString;
unsigned char	**gTheFullJumpHandle;

void InitTheProgram(void)
{
	short			i,j,k;
	AppFile			myFile;
	
	SetIndDispatchProc(kBoardSize, BoardSizeWindowDispatch);
	SetIndDispatchProc(kMainWindow, MainWindowDispatch);
	CallIndDispatchProc(kBoardSize, kStartup, 0L);
	CallIndDispatchProc(kMainWindow, kStartup, 0L);
	
	gTheFullJumpHandle=(unsigned char**)NewHandle(0L);
	InitTheEndGame();
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		GetAppFiles(1, &myFile);
		MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &gameFile);
		HandleError(GetSavedGame(&gameFile), FALSE);
		for (k=1; k<=j; k++)
			ClrAppFiles(k);
	}
}

void NewGame(void)
{
	short			i,j;
	
// init game globals here, stuff that would be saved to disk in saved game
	for (i=0; i<9; i++)
		for (j=0; j<9; j++)
			Board[i][j]=kNoPiece;
	for (i=gNumRows-1; i>gNumRows-4; i--)
		for (j=0; j<3; j++)
			Board[i][j]=kPiece;
	gNumMoves=gNumJumps=0;
	gThisJumpString[0]=0x00;
	gStickyButtonRow=gStickyButtonColumn=-1;
	InitLoadSave();
	StartGame();
}

void StartGame(void)
{
// init game globals here, stuff that always needs to be initted
	if (GetIndWindowGrafPtr(kBoardSize))
		CloseTheIndWindow(kBoardSize);
	OpenTheIndWindow(kMainWindow);
}

void ShutDownTheProgram(void)
{
	ShutDownTheEndGame();
	DisposeHandle((Handle)gTheFullJumpHandle);
	CallIndDispatchProc(kBoardSize, kShutdown, 0L);
	CallIndDispatchProc(kMainWindow, kShutdown, 0L);
}
