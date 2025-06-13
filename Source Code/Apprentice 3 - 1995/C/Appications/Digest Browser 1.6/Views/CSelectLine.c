/******************************************************************************
	CSelectLine.c
	
	Methods for a text list with selectable lines.
		
	Copyright © 1989 Symantec Corporation. All rights reserved.
    Copyright © 1991 Manuel A. PŽrez.  All rights reserved.

 ******************************************************************************/


#include "CSelectLine.h"
#include <Commands.h>
#include <CDocument.h>
#include <CBartender.h>
#include <Constants.h>
#include "Global.h"
#include <string.h>

/*** Global Variables ***/
extern	CBartender	*gBartender;
extern EventRecord  gLastMouseUp;

void CSelectLine::ISelectLine(CView *anEnclosure, CBureaucrat *aSupervisor,
		short vLoc, short vHeight)
{
Rect	margin;

	selLine = -1;
	IStyleText(anEnclosure, aSupervisor,
		1, vHeight,	 				// aWidth, aHeight
		0, vLoc,					// aHEncl, aVEncl
		sizELASTIC, sizELASTIC,		// aHSizing, aVSizing
		-1);						// lineWidth
	FitToEnclosure(TRUE, FALSE);		// fit horiz, and vert

		/**
		 **	Give the edit pane a little margin.
		 **	Each element of the margin rectangle
		 **	specifies by how much to change that
		 **	edge. Positive values are down and to
		 **	right, negative values are up and to
		 **	the left.
		 **
		 **/

	(*macTE)->crOnly = -1;			// do not wrap around
	SetRect(&margin, 2, 2, -2, -2);
	ChangeSize(&margin, FALSE);
	Specify(false, true, false);	// edit, select, style
	SetCanBeGopher(true);

}

/***
 *
 * FindEnds
 *
 ***/

void	CSelectLine::FindEnds(long line, long *lineStart, long *lineEnd)
{
	register TEPtr		t;
	long				lineNum;
	
	t = *macTE;
				
	if (line < 0)						// avoid problems with negative line #
		line = 0;

	*lineStart = t->lineStarts[line];
	if (line > t->nLines)
		*lineEnd = t->teLength;			// special case, past end
	else
//		*lineEnd = t->lineStarts[line+1];
// JRB change - to get ScrollToSelection to work, lineEnd must point to
//				last char of line, not first char of next line
		*lineEnd = t->lineStarts[line+1]-1;
}

/***
 *
 * DoClick {OVERRIDE}
 *
 *	Respond to a mouse click within the EditText
 ***/

void	CSelectLine::DoClick(
	Point		hitPt,					/* Mouse location in Frame coords	*/
	short		modifierKeys,			/* State of modifier keys			*/
	long		when)					/* Tick time of mouse click			*/
{ 	
long offset;
LongPt lHitPt;

	QDToLongPt(hitPt, &lHitPt);
	SetSelectedLine(FindLine(GetCharOffset(&lHitPt)), true);
	
		// CSwitchboard will never see the mouse up that ended
		// the drag, so we stuff gLastMouseUp here to allow
		// multi-click counting to work.
		
	gLastMouseUp.what = mouseUp;
	gLastMouseUp.when = TickCount();
	gLastMouseUp.where = hitPt;
	LocalToGlobal( &gLastMouseUp.where);
	gLastMouseUp.modifiers = modifierKeys;

}

/***
 *
 * DoKeyDown {OVERRIDE}
 *
 *	Respond to a key down event
 ***/

void CSelectLine::DoKeyDown(char theChar, Byte keyCode, EventRecord *macEvent)
{
// JRB addition - allow arrow keys to change selection
// MAP modified to use keyCodes whenever possible.  Also checked
// for invalid moves.  It was giving an error before.

long selectedLine;

	selectedLine = GetSelectedLine();

    switch (keyCode) {
        case KeyHome:
        case KeyPageUp:
			SetSelectedLine(0, true);
			ScrollToSelection();
			break;
             
        case KeyPageDown:
        case KeyEnd:
			SetSelectedLine(GetNumLines()-1, true);
			ScrollToSelection();
            break;
            
        case KeyUpCursor:
			if (selectedLine > 0) 	{	
				SetSelectedLine(selectedLine - 1, true);
				ScrollToSelection();
			}
			break;

        case KeyDownCursor:
			if (selectedLine < GetNumLines()-1) {
				SetSelectedLine(selectedLine+1, true);
				ScrollToSelection();
			}
			break;

		default:
			if ((theChar == kEnterKey) || (theChar == '\r')) {
				SetSelectedLine(selectedLine+1, true);
				ScrollToSelection();
			}

			// Move selection to the top
			else if (theChar == '<') 	{
				SetSelectedLine(0, true);
				ScrollToSelection();
			}
		
			// Move selection to the bottom
			else if (theChar == '>') 	{
				SetSelectedLine(GetNumLines()-1, true);
				ScrollToSelection();
			}
			break;
	}
// end JRB addition
}

/******************************************************************************
 SetSelectedLine
 
 	Sets the selected text to the range corresponding to character positions
 	selStart through selEnd.
******************************************************************************/

void CSelectLine::SetSelectedLine( long line, Boolean fRedraw)
{
long selStart, selEnd;

	FindEnds(line, &selStart, &selEnd);
	SetSelection(selStart, selEnd, true);

	if (line != selLine) {
		selLine = line;
		if (fRedraw)
			SelectionChanged();
	}
}	/* CSelectLine::SetSelection */

long CSelectLine::GetSelectedLine(void)
{
	return selLine;
}

/******************************************************************************
 AdjustCursor {OVERRIDE}

		Mouse is inside the CSelectLine area, set the cursor to the arrow
 ******************************************************************************/

void	CSelectLine::AdjustCursor(
	Point		where,					/* Mouse location in Window coords	*/
	RgnHandle	mouseRgn)
{
	SetCursor(&arrow);					/* Use the standard arrow cursor	*/
}

