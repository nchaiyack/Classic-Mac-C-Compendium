/**********************************************************************\

File:		msg graphics.c

Purpose:	This module handles the about box, about MSG box, main
			window opening/closing/updating, and help windows
			opening/closing/updating.


Menu Fixer -=- synchronize menu IDs and menu resource IDs
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

#include "msg graphics.h"
#include "msg dialogs.h"
#include "msg environment.h"
#include "msg menus.h"
#include "msg error.h"
#include "fix.h"
#include "fix help.h"

Rect			gMainScreenBounds;
GDHandle		gBiggestDevice;
WindowPtr		gHelp[NUM_HELP];
int				gHelpWidth;
int				gHelpHeight;

void ShowInformation(void)
{
	DialogPtr	theDlog;
	int			itemSelected = 0;
	int			newleft;
	int			newtop;
	int			dlogID;
	
	if(GetWindowDepth() > 2)
		dlogID = colorInfoDialog;
	else
		dlogID = bwInfoDialog;
	
	theDlog = GetNewDialog(dlogID, 0L, (WindowPtr)-1L);
	newleft = gMainScreenBounds.left + (((gMainScreenBounds.right -
				gMainScreenBounds.left) - (theDlog->portRect.right -
				theDlog->portRect.left)) / 2);
	newtop = gMainScreenBounds.top + (((gMainScreenBounds.bottom -
				gMainScreenBounds.top) - (theDlog->portRect.bottom -
				theDlog->portRect.top)) / 2);
	if(newtop < 15)
		newtop = 15;
	MoveWindow(theDlog, newleft, newtop, TRUE);
	ShowWindow(theDlog);
	
	while(itemSelected == 0)
	{
		ModalDialog(0L, &itemSelected);
	}
	HideWindow(theDlog);
	DisposeDialog(theDlog);
}

void GetMainScreenBounds(void)
{
	gMainScreenBounds = screenBits.bounds;
	gMainScreenBounds.top += MBarHeight;
}

int GetWindowDepth(void)
{
	return (gHasColorQD) ? (**(**GetMainDevice()).gdPMap).pixelSize : 1;
}

void OpenHelpWindow(int whichHelp)
{
	Rect	helpRect;
	
	if (!(gHelp[whichHelp]))
	{
		helpRect.left=10+20*whichHelp;
		helpRect.top=50+20*whichHelp;
		helpRect.bottom=helpRect.top+gHelpHeight;
		helpRect.right=helpRect.left+gHelpWidth;
		
		gHelp[whichHelp]=NewWindow(0L, &helpRect, "\p", TRUE, noGrowDocProc, 
									(WindowPtr)-1L, TRUE, 0L);
		SetHelpTitle(whichHelp);
	}
	
	SetPort(gHelp[whichHelp]);
	InvalRect(&((gHelp[whichHelp])->portRect));
}

void SetHelpTitle(int whichHelp)
{
	Str255		name;
	
	GetItem(gHelpMenu, whichHelp + 1, name);
	SetWTitle(gHelp[whichHelp], name);
}

void UpdateHelp(int whichHelp)
{
	long		offRowBytes, sizeOfOff;
	Ptr			myBits;
	Rect		destRect, bRect;
	int			err;
	GrafPort	myGrafPort;
	GrafPtr		myGrafPtr;
	RgnHandle	oldClipRgn;
	
	SetPort(gHelp[whichHelp]);
	
	bRect = (gHelp[whichHelp])->portRect;
	
	myGrafPtr = &myGrafPort;
	OpenPort(myGrafPtr);
	
	offRowBytes = ((gHelpWidth + 15) >> 4) << 1;
	sizeOfOff = (long)(gHelpHeight) * offRowBytes;
	OffsetRect(&bRect, -bRect.left, -bRect.top);
	
	myBits = NewPtr(sizeOfOff);
	if(myBits == 0L)
		ErrorString("\pThere is not enough memory.  ", "\p");
	
	myGrafPort.portBits.baseAddr = myBits;
	myGrafPort.portBits.rowBytes = offRowBytes;
	myGrafPort.portBits.bounds = bRect;
	
	myGrafPort.portRect = bRect;
	
	oldClipRgn = myGrafPort.clipRgn;
	myGrafPort.clipRgn = (gHelp[whichHelp])->visRgn;
	
	SetPort(myGrafPtr);
	
	DrawHelp(whichHelp);
	
	SetPort(gHelp[whichHelp]);
	
	CopyBits(&(myGrafPtr->portBits),
		&((gHelp[whichHelp])->portBits), &bRect, &bRect, 0, 0L);
	
	myGrafPort.clipRgn = oldClipRgn;
	
	ClosePort(myGrafPtr);
	DisposPtr(myBits);
}
