// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ViewTools.h

#include "ViewTools.h"
#include "Tools.h"

#pragma segment MyViewTools

#define qDebugDDR qDebug & 0

void DirectDeltaRows(TGridView *gv, 
		short beforeRow, short numOfRows, short aHeight)
{
#if qDebugDDR
	fprintf(stderr, "DDR: TGridView at $%lx, fNumOfRows = %hd\n", long(gv), gv->fNumOfRows);
	fprintf(stderr, "-    beforeRow = %hd, numOfRows = %ld, aHeight = %hd\n", beforeRow, long(numOfRows), aHeight);
#endif
	if (!numOfRows)
		return;
	gv->Update();
	gv->Focus();
	if (beforeRow > gv->fNumOfRows)
	{
#if qDebugDDR
		fprintf(stderr, "beforeRow > gv->fNumOfRows\n");
#endif
		if (numOfRows > 0)
			gv->InsRowBefore(beforeRow, numOfRows, aHeight);
#if qDebug
			ProgramBreak("beforeRow > fNumOfRows && numOfRows < 0");
#endif
		return;
	}
	short cFromRow, cToRow, cNoMoveRows;
	if (numOfRows > 0)
	{
		cFromRow = beforeRow;
		cToRow   = beforeRow + numOfRows;
		cNoMoveRows = gv->fNumOfRows - beforeRow + 1;
	}
	else
	{
		cToRow   = beforeRow;
		cFromRow = beforeRow - numOfRows;
		cNoMoveRows = gv->fNumOfRows - beforeRow + numOfRows + 1;
	}
#if qDebugDDR
	fprintf(stderr, "DDR: cFromRow = %hd, cToRow = %hd, cNoMoveRows = %hd\n", cFromRow, cToRow, cNoMoveRows);
#endif
	if (!cNoMoveRows)
	{
#if qDebugDDR
		fprintf(stderr, "DDR: no rows to ScrollRect\n");
#endif
		if (numOfRows > 0)
			gv->InsRowBefore(beforeRow, numOfRows, aHeight);
		else
		{
			CRect r(1, beforeRow, gv->fNumOfCols, beforeRow-numOfRows-1);
#if qDebugDDR
			fprintf(stderr, "Clears selection for %s\n", (char*)r);
#endif
			gv->SetSelectionRect(r, true, true, false);
			gv->DelRowAt(beforeRow, -numOfRows);
		}
		return;
	}
#if qDebugDDR
	fprintf(stderr, "DDR: Old view size: %s\n", (char*)gv->fSize.Copy());
#endif
	if (numOfRows > 0)
	{
		VPoint oldSize(0, gv->fSize.v);
		gv->fRowHeights->InsertItems(beforeRow, numOfRows, aHeight);
		gv->fNumOfRows += numOfRows;
		gv->AdjustFrame();
		gv->Focus();
		VRect validRect(oldSize, gv->fSize);
		gv->ValidateVRect(validRect);
		gv->InvalidateFocus(); // as updateRgn has changed
		gv->Focus();
#if qDebugDDR
		fprintf(stderr, "DDR: New view size: %s\n", (char*)gv->fSize.Copy());
#endif
	}
	VRect moveFromVRect, moveToVRect;
	gv->RowToVRect(cFromRow, cNoMoveRows, moveFromVRect);
	gv->RowToVRect(cToRow,   cNoMoveRows,   moveToVRect);
#if qDebugDDR
	fprintf(stderr, "DDR: moveFromVRect = %s\n", (char*) moveFromVRect);
	fprintf(stderr, "DDR: moveToVRect   = %s\n", (char*) moveToVRect);
#endif
	VRect beforeRowRect;
	gv->RowToVRect(beforeRow, 1, beforeRowRect);
#if qDebugDDR
	fprintf(stderr, "DDR: beforeRowRect = %s\n", (char*)beforeRowRect);
#endif
	VRect visible;
	gv->GetVisibleRect(visible);
#if qDebugDDR
	fprintf(stderr, "DDR: visible = %s\n", (char*) visible);
#endif
	if (visible.bottom < beforeRowRect.top)
	{
#if qDebugDDR
		fprintf(stderr, "beforeRowRect is not visible\n");
#endif
	}
	else
	{
		VRect visibleMoveRect;
		visibleMoveRect = (moveFromVRect | moveToVRect) & visible;
#if qDebugDDR
		fprintf(stderr, "DDR: visibleMoveRect = %s\n", (char*)visibleMoveRect);
#endif
#if qDebugDDR
		if (visibleMoveRect.Empty())
			fprintf(stderr, "DDR: visibleMoveRect was empty, should not have been it\n");
#endif
		CRect qdMoveRect;
		gv->ViewToQDRect(visibleMoveRect, qdMoveRect);
#if qDebugDDR
		fprintf(stderr, "DDR: qdMoveRect = %s\n", (char*)qdMoveRect);
#endif
		CPoint delta;
		if (numOfRows > 0)
			delta = CPoint(0, numOfRows * aHeight);
		else
		{
			VRect vr;
			gv->RowToVRect(beforeRow, -numOfRows, vr);
			delta = CPoint(0, -short(vr.GetLength(vSel)));
		}
#if qDebugDDR
		fprintf(stderr, "DDR: delta = %s\n", (char*)delta);
#endif
		if (delta.v)
		{
			CTemporaryRegion updateRgn;
			ScrollRect(qdMoveRect, delta.h, delta.v, updateRgn);
			gv->InvalidateRegion(updateRgn);
		}
	}
	if (gv->IsAnyCellSelected())
	{
		CTemporaryRegion moveRgn;
		CopyRgn(gv->fSelections, moveRgn);
		CTemporaryRegion tmpRgn;
		SetRectRgn(tmpRgn, 1, cFromRow, gv->fNumOfCols + 1, cFromRow + cNoMoveRows);
		SectRgn(tmpRgn, moveRgn, moveRgn);
		OffsetRgn(moveRgn, 0, numOfRows);

		SetRectRgn(tmpRgn, 1, 1, gv->fNumOfCols + 1, beforeRow);
		SectRgn(tmpRgn, gv->fSelections, gv->fSelections);
		
		UnionRgn(moveRgn, gv->fSelections, gv->fSelections);

		CopyRgn(gv->fHLRegion, moveRgn);
		SetRectRgn(tmpRgn, 1, cFromRow, gv->fNumOfCols + 1, cFromRow + cNoMoveRows);
		SectRgn(tmpRgn, moveRgn, moveRgn);
		OffsetRgn(moveRgn, 0, numOfRows);

		SetRectRgn(tmpRgn, 1, 1, gv->fNumOfCols + 1, beforeRow);
		SectRgn(tmpRgn, gv->fHLRegion, gv->fHLRegion);
		
		UnionRgn(moveRgn, gv->fHLRegion, gv->fHLRegion);
	}

	if (numOfRows < 0)
	{
		gv->fRowHeights->DeleteItems(beforeRow, -numOfRows);
		gv->fNumOfRows += numOfRows;
		gv->AdjustFrame();
		gv->Focus();
#if qDebugDDR
		fprintf(stderr, "DDR: New view size: %s\n", (char*)gv->fSize.Copy());
#endif
	}
#if qDebugDDR
	fprintf(stderr, "DDR: done\n");
#endif
}

//-----------------------------
Boolean DoGridViewKey(TGridView *gv, char ch)
{
	GridCell aCell;
	switch (ch)
	{
		case chUp:
			if (gv->IsAnyCellSelected())
			{
				aCell = gv->FirstSelectedCell();
				aCell.v--;
			}
			else
				aCell = GridCell(1, gv->fNumOfRows);
			break;

		case chDown:
			if (gv->IsAnyCellSelected())
			{
				aCell = gv->LastSelectedCell();
				aCell.v++;
			}
			else
				aCell = GridCell(1, 1);
			break;
	
		case chLeft:
			if (gv->IsAnyCellSelected())
			{
				aCell = gv->FirstSelectedCell();
				aCell.h--;
			}
			else
				aCell = GridCell(gv->fNumOfCols, 1);
			break;

		case chRight:
			if (gv->IsAnyCellSelected())
			{
				aCell = gv->LastSelectedCell();
				aCell.h++;
			}
			else
				aCell = GridCell(1, 1);
			break;
			
		default:
			return false;
	}	
	if (aCell.h < 1 || aCell.h > gv->fNumOfCols)
		return true;
	if (aCell.v < 1 || aCell.v > gv->fNumOfRows)
		return true;
	gv->SelectCell(aCell, false, true, true);
	gv->ScrollSelectionIntoView(kRedraw);
	gv->Focus();
	gv->Update();
	return true;
}

IDType MyPoseModally(TWindow *&window)
{
	FailInfo fi;
	if (fi.Try())
	{
		IDType id = window->PoseModally();
		fi.Success();
		return id;
	}
	else // fail
	{
		window = nil; // TWindow::PoseModally frees the window
		fi.ReSignal();
	}
}

//========================================================================
class TDoEventActionBehavior : public TBehavior
{
public:
	TDoEventActionBehavior();
	virtual pascal void DoEvent(EventNumber eventNumber, TEventHandler* source, TEvent* event);
	PlainDoEvent fFunc;
	void *fUserItem;
};

TDoEventActionBehavior::TDoEventActionBehavior()
{
}

pascal void TDoEventActionBehavior::DoEvent(EventNumber eventNumber, TEventHandler* source, TEvent* event)
{
#if qDebug
	if (source)
		fprintf(stderr, "DDDD: eventNumber = %ld, id = %s\n", eventNumber, OSType2String(source->fIdentifier));
#endif
	inherited::DoEvent(eventNumber, source, event);
	TView *view = (TView*)fOwner;
	FailNonObject(view);
	if (mCheckBoxHit == 4)
		fFunc(fUserItem, view, eventNumber, source, event);
}

void AddActionBehaviour(TView *view, PlainDoEvent func, void *useritem)
{
	FailNonObject(view);
	TDoEventActionBehavior *be = new TDoEventActionBehavior();
	be->IBehavior('AcBe');
	be->fFunc = func;
	be->fUserItem = useritem;
	view->AddBehavior(be);
}
