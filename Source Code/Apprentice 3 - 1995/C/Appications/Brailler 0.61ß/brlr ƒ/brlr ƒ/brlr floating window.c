#include "brlr floating window.h"
#include "brlr grade 2 meat.h"
#include "brlr conversion.h"
#include "environment.h"
#include "main.h"
#include "menus.h"
#include "util.h"
#include "graphics.h"
#include "window layer.h"
#include "program globals.h"

enum { key_LeftArrow=0x1c, key_RightArrow, key_UpArrow, key_DownArrow };
enum { key_PageUp=0x0b, key_PageDown };
enum { key_Home=0x01 };
enum { key_End=0x04 };

#define HEADER_SPACE		(30+TE_HEIGHT)
#define FOOTER_SPACE		5
#define DEAD_SPACE_LEFT		6
#define DEAD_SPACE_RIGHT	DEAD_SPACE_LEFT
#define WINDOW_WIDTH		200
#define	WINDOW_HEIGHT		75
#define TE_HEIGHT			12

static	Point			lastTopLeft;
static	Boolean			gSetupDone=FALSE;

void SetupTheFloatingWindow(WindowPtr theWindow)
{
	unsigned char	*titleStr="\pGrade 2";
	Point			topLeft;
	
	SetWindowHeight(theWindow, WINDOW_HEIGHT);
	SetWindowWidth(theWindow, WINDOW_WIDTH);
	SetWindowAttributes(theWindow, kHasPaletteTitlebarMask);
	SetWindowMaxDepth(theWindow, 1);
	SetWindowDepth(theWindow, 1);
	SetWindowIsFloat(theWindow, TRUE);
	SetWindowTitle(theWindow, titleStr);
	SetWindowAutoCenter(theWindow, FALSE);
	SetWindowIsPrintable(theWindow, FALSE);
	SetWindowIsDraggable(theWindow, TRUE);
	
	if (gSetupDone)
	{
		topLeft=lastTopLeft;
	}
	else
	{
		topLeft.v=qd.screenBits.bounds.bottom-150;
		topLeft.h=(qd.screenBits.bounds.left+qd.screenBits.bounds.right)/2+
			qd.screenBits.bounds.left-(WINDOW_WIDTH/2);
	}
	SetWindowTopLeft(theWindow, topLeft);
	
	gSetupDone=TRUE;
}

void OpenTheFloatingWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	FontInfo		theFontInfo;
	Rect			destRect, viewRect;
	
	hTE=GetWindowTE(theWindow);
	if (hTE==0L)
	{
		SetRect(&destRect, 10, 10, WINDOW_WIDTH-10, 10+TE_HEIGHT);
		viewRect=destRect;
		hTE=TENew(&destRect, &viewRect);
		SetWindowTE(theWindow, hTE);
		TextFont((**hTE).txFont=monaco);
		TextSize((**hTE).txSize=9);
		TextFace((**hTE).txFace=0);
		TextFont(0);
		TextSize(12);
		TextFace(0);
		GetFontInfo(&theFontInfo);
		(**hTE).fontAscent=theFontInfo.ascent;
		(**hTE).lineHeight=theFontInfo.ascent+theFontInfo.descent+theFontInfo.leading;
		TEAutoView(TRUE, hTE);
	}
	
	SetWindowIsActive(theWindow, TRUE);
	AdjustMenus();
}

void CloseTheFloatingWindow(WindowPtr theWindow)
{
	lastTopLeft=GetWindowTopLeft(theWindow);
}

Boolean KeyPressedInFloatingWindow(WindowPtr theWindow, unsigned char theChar)
{
	TEHandle		hTE;
	
	if ((gPostingEvents) || (GetFrontDocumentWindow()!=GetIndWindowPtr(kMainWindow)))
		return FALSE;
	if (gInProgress)
		return FALSE;
	
	hTE=GetWindowTE(theWindow);
	switch (theChar)
	{
		case key_PageUp:
		case key_PageDown:
		case key_Home:
		case key_End:
		case key_LeftArrow:
		case key_RightArrow:
		case key_UpArrow:
		case key_DownArrow:
			return FALSE;
		case ' ':
		case '\r':
		case 0x03:
			TEKey(theChar, hTE);
			ReceiveDragInFloatingWindow(theWindow);
			break;
		default:
			TEKey(theChar, hTE);
			break;
	}
	TESelView(hTE);
	
	return TRUE;
}

void DisposeTheFloatingWindow(WindowPtr theWindow)
{
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	if (hTE!=0L)
	{
		TEDispose(hTE);
		SetWindowTE(theWindow, 0L);
	}
}

void DrawTheFloatingWindow(WindowPtr theWindow, short theDepth)
{
	GrafPtr			curPort;
	Rect			tempRect;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	tempRect=(**GetWindowTE(theWindow)).viewRect;
	InsetRect(&tempRect, -4, -4);
	FrameRect(&tempRect);
	
	MoveTo(DEAD_SPACE_LEFT, HEADER_SPACE);
	TextFont(geneva);
	TextSize(9);
	TextFace(0);
	DrawString("\pText will be converted to grade 2 Braille");
	MoveTo(DEAD_SPACE_LEFT, HEADER_SPACE+12);
	DrawString("\pand inserted as soon as you type SPACE,");
	MoveTo(DEAD_SPACE_LEFT, HEADER_SPACE+24);
	DrawString("\pRETURN, or ENTER.");
}

void ReceiveDragInFloatingWindow(WindowPtr theWindow)
{
	WindowPtr		mainWindow;
	TEHandle		hTE;
	
	hTE=GetWindowTE(theWindow);
	mainWindow=GetIndWindowPtr(kMainWindow);
	if (GetFrontDocumentWindow()!=mainWindow)
		MySelectWindow(mainWindow);
	HLock((**hTE).hText);
	DealWithGrade2Text(mainWindow, *((unsigned char**)(**hTE).hText), (**hTE).teLength);
	HUnlock((**hTE).hText);
	TESetSelect(0, 32767, hTE);
	TEDelete(hTE);
}
