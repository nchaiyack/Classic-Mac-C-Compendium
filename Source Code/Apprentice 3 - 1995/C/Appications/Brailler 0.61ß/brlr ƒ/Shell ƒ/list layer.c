#include "list layer.h"

#define kDoDraw			TRUE	/* always draw list after changes */
#define kNoGrow			FALSE	/* don't leave room for size box */
#define kIncludeScroll	TRUE	/* leave room for scroll bar */
#define kScrollWidth	15		/* width of scroll bar */

ListHandle MyCreateVerticalScrollingList(WindowPtr theWindow, Rect boundsRect,
	short columnsInList, short theLDEF, short cellHeight)
{
	Rect			dataBoundsRect;
	Point			cellSize;
	
	SetRect(&dataBoundsRect, 0, 0, columnsInList, 0);
	boundsRect.right=boundsRect.right-kScrollWidth;
	if (cellHeight!=0)
		SetPt(&cellSize, boundsRect.right-boundsRect.left, cellHeight);
	else
		SetPt(&cellSize, 0, 0);
	return LNew(&boundsRect, &dataBoundsRect, cellSize, theLDEF, theWindow, kDoDraw,
		kNoGrow, !kIncludeScroll, kIncludeScroll);
}

void MyDrawListBorder(ListHandle theList)
{
	Rect			theBorder;
	PenState		thePenState;
	
	theBorder=(**theList).rView;
	GetPenState(&thePenState);
	PenSize(1,1);
	InsetRect(&theBorder, -1, -1);
	FrameRect(&theBorder);
	SetPenState(&thePenState);
}

void MyDrawActiveListBorder(ListHandle theList, Boolean isActive)
{
	Rect			outlineRect;
	PenState		thePenState;
	
	outlineRect=(**theList).rView;
	if (((**theList).vScroll)!=0L)
		outlineRect.right+=kScrollWidth;
	if (((**theList).hScroll)!=0L)
		outlineRect.bottom+=kScrollWidth;
	SetPort((**theList).port);
	InsetRect(&outlineRect, -4, -4);
	GetPenState(&thePenState);
	if ((isActive) && ((**theList).lActive))
		PenPat(&qd.black);
	else
		PenPat(&qd.white);
	PenSize(2,2);
	FrameRect(&outlineRect);
	SetPenState(&thePenState);
}

void MyAddStr255ToList(ListHandle theList, Str255 theStr)
{
	short			rowNum;
	Cell			theCell;
	
	rowNum=(**theList).dataBounds.bottom;
	rowNum=LAddRow(1, rowNum, theList);
	SetPt(&theCell, 0, rowNum);
	LSetCell(&theStr[1], theStr[0], theCell, theList);
}

void MyDeleteItemFromList(ListHandle theList, short index)
{
	/* index is 0-based */
	LDelRow(1, index, theList);
}

Boolean MyHandleMouseDownInList(ListHandle theList, Point thePoint, short modifiers)
/* returns TRUE if double click */
{
	SetPort((**theList).port);
	return LClick(thePoint, modifiers, theList);
}

void MyUpdateList(ListHandle theList)
{
	WindowPtr		theWindow;
	
	theWindow=(WindowPtr)((**theList).port);
	SetPort(theWindow);
	TextFont(theWindow->txFont);
	TextSize(theWindow->txSize);
	LUpdate(theWindow->visRgn, theList);
	MyDrawListBorder(theList);
}

void MyActivateList(ListHandle theList, Boolean isActive)
{
	LActivate(isActive, theList);
}

Boolean MyGetFirstSelectedCell(ListHandle theList, Cell *theCell)
{
	SetPt(theCell, 0, 0);
	return LGetSelect(TRUE, theCell, theList);
}

void MySelectOneCell(ListHandle theList, Cell theCell)
{
	Cell			temp;
	
	LSetDrawingMode(FALSE, theList);
	while (MyGetFirstSelectedCell(theList, &temp))
		LSetSelect(FALSE, temp, theList);
	LSetSelect(TRUE, theCell, theList);
	MyMakeCellVisible(theList, theCell);
	LSetDrawingMode(TRUE, theList);
	MyUpdateList(theList);
}

void MyMakeCellVisible(ListHandle theList, Cell theCell)
{
	Rect			visibleRect;
	short			dCols, dRows;
	
	visibleRect=(**theList).visible;
	if (!PtInRect(theCell, &visibleRect))
	{
		dCols=dRows=0;
		if (theCell.h>visibleRect.right-1)
			dCols=theCell.h-visibleRect.right+1;
		else if (theCell.h<visibleRect.left)
			dCols=theCell.h-visibleRect.left;
		else if (theCell.v>visibleRect.bottom-1)
			dRows=theCell.v-visibleRect.bottom+1;
		else if (theCell.v<visibleRect.top)
			dRows=theCell.v-visibleRect.top;
		LScroll(dCols, dRows, theList);
	}
}

void MyDeselectAllCells(ListHandle theList)
{
	Cell			nextSelectedCell;
	
	if (MyGetFirstSelectedCell(theList, &nextSelectedCell))
	{
		LSetDrawingMode(FALSE, theList);
		while (LGetSelect(TRUE, &nextSelectedCell, theList))
		{
			LSetSelect(FALSE, nextSelectedCell, theList);
		}
		LSetDrawingMode(TRUE, theList);
	}
}

void MyClearAllCells(ListHandle theList)
{
	LDelRow(0, 0, theList);
}

void MyGetCellData(ListHandle theList, Cell theCell, Str255 theName)
{
	short			len;
	
	len=255;
	LGetCell(&theName[1], &len, theCell, theList);
	theName[0]=len;
}

void MySetCellData(ListHandle theList, Cell theCell, Str255 theName)
{
	short			len;
	
	len=theName[0];
	LSetCell(&theName[1], len, theCell, theList);
}

void MyScrollList(ListHandle theList, short diff)
{
	LScroll(0, diff, theList);
}

void MySetListSelectionFlags(ListHandle theList, short flags)
{
	(**theList).selFlags=flags;
}

void MySetListSize(ListHandle theList, short width, short height, short cellHeight)
{
	Point			size;
	Point			theCell;
	
	LSize(width, height, theList);
	SetPt(&theCell, 0, 0);
	theCell=(**theList).cellSize;
	SetPt(&size, width, cellHeight);
	LCellSize(size, theList);
}

void MyGetListRect(ListHandle theList, Rect *boundsRect)
{
	*boundsRect=(**theList).rView;
	boundsRect->right+=kScrollWidth+1;
}

void MySetListRect(ListHandle theList, Rect boundsRect)
{
	boundsRect.right-=(kScrollWidth+1);
	(**theList).rView=boundsRect;
}
