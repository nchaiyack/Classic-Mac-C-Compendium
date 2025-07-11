// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UNavigatorView.h

#ifndef __FASTBITS__
#include "FastBits.h"
#endif

enum HighlightType {kNormalArrow, kDimmedArrow, kHighlightArrow};
const short kNoHighlightTypes = 3;

class TNavigatorArrowView : public TControl 
{
	public:
		pascal void Draw(const VRect &area);

		pascal void Hilite();
		pascal void DoEvent(EventNumber eventNumber,
								TEventHandler *source, TEvent *event);
		pascal void SetEnable(Boolean state);
		pascal void Activate(Boolean entering);


		TNavigatorArrowView();
		pascal void Initialize();
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Free();
	private:
		SicnFastBits fArrowFastBits[kNoHighlightTypes];
		BitMap *fPortBitsP;	
		CRect fRect;
		short fRsrcID;
		EventNumber fHitCommandNumber;
		Boolean fIsActive;
};

class TNavigatorView : public TView
{
	public:
		pascal void Draw(const VRect &area);
		pascal void ComputeFrame(VRect &newFrame);

		TNavigatorView();
		pascal void Initialize();
		pascal void ReadFields(TStream *aStream);
		pascal void DoPostCreate(TDocument *itsDocument);
		pascal void Free();
	private:
};
