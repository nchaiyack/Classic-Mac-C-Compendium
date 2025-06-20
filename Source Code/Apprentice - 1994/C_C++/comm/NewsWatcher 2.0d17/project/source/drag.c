/*----------------------------------------------------------------------------

	drag.c

	This module handles dragging groups within and between group windows.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <stdio.h>

#include "glob.h"
#include "drag.h"
#include "subscribe.h"



static Boolean gFirstCall;		/* true if first click loop call after mouse down */



/*----------------------------------------------------------------------------
	MoveGroups
	
	This function handles moving groups in a user group list after the user
	drags cells around in the list and releases the mouse.
	
	Entry:	wind = pointer to user group list window.
			destRow = row number of cell following destination.
	
	All selected cells in the window are moved to the destination location.
----------------------------------------------------------------------------*/

static void MoveGroups (WindowPtr wind, short destRow)
{
	TWindow **info;
	ListHandle theList;
	short count;
	Cell oldCell, newCell;
	short dataLen, cellData, firstRow, prevRow;
	Boolean contiguous = true;

	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	
	/* Count the number of cells to be moved. Also check to see if the
	   cells being moved are contiguous. */
	
	count = 0;
	SetPt(&oldCell, 0, 0);
	while (LGetSelect(true, &oldCell, theList)) {
		if (count == 0) firstRow = oldCell.v;
		if (contiguous && count > 0 && oldCell.v != prevRow + 1) contiguous = false;
		prevRow = oldCell.v;
		count++;
		oldCell.v++;
	}
	
	/* If the cells being moved are contiguous, check to see if there is no
	   change. If not, just return. */
	   
	if (contiguous && firstRow <= destRow && destRow <= firstRow + count) return;
	
	/* Insert the new cells at the destination location. */

	LDoDraw(false, theList);
	LAddRow(count, destRow, theList);
	
	/* Copy the selected cells to their new location. */
	
	SetPt(&newCell, 0, destRow);
	SetPt(&oldCell, 0, 0);
	while (LGetSelect(true, &oldCell, theList)) {
		dataLen = 2;
		LGetCell(&cellData, &dataLen, oldCell, theList);
		LSetCell(&cellData, dataLen, newCell, theList);
		oldCell.v++;
		newCell.v++;
	}
	
	/* Delete the old cells. */

	SetPt(&oldCell, 0, 0);
	while (LGetSelect(true, &oldCell, theList)) {
		LDelRow(1, oldCell.v, theList);
		if (oldCell.v < destRow) destRow--;
	}
	
	/* Select the new cells. */
	
	SetPt(&newCell, 0, destRow);
	while (count--) {
		LSetSelect(true, newCell, theList);
		newCell.v++;
	}
	
	/* Redraw the window. */

	InvalRect(&wind->portRect);
	LDoDraw(true, theList);
	
	(**info).changed = true;
}



/*----------------------------------------------------------------------------
	DrawDragLine
	
	Draw or erase a horizontal dividing line in a list.
	
	Entry:	list = handle to the list.
			lineV = vertical coordinate in list of line to draw or erase.
----------------------------------------------------------------------------*/

static void DrawDragLine (ListHandle list, short lineV)
{
	GrafPtr savePort, listPort;
	PenState savePen;
	Rect clipRect;
	
	listPort = (**list).port;

	GetPort(&savePort);
	GetPenState(&savePen);

	SetPort(listPort);
	clipRect = (**list).rView;
	ClipRect(&clipRect);
	PenMode(patXor);
	PenPat(qd.black);
	PenSize(3, 3);
	MoveTo((**list).rView.left, lineV);
	LineTo((**list).rView.right - 3, lineV);

	SetPenState(&savePen);
	SetPort(savePort);
}



/*----------------------------------------------------------------------------
	GetDestRow
	
	Gets the destination row in a list given a mouse position. Also determines
	whether or not the list needs to be scrolled.
	
	Entry:	list = handle to list record.
			mousePt = mouse position in local coordinates.
	
	Exit:	*destRow = row number of cell following destination, or 
				-10 if mouse to left or right of list.
			*lineV = vertical coordinate of dividing line at destination
				in local coordinates, or -10 if mouse to left or right of list.
			*scrollDelta = delta in cells that list must be scrolled
				(-1, 0, or +1).
----------------------------------------------------------------------------*/

static void GetDestRow (ListHandle list, Point mousePt, short *destRow,
	short *lineV, short *scrollDelta)
{
	Rect rView, visible, dataBounds;
	Point cellSize;
	
	rView = (**list).rView;
	visible = (**list).visible;
	dataBounds = (**list).dataBounds;
	cellSize = (**list).cellSize;

	/* Check to see if the mouse is to the left or right of the list. */
	
	if (mousePt.h < rView.left || mousePt.h > rView.right) {
		*destRow = -10;
		*lineV = -10;
		*scrollDelta = 0;
		return;
	}

	/* See if scrolling is necessary. */
	
	if (mousePt.v < rView.top && visible.top > dataBounds.top) {
		*scrollDelta = -1;
	} else if (mousePt.v > rView.bottom && visible.bottom < dataBounds.bottom) {
		*scrollDelta = +1;
	} else {
		*scrollDelta = 0;
	}

	/* Compute the destination row. */
	
	*destRow = (mousePt.v - rView.top + (cellSize.v >> 1)) / cellSize.v + visible.top;
	if (*destRow < visible.top) *destRow = visible.top;
	if (*destRow > visible.bottom) *destRow = visible.bottom;
	if (*destRow > dataBounds.bottom) *destRow = dataBounds.bottom;
	
	/* Compute the v coordinate of the dividing line. */
	
	*lineV = cellSize.v * (*destRow - visible.top) + rView.top - 1;

	/* Adjust destination row for scrolling. */
	
	*destRow += *scrollDelta;
}



/*----------------------------------------------------------------------------
	DragMoveGroups
	
	This function handles dragging of groups within a user group list.
	It tracks a bold dividing line at the location where the groups will
	be moved if the mouse button is released.  It also handles autoscrolling.
	
	Entry:	wind = pointer to user group list window.
----------------------------------------------------------------------------*/

static void DragMoveGroups (WindowPtr wind)
{
	TWindow **info;
	ListHandle theList;
	Boolean lineDrawn = false;
	Point mousePt;
	short destRow, lineV, scrollDelta, prevLineV;

	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;

	while (StillDown()) {
	
		GetMouse(&mousePt);
		GetDestRow(theList, mousePt, &destRow, &lineV, &scrollDelta);
		
		/* Erase the old dividing line if necessary. */
		
		if (lineDrawn && (lineV != prevLineV || scrollDelta != 0)) {
			DrawDragLine(theList, prevLineV);
			lineDrawn = false;
		}
		
		/* Autoscroll if necessary. */
		
		if (scrollDelta != 0) LScroll(0, scrollDelta, theList);
		
		/* Draw the new dividing line if necessary. */
		
		if (!lineDrawn && destRow >= 0) {
			DrawDragLine(theList, lineV);
			prevLineV = lineV;
			lineDrawn = true;
		}
		
	}
	
	if (lineDrawn) {
		DrawDragLine(theList, lineV);
		MoveGroups(wind, destRow);
	}
}



/*----------------------------------------------------------------------------
	MouseIsOverUserGroupList
	
	Checks to see if the mouse is over a user group list.
	
	Exit:	function result = true if mouse is over a user group list.
			*thePt = current mouse position in global coords.
			if function result = true:
				*theWind = pointer to user group list window.
				*theInfo = handle to group list window info.
				*theList = handle to group list record.
			if function result = false:
				*theWind, *theInfo, *theList unchanged.
----------------------------------------------------------------------------*/

static Boolean MouseIsOverUserGroupList (Point *thePt, WindowPtr *theWind,
	TWindow ***theInfo, ListHandle *theList)
{
	WindowPtr wind;
	TWindow **info;
	ListHandle list;
	Rect rView;
	GrafPtr savePort;
	Point localPt;
	
	GetPort(&savePort);
	GetMouse(thePt);
	LocalToGlobal(thePt);
	FindWindow(*thePt, &wind);
	if (wind != nil) {
		info = (TWindow**)GetWRefCon(wind);
		if ((**info).kind == kUserGroup) {
			list = (**info).theList;
			rView = (**list).rView;
			localPt = *thePt;
			SetPort(wind);
			GlobalToLocal(&localPt);
			if (PtInRect(localPt, &rView)) {
				*theWind = wind;
				*theInfo = info;
				*theList = list;
				SetPort(savePort);
				return true;
			}
		}
	}
	SetPort(savePort);
	return false;
}



/*----------------------------------------------------------------------------
	DragSubscribeGroups
	
	This function handles dragging of groups from the full group list window
	or new groups list window to a user group list window. 
	
	There are two stages to the drag:
	
	First, a gray region outline of the cells is dragged out of the starting
	window. The user drags this gray region around the screen until the mouse 
	is over a user group list window. At this point the gray region disappears 
	and the first stage ends.
	
	In the second stage, a bold dividing line is tracked at the location where
	the groups will be copied if the mouse button is released, with autoscrolling.
	If the user moves the mouse to some other user group list window, that
	new window becomes the target window.
	
	Entry:	startWind = pointer to starting group list window.
			startPt = initial click location in starting window, in 
				local coordinates.
----------------------------------------------------------------------------*/

static void DragSubscribeGroups (WindowPtr startWind, Point startPt)
{
	WindowPtr curWind, prevWind, wind;
	TWindow **startInfo, **curInfo, **info;
	ListHandle startList, prevList = nil, curList;
	RgnHandle grayRgn;
	Cell curCell;
	Rect cellRect;
	Point tempPt, prevPt, curPt;
	GrafPtr savePort;
	GrafPort fullPort;
	Boolean rgnDrawn = true, lineDrawn = false;
	short lineV, prevLineV, destRow, scrollDelta;

	startInfo = (TWindow**)GetWRefCon(startWind);
	startList = (**startInfo).theList;
	
	/* Start stage 1 - drag a gray region around. */
	
	/* Construct the gray region from the selected cells in the starting window. */

	grayRgn = NewRgn();
	OpenRgn();
	SetPt(&curCell,0,0);
	while (LGetSelect(true, &curCell, startList)) {
		LRect(&cellRect, curCell, startList);
		FrameRect(&cellRect);
		curCell.v++;
	}
	CloseRgn(grayRgn);
	
	/* Offset the gray region to the current mouse location. */
	
	GetMouse(&curPt);
	OffsetRgn(grayRgn, curPt.h - startPt.h, curPt.v - startPt.v);
	
	/* Convert the gray region and the current mouse loc to global coords. */
	
	SetPt(&tempPt, 0, 0);
	LocalToGlobal(&tempPt);
	OffsetRgn(grayRgn, tempPt.h, tempPt.v);
	LocalToGlobal(&curPt);
	
	/* Create a full screen GrafPort. */

	GetPort(&savePort);
	OpenPort(&fullPort);
	CopyRgn(GetGrayRgn(), fullPort.visRgn);
	fullPort.portRect = (**GetGrayRgn()).rgnBBox;
	PenPat(qd.gray);
	PenMode(patXor);

	/* Draw the initial gray region. */
	
	FrameRgn(grayRgn);
	prevPt = curPt;
	
	/* Drag the gray region around until the mouse is over a user 
	   group list window or the mouse is released. */
	
	while (StillDown()) {
	
		/* Break out of the loop if the mouse is over a user group list. */
	
		if (MouseIsOverUserGroupList(&curPt, &curWind, &curInfo, &curList)) break;
		
		/* Erase the old gray region if necessary. */
		
		if (rgnDrawn && !EqualPt(curPt, prevPt)) {
			FrameRgn(grayRgn);
			rgnDrawn = false;
		}
		
		/* Draw the new gray region if necessary. */
		
		if (!rgnDrawn) {
			OffsetRgn(grayRgn, curPt.h - prevPt.h, curPt.v - prevPt.v);
			FrameRgn(grayRgn);
			prevPt = curPt;
			rgnDrawn = true;
		}
				
	}

	if (rgnDrawn) FrameRgn(grayRgn);
	DisposeRgn(grayRgn);
	ClosePort(&fullPort);
	SetPort(savePort);
	if (!StillDown()) return;
	
	/* Start stage 2 - track the destination dividing line, switching target
	   user group list windows as necessary. */
	
	SetPort(curWind);
	
	while (StillDown()) {
	
		/* If the mouse is in some other user group list window, make that window
		   the target window. */
	
		prevList = curList;
		if (MouseIsOverUserGroupList(&curPt, &curWind, &curInfo, &curList)) SetPort(curWind);
		GlobalToLocal(&curPt);

		/* Get the destination row. */
		
		GetDestRow(curList, curPt, &destRow, &lineV, &scrollDelta);
		
		/* Erase the old dividing line if necessary. */
		
		if (lineDrawn && (curList != prevList || scrollDelta != 0 || lineV != prevLineV)) {
			DrawDragLine(prevList, prevLineV);
			lineDrawn = false;
		}
		
		/* Autoscroll if necessary. */
		
		if (scrollDelta != 0) LScroll(0, scrollDelta, curList);
		
		/* Draw the new dividing line if necessary. */
		
		if (!lineDrawn && destRow >= 0) {
			DrawDragLine(curList, lineV);
			prevLineV = lineV;
			lineDrawn = true;
		}
		
	}

	if (lineDrawn) {
		DrawDragLine(prevList, prevLineV);
		SubscribeSelected(startWind, curWind, destRow);
	}
	
	SetPort(savePort);
}



/*----------------------------------------------------------------------------
	GroupListClickLoop
	
	The click loop routine for group list windows. It handles dragging 
	groups between and within group list windows.
	
	Groups can be dragged from the full group list window or new groups
	list window to a user group list window to subscribe to the groups.

	Groups can be dragged within a user group list window to reorder
	the groups.
	
	Exit:	function result = true if mouse button still down, false
				if mouse button released.
----------------------------------------------------------------------------*/

pascal Boolean GroupListClickLoop (void)
{	
	static Point firstPt;
	WindowPtr wind;
	TWindow **info;
	ListHandle theList;
	Point thePt;
	Rect rect, cellRect;
	Cell cellClicked;

	if (gFirstCall) {
		gFirstCall = false;
		GetMouse(&firstPt);
		return true;
	}

	wind = FrontWindow();
	info = (TWindow**)GetWRefCon(wind);
	theList = (**info).theList;
	
	SetRect(&rect, firstPt.h - 3, firstPt.v - 3, firstPt.h + 3, firstPt.v + 3);
	cellClicked = LLastClick(theList);
	LRect(&cellRect, cellClicked, theList);
	SectRect(&rect, &cellRect, &rect);
	GetMouse(&thePt);
	if (PtInRect(thePt, &rect)) return true;
	
	if ((**info).kind == kUserGroup) {
		DragMoveGroups(wind);
	} else {
		DragSubscribeGroups(wind, firstPt);
	}
	return false;
}



/*----------------------------------------------------------------------------
	BeginGroupListClick
	
	This function must be called just before calling LClick for a group list
	window.
	
	Exit:	gFirstCall = true, to notify our click loop function that this
				is the first call after a mouse down.
----------------------------------------------------------------------------*/

void BeginGroupListClick (void)
{
	gFirstCall = true;
}
