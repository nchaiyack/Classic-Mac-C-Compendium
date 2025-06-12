/**********************************************************************\

File:		pent setup.c

Purpose:	This module handles calculations made at the beginning of
			each new game, to determine how large the main window is.


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

#include "pent setup.h"
#include "pent.h"
#include "pent load-save.h"
#include "msg sounds.h"
#include "msg graphics.h"
#include "msg menus.h"

void CalculateRowsandCols(void)
{
	switch (gWhichBoard)
	{
		case 1:
		case 2:
		case 3:
		case 4:
		case 5:
		case 10:
		case 11:
		case 12:
		case 13:
			gNumRows=gNumCols=8;
			break;
		case 6:
			gNumRows=6;
			gNumCols=10;
			break;
		case 7:
			gNumRows=5;
			gNumCols=12;
			break;
		case 8:
			gNumRows=4;
			gNumCols=15;
			break;
		case 9:
			gNumRows=3;
			gNumCols=20;
			break;
		case 14:
		case 15:
		case 16:
		case 17:
		case 18:
		case 19:
		case 20:
		case 21:
		case 22:
		case 23:
		case 24:
		case 25:
		case 30:
			gNumRows=5;
			gNumCols=13;
			break;
		case 26:
		case 27:
			gNumRows=8;
			gNumCols=9;
			break;
		case 28:
			gNumRows=7;
			gNumCols=10;
			break;
		case 29:
			gNumRows=6;
			gNumCols=15;
			break;
		case 31:
			gNumRows=4;
			gNumCols=16;
			break;
	}
}

void InitGameVariables(void)
{
	int			i,j;
	
	gWindowWidth = (gNumCols*25) + 10;
	gWindowHeight = (gNumRows*25) + 95;
	
	for (i=0; i<gNumRows; i++)
		for (j=0; j<gNumCols; j++)
			Board[i][j] = -1;
	
	for (i=0; i<12; i++)
		PieceUsed[i] = FALSE;
	
	switch (gWhichBoard)
	{
		case 2:
			Board[0][0]=Board[0][7]=Board[7][0]=Board[7][7]=12;
			break;
		case 3:
			Board[3][3]=Board[4][3]=Board[3][4]=Board[4][4]=12;
			break;
		case 4:
			Board[1][1]=Board[6][6]=Board[1][6]=Board[6][1]=12;
			break;
		case 5:
			Board[2][3]=Board[3][5]=Board[4][2]=Board[5][4]=12;
			break;
		case 10:
			Board[7][7]=Board[7][6]=Board[6][7]=Board[6][6]=12;
			break;
		case 11:
			Board[3][5]=Board[3][6]=Board[4][5]=Board[4][6]=12;
			break;
		case 12:
			Board[0][4]=Board[3][0]=Board[4][7]=Board[7][3]=12;
			break;
		case 13:
			Board[2][2]=Board[2][5]=Board[5][2]=Board[5][5]=12;
			break;
		case 14:
			Board[0][6]=Board[1][6]=Board[2][6]=Board[3][6]=Board[4][6]=12;
			break;
		case 15:
			Board[0][6]=Board[1][6]=Board[2][6]=Board[3][6]=Board[3][7]=12;
			break;
		case 16:
			Board[1][5]=Board[2][5]=Board[3][5]=Board[3][6]=Board[3][7]=12;
			break;
		case 17:
			Board[1][6]=Board[2][5]=Board[2][6]=Board[2][7]=Board[3][6]=12;
			break;
		case 18:
			Board[1][6]=Board[1][7]=Board[2][6]=Board[2][7]=Board[3][6]=12;
			break;
		case 19:
			Board[1][5]=Board[2][5]=Board[2][6]=Board[3][6]=Board[3][7]=12;
			break;
		case 20:
			Board[1][6]=Board[1][7]=Board[2][6]=Board[3][6]=Board[3][7]=12;
			break;
		case 21:
			Board[1][6]=Board[1][7]=Board[2][6]=Board[3][5]=Board[3][6]=12;
			break;
		case 22:
			Board[0][6]=Board[1][6]=Board[2][6]=Board[2][7]=Board[3][6]=12;
			break;
		case 23:
			Board[1][6]=Board[1][7]=Board[2][5]=Board[2][6]=Board[3][6]=12;
			break;
		case 24:
			Board[1][5]=Board[1][6]=Board[1][7]=Board[2][6]=Board[3][6]=12;
			break;
		case 25:
			Board[0][6]=Board[1][6]=Board[2][6]=Board[2][7]=Board[3][7]=12;
			break;
		case 26:
			for (i=2; i<=5; i++)
				for (j=3; j<=5; j++)
					Board[i][j]=12;
			break;
		case 27:
			Board[3][3]=Board[3][5]=Board[4][3]=Board[4][5]=12;
			for (i=0; i<=7; i++)
				Board[i][4]=12;
			break;
		case 28:
			for (i=1; i<=5; i++)
				Board[i][0]=Board[i][9]=12;
			break;
		case 29:
			for (i=1; i<=5; i++)
				for (j=0; j<i; j++)
					Board[i][j]=Board[5-i][14-j]=12;
			break;
		case 30:
			for (i=0; i<5; i++)
				Board[i][3]=12;
			break;
		case 31:
			for (i=0; i<4; i++)
				Board[i][5]=12;
			break;
	}
	
	gNumHilited = 0;
	gNumPlayed = 0;
	gameFile.name[0]=0x00;
}

void OpenNewGame(void)
{
	gCurrentColor=-2;
	DoSound(sound_startgame);
	OpenMainWindow();
	AdjustMenus();
	DrawMenuBar();
}
