/**********************************************************************\

File:		graphics.c

Purpose:	This module handles opening/closing/updating all windows:
			this includes manipulating offscreen GWorlds & bitmaps
			for fun and profit.

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

#include "graphics.h"
#include "about.h"
#include "about MSG.h"
#include "help.h"
#include "dialogs.h"
#include "error.h"
#include "menus.h"
#include "environment.h"
#include "prefs.h"
#include "util.h"
#include "program globals.h"

/* internal global variables for use by graphics.c only */
static	ExtendedWindowDataHandle	gTheWindowData[NUM_WINDOWS];
static	Rect		gBoundsRect[NUM_WINDOWS];		/* rectangle of offscreen bitmap */
static	Rect		gMainScreenBounds;				/* bounds of main monitor */
static	GWorldPtr	gTheGWorld[NUM_WINDOWS];		/* offscreen graphics world */
static	Ptr			gBWBitMap[NUM_WINDOWS];			/* offscreen bitmap for B/W machines */
static	GrafPort	gBWGrafPort[NUM_WINDOWS];		/* offscreen grafport "  "     "     */
static	GrafPtr		gBWGrafPtr[NUM_WINDOWS];		/* offscreen grafptr  "  "     "     */
static	GWorldPtr	currentGWorld;
static	GDHandle	currentGDHandle;

Boolean InitTheGraphics(void)
{
	short			i,j;
	
	GetMainScreenBounds();
	
	for (i=0; i<NUM_WINDOWS; i++)
	{
		/* nothing is inited; if there's an error later on, we'll know how much to */
		/* clean up in ShutDownTheGraphics() */
		gTheGWorld[i]=(GWorldPtr)0L;
		gTheWindowData[i]=(ExtendedWindowDataHandle)0L;
	}
	
	for (i=0; i<NUM_WINDOWS; i++)
	{
		gTheWindowData[i]=(ExtendedWindowDataHandle)NewHandle(sizeof(ExtendedWindowDataRec));
		if (gTheWindowData[i]==0L)							/* return if error */
			return FALSE;
		
		(**(gTheWindowData[i])).offscreenNeedsUpdate=TRUE;	/* offscreen not inited */
		(**(gTheWindowData[i])).theWindowPtr=0L;			/* window ptr not inited */
		(**(gTheWindowData[i])).windowIndex=i;				/* so we can retrieve it O(1) */
		(**(gTheWindowData[i])).windowDepth=
			(**(gTheWindowData[i])).maxDepth=1;				/* init at B/W */
		(**(gTheWindowData[i])).isColor=FALSE;				/* init to grayscale */
		for (j=0; j<MAX_TE_HANDLES; j++)
			(**(gTheWindowData[i])).hTE[j]=0L;
		HLockHi((Handle)gTheWindowData[i]);
	}
	
	/* set window dispatch routines */
	SetIndDispatchProc(kAbout, AboutBoxDispatch);
	SetIndDispatchProc(kHelp, HelpWindowDispatch);
	SetIndDispatchProc(kAboutMSG, AboutMSGBoxDispatch);
	
	/* call window dispatch routines with "startup" message */
	CallIndDispatchProc(kAbout, kStartup, 0L);
	CallIndDispatchProc(kAboutMSG, kStartup, 0L);
	
	return TRUE;
}

void OpenTheIndWindow(short index)
{
	WindowPtr		theWindow;
	
	if (!((**gTheWindowData[index]).theWindowPtr))		/* if window exists, we'll just update it (see below) */
	{
		if (CallIndDispatchProc(index, kInitialize, 0L)==kFailure)
		{		/* default is to center window on main screen */
			(**(gTheWindowData[index])).initialTopLeft.h =
				gMainScreenBounds.left + (((gMainScreenBounds.right -
				gMainScreenBounds.left) - (**(gTheWindowData[index])).windowWidth) / 2);
			(**(gTheWindowData[index])).initialTopLeft.v =
				gMainScreenBounds.top + (((gMainScreenBounds.bottom -
				gMainScreenBounds.top) - (**(gTheWindowData[index])).windowHeight) / 2);
		}
		
		(**(gTheWindowData[index])).windowBounds.left=
			(**(gTheWindowData[index])).initialTopLeft.h;
		
		(**(gTheWindowData[index])).windowBounds.top=
			(**(gTheWindowData[index])).initialTopLeft.v;
			
		if (((**(gTheWindowData[index])).windowType==noGrowDocProc) ||
			((**(gTheWindowData[index])).windowType==documentProc) ||
			((**(gTheWindowData[index])).windowType==movableDBoxProc) ||
			((**(gTheWindowData[index])).windowType==zoomDocProc) ||
			((**(gTheWindowData[index])).windowType==zoomNoGrow) ||
			((**(gTheWindowData[index])).windowType==rDocProc))
				(**(gTheWindowData[index])).windowBounds.top += 9;	/* compensate for title */
		
		/* don't put window over menu bar */
		if ((**(gTheWindowData[index])).windowBounds.top < GetMBarHeight()+1)
			(**(gTheWindowData[index])).windowBounds.top = GetMBarHeight()+1;
		
		(**(gTheWindowData[index])).windowBounds.bottom =
			(**(gTheWindowData[index])).windowBounds.top +
			(**(gTheWindowData[index])).windowHeight;
		
		(**(gTheWindowData[index])).windowBounds.right =
			(**(gTheWindowData[index])).windowBounds.left +
			(**(gTheWindowData[index])).windowWidth;
		
		KillOffscreen(index);		/* kill offscreen bitmaps that are left over */
		
		if (gHasColorQD)
		{
			/* create the color window with our specs, see IM Essentials 4-79ff */
			(**gTheWindowData[index]).theWindowPtr=
				NewCWindow(0L, &((**(gTheWindowData[index])).windowBounds),
				(**(gTheWindowData[index])).windowTitle, FALSE,
				(**(gTheWindowData[index])).windowType, (WindowPtr)-1L,
				(**(gTheWindowData[index])).hasCloseBox,
				(unsigned long)gTheWindowData[index]);
		}
		else
		{
			/* create the B/W window with our specs, see IM Essentials 4-82ff */
			(**gTheWindowData[index]).theWindowPtr=
				NewWindow(0L, &((**(gTheWindowData[index])).windowBounds),
				(**(gTheWindowData[index])).windowTitle, FALSE,
				(**(gTheWindowData[index])).windowType, (WindowPtr)-1L,
				(**(gTheWindowData[index])).hasCloseBox,
				(unsigned long)gTheWindowData[index]);
		}
	}
	
	if ((theWindow=GetIndWindowGrafPtr(index))!=0L)
	{
		ShowWindow(theWindow);			/* immediately show this new window */
		SelectWindow(theWindow);		/* immediately select this new window */
		SetPort(theWindow);			/* important! for TE info to stick*/
		/* call window's dispatch routine to alert it that it's open now */
		CallIndDispatchProc(index, kOpen, 0L);
		UpdateTheWindow((ExtendedWindowDataHandle)gTheWindowData[index]);	/* immediately update this new window */
	}
	else HandleError(kNoMemory, FALSE);			/* if unsuccessful, display error */
}

void GetMainScreenBounds(void)
{
	gMainScreenBounds = screenBits.bounds;		/* low-mem global */
	gMainScreenBounds.top += GetMBarHeight();	/* don't include menu bar */
}

short GetBiggestDeviceDepth(ExtendedWindowDataHandle theData)
{
	short			index;
	Rect			tempRect;
	long			biggestSize;
	long			tempSize;
	GDHandle		thisHandle, gBiggestDevice;
	
	if (!gHasColorQD)
		return 1;
	
	index=(**theData).windowIndex;
	
	if (!GetIndWindowGrafPtr(index))
		return (**(**GetMainDevice()).gdPMap).pixelSize;
	
	thisHandle = GetDeviceList();
	gBiggestDevice = 0L;
	biggestSize = 0L;
	
	while (thisHandle)
	{
		if (TestDeviceAttribute(thisHandle, screenDevice) &&
			TestDeviceAttribute(thisHandle, screenActive))
		{
			if (SectRect(&(GetIndWindowGrafPtr(index)->portRect), &((**thisHandle).gdRect),
					&tempRect))
			{
				if (biggestSize < (tempSize = ((long)(tempRect.bottom - tempRect.top))*
					((long)(tempRect.right - tempRect.left))))
				{
					biggestSize = tempSize;
					gBiggestDevice = thisHandle;
				}
			}
		}
		thisHandle = GetNextDevice(thisHandle);
	}
	
	return (gBiggestDevice) ? (**(**gBiggestDevice).gdPMap).pixelSize : 1;
}

short GetWindowDepth(ExtendedWindowDataHandle theData)
{
	short			index;
	
	index=(**theData).windowIndex;
	/* if Color Quickdraw is not available, the depth must be 1. */
	/* if Color Quickdraw is available and the window exists, return the window's
	   GWorld's graphics device's pixel map's pixel depth */
	/* if Color Quickdraw is available and the window does not exist, return the
	   pixel depth of the main screen */
	return (gHasColorQD) ? ((GetIndWindowGrafPtr(index)) ?
			(**(**(GetGWorldDevice(gTheGWorld[index]))).gdPMap).pixelSize :
			(**(**GetMainDevice()).gdPMap).pixelSize) : 1;
}

Boolean WindowIsColor(ExtendedWindowDataHandle theData)
{
	short			index;
	
	index=(**theData).windowIndex;
	return (gHasColorQD) ? ((GetWindowDepth(theData)>8) ? TRUE :
		TestDeviceAttribute(GetGWorldDevice(gTheGWorld[index]), gdDevType)) : FALSE;
}

void UpdateTheWindow(ExtendedWindowDataHandle theData)
{
	short			index;
	long			offRowBytes, sizeOfOff;
	unsigned long	updateResult;
	Boolean			isColor;
	PixMapHandle	thePixMapHandle;
	short			TEindex;
	
	index=(**theData).windowIndex;
	gBoundsRect[index]=GetIndWindowGrafPtr(index)->portRect;
	OffsetRect(&gBoundsRect[index], -gBoundsRect[index].left, -gBoundsRect[index].top);

	if (gHasColorQD)	/* w/o Color Quickdraw, GWorlds may not be supported */
	{
		if (gTheGWorld[index]==0L)		/* create new graphics world if none exists */
		{
			/* try to create new graphics world; display error if unsuccessful */
			if (NewGWorld(&gTheGWorld[index],
				(GetBiggestDeviceDepth(theData)>=(**theData).maxDepth) ?
				(**theData).maxDepth : 0, &gBoundsRect[index], 0L, 0L, 0)!=0)
			{
				HandleError(kNoMemory, TRUE);		/* quits */
			}
			
			(**theData).windowDepth=GetWindowDepth(theData);
			NoPurgePixels(GetGWorldPixMap(gTheGWorld[index]));	/* never purge our pixmap! */
			updateResult=1;
		}
		else updateResult=0;
		
		GetGWorld(&currentGWorld, &currentGDHandle);	/* get current settings */
		LockPixels(thePixMapHandle=GetGWorldPixMap(gTheGWorld[index]));	/* important!  copybits may move mem */
		/* update offscreen graphics world, compensating for change in pixel depth */
		updateResult|=(unsigned long)UpdateGWorld(&gTheGWorld[index],
			(GetBiggestDeviceDepth(theData)>=(**theData).maxDepth) ? (**theData).maxDepth :
			0, &gBoundsRect[index], 0L, 0L, 0);
		SetGWorld(gTheGWorld[index], 0L);				/* set to our offscreen gworld */
		
		isColor=WindowIsColor(theData);
		if (isColor!=(**theData).isColor)
		{
			(**theData).isColor=isColor;
			updateResult=1;
		}
		
		if ((updateResult!=0L) || ((**theData).windowDepth!=GetWindowDepth(theData)))
		{
			(**theData).windowDepth=GetWindowDepth(theData);	/* save new depth */
			(**theData).offscreenNeedsUpdate=TRUE;				/* we'll need to redraw */
			CallDispatchProc(theData, kChangeDepth, 0L);
		}
	}
	else	/* deal with (guaranteed) B/W bitmaps manually */
	{
		if (gBWGrafPtr[index]==0L)	/* create new offscreen bitmap if none exists */
		{
			gBWGrafPtr[index]=&gBWGrafPort[index];
			OpenPort(gBWGrafPtr[index]);	/* make a new port */
			
			/* calculate the size of the offscreen bitmap from the boundsrect */
			offRowBytes=(((gBoundsRect[index].right-gBoundsRect[index].left)+15)>>4)<<1;
			sizeOfOff=(long)(gBoundsRect[index].bottom-gBoundsRect[index].top)*offRowBytes;
			
			gBWBitMap[index]=NewPtr(sizeOfOff);		/* allocate space for bitmap */
			if (gBWBitMap[index]==0L)				/* abort if unsuccessful */
			{
				ClosePort(gBWGrafPtr[index]);		/* cleaning up... */
				gBWGrafPtr[index]=0L;
				HandleError(kNoMemory, TRUE);		/* displaying error... */
			}
			
			gBWGrafPort[index].portBits.baseAddr=gBWBitMap[index];	/* --> our bitmap */
			gBWGrafPort[index].portBits.rowBytes=offRowBytes;		/* bitmap size */
			gBWGrafPort[index].portBits.bounds=						/* bitmap bounds */
				gBWGrafPort[index].portRect=gBoundsRect[index];
			
			SetPort(gBWGrafPtr[index]);
			(**theData).offscreenNeedsUpdate=TRUE;
		}
		else SetPort(gBWGrafPtr[index]);			/* set port for subsequent drawing */
	}	
	
	if ((**theData).offscreenNeedsUpdate)			/* if we need to redraw */
	{
		(**theData).offscreenNeedsUpdate=FALSE;		/* not anymore */
		/* call window's dispatch and tell it to redraw itself */
		CallDispatchProc(theData, kUpdate, GetWindowDepth(theData));
	}
	
	if (gHasColorQD)
		SetGWorld(currentGWorld, currentGDHandle);	/* restore old settings */
	
	SetPort(GetWindowGrafPtr((WindowDataHandle)theData));
	
	/* copy offscreen bitmap from graphics world or bitmap to onscreen window */
	if (CallDispatchProc(theData, kCopybits, gHasColorQD ? (unsigned long)gTheGWorld[index] :
		(unsigned long)gBWGrafPtr[index])==kFailure)
		CopyBits(gHasColorQD ? &(((GrafPtr)gTheGWorld[index])->portBits) :
					&(gBWGrafPtr[index]->portBits), &(GetIndWindowGrafPtr(index)->portBits),
					&gBoundsRect[index], &gBoundsRect[index], 0, 0L);
	
	for (TEindex=0; TEindex<MAX_TE_HANDLES; TEindex++)
	{
		if ((**theData).hTE[TEindex]!=0L)
			TEUpdate(&(GetIndWindowGrafPtr(TEindex)->portRect), (**theData).hTE[TEindex]);
	}

	if (gHasColorQD)
		UnlockPixels(thePixMapHandle);	/* remember we locked these? */
	
	ValidRect(&(GetIndWindowGrafPtr(index)->portRect));		/* so we don't reupdate */
	
	if (index!=kMainWindow)
		KillOffscreen(index);
}

Boolean CloseTheWindow(ExtendedWindowDataHandle theData)
{
	short			index;
	
	index=(**theData).windowIndex;
	
	/* if the window's dispatch cancels the close, abort */
	if (CallDispatchProc(theData, kClose, 0L)==kCancel)
		return FALSE;
	
	DisposeWindow(GetIndWindowGrafPtr(index));	/* get rid of the actual window in memory */
	(**gTheWindowData[index]).theWindowPtr=0L;	/* so _we_ know the window doesn't exist */
	KillOffscreen(index);				/* kill offscreen bitmaps left over */
	
	/* tell window's dispatch that it's disposed of now */
	CallDispatchProc(theData, kDispose, 0L);
	
	return TRUE;	/* successful close */
}

Boolean CloseTheIndWindow(short index)
{
	return CloseTheWindow(gTheWindowData[index]);
}

PicHandle DrawThePicture(PicHandle thePict, short whichPict, short x, short y)
/* a standard routine for loading a picture (if necessary) and then drawing it */
{
	Rect			temp;
	
	if (thePict==0L)		/* get it if it doesn't exist */
		thePict=(PicHandle)GetPicture(whichPict);
	
	HLock((Handle)thePict);		/* lock it down for dereferencing to get picture bounds */
	temp.top=y;
	temp.left=x;
	temp.bottom=temp.top+(**thePict).picFrame.bottom-(**thePict).picFrame.top;
	temp.right=temp.left+(**thePict).picFrame.right-(**thePict).picFrame.left;
	DrawPicture(thePict, &temp);	/* draw picture at (x,y) */
	HUnlock((Handle)thePict);		/* unlock for better memory management */
	return thePict;
}

PicHandle ReleaseThePict(PicHandle thePict)
{
	if (thePict!=0L)	/* if exists, release it */
		ReleaseResource((Handle)thePict);
	return 0L;
}

Boolean SetPortToOffscreen(WindowDataHandle theData)
{
	short			index;
	
	index=(**theData).windowIndex;
	
	if (gHasColorQD)
	{
		if (gTheGWorld[index]==0L)
			return FALSE;
		GetGWorld(&currentGWorld, &currentGDHandle);	/* get current settings */
		LockPixels(GetGWorldPixMap(gTheGWorld[index]));	/* important!  copybits may move mem */
		SetGWorld(gTheGWorld[index], 0L);
	}
	else
	{
		if (gBWGrafPtr[index]==0L)
			return FALSE;
		SetPort(gBWGrafPtr[index]);
	}
	
	return TRUE;
}

void RestorePortToScreen(WindowDataHandle theData)
{
	if (gHasColorQD)
	{
		SetGWorld(currentGWorld, currentGDHandle);	/* restore old settings */
		UnlockPixels(GetGWorldPixMap(gTheGWorld[(**theData).windowIndex]));
	}
	
	SetPort(GetWindowGrafPtr(theData));
}

GrafPtr GetOffscreenGrafPtr(WindowDataHandle theData)
{
	if (gHasColorQD)
		return (GrafPtr)gTheGWorld[(**theData).windowIndex];
	else
		return gBWGrafPtr[(**theData).windowIndex];
}

GrafPtr GetIndOffscreenGrafPtr(short index)
{
	return GetOffscreenGrafPtr((WindowDataHandle)gTheWindowData[index]);
}

GrafPtr GetWindowGrafPtr(WindowDataHandle theData)
{
	return (GrafPtr)((**theData).theWindowPtr);
}

GrafPtr GetIndWindowGrafPtr(short index)
{
	return GetWindowGrafPtr((WindowDataHandle)gTheWindowData[index]);
}

ExtendedWindowDataHandle GetIndWindowDataHandle(short index)
{
	return gTheWindowData[index];
}

void SetIndDispatchProc(short index, ProcPtr theProc)
{
	(**gTheWindowData[index]).dispatchProc=theProc;
}

short CallIndDispatchProc(short index, short theMessage, unsigned long misc)
{
	return ((**(gTheWindowData[index])).dispatchProc)((WindowDataHandle)gTheWindowData[index], theMessage, misc);
}

short CallDispatchProc(ExtendedWindowDataHandle theData, short theMessage, unsigned long misc)
{
	return ((**theData).dispatchProc)((WindowDataHandle)theData, theMessage, misc);
}

void SetIndWindowTitle(short index, Str255 theTitle)
{
	Mymemcpy((Ptr)((**(gTheWindowData[index])).windowTitle), (Ptr)theTitle, theTitle[0]+1);
}

void KillOffscreen(short index)
{
	if (gTheGWorld[index]!=0L)
		DisposeGWorld(gTheGWorld[index]);
	if (gBWGrafPtr[index]!=0L)
		DisposePtr((Ptr)gBWGrafPtr[index]);
	gTheGWorld[index]=0L;
	gBWGrafPtr[index]=0L;
	(**(gTheWindowData[index])).offscreenNeedsUpdate=TRUE;
}

void ShutDownTheGraphics(void)
{
	short			i;
	
	/* send shutdown messages to the shell's windows */
	CallIndDispatchProc(kAbout, kShutdown, 0L);
	CallIndDispatchProc(kHelp, kShutdown, 0L);
	CallIndDispatchProc(kAboutMSG, kShutdown, 0L);
	
	for (i=0; i<NUM_WINDOWS; i++)
	{
		KillOffscreen(i);
		if (GetIndWindowGrafPtr(i)!=0L)
			DisposeWindow((**gTheWindowData[i]).theWindowPtr);
		if (gTheWindowData[i]!=0L)
			DisposeHandle((Handle)gTheWindowData[i]);
	}
}
