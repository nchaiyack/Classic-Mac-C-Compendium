/*
 * CScrollorama.c
 *
 * A panorama, with a few features affixed.
 *
 * � Copyright 1992 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 * Telephone:  800-421-4157 or US 616-665-7075 (9:00-5:00 Eastern time)
 * I'm releasing this code with the hope that someone will get
 * something out of it.  Feedback of any sort, even just letting
 * me know that you're using it, is greatly appreciated!
 *
 * This class was made to be hacked up and stolen from.  I don't
 * imagine you'll want to use all four features of CScrollorama
 * in exactly the way I've implemented them.  But you're welcome
 * to cut'n'paste together your own subclass of CPanorama.
 *
 *
 *								FOUR FABULOUS FEATURES
 *
 * THE FIRST FEW FEATURES
 *
 * 1:  CScrollorama::Scroll() doesn't call _ScrollRect, so the
 * scrolled region doesn't get quickly painted white or whatever the
 * background pattern is.  Instead, it just leaves that part alone.
 * (If the redraw argument is TRUE, of course, it will redraw it
 * immediately;  otherwise, it will wait till the next update event.)
 *
 * 2:  A CScrollorama won't let you scroll the frame past a given
 * margin around the bounds.  If you use a CPanorama in a CScrollPane,
 * the scroll pane will protect you from doing this (check out
 * CScrollPane::DoHorizScroll and CScrollPane::DoVertScroll).  But
 * if you're using a "naked" CPanorama, you have no such protection
 * unless you do something like CScrollorama does.  The default
 * margin is zero pixels on each side.  Note that you'll probably
 * want to specify negative values for top and left, positive ones
 * for bottom and right.
 * 
 * 3:  CScrollorama includes M�rten S�rliden's SetBounds() bug fix.
 *
 * THE FOURTH FEATURE
 *
 * OK, first comes a definition.  "Avoided" is my term for a
 * region of a pane which will not be drawn into by its
 * _immediate_enclosure_.
 *
 * A CScrollorama may have its aperture, or any subregion of it,
 * avoided (by its enclosure's Draw() method), clipped (for its
 * own Draw() method), or both, or neither.
 *
 * For example, if you want your pane that draws Kilimanjaro to
 * enclose your pane that draws a leopard, and you don't want an
 * ugly white blank rectangle around the leopard, you would tell
 * Kilimanjaro to avoid the leopard, and then set the leopard's
 * avoided region to something leopard-shaped.  Then, when
 * Kilimanjaro draws, it will leave a leopard-shaped hole, which
 * the leopard pane will subsequently draw into.  But let's say
 * your leopard-drawing routine just copies a rectangle (with a
 * leopard in it) onto the screen anyway;  in that case, a
 * leopard-shaped hole doesn't help.  So you also need to set
 * the leopard pane to clip _itself_ to that same region.  Then
 * Kilimanjaro will draw everywhere _but_ in the leopard, the
 * leopard will draw _only_ in the leopard, and everyone will
 * be happy (except the leopard, who will shortly freeze).
 *
 * Or, if you had ten concentric panes, each forming one ring
 * of a bullseye by calling PaintOval(), the TCL would just draw
 * one over the other.  This means the center pane would flash
 * ten times.  To make each pane draw a ring not a circle,
 * thereby removing the "flashing center" effect, you would set
 * each one's avoided region to the circle that it draws.
 *
 * By default, a scrollorama will avoid all subpanes which are
 * also CScrolloramas.  You can tell it not to do this--i.e. to
 * revert to typical TCL pane behavior--by calling
 * setAvoidSubscrolloramas(FALSE).  If you want a particular
 * scrollorama to not be avoided, call setBeAvoided(FALSE).
 *
 * By default, a scrollorama's avoided region is a "wide-open"
 * region.  (The avoided region applies to the bounds, not the
 * frame.  It's expressed in frame coordinates--pixels--but as
 * the frame moves over the bounds, the avoided region "sticks"
 * to the contents of the pane.  The actual clipping region at
 * the time of drawing will be the intersection of the avoided
 * region and the avoided pane's aperture.)  To change it, call
 * setAvoidedRgn().
 *
 * By default, a scrollorama does _not_ clip itself to the
 * avoided region.  If you would like it to do so, call
 * setClipToAvoidedRgn(TRUE).
 *
 * Note that it is possible to have an avoided region going
 * to waste.  A region that's neither being avoided nor being
 * clipped-to might as well not be there.  To release its
 * memory, call setAvoidedRgn(NULL).  This returns the avoided
 * region to wide-open.
 *
 * Note also that a scrollorama with an empty avoided region,
 * that clips drawing to that region, is essentially hidden.
 * It's more efficient just to Hide() the pane, though.
 *
 * Note, thirdly, that a scrollorama can only avoid its
 * immediate subscrolloramas, not subsubscrolloramas or
 * subsubsubscrolloramas.  If you have, say, a Kilimanjaro pane
 * enclosing a leopard pane enclosing a "flea" pane, and you
 * tell the leopard to setBeAvoided(FALSE), then there's no way
 * to tell Kilimanjaro to avoid the flea.  If this discrepancy
 * gets under your skin, let me know and I'll consider
 * implementing recursive avoidance.
 *
 */



/********************************/

#include "CScrollorama.h"

/********************************/

#include <CList.h>
#include <CWindow.h>

/********************************/

extern RgnHandle gUtilRgn;

/********************************/

void nonErasingScrollRect(Rect *srcArea, short hDelta, short vDelta, RgnHandle updateRgn);

/********************************/



void CScrollorama::IScrollorama(CView *anEnclosure, CBureaucrat *aSupervisor,
	short aWidth, short aHeight,
	short aHEncl, short aVEncl,
	SizingOption aHSizing, SizingOption aVSizing)
{
	showMargin.left =
		showMargin.top =
		showMargin.right =
		showMargin.bottom = 0;
	
	inherited::IPanorama(anEnclosure, aSupervisor,
		aWidth, aHeight,
		aHEncl, aVEncl,
		aHSizing, aVSizing);
	
	IScrolloramaX();
}



void CScrollorama::IViewTemp(CView *anEnclosure, CBureaucrat *aSupervisor,
	Ptr viewData)
{
	showMargin.left =
		showMargin.top =
		showMargin.right =
		showMargin.bottom = 0;
	
	inherited::IViewTemp(anEnclosure, aSupervisor, viewData);
	
	IScrolloramaX();
}



void CScrollorama::IScrolloramaX(void)
{
	paneVisRgn = NULL;
	pvrIsUpToDate = TRUE;
	
		/*
		 * By default, avoid all subscrolloramas.
		 */
	avoidSubscrolloramas = TRUE;
	
		/*
		 * By default, if the enclosing pane is a scrollorama that wants to
		 * avoid drawing over me, then let it avoid drawing over me.
		 */
	beAvoided = TRUE;
	
		/*
		 * By default, don't clip my drawing to my avoided region.
		 */
	clipToAvoidedRgn = FALSE;
	
		/*
		 * By default, if the enclosing pane wants to avoid this pane,
		 * then it should avoid the whole aperture of this pane.
		 */
	avoidedRgn = NULL;
}


void CScrollorama::Dispose(void)
{
	if (beAvoided
		&& member(itsEnclosure, CScrollorama)
		&& ((CScrollorama*)itsEnclosure)->getAvoidSubscrolloramas()) {
		
		((CScrollorama*)itsEnclosure)->markPaneVisRgnAsOutOfDate();
		
	}
	
	forgetPaneVisRgn();
	forgetAvoidedRgn();
	inherited::Dispose();
}



void CScrollorama::setShowMargin(LongRect *newShowMargin)
{
	showMargin = *newShowMargin;
}



void CScrollorama::getShowMargin(LongRect *theShowMargin)
{
	*theShowMargin = showMargin;
}



void CScrollorama::setShowBounds(register LongRect *newShowBounds)
{
	LongRect newShowMargin;
	newShowMargin.top = newShowBounds->top - bounds.top;
	newShowMargin.left = newShowBounds->left - bounds.left;
	newShowMargin.right = newShowBounds->right - bounds.right;
	newShowMargin.bottom = newShowBounds->bottom - bounds.bottom;
	setShowMargin(&newShowMargin);
}



void CScrollorama::getShowBounds(register LongRect *theShowBounds)
{
	theShowBounds->top = bounds.top + showMargin.top;
	theShowBounds->left = bounds.left + showMargin.left;
	theShowBounds->right = bounds.right + showMargin.right;
	theShowBounds->bottom = bounds.bottom + showMargin.bottom;
}



void CScrollorama::Scroll(long hDelta, long vDelta, Boolean redraw)
{
	LongRect theShowBounds;
	register long hPixelDelta, vPixelDelta;
	Rect scrollArea;
	
	
		/*
		 * The first feature:  contained scrolling.
		 */
		
	getShowBounds(&theShowBounds);
	if (frame.left + hDelta < theShowBounds.left)
		hDelta = theShowBounds.left - frame.left;
	if (frame.right + hDelta > theShowBounds.right)
		hDelta = theShowBounds.right - frame.right;
	if (frame.top + vDelta < theShowBounds.top)
		vDelta = theShowBounds.top - frame.top;
	if (frame.bottom + vDelta > theShowBounds.bottom)
		vDelta = theShowBounds.bottom - frame.bottom;
	if (hDelta == 0 && vDelta == 0) return;
	
	
		/*
		 * Do the actual scroll.
		 */
		
	hPixelDelta = hDelta * hScale;
	vPixelDelta = vDelta * vScale;
	
	if (redraw) {
		if ( Abs(hPixelDelta)<width && Abs(vPixelDelta)<height ) {
				/*
				 * If some of what's presently visible will still be visible
				 * after we scroll, save that part.
				 */
			Prepare();
			FrameToQDR(&aperture, &scrollArea);
				/*
				 * The second feature:  non-flashing scrolling.
				 */
			nonErasingScrollRect(&scrollArea, -hPixelDelta, -vPixelDelta, gUtilRgn);
			InvalRgn(gUtilRgn);
		} else {
				/*
				 * Otherwise, the whole aperture has to be redrawn.
				 */
			Refresh();
		}
	}
	
	
		/*
		 * Take care of the instance variables that have to be updated.
		 */
	
	position.h += hDelta;
	position.v += vDelta;
	
	hOrigin += hPixelDelta;
	vOrigin += vPixelDelta;
	
	ForceNextPrepare();
	
	frame.left += hPixelDelta;
	frame.right += hPixelDelta;
	frame.top += vPixelDelta;
	frame.bottom += vPixelDelta;
	
	aperture.left += hPixelDelta;
	aperture.right += hPixelDelta;
	aperture.top += vPixelDelta;
	aperture.bottom += vPixelDelta;
	
	
		/*
		 * Notify subpanes, if any, that this view has scrolled.
		 */
		
	if (itsSubviews != NULL) {
		LongPt offset;
		offset.h = hPixelDelta;
		offset.v = vPixelDelta;
		itsSubviews->DoForEach1((EachFunc1) Pane_EnclosureScrolled, (long) &offset);
	}
	
	
		/*
		 * If:
		 *     � this pane wants to be avoided,
		 * and � this pane is enclosed by a CScrollorama that wants to avoid,
		 * and � this pane wants a specific area to be avoided (as opposed
		 *       to just wanting to avoid its whole aperture),
		 * then:
		 * the enclosure's paneVisRgn is out of date, and needs to be
		 * marked as such.
		 */
		
	if (beAvoided
		&& member(itsEnclosure, CScrollorama)
		&& ((CScrollorama*)itsEnclosure)->getAvoidSubscrolloramas()
		&& avoidedRgn != NULL) {
		
		((CScrollorama*)itsEnclosure)->markPaneVisRgnAsOutOfDate();
		
	}
	
	
		/*
		 * If this pane clips to its avoided region, then it is now
		 * out of date and needs to be marked as so.
		 */
		
	if (clipToAvoidedRgn) {
		markPaneVisRgnAsOutOfDate();
	}
	
	
		/*
		 * If an immediate redraw was requested, tell the CWindow
		 * to update itself.
		 */
		
	if (redraw) {
		( (CWindow *) ((WindowPeek) macPort)->refCon )->Update();
	}
}


	
void CScrollorama::SetBounds(LongRect *aBounds)
{
		/*
		 * M�rten S�rliden's "if the bounds shrinks, keep the frame
		 * inside it" bug fix (marten@uidesign.se).
		 */
		
	long hDelta, vDelta;
	
	hDelta = 0;
	vDelta = 0;
	
	if (aBounds->right < frame.right)
		hDelta = Min(frame.left - aBounds->left, frame.right - aBounds->right);
	
	if (aBounds->bottom < frame.bottom)
		vDelta = Min(frame.top - aBounds->top, frame.bottom - aBounds->bottom);
	
	if (hDelta != 0 || vDelta != 0) {
		Scroll(-hDelta, -vDelta, FALSE);
	}
	
	inherited::SetBounds(aBounds);
}



void CScrollorama::setAvoidSubscrolloramas(Boolean newAvoidSubscrolloramas)
{
	if (newAvoidSubscrolloramas == avoidSubscrolloramas) return;
	avoidSubscrolloramas = (newAvoidSubscrolloramas != FALSE);
	if (itsSubviews != NULL) {
		markPaneVisRgnAsOutOfDate();
	}
}



Boolean CScrollorama::getAvoidSubscrolloramas(void)
{
	return avoidSubscrolloramas;
}



void CScrollorama::setBeAvoided(Boolean newBeAvoided)
{
	if (beAvoided == newBeAvoided) return;
	beAvoided = (newBeAvoided != FALSE);
	if (member(itsEnclosure, CScrollorama)) {
		((CScrollorama*)itsEnclosure)->markPaneVisRgnAsOutOfDate();
	}
}



Boolean CScrollorama::getBeAvoided(void)
{
	return beAvoided;
}



void CScrollorama::setClipToAvoidedRgn(Boolean newClipToAvoidedRgn)
{
	if (clipToAvoidedRgn == newClipToAvoidedRgn) return;
	clipToAvoidedRgn = (newClipToAvoidedRgn != FALSE);
	if (avoidedRgn != NULL) {
		markPaneVisRgnAsOutOfDate();
	}
}



Boolean CScrollorama::getClipToAvoidedRgn(void)
{
	return clipToAvoidedRgn;
}



void CScrollorama::setAvoidedRgn(RgnHandle newAvoidedRgn)
{
	if (newAvoidedRgn == avoidedRgn) return;
	
	forgetAvoidedRgn();
	
	if (newAvoidedRgn != NULL) {
		
			/*
			 * Make sure we don't try to avoid drawing things that are
			 * outside our bounds.
			 */
		
		Rect myBounds;
		RgnHandle avoidWithinBoundsRgn;
		
		avoidWithinBoundsRgn = NewRgn();
		LongToQDRect(&bounds, &myBounds);
		RectRgn(avoidWithinBoundsRgn, &myBounds);
		SectRgn(avoidWithinBoundsRgn, newAvoidedRgn, avoidWithinBoundsRgn);
		
		avoidedRgn = avoidWithinBoundsRgn;
		
	}
	
	markPaneVisRgnAsOutOfDate();
}




void CScrollorama::getAvoidedRgn(RgnHandle theAvoidedRgn)
{
	if (avoidedRgn == NULL) {
		
			/*
			 * A NULL avoidedRgn means "avoid everything."
			 */
		
		Rect myBounds;
		LongToQDRect(&bounds, &myBounds);
		RectRgn(theAvoidedRgn, &myBounds);
		
	} else {
		
		CopyRgn(avoidedRgn, theAvoidedRgn);
		
	}
}



void CScrollorama::markPaneVisRgnAsOutOfDate(void)
{
	pvrIsUpToDate = FALSE;
}



Boolean CScrollorama::getPaneVisRgnIsUpToDate(void)
{
	return pvrIsUpToDate;
}



	void doRemoveFromRgn(CView *theSubview, long theNewPaneVisRgn);
	void doRemoveFromRgn(CView *theSubview, long theNewPaneVisRgn)
	{
		ASSERT(member(theSubview, CPane));
		if (member(theSubview, CScrollorama)
			&& ((CScrollorama*)theSubview)->getBeAvoided()) {
			
			RgnHandle theAvoidedRgn;
			RgnHandle theApertureRgn;
			LongRect theApertureL;
			Rect theAperture;
			
			theAvoidedRgn = NewRgn();
			((CScrollorama*)theSubview)->getAvoidedRgn(theAvoidedRgn);
			
				/*
				 * A subscrollorama can't make its enclosure avoid anything
				 * that's not in the subpane's aperture.
				 */
			theApertureRgn = NewRgn();
			((CPane*)theSubview)->GetAperture(&theApertureL);
			LongToQDRect(&theApertureL, &theAperture);
			RectRgn(theApertureRgn, &theAperture);
			SectRgn(theAvoidedRgn, theApertureRgn, theAvoidedRgn);
			DisposeRgn(theApertureRgn);
			
				/*
				 * Change the subpane's avoided region into the enclosure's
				 * frame coordinate system.
				 */
			OffsetRgn(theAvoidedRgn,
				((CPane*)theSubview)->hEncl,
				((CPane*)theSubview)->vEncl);
			
				/*
				 * Subtract its area out from the enclosure's visible region.
				 */
			DiffRgn((RgnHandle) theNewPaneVisRgn,
				theAvoidedRgn,
				(RgnHandle) theNewPaneVisRgn);
			
			DisposeRgn(theAvoidedRgn);
			
		}
	}
	
void CScrollorama::updatePaneVisRgn(void)
{
	Rect theApertureRect;
	
	if (getPaneVisRgnIsUpToDate()) return;
	
	forgetPaneVisRgn();
	
	LongToQDRect(&aperture, &theApertureRect);
	
	if (EmptyRect(&theApertureRect)) {
		
			/*
			 * Leave the instance var "paneVisRgn" set to NULL.  NULL means
			 * "the whole aperture is visible," which, if the aperture is
			 * an empty rectangle, is surely true.
			 */
		
	} else {
		
		RgnHandle theApertureRgn;
		RgnHandle theNewPaneVisRgn;
		
		theApertureRgn = NewRgn();
		theNewPaneVisRgn = NewRgn();
		RectRgn(theApertureRgn, &theApertureRect);
		CopyRgn(theApertureRgn, theNewPaneVisRgn);
		
		if (itsSubviews != NULL) {
			itsSubviews->DoForEach1(doRemoveFromRgn, (long) theNewPaneVisRgn);
		}
		
		if (getClipToAvoidedRgn() && avoidedRgn != NULL) {
			SectRgn(theNewPaneVisRgn, avoidedRgn, theNewPaneVisRgn);
		}
		
		if (EqualRgn(theNewPaneVisRgn, theApertureRgn)) {
				/*
				 * If the new paneVisRgn would just be the entire aperture--i.e.
				 * if there's no special clipping going on--just leave the
				 * instance variable set to NULL.
				 */
			DisposeRgn(theNewPaneVisRgn);
		} else {
			paneVisRgn = theNewPaneVisRgn;
		}
		
		DisposeRgn(theApertureRgn);
		
	}
	
	pvrIsUpToDate = TRUE;
}



void CScrollorama::Hide(void)
{
	inherited::Hide();
	if (member(itsEnclosure, CScrollorama)) {
		((CScrollorama*)itsEnclosure)->markPaneVisRgnAsOutOfDate();
	}
}



void CScrollorama::Show(void)
{
	inherited::Show();
	if (member(itsEnclosure, CScrollorama)) {
		((CScrollorama*)itsEnclosure)->markPaneVisRgnAsOutOfDate();
	}
}



void CScrollorama::AddSubview(CView *theSubview)
{
	inherited::AddSubview(theSubview);
	if (member(theSubview, CScrollorama)) {
		markPaneVisRgnAsOutOfDate();
	}
}



void CScrollorama::RemoveSubview(CView *theSubview)
{
	inherited::RemoveSubview(theSubview);
	if (member(theSubview, CScrollorama)) {
		markPaneVisRgnAsOutOfDate();
	}
}



void CScrollorama::Offset(long hOffset, long vOffset, Boolean redraw)
{
	inherited::Offset(hOffset, vOffset, redraw);
	if (member(itsEnclosure, CScrollorama)) {
		((CScrollorama*)itsEnclosure)->markPaneVisRgnAsOutOfDate();
	}
}



void CScrollorama::ChangeSize(Rect *delta, Boolean redraw)
{
	inherited::ChangeSize(delta, redraw);
	if (member(itsEnclosure, CScrollorama)) {
		((CScrollorama*)itsEnclosure)->markPaneVisRgnAsOutOfDate();
	}
}



	/*
	 * This method is the same as CPane::DrawAll(), but with two lines
	 * of code #if'fed out.
	 */
	
void CScrollorama::DrawAll(Rect *area)
{
	Rect	tmpArea = *area;
	
	Prepare();
	
	/* area is in Window coordinates, put it into Frame or QD coordinates	*/

	if (usingLongCoord)
		OffsetRect( &tmpArea, thePort->portRect.left, thePort->portRect.top);
	else
		OffsetRect( &tmpArea, hOrigin, vOrigin);
	
#if 0
		/*
		 * At this point, CPane would decide to force the port's clipRgn
		 * to the full area that's being drawn.  CScrollorama wants to do
		 * something else, though, which is why this code has been
		 * #if'fed out.
		 */
	if (!printing) {	/* no need to SectRect here, already done in Pane_Draw */

		ClipRect( &tmpArea);
	}
#else
	if (!printing) {
			/*
			 * We only want to draw into the _intersection_ of the requested
			 * rectangle and the paneVisRgn.  The port's clipRgn has been
			 * set to the paneVisRgn in CScrollorama::Prepare().
			 */
		RgnHandle drawAreaRgn;
		drawAreaRgn = NewRgn();
		RectRgn(drawAreaRgn, &tmpArea);
		SectRgn(macPort->clipRgn, drawAreaRgn, macPort->clipRgn);
		DisposeRgn(drawAreaRgn);
	}
#endif

	Draw( &tmpArea);
				
	if (itsSubviews != NULL)
	{
		// clip area to aperture of this view before drawing subviews
		
		if (!printing)
		{
			FrameToWindR( &aperture, &tmpArea);
			SectRect( &tmpArea, area, &tmpArea);
		}
		else			/* TCL 1.1.2 DLP 10/28/91 */
			tmpArea = *area;
			
		itsSubviews->DoForEach1((EachFunc1) Pane_Draw, (long) &tmpArea);
	}
}



void CScrollorama::Prepare()
{
		/*
		 * The inherited method, CPane::Prepare(), sets the clipping region
		 * to the aperture of this pane.
		 */
	inherited::Prepare();
	
	if (getAvoidSubscrolloramas() || getClipToAvoidedRgn()) {
		
			/*
			 * OK, we need to go a bit further.  Update the paneVisRgn,
			 * if it isn't already, and then get the intersection of
			 * that with the aperture.
			 */
			 
		updatePaneVisRgn();
		if (paneVisRgn != NULL) {
			SectRgn(macPort->clipRgn, paneVisRgn, macPort->clipRgn);
		}
		
	}
}



/********************************/



void CScrollorama::forgetPaneVisRgn(void)
{
	if (paneVisRgn != NULL) {
		DisposeRgn(paneVisRgn);
		paneVisRgn = NULL;
	}
}



void CScrollorama::forgetAvoidedRgn(void)
{
	if (avoidedRgn != NULL) {
		DisposeRgn(avoidedRgn);
		avoidedRgn = NULL;
	}
}



/********************************/



void nonErasingScrollRect(Rect *srcArea, short hDelta, short vDelta, RgnHandle updateRgn)
{
	Rect destArea;
	RgnHandle destRgn;
	destArea = *srcArea;
	OffsetRect(&destArea, hDelta, vDelta);
	CopyBits(&thePort->portBits, &thePort->portBits,
		srcArea, &destArea,
		srcCopy, NULL);
	RectRgn(updateRgn, srcArea);
	destRgn = NewRgn();
	RectRgn(destRgn, &destArea);
	DiffRgn(updateRgn, destRgn, updateRgn);
	DisposeRgn(destRgn);
		/*
		 * ...and then _don't_ EraseRgn(updateRgn).  I believe
		 * that to be the only logical difference between this
		 * code and _ScrollRect.
		 */
}
