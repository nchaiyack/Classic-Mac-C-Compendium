/**********************************************************************\

File:		cube graphics.c

Purpose:	This module handles actually drawing the board to an
			offscreen bitmap; also drawing individual cubes.


Devil�s Cubes -- a simple cubes puzzle
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

#include "cube graphics.h"
#include "cube.h"
#include "msg graphics.h"

void DrawBoardColor(void)
{
	RGBColor	oldForeColor, oldBackColor;
	GrafPtr		curPort;
	Rect		space,dest;
	RgnHandle	myRegion;
	int			i,j;
	int			x,y;
	
	GetForeColor(&oldForeColor);
	GetBackColor(&oldBackColor);
	
	GetPort(&curPort);
	
	EraseRect(&(curPort->portRect));
	
	space.left=(gWindowWidth/2)-181;
	space.right=space.left+359;
	space.bottom=gWindowHeight-5;
	space.top=space.bottom-57;
	DrawPicture(gKeyPic,&space);

	space.left=(gWindowWidth/2)-219;
	space.right=space.left+437;
	space.top=5;
	space.bottom=space.top+137;
	if (gUseMirror)
		DrawPicture(gCleanMirror, &space);
	
	for (i=0; i<4; i++)
		DrawCubeColor(i);

	RGBForeColor(&oldForeColor);
	RGBBackColor(&oldBackColor);
}

void DrawBoardBW(void)
{
	GrafPtr		curPort;
	Rect		space;
	int			i;
	
	GetPort(&curPort);
	
	EraseRect(&(curPort->portRect));
	
	space.left=(gWindowWidth/2)-181;
	space.right=space.left+359;
	space.bottom=gWindowHeight-5;
	space.top=space.bottom-57;
	DrawPicture(gKeyPic,&space);

	space.left=(gWindowWidth/2)-219;
	space.right=space.left+437;
	space.top=5;
	space.bottom=space.top+137;
	if (gUseMirror)
		DrawPicture(gCleanMirror, &space);
	
	for (i=0; i<4; i++)
		DrawCubeBW(i);
}

void DrawCubeColor(int i)
{
	int			x,y;
	RgnHandle	myRegion;
	
	y=73;
	x=(gWindowWidth/2)-152+100*i;
	
	myRegion=NewRgn();
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(34,20);
		Line(-34,20);
		Line(-34,-20);
		Line(34,-20);
	CloseRgn(myRegion);
	FillCRgn(myRegion, gColorPatterns[Cube[i][3]]);
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(0,-40);
		Line(-34,20);
		Line(0,40);
		Line(34,-20);
	CloseRgn(myRegion);
	if (gShowAll)
		FillCRgn(myRegion, gColorPatterns[Cube[i][4]]);
	else
		FillRgn(myRegion, black);
		
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(0,-40);
		Line(34,20);
		Line(0,40);
		Line(-34,-20);
	CloseRgn(myRegion);
	FillCRgn(myRegion, gColorPatterns[Cube[i][0]]);
	
	MoveTo(x,y);
	Line(0,-40);
	Line(-34,20);
	Line(0,40);
	Line(34,-20);
	Line(34,20);
	Line(0,-40);
	Line(-34,-20);
	MoveTo(x+34,y+20);
	Line(-34,20);
	Line(-34,-20);
	
	y+=115;
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(-34,-20);
		Line(34,-20);
		Line(34,20);
		Line(-34,20);
	CloseRgn(myRegion);
	FillCRgn(myRegion, gColorPatterns[Cube[i][1]]);
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(-34,-20);
		Line(0,40);
		Line(34,20);
		Line(0,-40);
	CloseRgn(myRegion);
	FillCRgn(myRegion, gColorPatterns[Cube[i][2]]);
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(34,-20);
		Line(0,40);
		Line(-34,20);
		Line(0,-40);
	CloseRgn(myRegion);
	if (gShowAll)
		FillCRgn(myRegion, gColorPatterns[Cube[i][5]]);
	else
		FillRgn(myRegion, black);
		
	MoveTo(x,y);
	Line(-34,-20);
	Line(0,40);
	Line(34,20);
	Line(0,-40);
	Line(34,-20);
	Line(0,40);
	Line(-34,20);
	MoveTo(x-34,y-20);
	Line(34,-20);
	Line(34,20);
	
	DisposeRgn(myRegion);
}

void DrawCubeBW(int i)
{
	int			x,y;
	RgnHandle	myRegion;
	
	y=73;
	x=(gWindowWidth/2)-152+100*i;
	
	myRegion=NewRgn();
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(34,20);
		Line(-34,20);
		Line(-34,-20);
		Line(34,-20);
	CloseRgn(myRegion);
	FillRgn(myRegion, *gBWPatterns[Cube[i][3]]);
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(0,-40);
		Line(-34,20);
		Line(0,40);
		Line(34,-20);
	CloseRgn(myRegion);
	if (gShowAll)
		FillRgn(myRegion, *gBWPatterns[Cube[i][4]]);
	else
		FillRgn(myRegion, black);
		
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(0,-40);
		Line(34,20);
		Line(0,40);
		Line(-34,-20);
	CloseRgn(myRegion);
	FillRgn(myRegion, *gBWPatterns[Cube[i][0]]);
	
	MoveTo(x,y);
	Line(0,-40);
	Line(-34,20);
	Line(0,40);
	Line(34,-20);
	Line(34,20);
	Line(0,-40);
	Line(-34,-20);
	MoveTo(x+34,y+20);
	Line(-34,20);
	Line(-34,-20);
	
	y+=115;
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(-34,-20);
		Line(34,-20);
		Line(34,20);
		Line(-34,20);
	CloseRgn(myRegion);
	FillRgn(myRegion, *gBWPatterns[Cube[i][1]]);
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(-34,-20);
		Line(0,40);
		Line(34,20);
		Line(0,-40);
	CloseRgn(myRegion);
	FillRgn(myRegion, *gBWPatterns[Cube[i][2]]);
	
	SetEmptyRgn(myRegion);
	MoveTo(x,y);
	OpenRgn();
		Line(34,-20);
		Line(0,40);
		Line(-34,20);
		Line(0,-40);
	CloseRgn(myRegion);
	if (gShowAll)
		FillRgn(myRegion, *gBWPatterns[Cube[i][5]]);
	else
		FillRgn(myRegion, black);
		
	MoveTo(x,y);
	Line(-34,-20);
	Line(0,40);
	Line(34,20);
	Line(0,-40);
	Line(34,-20);
	Line(0,40);
	Line(-34,20);
	MoveTo(x-34,y-20);
	Line(34,-20);
	Line(34,20);
	
	DisposeRgn(myRegion);

}
