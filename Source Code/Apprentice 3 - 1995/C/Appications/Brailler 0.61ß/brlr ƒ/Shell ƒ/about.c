#include "about.h"
#include "environment.h"
#include "graphics.h"
#include "pict utilities.h"
#include "offscreen layer.h"
#include "prefs.h"
#include "util.h"
#include "text twiddling.h"
#include "file utilities.h"
#include "window layer.h"
#include "program globals.h"

#define DEAD_SPACE_H		10
#define DEAD_SPACE_V		10
#define kLogoWidth			73
#define kLogoHeight			40
#define kTextHeight			15
#define kLogoPictID			128
#define kAboutTextID		128
#define kCalligraphyFont	36
#define kScrollSpeed		1

static	Boolean			gSetupDone=FALSE;
static	MyOffscreenComboPtr	gLogoComboPtr=0L;
static	MyOffscreenComboPtr	gTextComboPtr=0L;
static	Handle			gTheTextHandle=0L;
static	Boolean			gScrollingAvailable=FALSE;
static	Str31			gVersionStr;
static	Boolean			gDoFade=TRUE;
static	Boolean			gDoChangeDepth=FALSE;

static	void DrawTheLogos(WindowPtr theWindow, Boolean doFade, short theDepth);
static void DrawTheTextPattern(WindowPtr theWindow, MyOffscreenComboPtr offscreenComboPtr,
				short theDepth);

void SetupTheAboutWindow(WindowPtr theWindow)
{
	unsigned char	*titleStr="\pAbout";
	Point			topLeft;
	unsigned char	*versionBit="\pversion ";
	Str255			versionNumberString;
	
	SetWindowWidth(theWindow, 400);
	SetWindowHeight(theWindow, DEAD_SPACE_V*3+kTextHeight+kLogoHeight);
	SetWindowAttributes(theWindow, kHasDocumentTitlebarMask+kHasCloseBoxMask);
	SetWindowTitle(theWindow, titleStr);
	SetWindowMaxDepth(theWindow, 8);
	SetWindowDepth(theWindow, 8);
	SetWindowIsFloat(theWindow, FALSE);
	SetWindowAutoCenter(theWindow, FALSE);
	topLeft.v=(qd.screenBits.bounds.bottom-qd.screenBits.bounds.top-GetWindowHeight(theWindow))/3+
		qd.screenBits.bounds.top;
	topLeft.h=(qd.screenBits.bounds.right-qd.screenBits.bounds.left-GetWindowWidth(theWindow))/2+
		qd.screenBits.bounds.left;
	SetWindowTopLeft(theWindow, topLeft);
	if (gSetupDone)
		return;
	
	gSetupDone=TRUE;
	gNeedToOpenWindow=FALSE;
	GetVersionString(versionNumberString);
	Mymemcpy((Ptr)gVersionStr, (Ptr)versionBit, versionBit[0]+1);
	AppendStr255(gVersionStr, versionNumberString);
}

void OpenTheAboutWindow(WindowPtr theWindow)
/* leaks memory on error */
{
	Rect 			logoSourceRect, logoDestRect;
	Rect			titleSourceRect, titleDestRect;
	PicHandle		logoPict;
	unsigned char	*titleStr=APPLICATION_NAME;
	short			realDepth, maxDepth, theDepth;
	
	realDepth=GetWindowRealDepth(theWindow);
	maxDepth=GetWindowMaxDepth(theWindow);
	theDepth=(realDepth>=maxDepth) ? maxDepth : realDepth;
	
	if (gLogoComboPtr==0L)
	{
		logoPict=0L;
		SetRect(&logoSourceRect, 0, 0, kLogoWidth, kLogoHeight);
		logoDestRect=logoSourceRect;
		OffsetRect(&logoDestRect, DEAD_SPACE_H, DEAD_SPACE_V);
		gLogoComboPtr=AllocateOffscreenCombo(theWindow, &logoSourceRect, &logoDestRect, 0, 0L, 0, 0);
		if (gLogoComboPtr==0L)
			return;
		SetPortToOffscreenWindow(&(gLogoComboPtr->offscreenWorldRec));
		logoPict=DrawThePicture(logoPict, kLogoPictID, 0, 0);
		InvertRect(&logoSourceRect);
		SetPortToOnscreenWindow(theWindow, &(gLogoComboPtr->offscreenWorldRec));
		logoPict=ReleaseThePict(logoPict);
		
		SetRect(&titleSourceRect, 0, 0, GetWindowWidth(theWindow), kTextHeight);
		titleDestRect=titleSourceRect;
		OffsetRect(&titleDestRect, 0, GetWindowHeight(theWindow)-kTextHeight-DEAD_SPACE_V);
		gTheTextHandle=GetResource('TEXT', kAboutTextID);
		AppendStr255ToHandle(gTheTextHandle, gMyName);
		if (gMyOrg[0]!=0x00)
		{
			AppendStr255ToHandle(gTheTextHandle, "\p, ");
			AppendStr255ToHandle(gTheTextHandle, gMyOrg);
		}
		AppendStr255ToHandle(gTheTextHandle, "\p.     -=-     ");
		gTextComboPtr=AllocateOffscreenCombo(theWindow, &titleSourceRect, &titleDestRect, kScrollSpeed,
			gTheTextHandle, geneva, 9);
		if (gTextComboPtr==0L)
			return;
		SetPortToOffscreenWindow(&(gTextComboPtr->offscreenWorldRec));
		FillRect(&(gTextComboPtr->offscreenWorldRec.offscreenPtr->portRect), &qd.black);
		SetPortToOnscreenWindow(theWindow, &(gTextComboPtr->offscreenWorldRec));
		DrawTheTextPattern(theWindow, gTextComboPtr, theDepth);
		gScrollingAvailable=TRUE;
	}
	
	if (!GetWindowVisible(theWindow))
		MyShowWindow(theWindow);
	FillRect(&(theWindow->portRect), &qd.black);
	if ((theDepth>=8) && (gDoFade))
	{
		DrawTheLogos(theWindow, TRUE, theDepth);
	}
}

void IdleInTheAboutWindow(WindowPtr theWindow)
{
	if (gDoChangeDepth)
	{
		DisposeTheAboutWindow(theWindow);
		gDoFade=FALSE;
		OpenTheAboutWindow(theWindow);
		gDoFade=TRUE;
		InvalRect(&(theWindow->portRect));
		gDoChangeDepth=FALSE;
	}
	else
	{
		if ((gScrollingAvailable) && (!gIsInBackground))
			ScrollTextCombo(gTextComboPtr);
	}
}

void DisposeTheAboutWindow(WindowPtr theWindow)
{
	gLogoComboPtr=DisposeOffscreenCombo(gLogoComboPtr);
	gTextComboPtr=DisposeOffscreenCombo(gTextComboPtr);
	ReleaseResource(gTheTextHandle);
	gTheTextHandle=0L;
}

void ShutDownTheAboutWindow(void)
{
}

void KeyDownInAboutWindow(WindowPtr theWindow, unsigned char theChar)
{
	CloseTheWindow(theWindow);
}

void MouseDownInAboutWindow(WindowPtr theWindow, Point thePoint)
{
	CloseTheWindow(theWindow);
}

void ChangeDepthTheAboutWindow(WindowPtr theWindow)
{
	gDoChangeDepth=TRUE;
}

void DrawTheAboutWindow(WindowPtr theWindow, short theDepth)
{
	short			titleWidth, versionWidth;
	unsigned char	*titleStr=APPLICATION_NAME;
	
	FillRect(&(theWindow->portRect), &qd.black);
	SetTheDrawingFont(kCalligraphyFont, 12, 0);
	TextMode(notSrcCopy);
	titleWidth=StringWidth(titleStr);
	MoveTo((GetWindowWidth(theWindow)-titleWidth)/2, DEAD_SPACE_V+18);
	DrawString(titleStr);
	SetTheDrawingFont(geneva, 9, 0);
	versionWidth=StringWidth(gVersionStr);
	MoveTo((GetWindowWidth(theWindow)-versionWidth)/2, DEAD_SPACE_V*2+15+10);
	DrawString(gVersionStr);
	UseDefaultDrawingFont();
	TextMode(srcCopy);
}

void CopybitsTheAboutWindow(WindowPtr theWindow, WindowPtr offscreenWindowPtr)
{
	RgnHandle		logoRgn, copyRgn;
	Rect			logoRect;
	
	copyRgn=NewRgn();
	logoRgn=NewRgn();
	RectRgn(copyRgn, &(theWindow->portRect));
	logoRect=gLogoComboPtr->destRect;
	RectRgn(logoRgn, &logoRect);
	DiffRgn(copyRgn, logoRgn, copyRgn);
	SetEmptyRgn(logoRgn);
	logoRect.right=GetWindowWidth(theWindow)-DEAD_SPACE_H;
	logoRect.left=logoRect.right-kLogoWidth;
	RectRgn(logoRgn, &logoRect);
	DiffRgn(copyRgn, logoRgn, copyRgn);
	
	CopyBits(	&(offscreenWindowPtr->portBits),
				&(theWindow->portBits),
				&(theWindow->portRect),
				&(theWindow->portRect), 0, copyRgn);
	
	DrawTheLogos(theWindow, FALSE, GetWindowDepth(theWindow));
	CopybitsCombo(gTextComboPtr);
	
	DisposeRgn(copyRgn);
	DisposeRgn(logoRgn);
}

static	void DrawTheLogos(WindowPtr theWindow, Boolean doFade, short theDepth)
{
	unsigned short	j, numRows, startJ;
	RGBColor		patternColor;
	Rect			logoRect1, logoRect2, sourceRect;
	PixPatHandle	backgroundppat;
	
	logoRect1=logoRect2=gLogoComboPtr->destRect;
	logoRect2.right=GetWindowWidth(theWindow)-DEAD_SPACE_H;
	logoRect2.left=logoRect2.right-kLogoWidth;
	sourceRect=gLogoComboPtr->sourceRect;
	
	if (theDepth>=4)
	{
		numRows=kLogoHeight;
		patternColor.red=patternColor.green=0;
		if (theDepth>=8)
			startJ=doFade ? 0 : 64;
		else
			startJ=64;
			
		for (j=startJ; j<=64; j++)
		{
			if (j<64)
				patternColor.blue=j*1024;
			else
				patternColor.blue=65535;
			SetPortToOffscreenWindow(&(gLogoComboPtr->patternWorldRec));
			backgroundppat=NewPixPat();
			MakeRGBPat(backgroundppat, &patternColor);
			FillCRect(&sourceRect, backgroundppat);
			DisposePixPat(backgroundppat);
			SetPortToOnscreenWindow(theWindow, &(gLogoComboPtr->patternWorldRec));
			CopybitsCombo(gLogoComboPtr);
			gLogoComboPtr->destRect=logoRect2;
			CopybitsCombo(gLogoComboPtr);
			gLogoComboPtr->destRect=logoRect1;
		}
	}
	else
	{
		SetPortToOffscreenWindow(&(gLogoComboPtr->patternWorldRec));
		EraseRect(&sourceRect);
		SetPortToOnscreenWindow(theWindow, &(gLogoComboPtr->patternWorldRec));
		CopybitsCombo(gLogoComboPtr);
		gLogoComboPtr->destRect=logoRect2;
		CopybitsCombo(gLogoComboPtr);
		gLogoComboPtr->destRect=logoRect1;
	}
}

static void DrawTheTextPattern(WindowPtr theWindow, MyOffscreenComboPtr offscreenComboPtr,
	short theDepth)
{
	short			i;
	short			WIDTH;
	RGBColor		*theColor;
	Rect			sourceRect;
	
	WIDTH=offscreenComboPtr->sourceRect.right-offscreenComboPtr->sourceRect.left;
	sourceRect=offscreenComboPtr->sourceRect;
	SetPortToOffscreenWindow(&(offscreenComboPtr->patternWorldRec));
	
	if (theDepth>=8)
	{
		theColor=(RGBColor*)NewPtr(sizeof(RGBColor)*WIDTH);
		for (i=0; i<64; i++)
		{
			theColor[i].red=i*1024;
			theColor[i].green=theColor[i].blue=0;
		}
		for (i=64; i<WIDTH-64; i++)
		{
			theColor[i].red=65535;
			theColor[i].green=theColor[i].blue=0;
		}
		for (i=WIDTH-64; i<WIDTH; i++)
		{
			theColor[i].red=theColor[WIDTH-i-1].red;
			theColor[i].green=theColor[i].blue=0;
		}
		for (i=0; i<WIDTH; i++)
		{
			MoveTo(i, 0);
			RGBForeColor(&theColor[i]);
			Line(0, kTextHeight);
		}
		DisposePtr((Ptr)theColor);
	}
	else if (theDepth==4)
	{
		ForeColor(redColor);
		PaintRect(&sourceRect);
		ForeColor(blackColor);
	}
	else
	{
		EraseRect(&sourceRect);
	}
	
	SetPortToOnscreenWindow(theWindow, &(offscreenComboPtr->patternWorldRec));
}
