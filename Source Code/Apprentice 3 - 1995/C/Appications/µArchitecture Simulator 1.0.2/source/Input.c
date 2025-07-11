/*
Copyright � 1993,1994,1995 Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"CursorBalloon.h"
#include	"Globals.h"
#include	"Input.h"
#include	"Main.h"
#include	"SimUtils.h"

//#pragma segment Main

/*	kTextMargin is the number of pixels we leave blank at the edge of the window. */
enum {
kTextMargin = 2,

/*	kCrChar is used to match with a carriage return when calculating the
	number of lines in the TextEdit record. */
kCrChar = 13,

/*	kButtonScroll is how many pixels to scroll horizontally when the button part
	of the horizontal scrollbar is pressed. */
kButtonScroll = 4
};
	
/* static routines */
static void GetLocalUpdateRgn(WindowPtr window, RgnHandle localRgn);
static pascal void VActionProc(ControlHandle control, short part);
static pascal void HActionProc(ControlHandle control, short part);
static void DrawWindow(WindowPtr window);
static void AdjustTE(WindowPtr window);
static void AdjustHorz(ControlHandle control, TEHandle docTE);
static void AdjustVert(ControlHandle control, TEHandle docTE);
static void AdjustScrollSizes(WindowPtr window);
static void AdjustScrollValues(WindowPtr window);
static short LinesInTE(TEHandle theTE);


void IOHome(void)
{
register ControlHandle	vscrollBar = ((DocumentPeek)gWPtr_IO)->docVScroll;

SetControlValue(vscrollBar, GetControlMinimum(vscrollBar));
AdjustTE(gWPtr_IO);
}

void IOEnd(void)
{
register ControlHandle	vscrollBar = ((DocumentPeek)gWPtr_IO)->docVScroll;

SetControlValue(vscrollBar, GetControlMaximum(vscrollBar));
AdjustTE(gWPtr_IO);
}

void IOPgUp(void)
{
VActionProc(((DocumentPeek)gWPtr_IO)->docVScroll, kInPageUpControlPart);
}

void IOPgDn(void)
{
VActionProc(((DocumentPeek)gWPtr_IO)->docVScroll, kInPageDownControlPart);
}

/*	Called when a mouseDown occurs in the grow box of an active window. In
	order to eliminate any 'flicker', we want to invalidate only what is
	necessary. Since ResizeWindow invalidates the whole portRect, we save
	the old TE viewRect, intersect it with the new TE viewRect, and
	remove the result from the update region. However, we must make sure
	that any old update region that might have been around gets put back. */

void DoGrowWindow(WindowPtr window, EventRecord	*event)
{
Rect	tempRect;
register long	growResult;
register RgnHandle	tempRgn;
register DocumentPeek doc;

if (window == gWPtr_IO) {
	tempRect.right = tempRect.bottom = 0x7FFF;		/* set up limiting values */
	tempRect.top = tempRect.left = kMinDocDim;
/* see if it really changed size */
	if (growResult = GrowWindow(window, event->where, &tempRect)) {
		doc = (DocumentPeek) window;
		tempRect = (*doc->docTE)->viewRect;		/* save old text box */
		tempRgn = NewRgn();
		GetLocalUpdateRgn(window, tempRgn);		/* get localized update region */
		SizeWindow((WindowPtr)doc, LoWrd(growResult), HiWrd(growResult), true);
		ResizeWindow((WindowPtr)doc);
/* calculate & validate the region that hasn�t changed so it won�t get redrawn */
		SectRect(&tempRect, &(*doc->docTE)->viewRect, &tempRect);
		ValidRect(&tempRect);				/* take it out of update */
		InvalRgn(tempRgn);					/* put back any prior update */
		DisposeRgn(tempRgn);
		}
	}
} /* DoGrowWindow */


/* 	Called when a mouseClick occurs in the zoom box of an active window.
	Everything has to get re-drawn here, so we don't mind that
	ResizeWindow invalidates the whole portRect. */

void DoZoomWindow(WindowPtr window)
{
AdjustScrollbars(window, true);
AdjustTE(window);
} /*  DoZoomWindow */


/* Called when the window has been resized to fix up the controls and content. */

void ResizeWindow(WindowPtr window)
{
InvalRect(&window->portRect);
AdjustScrollbars(window, true);
AdjustTE(window);
} /* ResizeWindow */


/* Returns the update region in local coordinates */

void GetLocalUpdateRgn(WindowPtr window, RgnHandle localRgn)
{
Point	mypt = { 0, 0};

LocalToGlobal(&mypt);
CopyRgn(((WindowPeek) window)->updateRgn, localRgn);/* save old update region */
OffsetRgn(localRgn, mypt.h, mypt.v);
} /* GetLocalUpdateRgn */


/*	This is called when an update event is received for a window.
	It calls DrawWindow to draw the contents of an application window.
	As an efficiency measure that does not have to be followed, it
	calls the drawing routine only if the visRgn is non-empty. This
	will handle situations where calculations for drawing or drawing
	itself is very time-consuming. */

void DoUpdateWindow(WindowPtr window)
{
if (window == gWPtr_IO)
	if (EmptyRgn(window->visRgn) == false)	/* draw if updating needs to be done */
		DrawWindow(window);
} /*DoUpdate*/


/*	This is called when a window is activated or deactivated.
	It calls TextEdit to deal with the selection. */

void DoActivateWindow(EventRecord *evt, WindowPtr window, Boolean becomingActive)
{
Rect	growRect;
register RgnHandle	tempRgn, clipRgn;
register DocumentPeek doc;

if (window == gWPtr_IO) {
	doc = (DocumentPeek) window;
/* the growbox needs to be redrawn on activation: */
	growRect = window->portRect;
/* adjust for the scrollbars */
	growRect.top = growRect.bottom - kScrollbarAdjust + 1;
	growRect.left = growRect.right - kScrollbarAdjust + 1;
	if (becomingActive) {
/*	since we don�t want TEActivate to draw a selection in an area where
	we�re going to erase and redraw, we�ll clip out the update region
	before calling it. */
		tempRgn = NewRgn();
		clipRgn = NewRgn();
		GetLocalUpdateRgn(window, tempRgn);		/* get localized update region */
		GetClip(clipRgn);
		DiffRgn(clipRgn, tempRgn, tempRgn);		/* subtract updateRgn from clipRgn */
		SetClip(tempRgn);
		TEActivate(doc->docTE);
		gTheInput = doc->docTE;
		SetClip(clipRgn);						/* restore the full-blown clipRgn */
		DisposeRgn(tempRgn);
		DisposeRgn(clipRgn);
		InvalRect(&growRect);	/* we cannot avoid grow box flickering */
/* the controls must be redrawn on activation: */
		if ((*doc->docVScroll)->contrlVis == 0) {
			ShowControl(doc->docVScroll);
			ValidRect(&(*doc->docVScroll)->contrlRect);
			}
		if ((*doc->docHScroll)->contrlVis == 0) {
			ShowControl(doc->docHScroll);
			ValidRect(&(*doc->docHScroll)->contrlRect);
			}
		}
	else {		
		TEDeactivate(doc->docTE);
		gTheInput = nil;
/* the controls must be redrawn on deactivation: */
		HideControl(doc->docVScroll);
		HideControl(doc->docHScroll);
		InvalRect(&growRect);
		}
	}
} /*DoActivate*/


/*	This is called when a mouseDown occurs in the content of a window. */

void DoContentClick(WindowPtr window, EventRecord *event)
{
Rect	teRect;
Point	mouse;
ControlHandle control;
TEHandle	theTE;
DocumentPeek doc;
long	tempc;
TEPtr	tPtr;
short	part, value, lines;

if (window == gWPtr_IO) {
	mouse = event->where;				/* get the click position */
	GlobalToLocal(&mouse);
	doc = (DocumentPeek)window;
/* see if we are in the viewRect. if so, we won�t check the controls */
	GetTERect(window, &teRect);
	theTE = doc->docTE;
	if (PtInRect(mouse, &teRect)) {
		/* see if we need to extend the selection */
		TEClick(mouse, (event->modifiers & shiftKey) != 0, theTE);
		AdjustScrollValues(window);
		}
	else {
		part = FindControl(mouse, window, &control);
		switch ( part ) {
			case 0:		/* do nothing for viewRect case */
				break;
			case kInIndicatorControlPart:
				value = GetControlValue(control);
				part = TrackControl(control, mouse, nil);
				if ( part ) {
					if ( control == doc->docVScroll ) {
						lines = LinesInTE(theTE);
						tPtr = *theTE;
						tempc = ((long)lines * tPtr->lineHeight) -
								(tPtr->viewRect.bottom - tPtr->viewRect.top);
						TEPinScroll(0, tPtr->viewRect.top - tPtr->destRect.top -
							((tempc * GetControlValue(control)) / GetControlMaximum(control)), theTE);
						}
					else
						TEPinScroll(value - GetControlValue(control), 0, theTE);
					}
				break;
			default:		/* clicked in an arrow, so track & scroll */
				{
				ControlActionUPP HVActionProcUPP = NewControlActionProc(control == doc->docVScroll ? (ProcPtr)VActionProc : (ProcPtr)HActionProc);

				(void)TrackControl(control, mouse, HVActionProcUPP);
				if (HVActionProcUPP)
					DisposeRoutineDescriptor(HVActionProcUPP);
				}
				break;
			}
		}
	}
} /*DoContentClick*/


/* This is called for any keyDown or autoKey events, except when the
 Command key is held down. It looks at the frontmost window to decide what
 to do with the key typed. */

void DoKeyDown(WindowPtr window, unsigned char key, Boolean isUserInput)
{
if (window == gWPtr_IO) {
	TEKey(key, ((DocumentPeek) window)->docTE);
	AdjustScrollbars(window, false);
	if (isUserInput) {
		gMMemory[kSIZE_RAM - 4] = 0;
		gMMemory[kSIZE_RAM - 3] = key;
		}
	}
} /*DoKeyDown*/

/* Determines how much to change the value of the vertical scrollbar by and how
	much to scroll the TE record. */

pascal void VActionProc(ControlHandle control, short part)
{
register WindowPtr	window;
register TEPtr		te;
register short		amount;

if ( part ) {			/* if it was actually in the control */
	window = (*control)->contrlOwner;
	te = *((DocumentPeek) window)->docTE;
	switch ( part ) {
		case kInUpButtonControlPart:
			amount = te->lineHeight;
			break;
		case kInDownButtonControlPart:
			amount = - te->lineHeight;
			break;
		case kInPageUpControlPart:
			amount = te->viewRect.bottom - te->viewRect.top - te->lineHeight;
			break;
		case kInPageDownControlPart:
			amount = te->viewRect.top - te->viewRect.bottom + te->lineHeight;
			break;
		}
	TEPinScroll(0, amount, ((DocumentPeek) window)->docTE);
	AdjustVert(control, ((DocumentPeek) window)->docTE);
	}
} /* VActionProc */


/* Determines how much to change the value of the horizontal scrollbar by and how
much to scroll the TE record. */

pascal void HActionProc(ControlHandle control, short part)
{
register WindowPtr	window;
register TEPtr		te;
register short		amount;

if ( part ) {
	window = (*control)->contrlOwner;
	te = *((DocumentPeek) window)->docTE;
	switch ( part ) {
		case kInUpButtonControlPart:
			amount = kButtonScroll;
			break;
		case kInDownButtonControlPart:
			amount = - kButtonScroll;
			break;
		case kInPageUpControlPart:
			amount = te->viewRect.right - te->viewRect.left - kScrollbarAdjust;
			break;
		case kInPageDownControlPart:
			amount = te->viewRect.left - te->viewRect.right + kScrollbarAdjust;
			break;
		}
	TEPinScroll(amount, 0, ((DocumentPeek) window)->docTE);
	AdjustHorz(control, ((DocumentPeek) window)->docTE);
	}
} /* VActionProc */


/* Draw the contents of an application window. */

void DrawWindow(WindowPtr window)
{
EraseRect(&window->portRect);
TEUpdate(&(*((DocumentPeek)window)->docTE)->viewRect, ((DocumentPeek) window)->docTE);
DrawGrowIcon(window);
UpdateControls(window, window->visRgn);
} /*DrawWindow*/


/* Return a rectangle that is inset from the portRect by the size of
	the scrollbars and a little extra margin. */

void GetTERect(WindowPtr window, RectPtr teRect)
{
*teRect = window->portRect;
teRect->left += kTextMargin;
teRect->bottom -= kScrollbarAdjust;		/* and for the scrollbars */
teRect->right -= kScrollbarAdjust;
} /*GetTERect*/

/* Scroll the TERec around to match up to the potentially updated scrollbar
	values. This is really useful when the window has been resized such that the
	scrollbars became inactive but the TERec was already scrolled. */

void AdjustTE(WindowPtr window)
{
register TEPtr		te;

te = *((DocumentPeek)window)->docTE;
TEPinScroll((te->viewRect.left - te->destRect.left) -
		GetControlValue(((DocumentPeek)window)->docHScroll),
		(te->viewRect.top - te->destRect.top) -
			(GetControlValue(((DocumentPeek)window)->docVScroll) *
			te->lineHeight),
		((DocumentPeek)window)->docTE);
} /*AdjustTE*/


/* Calculate the new control maximum value and current value, whether it is the
horizontal or vertical scrollbar. The vertical max is calculated by comparing the
number of lines to the vertical size of the viewRect. The horizontal max is
calculated by comparing the maximum document width to the width of the viewRect.
The current values are set by comparing the offset between the view and
destination rects. */

void AdjustHorz(ControlHandle control, TEHandle docTE)
{
register TEPtr	te;
register short	max, min;

te = *docTE;
max = (qd.screenBits.bounds.right - qd.screenBits.bounds.left) -
			(te->viewRect.right - te->viewRect.left);
if ( max < (min = GetControlMinimum(control)) )
	max = min;
SetControlMaximum(control, max);
te = *docTE;	/* SetControlMaximum can move memory */
SetControlValue(control, te->viewRect.left - te->destRect.left);
} /*AdjustHorz*/

void AdjustVert(ControlHandle control, TEHandle docTE)
{
register TEPtr	te;
register short	max, lines, min, divisor;

lines = LinesInTE(docTE);
te = *docTE;
max = lines - ((te->viewRect.bottom - te->viewRect.top) / te->lineHeight);
if ( max < (min = GetControlMinimum(control)) ) max = min;
SetControlMaximum(control, max);
te = *docTE;	/* SetControlMaximum can move memory */
divisor = ((long)lines * te->lineHeight) - (te->viewRect.bottom - te->viewRect.top) - (te->lineHeight >> 1);
SetControlValue(control, divisor ?
	((long)max * (te->viewRect.top - te->destRect.top)) / divisor
	: 0);
} /*AdjustVert*/

/* Simply call the adjust routines for the vertical and horizontal scrollbars */

void AdjustScrollValues(WindowPtr window)
{
register DocumentPeek doc;

doc = (DocumentPeek)window;
AdjustHorz(doc->docHScroll, doc->docTE);
AdjustVert(doc->docVScroll, doc->docTE);
} /*AdjustScrollValues*/


/* Re-calculate the position and size of the viewRect and the scrollbars */

void AdjustScrollSizes(WindowPtr window)
{
Rect		teRect;
register DocumentPeek doc;

doc = (DocumentPeek) window;
GetTERect(window, &teRect);			/* start with TERect */
(*doc->docTE)->viewRect = teRect;
SetupVertScrollBar(window, doc->docVScroll);
SetupHorzScrollBar(window, doc->docHScroll);
} /*AdjustScrollSizes*/

void AdjustScrollbars(WindowPtr window, Boolean needsResize)
{
if ( needsResize ) {		/* move & size as needed */
	HideControl(((DocumentPeek) window)->docVScroll);
	HideControl(((DocumentPeek) window)->docHScroll);
	AdjustScrollSizes(window);
	}
AdjustScrollValues(window);	/* fool with max and current value */
if ( needsResize ) {						/* move & size as needed */
	ShowControl(((DocumentPeek) window)->docVScroll);
	ShowControl(((DocumentPeek) window)->docHScroll);
	ValidRect(&(*((DocumentPeek) window)->docVScroll)->contrlRect);
	ValidRect(&(*((DocumentPeek) window)->docHScroll)->contrlRect);
	}
} /* AdjustScrollbars */

/* LinesInTE: returns the number of lines in a TextEdit field;
since it calls TEGetText, it _can_ move memory */

static short LinesInTE(TEHandle theTE)
{
register Handle	textHand;
register short	lines;

lines = (*theTE)->nLines;
textHand = (Handle)TEGetText(theTE);
/* since nLines isn�t right if the last character is a return, check for that case */
if ( *(*textHand + InlineGetHandleSize(textHand) - 1) == kCrChar )
	lines++;
return(lines);
}

/* procedure called when closing the IO window */

void CloseIO(WindowPtr w)
{
DoCloseWindow(w, kMItem_IO);
}

void RecalcIO(FabWindowPtr w, RgnBalloonCursPtr theObj)
{
Rect	tempRect;

GetTERect((WindowPtr)w, &tempRect);
RectRgn(theObj->zoneLocal, &tempRect);
}

