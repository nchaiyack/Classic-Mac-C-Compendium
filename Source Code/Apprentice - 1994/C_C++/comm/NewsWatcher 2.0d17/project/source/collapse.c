/*----------------------------------------------------------------------------

	collapse.c

	This module handles collapsing and expanding threads.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "collapse.h"
#include "draw.h"
#include "menus.h"
#include "resize.h"
#include "util.h"
#include "wind.h"
#include "scroll.h"


/*	ExpandCollapseThread expands or collapses a single thread.

	Entry:	wind = pointer to window record.
			theCell = any cell in the thread to be expanded or collapsed.
			
	Exit:	function result = true if thead collapsed, false if expanded.
*/

static Boolean ExpandCollapseThread (WindowPtr wind, Cell theCell)
{
	TWindow **info;
	ListHandle theList;
	TSubject **subjectArray, theSubject;
	Cell tmpCell, newCell;
	long finalTicks;
	short i, nextInThread, cellDataLen, cellData;
	Rect inval;
	
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	subjectArray = (**info).subjectArray;
	cellDataLen = 2;
	LGetCell(&cellData, &cellDataLen, theCell, theList);
	theSubject = (*subjectArray)[cellData];
	if (theSubject.threadOrdinal != 1) {
		theCell.v -= theSubject.threadOrdinal-1;
		LGetCell(&cellData, &cellDataLen, theCell, theList);
		theSubject = (*subjectArray)[cellData];
	}
	theSubject.collapsed = !theSubject.collapsed;
	nextInThread = cellData;
	for (i = 0; i < theSubject.threadLength; i++) {
		(*subjectArray)[nextInThread].collapsed = theSubject.collapsed;
		nextInThread = (*subjectArray)[nextInThread].nextInThread;
	}
	(*subjectArray)[cellData].onlyRedrawTriangle = true;
	(*subjectArray)[cellData].onlyRedrawCheck = false;
	(*subjectArray)[cellData].drawTriangleFilled = true;
	LDraw(theCell, theList);
	(*subjectArray)[cellData].onlyRedrawTriangle = false;
	(*subjectArray)[cellData].onlyRedrawCheck = true;
	LDraw(theCell, theList);
	(*subjectArray)[cellData].onlyRedrawTriangle = true;
	(*subjectArray)[cellData].onlyRedrawCheck = false;
	
	if (theSubject.collapsed) {
		LDelRow(theSubject.threadLength-1, theCell.v+1, theList);
		SetPt(&tmpCell, 0, 0);
		if (!LGetSelect(true, &tmpCell, theList)) 
			LSetSelect(true, theCell, theList);
	} else {
		LDoDraw(false, theList);
		LAddRow(theSubject.threadLength-1, theCell.v+1, theList);
		newCell = theCell;
		nextInThread = theSubject.nextInThread;
		for (i = 1; i < theSubject.threadLength; i++) {
			newCell.v++;
			LSetCell(&nextInThread, 2, newCell, theList);
			nextInThread = (*subjectArray)[nextInThread].nextInThread;
		}
		LDoDraw(true, theList);
		inval = wind->portRect;
		inval.top = (theCell.v - (**theList).visible.top + 1) *
			(**theList).cellSize.v;
		inval.right -= 15;
		inval.bottom -= 15;
		InvalRect(&inval);
		HandleUpdate(wind);
	}
	
	Delay(8, &finalTicks);
	(*subjectArray)[cellData].drawTriangleFilled = false;
	LDraw(theCell, theList);
	(*subjectArray)[cellData].onlyRedrawTriangle = false;
	return theSubject.collapsed;
}


/*	RezoomWindow rezooms the window if a thread has been expanded and
	the "Zoom windows" preference is turned on. */
	
static void RezoomWindow (WindowPtr wind, Boolean expanded)
{
	TWindow **info;
	ListHandle theList;
	ControlHandle scrollBar;
	
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	scrollBar = (**theList).vScroll;
	if (expanded && gPrefs.zoomWindows && GetCtlMax(scrollBar) > 0) {
		DoZoom(wind, inZoomOut);
	} else {
		SetWindowNeedsZooming(wind);
	}	
}


/*	DoExpandCollapseSelectedThread expands or collapses just the currently 
	selected thread in a subject window.
	
	Entry:	wind = pointer to subject window. 
*/
	
void DoExpandCollapseSelectedThread (WindowPtr wind)
{
	TWindow **info;
	ListHandle theList;
	TSubject **subjectArray;
	Cell theCell, tmpCell;
	short cellDataLen, cellData;
	TSubject theSubject;
	Boolean collapsed;

	SetPort(wind);
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	subjectArray = (**info).subjectArray;
	SetPt(&theCell, 0, 0);
	if (!LGetSelect(true, &theCell, theList)) return;
	tmpCell = theCell;
	tmpCell.v++;
	if (LGetSelect(true, &tmpCell, theList)) return;
	cellDataLen = 2;
	LGetCell(&cellData, &cellDataLen, theCell, theList);
	theSubject = (*subjectArray)[cellData];
	if (theSubject.threadLength <= 1) return;
	collapsed = ExpandCollapseThread(wind, theCell);
	RezoomWindow(wind, !collapsed);
}


/*	TriangleClick handles mouse-downs on the expand/collapse triangle
	thread controls in subject windows.
	
	Entry:	wind = pointer to subject window.
			where = location of mouse down in local coordinates.
*/

Boolean TriangleClick (WindowPtr wind, Point where)
{
	TWindow **info;
	ListHandle theList;
	TSubject **subjectArray, theSubject;
	FontInfo fontInfo;
	Cell theCell;
	short cellData, cellDataLen;
	Rect hitRect;
	Boolean inHitRect, newInHitRect;
	short visTop, cellHeight;
	Boolean collapsed;

	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	visTop = (**theList).visible.top;
	cellHeight = (**theList).cellSize.v;
	subjectArray = (**info).subjectArray;
	GetFontInfo(&fontInfo);
	SetRect(&hitRect, 0, 0, (**theList).indent.h + fontInfo.ascent, 0);
	if (where.h > hitRect.right) return false;
	theCell.h = 0;
	theCell.v = where.v/cellHeight + visTop;
	if (theCell.v >= (**theList).dataBounds.bottom) return false;
	cellDataLen = 2;
	LGetCell(&cellData, &cellDataLen, theCell, theList);
	theSubject = (*subjectArray)[cellData];
	if (theSubject.threadLength == 1 || theSubject.threadOrdinal > 1) 
		return false;
	(*subjectArray)[cellData].drawTriangleFilled = inHitRect =  true;
	(*subjectArray)[cellData].onlyRedrawTriangle = true;
	LDraw(theCell, theList);
	hitRect.top = (theCell.v - visTop) * cellHeight;
	hitRect.bottom = hitRect.top + cellHeight;
	while (StillDown()) {
		GetMouse(&where);
		newInHitRect = PtInRect(where, &hitRect);
		if (newInHitRect != inHitRect) {
			(*subjectArray)[cellData].drawTriangleFilled = newInHitRect;
			LDraw(theCell, theList);
			inHitRect = newInHitRect;
		}
	}
	if (!inHitRect) {
		(*subjectArray)[cellData].drawTriangleFilled = false;
		(*subjectArray)[cellData].onlyRedrawTriangle = false;
		return true;
	}
	collapsed = ExpandCollapseThread(wind, theCell);
	RezoomWindow(wind, !collapsed);
	return true;
}

void ExpandCollapseKey (WindowPtr wind, char theChar)
{
	TWindow **info;
	ListHandle theList;
	TSubject **subjectArray;
	Cell theCell;
	short cellDataLen, cellData;
	TSubject theSubject;
	Boolean expanded = false;
	Boolean changed = false;

	SetPort(wind);
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	subjectArray = (**info).subjectArray;

	SetPt(&theCell, 0, 0);
	while (true) {
		if (!LGetSelect(true, &theCell, theList)) break;
		cellDataLen = 2;
		LGetCell(&cellData, &cellDataLen, theCell, theList);
		theSubject = (*subjectArray)[cellData];
		if (theSubject.threadLength > 1)
			if (theSubject.collapsed && theChar == rightArrow ||
				!theSubject.collapsed && theChar == leftArrow) 
			{ 
				ExpandCollapseThread(wind, theCell);
				changed = true;
				if (theChar == rightArrow) expanded = true;
			}
		theCell.v++;
	}
	if (changed) RezoomWindow(wind, expanded);
}




