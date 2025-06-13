/**********************************************************************\

File:		timing.c

Purpose:	This module handles timing loops, based on the TickCount,
			so that time-sensitive procedures operate at the same
			speed on all Macintoshes.

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

#include "timing.h"

long		oldTicks;

pascal void StartTiming(void)
{
	oldTicks=TickCount();
}

pascal void TimeCorrection(int tickCount)
{
	long		newTicks;
	
	newTicks=TickCount();
	while (newTicks-oldTicks<tickCount)
		Delay(1L, &newTicks);
}

pascal void ScrollTheRect(Rect *r, int dh, int dv, RgnHandle unused)
{
	Rect			theDestRect;
	GrafPtr			thePort;
	RgnHandle		temprgn;
	
	SetRect(&theDestRect, (*r).left+dh, (*r).top+dv, (*r).right+dh, (*r).bottom+dv);
	temprgn=NewRgn();
	RectRgn(temprgn, r);
	GetPort(&thePort);
	CopyBits(&(thePort->portBits), &(thePort->portBits), r, &theDestRect, 0, temprgn);
	DisposeRgn(temprgn);
}
