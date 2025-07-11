/******************************************************************************
 CTableScroller.c

		A subclass of CScrollPane that assists in providing synchronized
		scrolling of a table and its row and column labels. The other classes
		involved are CDemoTable and CTableLabels.
		
	SUPERCLASS = CScrollPane
	
	Copyright � 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CTableScroller.h"
#include "CTableLabels.h"
#include "CWindow.h"
#include "Global.h"


extern RgnHandle	gUtilRgn;

/******************************************************************************
 ITableScroller
 
 	Initialization method for CTableScroller.
******************************************************************************/

void CTableScroller::ITableScroller( CView *anEnclosure, CBureaucrat *aSupervisor,
				short aWidth, short	aHeight, short aHEncl, short aVEncl,
				SizingOption aHSizing, SizingOption aVSizing,
				Boolean	hasHoriz, Boolean hasVert, Boolean hasSizeBox)
{
	itsRowLabels = itsColLabels = NULL;

	CScrollPane::IScrollPane( anEnclosure, aSupervisor, aWidth, aHeight,
					aHEncl, aVEncl, aHSizing, aVSizing,
					hasHoriz, hasVert, hasSizeBox);
					
}

/******************************************************************************
 SetLabels
 
 	Specify the row and column label panes.
******************************************************************************/

void CTableScroller::SetLabels( CTableLabels *rowLabels, CTableLabels *colLabels)
{
	itsRowLabels = rowLabels;
	itsColLabels = colLabels;
}

/******************************************************************************
 ScrollBits
 
 	This method is called to scroll the necessary pixels in the 
 	table and labels. Performing the scroll here allows us to do a single 
 	ScrollRect for each pane rather than one for the table and then another
 	for the row/column labels. The visual appearance is much better.
******************************************************************************/

void CTableScroller::ScrollBits( long hDelta, long vDelta)
{
	Rect qdRect;
	LongRect panAperture;
			
		// determine the portion of the table to scroll in the
		// Frame coordinates of this pane.
		
	itsPanorama->Prepare();
	itsPanorama->GetAperture( &panAperture);
	itsPanorama->FrameToEnclR( &panAperture);
	
		// get the rect to scroll in QuickDraw coordinates
		
	Prepare();
	FrameToQDR( &panAperture, &qdRect);
	
		// If any horizontal scrolling is needed, then we must include
		// the column labels in the scroll rectangle.
		
	if (hDelta)
		qdRect.top = 0;

		// If any vertical scrolling is needed, then we must include
		// the row labels in the scroll rectangle.

	if (vDelta)
		qdRect.left = 0;

		// Check if any pixels actually need to be scrolled, if not
		// just do a refresh. Also check if we're doing a simultaneous
		// horizontal and vertical scroll. In that case we can't scroll
		// bits, we must do a refresh and redraw.
		
	if ((Abs(hDelta) < itsPanorama->width) && (Abs(vDelta) < itsPanorama->height) &&
		(!hDelta || !vDelta))
	{		
		ScrollRect( &qdRect, -hDelta, -vDelta, gUtilRgn);
		InvalRgn(gUtilRgn);
	}
	else
	{
		RefreshRect( &qdRect);
	}
}
	
/******************************************************************************
 DoScroll {OVERRIDE}
 
 	Override CScrollPane::DoScroll to provide synchronized scrolling.
 	
******************************************************************************/

void CTableScroller::DoScroll( long hDelta, long vDelta)
{
	ScrollBits( hDelta, vDelta);
	itsPanorama->Scroll( hDelta, vDelta, kNoRedraw);
	GetWindow()->Update();
}
