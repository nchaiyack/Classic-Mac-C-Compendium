#include "kant main window.h"
#include "kant load-save.h"
#include "drag layer.h"
#include "environment.h"
#include "memory layer.h"
#include "menus.h"
#include "main.h"
#include "dialogs.h"
#include "text layer.h"
#include "generic window handlers.h"
#include "window layer.h"
#include "program globals.h"
#include "save dialog.h"

#define kGrowBoxSize		15

static	void PutDataIntoTEFields(WindowRef theWindow);

static	short			gOldForegroundTime;		/* stored foreground wait time */
#if powerc
static	TEClickLoopUPP	gClickLoopUPP;
#endif
static	Str255			gMainWindowTitle;
static	Boolean			gSetupDone=FALSE;
static	short			gDocumentCount=1;

void SetupTheMainWindow(WindowRef theWindow)
{
	Str255			titleStr="\puntitled";
	Str255			countStr;
	Point			topLeft;
	FSSpec			fs;
	
	SetWindowHeight(theWindow, qd.screenBits.bounds.bottom-qd.screenBits.bounds.top-LMGetMBarHeight()-28);
	SetWindowWidth(theWindow, qd.screenBits.bounds.right-qd.screenBits.bounds.left-70);
	SetWindowAttributes(theWindow, kHasCloseBoxMask+kHasZoomBoxMask+kHasGrowBoxMask+kHasDocumentTitlebarMask);
	topLeft.v=qd.screenBits.bounds.top+LMGetMBarHeight()+20;
	topLeft.h=qd.screenBits.bounds.left+10;
	SetWindowTopLeft(theWindow, topLeft);
	SetWindowMaxDepth(theWindow, 8);
	SetWindowIsFloat(theWindow, FALSE);
	SetWindowIsZoomable(theWindow, TRUE);
	SetWindowIsEditable(theWindow, TRUE);
	if (gMainWindowTitle[0]==0x00)
	{
		if (gDocumentCount>1)
		{
			NumToString(gDocumentCount, countStr);
			AppendStr255(titleStr, "\p ");
			AppendStr255(titleStr, countStr);
		}
		
		SetWindowTitle(theWindow, titleStr);
		gDocumentCount++;
	}
	else
	{
		SetWindowTitle(theWindow, gMainWindowTitle);
	}
	gMainWindowTitle[0]=0x00;
	
	SetWindowAutoCenter(theWindow, FALSE);
	fs.name[0]=0x00;
	fs.vRefNum=0;
	fs.parID=0;
	SetWindowFS(theWindow, fs);
	SetWindowIsModified(theWindow, FALSE);
	SetWindowIsDraggable(theWindow, TRUE);
	SetWindowIsPrintable(theWindow, TRUE);
	
	if (gSetupDone)
		return;
	
	gSetupDone=TRUE;
#if powerc
	gClickLoopUPP=NewTEClickLoopProc(ClickLoopProc);
#endif
}

void ShutDownTheMainWindow(void)
{
#if powerc
	if (gClickLoopUPP!=0L)
		DisposeRoutineDescriptor(gClickLoopUPP);
#endif
}

void OpenTheMainWindow(WindowRef theWindow)
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
		TextFont((**hTE).txFont=monaco);
		TextSize((**hTE).txSize=9);
		TextFace((**hTE).txFace=0);
		GetFontInfo(&theFontInfo);
		(**hTE).fontAscent=theFontInfo.ascent;
		(**hTE).lineHeight=theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading;
		AdjustViewRect(hTE);
		TEAutoView(TRUE, hTE);
		SetWindowOldClickLoopProc(theWindow, (**hTE).clickLoop);
#if powerc
		TESetClickLoop(gClickLoopUPP, hTE);
#else
		(**hTE).clickLoop=(TEClickLoopUPP)MyClikLoop;
#endif
		PutDataIntoTEFields(theWindow);
	}
	
	AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
	AdjustMenus();
}

void KeyPressedInMainWindow(WindowRef theWindow, unsigned char theChar)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	TEKey(theChar, hTE);
	SetWindowIsModified(theWindow, TRUE);
	SetWindowLastFindPosition(theWindow, (**hTE).selStart);
	AdjustForEndScroll(vScrollBar, hTE);
	AdjustVScrollBar(vScrollBar, hTE);
}

void DisposeTheMainWindow(WindowRef theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE!=0L)
	{
		TEDispose(hTE);
		SetWindowTE(theWindow, 0L);
	}
	gCustomCursor=FALSE;
}

Boolean CloseTheMainWindow(WindowRef theWindow)
{
	return (DisplaySaveAlert(theWindow, GetWindowTitle(theWindow), "\pclosing")!=kUserCanceled);
}

void PasteInMainWindow(WindowRef theWindow)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	Handle			scrapHandle;
	long			offset;
	unsigned long	scrapLength;
	unsigned char	*theSource="\pMBDF-A";
	unsigned char	*theTarget="\pProgramming is not a crime.";
	
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
		
		TEFromScrap();
		TEPaste(hTE);
		TESelView(hTE);
		if (vScrollBar!=0L)
			AdjustVScrollBar(vScrollBar, hTE);
		if (scrapLength!=0L)
			SetWindowIsModified(theWindow, TRUE);
	}
	DisposeHandle(scrapHandle);
	SetWindowLastFindPosition(theWindow, (**hTE).selStart);
	ResetHiliteRgn(theWindow);
}

static	void PutDataIntoTEFields(WindowRef theWindow)
{
	TEHandle			hTE;
	
	hTE=GetWindowTE(theWindow);
	TESetSelect(0, 0, hTE);
	TEKey(0x00, hTE);
	TEKey(0x08, hTE);
}

void SetMainWindowTitle(Str255 theTitle)
{
	Mymemcpy((Ptr)gMainWindowTitle, (Ptr)theTitle, theTitle[0]+1);
}
