/**********************************************************************\

File:		text scroll.c

Purpose:	This modules handles scrolling text (for the alternate
			about box).


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
#include "msg graphics.h"
#include "msg error.h"
#include "msg timing.h"
#include "msg environment.h"

#define HEADLINE_BOTTOM	40
#define gScrollSpeed	2

int				scrollXPos;
long			gScrollPos;
Handle			gTheText;
unsigned char	iv;
Rect			scrollSource;
Rect			scrollDest;
Rect			wholeSource;
Rect			wholeDest;
Rect			gMainRect;

Rect			scrollRect;
Ptr				scrollBits;
CGrafPort		scrollCGrafPort;
CGrafPtr		scrollCGrafPtr;
CTabHandle		scrollCMHandle;
GrafPort		scrollGrafPort;
GrafPtr			scrollGrafPtr;
Boolean			isColorScroll;

Boolean ScrollIt(void)
{
	Rect			source, dest;
	unsigned char	theChar;
	
	StartTiming();
	if (isColorScroll)
	{
		SetPort(scrollCGrafPtr);
		CopyBits(&(((GrafPtr)scrollCGrafPtr)->portBits),
					&(((GrafPtr)scrollCGrafPtr)->portBits), &scrollSource, &scrollDest, 0, 0L);
	}
	else
	{
		SetPort(scrollGrafPtr);
		CopyBits(&(scrollGrafPtr->portBits),
					&(scrollGrafPtr->portBits), &scrollSource, &scrollDest, 0, 0L);
	}
	
	MoveTo(scrollXPos, HEADLINE_BOTTOM-10);
	theChar=(*gTheText)[gScrollPos]^iv;
	TextMode(notSrcCopy);
	TextFont(143);
	TextSize(36);

	DrawChar(theChar);
	
	if (isColorScroll)
		CopyBits(&(((GrafPtr)scrollCGrafPtr)->portBits),
					&(((GrafPtr)WMgrPort)->portBits), &wholeSource, &wholeDest, 0, 0L);
	else
		CopyBits(&(scrollGrafPtr->portBits),
					&(((GrafPtr)WMgrPort)->portBits), &wholeSource, &wholeDest, 0, 0L);
	
	scrollXPos-=gScrollSpeed;
	if (scrollXPos+CharWidth(theChar)<=509-gScrollSpeed)
	{
		scrollXPos+=CharWidth(theChar);
		gScrollPos++;
		iv^=theChar;
		if (gScrollPos==GetHandleSize(gTheText))
			return FALSE;
	}
	SetPort(WMgrPort);
	TimeCorrection(2);
	return TRUE;
}

void InitScroll(void)
{
	long		offRowBytes, sizeOfOff;
	int			theDepth, j, err;
	
	gMainRect.top=WMgrPort->portRect.top+
		((WMgrPort->portRect.bottom-WMgrPort->portRect.top)-342)/2;
	gMainRect.bottom=gMainRect.top+341;
	gMainRect.left=WMgrPort->portRect.left+
		((WMgrPort->portRect.right-WMgrPort->portRect.left)-512)/2;
	gMainRect.right=gMainRect.left+511;
	gScrollPos=0L;
	SetRect(&scrollRect, 0, 0, 509, HEADLINE_BOTTOM);
	SetRect(&scrollSource, 1+gScrollSpeed, 0, 509, HEADLINE_BOTTOM);
	SetRect(&scrollDest, 1, 0, 509-gScrollSpeed, HEADLINE_BOTTOM);
	wholeSource=wholeDest=scrollRect;
	OffsetRect(&wholeDest, gMainRect.left+1, gMainRect.top+(342-HEADLINE_BOTTOM)/2);
	
	scrollCGrafPtr=0L;
	scrollGrafPtr=0L;
	
	if ((theDepth = GetMainScreenDepth()) > 2)
	{
		isColorScroll=TRUE;
		scrollCGrafPtr = &scrollCGrafPort;
		OpenCPort(scrollCGrafPtr);
		
		offRowBytes = (((theDepth * (scrollRect.right - scrollRect.left)) + 15) >> 4) << 1;
		sizeOfOff = (long)(scrollRect.bottom - scrollRect.top) * offRowBytes;
		OffsetRect(&scrollRect, -scrollRect.left, -scrollRect.top);
		
		scrollBits = NewPtr(sizeOfOff);
		if(scrollBits == 0L)
			ErrorString("\pThere is not enough memory.","\p");
		
		(**(scrollCGrafPort).portPixMap).baseAddr = scrollBits;
		(**(scrollCGrafPort).portPixMap).rowBytes = offRowBytes + 0x8000;
		(**(scrollCGrafPort).portPixMap).bounds = scrollRect;
		
		scrollCGrafPort.portRect = scrollRect;
		
		scrollCMHandle = (**(**GetMainDevice()).gdPMap).pmTable;
		err = HandToHand(&scrollCMHandle);
		if(err != noErr)
			ErrorString("\pThere is not enough memory.","\p");
		
		for(j = 0; j <= (**scrollCMHandle).ctSize; j++)
			(**scrollCMHandle).ctTable[j].value = j;
		(**scrollCMHandle).ctFlags &= 0x7fff;
		(**scrollCMHandle).ctSeed = GetCTSeed();
		
		(**(scrollCGrafPort).portPixMap).pmTable = scrollCMHandle;
		
		SetPort(scrollCGrafPtr);
		FillRect(&wholeSource, black);
	}
	else
	{
		isColorScroll=FALSE;
		scrollGrafPtr = &scrollGrafPort;
		OpenPort(scrollGrafPtr);
		
		offRowBytes = (((scrollRect.right - scrollRect.left) + 15) >> 4) << 1;
		sizeOfOff = (long)(scrollRect.bottom - scrollRect.top) * offRowBytes;
		OffsetRect(&scrollRect, -scrollRect.left, -scrollRect.top);
		
		scrollBits = NewPtr(sizeOfOff);
		if(scrollBits == 0L)
			ErrorString("\pThere is not enough memory.", "\p");
		
		scrollGrafPort.portBits.baseAddr = scrollBits;
		scrollGrafPort.portBits.rowBytes = offRowBytes;
		scrollGrafPort.portBits.bounds = scrollRect;
		scrollGrafPort.portRect = scrollRect;
		
		SetPort(scrollGrafPtr);
		FillRect(&wholeSource, black);
	}
		
	gTheText=GetResource('Jot2', 1);
	iv=0x42;
	scrollXPos=509;
}

void ShutDownScroll(void)
{
	ReleaseResource(gTheText);

	if (scrollCGrafPtr!=0L)
	{
		DisposeHandle((**(scrollCGrafPort).portPixMap).pmTable);
		DisposePtr((**(scrollCGrafPort).portPixMap).baseAddr);
		CloseCPort(scrollCGrafPtr);
		scrollCGrafPtr=0L;
	}
	if (scrollGrafPtr!=0L)
	{
		DisposePtr(scrollGrafPort.portBits.baseAddr);
		ClosePort(scrollGrafPtr);
		scrollGrafPtr=0L;
	}
	DisposPtr(scrollBits);
}

int GetMainScreenDepth(void)
{
	return (gHasColorQD) ? (**(**GetMainDevice()).gdPMap).pixelSize : 1;
}
