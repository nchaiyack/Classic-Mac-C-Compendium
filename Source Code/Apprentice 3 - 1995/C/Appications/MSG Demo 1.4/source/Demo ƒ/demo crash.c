/**********************************************************************\

File:		demo crash.c

Purpose:	This module handles crashing your machine with grace and
			style.

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

#include "demo crash.h"
#include "demo.h"
#include "sounds.h"
#include "environment.h"
#include "main.h"

void RestoreScreen(int oldMenuHeight)
{
	TextFont(0);
	TextSize(0);
	TextMode(srcOr);
	FlushEvents(mDownMask+mUpMask+keyDownMask+keyUpMask+autoKeyMask, 0L);
	MBarHeight=oldMenuHeight;
	DrawMenuBar();
	PaintOne(0L,GrayRgn);
	PaintBehind(WindowList,GrayRgn);
	ShowCursor();
	ObscureCursor();
}

void CrashAndBurn(int whichSystemError)
{
	int				oldMenuHeight;
	int				whichWipe;
	
	DoSound(sound_crash, FALSE);
	
	ShutDownEnvironment();		/* If you have to crash a computer, */
								/* it costs nothing to be polite. */
	SysError(whichSystemError);
	
	whichWipe=(Ticks&0x7fffffff)%NUM_WIPES;
	DoFullScreenFade(whichWipe+1);
	ExitToShell();
}
