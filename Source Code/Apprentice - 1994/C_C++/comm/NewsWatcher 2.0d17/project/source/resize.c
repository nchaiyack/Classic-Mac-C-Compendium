/*----------------------------------------------------------------------------

	resize.c

	This module handles window resizing (growing and zooming).
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <string.h>

#include "glob.h"
#include "resize.h"
#include "scroll.h"
#include "util.h"


/*	SendButtonCont returns a handle to the Send button control for a
	message window. */

static ControlHandle SendButtonCont (WindowPtr wind)
{
	ControlHandle theControl;
	
	theControl = ((WindowPeek)wind)->controlList;
	
	while (theControl != nil && GetCRefCon(theControl) != kSendButton) 
		theControl = (**theControl).nextControl;
	return theControl;
}


/*	RedoControls is called when an article or message window is re-sized 
	and the controls for the window need to be moved/resized.
*/

static void RedoControls (WindowPtr wind)
{
	TWindow **info;
	EWindowKind kind;
	short panelHeight;
	ControlHandle vScroll, hScroll, sendButton;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	panelHeight = (**info).panelHeight;

	vScroll = VScrollCont(wind);
	hScroll = HScrollCont(wind);
	
	HideControl(vScroll);
	MoveControl(vScroll,
		wind->portRect.right - 15,
		wind->portRect.top + panelHeight - 1);
	SizeControl(vScroll,
		16,
		wind->portRect.bottom - wind->portRect.top - panelHeight - 13);
	ShowControl(vScroll);
	
	if (hScroll != nil) {
		HideControl(hScroll);
		MoveControl(hScroll,
			wind->portRect.left + kSectionMargin,
			wind->portRect.bottom - 15);
		SizeControl(hScroll,
			wind->portRect.right - wind->portRect.left - kSectionMargin - 14,
			16);
		ShowControl(hScroll);
	}
	
	if (kind == kMailMessage || kind == kPostMessage) {
		sendButton = SendButtonCont(wind);
		MoveControl(sendButton,
			wind->portRect.right - 65,
			wind->portRect.top + ((panelHeight - 24) >> 1));
	}
}


/*	FixText is called when a textedit window has been re-sized and the text
	needs to be re-flowed.
*/

static void FixText (WindowPtr wind)
{	
	TWindow **info;
	short controlLine,charPos;
	Rect viewRect;
	short lineHeight;
	TEHandle theTE;
	ControlHandle sBar;
	
	info = (TWindow**)GetWRefCon(wind);
	theTE = (**info).theTE;
	sBar = VScrollCont(wind);

	lineHeight = (**theTE).lineHeight;
	viewRect = wind->portRect;
	viewRect.right -= 15;
	viewRect.bottom -= 15;
	viewRect.top += (**info).panelHeight;
	InsetRect(&viewRect, kTextMargin, kTextMargin);
	(**theTE).destRect = viewRect;
	(**theTE).viewRect = viewRect;
	TECalText(theTE);
	controlLine = GetCtlValue(sBar);
	charPos = (**theTE).lineStarts[controlLine];
	AdjustScrollBar(wind);
	ScrollChar(wind, charPos, false);
}


/*	FixHeight calculates the height of a window after adjusting it to be an
	exact even multiple of cells or text lines.
	
	Entry:	wind = pointer to window.
			height = window height.
			
	Exit:	function result = adjusted height.
*/

static short FixHeight (WindowPtr wind, short height)
{
	TWindow **info;
	ListHandle theList;
	TEHandle theTE;
	short h, adjust;
	
	info = (TWindow**)GetWRefCon(wind);
	adjust = (**info).panelHeight + 15;
	theList = (**info).theList;
	if (theList != nil) {
		 h = (**theList).cellSize.v;
	} else {
		adjust += 2*kTextMargin;
		theTE = (**info).theTE;
		h = (**theTE).lineHeight;
	}
	height = (height - adjust) / h * h + adjust;
	return height;
}


/*	SizeContents resizes the window's contents, handling all types of windows.
	It also adjusts the window height so that it contains an exact even multiple
	of cells or text lines.
	
	Entry:	wind = pointer to window.
*/

void SizeContents (WindowPtr wind)
{
	TWindow **info;
	ListHandle theList;
	Point tmpPt;
	short width, height;
	
	info = (TWindow**) GetWRefCon(wind);
	width = wind->portRect.right;
	height = FixHeight(wind, wind->portRect.bottom);
	SizeWindow(wind, width, height, false);

	switch ((**info).kind) {
		case kFullGroup:
		case kNewGroup:
		case kUserGroup:
		case kSubject:
			theList = (**info).theList;
			LSize(width-15, height-15, theList);
			SetPt(&tmpPt, width-15, (**theList).cellSize.v);
			LCellSize(tmpPt, theList);
			break;
		case kArticle:
		case kMiscArticle:
		case kPostMessage:
		case kMailMessage:
			RedoControls(wind);
			FixText(wind);
			break;
	}
	InvalRect(&wind->portRect);
}


/*	CalcZoom Calculates the maximum needed size for the window.
*/

static Boolean CalcZoom (WindowPtr wind, Rect *zoomRect)
{
	TWindow **info;
	ListHandle theList;
	TEHandle theTE,tmpTE;
	Handle tmpHandle;
	long width,height,tmpWidth;
	Rect finalRect, tmpRect, userRect;
	Cell theCell;
	TGroup **groupArray;
	TSubject **subjectArray;
	char theString[257];
	short strLen;
	short cellData, cellDataLen;
	short numCells;
	char *p, *q, *pEnd;
	Boolean pastHeader;
	short headerWidth, bodyWidth, nominalWidth;
	Handle strings;
	EWindowKind kind;
	
	info = (TWindow**)GetWRefCon(wind);
	kind = (**info).kind;
	finalRect = *zoomRect;
	userRect = wind->portRect;
	LocalToGlobal((Point*)&userRect.top);
	LocalToGlobal((Point*)&userRect.bottom);
	
	switch (kind) {
		case kFullGroup:
		case kNewGroup:
		case kUserGroup:
		case kSubject:
			theList = (**info).theList;
			height = 20L + (long)((**theList).dataBounds.bottom - 
				(**theList).dataBounds.top) * 
				(long)(**theList).cellSize.v;
			if (kind == kFullGroup && gPrefs.maxGroupNameWidth != 0) {
				width = gPrefs.maxGroupNameWidth;
			} else {
				strings = (**info).strings;
				if (kind == kSubject) {
					subjectArray = (**info).subjectArray;
				} else {
					groupArray = (**info).groupArray;
				}
				numCells = (**theList).dataBounds.bottom;
				width = 0;
				theCell.h = 0;
				for (theCell.v=0; theCell.v < numCells; theCell.v++) {
					if (!GiveTime()) return false;
					cellDataLen = 2;
					LGetCell(&cellData, &cellDataLen, theCell, theList);
					if (kind == kSubject) {
						*theString = 'Ã';
						strcpy(theString+1, 
							*strings + (*subjectArray)[cellData].subjectOffset);
					} else {
						strcpy(theString, 
							*strings + (*groupArray)[cellData].nameOffset);
					}
					strLen = strlen(theString);
					tmpWidth = TextWidth(theString, 0, strLen);
					if (tmpWidth > width) width = tmpWidth;
				}
				if (kind == kFullGroup) gPrefs.maxGroupNameWidth = width;
			}
			width += 25;
			if (kind == kUserGroup) {
				width += (**info).groupNameHCoord;
			} else if (kind == kSubject) {
				width += (**info).subjectHCoord;
			}
			break;
		case kArticle:
		case kMiscArticle:
			theTE = (**info).theTE;
			HLock((Handle)(**theTE).hText);
			p = *(**theTE).hText;
			pastHeader = false;
			pEnd = p + (**theTE).teLength;
			headerWidth = bodyWidth = 0;
			while (p < pEnd) {
				if (!GiveTime()) {
					HUnlock((Handle)(**theTE).hText);
					return false;
				}
				for (q = p; q < pEnd && *q != CR; q++);
				tmpWidth = TextWidth(p, 0, q-p);
				if (pastHeader) {
					if (tmpWidth > bodyWidth) bodyWidth = tmpWidth;
				} else if (q == p+1) {
					pastHeader = true;
				} else {
					if (tmpWidth > headerWidth) headerWidth = tmpWidth;
				}
				p = q+1;
			}
			nominalWidth = 80 * CharWidth('W');
			if (bodyWidth <= nominalWidth && headerWidth <= nominalWidth) {
				width = bodyWidth > headerWidth ? bodyWidth : headerWidth;
			} else {
				width = nominalWidth;
			}
			HUnlock((Handle)(**theTE).hText);
			width += (2*kTextMargin) + 18;
			if (LinesInText(theTE) > 100) {
				height = 0x7fff;
			} else {
				if (width < 200) width = 200;
				if (finalRect.right-finalRect.left < width) 
					width = finalRect.right - finalRect.left;
				tmpRect = wind->portRect;
				tmpRect.right = tmpRect.left + width - 15 - 2*kTextMargin;
				SetPort(wind);
				tmpTE = TENew(&tmpRect,&tmpRect);
				tmpHandle = (**tmpTE).hText;
				(**tmpTE).hText = (**theTE).hText;
				TECalText(tmpTE);
				height = 15 + 2*kTextMargin + (**info).panelHeight + 
					LinesInText(tmpTE) * (**tmpTE).lineHeight;
				(**tmpTE).hText = tmpHandle;
				TEDispose(tmpTE);
			}
			break;
		case kMailMessage:
		case kPostMessage:
			height = 0x7fff;
			width = 80 * CharWidth('W') + (2*kTextMargin) + 18;
			break;
	}

	if (height < kMinWindHeight) height = kMinWindHeight;
	if (width < kMinWindWidth) width = kMinWindWidth;

	if ( (finalRect.right-finalRect.left) > width ) {
		finalRect.right = userRect.left+width;
		finalRect.left = userRect.left;
	}
	if ( (finalRect.bottom-finalRect.top) > height ) {
		finalRect.bottom = userRect.top+height;
		finalRect.top = userRect.top;
	}
	if (finalRect.bottom > zoomRect->bottom) {
		OffsetRect(&finalRect,0,-(finalRect.bottom-zoomRect->bottom));
	}
	if (finalRect.right > zoomRect->right) {
		OffsetRect(&finalRect,-(finalRect.right-zoomRect->right),0);
	}
	if (finalRect.top < zoomRect->top) {
		OffsetRect(&finalRect,0,zoomRect->top - finalRect.top);
		if (finalRect.bottom > zoomRect->bottom)
			finalRect.bottom = zoomRect->bottom;
	}
	if (finalRect.left < zoomRect->left) {
		OffsetRect(&finalRect,zoomRect->left - finalRect.left,0);
		if (finalRect.right > zoomRect->right)
			finalRect.right = zoomRect->right;
	}
	finalRect.bottom = finalRect.top + 
		FixHeight(wind, finalRect.bottom - finalRect.top);
	*zoomRect = finalRect;
	return true;
}


/* DoZoom: Handles zoom-window events -- from Apple Technical Note #??? */

Boolean DoZoom (WindowPtr wind, short zoomDir)
{
	Rect windRect, theSect, zoomRect, tmpRect, testRect;
	GDHandle nthDevice,dominantGDevice;
	long sectArea,greatestArea;
	short bias = 18;
	Boolean sectFlag;
	GrafPtr savePort;
	WStateData **stateHndl;
	
	GetPort(&savePort);
	SetPort(wind);
	
	stateHndl = (WStateData **) ((WindowPeek)wind)->dataHandle;
	
	if (zoomDir == inZoomOut) {
		windRect = wind->portRect;
		LocalToGlobal((Point*)&windRect.top);
		LocalToGlobal((Point*)&windRect.bottom);
		if (gHasColorQD) {
			testRect = windRect;
			testRect.top -= bias;
			nthDevice = GetDeviceList();
			dominantGDevice = 0;
			greatestArea = 0;
			while (nthDevice != nil) {
				if (TestDeviceAttribute(nthDevice,screenDevice) &&
					TestDeviceAttribute(nthDevice,screenActive)) {
						sectFlag = SectRect(&testRect,&((**nthDevice).gdRect),&theSect);
						sectArea = (long)(theSect.right - theSect.left) * 
							(long)(theSect.bottom - theSect.top);
						if (sectArea > greatestArea) {
							greatestArea = sectArea;
							dominantGDevice = nthDevice;
						}
				}
				nthDevice = GetNextDevice(nthDevice);
			}
			
			if (dominantGDevice == GetMainDevice())
				bias += GetMBarHeight();
			
			if (dominantGDevice)
				tmpRect = (**dominantGDevice).gdRect;
			else
				tmpRect = gDesktopExtent;
			
			zoomRect = tmpRect;
			zoomRect.top += bias;
			InsetRect(&zoomRect, 4, 4);
		} else {
			zoomRect = qd.screenBits.bounds;
			zoomRect.top += GetMBarHeight() + bias;
			InsetRect(&zoomRect, 4, 4);
		}
		if (!CalcZoom(wind, &zoomRect)) {
			SetPort(savePort);
			return false;
		}
		if (EqualRect(&windRect, &zoomRect)) {
			SetPort(savePort);
			return true;
		}
		(**stateHndl).stdState = zoomRect;
	}
	
	
	EraseRect(&wind->portRect);
	ZoomWindow(wind,zoomDir,false);

	InvalRect(&wind->portRect);
	SizeContents(wind);
	SetRect(&tmpRect,wind->portRect.left,wind->portRect.bottom-16,
				wind->portRect.right,wind->portRect.bottom);
	InvalRect(&tmpRect);
	
	SetPort(savePort);
	return true;
}


/* DoGrow handles grow window events */

void DoGrow (WindowPtr wind, Point globMouse)
{
	long newSize;
	Rect tmpRect;
	GrafPtr	savePort;
	WStateData **stateHndl;
	
	if ((newSize = GrowWindow(wind,globMouse,&gWindLimits)) != 0) {
		GetPort(&savePort);
		SetPort(wind);
		SetRect(&tmpRect,wind->portRect.right-15-kTextMargin,
			wind->portRect.top,
			wind->portRect.right,
			wind->portRect.bottom);
		InvalRect(&tmpRect);
		SetRect(&tmpRect,wind->portRect.left,
			wind->portRect.bottom-15-kTextMargin,
			wind->portRect.right-16,
			wind->portRect.bottom);
		InvalRect(&tmpRect);
		
		SizeWindow(wind, LoWord(newSize), HiWord(newSize), true);
		SizeContents(wind);
		
		SetRect(&tmpRect,wind->portRect.right-15,
			wind->portRect.top,
			wind->portRect.right,
			wind->portRect.bottom);
		InvalRect(&tmpRect);
		SetRect(&tmpRect,wind->portRect.left,
			wind->portRect.bottom-15,
			wind->portRect.right-15,
			wind->portRect.bottom);
		InvalRect(&tmpRect);
		
		stateHndl = (WStateData**)((WindowPeek)wind)->dataHandle;
		tmpRect = wind->portRect;
		LocalToGlobal((Point*)&tmpRect.top);
		LocalToGlobal((Point*)&tmpRect.bottom);
		(**stateHndl).userState = tmpRect;
		
		SetPort(savePort);
	}
}
