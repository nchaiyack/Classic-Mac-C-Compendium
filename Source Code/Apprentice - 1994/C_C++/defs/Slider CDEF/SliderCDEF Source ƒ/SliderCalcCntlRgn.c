#include "SliderCDEF.h"

// SliderCalcCntlRgn
//
// Calculates the entire control's region which is just the control rectangle for
// the slider control. Note: the calcCntlRgn message is only sent by the control
// manager when running in 32-bit mode. Under 24-bit mode the single message
// calcCRgns is sent for all region calculations.
void SliderCalcCntlRgn( short varCode, ControlHandle theControl, long param )
{
	// Lock the control handle.
	HLock( (Handle) theControl );

	// Put a handle to the slider's control region into param.
	RectRgn( (RgnHandle) param, &(**theControl).contrlRect );

	// Unlock the control handle.
	HUnlock( (Handle) theControl );
}