/*----------------------------------------------------------------------------

	open.c

	This module handles creating and opening windows.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "open.h"
#include "drag.h"
#include "ldef.h"
#include "scroll.h"
#include "util.h"
#include "wind.h"
#include "resize.h"
#include "full.h"



/*  	MyOffSet offsets a window so no two windows are at the same origin and the
		windows do not hang off of the screen.
*/

static Rect *MyOffSet (Rect *theRect)
{
	WindowPtr wind;
	Rect firstRect, bitsRect;
		
	firstRect = *theRect;
	wind = FrontWindow();

	while (wind) {
		if (wind->portBits.rowBytes & 0x8000 && wind->portBits.rowBytes & 0x4000) {
			PixMapHandle pmap = ((CGrafPtr)wind)->portPixMap;
			bitsRect = (**pmap).bounds;   				  /* Color GrafPort */
		} else {
			bitsRect = wind->portBits.bounds; 	  /* Ordinary GrafPort */
		}
		if ((theRect->top == (-bitsRect.top)) && 
			(theRect->left == (-bitsRect.left))) 
		{
			OffsetRect(theRect,25,25);
			if (theRect->right > gDesktopExtent.right) {
				OffsetRect(&firstRect,0,27);
				*theRect = firstRect;
			}
			if (theRect->bottom > gDesktopExtent.bottom) {
				OffsetRect(&firstRect,27,0);
				*theRect = firstRect;
			}
			wind = FrontWindow();
		} else {
			wind = (WindowPtr) ((WindowPeek) wind)->nextWindow;
		}
	}
		
	if (theRect->top > gDesktopExtent.bottom)
		theRect->top = gDesktopExtent.bottom - 10;
	if (theRect->left > gDesktopExtent.right)
		theRect->left = gDesktopExtent.right - 10;
	if (theRect->bottom > gDesktopExtent.bottom && 
		theRect->top < (gDesktopExtent.bottom-150))
		theRect->bottom = gDesktopExtent.bottom;
	if (theRect->right > gDesktopExtent.right && 
		theRect->left < (gDesktopExtent.right-150))
		theRect->right = gDesktopExtent.right;
				
	return(theRect);
}


/*	CalcWindowHCoords calculates the horizontal coordinates of the various
	components of a list window and records them in the TWindow struct
	for the window. For subject windows it also (re)creates the collapsed
	and expanded triangle polygons and saves handles to them in the TWindow
	struct.
*/

void CalcWindowHCoords (WindowPtr wind)
{
	TWindow **info;
	EWindowKind kind;
	short w;
	GrafPtr savePort;
	FontInfo fontInfo;
	short tHeight, tWidth, x;
	long response;
	
	GetPort(&savePort);
	SetPort(wind);
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	w = CharWidth('9');
	if (kind == kUserGroup) {
		(**info).groupNameHCoord = 6*w;
		(**info).numUnreadHCoord = 4*w;
	} else if (kind == kSubject) {
		GetFontInfo(&fontInfo);
		tWidth = fontInfo.ascent;
		if ((tWidth & 1) == 1) tWidth--;
		(**info).minusSignHCoord = ((tWidth - CharWidth('-')) >> 1) + 2;
		(**info).threadCountHCoord = x = 3*w + tWidth;
		(**info).checkHCoord = x = x + 2*w;
		if ((**info).authorsShown) {
			(**info).authorHCoord = x = x + 2*w;
			(**info).authorWidth = 16*w;
			(**info).subjectHCoord = x + 18*w;
		} else {
			(**info).subjectHCoord = (**info).authorHCoord = x = x + 2*w;
		}
		tHeight = tWidth >> 1; 
		if ((**info).collapseTriangle != nil) KillPoly((**info).collapseTriangle);
		if ((**info).expandTriangle != nil) KillPoly((**info).expandTriangle);
		(**info).collapseTriangle = OpenPoly();
			MoveTo(0,0);
			LineTo(0,tWidth);
			LineTo(tHeight,tHeight);
			LineTo(0,0);
		ClosePoly();
		(**info).expandTriangle = OpenPoly();
			MoveTo(0,0);
			LineTo(tWidth,0);
			LineTo(tHeight,tHeight);
			LineTo(0,0);
		ClosePoly();
	}
	SetPort(savePort);
}


/*	NewList makes a new list manager list for list windows */

static void NewList (WindowPtr wind)
{
	TWindow **info;
	ListHandle theList;
	Point thePt;
	Rect listRect, sizeRect;
	GrafPtr savePort;
	short fontNum;
	
	info = (TWindow**)GetWRefCon(wind);
	
	SetPt(&thePt,0,0);
	SetRect(&sizeRect,0,0,1,0);
	SetRect(&listRect,
		wind->portRect.left,
		wind->portRect.top,
		wind->portRect.right-15,
		wind->portRect.bottom-15);
	GetPort(&savePort);
	SetPort(wind);
	
	GetFNum(gPrefs.listFont, &fontNum);
	TextFont(fontNum);
	TextSize(gPrefs.listSize);
	theList = LNew(&listRect, &sizeRect, thePt, kLDEFProc,
		wind, false, true, false, true);
	(**theList).indent.h = 4;
	(**theList).selFlags |= lNoNilHilite;
	(**theList).refCon = (long)ListDefFunc;
	(**theList).lActive = wind == FrontWindow();
	(**info).theList = theList;
	CalcWindowHCoords(wind);
	SetPort(savePort);
}


/*	CalcPanelHeight calculates the panel height for an article or message 
	window. 
*/

void CalcPanelHeight (WindowPtr wind)
{
	TWindow **info;
	FontInfo fontInfo;
	EWindowKind kind;
	short panelHeight;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	if (kind != kArticle && kind != kMiscArticle && kind != kPostMessage &&
		kind != kMailMessage) return;
	GetFontInfo(&fontInfo);
	if (kind == kMailMessage || kind == kPostMessage) {
		panelHeight = fontInfo.ascent + fontInfo.descent + 9;
		if (panelHeight < 34) panelHeight = 34;
	} else {
		panelHeight = 3 * (fontInfo.ascent + fontInfo.descent + 
			fontInfo.leading) + 9;
	}
	(**info).panelHeight = panelHeight;
}


/*	NewText creates a new textedit record for article and message windows.
*/

static void NewText (WindowPtr wind)
{
	TWindow **info;
	TEHandle theTE;
	Rect theRect;
	short fontNum;
	
	info = (TWindow**)GetWRefCon(wind);
	
	SetPort(wind);
	
	GetFNum(gPrefs.textFont,&fontNum);
	TextFont(fontNum);
	TextSize(gPrefs.textSize);
	
	CalcPanelHeight(wind);
	
	SetRect(&theRect,
		wind->portRect.left,
		wind->portRect.top + (**info).panelHeight,
		wind->portRect.right-15,
		wind->portRect.bottom-15);
	InsetRect(&theRect,kTextMargin,kTextMargin);

	theTE = TENew(&theRect,&theRect);
	SetClikLoop(AutoScroll,theTE);
	
	(**info).theTE = theTE;
}


/*	NewSendButton creates a new "Send" button control for message windows. */

static void NewSendButton (WindowPtr wind)
{
	TWindow **info;
	Rect theRect;
	
	info = (TWindow**)GetWRefCon(wind);
	theRect.right = wind->portRect.right - 5;
	theRect.left = theRect.right-60;
	theRect.top = ((**info).panelHeight - 24) >> 1;
	theRect.bottom = theRect.top + 20;
	NewControl(wind, &theRect, "\pSend", true, 0, 0, 0, 
		pushButProc, kSendButton);
}


/*	MakeNewWindow is the low-level procedure to make a new NewsWatcher window.
	Depending on what type is needed, the window will have different
	features.
*/

WindowPtr MakeNewWindow (EWindowKind kind, Point topLeft, StringPtr title)
{
	
	WindowPtr wind, front, behind;
	TWindow **info;
	Rect bounds;
	short rightMark;
	long response;
		
	switch (kind) {
		case kFullGroup:
		case kNewGroup:
		case kUserGroup:
			rightMark = topLeft.h+300;
			break;
		case kSubject:
			rightMark = topLeft.h+400;
			break;
		case kArticle:
		case kMiscArticle:
		case kMailMessage:
		case kPostMessage:
			rightMark = topLeft.h+495;
			break;
	}

	SetRect(&bounds,
		topLeft.h,
		topLeft.v,
		rightMark,
		topLeft.v + kDefaultWindHeight);
	front = FrontWindow();
	behind = (IsStatusWindow(front)) ? front : (WindowPtr)-1;
	if (gHasColorQD) {
		wind = NewCWindow(nil, MyOffSet(&bounds), "\pUntitled", false,
			zoomDocProc, behind, true, 0);
	} else {
		wind = NewWindow(nil,MyOffSet(&bounds), "\pUntitled", false,
			zoomDocProc, behind, true, 0);
	}
	SetPort(wind);
	SetWTitle(wind, title);
	AddWindMenu(wind);
	
	info = (TWindow**)MyNewHandle(sizeof(TWindow));
	if (MyMemErr() != noErr)
		return nil;
		
	(**info).kind = kind;
	SetWRefCon(wind, (long)info);
	
	switch (kind) {
		case kFullGroup:
		case kNewGroup:
		case kUserGroup:
			NewList(wind);
			(**(**info).theList).lClikLoop = (ProcPtr)GroupListClickLoop;
			(**info).strings = gGroupNames;
			break;
		case kSubject:
			(**info).authorsShown = gPrefs.showAuthors;
			NewList(wind);
			break;
		case kArticle:
		case kMiscArticle:
		case kPostMessage:
		case kMailMessage:
			NewText(wind);
			MakeVScroller(wind);
			if (kind == kPostMessage || kind == kMailMessage) 
				NewSendButton(wind);
			break;
	}
	SizeContents(wind);

	return wind;
}


/*	NewUserGroupWindow creates a new user group window */

WindowPtr NewUserGroupWindow (StringPtr title, TGroup **groupArray, short numGroups)
{
	WindowPtr wind;
	TWindow **info;
	Point firstOffset;
	Cell theCell;
	
	SetPt(&firstOffset, kOffLeft, kOffTop);
	
	info = (TWindow**) GetWRefCon(wind = 
		MakeNewWindow(kUserGroup, firstOffset, title));
	if (groupArray == nil) {
		(**info).groupArray = (TGroup**)MyNewHandle(0);
	} else {
		(**info).groupArray = groupArray;
		(**info).numGroups = numGroups;
		MakeGroupList(numGroups, (**info).theList);
		SetPt(&theCell, 0, 0);
		LSetSelect(true, theCell, (**info).theList);
	}
	DoZoom(wind, inZoomOut);
	return wind;
}
