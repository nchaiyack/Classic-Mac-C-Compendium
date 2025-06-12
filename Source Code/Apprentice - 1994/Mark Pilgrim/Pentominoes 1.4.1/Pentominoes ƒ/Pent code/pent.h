/**********************************************************************\

File:		pent.h

Purpose:	This is the header file for pent.c


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

extern	int				gWhichBoard;
extern	int				gNumHilited;
extern	int				Board[8][20];
extern	int				gNumRows;
extern	int				gNumCols;
extern	Boolean			gShowGrid;
extern	Boolean			PieceUsed[12];
extern	int				gNumPlayed;
extern	int				PiecesPlayed[12];
extern	CIconHandle		gColorIcons[12];
extern	Handle			gBWIcons[12];
extern	PixPatHandle	gPlainColors[15];
extern	PatHandle		gBWPatterns[15];
extern	int				gShapes[12][5][5];
extern	char			SolutionLetters[14];
extern	int				gCurrentColor;
extern	Boolean			freshClick;

void InitGame(void);
void NewGame(void);
void ShutDownGame(void);
void WinGame(void);
void GameEvent(void);
void GameKeyEvent(char);
void GameUndo(void);
