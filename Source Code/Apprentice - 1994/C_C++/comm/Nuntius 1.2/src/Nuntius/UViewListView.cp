// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UViewListView.cp

#include "UViewListView.h"

#pragma segment MyViewTools

#define qDebugUpdateViews qDebug & 0
#define qDebugScrollBits qDebug & 0
#define qDebugRebuildFrames qDebug & 0
#define qDebugAOSV qDebug & 0


TViewListView::TViewListView()
{
}

pascal void TViewListView::Initialize()
{
	inherited::Initialize();
	fIsAdjustingSubViewFrames = false;
	fAddSeparatorAtBottom = false;
}

pascal void TViewListView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TViewListView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
}

pascal void TViewListView::Free()
{
	inherited::Free();
}

pascal void TViewListView::Draw(const VRect& area)
{
	inherited::Draw(area);
	VCoordinate offset = 0;
	Boolean gotOne = false;
	// block for failure handling
	{
		CSubViewIterator iter(this);
		for (TView *subView = iter.FirstSubView(); iter.More(); subView = iter.NextSubView())
		{
			if (gotOne)
			{
				if (area.Contains(VPoint(area.left, offset)))
					DrawViewSeparator(offset);
				else if (area.Contains(VPoint(area.left, offset + GetSeparatorHeight() - 1)))
					DrawViewSeparator(offset);				
			}
			gotOne = true;
			VRect frame;
			subView->GetFrame(frame);
			offset = frame.bottom;
		}
	}
	if (fAddSeparatorAtBottom)
	{
		if (area.Contains(VPoint(area.left, offset)))
			DrawViewSeparator(offset);
		else if (area.Contains(VPoint(area.left, offset + GetSeparatorHeight() - 1)))
			DrawViewSeparator(offset);
	}
}

pascal void TViewListView::SubViewChangedFrame(TView* theSubView,
								const VRect& oldFrame, const VRect& newFrame, Boolean invalidate)
{
	inherited::SubViewChangedFrame(theSubView, oldFrame, newFrame, invalidate);
	if (fIsAdjustingSubViewFrames)
	{
#if qDebugRebuildFrames
		fprintf(stderr, "TViewListView::SubViewChangedFrame called while fIsAdjustingSubViewFrames == true\n");
#endif
	return;
	}
	UpdateSubViewFrames(theSubView, oldFrame, newFrame);
//	RebuildSubViewFrames();
}

void TViewListView::UpdateSubViewFrames(TView* theSubView,
								const VRect& oldFrame, const VRect& newFrame)
{
	fIsAdjustingSubViewFrames = true;

	VRect fr;
	VRect myFrame(0, 0, 0, 0);
	VRect myOldFrame;
	GetFrame(myOldFrame);
	CSubViewIterator iter(this);
	for (TView *view = iter.FirstSubView(); iter.More(); view = iter.NextSubView())
	{
		view->GetFrame(fr);
		myFrame = myFrame | fr;
		if (view == theSubView)
			break;
	}
	if (!view)
		Failure(minErr, 0);
	theSubView->GetFrame(fr);
	myFrame = myFrame | fr;	
	VCoordinate scrollOffset = (newFrame.bottom - newFrame.top) - (oldFrame.bottom - oldFrame.top);
#if qDebugUpdateViews
	VCoordinate offset = newFrame.top - oldFrame.top;
	fprintf(stderr, "ChangeView: %s -> ", (char*)oldFrame);
	fprintf(stderr, "%s, � = %ld, scroll = %ld\n", (char*)newFrame, offset, scrollOffset);
#endif
	VCoordinate vOffset = newFrame.bottom;
	// update rest of views:
	for (view = iter.NextSubView(); iter.More(); view = iter.NextSubView())
	{
		vOffset += GetSeparatorHeight();
		view->GetFrame(fr);
		VRect newFr(fr);
		newFr.top = vOffset;
		newFr.bottom = newFr.top + (fr.bottom - fr.top);
		view->SetFrame(newFr, !kRedraw);
		myFrame = myFrame | newFr;
#if qDebugUpdateViews
		fprintf(stderr, "Adjusted subview: %s -> ", (char*)fr);
		fprintf(stderr, "%s\n", (char*)newFr);
#endif
		vOffset += (fr.bottom - fr.top);
	}
	if (fAddSeparatorAtBottom)
		myFrame.bottom += GetSeparatorHeight();
	CheckNewFrame(myFrame);	
	if (myFrame.bottom > myOldFrame.bottom)
		SetFrame(myFrame, !kRedraw);
	DoScrollBits(oldFrame.bottom, scrollOffset);
	if (myFrame.bottom < myOldFrame.bottom)
		SetFrame(myFrame, kRedraw); //@@ !kRedraw -> kRedraw
#if qDebugUpdateViews
	fprintf(stderr, "MyNewFrame = %s, ", (char*)myFrame);
	fprintf(stderr, "myOldFrame = %s\n", (char*)myOldFrame);
#endif
	if (newFrame.top >= myOldFrame.bottom)
	{
		// new view: invalidate separator
		VRect vr(myOldFrame.left, myOldFrame.bottom - GetSeparatorHeight(), myOldFrame.right, newFrame.top);
		Focus();
		InvalidateVRect(vr);
#if qDebugUpdateViews
		fprintf(stderr, "Invalidates %s: got new subView\n", (char*)vr);
#endif
		if (fAddSeparatorAtBottom)
		{
			vr = myFrame;
			vr.top = newFrame.top - GetSeparatorHeight();
			Focus();
			InvalidateVRect(vr);
#if qDebugUpdateViews
			fprintf(stderr, "Invalidates %s separator at bottom: got new subView\n", (char*)vr);
#endif
		}
	}
	VCoordinate grow = (newFrame.bottom - newFrame.top) - (oldFrame.bottom - oldFrame.top);
	if (grow > 0)
	{
		// view is grown bigger, invalidate new part
		VRect vr(newFrame);
		vr.top = vr.top + (oldFrame.bottom - oldFrame.top);
		Focus();
		InvalidateVRect(vr);
#if qDebugUpdateViews
		fprintf(stderr, "Invalidates %s as theSubView is grown bigger\n", (char*)vr);
#endif
	}
#if 0
	if (myFrame.bottom < myOldFrame.bottom)
	{
		// view is smaller, update non-used part of TViewListView
		VRect vr(myOldFrame);
		vr.top = myFrame.bottom;
		Focus();
		InvalidateVRect(vr);
#if qDebugUpdateViews
		fprintf(stderr, "Invalidates %s as myFrame is grown smaller\n", (char*)vr);
#endif
	}
#endif
#if qDebugUpdateViews
	fprintf(stderr, "\n");
#endif
	fIsAdjustingSubViewFrames = false;
}

void TViewListView::DoScrollBits(VCoordinate vFrom, VCoordinate offset)
{
	if (!offset)
		return;
	Focus();
	VRect visible;
	GetVisibleRect(visible);
	VRect moveFromVRect, moveToVRect;
	GetFrame(moveFromVRect);
	moveFromVRect.top = vFrom;
	moveToVRect = moveFromVRect;
	moveToVRect.top += offset;
	moveToVRect.bottom += offset;
	if (visible.bottom < moveFromVRect.top)
	{
#if qDebugScrollBits
		fprintf(stderr, "Scroll: �vis: %s -> ", (char*)moveFromVRect);
		fprintf(stderr, "%s, ", (char*)moveToVRect);
		fprintf(stderr, "vis = %s\n", (char*)visible);
#endif
		Focus();
		InvalidateVRect(moveToVRect);
		return;
	}
	VRect visibleMoveRect = (moveFromVRect | moveToVRect) & visible;
#if qDebugScrollBits
	fprintf(stderr, "Scroll: %s -> ", (char*)moveFromVRect);
	fprintf(stderr, "%s, ", (char*)moveToVRect);
	fprintf(stderr, "move = %s\n", (char*)visibleMoveRect);
#endif
#if qDebugScrollBits
	if (visibleMoveRect.Empty())
		fprintf(stderr, "Scroll: visibleMoveRect was empty, should not have been it\n");
#endif
/*
	if (offset > 0)
	{
		VRect myNewFrame(moveToVRect);
		myNewFrame.top = 0;
		SetFrame(myNewFrame, !kRedraw);
	}
*/
	CRect qdMoveRect;
	ViewToQDRect(visibleMoveRect, qdMoveRect);
#if qDebugScrollBits
	fprintf(stderr, "Scroll: qdMoveRect = %s\n", (char*)qdMoveRect);
#endif
	CPoint delta(0, short(offset));
#if qDebugScrollBits
	fprintf(stderr, "Scroll: delta = %s\n", (char*)delta);
#endif
	if (delta.v)
	{
		Focus();
		CTemporaryRegion updateRgn;
		ScrollRect(qdMoveRect, delta.h, delta.v, updateRgn);
		InvalidateRegion(updateRgn);
//qdMoveRect.right = 10;
//FillRect(qdMoveRect, qd.ltGray);
	}
}

void TViewListView::RebuildSubViewFrames()
{
	if (!fSubViews || !fSubViews->GetSize())
	{
		SetFrame(VRect(0, 0, 0, 0), kRedraw);
		return;
	}
	FailInfo fi;
	if (fi.Try())
	{	
		fIsAdjustingSubViewFrames = true;
		VCoordinate curHeight = 0;
		Boolean gotOne = false;
		VRect myFrame(0, 0, 0, 0);
		VRect myOldFrame;
		GetFrame(myOldFrame);
#if qDebugRebuildFrames
		fprintf(stderr, "TViewListView, begins update of subview frames, fAddSeparatorAtBottom = %hd\n", fAddSeparatorAtBottom);
#endif
		// block for failure handling
		{
			ArrayIndex maxIndex = fSubViews->GetSize();
			for (ArrayIndex index = 1; index <= maxIndex; index++)
			{
				TView *subView = (TView*)fSubViews->At(index);
				if (gotOne)
					curHeight += GetSeparatorHeight();
				gotOne = true;
				VRect oldFrame, newFrame;
				subView->GetFrame(oldFrame);
				VCoordinate offset = curHeight - oldFrame.top;
				newFrame = oldFrame;
				newFrame.top += offset;
				newFrame.bottom += offset;
#if qDebugRebuildFrames
				fprintf(stderr, "  move: %s -> ", (char*)oldFrame);
				fprintf(stderr, "%s, ", (char*)newFrame);
				fprintf(stderr, "� = %ld, curHeight = %ld\n", offset, curHeight);
#endif
				myFrame = myFrame | newFrame;
				VRect myCurrentFrame;
				GetFrame(myCurrentFrame);
				if (myFrame.bottom > myCurrentFrame.bottom)
					SetFrame(myFrame, !kRedraw);
				subView->SetFrame(newFrame, !kRedraw);
				VCoordinate grow = (newFrame.bottom - newFrame.top) - (oldFrame.bottom - oldFrame.top);

				if (offset)
				{
					VCoordinate top = oldFrame.top;
					if (top > 0) // if not first view, include separator
						top -= GetSeparatorHeight();
					DoScrollBits(top, offset);

					VCoordinate vAdjust = curHeight + (newFrame.bottom - newFrame.top);
					CSubViewIterator adjustIter(this, index + 1, kMaxLong, kIterateForward);
					for (TView *adjustView = adjustIter.FirstSubView(); adjustIter.More(); adjustView = adjustIter.NextSubView())
					{
						vAdjust += GetSeparatorHeight();
						VRect fr;
						adjustView->GetFrame(fr);
						VCoordinate adjustOffset = vAdjust - fr.top;
						VRect newFr(fr);
						newFr.top += adjustOffset;
						newFr.bottom += adjustOffset;
						adjustView->SetFrame(newFr, !kRedraw);
#if qDebugRebuildFrames
						fprintf(stderr, "  ScrollAdjust: %s -> ", (char*)fr);
						fprintf(stderr, "%s, � = %ld\n", (char*)newFr, adjustOffset);
#endif
						vAdjust += (fr.bottom - fr.top);
					}
				}
				else if (index == maxIndex && grow > 0)
				{
					// view is grown bigger, invalidate new part
					VRect vr(newFrame);
					vr.top += (oldFrame.bottom - oldFrame.top);
					Focus();
					InvalidateVRect(vr);
#if qDebug
					fprintf(stderr, "  LastView is grown bigger, %s is invalidated\n", (char*)vr);
#endif
				}
				curHeight += newFrame.bottom - newFrame.top;
			}
		}
		if (fAddSeparatorAtBottom)
			myFrame.bottom += GetSeparatorHeight();
		CheckNewFrame(myFrame);	
#if qDebugRebuildFrames
		fprintf(stderr, "end of update, myFrame = %s\n", (char*)myFrame);
		fprintf(stderr, "\n");
#endif
		SetFrame(myFrame, kRedraw);
		fIsAdjustingSubViewFrames = false;
		fi.Success();
	}
	else // fail
	{
		fIsAdjustingSubViewFrames = false;
		fi.ReSignal();
	}
}

void TViewListView::AdjustOneSubView(TView *view, VRect oldFrame, VRect newFrame, VCoordinate offset)
{
	Focus();
	VRect visible;
	GetVisibleRect(visible);
	if (visible.bottom < oldFrame.top || visible.top > oldFrame.bottom)
	{
#if qDebugAOSV
		fprintf(stderr, "AOSV: �vis: %s -> ", (char*)oldFrame);
		fprintf(stderr, "%s, ", (char*)newFrame);
		fprintf(stderr, "vis = %s\n", (char*)visible);
#endif
		view->SetFrame(newFrame, !kRedraw);
		return;
	}
	VRect moveFromVRect(oldFrame[topLeft], fSize);
	if (moveFromVRect.top > 0) // if not first view, include separator
		moveFromVRect.top -= GetSeparatorHeight();
	VRect moveToVRect(moveFromVRect);
	moveToVRect.top += offset;
	moveToVRect.bottom += offset;
	VRect visibleMoveRect = (moveFromVRect | moveToVRect) & visible;
#if qDebugAOSV
	fprintf(stderr, "AOSV: %s -> ", (char*)moveFromVRect);
	fprintf(stderr, "%s, ", (char*)moveToVRect);
	fprintf(stderr, "move = %s\n", (char*)visibleMoveRect);
#endif
#if qDebugAOSV
	if (visibleMoveRect.Empty())
		fprintf(stderr, "AOSV: visibleMoveRect was empty, should not have been it\n");
#endif
	CRect qdMoveRect;
	ViewToQDRect(visibleMoveRect, qdMoveRect);
#if qDebugAOSV
	fprintf(stderr, "AOSV: qdMoveRect = %s\n", (char*)qdMoveRect);
#endif
	CPoint delta(0, short(offset));
#if qDebugAOSV
	fprintf(stderr, "AOSV: delta = %s\n", (char*)delta);
#endif
	if (delta.v)
	{
		Focus();
		CTemporaryRegion updateRgn;
		ScrollRect(qdMoveRect, delta.h, delta.v, updateRgn);
		InvalidateRegion(updateRgn);
	}
	view->SetFrame(newFrame, !kRedraw);
}

VCoordinate TViewListView::GetSeparatorHeight()
{
	SubClassResponsibility();
	return 0;
}

void TViewListView::CheckNewFrame(VRect & /* newFrame */)
{
}

void TViewListView::DrawViewSeparator(VCoordinate /* vOffset */)
{
}
