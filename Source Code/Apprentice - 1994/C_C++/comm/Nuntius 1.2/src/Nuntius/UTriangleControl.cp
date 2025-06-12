// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UTriangleControl.cp

#include "UTriangleControl.h"
#include "ViewTools.h"

#include <RsrcGlobals.h>

#pragma segment MyViewTools

TTriangleControl::TTriangleControl()
{
}

pascal void TTriangleControl::Initialize()
{
	inherited::Initialize();
	fIsExpanded = false;
}

void TTriangleControl::ITriangleControl(TView *superView, VPoint location, Boolean expanded)
{
	IControl(superView, location, VPoint(16, 16), sizeFixed, sizeFixed);
	fIsExpanded = expanded;
}

pascal void TTriangleControl::Free()
{
	inherited::Free();
}

pascal void TTriangleControl::Hilite()
{
	Draw(VRect());
}

pascal void TTriangleControl::Draw(const VRect& /* area */)
{
	short transformCode = (fHilite ? ttSelected : ttNone);
	short rsrcID = (fIsExpanded ? kExpandedDiamondSicn : kNonExpandedDiamondSicn);
	CRect r(0, 0, 16, 16);
	PlotIconID(r, 0, transformCode, rsrcID);
}

Boolean TTriangleControl::IsExpanded()
{
	return fIsExpanded;
}

void TTriangleControl::SetExpand(Boolean expand)
{
	if (fIsExpanded == expand)
		return;
	fIsExpanded = expand;
	Focus();
	ForceRedraw();
}

pascal void TTriangleControl::TrackMouse(TrackPhase aTrackPhase,
								   VPoint& /* anchorPoint */, VPoint& /* previousPoint */,
									 VPoint& nextPoint, Boolean /* mouseDidMove */)
{

	switch (aTrackPhase)
	{
		case trackBegin:
			HiliteState(true, kRedraw);
			break;

		case trackContinue:
			HiliteState(ContainsMouse(nextPoint), kRedraw);
			break;

		case trackEnd:
			HiliteState(false, kRedraw);
			if (ContainsMouse(nextPoint))
			{
				DoAnimation();
				fIsExpanded = !fIsExpanded;
				fSuperView->HandleEvent(mToggle, this, nil);
			}
			break;
	}
}


void TTriangleControl::DoAnimation()
{
	CRect r(0, 0, 16, 16);
	EraseRect(r);
	PlotIconID(r, 0, ttSelected, kHalfExpandedDiamondSicn);
	long dl;
	Delay(2, dl); // from hacking Finderª
	fIsExpanded = !fIsExpanded;
	short rsrcID = (fIsExpanded ? kExpandedDiamondSicn : kNonExpandedDiamondSicn);
	EraseRect(r);
	PlotIconID(r, 0, ttSelected, rsrcID);
}