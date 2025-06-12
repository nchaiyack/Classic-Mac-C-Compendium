#include "SliderCDEF.h"

#define kPartMask	0xF0000000

// SliderCalcCRgns
//
// Calculates either the entire control's region which is just the control rectangle for
// the slider control or the indicator's region, depending on the high bit of param.
// Note: the calcCRgns message is only sent by the control manager when running
// in 24-bit mode. Under 32-bit mode, two messages are sent: one for the entire
// control (calcCntlRgn), and one for the indicator (calcThumbRgn).
void SliderCalcCRgns( short varCode, ControlHandle theControl, long param )
{
	Boolean	isIndicator;
	Rect		thumbRect;
			
	// Test the high bit of param
	isIndicator = param & kPartMask;
	if ( isIndicator ) {
		// Clear the high-order bit;
		param = param | kPartMask;
		
		thumbRect = CalcThumbRect( theControl );
		RectRgn( (RgnHandle) param, &thumbRect );
	} else {
		HLock( (Handle) theControl );
		RectRgn( (RgnHandle) param, &(**theControl).contrlRect );
		HUnlock( (Handle) theControl );
	}
}

#undef kPartMask