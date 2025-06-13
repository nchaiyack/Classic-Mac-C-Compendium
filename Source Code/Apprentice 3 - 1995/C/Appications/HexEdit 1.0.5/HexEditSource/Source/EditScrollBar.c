/*********************************************************************
 * EditScrollBar.c
 *
 * HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *********************************************************************/
#include "HexEdit.h"

void HEditScrollToPosition(EditWindowPtr dWin, long newPos);
void AdjustScrollBars(WindowPtr theWin, short resizeFlag);

void SetupScrollBars(EditWindowPtr dWin)
{
	Rect	sRect,r;
	// !! Set up vertical scroll bar
	//
	r = ((WindowPtr) dWin)->portRect;
	sRect.left = r.right - (SBarSize - 1);
	sRect.top = r.top - 1;
	sRect.right = r.right + 1;
	sRect.bottom = r.bottom  - GrowIconSize;
 	dWin->vScrollBar = NewControl((WindowPtr) dWin,&sRect,"\p",true,
 								0,0,sRect.bottom - sRect.top,scrollBarProc,1L);
	AdjustScrollBars((WindowPtr) dWin, 1);
}

//
// Adjust scroll bars when they need to be redrawn for some reason.
//
// resizeFlag is an optimization to avoid extra work when you aren't
// resizing.
//

void AdjustScrollBars(WindowPtr theWin, short resizeFlag)
{
	short			w,h;
	GrafPtr			savePort;
	EditWindowPtr	dWin = (EditWindowPtr) theWin;
	long			limit;

	GetPort(&savePort);

	if (resizeFlag) {
		// Adjust Lines Per Page
		dWin->linesPerPage = ((theWin->portRect.bottom - SBarSize) - HeaderHeight - TopMargin - BotMargin)/LineHeight;

		// Move sliders to new position
		//
		MoveControl(dWin->vScrollBar, theWin->portRect.right - (SBarSize - 1),
											theWin->portRect.top - 1);	
		// Change their sizes to fit new window dimensions
		//
		w = 16;
		h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
		SizeControl(dWin->vScrollBar,w,h);

		// Reset their maximum values
		//
		SetCtlMax(dWin->vScrollBar,h);
	}

	// Reposition painting if you have resized or scrolled past the legal
	// bounds  Note: this call will usually be followed by an update
	//
	limit = ((dWin->fileSize+15) & 0xFFFFFFF0) - (dWin->linesPerPage << 4);
	if (dWin->editOffset > limit)
		dWin->editOffset = limit;
	if (dWin->editOffset < 0)
		dWin->editOffset = 0;

	// Set the value of the sliders accordingly
	//
	h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
	if (limit > 0) {
		h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
		SetCtlMax(dWin->vScrollBar,h);
		if (dWin->editOffset < 64000L)
			SetCtlValue(dWin->vScrollBar,(short) ((dWin->editOffset*h)/limit));
		else
			SetCtlValue(dWin->vScrollBar,(short) (dWin->editOffset / (limit/h)));
	}
	else {
		SetCtlMax(dWin->vScrollBar,0);
		SetCtlValue(dWin->vScrollBar,0);
	}
		
	SetPort(savePort);
}

//
// Callback routine to handle arrows and page up, page down
//
EditWindowPtr	gDWin;

pascal void MyScrollAction(ControlHandle theControl, short thePart)
{
	long		curPos,newPos;
	short		pageWidth;
	Rect		myRect;
	WindowPtr	gp = (WindowPtr) gDWin;

	curPos = gDWin->editOffset;
	newPos = curPos;

	myRect = ((WindowPtr) gDWin)->portRect;
	myRect.right -= SBarSize-1;
	myRect.bottom -= SBarSize-1;

	pageWidth = (gDWin->linesPerPage-1)*16;

	switch (thePart) {
	  case inUpButton:		newPos = curPos - 16;			break;
	  case inDownButton:	newPos = curPos + 16;			break;
  	  case inPageUp:		newPos = curPos - pageWidth;	break;
	  case inPageDown:		newPos = curPos + pageWidth;	break;
	}

	if (newPos != curPos) {
		HEditScrollToPosition(gDWin, newPos);
	}
}

// Intercept Handler for scroll bars
// Returns true if user clicked on scroll bar
//

Boolean HandleControlClick(WindowPtr theWin, Point where)
{
	short 			ctlPart;
	ControlHandle	whichControl;
	short			vPos;
	EditWindowPtr	dWin = (EditWindowPtr) theWin;
	ProcPtr			trackActionProc;
	
	// Check if user clicked on scrollbar
	//
	if ((ctlPart = FindControl(where,theWin,&whichControl)) == 0)
		return false;

	// Identify window for callback procedure
	//
	gDWin = dWin;

	// Use default behavior for thumb
	// Program will crash if you don't!!
	//
	if (ctlPart == inThumb)
		trackActionProc = 0L;
	else
		trackActionProc = MyScrollAction;

	// Perform scrollbar tracking
	//
	if ((ctlPart = TrackControl(whichControl, where, trackActionProc)) == 0)
		return false;

	if (ctlPart == inThumb) {
		long	newPos,h,limit;
		vPos = GetCtlValue(dWin->vScrollBar);
		h = theWin->portRect.bottom - theWin->portRect.top - (GrowIconSize - 1);
		limit = ((dWin->fileSize+15) & 0xFFFFFFF0) - (dWin->linesPerPage << 4);
		if (vPos == h)
			newPos = ((dWin->fileSize+15) & 0xFFFFFFF0) - (dWin->linesPerPage << 4);
		else if (limit < 64000L)		// JAB 12/10 Prevent Overflow in Calcuation
			newPos = (vPos*limit)/h;
		else
			newPos = vPos*(limit/h);
		newPos -= newPos & 0x0F;
		HEditScrollToPosition(dWin, newPos);
	}

	return true;
}

// Scroll to an explicit position
//
void HEditScrollToPosition(EditWindowPtr dWin, long newPos)
{
	long	limit;

	SetPort((WindowPtr) dWin);

	// Constrain scrolling position to legal limits
	//
	limit = ((dWin->fileSize+15) & 0xFFFFFFF0) - (dWin->linesPerPage << 4);
	if (newPos > limit)
		newPos = limit;
	if (newPos < 0)
		newPos = 0;
	dWin->editOffset = newPos;
	
	// Adjust Scrollbars
	AdjustScrollBars((WindowPtr) dWin, false);

	// 12/10/93 - Optimize Drawing
	SetCurrentChunk(dWin, dWin->editOffset);

	DrawPage(dWin);
	UpdateOnscreen((WindowPtr) dWin);
}

void AutoScroll(EditWindowPtr dWin, Point pos)
{
	short	offset;
	if (pos.v < HeaderHeight+TopMargin)
		offset = -16;
	else if (pos.v >= HeaderHeight+TopMargin+dWin->linesPerPage*LineHeight)
		offset = 16;
	else
		return;
	HEditScrollToPosition(dWin, dWin->editOffset+offset);
}