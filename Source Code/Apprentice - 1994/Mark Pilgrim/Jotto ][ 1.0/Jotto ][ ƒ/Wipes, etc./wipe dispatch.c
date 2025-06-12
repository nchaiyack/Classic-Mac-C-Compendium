/**********************************************************************\

File:		wipe dispatch.c

Purpose:	This module handles calling the right graphic effect.


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
#include "wipe dispatch.h"
#include "msg graphics.h"
#include "msg prefs.h"

int			gWhichWipe;
int			gLastWipe;

void DoTheDissolve(GrafPtr myGrafPtr)
{
	DissolveBox(myGrafPtr, (GrafPtr)gMainWindow, &(myGrafPtr->portRect),
		&(gMainWindow->portRect));
}

void DoTheWipe(GrafPtr myGrafPtr)
{
	ObscureCursor();
	switch (gWhichWipe)
	{
		case 1:		BoxOutWipe(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 2:		SpiralGyra(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 3:		FullScrollUD(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 4:		CircularWipe(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 5:		FourCorner(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 6:		CircleIn(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);				break;
		case 7:		DiagonalWipe(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 8:		SlideWipe(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 9:		Skipaline(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 10:	RescueRaiders(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);		break;
		case 11:	CircleOut(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);			break;
		case 12:	MrDo(myGrafPtr, (GrafPtr)gMainWindow, gWindowHeight, gWindowWidth);					break;
	}
	gLastWipe=gWhichWipe;
	gWhichWipe=0;
	SaveThePrefs();
}
