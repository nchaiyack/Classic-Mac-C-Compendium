/**********************************************************************\

File:		msg graphics.c

Purpose:	This module handles opening/closing/updating all windows:
			help window and about windows.  This includes
			manipulating offscreen bitmaps for fun and no profit.


Fudd -=- convert text to Elmer Fudd talk
Copyright �1994, Mark Pilgrim

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
#include "msg about.h"
#include "msg help.h"
#include "msg dialogs.h"
#include "msg error.h"
#include "msg menus.h"
#include "msg environment.h"
#include "msg prefs.h"
#include "program globals.h"
#include "fudd.h"

Boolean			gInitedWindowBounds[NUM_WINDOWS];
Rect			gMainScreenBounds;
Rect			gWindowBounds[NUM_WINDOWS];
GDHandle		gBiggestDevice;
WindowPtr		gTheWindow[NUM_WINDOWS];
int				gWindowWidth[NUM_WINDOWS];
int				gWindowHeight[NUM_WINDOWS];
Str255			gWindowTitle[NUM_WINDOWS];
int				gWindowType[NUM_WINDOWS];
Boolean			gOffscreenNeedsUpdate[NUM_WINDOWS];
int				gNumHelp;

/* internal stuff */
Rect			bRect[NUM_WINDOWS];
Ptr				myBits[NUM_WINDOWS];
CGrafPort		myCGrafPort[NUM_WINDOWS];
CGrafPtr		myCGrafPtr[NUM_WINDOWS];
CTabHandle		ourCMHandle[NUM_WINDOWS];
GrafPort		myGrafPort[NUM_WINDOWS];
GrafPtr			myGrafPtr[NUM_WINDOWS];
int				gLastDepth[NUM_WINDOWS];
int				gMaxDepth[NUM_WINDOWS];

void InitMSGGraphics(void)
{
	int				i;
	
	gAboutColorPict=gAboutBWPict=0L;
	
	gWindowWidth[kAbout]=180;
	gWindowHeight[kAbout]=250;
	
	gWindowWidth[kAboutMSG]=243;
	gWindowHeight[kAboutMSG]=243;
	
	gWindowWidth[kHelp]=300;
	gWindowHeight[kHelp]=250;
	
	for (i=0; i<NUM_WINDOWS; i++)
	{
		myCGrafPtr[i]=myGrafPtr[i]=gTheWindow[i]=0L;
		gInitedWindowBounds[i]=FALSE;
		gOffscreenNeedsUpdate[i]=TRUE;
	}
	
	gMaxDepth[kAbout]=8;
	gMaxDepth[kAboutMSG]=1;
	gMaxDepth[kHelp]=1;
	
	gWindowType[kAbout]=altDBoxProc;
	gWindowType[kAboutMSG]=plainDBox;
	gWindowType[kHelp]=noGrowDocProc;
	
	StuffHex(gWindowTitle[kHelp], "\p0448656c70");
	gWindowTitle[kAbout][0]=gWindowTitle[kAboutMSG][0]==0x00;
}

void OpenTheWindow(int index)
{
	unsigned long		dummy;
	
	if (!gTheWindow[index])
	{
		if (!gInitedWindowBounds[index])
		{
			if (index==kHelp)
			{
				gWindowBounds[index].left=10;
				gWindowBounds[index].top=50;
			}
			else
			{
				gWindowBounds[index].left = gMainScreenBounds.left + (((gMainScreenBounds.right -
							gMainScreenBounds.left) - gWindowWidth[index]) / 2);
				gWindowBounds[index].top = 9+ gMainScreenBounds.top + (((gMainScreenBounds.bottom -
							gMainScreenBounds.top) - gWindowHeight[index]) / 2);
			}
			if(gWindowBounds[index].top < 30)
				gWindowBounds[index].top = 30;
			gWindowBounds[index].bottom = gWindowBounds[index].top + gWindowHeight[index];
			gWindowBounds[index].right = gWindowBounds[index].left + gWindowWidth[index];
			gInitedWindowBounds[index]=TRUE;
		}
		
		if(gHasColorQD)
		{
			gTheWindow[index] = NewCWindow(0L, &gWindowBounds[index], gWindowTitle[index],
				TRUE, gWindowType[index], (WindowPtr)-1L, TRUE, 0L);
		}
		else
		{
			gTheWindow[index] = NewWindow(0L, &gWindowBounds[index], gWindowTitle[index],
				TRUE, gWindowType[index], (WindowPtr)-1L, TRUE, 0L);
		}

		bRect[index] = gTheWindow[index]->portRect;
	}
	
	if (gTheWindow[index])
	{
		SelectWindow(gTheWindow[index]);
		SetPort(gTheWindow[index]);
		UpdateTheWindow(index);
		if (index==kAboutMSG)
			Delay(30, &dummy);
	}
	else ErrorString("\pThere is not enough memory to open the window.","\p");
}

void GetMainScreenBounds(void)
{
	gMainScreenBounds = screenBits.bounds;
	gMainScreenBounds.top += MBarHeight;
}

int GetWindowDepth(int index)
{
	Rect		tempRect;
	long		biggestSize;
	long		tempSize;
	GDHandle	thisHandle;
	
	if (gHasColorQD)
	{
		if (gTheWindow[index])
		{
			thisHandle = GetDeviceList();
			gBiggestDevice = 0L;
			biggestSize = 0L;
			
			while (thisHandle)
			{
				if (TestDeviceAttribute(thisHandle, screenDevice) &&
							TestDeviceAttribute(thisHandle, screenActive))
					if (SectRect(&(gTheWindow[index]->portRect), &((**thisHandle).gdRect),
								&tempRect))
						if (biggestSize < (tempSize =
								((long)(tempRect.bottom - tempRect.top))
								* ((long)(tempRect.right - tempRect.left))))
						{
							biggestSize = tempSize;
							gBiggestDevice = thisHandle;
						}
				thisHandle = GetNextDevice(thisHandle);
			}
			
			if (gBiggestDevice)
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

void UpdateTheWindow(int index)
{
	long		offRowBytes, sizeOfOff;
	int			theDepth, i, err;
	GDHandle	oldDevice;

	if (((theDepth = GetWindowDepth(index)) > 2) && (gMaxDepth[index]>2))
	{
		/* if we just changed from one color depth to another color depth */
		if((myCGrafPtr[index] != 0L) && (gLastDepth[index] != theDepth))
		{
			DisposeHandle((**(myCGrafPort[index]).portPixMap).pmTable);
			DisposePtr((**(myCGrafPort[index]).portPixMap).baseAddr);
			CloseCPort(myCGrafPtr[index]);
			myCGrafPtr[index] = 0L;			
			gOffscreenNeedsUpdate[index]=TRUE;
		}
		
		if (myCGrafPtr[index]==0L)
		{
			/* if we just switched from b/w to color, delete the b/w port */
			if(myGrafPtr[index] != 0L)
			{
				DisposePtr(myGrafPort[index].portBits.baseAddr);
				ClosePort(myGrafPtr[index]);
				myGrafPtr[index] = 0L;
				gOffscreenNeedsUpdate[index]=TRUE;
			}
			
			if (gBiggestDevice)
			{
				oldDevice = GetGDevice();
				SetGDevice(gBiggestDevice);
			}
			else
				oldDevice = 0L;
			
			myCGrafPtr[index] = &myCGrafPort[index];
			OpenCPort(myCGrafPtr[index]);
			gLastDepth[index] = theDepth = (**(myCGrafPort[index]).portPixMap).pixelSize;
			if (theDepth>gMaxDepth[index])
				gLastDepth[index]=theDepth=gMaxDepth[index];
			
			offRowBytes = (((theDepth * (bRect[index].right - bRect[index].left)) + 15) >> 4) << 1;
			sizeOfOff = (long)(bRect[index].bottom - bRect[index].top) * offRowBytes;
			OffsetRect(&bRect[index], -bRect[index].left, -bRect[index].top);
			
			myBits[index] = NewPtr(sizeOfOff);
			if(myBits[index] == 0L)
			{
				CloseCPort(myCGrafPtr[index]);
				myCGrafPtr[index]=0L;
				ErrorString("\pThere is not enough memory to open the window.","\p");
			}
			
			(**(myCGrafPort[index]).portPixMap).baseAddr = myBits[index];
			(**(myCGrafPort[index]).portPixMap).rowBytes = offRowBytes + 0x8000;
			(**(myCGrafPort[index]).portPixMap).bounds = bRect[index];
			
			myCGrafPort[index].portRect = bRect[index];
			
			ourCMHandle[index] = (**(**gBiggestDevice).gdPMap).pmTable;
			err = HandToHand(&ourCMHandle[index]);
			if(err != noErr)
			{
				DisposePtr((**(myCGrafPort[index]).portPixMap).baseAddr);
				CloseCPort(myCGrafPtr[index]);
				myCGrafPtr[index]=0L;
				ErrorString("\pThere is not enough memory to open the window.","\p");
			}
			
			for(i = 0; i <= (**ourCMHandle[index]).ctSize; i++)
				(**ourCMHandle[index]).ctTable[i].value = i;
			(**ourCMHandle[index]).ctFlags &= 0x7fff;
			(**ourCMHandle[index]).ctSeed = GetCTSeed();
			
			(**(myCGrafPort[index]).portPixMap).pmTable = ourCMHandle[index];
			
			if (oldDevice)
				SetGDevice(oldDevice);
		}
		
		UpdateTheWindowColor(index);
	}
	else
	{
		if (myGrafPtr[index]==0L)
		{
			if(myCGrafPtr[index] != 0L)
			{
				DisposeHandle((**(myCGrafPort[index]).portPixMap).pmTable);
				DisposePtr((**(myCGrafPort[index]).portPixMap).baseAddr);
				CloseCPort(myCGrafPtr[index]);
				myCGrafPtr[index] = 0L;
				gOffscreenNeedsUpdate[index]=TRUE;
			}
			
			myGrafPtr[index] = &myGrafPort[index];
			OpenPort(myGrafPtr[index]);
			
			offRowBytes = (((bRect[index].right - bRect[index].left) + 15) >> 4) << 1;
			sizeOfOff = (long)(bRect[index].bottom - bRect[index].top) * offRowBytes;
			OffsetRect(&bRect[index], -bRect[index].left, -bRect[index].top);
			
			myBits[index] = NewPtr(sizeOfOff);
			if(myBits[index] == 0L)
			{
				ClosePort(myGrafPtr[index]);
				myGrafPtr[index]=0L;
				ErrorString("\pThere is not enough memory to open the window.", "\p");
			}
			
			myGrafPort[index].portBits.baseAddr = myBits[index];
			myGrafPort[index].portBits.rowBytes = offRowBytes;
			myGrafPort[index].portBits.bounds = bRect[index];
			myGrafPort[index].portRect = bRect[index];			
		}
		
		UpdateTheWindowBW(index);
	}
	
	ValidRect(&(gTheWindow[index]->portRect));
}

void UpdateTheWindowColor(int index)
{
	GDHandle	oldDevice;
	RgnHandle	oldClipRgn;
	RgnHandle	newClipRgn;
	
	if (gOffscreenNeedsUpdate[index])
	{
		oldDevice = GetGDevice();
		SetGDevice(gBiggestDevice);
		oldClipRgn = myCGrafPort[index].clipRgn;
		newClipRgn=NewRgn();
		SetRectRgn(newClipRgn, 0, 0, gWindowWidth[index], gWindowHeight[index]);
		
		myCGrafPort[index].clipRgn=newClipRgn;
			
		SetPort((GrafPtr)myCGrafPtr[index]);
		
		switch (index)
		{
			case kAbout:
				DrawTheAboutBox(TRUE);
				break;
			case kAboutMSG:
				DrawTheCarpet();
				break;
			case kHelp:
				DrawTheHelp(TRUE);
				break;
		}
		SetGDevice(oldDevice);
		myCGrafPort[index].clipRgn = oldClipRgn;
		DisposeRgn(newClipRgn);
		gOffscreenNeedsUpdate[index]=FALSE;
	}
	
	SetPort(gTheWindow[index]);
	
	CopyBits(&(((GrafPtr)myCGrafPtr[index])->portBits),
				&(gTheWindow[index]->portBits), &bRect[index], &bRect[index], 0, 0L);
}

void UpdateTheWindowBW(int index)
{
	RgnHandle	oldClipRgn;
	RgnHandle	newClipRgn;
	
	if (gOffscreenNeedsUpdate[index])
	{
		oldClipRgn = myGrafPort[index].clipRgn;
		newClipRgn=NewRgn();
		SetRectRgn(newClipRgn, 0, 0, gWindowWidth[index], gWindowHeight[index]);
		myCGrafPort[index].clipRgn=newClipRgn;
		SetPort(myGrafPtr[index]);
		switch (index)
		{
			case kAbout:
				DrawTheAboutBox(FALSE);
				break;
			case kAboutMSG:
				DrawTheCarpet();
				break;
			case kHelp:
				DrawTheHelp(FALSE);
				break;
		}
		myGrafPort[index].clipRgn = oldClipRgn;
		DisposeRgn(newClipRgn);
		gOffscreenNeedsUpdate[index]=FALSE;
	}
	
	SetPort(gTheWindow[index]);
	
	CopyBits(&(myGrafPtr[index]->portBits),
				&(gTheWindow[index]->portBits), &bRect[index], &bRect[index], 0, 0L);
}

void UpdateHelpWindow(void)
{
	gOffscreenNeedsUpdate[kHelp]=TRUE;
	OpenTheWindow(kHelp);
}

void CloseTheWindow(int index)
{
	DisposeWindow(gTheWindow[index]);
	gTheWindow[index]=0L;
	
	if (index==kHelp)
		gOffscreenNeedsUpdate[kHelp]=TRUE;
}

void DrawThePicture(PicHandle *thePict, int whichPict, int x, int y)
{
	Rect			temp;
	
	if (*thePict==0L)
		*thePict=(PicHandle)GetPicture(whichPict);
	
	HLock(*thePict);
	temp.top=y;
	temp.left=x;
	temp.bottom=temp.top+(***thePict).picFrame.bottom-(***thePict).picFrame.top;
	temp.right=temp.left+(***thePict).picFrame.right-(***thePict).picFrame.left;
	DrawPicture(*thePict, &temp);
	HUnlock(*thePict);
}

void ReleaseThePict(PicHandle *thePict)
{
	if (*thePict!=0L)
	{
		ReleaseResource(*thePict);
		*thePict=0L;
	}
}

void ShutDownMSGGraphics(void)
{
	int				i;
	
	ReleaseThePict(gAboutColorPict);
	ReleaseThePict(gAboutBWPict);
	
	for (i=0; i<NUM_WINDOWS; i++)
	{
		if ((myCGrafPtr[i]!=0L) || (myGrafPtr[i]!=0L))
			DisposPtr(myBits[i]);
		if(myCGrafPtr[i] != 0L)
		{
			DisposeHandle((**(myCGrafPort[i]).portPixMap).pmTable);
			CloseCPort(myCGrafPtr[i]);
			myCGrafPtr[i] = 0L;
		}
		if(myGrafPtr[i] != 0L)
		{
			ClosePort(myGrafPtr[i]);
			myGrafPtr[i] = 0L;
		}
	}
}
