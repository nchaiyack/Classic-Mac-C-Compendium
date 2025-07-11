// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UTextScroller.h

#define __UTEXTSCROLLER__

class TTextScroller : public TScroller
{
	public:
		virtual void DoScrollUp();
		virtual void DoScrollDown();
		virtual void DoPageUp();
		virtual void DoPageDown();
		virtual void DoGoHome();
		virtual void DoGoEnd();

		virtual pascal void CreateScrollBar(VHSelect itsDirection);
		virtual pascal void DoScroll(const VPoint &delta, Boolean redraw);

		virtual pascal void DoKeyEvent(TToolboxEvent *event);
		virtual pascal void DoCommandKeyEvent(TToolboxEvent *event);
		virtual pascal VCoordinate ScrollStep(VHSelect vhs, short partCode);

		TTextScroller();
		virtual pascal void Initialize();	
		virtual pascal void IRes(TDocument *itsDocument,
							 TView *itsSuperView,
							 TStream *itsParams);
		virtual pascal void Free();
};
