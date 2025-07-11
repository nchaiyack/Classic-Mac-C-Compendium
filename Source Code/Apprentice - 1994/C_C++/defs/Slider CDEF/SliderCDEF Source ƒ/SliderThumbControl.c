#include "SliderCDEF.h"

// SliderThumbControl
//
// Calculates the parameters for dragging the indicator. This routine is not
// necessary for this slider, since dragging is performed by the CDEF, not
// the control manager.
void SliderThumbControl( short varCode, ControlHandle theControl, long param )
{
	Point		thePoint;
	Rect		thumbRect;
	long		thumbHalfWidth;
	long		offset;

	// Get the point where the user clicked from the limitRect topLeft point.
	thePoint.h = ((ThumbCntlRecPtr)param)->limitRect.left;

	// Calculate the thumb rectangle.
	thumbRect = CalcThumbRect( theControl );

	// Calculate the horizontal distance into the thumb from the left edge.
	offset = thePoint.h - thumbRect.left;

	// Calculate half the width of the thumb.
	thumbHalfWidth = (thumbRect.right - thumbRect.left)/2;

	// Equate the limitRect with the control's rectangle.
	((ThumbCntlRecPtr)param)->limitRect = (**theControl).contrlRect;

	// Inset the limitRect to account for interior borders of the track.
	InsetRect( &((ThumbCntlRecPtr)param)->limitRect, 1, 1 );

	// Add the offset into the thumb to the left edge of the limitRect.
	((ThumbCntlRecPtr)param)->limitRect.left += (offset);

	// Subtract the opposite offset into the thumb from the right edge of the limitRect.
	((ThumbCntlRecPtr)param)->limitRect.right -= (thumbHalfWidth*2 - offset);

	// Equate the slopRect and the limitRect.
	((ThumbCntlRecPtr)param)->slopRect  = ((ThumbCntlRecPtr)param)->limitRect;

	// Expand the slopRect.
	InsetRect( &((ThumbCntlRecPtr)param)->slopRect, -50, -30 );

	// Constrain dragging to the horizontal axis.
	((ThumbCntlRecPtr)param)->axis = hAxisOnly;
}