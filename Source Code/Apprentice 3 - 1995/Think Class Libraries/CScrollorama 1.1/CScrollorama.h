/*
 * CScrollorama.h
 *
 */



/********************************/

#pragma once

/********************************/

#include <CPanorama.h>

/********************************/



class CScrollorama : public CPanorama {
	
public:
	
	void			IScrollorama(CView *anEnclosure, CBureaucrat *aSupervisor,
						short aWidth, short aHeight,
						short aHEncl, short aVEncl,
						SizingOption aHSizing, SizingOption aVSizing);
	void			IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
						Ptr viewData);
	void			IScrolloramaX(void);
	void			Dispose(void);
	
	
						// The first feature:  contained scrolling.
						// (See also the code for Scroll()).
	void			setShowMargin(LongRect *newShowMargin);
	void			getShowMargin(LongRect *theShowMargin);
	void			setShowBounds(LongRect *newShowBounds); // convenience method
	void			getShowBounds(LongRect *theShowBounds); // convenience method
	
						// The second feature:  non-flashing scrolling.
	void			Scroll(long hDelta, long vDelta, Boolean redraw);
	
						// The third feature:  MŒrten Sšrliden's bug fix.
	void			SetBounds(LongRect *aBounds);
	
						// The fourth feature:  avoidance behavior.
						// (See also the code for Scroll()).
	void			setAvoidSubscrolloramas(Boolean newAvoidSubscrolloramas);
	Boolean		getAvoidSubscrolloramas(void);
	void			setBeAvoided(Boolean newBeAvoided);
	Boolean		getBeAvoided(void);
	void			setClipToAvoidedRgn(Boolean newClipToAvoidedRgn);
	Boolean		getClipToAvoidedRgn(void);
	void			setAvoidedRgn(RgnHandle newAvoidedRgn);
	void			getAvoidedRgn(RgnHandle theAvoidedRgn); // copies rgn into a valid RgnHandle
	
	
						/*
						 * Housekeeping methods for the fourth feature.  These are public
						 * because you can't screw anything up by calling them, and they
						 * can be useful.  (In fact, a CScrollorama must sometimes call
						 * markPaneVisRgnAsOutOfDate() for its enclosure.)
						 */
	void			markPaneVisRgnAsOutOfDate(void);
	Boolean		getPaneVisRgnIsUpToDate(void);
	void			updatePaneVisRgn(void);
	
	
						/*
						 * Overrides required by the fourth feature:
						 * keeping the paneVisRgn up to date.
						 */
	void			Hide(void);
	void			Show(void);
	void			AddSubview(CView *theSubview);
	void			RemoveSubview(CView *theSubview);
	void			Offset(long hOffset, long vOffset, Boolean redraw);
	void			ChangeSize(Rect *delta, Boolean redraw);
	
						/*
						 * More overrides required by the fourth feature:
						 * handling the actual drawing procedure.
						 */
	void			DrawAll(Rect *area);
	void			Prepare(void);
	
	
protected:
	
						// For the first feature.
	LongRect		showMargin;
	
						// For the fourth feature.
	Boolean		pvrIsUpToDate;
	Boolean		avoidSubscrolloramas;
	Boolean		beAvoided;
	Boolean		clipToAvoidedRgn;
	RgnHandle	avoidedRgn;
	RgnHandle	paneVisRgn;
	
						// Protected methods for the fourth feature.
	void			forgetPaneVisRgn(void);
	void			forgetAvoidedRgn(void);
	
	
private:
	
} ;
