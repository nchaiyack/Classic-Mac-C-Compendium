/**********************************************************************\

File:		cube.c

Purpose:	This module handles game initialization & shutdown, and
			handles key and mouse events during a game.


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

#include "cube.h"
#include "cube meat.h"
#include "cube generic open.h"
#include "cube load-save.h"
#include "cube files.h"
#include "cube graphics.h"
#include "cube win.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "msg environment.h"
#include "msg sounds.h"

PicHandle		gCleanMirror;
PicHandle		gKeyPic;

Boolean			gUseMirror;
Boolean			gShowAll;
int				Cube[4][6];

PixPatHandle	gColorPatterns[4];
Handle			gBWPatterns[4];

void InitGame(void)
{
	int			i,j,k;
	AppFile		myFile;
	
	gWindowHeight=300;
	gWindowWidth=500;
	
	gCleanMirror=GetPicture(201);
	gKeyPic=GetPicture(202);
	
	for (i=0; i<4; i++)
	{
		if (gHasColorQD)
			gColorPatterns[i]=GetPixPat(128+i);
		gBWPatterns[i]=GetPattern(128+i);
	}
	
	gameFile.name[0]=0x00;
	deleteTheThing=FALSE;
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		GetAppFiles(1, &myFile);
		MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &gameFile);
		GenericOpen(&gameFile);
		for (k=1; k<=j; k++)
			ClrAppFiles(k);
	}
}

void NewGame(void)
{
	int			i,r,d;
	
	Cube[0][0]=Cube[0][3]=Cube[1][1]=Cube[2][4]=Cube[3][4]=0;
	Cube[0][1]=Cube[1][0]=Cube[2][2]=Cube[2][5]=Cube[3][0]=Cube[3][2]=Cube[3][3]=1;
	Cube[0][4]=Cube[0][5]=Cube[1][2]=Cube[1][4]=Cube[2][3]=Cube[3][1]=2;
	Cube[0][2]=Cube[1][3]=Cube[1][5]=Cube[2][0]=Cube[2][1]=Cube[3][5]=3;
	
	for (i=0; i<100; i++)
	{
		r=(Random()&0x7fff)%4;
		d=(Random()&0x7fff)%6;
		switch (d)
		{
			case 0:
				Qrotate(r);
				break;
			case 1:
				Arotate(r);
				break;
			case 2:
				Zrotate(r);
				break;
			case 3:
				Wrotate(r);
				break;
			case 4:
				Srotate(r);
				break;
			case 5:
				Xrotate(r);
				break;
		}
	}
	
	OpenMainWindow();
	AdjustMenus();
}

void GameUndo(void)
{
}

void GameEvent(void)
{
}

void GameKeyEvent(char charPressed)
{
	int			doUpdate;
	
	ObscureCursor();
	
	doUpdate=-1;
	
	if (charPressed>0x60)
		charPressed-=0x20;
		
	switch (charPressed)
	{
		case 'Q':
			Qrotate(0);
			doUpdate=0;
			break;
		case 'A':
			Arotate(0);
			doUpdate=0;
			break;
		case 'Z':
			Zrotate(0);
			doUpdate=0;
			break;
		case 'W':
			Wrotate(0);
			doUpdate=0;
			break;
		case 'S':
			Srotate(0);
			doUpdate=0;
			break;
		case 'X':
			Xrotate(0);
			doUpdate=0;
			break;
		case 'E':
			Qrotate(1);
			doUpdate=1;
			break;
		case 'D':
			Arotate(1);
			doUpdate=1;
			break;
		case 'C':
			Zrotate(1);
			doUpdate=1;
			break;
		case 'R':
			Wrotate(1);
			doUpdate=1;
			break;
		case 'F':
			Srotate(1);
			doUpdate=1;
			break;
		case 'V':
			Xrotate(1);
			doUpdate=1;
			break;
		case 'T':
			Qrotate(2);
			doUpdate=2;
			break;
		case 'G':
			Arotate(2);
			doUpdate=2;
			break;
		case 'B':
			Zrotate(2);
			doUpdate=2;
			break;
		case 'Y':
			Wrotate(2);
			doUpdate=2;
			break;
		case 'H':
			Srotate(2);
			doUpdate=2;
			break;
		case 'N':
			Xrotate(2);
			doUpdate=2;
			break;
		case 'U':
			Qrotate(3);
			doUpdate=3;
			break;
		case 'J':
			Arotate(3);
			doUpdate=3;
			break;
		case 'M':
			Zrotate(3);
			doUpdate=3;
			break;
		case 'I':
			Wrotate(3);
			doUpdate=3;
			break;
		case 'K':
			Srotate(3);
			doUpdate=3;
			break;
		case ',':
			Xrotate(3);
			doUpdate=3;
			break;
	}

	if (doUpdate>=0)
	{
		DoSound(sound_rotate);
		if (GetWindowDepth() <= 2)
			DrawCubeBW(doUpdate);
		else
			DrawCubeColor(doUpdate);
		
		if (CheckForWin())
			WinGame();
	}
}

void ShutDownGame(void)
{
	int			i;
	
	ReleaseResource(gCleanMirror);
	ReleaseResource(gKeyPic);
	
	for (i=0; i<4; i++)
	{
		if (gHasColorQD)
			DisposePixPat(gColorPatterns[i]);
		ReleaseResource(gBWPatterns[i]);
	}
}
