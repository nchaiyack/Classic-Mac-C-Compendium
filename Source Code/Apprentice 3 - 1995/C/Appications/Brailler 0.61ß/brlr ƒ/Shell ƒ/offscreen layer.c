#include "offscreen layer.h"
#include "window layer.h"
#include "graphics.h"
#include "timing.h"

extern	Boolean			gHasColorQD;	/* see environment.c */

MyOffscreenComboPtr AllocateOffscreenCombo(WindowPtr theWindow, Rect *sourceRect, Rect *destRect,
	short scrollSpeed, Handle theText, short font, short size)
/* leaks memory on error */
{
	OSErr			oe;
	MyOffscreenComboPtr	offscreenComboPtr;
	Rect			scrollSourceRect, scrollDestRect;
	Boolean			dummy;
	
	offscreenComboPtr=(MyOffscreenComboPtr)NewPtr(sizeof(MyOffscreenComboRec));
	offscreenComboPtr->sourceRect=*sourceRect;
	offscreenComboPtr->destRect=*destRect;
	offscreenComboPtr->destWindow=theWindow;
	if ((oe=AllocatePartialOffscreenWorld(theWindow, &(offscreenComboPtr->offscreenWorldRec),
		sourceRect, FALSE, &dummy))!=noErr)
		return 0L;
	if ((oe=AllocatePartialOffscreenWorld(theWindow, &(offscreenComboPtr->patternWorldRec),
		sourceRect, FALSE, &dummy))!=noErr)
		return 0L;
	if ((oe=AllocatePartialOffscreenWorld(theWindow, &(offscreenComboPtr->scratchWorldRec),
		sourceRect, FALSE, &dummy))!=noErr)
		return 0L;
	if ((scrollSpeed!=0) && (theText!=0L))
	{
		offscreenComboPtr->scrollSpeed=scrollSpeed;
		offscreenComboPtr->theText=theText;
		scrollSourceRect=*sourceRect;
		scrollSourceRect.left+=scrollSpeed;
		scrollDestRect=*sourceRect;
		scrollDestRect.right-=scrollSpeed;
		offscreenComboPtr->scrollSourceRect=scrollSourceRect;
		offscreenComboPtr->scrollDestRect=scrollDestRect;
		offscreenComboPtr->scrollXPos=sourceRect->right-sourceRect->left-1;
		offscreenComboPtr->font=font;
		offscreenComboPtr->size=size;
		offscreenComboPtr->offset=0L;
	}
	else
	{
		offscreenComboPtr->scrollSpeed=0;
		offscreenComboPtr->theText=0L;
	}
	
	return offscreenComboPtr;
}

OSErr AllocatePartialOffscreenWorld(WindowPtr theWindow, MyOffscreenPtr offscreenWorldPtr,
	Rect *boundsRect, Boolean setPort, Boolean *depthChanged)
{
	long			offRowBytes, sizeOfOff;
	PixMapHandle	thePixMapHandle;
	GWorldPtr		theGWorld;
	Ptr				bwBitMap;
	GrafPort		bwGrafPort;
	GrafPtr			bwGrafPtr;
	OSErr			memError;
	short			realWindowDepth, maxWindowDepth;
	
	maxWindowDepth=GetWindowMaxDepth(theWindow);
	realWindowDepth=GetWindowRealDepth(theWindow);
//	OffsetRect(boundsRect, -boundsRect->left, -boundsRect->top);
	
	*depthChanged=FALSE;
	if (gHasColorQD)	/* w/o Color Quickdraw, GWorlds may not be supported */
	{
		/* try to create new graphics world; display error if unsuccessful */
		if (NewGWorld(&theGWorld, (realWindowDepth>=maxWindowDepth) ? maxWindowDepth : 0,
				boundsRect, 0L, 0L, 0)!=0)
		{
			return MemError();
		}
		
		NoPurgePixels(GetGWorldPixMap(theGWorld));	/* never purge our pixmap! */
		
		GetGWorld(&(offscreenWorldPtr->currentGWorld),
			&(offscreenWorldPtr->currentGDHandle));	/* get current settings */
		LockPixels(thePixMapHandle=GetGWorldPixMap(theGWorld));	/* important!  copybits may move mem */
		/* update offscreen graphics world, compensating for change in pixel depth */
		UpdateGWorld(&theGWorld, (realWindowDepth>=maxWindowDepth) ? maxWindowDepth : 0,
				boundsRect, 0L, 0L, 0);
		if (setPort)
			SetGWorld(theGWorld, 0L);				/* set to our offscreen gworld */
		else
			SetGWorld((GWorldPtr)offscreenWorldPtr->currentGWorld, offscreenWorldPtr->currentGDHandle);
		
		offscreenWorldPtr->offscreenPtr=(GrafPtr)theGWorld;
		offscreenWorldPtr->pixMapHandle=thePixMapHandle;
		offscreenWorldPtr->isColor=TRUE;

		if (((realWindowDepth>=maxWindowDepth) ? maxWindowDepth : realWindowDepth)!=GetWindowDepth(theWindow))
		{
			*depthChanged=TRUE;
		}
	}
	else	/* deal with (guaranteed) B/W bitmaps manually */
	{
		bwGrafPtr=&bwGrafPort;
		OpenPort(bwGrafPtr);
		offRowBytes=(((boundsRect->right-boundsRect->left)+15)>>4)<<1;
		sizeOfOff=(long)(boundsRect->bottom-boundsRect->top)*offRowBytes;
		bwBitMap=NewPtr(sizeOfOff);
		if (bwBitMap==0L)
		{
			memError=MemError();
			ClosePort(bwGrafPtr);
			return memError;
		}
		
		bwGrafPort.portBits.baseAddr=bwBitMap;
		bwGrafPort.portBits.rowBytes=offRowBytes;
		bwGrafPort.portBits.bounds=bwGrafPort.portRect=*boundsRect;
		
		if (setPort)
			SetPort(bwGrafPtr);
		else
			SetPort(theWindow);
		
		offscreenWorldPtr->offscreenPtr=bwGrafPtr;
		offscreenWorldPtr->bwBitMap=bwBitMap;
		offscreenWorldPtr->bwGrafPort=bwGrafPort;
		offscreenWorldPtr->isColor=FALSE;
	}
	
	return noErr;
}

OSErr AllocateOffscreenWorld(WindowPtr theWindow, MyOffscreenPtr offscreenWorldPtr,
	Boolean *depthChanged)
{
	return AllocatePartialOffscreenWorld(theWindow, offscreenWorldPtr, &(theWindow->portRect),
		TRUE, depthChanged);
}

void CopybitsCombo(MyOffscreenComboPtr offscreenComboPtr)
{
	Rect			sourceRect, destRect;
	GrafPtr			patternPtr;
	GrafPtr			sourcePtr;
	GrafPtr			comboPtr;
	WindowPtr		destWindow;
	
	sourceRect=offscreenComboPtr->sourceRect;
	destRect=offscreenComboPtr->destRect;
	destWindow=offscreenComboPtr->destWindow;
	sourcePtr=offscreenComboPtr->offscreenWorldRec.offscreenPtr;
	patternPtr=offscreenComboPtr->patternWorldRec.offscreenPtr;
	comboPtr=offscreenComboPtr->scratchWorldRec.offscreenPtr;

	CopyBits(&(sourcePtr->portBits), &(comboPtr->portBits), &sourceRect, &sourceRect, srcCopy, 0L);
	CopyBits(&(patternPtr->portBits), &(comboPtr->portBits), &sourceRect, &sourceRect, srcOr, 0L);
	CopyBits(&(comboPtr->portBits), &(destWindow->portBits), &sourceRect, &destRect, srcCopy, 0L);
}

void ScrollTextCombo(MyOffscreenComboPtr offscreenComboPtr)
{
	unsigned char	theChar;
	Rect			sourceRect;
	Rect			destRect;
	GrafPtr			patternPtr;
	GrafPtr			sourcePtr;
	GrafPtr			comboPtr;
	WindowPtr		destWindow;
	short			charWidth;
	
	StartTiming();
	
	sourceRect=offscreenComboPtr->sourceRect;
	destRect=offscreenComboPtr->destRect;
	destWindow=offscreenComboPtr->destWindow;
	sourcePtr=offscreenComboPtr->offscreenWorldRec.offscreenPtr;
	patternPtr=offscreenComboPtr->patternWorldRec.offscreenPtr;
	comboPtr=offscreenComboPtr->scratchWorldRec.offscreenPtr;

	SetPortToOffscreenWindow(&(offscreenComboPtr->offscreenWorldRec));
	CopyBits(&(sourcePtr->portBits), &(sourcePtr->portBits), &(offscreenComboPtr->scrollSourceRect),
			 &(offscreenComboPtr->scrollDestRect), 0, 0L);
	
	MoveTo(sourceRect.left+(offscreenComboPtr->scrollXPos), sourceRect.bottom-3);
	
	theChar=(*(offscreenComboPtr->theText))[offscreenComboPtr->offset];
	TextMode(notSrcCopy);
	TextFont(offscreenComboPtr->font);
	TextSize(offscreenComboPtr->size);
	
	charWidth=CharWidth(theChar);
	DrawChar(theChar);
	
	SetPortToOnscreenWindow(offscreenComboPtr->destWindow, &(offscreenComboPtr->offscreenWorldRec));
	
	CopyBits(&(sourcePtr->portBits), &(comboPtr->portBits), &sourceRect, &sourceRect, srcCopy, 0L);
	CopyBits(&(patternPtr->portBits), &(comboPtr->portBits), &sourceRect, &sourceRect, srcOr, 0L);
	CopyBits(&(comboPtr->portBits), &(destWindow->portBits), &sourceRect, &destRect, srcCopy, 0L);
	
	offscreenComboPtr->scrollXPos-=offscreenComboPtr->scrollSpeed;
	if (offscreenComboPtr->scrollXPos+charWidth<=(sourceRect.right-sourceRect.left-1-offscreenComboPtr->scrollSpeed))
	{
		offscreenComboPtr->scrollXPos+=charWidth;
		if ((++(offscreenComboPtr->offset))==GetHandleSize(offscreenComboPtr->theText))
			offscreenComboPtr->offset=0L;
	}
	TimeCorrection(2);
}

void SetPortToOnscreenWindow(WindowPtr theWindow, MyOffscreenPtr offscreenWorldPtr)
{
	if (offscreenWorldPtr->isColor)
	{
		SetGWorld(offscreenWorldPtr->currentGWorld, offscreenWorldPtr->currentGDHandle);
	}
	
	SetPort(theWindow);
}

void SetPortToOffscreenWindow(MyOffscreenPtr offscreenWorldPtr)
{
	if (offscreenWorldPtr->isColor)
	{
		SetGWorld((GWorldPtr)offscreenWorldPtr->offscreenPtr, 0L);
	}
	else
	{
		SetPort(offscreenWorldPtr->offscreenPtr);
	}
}

void DisposeOffscreenWorld(MyOffscreenPtr offscreenWorldPtr)
{
	if (offscreenWorldPtr->isColor)
	{
		UnlockPixels(offscreenWorldPtr->pixMapHandle);
		DisposeGWorld((GWorldPtr)(offscreenWorldPtr->offscreenPtr));
	}
	else
	{
		ClosePort(offscreenWorldPtr->offscreenPtr);
		DisposePtr(offscreenWorldPtr->bwBitMap);
	}
}

MyOffscreenComboPtr DisposeOffscreenCombo(MyOffscreenComboPtr offscreenComboPtr)
{
	DisposeOffscreenWorld(&(offscreenComboPtr->offscreenWorldRec));
	DisposeOffscreenWorld(&(offscreenComboPtr->patternWorldRec));
	DisposeOffscreenWorld(&(offscreenComboPtr->scratchWorldRec));
	DisposePtr((Ptr)offscreenComboPtr);
	
	return 0L;
}
