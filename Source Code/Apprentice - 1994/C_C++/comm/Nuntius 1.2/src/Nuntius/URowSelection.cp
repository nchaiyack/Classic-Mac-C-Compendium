// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// URowSelection.cp

#include "URowSelection.h"

#pragma segment MyViewTools

TImprovedRowSelectCommand::TImprovedRowSelectCommand()
{
}

pascal void TImprovedRowSelectCommand::ComputeNewSelection(GridCell &clickedCell)
{
	clickedCell.h = fGridView->fNumOfCols;
	CRect r;
	if (fGridView->CanSelectCell(clickedCell))
	{
		if (fGridView->fSingleSelection)
			r = CRect(clickedCell.v, clickedCell.h, clickedCell.v + 1, clickedCell.h + 1);
		else
		{
			if (!fShiftKey && !fCommandKey && PtInRgn(clickedCell, fGridView->fSelections))
			{
				// keep the selection in case of double-click on selection
				CopyRgn(fPreviousSelection, fThisSelection);
				return;
			}
			else
			{
				Pt2Rect(fAnchorCell, clickedCell, r);
				++r.right;
				++r.bottom;
			}
		}
		RectRgn(fThisSelection, r);
		if (fCommandKey && !fGridView->fSingleSelection)
			if (fDeselecting)
				DiffRgn(fPreviousSelection, fThisSelection, fThisSelection);
			else
				UnionRgn(fPreviousSelection, fThisSelection, fThisSelection);
	}
}

pascal void TImprovedRowSelectCommand::IImprovedRowSelectCommand(TGridView* itsView,
						  const VPoint& itsMouse, Boolean theShiftKey, Boolean theCommandKey)
{
	inherited::IRowSelectCommand(itsView, itsMouse, theShiftKey, theCommandKey);
}
//-------------------------------------------------------------------

TStickySelectCommand::TStickySelectCommand()
{
}

void TStickySelectCommand::IStickySelectCommand(TGridView *gridView, 
										   const VPoint &itsMouse,
										   Boolean theShiftKey,
										   Boolean theCommandKey)
{
	fSpecialMethodActivated = false;
	inherited::ICellSelectCommand(gridView, itsMouse, theShiftKey, theCommandKey);
}

pascal void TStickySelectCommand::ComputeAnchorCell(GridCell &clickedCell)
{
	if (fSpecialMethodActivated)
		return;
	if (!fShiftKey && !fCommandKey)
	{
		fAnchorCell = clickedCell;
		fPreviousCell = fAnchorCell;
		fSpecialMethodActivated = true;
		if (!PtInRgn(clickedCell, fGridView->fSelections))
		{
			// if we click on a deselected cell, then deselect all other cells
			CRect r;
			Pt2Rect(fAnchorCell, fAnchorCell, r);
			++r.right;
			++r.bottom;
			RectRgn(fThisSelection, r);

			HighlightNewSelection();
			CopyRgn(fThisSelection, fPreviousSelection);
		}

		ComputeNewSelection(clickedCell);
		HighlightNewSelection();
		CopyRgn(fThisSelection, fPreviousSelection);
	}
	else
		inherited::ComputeAnchorCell(clickedCell);
}

pascal void TStickySelectCommand::ComputeNewSelection(GridCell &clickedCell)
{
	if (!fSpecialMethodActivated)
	{
		inherited::ComputeNewSelection(clickedCell);
		return;
	}
	CRect r;
	Pt2Rect(fAnchorCell, fPreviousCell, r);
	++r.right;
	++r.bottom;
	CTemporaryRegion tempRgn;
	RectRgn(tempRgn, r);
	DiffRgn(fPreviousSelection, tempRgn, tempRgn); // c = a & ~b

	Pt2Rect(fAnchorCell, clickedCell, r);
	++r.right;
	++r.bottom;
	RectRgn(fThisSelection, r);
	UnionRgn(tempRgn, fThisSelection, fThisSelection); // c = a | b
}
