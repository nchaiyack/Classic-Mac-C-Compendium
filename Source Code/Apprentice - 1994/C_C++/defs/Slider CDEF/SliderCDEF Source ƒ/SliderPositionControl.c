#include "SliderCDEF.h"

// SliderPositionControl
//
// Positions the indicator in the control track in response to a mouseUp after
// dragging the indicator. The routine adjusts the contrlValue field and redraws
// the control to reflect the new value. This is not really necessary for this
// slider, since the contrlValue and control's appearance are constantly updated
// during custom dragging of the indicator.
void SliderPositionControl( short varCode, ControlHandle theControl, long param )
{
	Point			thePoint;
	ControlPtr	theControlPtr;
	Rect			thumbRect;
	long			prevOffset;
	long			range;
	Rect			trackRect;
	short		trackWidth;
	short		thumbWidth;
	short		thumbHalfWidth;
	short		thumbCenter;
	float			ratio;
	
	// Lock the control handle and dereference.
	HLock( (Handle) theControl );
	theControlPtr = *theControl;
	
	// Get the offset from param.
	thePoint.v = HiWord( param );
	thePoint.h = LoWord( param );
	
	// Have'nt done vertical sliders yet.
	if ( thePoint.h == 0 ) return;

	// Calculate the thumb rectangle at the old value.
	thumbRect = CalcThumbRect( theControl );
	
	// Calculate the thumb width and half-width.
	thumbWidth = kThumbSize;
	thumbHalfWidth = thumbWidth / 2;

	// Calculate the track's rectangle and its width in pixels.
	trackRect = theControlPtr->contrlRect;
	InsetRect( &trackRect,  thumbHalfWidth + 1, 1 );
	trackWidth = trackRect.right - trackRect.left;
	
	// Calculate the range of conrol values allowed.
	range = theControlPtr->contrlMax - theControlPtr->contrlMin;
	
	// Calculate the ratio of control values to track width.
	// This would be better with fixed-point math.
	ratio = (float) ((float) range) / ((float)trackWidth);

	// Calculate the new thumb rectangle.
	thumbRect.left += thePoint.h;
	thumbRect.right += thePoint.h;
	
	// Calculate the new center of the thumb.
	thumbCenter = thumbRect.left + thumbHalfWidth;
	
	// Calculate the new control value.
	// (thumbCenter - trackRect.left) is how far into the track the thumbCenter is.
	theControlPtr->contrlValue = (thumbCenter - trackRect.left) * ratio;
	
	// Reajust the value if it's above the max or below the min.
	if ( theControlPtr->contrlValue > theControlPtr->contrlMax ) {
		theControlPtr->contrlValue = theControlPtr->contrlMax;
	}
	if ( theControlPtr->contrlValue < theControlPtr->contrlMin ) {
		theControlPtr->contrlValue = theControlPtr->contrlMin;
	}
	
	// Redraw the control with the new value if it's visible.
	if ( (**theControl).contrlVis ) {
		SliderDrawControl( varCode, theControl, param );
	}

	// Unlock the control handle.
	HUnlock( (Handle) theControl );
}