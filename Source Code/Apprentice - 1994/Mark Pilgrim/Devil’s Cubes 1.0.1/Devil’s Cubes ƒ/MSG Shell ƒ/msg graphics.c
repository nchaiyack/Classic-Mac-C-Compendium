/**********************************************************************\

File:		msg graphics.c

Purpose:	This module handles the about box, about MSG box, main
			window opening/closing/updating, and help windows
			opening/closing/updating.


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

#include "msg graphics.h"
#include "msg dialogs.h"
#include "msg error.h"
#include "msg menus.h"
#include "msg sounds.h"
#include "msg environment.h"
#include "msg prefs.h"
#include "cube.h"
#include "cube graphics.h"
#include "cube help.h"
#include "cube load-save.h"

Boolean			gInitedWindowBounds;
Rect			gMainScreenBounds;
Rect			gMainWindowBounds;
GDHandle		gBiggestDevice;
WindowPtr		gMainWindow;
WindowPtr		gHelp[NUM_HELP];
int				gHelpWidth;
int				gHelpHeight;
int				gWindowWidth;
int				gWindowHeight;

/* internal stuff */
Rect			bRect;
Ptr				myBits;
CGrafPort		myCGrafPort;
CGrafPtr		myCGrafPtr;
CTabHandle		ourCMHandle;
GrafPort		myGrafPort;
GrafPtr			myGrafPtr;

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

void ShowSplashScreen(void)
{
	int				dlogID;
	DialogPtr		theDlog;
	int				itemSelected = 0;
	int				newleft;
	int				newtop;
	
	if(GetWindowDepth() > 2)
		dlogID = colorSplashDialog;
	else
		dlogID = bwSplashDialog;
	
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
	
	ParamText(gMyName,gMyOrg,"","");

	DoSound(sound_aboutMSG);
	OpenTheSoundDevice();
	ShowWindow(theDlog);
	while(itemSelected == 0)
		ModalDialog(ProcOFilter, &itemSelected);
	
	HideWindow(theDlog);
	DisposeDialog(theDlog);
	CloseTheSoundDevice();
}

void OpenMainWindow(void)
{
	if(!gMainWindow)
	{
		if(!gInitedWindowBounds)
		{
			gMainWindowBounds.left = gMainScreenBounds.left + (((gMainScreenBounds.right -
						gMainScreenBounds.left) - gWindowWidth) / 2);
			gMainWindowBounds.top = 9+ gMainScreenBounds.top + (((gMainScreenBounds.bottom -
						gMainScreenBounds.top) - gWindowHeight) / 2);
			if(gMainWindowBounds.top < 30)
				gMainWindowBounds.top = 30;
			gMainWindowBounds.bottom = gMainWindowBounds.top + gWindowHeight;
			gMainWindowBounds.right = gMainWindowBounds.left + gWindowWidth;
			gInitedWindowBounds = 1;
		}
		
		myCGrafPtr=0L;
		myGrafPtr=0L;

		if(gHasColorQD)
		{
			gMainWindow = NewCWindow(0L, &gMainWindowBounds, "\pDevil�s Cubes", TRUE,
					noGrowDocProc, (WindowPtr)-1L, TRUE, 0L);
		}
		else
		{
			gMainWindow = NewWindow(0L, &gMainWindowBounds, "\pDevil�s Cubes", TRUE,
					noGrowDocProc, (WindowPtr)-1L, TRUE, 0L);
		}

		bRect = gMainWindow->portRect;
	}
	
	if(gMainWindow)
	{
		SetPort(gMainWindow);
		InvalRect(&(gMainWindow->portRect));
	}
	else ErrorString("\pThere is not enough memory to open the main window.","\p");
}

void GetMainScreenBounds(void)
{
	gMainScreenBounds = screenBits.bounds;
	gMainScreenBounds.top += MBarHeight;
}

int GetWindowDepth(void)
{
	Rect		tempRect;
	long		biggestSize;
	long		tempSize;
	GDHandle	thisHandle;
	
	if(gHasColorQD)
	{
		if(gMainWindow)
		{
			thisHandle = GetDeviceList();
			gBiggestDevice = 0L;
			biggestSize = 0L;
			
			while(thisHandle)
			{
				if(TestDeviceAttribute(thisHandle, screenDevice) &&
							TestDeviceAttribute(thisHandle, screenActive))
					if(SectRect(&(gMainWindow->portRect), &((**thisHandle).gdRect),
								&tempRect))
						if(biggestSize < (tempSize =
								((long)(tempRect.bottom - tempRect.top))
								* ((long)(tempRect.right - tempRect.left))))
						{
							biggestSize = tempSize;
							gBiggestDevice = thisHandle;
						}
				thisHandle = GetNextDevice(thisHandle);
			}
			
			if(gBiggestDevice)
				return (**(**gBiggestDevice).gdPMap).pixelSize;
			else
				return 1;
		}
		else
		{
			return (**(**GetMainDevice()).gdPMap).pixelSize;
		}
	}
	else
	{
		return 1;
	}
}

void UpdateBoard(void)
{
	long		offRowBytes, sizeOfOff;
	int			theDepth, i, err;
	GDHandle	oldDevice;
	static int	gLastDepth;

	if((theDepth = GetWindowDepth()) > 2)
	{
		/* if we just changed from one color depth to another color depth */
		if((myCGrafPtr != 0) && (gLastDepth != theDepth))
		{
			DisposeHandle((**(myCGrafPort).portPixMap).pmTable);
			DisposePtr((**(myCGrafPort).portPixMap).baseAddr);
			CloseCPort(myCGrafPtr);
			myCGrafPtr = 0;
		}
		
		if (myCGrafPtr==0L)
		{
			/* if we just switched from b/w to color, delete the b/w port */
			if(myGrafPtr != 0L)
			{
				DisposePtr(myGrafPort.portBits.baseAddr);
				ClosePort(myGrafPtr);
				myGrafPtr = 0;
			}
			
			if(gBiggestDevice)
			{
				oldDevice = GetGDevice();
				SetGDevice(gBiggestDevice);
			}
			else
				oldDevice = 0;
			
			myCGrafPtr = &myCGrafPort;
			OpenCPort(myCGrafPtr);
			gLastDepth = theDepth = (**(myCGrafPort).portPixMap).pixelSize;
			
			offRowBytes = (((theDepth * (bRect.right - bRect.left)) + 15) >> 4) << 1;
			sizeOfOff = (long)(bRect.bottom - bRect.top) * offRowBytes;
			OffsetRect(&bRect, -bRect.left, -bRect.top);
			
			myBits = NewPtr(sizeOfOff);
			if(myBits == 0L)
				ErrorString("\pThere is not enough memory to open the main window.","\p");
			
			(**(myCGrafPort).portPixMap).baseAddr = myBits;
			(**(myCGrafPort).portPixMap).rowBytes = offRowBytes + 0x8000;
			(**(myCGrafPort).portPixMap).bounds = bRect;
			
			myCGrafPort.portRect = bRect;
			
			ourCMHandle = (**(**gBiggestDevice).gdPMap).pmTable;
			err = HandToHand(&ourCMHandle);
			if(err != noErr)
				ErrorString("\pThere is not enough memory to open the main window.","\p");
			
			for(i = 0; i <= (**ourCMHandle).ctSize; i++)
				(**ourCMHandle).ctTable[i].value = i;
			(**ourCMHandle).ctFlags &= 0x7fff;
			(**ourCMHandle).ctSeed = GetCTSeed();
			
			(**(myCGrafPort).portPixMap).pmTable = ourCMHandle;
			
			if(oldDevice)
				SetGDevice(oldDevice);
		}
		
		UpdateBoardColor();
	}
	else
	{
		if (myGrafPtr==0L)
		{
			if(myCGrafPtr != 0)
			{
				DisposeHandle((**(myCGrafPort).portPixMap).pmTable);
				DisposePtr((**(myCGrafPort).portPixMap).baseAddr);
				CloseCPort(myCGrafPtr);
				myCGrafPtr = 0;
			}
			
			myGrafPtr = &myGrafPort;
			OpenPort(myGrafPtr);
			
			offRowBytes = (((bRect.right - bRect.left) + 15) >> 4) << 1;
			sizeOfOff = (long)(bRect.bottom - bRect.top) * offRowBytes;
			OffsetRect(&bRect, -bRect.left, -bRect.top);
			
			myBits = NewPtr(sizeOfOff);
			if(myBits == 0L)
				ErrorString("\pThere is not enough memory to open the main window.", "\p");
			
			myGrafPort.portBits.baseAddr = myBits;
			myGrafPort.portBits.rowBytes = offRowBytes;
			myGrafPort.portBits.bounds = bRect;
			myGrafPort.portRect = bRect;
		}
		
		UpdateBoardBW();
	}
}

void UpdateBoardColor(void)
{
	GDHandle	oldDevice;
	RgnHandle	oldClipRgn;
	
	SetPort(gMainWindow);
	
	oldDevice = GetGDevice();
	SetGDevice(gBiggestDevice);
	
	oldClipRgn = myCGrafPort.clipRgn;
	myCGrafPort.clipRgn = gMainWindow->visRgn;
	
	SetPort((GrafPtr)myCGrafPtr);
	
	DrawBoardColor();
	
	SetPort(gMainWindow);
	SetGDevice(oldDevice);
	
	CopyBits(&(((GrafPtr)myCGrafPtr)->portBits),
				&(gMainWindow->portBits), &bRect, &bRect, 0, 0L);
	
	myCGrafPort.clipRgn = oldClipRgn;
}

void UpdateBoardBW(void)
{
	RgnHandle	oldClipRgn;
	
	SetPort(gMainWindow);
	
	oldClipRgn = myGrafPort.clipRgn;
	myGrafPort.clipRgn = gMainWindow->visRgn;
	
	SetPort(myGrafPtr);
	
	DrawBoardBW();
	
	SetPort(gMainWindow);
	
	CopyBits(&(myGrafPtr->portBits),
				&(gMainWindow->portBits), &bRect, &bRect, 0, 0L);
	
	myGrafPort.clipRgn = oldClipRgn;
}

void CloseMainWindow(void)
{
	gameFile.name[0]=0x00;
	
	DisposeWindow(gMainWindow);
	gMainWindow=0L;
	gInitedWindowBounds=FALSE;
	
	if(myCGrafPtr != 0)
	{
		DisposeHandle((**(myCGrafPort).portPixMap).pmTable);
		DisposePtr((**(myCGrafPort).portPixMap).baseAddr);
		CloseCPort(myCGrafPtr);
		myCGrafPtr = 0;
	}
	if(myGrafPtr != 0L)
	{
		DisposePtr(myGrafPort.portBits.baseAddr);
		ClosePort(myGrafPtr);
		myGrafPtr = 0;
	}
	if ((myCGrafPtr!=0L) || (myGrafPtr!=0L))
		DisposPtr(myBits);
	
	AdjustMenus();
	DrawMenuBar();
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
	Rect		bRect;
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
