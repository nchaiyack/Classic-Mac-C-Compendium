/*************************************************************************************

 CScrollListDragger.c
	
		Used with scrolling lists that allow the movement of array elements
		within the list.  First it selects the hit cell.  Once the mouse is
		moved, it starts tracking its position.  When it is released, it moves
		the selected cell to the new position.  It will scroll the list to
		follow the mouse.
	
	SUPERCLASS = CMouseTask
	
	REQUIRES: gHandCursor be handle to the hand cursor
	
		© 1992 Dave Harkness

*************************************************************************************/


#include "CScrollListDragger.h"
#include "CScrollList.h"
#include <CArray.h>
#include <TBUtilities.h>


#define DRAGTHRESH		4				// Distance cursor must move vertically
										// ...before the cell starts to drag.


extern CursHandle		gHandCursor;


/******************************************************************************
 IScrollListDragger
******************************************************************************/

void
CScrollListDragger::IScrollListDragger( CScrollList *aList, short theModifiers,
										short aHeight, short fOptions)

{
	CMouseTask::IMouseTask( 0);
	
	itsScrollList = aList;
	itsArray = itsScrollList->GetArray();
	modifierKeys = theModifiers;
	cellHeight = aHeight;
	listOptions = fOptions;
	
	movingCell = 0;											// not moving yet
	dragging = FALSE;

}	/* CScrollListDragger::IScrollListDragger */


/******************************************************************************
 Dispose
  	
  	Dispose of a CScrollListDragger. This method clears the list's
  	reference to this task.
******************************************************************************/

void
CScrollListDragger::Dispose(void)

{
	if ( itsScrollList->itsLastTask == this)
		itsScrollList->itsLastTask = NULL;
	
	inherited::Dispose();

}  /* CScrollListDragger::Dispose */


/******************************************************************************
 BeginTracking  {OVERRIDE}

	Called at the start of mouse tracking.  This method selects the cell
	being dragged and calculates the rectangle to drag.
******************************************************************************/

void
CScrollListDragger::BeginTracking( LongPt *startPt)

{
	long	hExtent, vExtent;
	
			// Find and select the moving cell
	
	movingCell = itsScrollList->FindHitCell( startPt);
	if ( listOptions & kSLSelectable )
		itsScrollList->SelectCell( movingCell, kDoRedraw);
	
			// Calculate the dragging and clipping rectangles
	
	itsScrollList->GetCellRect( movingCell, &dragRect);
	OffsetLongRect( &dragRect, 0,
					startPt->v - ((dragRect.top + dragRect.bottom) >> 1));
	
	itsScrollList->GetPixelExtent( &hExtent, &vExtent);
	SetLongRect( &clipRect, dragRect.left, 0,
				 dragRect.right, vExtent);

}	/* CScrollListDragger::BeginTracking */


/******************************************************************************
 KeepTracking  {OVERRIDE}
 
 	Called continously while the mouse button is held down. This method
 	continously draws a gray rectangle and autoscrolls the table.
 	Only begins drawing after the mouse has moved.
******************************************************************************/

void
CScrollListDragger::KeepTracking( LongPt *currPt, LongPt *prevPt, LongPt *startPt)

{
	long		curTicks;
	Rect		qdRect;
	
	if ( !dragging )
	{
		long	vdist = currPt->v - startPt->v;
		
		if ( vdist >= DRAGTHRESH || vdist <= -DRAGTHRESH )
		{												// initiate dragging
			if ( !(listOptions & kSLSelectable) )
				itsScrollList->FrameCell( movingCell, TRUE);
			
			itsScrollList->FrameToQDR( &dragRect, &qdRect);
			PenPat( gray);
			PenMode( patXor);
			FrameRect( &qdRect);
			
			SetCursor( *gHandCursor);				// change to hand cursor
			dragging = TRUE;
		}
		else
			return;
	}
	
	PinInRect( &clipRect, prevPt);
	PinInRect( &clipRect, currPt);
	
	if ( currPt->v != prevPt->v )
	{
		itsScrollList->FrameToQDR( &dragRect, &qdRect);		// erase old rect
		PenPat( gray);
		PenMode( patXor);
		
		for ( curTicks = TickCount(); curTicks == TickCount(); )
			/* try to avoid flicker */ ;
		FrameRect( &qdRect);
		
		itsScrollList->AutoScroll( currPt);					// calculate new rect
		OffsetLongRect( &dragRect, 0, currPt->v - prevPt->v);
		
		itsScrollList->FrameToQDR( &dragRect, &qdRect);		// draw new rect
		PenPat( gray);
		PenMode( patXor);
		
		for ( curTicks = TickCount(); curTicks == TickCount(); )
			/* try to avoid flicker */ ;
		FrameRect( &qdRect);
	}

}	/* CScrollListDragger::KeepTracking */


/******************************************************************************
 EndTracking  {OVERRIDE}
 
 	Erase the dragging rectangle and move the cell to its new location.
 	Select the new cell position only if the list is selectable.
******************************************************************************/

void
CScrollListDragger::EndTracking( LongPt *currPt, LongPt *prevPt, LongPt *startPt)

{
	Rect		qdRect;
	short		endCell;
	
	if ( dragging )
	{
		itsScrollList->FrameToQDR( &dragRect, &qdRect);
		PenPat( gray);
		PenMode( patXor);
		FrameRect( &qdRect);
		
		endCell = itsScrollList->FindHitCell( currPt);
		if ( currPt->v - cellHeight * (endCell-1) >= cellHeight >> 1 )
			++endCell;
		
		if ( endCell > movingCell )
			--endCell;
		
		if ( endCell != movingCell )					// move cells
		{
			itsArray->MoveItemToIndex( movingCell, endCell);
			if ( listOptions & kSLSelectable )
				itsScrollList->SelectCell( endCell, kDoRedraw);
		}
		else if ( !(listOptions & kSLSelectable) )		// remove frame
			itsScrollList->FrameCell( movingCell, FALSE);
	}

}	/* CScrollListDragger::KeepTracking */
