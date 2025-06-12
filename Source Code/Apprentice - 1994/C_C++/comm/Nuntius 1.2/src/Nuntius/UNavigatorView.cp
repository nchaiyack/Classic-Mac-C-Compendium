// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNavigatorView.cp

#include "UNavigatorView.h"
#include "UFatalError.h"

#include <RsrcGlobals.h>

#pragma segment MyViewTools


TNavigatorView::TNavigatorView()
{
}


pascal void TNavigatorView::Initialize()
{
	inherited::Initialize();
}

pascal void TNavigatorView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
}

pascal void TNavigatorView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
}

pascal void TNavigatorView::Free()
{
	inherited::Free();
}

pascal void TNavigatorView::Draw(const VRect& /* area */)
{
	PenNormal();
	MoveTo(0, 0);
	Line(short(fSize.h), 0);
}

pascal void TNavigatorView::ComputeFrame(VRect &newFrame)
{
	newFrame += VPoint(0, fSuperView->fSize.v - newFrame.bottom);
}

//============================================================
TNavigatorArrowView::TNavigatorArrowView()
{
}


pascal void TNavigatorArrowView::Initialize()
{
	inherited::Initialize();
	InitializeSicnFastBits(fArrowFastBits[kNormalArrow]);
	InitializeSicnFastBits(fArrowFastBits[kDimmedArrow]);
	InitializeSicnFastBits(fArrowFastBits[kHighlightArrow]);
	fPortBitsP = nil;
	fIsActive = true;
}

pascal void TNavigatorArrowView::ReadFields(TStream *aStream)
{
	inherited::ReadFields(aStream);
#if 0
	fHitCommandNumber = aStream->ReadInteger();
	fRsrcID = aStream->ReadInteger();
#else
	switch (fIdentifier)
	{
		case kNavigatorLeftArrowView: 
			fHitCommandNumber = cPreviousDiscussion; 
			fRsrcID = kNavigatorLeftArrow; 
			break;

		case kNavigatorSpaceArrowView: 
			fHitCommandNumber = cMultiSuperNextKey; 
			fRsrcID = kNavigatorSpaceArrow; 
			break;

		case kNavigatorRightArrowView: 
			fHitCommandNumber = cNextDiscussion; 
			fRsrcID = kNavigatorRightArrow; 
			break;

		default:
			ReportFatalError("default in TNavigatorArrowView::ReadFields");
			break;
	}
#endif
}

pascal void TNavigatorArrowView::DoPostCreate(TDocument *itsDocument)
{
	inherited::DoPostCreate(itsDocument);
	fPortBitsP = &GetWindow()->GetGrafPort()->portBits;
	Boolean prevLock = Lock(true);
	GetNewSicnFastBits(fArrowFastBits[kNormalArrow], fRsrcID);
	GetNewSicnFastBits(fArrowFastBits[kDimmedArrow], fRsrcID + 1);
	GetNewSicnFastBits(fArrowFastBits[kHighlightArrow], fRsrcID + 2);
	Lock(prevLock);
}

pascal void TNavigatorArrowView::Free()
{
	Boolean prevLock = Lock(true);
	DisposeSicnFastBits(fArrowFastBits[kNormalArrow]);
	DisposeSicnFastBits(fArrowFastBits[kDimmedArrow]);
	DisposeSicnFastBits(fArrowFastBits[kHighlightArrow]);
	Lock(prevLock);
	inherited::Free();
}

pascal void TNavigatorArrowView::Draw(const VRect& /* area */)
{
	HighlightType hl;
	if (!fEnabled || !fIsActive)	
		hl = kDimmedArrow;	
	else if (fHilite)
		hl = kHighlightArrow;
	else
		hl = kNormalArrow;
	DrawSicnFastBits(fPortBitsP, fArrowFastBits[hl], CRect(0, 0, 16, 12), CRect(0, 0, 16, 12));
}

pascal void TNavigatorArrowView::Hilite()
{
	Draw(VRect());
}

pascal void TNavigatorArrowView::DoEvent(EventNumber eventNumber,
								TEventHandler *source, TEvent *event)
{
	if (eventNumber == mOKHit)
	{
		TEventHandler *target = gApplication->GetTarget();
		if (target)
			target->DoMenuCommand(fHitCommandNumber);
	}
	inherited::DoEvent(eventNumber, source, event);
}

pascal void TNavigatorArrowView::SetEnable(Boolean state)
{
	Boolean update = (state != fEnabled);
	inherited::SetEnable(state);
	if (update)
	{
		Focus();
		Draw(VRect());
	}
}

pascal void TNavigatorArrowView::Activate(Boolean entering)
{
	if (entering != fIsActive)
	{
		fIsActive = entering;
		if (fEnabled)
		{
			Focus();
			Draw(VRect());
		}
	}
	inherited::Activate(entering);
}

