/**********************************************************************\

File:		v graphics.c

Purpose:	This module handles drawing the main window (the actual
			graphics part of displaying the file data).


Voyeur -- a no-frills file viewer
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

#include "v graphics.h"
#include "v.h"
#include "v structs.h"
#include "v hex.h"
#include "msg graphics.h"

void DrawBoardColor(void)
{
	RGBColor	oldForeColor, oldBackColor;

	GetForeColor(&oldForeColor);
	GetBackColor(&oldBackColor);
	
	DrawBoardBW();
	
	RGBForeColor(&oldForeColor);
	RGBBackColor(&oldBackColor);
}

void DrawBoardBW(void)
{
	GrafPtr		curPort;
	int			i,j;
	int			temp;
	Rect		invertRect;
	Str255		tempStr;
	
	GetPort(&curPort);
	
	EraseRect(&(curPort->portRect));

	TextFont(monaco);
	TextSize(9);
	
	for (i=0; i<16; i++)
	{
		temp=i*16;
		MoveTo(10,15+i*12);
		DrawChar(hexchar[1][i]);
		DrawString("\p0:  ");
		for (j=0; j<16; j++)
		{
			DrawChar(hexchar[0][buffer[temp+j]]);
			DrawChar(hexchar[1][buffer[temp+j]]);
			DrawChar(' ');
		}
		DrawChar(' ');
		for (j=0; j<16; j++)
		{
			DrawChar(asciichar[buffer[temp+j]]);
		}
	}
	invertRect.top=6+12*(gBufferOffset/16);
	invertRect.bottom=invertRect.top+11;
	invertRect.left=38+18*(gBufferOffset%16);
	invertRect.right=invertRect.left+15;
	InvertRect(&invertRect);
	
	invertRect.left=333+6*(gBufferOffset%16);
	invertRect.right=invertRect.left+7;
	InvertRect(&invertRect);
	
	MoveTo(10, 219);
	DrawString("\pOffset: ");
	LongToHexString(offset[forknum], tempStr);
	DrawString(tempStr);
	DrawString("\p    Total length: ");
	LongToHexString(totallen[forknum], tempStr);
	DrawString(tempStr);
	DrawString("\p    ");
	if (forknum==0)
		DrawString("\pData");
	else
		DrawString("\pResource");
	DrawString("\p fork");
}
