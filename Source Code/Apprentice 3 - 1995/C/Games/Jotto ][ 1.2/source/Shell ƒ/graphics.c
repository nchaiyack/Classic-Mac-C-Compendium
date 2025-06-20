#include "graphics.h"
#include "graphics dispatch.h"
#include "window layer.h"
#include <QDOffscreen.h>

extern	Boolean			gHasColorQD;	/* see environment.c */

static	void GetMainScreenBounds(Rect *screenRect);
static	short GetBiggestDeviceDepth(WindowPtr theWindow);
static	short GetWindowRealDepth(WindowPtr theWindow);

OSErr OpenTheIndWindow(short index)
{
	WindowPtr		theWindow;
	Point			topLeft;
	ExtendedWindowPtr	storage;
	Rect			screenRect;
	Rect			boundsRect;
	short			windowType;
	
	if (!IndWindowExistsQQ(index))
	{
		storage=(ExtendedWindowPtr)NewPtr(sizeof(ExtendedWindowRec));
		if (storage==0L)
			return MemError();
		
		theWindow=(WindowPtr)storage;
		SetupWindowDispatch(index, theWindow);
		windowType=GetWindowType(theWindow);
		
		if (WindowAutoCenterQQ(theWindow))
		{
			GetMainScreenBounds(&screenRect);
			topLeft.h=screenRect.left+
				(((screenRect.right-screenRect.left)-GetWindowWidth(theWindow))/2);
			topLeft.v=screenRect.top+
				(((screenRect.bottom-screenRect.top)-GetWindowHeight(theWindow))/2);
			SetWindowTopLeft(theWindow, topLeft);
			if ((windowType==noGrowDocProc) || (windowType==documentProc) ||
				(windowType==movableDBoxProc) || (windowType==zoomDocProc) ||
				(windowType==zoomNoGrow) || (windowType==rDocProc))
			{
				topLeft.v+=9;
			}
		}
		else
		{
			topLeft=GetWindowTopLeft(theWindow);
		}
		
		if (topLeft.v<GetMBarHeight()+1)
			topLeft.v=GetMBarHeight()+1;
		
		SetRect(&boundsRect, topLeft.h, topLeft.v, topLeft.h+GetWindowWidth(theWindow),
			topLeft.v+GetWindowHeight(theWindow));
		SetWindowBounds(theWindow, boundsRect);
		
		if (gHasColorQD)
		{
			if (WindowIsFloatQQ(theWindow))
			{
				theWindow=MyNewFloatCWindow(storage, &(GetWindowBounds(theWindow)),
					GetWindowTitle(theWindow), FALSE, windowType, (WindowPtr)-1L,
					WindowHasCloseBoxQQ(theWindow), 0L);
			}
			else
			{
				theWindow=MyNewCWindow(storage, &(GetWindowBounds(theWindow)),
					GetWindowTitle(theWindow), FALSE, windowType, (WindowPtr)-1L,
					WindowHasCloseBoxQQ(theWindow), 0L);
			}
		}
		else
		{
			if (WindowIsFloatQQ(theWindow))
			{
				theWindow=MyNewFloatWindow(storage, &(GetWindowBounds(theWindow)),
					GetWindowTitle(theWindow), FALSE, windowType, (WindowPtr)-1L,
					WindowHasCloseBoxQQ(theWindow), 0L);
			}
			else
			{
				theWindow=MyNewWindow(storage, &(GetWindowBounds(theWindow)),
					GetWindowTitle(theWindow), FALSE, windowType, (WindowPtr)-1L,
					WindowHasCloseBoxQQ(theWindow), 0L);
			}
		}
	}
	
	if (IndWindowExistsQQ(index))
	{
		theWindow=GetIndWindowPtr(index);
		MySelectWindow(theWindow);		/* immediately select this new window */
		SetPort(theWindow);				/* important! for TE info to stick*/
		InvalRect(&(theWindow->portRect));
		/* call window's dispatch routine to alert it that it's open now */
		OpenWindowDispatch(index);
//		ActivateWindowDispatch(index);
	}
	else return -1;
	
	return noErr;
}

OSErr UpdateTheWindow(WindowPtr theWindow)
{
	short			index;
	long			offRowBytes, sizeOfOff;
	PixMapHandle	thePixMapHandle;
	GWorldPtr		currentGWorld;
	GDHandle		currentGDHandle;
	Rect			boundsRect;
	GWorldPtr		theGWorld;
	Ptr				bwBitMap;
	GrafPort		bwGrafPort;
	GrafPtr			bwGrafPtr;
	OSErr			memError;
	TEHandle		hTE;
	short			realWindowDepth, maxWindowDepth;
	
	index=GetWindowIndex(theWindow);
	realWindowDepth=GetWindowRealDepth(theWindow);
	maxWindowDepth=GetWindowMaxDepth(theWindow);
	boundsRect=theWindow->portRect;
	OffsetRect(&boundsRect, -boundsRect.left, -boundsRect.top);

	if (gHasColorQD)	/* w/o Color Quickdraw, GWorlds may not be supported */
	{
		/* try to create new graphics world; display error if unsuccessful */
		if (NewGWorld(&theGWorld, (realWindowDepth>=maxWindowDepth) ? maxWindowDepth : 0,
				&boundsRect, 0L, 0L, 0)!=0)
		{
			return MemError();
		}
		
		NoPurgePixels(GetGWorldPixMap(theGWorld));	/* never purge our pixmap! */
		
		GetGWorld(&currentGWorld, &currentGDHandle);	/* get current settings */
		LockPixels(thePixMapHandle=GetGWorldPixMap(theGWorld));	/* important!  copybits may move mem */
		/* update offscreen graphics world, compensating for change in pixel depth */
		UpdateGWorld(&theGWorld, (realWindowDepth>=maxWindowDepth) ? maxWindowDepth : 0,
				&boundsRect, 0L, 0L, 0);
		SetGWorld(theGWorld, 0L);				/* set to our offscreen gworld */
		
		if ((**(**(GetGWorldDevice(theGWorld))).gdPMap).pixelSize!=GetWindowDepth(theWindow))
			ChangeDepthDispatch(index, GetWindowDepth(theWindow));
	}
	else	/* deal with (guaranteed) B/W bitmaps manually */
	{
		bwGrafPtr=&bwGrafPort;
		OpenPort(bwGrafPtr);
		offRowBytes=(((boundsRect.right-boundsRect.left)+15)>>4)<<1;
		sizeOfOff=(long)(boundsRect.bottom-boundsRect.top)*offRowBytes;
		bwBitMap=NewPtr(sizeOfOff);
		if (bwBitMap==0L)
		{
			memError=MemError();
			ClosePort(bwGrafPtr);
			return memError;
		}
		
		bwGrafPort.portBits.baseAddr=bwBitMap;
		bwGrafPort.portBits.rowBytes=offRowBytes;
		bwGrafPort.portBits.bounds=bwGrafPort.portRect=boundsRect;
		
		SetPort(bwGrafPtr);
	}	
	
	SetWindowDepth(theWindow, (realWindowDepth>maxWindowDepth) ? maxWindowDepth : realWindowDepth);
	if (DrawWindowDispatch(index, GetWindowDepth(theWindow))==kFailure)
	{
		EraseRect(&(theWindow->portRect));
	}
	
	if (gHasColorQD)
		SetGWorld(currentGWorld, currentGDHandle);
	
	SetPort(theWindow);
	
	if (CopybitsDispatch(index, gHasColorQD ? (WindowPtr)theGWorld : (WindowPtr)bwGrafPtr)==kFailure)
	{
		CopyBits(gHasColorQD ?
					&(((WindowPtr)theGWorld)->portBits) :
					&(((WindowPtr)bwGrafPtr)->portBits),
				&(theWindow->portBits),
				&boundsRect,
				&boundsRect,
				0,
				0L);
		if ((GetWindowVScrollBar(theWindow)!=0L) || (GetWindowHScrollBar(theWindow)!=0L))
			UpdateControls(theWindow, theWindow->visRgn);
	}
	
	if ((hTE=GetWindowTE(theWindow))!=0L)
	{
		EraseRect(&((**hTE).viewRect));
		TEUpdate(&(theWindow->portRect), hTE);
	}
	
	ValidRect(&(theWindow->portRect));
	
	if (gHasColorQD)
	{
		UnlockPixels(thePixMapHandle);
		DisposeGWorld(theGWorld);
	}
	else
	{
		ClosePort(bwGrafPtr);
		DisposePtr(bwBitMap);
	}
	
	CompactMem(maxSize);
	
	return noErr;
}

Boolean CloseTheWindow(WindowPtr theWindow)
{
	short			index;
	
	index=GetWindowIndex(theWindow);
	
	if (CloseWindowDispatch(index)==kCancel)
		return FALSE;
	
	DisposeWindowDispatch(index);
	
	MyDisposeWindow(theWindow);
	
	return TRUE;	/* successful close */
}

/* -------------------------------------------- */
/* the rest of these are internal to graphics.c */

static	void GetMainScreenBounds(Rect *screenRect)
{
	*screenRect = qd.screenBits.bounds;		/* low-mem global */
	(*screenRect).top += GetMBarHeight();	/* don't include menu bar */
}

static	short GetBiggestDeviceDepth(WindowPtr theWindow)
{
	Rect			tempRect;
	long			biggestSize;
	long			tempSize;
	GDHandle		thisHandle, gBiggestDevice;
	
	if (!gHasColorQD)
		return 1;
	
	if (!theWindow)
		return (**(**GetMainDevice()).gdPMap).pixelSize;
	
	thisHandle = GetDeviceList();
	gBiggestDevice = 0L;
	biggestSize = 0L;
	
	while (thisHandle)
	{
		if (TestDeviceAttribute(thisHandle, screenDevice) &&
			TestDeviceAttribute(thisHandle, screenActive))
		{
			if (SectRect(&(theWindow->portRect),
					&((**thisHandle).gdRect), &tempRect))
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

static	short GetWindowRealDepth(WindowPtr theWindow)
{
	return (gHasColorQD) ?
				((theWindow) ?
					GetBiggestDeviceDepth(theWindow) :
					(**(**GetMainDevice()).gdPMap).pixelSize) :
			1;
}
