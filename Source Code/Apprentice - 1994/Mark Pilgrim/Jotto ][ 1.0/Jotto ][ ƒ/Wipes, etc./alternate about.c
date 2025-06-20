/**********************************************************************\

File:		alternate about.c

Purpose:	This module handles the alternate about box.


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

#include "text scroll.h"
#include "alternate about.h"
#include "gamma.h"

int				oldMenuHeight;

void AlternateInformation(void)
{
	InitScroll();
	SetupNonMacMode();
	while ((!Button()) && (ScrollIt()));
	while (Button());
	ShutDownScroll();
	SetupMacMode();
}

void SetupNonMacMode(void)
{
	int				i;
	Boolean			doGamma;
	
	doGamma=IsGammaAvailable();
	if (doGamma)
	{
		SetupGammaTools();
		for (i=100; i>=0; i-=5)
			DoGammaFade(i);
	}
	HideCursor();
	oldMenuHeight=MBarHeight;
	MBarHeight=0;
	DrawMenuBar();
	SetPort(WMgrPort);
	FillRect(&WMgrPort->portRect, black);
	if (doGamma)
	{
		DoGammaFade(100);
		DisposeGammaTools();
	}
}

void SetupMacMode(void)
{
	TextFont(0);
	TextSize(0);
	TextMode(srcOr);
	FlushEvents(mDownMask+mUpMask+keyDownMask+keyUpMask+autoKeyMask, 0);
	MBarHeight=oldMenuHeight;
	DrawMenuBar();
	PaintOne(0L,GrayRgn);
	PaintBehind(WindowList,GrayRgn);
	ShowCursor();
}
