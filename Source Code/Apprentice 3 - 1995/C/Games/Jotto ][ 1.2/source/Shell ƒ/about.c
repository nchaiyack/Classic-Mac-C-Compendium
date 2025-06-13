#include "about.h"
#include "prefs.h"
#include "util.h"
#include "text twiddling.h"
#include "window layer.h"
#include "graphics.h"

#define	kAboutTextID	128

static	Boolean			gSetupDone=FALSE;
static	Handle			gTheTextHandle=0L;

void SetupTheAboutWindow(WindowPtr theWindow)
{
	unsigned char	*titleStr="\pAbout";
	unsigned char	returnChar='\r';
	long			oldSize;
	
	SetWindowWidth(theWindow, 170);
	SetWindowHeight(theWindow, 212);
	SetWindowType(theWindow, noGrowDocProc);
	SetWindowTitle(theWindow, titleStr);
	SetWindowHasCloseBox(theWindow, TRUE);
	SetWindowMaxDepth(theWindow, 1);
	SetWindowDepth(theWindow, 1);
	SetWindowIsFloat(theWindow, FALSE);
	SetWindowAutoCenter(theWindow, TRUE);
	if (gSetupDone)
		return;
	
	gTheTextHandle=Get1Resource('TEXT', kAboutTextID);
	if (gTheTextHandle!=0L)
		DetachResource(gTheTextHandle);
	oldSize=GetHandleSize(gTheTextHandle);
	HLock(gTheTextHandle);
	SetHandleSize(gTheTextHandle, oldSize+gMyName[0]+gMyOrg[0]+1);
	Mymemcpy((Ptr)((long)*gTheTextHandle+oldSize), (Ptr)&gMyName[1], gMyName[0]);
	Mymemcpy((Ptr)((long)*gTheTextHandle+oldSize+gMyName[0]), &returnChar, 1);
	Mymemcpy((Ptr)((long)*gTheTextHandle+oldSize+gMyName[0]+1), &gMyOrg[1], gMyOrg[0]);
	HUnlock(gTheTextHandle);
	gSetupDone=TRUE;
}

void OpenTheAboutWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	FontInfo		theFontInfo;
	Rect			destRect, viewRect;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
	{
		GetTERect(theWindow, &destRect, FALSE);
		viewRect=destRect;
		hTE=TENew(&destRect, &viewRect);
		SetWindowTE(theWindow, hTE);
		TextFont((**hTE).txFont=geneva);
		TextSize((**hTE).txSize=9);
		TextFace((**hTE).txFace=0);
		GetFontInfo(&theFontInfo);
		(**hTE).fontAscent=theFontInfo.ascent;
		(**hTE).lineHeight=theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading;
		HLock(gTheTextHandle);
		SetTheText(theWindow, *gTheTextHandle, GetHandleSize(gTheTextHandle));
		HUnlock(gTheTextHandle);
		TESetAlignment(teCenter, hTE);
		TESetSelect(0, 0, hTE);
		TEAutoView(TRUE, hTE);
		TESelView(hTE);
	}
}

void DisposeTheAboutWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE!=0L)
	{
		TEDispose(hTE);
		SetWindowTE(theWindow, 0L);
	}
}

void ShutDownTheAboutWindow(void)
{
	if (gTheTextHandle!=0L)
		DisposeHandle(gTheTextHandle);
	gTheTextHandle=0L;
}

void KeyDownInAboutWindow(WindowPtr theWindow, unsigned char theChar)
{
	CloseTheWindow(theWindow);
}

void MouseDownInAboutWindow(WindowPtr theWindow, Point thePoint)
{
	CloseTheWindow(theWindow);
}
