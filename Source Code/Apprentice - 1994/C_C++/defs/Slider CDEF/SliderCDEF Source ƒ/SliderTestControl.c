#include "SliderCDEF.h"

// SliderTestControl
//
// Finds the part, if any, that the point is in.
long SliderTestControl( short varCode, ControlHandle theControl, long param )
{
	Rect		thumbRect;
	Point		clickLoc;

	// The test point is contained in param.
	clickLoc.v = HiWord( param );
	clickLoc.h = LoWord( param );
	
	// Calculate the thumb rectangle
	thumbRect = CalcThumbRect( theControl );
	
	// If in the thumb, return 129 (standard part code )
	if ( PtInRect( clickLoc, &thumbRect ) ) {
		return (long) inThumb;
	} else {
		return 0L;
	}
}