// Copyright � 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// UViewListView.h

#define __UVIEWLISTVIEWVIEW__

class TViewListView : public TView
{
	public:
		virtual pascal void SubViewChangedFrame(TView* theSubView,
											const VRect& oldFrame, const VRect& newFrame, Boolean invalidate);

		virtual pascal void Draw(const VRect& area);
		virtual void DrawViewSeparator(VCoordinate vOffset); // currently does nothing

		void RebuildSubViewFrames();
		
		TViewListView();
		virtual pascal void Initialize();
		virtual pascal void ReadFields(TStream *aStream);
		virtual pascal void DoPostCreate(TDocument *itsDocument);
		virtual pascal void Free();
	protected:
		Boolean fIsAdjustingSubViewFrames;
		Boolean fAddSeparatorAtBottom;
		
		virtual VCoordinate GetSeparatorHeight();
		virtual void CheckNewFrame(VRect &newFrame);	
	
	private:
		void UpdateSubViewFrames(TView* theSubView,
								const VRect& oldFrame, const VRect& newFrame);
		void DoScrollBits(VCoordinate vFrom, VCoordinate offset);
		void AdjustOneSubView(TView *view, VRect oldFrame, VRect newFrame, VCoordinate offset);
};
