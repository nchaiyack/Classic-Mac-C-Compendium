#include "brlr main window.h"
#include "brlr load-save.h"
#include "brlr conversion.h"
#include "brlr grade 2 meat.h"
#include "environment.h"
#include "menus.h"
#include "main.h"
#include "util.h"
#include "text twiddling.h"
#include "dialog layer.h"
#include "graphics.h"
#include "window layer.h"
#include "key layer.h"
#include "program globals.h"

#define kGrowBoxSize		15
#define kBrailleFont		10158	/* resource # of braille font */

static	void DealWithKeyPressed(WindowPtr theWindow, unsigned char theChar);
static	void DealWithKeys(void);
static	void PutDataIntoTEFields(WindowPtr theWindow);

static	Str63			gMainWindowTitle;
static	Boolean			gCharInProgress;
static	unsigned char	gTheChar;
#if powerc
static	TEClickLoopUPP	gClickLoopUPP;
#endif

void SetupTheMainWindow(WindowPtr theWindow)
{
	unsigned char	*titleStr="\puntitled";
	Point			topLeft;
	FSSpec			fs;
	
	SetWindowHeight(theWindow, qd.screenBits.bounds.bottom-qd.screenBits.bounds.top-LMGetMBarHeight()-28);
	SetWindowWidth(theWindow, qd.screenBits.bounds.right-qd.screenBits.bounds.left-70);
	SetWindowAttributes(theWindow, kHasCloseBoxMask+kHasZoomBoxMask+kHasGrowBoxMask+kHasDocumentTitlebarMask);
	topLeft.v=qd.screenBits.bounds.top+LMGetMBarHeight()+20;
	topLeft.h=qd.screenBits.bounds.left+10;
	SetWindowTopLeft(theWindow, topLeft);
	SetWindowMaxDepth(theWindow, 1);
	SetWindowDepth(theWindow, 1);
	SetWindowIsFloat(theWindow, FALSE);
	if (gMainWindowTitle[0]==0x00)
	{
		SetWindowTitle(theWindow, titleStr);
	}
	else
	{
		SetWindowTitle(theWindow, gMainWindowTitle);
		gMainWindowTitle[0]=0x00;
	}
	SetWindowAutoCenter(theWindow, FALSE);
	SetWindowIsPrintable(theWindow, TRUE);
	SetWindowIsDraggable(theWindow, TRUE);
	
	fs.name[0]=0x00;
	fs.vRefNum=0;
	fs.parID=0;
	SetWindowFS(theWindow, fs);
	SetWindowIsModified(theWindow, FALSE);
	
	gTheChar=0x00;
	gCharInProgress=FALSE;
	
	if (gGrade==2)
		OpenTheIndWindow(kFloatingWindow);
	
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

void OpenTheMainWindow(WindowPtr theWindow)
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
		TextFont((**hTE).txFont=kBrailleFont);
		TextSize((**hTE).txSize=24);
		TextFace((**hTE).txFace=0);
		GetFontInfo(&theFontInfo);
		(**hTE).fontAscent=theFontInfo.ascent+5;
		(**hTE).lineHeight=theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading+5;
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
	
	SetWindowIsActive(theWindow, TRUE);
	AdjustVScrollBar(GetWindowVScrollBar(theWindow), hTE);
	AdjustMenus();
}

void IdleInMainWindow(WindowPtr theWindow, Point mouseLoc)
{
	KeyMap			rawKeys;
	unsigned short	theKeys[8];
	unsigned char	theChar;
	
	if (gInProgress)
		return;
	
	GetKeys(rawKeys);
	Mymemcpy((Ptr)theKeys, (Ptr)rawKeys, sizeof(rawKeys));
	if ((theKeys[0]) || (theKeys[1]) || (theKeys[2]) || (theKeys[3]&0xfffd) ||
		(theKeys[4]) || (theKeys[5]) || (theKeys[6]) || (theKeys[7]))
	{}
	else if (gCharInProgress)
	{
		theChar=gTheChar+' ';
		gTheChar=0x00;
		gCharInProgress=FALSE;
		DealWithKeyPressed(theWindow, theChar);
	}
}

void KeyPressedInMainWindow(WindowPtr theWindow, unsigned char theChar)
{
	TEHandle		hTE;
	ControlHandle	vScrollBar;
	
	if (gInProgress)
		return;
	
	hTE=GetWindowTE(theWindow);
	vScrollBar=GetWindowVScrollBar(theWindow);
	
	if (gPostingEvents)
	{
		DealWithKeyPressed(theWindow, theChar);
	}
	else
	{
		switch (theChar)
		{
			case key_UpArrow:
			case key_DownArrow:
			case key_LeftArrow:
			case key_RightArrow:
			case key_Return:
			case key_Enter:
			case key_Tab:
			case key_Delete:
			case ' ':
				DealWithKeyPressed(theWindow, theChar);
				break;
			default:
				if (gGrade==1)
				{
					short			time, oldTime;
					unsigned char	brailleChar;
					
					time=1;
					do
					{
						oldTime=time;
						brailleChar=DealWithLetter(theChar, &time);
						if (brailleChar!=0x00)
							DealWithKeyPressed(theWindow, brailleChar);
					}
					while (oldTime!=time);
				}
				else
					DealWithKeys();
				break;
		}
	}
	
	AdjustForEndScroll(vScrollBar, hTE);
	AdjustVScrollBar(vScrollBar, hTE);
}

Boolean CloseTheMainWindow(WindowPtr theWindow)
{
	WindowPtr		floatingWindow;
	short			result;
	
	if (WindowIsModifiedQQ(theWindow))
	{
		SetCursor(&qd.arrow);
		result=DisplayTheAlert(kCautionAlert, kSaveAlertID, GetWindowTitle(theWindow),
			"\p", "\p", "\p", (UniversalProcPtr)ThreeButtonFilter);
		switch (result)
		{
			case 1:	/* save */
				LoadSaveDispatch(FALSE, TRUE);
				if (WindowIsModifiedQQ(theWindow))	/* save didn't work for some reason */
					return FALSE;
				break;
			case 2:	 /* cancel */
				return FALSE;
				break;
			case 3:	/* don't save */
				break;
		}
	}
	
	floatingWindow=GetIndWindowPtr(kFloatingWindow);
	if (floatingWindow!=0L)
		CloseTheWindow(floatingWindow);
	
	gCustomCursor=FALSE;
	
	return TRUE;
}

void DisposeTheMainWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE!=0L)
	{
		TEDispose(hTE);
		SetWindowTE(theWindow, 0L);
	}
}

void ActivateTheMainWindow(WindowPtr theWindow)
{
	WindowRef		floatingWindow;
	
	floatingWindow=GetIndWindowPtr(kFloatingWindow);
	if (floatingWindow!=0L)
	{
		ShowOneFloatingWindow(floatingWindow);
	}
}

void DeactivateTheMainWindow(WindowPtr theWindow)
{
	WindowRef		floatingWindow;
	
	floatingWindow=GetIndWindowPtr(kFloatingWindow);
	if (floatingWindow!=0L)
	{
		HideOneFloatingWindow(floatingWindow);
	}
}

void SetMainWindowTitle(Str63 theTitle)
{
	Mymemcpy((Ptr)gMainWindowTitle, (Ptr)theTitle, theTitle[0]+1);
}

/* ---------------------------------------------------- */
/* the rest of these are internal to brlr main window.c */

static	void DealWithKeyPressed(WindowPtr theWindow, unsigned char theChar)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	TEKey(theChar, hTE);
	TESelView(hTE);
	AdjustForEndScroll(GetWindowVScrollBar(theWindow), hTE);
	SetWindowIsModified(theWindow, TRUE);
}

static	void DealWithKeys(void)
{
	KeyMap			rawKeys;
	unsigned short	theKeys[8];
	
	GetKeys(rawKeys);
	Mymemcpy((Ptr)theKeys, (Ptr)rawKeys, sizeof(rawKeys));
	if (theKeys[0]&16384)	{ gTheChar|=0x04; gCharInProgress=TRUE; }
	if (theKeys[0]&32768)	{ gTheChar|=0x02; gCharInProgress=TRUE; }
	if (theKeys[0]&1)		{ gTheChar|=0x01; gCharInProgress=TRUE; }
	if (theKeys[2]&8)		{ gTheChar|=0x08; gCharInProgress=TRUE; }
	if (theKeys[2]&128)		{ gTheChar|=0x10; gCharInProgress=TRUE; }
	if (theKeys[2]&16)		{ gTheChar|=0x20; gCharInProgress=TRUE; }
}

static	void PutDataIntoTEFields(WindowPtr theWindow)
{
	TEHandle			hTE;
	
	hTE=GetWindowTE(theWindow);
	TESetSelect(0, 0, hTE);
	TEKey(0x00, hTE);
	TEKey(0x08, hTE);
}
