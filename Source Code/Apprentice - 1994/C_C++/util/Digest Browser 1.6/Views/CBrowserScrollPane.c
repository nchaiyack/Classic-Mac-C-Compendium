#include "CBrowserScrollPane.h"

/******************************************************************************
 CBrowserScrollPane.c

	Methods to support resizeable scroll panes within a single window
	
  	Copyright © 1992 J. Robert Boonstra, II.  All rights reserved.
 ******************************************************************************/

#include "CScrollBar.h"
#include "CBrowserSizeBox.h"	
#include "CPanorama.h"

/******************************************************************************
 IBrowserScrollPane
 
 	differs from IScrollPane only in that it accepts the hasSICN argument
 	and uses a CBrowserSizeBox instead of a CSizeBox
 ******************************************************************************/

void	CBrowserScrollPane::IBrowserScrollPane(
	CView			*anEnclosure,
	CBureaucrat		*aSupervisor,
	short			aWidth,
	short			aHeight,
	short			aHEncl,
	short			aVEncl,
	SizingOption	aHSizing,
	SizingOption	aVSizing,
	Boolean			hasHoriz,
	Boolean			hasVert,
	Boolean			hasSizeBox,
	Boolean			hasSICN)
{	
	inherited::IScrollPane(anEnclosure, aSupervisor, aWidth, aHeight, aHEncl, aVEncl,
								aHSizing, aVSizing,
								hasHoriz, hasVert, hasSizeBox); 	// no size box, set up our own


	// If we want a size box, and want a SICN, then use our own size box
	// class.
	if (hasSizeBox && hasSICN) {
		if (itsSizeBox)				// release the old one
			itsSizeBox->Dispose();

		itsSizeBox = new(CBrowserSizeBox);
		itsSizeBox->ISizeBox(this, this);
	}

}


void	CBrowserScrollPane::Calibrate()
/******************************************************************************
 Calibrate
 
 	differs from CScrollPane::Calibrate only in that it adjusts the frame
 	position if necessary when the partition has been adjusted
 ******************************************************************************/
{
	long	hPos;
	long	vPos;
	if (itsPanorama == NULL)			/* Take a quick exit if no			*/
		return;							/*   Panorama is installed			*/

	itsPanorama->GetFramePosition(&hPos, &vPos);

	if (vPos < 0) {
		itsPanorama->Scroll( 0, -vPos, false);	// avoid redundant redraw
		vPos=0;
	}
		
	if (itsHorizSBar != NULL) {
		itsHorizSBar->SetValue((short) (hPos / hUnit));
	}
	
	if (itsVertSBar != NULL) {
		itsVertSBar->SetValue((short) (vPos / vUnit));
	}
}
