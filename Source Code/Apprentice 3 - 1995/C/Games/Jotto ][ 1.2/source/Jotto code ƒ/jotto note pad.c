#include "jotto note pad.h"
#include "jotto environment.h"
#include "environment.h"
#include "util.h"
#include "menus.h"
#include "main.h"
#include "text twiddling.h"
#include "generic window handlers.h"
#include "window layer.h"
#include "program globals.h"

#define WINDOW_WIDTH		220
#define	WINDOW_HEIGHT		220
#define kGrowBoxSize		15

static	void PutDataIntoTEFields(WindowPtr theWindow);

static	short			gOldForegroundTime;		/* stored foreground wait time */
static	CursHandle		gMyIBeamHandle;
static	Handle			gSavedText;
static	short			gSavedSelStart, gSavedSelEnd;
static	Boolean			gSetupDone=FALSE;
static	Boolean			gIsActive=FALSE;

void SetupTheNotePad(WindowPtr theWindow)
{
	unsigned char	*titleStr="\pNotes";
	Point			topLeft;
	
	SetWindowWidth(theWindow, WINDOW_WIDTH);
	SetWindowHeight(theWindow, WINDOW_HEIGHT);
	SetWindowType(theWindow, zoomDocProc);
	topLeft.v=qd.screenBits.bounds.bottom-WINDOW_HEIGHT-10;
	topLeft.h=10;
	SetWindowTopLeft(theWindow, topLeft);
	SetWindowHasCloseBox(theWindow, TRUE);
	SetWindowTitle(theWindow, titleStr);
	SetWindowAutoCenter(theWindow, FALSE);
	SetWindowIsFloat(theWindow, FALSE);
	if (gSetupDone)
		return;
	
	gSavedText=0L;
	gSavedSelStart=0;
	gSavedSelEnd=32767;
	gSetupDone=TRUE;
	gMyIBeamHandle=GetCursor(iBeamCursor);
}

void ShutDownTheNotePad(void)
{
	if (gSavedText!=0L)
		DisposeHandle(gSavedText);
	if (gMyIBeamHandle!=0L)
		ReleaseResource((Handle)gMyIBeamHandle);
}

void OpenTheNotePad(WindowPtr theWindow)
{
	TEHandle		hTE;
	FontInfo		theFontInfo;
	Rect			vScrollBarRect, hScrollBarRect;
	Rect			destRect, viewRect;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
	{
		SetRect(&vScrollBarRect, GetWindowWidth(theWindow)-kGrowBoxSize, -1,
			GetWindowWidth(theWindow)+1, GetWindowHeight(theWindow)+1-kGrowBoxSize);
		SetRect(&hScrollBarRect, -1, GetWindowHeight(theWindow)-kGrowBoxSize,
			GetWindowWidth(theWindow)-kGrowBoxSize+1, GetWindowHeight(theWindow)+1);
		SetWindowVScrollBar(theWindow,
			NewControl(theWindow, &vScrollBarRect, "\p", TRUE, 0, 0, 0, scrollBarProc, 0));
		SetWindowHScrollBar(theWindow,
			NewControl(theWindow, &hScrollBarRect, "\p", TRUE, 0, 0, 0, scrollBarProc, 0));
		
		GetTERect(theWindow, &destRect, TRUE);
		viewRect=destRect;
		hTE=TENew(&destRect, &viewRect);
		SetWindowTE(theWindow, hTE);
		TextFont((**hTE).txFont=geneva);
		TextSize((**hTE).txSize=9);
		TextFace((**hTE).txFace=0);
		GetFontInfo(&theFontInfo);
		(**hTE).fontAscent=theFontInfo.ascent;
		(**hTE).lineHeight=theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading;
		AdjustViewRect(hTE);
		TEAutoView(TRUE, hTE);
		TESetClickLoop((TEClickLoopUPP)MyClikLoop, hTE);
		PutDataIntoTEFields(theWindow);
	}
	
	gIsActive=TRUE;
	AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
	AdjustMenus();
}

void IdleInNotePad(WindowPtr theWindow, Point mouseLoc)
{
	TEHandle		hTE;
	
	if (gInProgress)
		return;
	
	hTE=GetWindowTE(theWindow);
	TEIdle(hTE);
	
	if (PtInRect(mouseLoc, &((**hTE).viewRect)))
	{
		if (!gCustomCursor)
		{
			SetCursor(*gMyIBeamHandle);
			gCustomCursor=TRUE;
		}
	}
	else
	{
		gCustomCursor=FALSE;
	}
}

void KeyPressedInNotePad(WindowPtr theWindow, unsigned char theChar)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	
	switch (theChar)
	{
		case 0x09:
			TESetSelect(0, 32767, hTE);
			break;
		default:
			TEKey(theChar, hTE);
	}
	
	AdjustVScrollBar(vScrollBar, hTE);
}

void DisposeTheNotePad(WindowPtr theWindow)
{
	unsigned long	theLength;
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (gSavedText!=0L)
		DisposeHandle(gSavedText);
	if (hTE!=0L)
	{
		gSavedText=NewHandle(theLength=(**hTE).teLength);
		HLock(gSavedText);
		HLock((Handle)hTE);
		Mymemcpy(*gSavedText, *((**hTE).hText), theLength);
		HUnlock(gSavedText);
		HUnlock((Handle)hTE);
		gSavedSelStart=(**hTE).selStart;
		gSavedSelEnd=(**hTE).selEnd;
		TEDispose(hTE);
		SetWindowTE(theWindow, 0L);
	}
	else
	{
		gSavedText=NewHandle(0L);
	}
}

void ActivateTheNotePad(WindowPtr theWindow)
{
	gOldForegroundTime=gForegroundWaitTime;
	gForegroundWaitTime=0;
	gIsActive=TRUE;
}

void DeactivateTheNotePad(WindowPtr theWindow)
{
	gForegroundWaitTime=gOldForegroundTime;
	gIsActive=FALSE;
}

void CopybitsTheNotePad(WindowPtr theWindow, WindowPtr offscreenWindow)
{
	GenericCopybits(theWindow, offscreenWindow, gIsActive);
}

void PasteInNotePad(WindowPtr theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	Handle			scrapHandle;
	long			offset;
	unsigned long	scrapLength;
	unsigned char	*theSource="\pMBDF-A";
	unsigned char	*theTarget="\pProgramming is not a crime.";
	unsigned char	*cheat="\pcheat";
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	
	scrapHandle=NewHandle(0L);
	if (GetScrap(scrapHandle, 'TEXT', &offset)!=noTypeErr)
	{
		scrapLength=GetHandleSize(scrapHandle);
		if (scrapLength==theSource[0])
		{
			HLock(scrapHandle);
			if (Mymemcompare((Ptr)*scrapHandle, (Ptr)&theSource[1], theSource[0]))
			{
				ZeroScrap();
				PutScrap(theTarget[0], 'TEXT', (char*)&theTarget[1]);
			}
		}
		else if ((GameInProgressQQ()) && (scrapLength==cheat[0]))
		{
			HLock(scrapHandle);
			if (Mymemcompare((Ptr)*scrapHandle, (Ptr)&cheat[1], cheat[0]))
			{
				ZeroScrap();
				PutScrap(gNumLetters, 'TEXT', gComputerWord);
			}
		}
		
		TEFromScrap();
		TEPaste(hTE);
		TESelView(hTE);
		if (vScrollBar!=0L)
			AdjustVScrollBar(vScrollBar, hTE);
	}
	DisposeHandle(scrapHandle);
}

static	void PutDataIntoTEFields(WindowPtr theWindow)
{
	TEHandle			hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
		return;
	
	if (gSavedText!=0L)
	{
		HLock(gSavedText);
		SetTheText(theWindow, *gSavedText, GetHandleSize(gSavedText));
		HUnlock(gSavedText);
	}
	
	TESetSelect(gSavedSelStart, gSavedSelEnd, hTE);
}
