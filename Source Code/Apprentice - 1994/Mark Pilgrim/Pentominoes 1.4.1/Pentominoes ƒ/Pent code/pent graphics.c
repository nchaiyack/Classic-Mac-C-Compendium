/**********************************************************************\

File:		pent graphics.c

Purpose:	This module handles drawing the playing board in a game.


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

#include "pent graphics.h"
#include "pent.h"
#include "msg graphics.h"

void DrawBoardColor(void)
{
	RGBColor	oldForeColor, oldBackColor;
	GrafPtr		curPort;
	Rect		boardRect;
	int			i,j;
	
	GetForeColor(&oldForeColor);
	GetBackColor(&oldBackColor);
	
	GetPort(&curPort);
	
	EraseRect(&(curPort->portRect));

	SetRect(&boardRect,5,gWindowHeight-(gNumRows*25)-5,gWindowWidth-4,gWindowHeight-4);
	FrameRect(&boardRect);
	
	for (i=1; i<gNumCols; i++)
	{
		MoveTo(boardRect.left+i*25,boardRect.top);
		Line(0,gNumRows*25-1);
	}
	
	for (i=1; i<gNumRows; i++)
	{
		MoveTo(boardRect.left,boardRect.top+i*25);
		Line(gNumCols*25-1,0);
	}
	
	for (i=0; i<6; i++)
	{
		boardRect.top=10;
		boardRect.bottom=42;
		boardRect.left=(gWindowWidth/2)-((3-i)*34)+1;
		boardRect.right=boardRect.left+32;
		if (!PieceUsed[i])
			PlotCIcon(&boardRect, gColorIcons[i]);
		
		boardRect.top+=40;
		boardRect.bottom+=40;
		if (!PieceUsed[i+6])
			PlotCIcon(&boardRect, gColorIcons[i+6]);
	}

	boardRect.bottom=gWindowHeight+20-(gNumRows*25);
	boardRect.top=boardRect.bottom-24;
	
	for (i=0; i<gNumRows; i++)
	{
		boardRect.left=6;
		boardRect.right=30;
		for (j=0; j<gNumCols; j++)
		{
			FillCRect(&boardRect,gPlainColors[Board[i][j]+2]);
			boardRect.left+=25;
			boardRect.right+=25;
		}
		boardRect.top+=25;
		boardRect.bottom+=25;
	}
	
	RGBForeColor(&oldForeColor);
	RGBBackColor(&oldBackColor);
}

void DrawBoardBW(void)
{
	int			i,j;
	Rect		boardRect;
	GrafPtr		curPort;
	
	GetPort(&curPort);
	
	EraseRect(&(curPort->portRect));

	SetRect(&boardRect,5,gWindowHeight-(gNumRows*25)-5,gWindowWidth-4,gWindowHeight-4);
	FrameRect(&boardRect);
	
	for (i=1; i<gNumCols; i++)
	{
		MoveTo(boardRect.left+i*25,boardRect.top);
		Line(0,gNumRows*25-1);
	}
	
	for (i=1; i<gNumRows; i++)
	{
		MoveTo(boardRect.left,boardRect.top+i*25);
		Line(gNumCols*25-1,0);
	}
	
	for (i=0; i<6; i++)
	{
		boardRect.top=10;
		boardRect.bottom=42;
		boardRect.left=(gWindowWidth/2)-((3-i)*34)+1;
		boardRect.right=boardRect.left+32;
		if (!PieceUsed[i])
			PlotIcon(&boardRect, gBWIcons[i]);
		
		boardRect.top+=40;
		boardRect.bottom+=40;
		if (!PieceUsed[i+6])
			PlotIcon(&boardRect, gBWIcons[i+6]);
	}

	boardRect.bottom=gWindowHeight+20-(gNumRows*25);
	boardRect.top=boardRect.bottom-24;
	
	for (i=0; i<gNumRows; i++)
	{
		boardRect.left=6;
		boardRect.right=30;
		for (j=0; j<gNumCols; j++)
		{
			FillRect(&boardRect,*gBWPatterns[Board[i][j]+2]);
			boardRect.left+=25;
			boardRect.right+=25;
		}
		boardRect.top+=25;
		boardRect.bottom+=25;
	}
}
