/*

		Help Window.c++
		by Hiep Dam, 3G Software.
		March 1994.
		Last Update April 1994
		Contact: America Online: Starlabs
		         Delphi        : StarLabs
		         Internet      : starlabs@aol.com
		                      or starlabs@delphi.com

		Much of this code (especially scrolling) is by Dana Basken.
		Thanks Dana!

*/

#include "QDUtils.h"
#include "Compat.h"
#include "Help Window.h"

enum {
	kHelpTextID = 128
};

WindowPtr gHelpWindow;
Handle gHelpText;
TEHandle gHelpTE;
StScrpHandle gHelpStyle;
ControlHandle gScrollbar;
Rect gHelpWindowBounds;
Rect gTextFrame;
Rect gTextView;

void InitHelpWindow() {
	GrafPtr savePort;
	Rect textView, textFrame, scrollRect;
	short lineHeight, ascent;
	TextStyle theStyle;

	GetPort(&savePort);

	SetRect(&gHelpWindowBounds, 0, 0, 400, 300);
	SetRect(&gTextFrame, gHelpWindowBounds.left + 2, gHelpWindowBounds.top + 4,
		gHelpWindowBounds.right - 2 - 16, gHelpWindowBounds.bottom - 4);
	SetRect(&scrollRect, gTextFrame.right - 1, 4, gTextFrame.right - 1 + 16, 296);
	gTextView = gTextFrame;
	InsetRect(&gTextView, 4, 2);

	CenterRect(&gHelpWindowBounds, &screenBits.bounds);
	if (gEnviron.hasColor)
		gHelpWindow = NewCWindow(nil, &gHelpWindowBounds, "\pRgnMaster Help & Information",
			false, noGrowDocProc, nil, true, 0);
	else
		gHelpWindow = NewWindow(nil, &gHelpWindowBounds, "\pRgnMaster Help & Information",
			false, noGrowDocProc, nil, true, 0);
	SetPort(gHelpWindow);

	gHelpStyle = (StScrpHandle)GetNamedResource('styl', "\pHelp");
	HLockHi((Handle)gHelpStyle);
	gHelpText = GetNamedResource('TEXT', "\pHelp");
	HLockHi(gHelpText);

	gHelpTE = TEStylNew(&gTextView, &gTextView);
	TEStylInsert(*gHelpText, GetHandleSize(gHelpText), gHelpStyle, gHelpTE);
	TECalText(gHelpTE);

	// Get lineheight, at arbitrary character 100 (just a random pick).
	// Need the lineheight in order to determine # of lines for the scrollbar
	TEGetStyle(100, &theStyle, &lineHeight, &ascent, gHelpTE);
	gScrollbar = NewControl(gHelpWindow, &scrollRect, "\pScroll", true, 1, 1,
		(**gHelpTE).nLines - ((gTextView.bottom - gTextView.top) / lineHeight) + 1,
		scrollBarProc, 0);

	HUnlock((Handle)gHelpStyle);
	ReleaseResource((Handle)gHelpStyle);
	HUnlock(gHelpText);
	ReleaseResource(gHelpText);

	SetPort(savePort);
} // END InitHelpWindow

void ShowHelpWindow() {
	ShowWindow(gHelpWindow);
	SelectWindow(gHelpWindow);
} // END ShowHelpWindow

void HideHelpWindow() {
	TextStyle theStyle;
	short lineHeight, ascent;
	short curVal;

	HideWindow(gHelpWindow);

	TEGetStyle(100, &theStyle, &lineHeight, &ascent, gHelpTE);
	curVal = GetCtlValue(gScrollbar);
	TEScroll(0, lineHeight * (curVal-1), gHelpTE);
	SetCtlValue(gScrollbar, 1);
} // END HideHelpWindow

void UpdateHelpWindow() {
	GrafPtr savePort;
	
	GetPort(&savePort);

	SetPort(gHelpWindow);
	EraseRect(&gTextFrame);
	TEUpdate(&gTextView, gHelpTE);
	FrameRect(&gTextFrame);
	DrawControls(gHelpWindow);

	SetPort(savePort);
} // END UpdateHelpWindow

pascal void HelpWindowScrollProc(ControlHandle theControl, short actionPart) {
	TextStyle theStyle;
	short lineHeight, ascent;

	short minVal = GetCtlMin(theControl);
	short maxVal = GetCtlMax(theControl);
	short curVal = GetCtlValue(theControl);
	short oldVal = curVal;

	TEGetStyle(100, &theStyle, &lineHeight, &ascent, gHelpTE);

	switch(actionPart) {
		case inUpButton:
			if (curVal != minVal) {
				SetCtlValue(theControl, curVal - 1);
				TEScroll(0, lineHeight, gHelpTE);
			}
		break;
		
		case inDownButton:
			if (curVal != maxVal) {
				SetCtlValue(theControl, curVal + 1);
				TEScroll(0, -lineHeight, gHelpTE);
			}
		break;
		
		case inPageUp:
			// Attempt to move control up 1/8 of total
			curVal -= ((maxVal - minVal) / 8);
			if (curVal < minVal)
				curVal = minVal;
			SetCtlValue(theControl, curVal);
			TEScroll(0, lineHeight * (oldVal - curVal), gHelpTE);
		break;
		
		case inPageDown:
			curVal += ((maxVal - minVal) / 8);
			if (curVal > maxVal)
				curVal = maxVal;
			SetCtlValue(theControl, curVal);
			TEScroll(0, lineHeight * (oldVal - curVal), gHelpTE);
		break;
	} // END switch
} // END HelpWindowScroll

void ScrollHelpWindow(Point mouseLoc) {
	ControlHandle hitControl;
	short hitLoc, dummy, oldVal, curVal;
	
	hitLoc = FindControl(mouseLoc, gHelpWindow, &hitControl);
	if (hitControl == gScrollbar) {
		switch(hitLoc) {
			case inUpButton:
			case inDownButton:
			case inPageUp:
			case inPageDown:
				dummy = TrackControl(hitControl, mouseLoc, (ProcPtr)HelpWindowScrollProc);
			break;
			
			case inThumb:
				TextStyle theStyle;
				short lineHeight, ascent;
				TEGetStyle(100, &theStyle, &lineHeight, &ascent, gHelpTE);

				oldVal = GetCtlValue(hitControl);
				dummy = TrackControl(hitControl, mouseLoc, nil);
				curVal = GetCtlValue(hitControl);
				if (oldVal != curVal)
					TEScroll(0, lineHeight * (oldVal - curVal), gHelpTE);
			break;
		}
	}
} // END ScrollHelpWindow

// END Help Window.c++