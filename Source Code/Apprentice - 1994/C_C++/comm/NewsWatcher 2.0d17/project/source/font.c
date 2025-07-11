/*----------------------------------------------------------------------------

	font.c

	This module handles the Font and Size menu commands.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "font.h"
#include "menus.h"
#include "open.h"
#include "resize.h"
#include "util.h"
#include "wind.h"


/*	UpdateFontInfo updates the font information for a window */

static void UpdateFontInfo (WindowPtr wind)
{
	FontInfo fontInfo;
	TWindow **info;
	ListHandle theList;
	TEHandle theTE;
	Point newSize;
		
	GetFontInfo(&fontInfo);
	info = (TWindow**)GetWRefCon(wind);
	
	switch ((**info).kind) {
		case kFullGroup:
		case kNewGroup:
		case kUserGroup:
		case kSubject:
			theList = (**info).theList;
			newSize.h = (**theList).cellSize.h;
			newSize.v = fontInfo.ascent+fontInfo.descent+fontInfo.leading;
			LCellSize(newSize,theList);
			CalcWindowHCoords(wind);
			break;
		case kArticle:
		case kMiscArticle:
		case kMailMessage:
		case kPostMessage:
			theTE = (**info).theTE;
			(**theTE).txFont = wind->txFont;
			(**theTE).txFace = wind->txFace;
			(**theTE).txMode = wind->txMode;
			(**theTE).txSize = wind->txSize;
			(**theTE).lineHeight = 
				fontInfo.ascent+fontInfo.descent+fontInfo.leading;
			(**theTE).fontAscent = fontInfo.ascent;
			CalcPanelHeight(wind);
			break;
	}
	SizeContents(wind);
	if (gPrefs.zoomWindows) {
		DoZoom(wind, inZoomOut);
	} else {
		SetWindowNeedsZooming(wind);
	}
}

void FontWasChanged (Boolean listFont)
{
	WindowPtr	wind;
	WindowPeek	peek;
	TWindow **	info;
	EWindowKind kind;
	short		fontID, fontSize;
	Boolean		changeWind;
	GrafPtr		savePort;

	/* Figure out the new font ID and size */
	if (listFont) {
		GetFNum(gPrefs.listFont, &fontID);
		fontSize = gPrefs.listSize;
	} else {
		GetFNum(gPrefs.textFont, &fontID);
		fontSize = gPrefs.textSize;
	}

	/* Loop through all the windows and see if they need updating */ 
	GetPort(&savePort);
	wind = FrontWindow();
	while (wind != nil) {
		peek = (WindowPeek)wind;
		if (!IsStatusWindow(wind) && IsAppWindow(wind)) {
			info = (TWindow**)GetWRefCon(wind);
			kind = (**info).kind;
			changeWind = false;
			if (listFont) {
				changeWind = (kind <= kSubject);
			} else {
				changeWind = (kind >= kArticle && kind <= kPostMessage);
			}
			if (changeWind) {
				if (!listFont) (**(**info).theTE).txFont = fontID;
				SetPort(wind);
				TextFont(fontID);
				TextSize(fontSize);
				UpdateFontInfo(wind);
				InvalRect(&wind->portRect);
			}
		}
		wind = (WindowPtr)peek->nextWindow;
	}
}
