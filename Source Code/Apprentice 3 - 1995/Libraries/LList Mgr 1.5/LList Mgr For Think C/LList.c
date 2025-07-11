/************************************************************/
/*															*/
/* LList.c													*/
/*															*/
/* Version 1.5, June 1994, added LLSize function,			*/
/*						   added type to colDesc record,	*/
/*						   enhanced LLSort to support		*/
/*						   ascii and numeric types			*/
/*						   and use a quick sort algorithm	*/
/* Version 1.4, May 1994, added ignoreCase to LLSearch,		*/
/*						  added drawNow to LLDoDraw,		*/
/*						  added LLSort function				*/
/* Version 1.3, April 1994, fixed bug w/no scroll bar,		*/
/*							LLDoDraw(theList, 1) no longer 	*/
/*							auto-redraws the list so call 	*/
/*							LLUpdate if necessary			*/
/* Version 1.2, December 1993, added column frame option,	*/
/*							   condenses type if needed,	*/
/*							   added LLRect function,		*/
/*							   fixed LLAddRow redraw bug	*/
/* Version 1.1, November 1993, fixed thumb scrolling bug	*/
/* Version 1.0, September 1993								*/
/* Tad Woods, T&T Software, 70312,3552 on CompuServe		*/
/*															*/
/* See LList.h for function descriptions.					*/
/*															*/
/************************************************************/

#include "LList.h"

/************************************************************/

#define LLabs(a) (((a) < 0) ? (-(a)) : (a))
#define LLmax(a,b) (((a) > (b)) ? (a) : (b))
#define LLmin(a,b) (((a) < (b)) ? (a) : (b))

/************************************************************/

typedef struct {	// minimizes use of stack space since LLQuickSort is recursive
	LList		*theList;
	short		col1;
	short		col2;
	short		col3;
	short		ignoreCase;
	short		order;
	short		middle;
} QuickSortParms;

/************************************************************/

// prototypes

LRow *RowNumToRow(LList *theList, short rowNumber);
short RowToRowNum(LRow *row);
void LLHighlight1(LList *theList, LRow *highlightRow);
void InvalRow(LList *theList, LRow *row, short count);
void LLDraw(LList *theList, short drawAllRows, short drawAllSelectedRows);
void LLDelCol1(LList *theList, LRow *row);
void LLDelRow1(LList *theList, LRow *row);
void LLScrollToRowNum(LList *theList, short rowNum);
short CompareNumbers(Ptr data1, Ptr data2, short len1, short len2);
short CompareText(Ptr data1, Ptr data2, short len1, short len2, short ignoreCase);
short LLCompare(Ptr data1, Ptr data2, short len1, short len2, short ignoreCase, short type);
short LLEqualText(Ptr data1, Ptr data2, short len1, short len2, short ignoreCase);
void LLSwap(LList *theList, LRow **row1, LRow **row2);
short LLCompare3(LList *theList, LRow *a, LRow *b, short col1, short col2, short col3, short ignoreCase);
short LLPartition(LList *theList, LRow *first, LRow *last, short col1, short col2, short col3, short ignoreCase, short order);
void LLQuickSort(QuickSortParms *qsParm, short first, short last);


/************************************************************/

LRow *RowNumToRow(LList *theList, short rowNumber)
{
	LRow		*row;
	
	row = theList->row;
	
	while ((row != NULL) && (rowNumber > 0))
	{
		row = row->nextRow;
		rowNumber--;
	}
	
	return row;
} // RowNumToRow

/************************************************************/

short RowToRowNum(LRow *row)
{
	short		rowNumber = -1;
	
	while (row != NULL)
	{
		row = row->prevRow;
		rowNumber++;
	}
	
	return rowNumber;
} // RowToRowNum

/************************************************************/

void LLHighlight1(LList *theList, LRow *highlightRow)
{
	GrafPtr		remPort;
	LRow		*row;
	LColDesc	*colDesc;
	short		rowCount;
	Rect		highlightRect;
	short		loop;
	
	// go through all currently visible rows
	rowCount = 0;
	row = RowNumToRow(theList, theList->firstVisRowNum);
	while ((row != NULL) && (rowCount < theList->numOfVisRows))
	{
		if (row == highlightRow)
		{
			// if drawing is not on this row needs to be drawn later
			if ((theList->dodraw == 0) || (theList->activeFlag == 0))
			{
				row->needDraw = 1;
				return;
			}
			
			GetPort(&remPort);
			SetPort(theList->window);

			// make highlightRect describe the row's rectangle
			highlightRect.left = theList->view.left;
			highlightRect.right = theList->view.left;
			highlightRect.top = theList->view.top + rowCount * theList->height;
			highlightRect.bottom = highlightRect.top + theList->height;
			
			// go through all columns
			for(loop = 0; loop < theList->numOfColumns; loop++)
			{
				colDesc = &(theList->colDesc[loop]);
				
				// highlight cell			
				if (!(colDesc->highlight))
				{
					// this column is not highlighted, so highlight the preceeding column
					if (highlightRect.left != highlightRect.right)
					{
						*((Byte *)0x0938) &= 0x7F;	// use hilite transfer mode
						InvertRect(&highlightRect);
						*((Byte *)0x0938) |= 0x80;
					}
					highlightRect.left = highlightRect.right + colDesc->width;
				}
				// add this column to the highlight rect
				highlightRect.right += colDesc->width;
			}
			// highlight whole row at once so it looks good on screen
			if (highlightRect.left != highlightRect.right)
			{
				*((Byte *)0x0938) &= 0x7F;	// use hilite transfer mode
				InvertRect(&highlightRect);
				*((Byte *)0x0938) |= 0x80;
			}
			SetPort(remPort);
			return;
		}	
		row = row->nextRow;
		rowCount++;
	}
} // LLHighlight1

/************************************************************/

void InvalRow(LList *theList, LRow *row, short count)
{
	LRow		*row2;
	short		rowSearch;
	short		drawFlag;
	
	if (count < 1)
		return;
		
	// start with first visible row
	row2 = RowNumToRow(theList, theList->firstVisRowNum);
	
	// set drawFlag for row and count following rows if they are currently visible
	drawFlag = 0;
	rowSearch = 0;
	while ((row2 != NULL) && (rowSearch < theList->numOfVisRows))
	{
		if ((drawFlag) || (row2 == row))
		{
			row2->needDraw = 1;
			
			count--;
			if (count < 1)
				drawFlag = 0;
			else 
				drawFlag = 1;
		}
		
		row2 = row2->nextRow;
		rowSearch++;
	}
} // InvalRow

/************************************************************/

void LLDraw(LList *theList, short drawAllRows, short drawAllSelectedRows)
{
	GrafPtr		remPort;
	short		remFont;
	short		remSize;
	short		remStyle;
	short		remhSize;
	short		remvSize;
	LRow		*row;
	LCol		*col;
	LColDesc	*colDesc;
	short		rowCount;
	short		width;
	short		nextWidth;
	short		size1;
	short		size2;
	Rect		theRect;
	Rect		highlightRect;
	short		loop;
	
	if (theList->dodraw == 0)
		return;
	
	GetPort(&remPort);
	SetPort(theList->window);
	remFont = theList->window->txFont;
	remSize = theList->window->txSize;
	remStyle = theList->window->txFace;
	TextFont(theList->font);
	TextSize(theList->size);
	
	// go through all currently visible rows
	rowCount = 0;
	row = RowNumToRow(theList, theList->firstVisRowNum);
	while (rowCount < theList->numOfVisRows)
	{
		// make theRect describe the row's rectangle
		theRect.left = theList->view.left;
		theRect.right = theList->view.right;
		theRect.top = theList->view.top + rowCount * theList->height;
		theRect.bottom = theRect.top + theList->height;

		if (row == NULL)
		{
			// erase empty row
			EraseRect(&theRect);
		}
		
		// draw and highlight row
		else if ((drawAllRows) || (row->needDraw) || ((drawAllSelectedRows) && (row->selected)))
		{
			// copy theRect to highlightRect
			highlightRect.left = theRect.left;
			highlightRect.right = theRect.left;
			highlightRect.top = theRect.top;
			highlightRect.bottom = theRect.bottom;
			
			// erase the row
			EraseRect(&theRect);
			
			row->needDraw = 0;
			
			// go through all columns
			for(loop = 0; loop < theList->numOfColumns; loop++)
			{
				colDesc = &(theList->colDesc[loop]);
				col = &(row->col[loop]);
				
				// set right boundary to individual cell size
				theRect.right = theRect.left + colDesc->width;

				// draw contents of the cell
				if ((colDesc->dodraw) && (col->data != NULL))
				{
					TextFace(colDesc->style);
					width = 0;
					size1 = 0;
					size2 = col->dataLen;
					while (size1 < size2)
					{
						// include next character if it fits within the column width
						nextWidth = CharWidth(((char *)(col->data))[size1]);
						if (width + nextWidth < colDesc->width - theList->indent)
						{
							width += nextWidth;
							size1++;
						}
						else
						{
							// stop if we've already tried condensed type
							if (theList->window->txFace & 0x0020)
								break;
								
							// try to fit again with condensed type
							TextFace(colDesc->style | 32);
							width = 0;
							size1 = 0;
						}
						
					}
					
					if (colDesc->justify == 0)		// left justify
						MoveTo(theRect.left + 2, theRect.bottom - theList->baseline);
					else if (colDesc->justify > 0)	// center
						MoveTo(theRect.left + ((colDesc->width - width) / 2), theRect.bottom - theList->baseline);
					else if (colDesc->justify < 0)	// right justify
						MoveTo(theRect.right - width - theList->indent, theRect.bottom - theList->baseline);
					DrawText(col->data, 0, size1);
				}

				// frame the cell
				if (colDesc->frame)
				{
					remhSize = theList->window->pnSize.h;
					remvSize = theList->window->pnSize.v;
					PenSize(1,1);
					
					// frame left
					if ((colDesc->frame & 0x0008) && (loop > 0))
					{
						MoveTo(theRect.left, theRect.top-1);
						LineTo(theRect.left, theRect.bottom-1);
					}
					// frame right
					if (colDesc->frame & 0x0004)
					{
						MoveTo(theRect.right, theRect.top-1);
						LineTo(theRect.right, theRect.bottom-1);
					}
					// frame top
					if (colDesc->frame & 0x0002)
					{
						MoveTo(theRect.left, theRect.top-1);
						LineTo(theRect.right, theRect.top-1);
					}
					// frame bottom
					if (colDesc->frame & 0x0001)
					{
						MoveTo(theRect.left, theRect.bottom-1);
						LineTo(theRect.right, theRect.bottom-1);
					}
					PenSize(remhSize,remvSize);
				}
				
				// highlight cell			
				if (!((theList->activeFlag) && (row->selected) && (colDesc->highlight)))
				{
					// this column is not highlighted, so highlight the preceeding column
					if (highlightRect.left != highlightRect.right)
					{
						*((Byte *)0x0938) &= 0x7F;	// use hilite transfer mode
						InvertRect(&highlightRect);
						*((Byte *)0x0938) |= 0x80;
					}
					highlightRect.left = highlightRect.right + colDesc->width;
				}
				// add this column to the highlight rect
				highlightRect.right += colDesc->width;
				
				theRect.left += colDesc->width;
			}
			// highlight whole row at once so it looks good on screen
			if (highlightRect.left != highlightRect.right)
			{
				*((Byte *)0x0938) &= 0x7F;	// use hilite transfer mode
				InvertRect(&highlightRect);
				*((Byte *)0x0938) |= 0x80;
			}
		}	

		if (row != NULL)
			row = row->nextRow;
		rowCount++;
	}
	
	// frame the list and draw the scroll bar if there is one
	theRect.top = theList->view.top-1;
	theRect.bottom = theList->view.bottom+1;
	theRect.left = theList->view.left-1;
	theRect.right = theList->view.right+1;
	FrameRect(&theRect);
	if (theList->scroll)
		Draw1Control(theList->scroll);
	
	// restore the original state of the drawing port
	TextFont(remFont);
	TextSize(remSize);
	TextFace(remStyle);
	SetPort(remPort);
} // LLDraw

/************************************************************/

pascal void LLUpdate(LList *theList)
{
	LLDraw(theList, 1, 0);
} // LLUpdate

/************************************************************/

pascal void LLDoDraw(LList *theList, short dodraw, short drawNow)
{
	theList->dodraw = dodraw;
	if ((dodraw) && (drawNow))
		LLDraw(theList, 0, 0);
} // LLDoDraw

/************************************************************/

pascal void LLActivate(LList *theList, short activate)
{
	LRow		*row;
	
	if (activate)
	{
		if (theList->scroll)
			ShowControl(theList->scroll);
		theList->activeFlag = 1;
	}
	else
	{
		if (theList->scroll)
			HideControl(theList->scroll);
		theList->activeFlag = 0;
	}
	LLDraw(theList, 0, 1);
} // LLActivate

/************************************************************/

pascal LRow *LLNextRow(LList *theList, LRow *row)
{
	if (row == NULL)
		return theList->row;
	else
		return row->nextRow;
} // LLNextRow

/************************************************************/

pascal LRow *LLPrevRow(LList *theList, LRow *row)
{
	if (row == NULL)
		return RowNumToRow(theList, theList->numOfRows-1);
	else
		return row->prevRow;
} // LLNextRow

/************************************************************/

pascal LList *LLNew(Rect *view, WindowPtr window, short rowHeight, short numOfColumns, short hasScroll, short selFlags)
{
	LList		*theList;
	Rect		ctrlRect;
	short		loop;
	
	theList = (LList *)NewPtr(sizeof(LList) + (sizeof(LColDesc) * numOfColumns));
	if (theList == NULL)
		return NULL;
	theList->view.top = view->top;
	theList->view.bottom = view->bottom;
	theList->view.left = view->left;
	theList->view.right = view->right;
	theList->window = window;
	theList->font = window->txFont;
	theList->size = window->txSize;
	theList->height = rowHeight;
	theList->baseline = 4;
	theList->indent = 2;
	theList->dodraw = 1;
	theList->selFlags = selFlags;
	theList->activeFlag = 1;
	theList->numOfRows = 0;
	theList->firstVisRowNum = 0;
	theList->numOfVisRows = (view->bottom - view->top) / rowHeight;
	theList->numOfColumns = numOfColumns;
	theList->refCon = 0L;
	theList->row = NULL;
	theList->lastClkRow = NULL;
	theList->lastClkTime = 0L;
	for(loop = 0; loop < numOfColumns; loop++)
	{
		theList->colDesc[loop].width = (view->right - view->left) / numOfColumns;
		theList->colDesc[loop].style = 0;		// plain
		theList->colDesc[loop].justify = 0;		// left
		theList->colDesc[loop].dodraw = 1;		// drawing on
		theList->colDesc[loop].highlight = 1;	// highlight on
		theList->colDesc[loop].frame = 0x0000;	// no frame
		theList->colDesc[loop].type = 0;		// ascii text
	}
	
	if (hasScroll)
	{
		ctrlRect.top = view->top-1;
		ctrlRect.bottom = view->bottom+1;
		ctrlRect.left = view->right;
		ctrlRect.right = view->right+16;
		theList->scroll = NewControl(window, &ctrlRect, "\p", 1, 0, 0, 0, scrollBarProc, (long)theList);
	}
	else
		theList->scroll = NULL;
	
	return theList;
} // LLNew

/************************************************************/

pascal void LLSize(LList *theList, short listWidth, short listHeight)
{
	GrafPtr		remPort;
	RgnHandle	oldListRgn;
	RgnHandle	newListRgn;
	RgnHandle	diffRgn;
	Rect		oldView;
	Rect		newView;
	short		oldWidth = theList->view.right - theList->view.left;
	short		oldVisRows = theList->numOfVisRows;
	
	GetPort(&remPort);
	SetPort(theList->window);

	// calculate old view invluding frame
	oldView.top = theList->view.top - 1;
	oldView.bottom = theList->view.bottom + 1;
	oldView.left = theList->view.left - 1;
	oldView.right = theList->view.right + 1;
	
	// redefine list to new view rect
	theList->view.bottom = theList->view.top + listHeight;
	theList->view.right = theList->view.left + listWidth;
	theList->numOfVisRows = listHeight / theList->height;

	// calculate new view invluding frame
	newView.top = theList->view.top - 1;
	newView.bottom = theList->view.bottom + 1;
	newView.left = theList->view.left - 1;
	newView.right = theList->view.right + 1;

	// redraw or invalidate only the invalid portion of the list
	
	// invalidate any left over part of a row after the last row in a shorter new list
	if (oldView.bottom < newView.bottom)
		oldView.bottom -= (listHeight % theList->height);
	else
		newView.bottom -= (listHeight % theList->height);

	// erase or invalidate the invalid region between the old and new lists
	oldListRgn = NewRgn();
	newListRgn = NewRgn();
	diffRgn = NewRgn();
	
	RectRgn(oldListRgn, &oldView);
	RectRgn(newListRgn, &newView);

	DiffRgn(oldListRgn, newListRgn, diffRgn);
	if (theList->dodraw)
		EraseRgn(diffRgn);
	else
		InvalRgn(diffRgn);

	DiffRgn(newListRgn, oldListRgn, diffRgn);
	if (theList->dodraw)
		EraseRgn(diffRgn);
	else
		InvalRgn(diffRgn);
	
	DisposeRgn(oldListRgn);
	DisposeRgn(newListRgn);
	DisposeRgn(diffRgn);
	
	// invalidate affected rows
	if (oldWidth < listWidth)
		InvalRow(theList, RowNumToRow(theList, theList->firstVisRowNum), theList->numOfVisRows);
	else if (oldVisRows < theList->numOfVisRows)
		InvalRow(theList, RowNumToRow(theList, theList->firstVisRowNum + oldVisRows), theList->numOfVisRows - oldVisRows);
	
	// update the scroll bar
	if (theList->scroll)
	{
		MoveControl(theList->scroll, theList->view.right, theList->view.top - 1);
		SizeControl(theList->scroll, 16, listHeight + 2);
		if (theList->scroll)
			SetCtlMax(theList->scroll, LLmax(theList->numOfRows - theList->numOfVisRows, 0));
	}

	SetPort(remPort);
	
	// scroll the list to fit the new view (LLScroll calls LLDraw)
	LLScroll(theList, 0);
} // LLSize

/************************************************************/

void LLDelCol1(LList *theList, LRow *row)
{
	short		loop;

	// dispose of the data in columns
	for(loop = 0; loop < theList->numOfColumns; loop++)
		if (row->col[loop].data)
			DisposPtr(row->col[loop].data);
} // LLDelCol1

/************************************************************/

pascal void LLDispose(LList *theList)
{
	LRow		*row;
	short		loop;
	
	// dispose of scroll bar
	if (theList->scroll)
		DisposeControl(theList->scroll);
	
	// dispose of all rows
	while ((row = theList->row) != NULL)
	{
		theList->row = row->nextRow;
		
		// dispose of the data in columns
		LLDelCol1(theList, row);

		DisposPtr((Ptr)row);
	}

	// dispose of this list
	DisposPtr((Ptr)theList);
} // LLDispose

/************************************************************/

pascal LRow *LLAddRow(LList *theList, LRow *beforeRow)
{
	LRow		*row;
	short		rowNum;
	short		loop;
	
	if (theList->numOfRows == 32767)
		return NULL;
	row = (LRow *)NewPtr(sizeof(LRow) + (sizeof(LCol) * theList->numOfColumns));
	if (row == NULL)
		return NULL;
	row->nextRow = NULL;
	row->prevRow = NULL;
	row->needDraw = 0;
	row->selected = 0;
	for(loop = 0; loop < theList->numOfColumns; loop++)
		row->col[loop].data = NULL;
	
	if (beforeRow == NULL) {
		if (theList->row == NULL)
		{
			// insert as only row
			theList->row = row;
		}
		else
		{
			// insert row as last row
			beforeRow = theList->row;
			while (beforeRow->nextRow != NULL)
				beforeRow = beforeRow->nextRow;
			beforeRow->nextRow = row;
			row->prevRow = beforeRow;
		}
	}
	else
	{
		// insert row before an existing row
		row->prevRow = beforeRow->prevRow;
		row->nextRow = beforeRow;
		beforeRow->prevRow = row;
		if (row->prevRow != NULL)
			((LRow *)(row->prevRow))->nextRow = row;
		else
			// first row in list
			theList->row = row;
	}
	
	// increment total number of rows
	theList->numOfRows++;
	
	// update scroll bar's maximum range
	if (theList->scroll)
		SetCtlMax(theList->scroll, LLmax(theList->numOfRows - theList->numOfVisRows, 0));
	
	rowNum = RowToRowNum(row);
	if (rowNum < theList->firstVisRowNum)
	{
		// compensate when row is added before visible rows
		theList->firstVisRowNum++;
		if (theList->scroll)
			SetCtlValue(theList->scroll, theList->firstVisRowNum);
	}
	else if (rowNum < theList->firstVisRowNum + theList->numOfVisRows)
	{
		// redraw when row is added within visible rows
		InvalRow(theList, row, theList->numOfVisRows);
		LLDraw(theList, 0, 0);
	}
	
	return row;
} // LLAddRow

/************************************************************/

void LLDelRow1(LList *theList, LRow *row)
{
	short		loop;
	
	theList->numOfRows--;
	if (theList->firstVisRowNum > theList->numOfRows)
		theList->firstVisRowNum = theList->numOfRows;
	InvalRow(theList, row, theList->numOfVisRows);
	if (theList->scroll)
		SetCtlMax(theList->scroll, LLmax(theList->numOfRows - theList->numOfVisRows, 0));

	if (row->prevRow == NULL)
		// row was first row in list
		theList->row = row->nextRow;
	else
		((LRow *)(row->prevRow))->nextRow = row->nextRow;
		
	if (row->nextRow != NULL)
	{
		// row was not the last row
		((LRow *)(row->nextRow))->prevRow = row->prevRow;
	}
	
	// dispose of the data in columns
	LLDelCol1(theList, row);
	
	if (row->nextRow != NULL)
		InvalRow(theList, row->nextRow, theList->numOfVisRows);
	DisposPtr((Ptr)row);
} // LLDelRow1

/************************************************************/

pascal void LLDelRow(LList *theList, LRow *row)
{
	if (row == NULL)
	{
		// delete all rows quickly
		row = theList->row;
		while (row != NULL)
		{
			theList->row = row->nextRow;
			LLDelCol1(theList, row);
			DisposPtr((Ptr)row);
			row = theList->row;
		}
		if (theList->scroll)
			SetCtlMax(theList->scroll, 0);
		theList->numOfRows = 0;
		theList->firstVisRowNum = 0;
	}
	else
		LLDelRow1(theList, row);
	LLDraw(theList, 0, 0);
} // LLDelRow

/************************************************************/

pascal short LLGetSelect(LList *theList, LRow **row, short advanceit)
{
	LRow		*row2;
	
	row2 = *row;
	if (row2 == NULL)
		return 0;
	else if (row2->selected)
		return 1;
	else if (advanceit)
	{
		while ((row2 = row2->nextRow) != NULL)
		{
			if (row2->selected)
			{
				*row = row2;
				return 1;
			}
		}
	}
	return 0;
} // LLGetSelect

/************************************************************/

pascal void LLSetSelect(LList *theList, LRow *row, short setit)
{
	if (row->selected != setit)
	{
		row->selected = setit;
		LLHighlight1(theList, row);
	}
} // LLSetSelect

/************************************************************/

pascal void LLScroll(LList *theList, short rows)
{
	GrafPtr		remPort;
	Rect		theRect;
	RgnHandle	theUpdateRgn;
	
	// don't scroll past end of list
	if ((theList->firstVisRowNum + theList->numOfVisRows) + rows > theList->numOfRows)
		rows = theList->numOfRows - (theList->firstVisRowNum + theList->numOfVisRows);
		
	// don't scroll past start of list
	if (theList->firstVisRowNum + rows < 0)
		rows = -(theList->firstVisRowNum);
	
	theList->firstVisRowNum += rows;

	// call Quickdraw to scroll the rows if we are scrolling less than a whole "page"
	if (LLabs(rows) < theList->numOfVisRows)
	{
		theRect.top = theList->view.top;
		theRect.bottom = theRect.top + (theList->numOfVisRows * theList->height);
		theRect.left = theList->view.left;
		theRect.right = theList->view.right;
		GetPort(&remPort);
		SetPort(theList->window);
		theUpdateRgn = NewRgn();
		ScrollRect(&theRect, 0, -(rows * theList->height), theUpdateRgn);
		if (theList->dodraw == 0)
			InvalRgn(theUpdateRgn);
		DisposeRgn(theUpdateRgn);
		SetPort(remPort);
		if (rows > 0)
			// draw new rows at bottom
			InvalRow(theList, RowNumToRow(theList, theList->firstVisRowNum + theList->numOfVisRows - rows), rows);
		else
			// draw new rows at top
			InvalRow(theList, RowNumToRow(theList, theList->firstVisRowNum), -rows);
	}
	else
		InvalRow(theList, RowNumToRow(theList, theList->firstVisRowNum), theList->numOfVisRows);
		
	if (theList->scroll)
		SetCtlValue(theList->scroll, theList->firstVisRowNum);
	LLDraw(theList, 0, 0);
} // LLScroll

/************************************************************/

void LLScrollToRowNum(LList *theList, short rowNum)
{
	// converts rows into a value relative to the first visible row
	LLScroll(theList, rowNum - theList->firstVisRowNum);
} // LLScrollToRowNum

/************************************************************/

pascal void LLScrollToRow(LList *theList, LRow *row)
{
	LRow		*row2;
	short		rowCount;
	
	if (row != NULL)
	{
		rowCount = 0;
		row2 = theList->row;
		while (row2 != NULL)
		{
			if (row2 == row)
			{
				LLScrollToRowNum(theList, rowCount);
				return;
			}
			rowCount++;
			row2 = row2->nextRow;
		}
	}
	
	// scroll to end of list
	LLScroll(theList, theList->numOfRows);
	return;
} // LLScrollToRow

/************************************************************/

pascal void LLScrollAction(ControlHandle scroll, short part)
{
	LList		*theList;
	
	theList = (LList *)((**scroll).contrlRfCon);
	
	switch (part) {
		case inUpButton:
			LLScroll(theList, -1);
			break;
		case inDownButton:
			LLScroll(theList, 1);
			break;
		case inPageUp:
			LLScroll(theList, -(theList->numOfVisRows-1));
			break;
		case inPageDown:
			LLScroll(theList, theList->numOfVisRows-1);
			break;
		default:
			break;
	}
} // LLScrollAction

/************************************************************/

pascal short LLClick(LList *theList, Point localPoint, short modifiers)
{
	short			controlPart;
	ControlHandle	whichControl;
	LRow			*clickRow;
	long			clickTime;
	short			dblClickFlag;
	LRow			*row;
	short			startRowNum;
	short			stopRowNum;
	short			remRowNum;
	short			rowCount;
	short			highlight;
	short			llNoNilHilite = (theList->selFlags & LLNoNilHilite);
	short			llUseSense = (theList->selFlags & LLUseSense);
	short			llNoExtend = (theList->selFlags & LLNoExtend);
	short			llNoDisjoint = (theList->selFlags & LLNoDisjoint);
	short			llExtendDrag = (theList->selFlags & LLExtendDrag);
	short			llOnlyOne = (theList->selFlags & LLOnlyOne);
	short			shiftDown = (modifiers & 0x0200);
	short			cmdDown = (modifiers & 0x0100);
	
	if ((controlPart = FindControl(localPoint, theList->window, &whichControl)) && (whichControl == theList->scroll))
	{
		if (controlPart == inThumb)
		{
			if (TrackControl(whichControl, localPoint, NULL) == inThumb)
				LLScrollToRowNum(theList, GetCtlValue(whichControl));
		}
		else
			TrackControl(whichControl, localPoint, &LLScrollAction);
	}
	
	// is the mouse initially inside the list rectangle?
	else if (PtInRect(localPoint, &(theList->view)))
	{
		// get the "time" of the click
		clickTime = TickCount();

		// make the clicked in row the startRowNum
		startRowNum = (localPoint.v - theList->view.top) / theList->height + theList->firstVisRowNum;
		clickRow = RowNumToRow(theList, startRowNum);
		
		if ((llNoNilHilite) && (clickRow == NULL))
			return 0;
		
		if (llUseSense)
			highlight = !(LLGetSelect(theList, &clickRow, 0));
		else
			highlight = 1;

		// if extending the selection, make the farthest away selected row the startRowNum
		if ((shiftDown) && (!llNoExtend))
		{
			remRowNum = startRowNum;
			rowCount = 0;
			row = theList->row;
			while (row != NULL)
			{
				if (LLGetSelect(theList, &row, 0))
				{
					startRowNum = rowCount;
					if (startRowNum <= remRowNum)
						break;
				}
				rowCount++;
				row = row->nextRow;
			}
		}
		
		remRowNum = -1;
		do
		{
			// make the row where the mouse is now the stopRowNum
			stopRowNum = (localPoint.v - theList->view.top) / theList->height;
			if (stopRowNum < 0)
				stopRowNum = 0;
			else if (stopRowNum > theList->numOfVisRows)
				stopRowNum = theList->numOfVisRows;
			stopRowNum += theList->firstVisRowNum;
			
			if (stopRowNum != remRowNum)
			{
				if ((llOnlyOne) || ((!llExtendDrag) && (!shiftDown)) || (llNoDisjoint))
				{
					startRowNum = stopRowNum;
					llNoDisjoint = 0;
				}

				rowCount = 0;
				row = theList->row;
				while (row != NULL)
				{
					if ((rowCount >= LLmin(startRowNum, stopRowNum)) && (rowCount <= LLmax(startRowNum, stopRowNum)))
					{
						LLSetSelect(theList, row, highlight);
					}
					else
					{
						if ((llOnlyOne) || ((!llExtendDrag) && (!cmdDown) && (!shiftDown)))
							LLSetSelect(theList, row, 0);
					}
					
					rowCount++;
					row = row->nextRow;
				}
				
				// prevent a double-click if there's any dragging
				dblClickFlag = (remRowNum == -1);
					
				remRowNum = stopRowNum;
			}

			if (localPoint.v < theList->view.top)
				// auto scroll up
				LLScroll(theList, -1);
			else if (localPoint.v > theList->view.bottom)
				// auto scroll down
				LLScroll(theList, 1);

			GetMouse(&localPoint);
		} while (Button());
		
		// return 1 if double-click occurs in same row within time limit of double-click
		if ((clickRow == theList->lastClkRow) && (clickTime < theList->lastClkTime + GetDblTime()) && (clickRow != NULL) && (!llUseSense))
			return 1;
			
		theList->lastClkRow = clickRow;
		if (dblClickFlag)
			theList->lastClkTime = clickTime;
	}
	return 0;
} // LLClick

/************************************************************/

pascal void LLGetCell(LList *theList, LRow *row, short colNum, short *dataLen, Ptr data)
{
	if (row->col[colNum].data == NULL)
		*dataLen = 0;
	else 
	{
		*dataLen = LLmin(*dataLen, row->col[colNum].dataLen);
		BlockMove(row->col[colNum].data, data, *dataLen);
	}
} // LLGetCell

/************************************************************/

pascal void LLSetCell(LList *theList, LRow *row, short colNum, short dataLen, Ptr data)
{
	if (row->col[colNum].data != NULL)
		DisposPtr(row->col[colNum].data);
	
	if (dataLen == 0)
		row->col[colNum].data = NULL;
	else 
	{
		row->col[colNum].data = NewPtr(dataLen);
		if (row->col[colNum].data == NULL)
			row->col[colNum].dataLen = 0;
		else
		{
			row->col[colNum].dataLen = dataLen;
			BlockMove(data, row->col[colNum].data, dataLen);
		}
	}
	InvalRow(theList, row, 1);
	LLDraw(theList, 0, 0);
} // LLSetCell

/************************************************************/

short CompareNumbers(Ptr data1, Ptr data2, short len1, short len2)
{
	Str32		n1Str, n2Str, f1Str, f2Str;
	StringPtr	s;
	long		n1, n2, f1, f2;
	short		pos;
	
	pos = 0;
	n1Str[0] = '\0';
	f1Str[0] = '\0';
	s = n1Str;
	while (pos < LLmin(len1, 32))
	{
		if (data1[pos] == '.')
			s = f1Str;
		else if (((data1[pos] >= '0') && (data1[pos] <= '9')) || (data1[pos] == '-')  || (data1[pos] == '+'))
		{
			(s[0])++;
			s[s[0]] = data1[pos];
		}
		pos++;
	}
		
	pos = 0;
	n2Str[0] = '\0';
	f2Str[0] = '\0';
	s = n2Str;
	while (pos < LLmin(len2, 32))
	{
		if (data2[pos] == '.')
			s = f2Str;
		else if (((data2[pos] >= '0') && (data2[pos] <= '9')) || (data2[pos] == '-')  || (data2[pos] == '+'))
		{
			(s[0])++;
			s[s[0]] = data2[pos];
		}
		pos++;
	}

	StringToNum(n1Str, &n1);
	StringToNum(n2Str, &n2);
	
	if (n1 > n2)
		return 1;		// data1 > data2
	else if (n1 < n2)
		return -1;		// data1 < data2
	else
	{
		while (f1Str[0] < f2Str[0])
		{
			(f1Str[0])++;
			f1Str[f1Str[0]] = '0';
		}
		
		while (f2Str[0] < f1Str[0])
		{
			(f2Str[0])++;
			f2Str[f2Str[0]] = '0';
		}

		StringToNum(f1Str, &f1);
		StringToNum(f2Str, &f2);
		
		if (f1 > f2)
			return 1;	// data1 > data2
		else if (f1 < f2)
			return -1;	// data1 < data2
		else
			return 0;	// data1 == data2
	}
} // CompareNumbers

/************************************************************/

short CompareText(Ptr data1, Ptr data2, short len1, short len2, short ignoreCase)
{
	char	c1, c2;
	
	if ((len1 == 0) | (len2 == 0))
	{
		if (len1 > 0)
			return 1;	// data1 > data2
		if (len2 > 0)
			return -1;	// data1 < data2
		return 0;		// data1 == data2
	}
	
	len2 = LLmin(len1, len2);
	len1 = 0;
	
	while (len1 < len2)
	{
		c1 = ((char *)data1)[len1];
		c2 = ((char *)data2)[len1];
		if (ignoreCase)
		{
			if ((c1 >= 0x61) && (c1 <= 0x7A)) c1 -= 0x20;
			if ((c2 >= 0x61) && (c2 <= 0x7A)) c2 -= 0x20;
		}
		if (c1 > c2)
			return 1;	// data1 > data2
		else if (c1 < c2)
			return -1;	// data1 < data2
		len1++;
	}
	
	return 0;			// data1 == data2
} // CompareText

/************************************************************/

short LLCompare(Ptr data1, Ptr data2, short len1, short len2, short ignoreCase, short type)
{
	switch (type)
	{
		case 0:
			return CompareText(data1, data2, len1, len2, ignoreCase);
			
		case 1:
			return CompareNumbers(data1, data2, len1, len2);
			
		default:
			return CompareText(data1, data2, len1, len2, ignoreCase);
	}
} // LLCompare

/************************************************************/

short LLEqualText(Ptr data1, Ptr data2, short len1, short len2, short ignoreCase)
{
	if (len1 != len2)
		return 0;
	else
		return (CompareText(data1, data2, len1, len2, ignoreCase) == 0);
} // LLEqualText

/************************************************************/

pascal short LLSearch(LList *theList, LRow **row, short *colFound, short colFirst, short colLast, Ptr data, short dataLen, short ignoreCase)
{
	LRow		*row2;
	short		loop;
	
	row2 = *row;
	colLast = LLmin(colLast, theList->numOfColumns - 1);
	
	if (row2 == NULL)
		row2 = theList->row;
		
	while (row2 != NULL)
	{
		for(loop = colFirst; loop <= colLast; loop++)
		{
			if (LLEqualText(data, row2->col[loop].data, dataLen, row2->col[loop].dataLen, ignoreCase))
			{
				*row = row2;
				*colFound = loop;
				return 1;
			}
		}
		row2 = row2->nextRow;
	}
	return 0;
} // LLSearch

/************************************************************/

void LLSwap(LList *theList, LRow **row1, LRow **row2)
{
	LRow		*remRow;
	
	if (*row1 == *row2)
		return;
	else if ((*row1)->nextRow == *row2)
	{
		if ((*row2)->nextRow)
			((LRow *)((*row2)->nextRow))->prevRow = *row1;
		(*row1)->nextRow = (*row2)->nextRow;
		if ((*row1)->prevRow)
			((LRow *)((*row1)->prevRow))->nextRow = *row2;
		else
			theList->row = *row2;
		(*row2)->prevRow = (*row1)->prevRow;
		(*row1)->prevRow = *row2;
		(*row2)->nextRow = *row1;
	}
	else if ((*row2)->nextRow == *row1)
	{
		if ((*row1)->nextRow)
			((LRow *)((*row1)->nextRow))->prevRow = *row2;
		(*row2)->nextRow = (*row1)->nextRow;
		if ((*row2)->prevRow)
			((LRow *)((*row2)->prevRow))->nextRow = *row1;
		else
			theList->row = *row1;
		(*row1)->prevRow = (*row2)->prevRow;
		(*row2)->prevRow = *row1;
		(*row1)->nextRow = *row2;
	}
	else // rows are not adjacent
	{
		if ((*row2)->nextRow)
			((LRow *)((*row2)->nextRow))->prevRow = *row1;
		if ((*row1)->nextRow)
			((LRow *)((*row1)->nextRow))->prevRow = *row2;
		if ((*row2)->prevRow)
			((LRow *)((*row2)->prevRow))->nextRow = *row1;
		else
			theList->row = *row1;
		if ((*row1)->prevRow)
			((LRow *)((*row1)->prevRow))->nextRow = *row2;
		else
			theList->row = *row2;
		remRow = (*row2)->nextRow;
		(*row2)->nextRow = (*row1)->nextRow;
		(*row1)->nextRow = remRow;
		remRow = (*row2)->prevRow;
		(*row2)->prevRow = (*row1)->prevRow;
		(*row1)->prevRow = remRow;
		
	}
	remRow = *row1;
	*row1 = *row2;
	*row2 = remRow;
	InvalRow(theList, *row1, 1);
	InvalRow(theList, *row2, 1);
} // LLSwap

/************************************************************/

short LLCompare3(LList *theList, LRow *a, LRow *b, short col1, short col2, short col3, short ignoreCase)
{
	short		result;
	
	result = LLCompare(a->col[col1].data, b->col[col1].data, a->col[col1].dataLen, b->col[col1].dataLen, ignoreCase, theList->colDesc[col1].type);
	if ((result == 0) && (col2 >= 0))
	{
		result = LLCompare(a->col[col2].data, b->col[col2].data, a->col[col2].dataLen, b->col[col2].dataLen, ignoreCase, theList->colDesc[col2].type);
		if ((result == 0) && (col3 >= 0))
			result = LLCompare(a->col[col3].data, b->col[col3].data, a->col[col3].dataLen, b->col[col3].dataLen, ignoreCase, theList->colDesc[col3].type);
	}
	
	return result;
} // LLCompare3

/************************************************************/

short LLPartition(LList *theList, LRow *first, LRow *last, short col1, short col2, short col3, short ignoreCase, short order)
{
	LRow		*pivot;
	LRow		*first1;
	LRow		*last1;
	short		overlap = 0;
	
	// order the first and last elements
	if (LLCompare3(theList, first, last, col1, col2, col3, ignoreCase) == order)
		LLSwap(theList, &first, &last);
	
	pivot = first;
	first1 = first;
	last1 = last;
	
	// keep partitioning as long as the first and last pointers have not overlapped
	while (!overlap)
	{
		// move first1 forward
		while (LLCompare3(theList, first1, pivot, col1, col2, col3, ignoreCase) != order)
		{
			first1 = first1->nextRow;
			if (first1 == last1)
				overlap = 1;
			if (first1 == last)
				break;
		}
		
		// move last1 backward
		while (LLCompare3(theList, last1, pivot, col1, col2, col3, ignoreCase) == order)
		{
			last1 = last1->prevRow;
			if (last1 == first1)
				overlap = 1;
			if (last1 == first)
				break;
		}
		
		if (!overlap)
			LLSwap(theList, &first1, &last1);
	}
	
	// put the pivot element in the middle of the sub-list
	LLSwap(theList, &pivot, &last1);
	
	return RowToRowNum(last1);
} // LLPartition

/************************************************************/

void LLQuickSort(QuickSortParms *qsParm, short first, short last)
{
	while (first < last)
	{
		qsParm->middle = LLPartition(qsParm->theList, RowNumToRow(qsParm->theList, first), RowNumToRow(qsParm->theList, last), qsParm->col1, qsParm->col2, qsParm->col3, qsParm->ignoreCase, qsParm->order);
		
		// sort the smaller half of the list recursively to minimze recursive calls and save stack space
		if (qsParm->middle - first < last - qsParm->middle)
		{
			LLQuickSort(qsParm, first, qsParm->middle - 1);
			first = qsParm->middle + 1; // LLQuickSort(qsParm, qsParm->middle + 1, last);
		}
		else
		{
			LLQuickSort(qsParm, qsParm->middle + 1, last);
			last = qsParm->middle - 1; // LLQuickSort(qsParm, first, qsParm->middle - 1);
		}
	}
} // LLQuickSort

/************************************************************/

pascal void LLSort(LList *theList, short col1, short col2, short col3, short ignoreCase, short order)
{
	QuickSortParms		qsParm;
	
	if (col1 < 0)
		return;

	if ((order != 1) && (order != -1))
		order = 1;
		
	qsParm.theList = theList;
	qsParm.col1 = LLmin(col1, theList->numOfColumns - 1);
	qsParm.col2 = LLmin(col2, theList->numOfColumns - 1);
	qsParm.col3 = LLmin(col3, theList->numOfColumns - 1);
	qsParm.ignoreCase = ignoreCase;
	qsParm.order = order;

	LLQuickSort(&qsParm, 0, theList->numOfRows - 1);
	
	LLDraw(theList, 0, 0);
} // LLSort

/************************************************************/

pascal void LLRect(LList *theList, LRow *row, short colFirst, short colLast, Rect *theRect)
{
	LRow		*row2;
	short		row2Count;
	short		loop;
	
	// go through all visible rows
	row2Count = 0;
	row2 = RowNumToRow(theList, theList->firstVisRowNum);
	while ((row2 != NULL) && (row2Count < theList->numOfVisRows))
	{
		if (row2 == row)
		{
			// set theRect to the visible rectangle of the row and column(s)
			theRect->left = theList->view.left;
			theRect->right = theList->view.left;
			theRect->top = theList->view.top + row2Count * theList->height;
			theRect->bottom = theRect->top + theList->height;

			for(loop = 0; loop <= colLast; loop++)
			{
				if (loop < colFirst)
					theRect->left += theList->colDesc[loop].width;
				theRect->right += theList->colDesc[loop].width;
			}
			
			return;
		}
		
		row2 = row2->nextRow;
		row2Count++;
	}
	
	// the row isn't visible
	theRect->top = -1;
	theRect->bottom = -1;
	theRect->left = -1;
	theRect->right = -1;
} // LLRect

/************************************************************/
