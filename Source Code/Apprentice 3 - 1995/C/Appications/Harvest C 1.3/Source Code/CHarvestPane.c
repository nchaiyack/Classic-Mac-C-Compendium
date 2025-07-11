/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/****
 * CHarvestPane.c
 *
 *	Pane methods for a typical application.
 *
 *  Copyright � 1990 Symantec Corporation.  All rights reserved.
 *
 ****/
 
/**
 *
 *  Most applications will want a scrollable window, so this
 *  class is based on the class CPanorama. All the methods here
 *  would still apply to classes based directly on CPane.
 *
 **/

#include "CHarvestPane.h"
#include "CSourceFile.h"
#include "CFile.h"
#include "CDataFile.h"
#include "CList.h"
#include "CHarvestDoc.h"

extern RgnHandle gUtilRgn;				// a global utility region

void CHarvestPane::IHarvestPane(CScrollPane *anEnclosure, CBureaucrat *aSupervisor)
{
		// call CTable's initialization method. Note that we set the
		// size to zero, because we are going to fit it to its enclosure

	CTable::ITable( (CView *) anEnclosure, aSupervisor, 0, 0, 0, 0,
					sizELASTIC, sizELASTIC);
					
		// Fit it to its enclosing view, which is the CTableScroller
					
	FitToEnclosure( TRUE, TRUE);
	
		// Specify gray row and column borders for the cells
	
	SetRowBorders( 1, patCopy, gray);
	SetColBorders( 1, patCopy, gray);
	
	SetDefaults(160,20);

		// set selection behavior so dragging selects rectangular areas
	
	SetSelectionFlags( selOnlyOne);
		
		
}

void CHarvestPane::GetCellText( Cell aCell, short availableWidth,
			StringPtr itsText)
{
	CHarvestDoc *myDoc;
	CSourceFile *aFile;
	myDoc = (CHarvestDoc *) itsSupervisor;
	aFile = (CSourceFile *) (myDoc->itsSourceFiles->NthItem(aCell.v+1));
	aFile->theFile->GetName(itsText);
}

/******************************************************************************
 Draw {OVERRIDE}
 
 	Normally you don't need to override CTable::Draw - you override
 	DrawCell or GetCellText instead. Here we do it in order to fill
 	the area outside the cells with a gray pattern.
 	
******************************************************************************/

void CHarvestPane::Draw( Rect *area)
{
	LongRect	r;
	Rect		qdRect;
	RgnHandle	tmpRgn;
	
		// Get a region consisting of the area to be drawn
		// it is already in QuickDraw coordinates.
		
	RectRgn( gUtilRgn, area);
	
		// Get the intersection of the bounds and frame rects
		// This corresponds to the cells that are visible.
	
	SectLongRect( &bounds, &frame, &r);
	
		// Since that rect is in Frame coordinates, convert it to
		// a rect in QuickDraw coordinates
		
	FrameToQDR( &r, &qdRect);
	
		// Get a region consisting of the above rect
	
	tmpRgn = NewRgn();
	RectRgn( tmpRgn, &qdRect);
	
		// Subtract the visible cell area from the area to be drawn.
		// The region remaining gives use the area to be drawn outside
		// the bounds.
	
	DiffRgn( gUtilRgn, tmpRgn, tmpRgn);
	
		// If it ain't empty, fill it. Then dispose of the temporary region
	
	if (!EmptyRgn( tmpRgn))
		FillRgn( tmpRgn, gray);
	
	DisposeRgn( tmpRgn);
	
		// inherited::Draw draws the cells
		
	inherited::Draw( area);
	
}
