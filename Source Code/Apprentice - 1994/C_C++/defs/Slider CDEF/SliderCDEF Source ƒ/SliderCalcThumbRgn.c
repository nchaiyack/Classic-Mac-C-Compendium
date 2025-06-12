#include "SliderCDEF.h"

// SliderCalcThumbRgn
//
// Calculates the entire indicator's region.  Note: the calcCntlRgn message is only
// sent by the control manager when running in 32-bit mode. Under 24-bit mode
// the single message calcCRgns is sent for all region calculations.
void SliderCalcThumbRgn( short varCode, ControlHandle theControl, long param )
{
	Rect	thumbRect;
	
	// Get the thumb's rect.
	thumbRect = CalcThumbRect( theControl );
	
	// Make a tiny adjustment.
	thumbRect.bottom += 1;
	thumbRect.right += 1;
	
	// Give the control manager a handle to the thumb's region.
	RectRgn( (RgnHandle) param, &thumbRect );
}