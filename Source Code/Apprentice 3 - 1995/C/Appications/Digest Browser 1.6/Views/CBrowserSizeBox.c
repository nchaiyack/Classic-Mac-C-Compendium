/******************************************************************************
 CBrowserSizeBox.c

	Methods to support a size box associated with a pane instead of a window.
	Needed to implement resizeable scroll panes within a single window
	
  	Copyright © 1992 J. Robert Boonstra, II.  All rights reserved.
 ******************************************************************************/

// CBrowserDoc is needed so we can adjust both parent scroll panes
#include "CBrowserDoc.h"	
#include "CWindow.h"		
#include "CBrowserSizeBox.h"	
#include "TBUtilities.h"


/******************************************************************************
 Draw {OVERRIDE}

		Draw a SizeBox
 ******************************************************************************/

void CBrowserSizeBox::Draw(Rect *area)
{
	Point		pt;

	if (IsActive()) {		// if it is active, then we draw the icon
		LongToQDPt( &topLeftL( frame), &pt);
		DrawSICN(201, 1, pt);	// uses a different icon
	}
	else					// otherwise, let the inherited method draw
		inherited::Draw(area);
}

/******************************************************************************
 DoClick
 ******************************************************************************/

void	CBrowserSizeBox::DoClick(
	Point		hitPt,
	short		modifierKeys,
	long		when)
{
LongPt			frameHitPt;
Rect			theDragBox;
LongRect		theLongDragBox;
CBrowserDoc	*theDoc;

	QDToFrame(hitPt,&frameHitPt);
	FrameToWind(&frameHitPt,&hitPt);
	
	GetFrame(&theLongDragBox);
	FrameToWindR(&theLongDragBox,&theDragBox);
	theDoc = (CBrowserDoc *)(GetWindow()->itsSupervisor);
//	tell the supervisor to adjust the pane partitions	
	theDoc->AdjustPaneDivider(hitPt,&theDragBox);
}						
