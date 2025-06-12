/**********************************************************************\

File:		Dissolve wipe.c

Purpose:	Graphic effect from offscreen bitmap to main window (on
			screen).  See comments below for more description.


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

#include "msg timing.h"
#include "wipe dispatch.h"

#define CorrectTime 2

void DissolveBox(GrafPtr newImage, GrafPtr destGrafPtr, Rect *source, Rect *dest)
{
	long		offRowBytes, sizeOfOff;
	Ptr			myBits;
	Rect		bRect;
	GrafPort	myGrafPort;
	GrafPtr		myGrafPtr;
	RgnHandle	oldClipRgn;
	int			i, j;
	Pattern		thePattern;
	char		order[16];
	long		randtemp;
	char		ordertemp;
	
	/* the dissolve effect works by creating a random set of patterns which sum
	(OR) to a black pattern.  We make another offscreen bitmap and fill it with
	the pattern at each stage, and use it as a mask for the bitcopy.  Here, we
	create the offscreen bitmap. */
	
	SetPort(newImage);
	bRect = *source;
	myGrafPtr = &myGrafPort;
	OpenPort(myGrafPtr);
	offRowBytes = (((source->right - source->left) + 15) >> 4) << 1;
	sizeOfOff = (long)(source->bottom - source->top) * offRowBytes;
	OffsetRect(&bRect, -bRect.left, -bRect.top);
	myBits = NewPtr(sizeOfOff);
	if(myBits == 0L)
	{
		CopyBits(&(newImage->portBits), &(destGrafPtr->portBits), &source, &dest, 0, 0L);
		ClosePort(myGrafPtr);
		return;
	}
	myGrafPort.portBits.baseAddr = myBits;
	myGrafPort.portBits.rowBytes = offRowBytes;
	myGrafPort.portBits.bounds = bRect;
	myGrafPort.portRect = bRect;
	oldClipRgn = myGrafPort.clipRgn;
	myGrafPort.clipRgn = destGrafPtr->visRgn;
	
	for(i = 0; i < 16; i++)
		order[i] = i;
	
	/* this randomly shuffles the order in which the pattern bits will appear */
	for(i = 15; i >= 0; i--) {
		randtemp = (((long)Random() + 32767) * (i + 1)) / 65535;
		ordertemp = order[randtemp];
		order[randtemp] = order[i];
		order[i] = ordertemp;
	}
	
	for(i = 0; i < 16; i++)
	{
		StartTiming();
		SetPort(myGrafPtr);
		
		for(j = 0; j < 8; j++) thePattern[j] = 0;
		thePattern[order[i] >> 2] = 1 << (order[i] & 0x03);
		thePattern[order[i] >> 2] |= 16 << (order[i] & 0x03);
		thePattern[(order[i] >> 2) + 4] = 1 << (order[i] & 0x03);
		thePattern[(order[i] >> 2) + 4] |= 16 << (order[i] & 0x03);
		
		FillRect(&bRect, &thePattern);
		
		SetPort(destGrafPtr);
		
		CopyMask(&(newImage->portBits), &(myGrafPtr->portBits),
				&(destGrafPtr->portBits), source, &bRect, dest);
		TimeCorrection(CorrectTime);
	}
	
	myGrafPort.clipRgn = oldClipRgn;
	ClosePort(myGrafPtr);
	DisposPtr(myBits);
}
