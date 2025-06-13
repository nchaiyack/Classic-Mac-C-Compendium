#include "graphics.h"
#include "graphics dispatch.h"
#include "window layer.h"
#include "offscreen layer.h"
#include "util.h"
#include "main.h"

extern	Boolean			gHasColorQD;	/* see environment.c */

static	MyOffscreenPtr		gPreallocatedOffscreenPtr=0L;

OSErr OpenTheIndWindow(short index)
{
	WindowPtr		theWindow;
	Point			topLeft;
	ExtendedWindowPtr	storage;
	Rect			screenRect;
	Rect			boundsRect;
	WindowAttributes	windowAttrs;
	
	if (!IndWindowExistsQQ(index))
	{
		storage=(ExtendedWindowPtr)NewPtr(sizeof(ExtendedWindowRec));
		if (storage==0L)
			return MemError();
		
		theWindow=(WindowPtr)storage;
		SetupWindowDispatch(index, theWindow);
		windowAttrs=GetWindowAttributes(theWindow);
		
		if (WindowAutoCenterQQ(theWindow))
		{
			GetMainScreenBounds(&screenRect);
			topLeft.h=screenRect.left+
				(((screenRect.right-screenRect.left)-GetWindowWidth(theWindow))/2);
			topLeft.v=screenRect.top+
				(((screenRect.bottom-screenRect.top)-GetWindowHeight(theWindow))/2);
			SetWindowTopLeft(theWindow, topLeft);

			if ((windowAttrs&kHasDocumentTitlebarMask) || (windowAttrs&kHasRoundedTitlebarMask) ||
				(windowAttrs&kHasModalBorderMask))
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
		
		theWindow=MyNewWindow(storage, &(GetWindowBounds(theWindow)), GetWindowTitle(theWindow),
			FALSE, GetWindowAttributes(theWindow), (WindowRef)-1L, 0L, WindowIsFloatQQ(theWindow));
	}
	
	if (IndWindowExistsQQ(index))
	{
		theWindow=GetIndWindowPtr(index);
		SetPort(theWindow);				/* important! for TE info to stick*/
		OpenWindowDispatch(index);
		MyShowWindow(theWindow);
		MySelectWindow(theWindow);		/* immediately select this new window */
		InvalRect(&(theWindow->portRect));
	}
	else return -1;
	
	return noErr;
}

GrafPtr PreallocateOffscreenWorld(WindowPtr theWindow)
{
	TEHandle		hTE;
	GrafPtr			offscreenPtr;
	Boolean			dummy;
	
	gPreallocatedOffscreenPtr=(MyOffscreenPtr)NewPtr(sizeof(MyOffscreenRec));
	if (gPreallocatedOffscreenPtr==0L)
		return 0L;
	
	if (AllocateOffscreenWorld(theWindow, gPreallocatedOffscreenPtr, &dummy)!=noErr)
		return 0L;
	
	offscreenPtr=gPreallocatedOffscreenPtr->offscreenPtr;
	
	if ((hTE=GetWindowTE(theWindow))!=0L)
		(**hTE).inPort=offscreenPtr;
	
	return offscreenPtr;
}

#define theGWorld		offscreenWorld.offscreenPtr
#define thePixMapHandle	offscreenWorld.pixMapHandle
#define bwBitMap		offscreenWorld.bwBitMap
#define bwGrafPort		offscreenWorld.bwGrafPort
#define bwGrafPtr		offscreenWorld.offscreenPtr
#define currentGWorld	offscreenWorld.currentGWorld
#define currentGDHandle	offscreenWorld.currentGDHandle

OSErr UpdateTheWindow(WindowPtr theWindow)
{
	OSErr			memError;
	Rect			boundsRect;
	TEHandle		hTE;
	short			realWindowDepth, maxWindowDepth;
	MyOffscreenRec	offscreenWorld;
	short			index;
	Boolean			depthChanged, goon;
	
	index=GetWindowIndex(theWindow);
	realWindowDepth=GetWindowRealDepth(theWindow);
	maxWindowDepth=GetWindowMaxDepth(theWindow);
	boundsRect=theWindow->portRect;
	OffsetRect(&boundsRect, -boundsRect.left, -boundsRect.top);
	
	if (gPreallocatedOffscreenPtr!=0L)
	{
		Mymemcpy((Ptr)&offscreenWorld, (Ptr)gPreallocatedOffscreenPtr, sizeof(MyOffscreenRec));
		DisposePtr((Ptr)gPreallocatedOffscreenPtr);
		gPreallocatedOffscreenPtr=0L;
	}
	else
	{
		if ((memError=AllocateOffscreenWorld(theWindow, &offscreenWorld, &depthChanged))!=noErr)
			return memError;
	}
	
	SetWindowDepth(theWindow, (realWindowDepth>=maxWindowDepth) ? maxWindowDepth : realWindowDepth);
	
	if (depthChanged)
	{
		goon=(ChangeDepthDispatch(index)==kFailure);
	}
	else
	{
		goon=TRUE;
	}
	
	if (goon)
	{
		if (DrawWindowDispatch(index, GetWindowDepth(theWindow))==kFailure)
		{
			EraseRect(&(theWindow->portRect));
		}
		
		if ((hTE=GetWindowTE(theWindow))!=0L)
		{
			(**hTE).inPort=gHasColorQD ? (GrafPtr)theGWorld : bwGrafPtr;
			EraseRect(&((**hTE).viewRect));
			TEUpdate(&(theWindow->portRect), hTE);
			(**hTE).inPort=theWindow;
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
		
		ValidRect(&(theWindow->portRect));
	}
	
	DisposeOffscreenWorld(&offscreenWorld);
	
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

void GetMainScreenBounds(Rect *screenRect)
{
	*screenRect = qd.screenBits.bounds;		/* low-mem global */
	(*screenRect).top += GetMBarHeight();	/* don't include menu bar */
}

short GetBiggestDeviceDepth(WindowPtr theWindow)
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

short GetWindowRealDepth(WindowPtr theWindow)
{
	return (gHasColorQD) ?
				((theWindow) ?
					GetBiggestDeviceDepth(theWindow) :
					(**(**GetMainDevice()).gdPMap).pixelSize) :
			1;
}
