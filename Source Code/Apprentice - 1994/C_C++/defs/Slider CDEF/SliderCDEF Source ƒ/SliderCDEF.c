#include <SetUpA4.h>
#include "SliderCDEF.h"

// main for SliderCDEF
//
// The entry point to the control definition. This function is called directly by the control
// manager for all control actions.
pascal long main( short varCode, ControlHandle theControl, short message, long param )
{
	Point		thePoint;
	long		result = 0L;
	
	// Set up global variable access using register A4.
	RememberA0();
	SetUpA4();
		
	switch( message ) {
		case drawCntl:
			// If the control is visible, draw it.
			if ( (**theControl).contrlVis ) {
				SliderDrawControl( varCode, theControl, param );
			}
			// Always return 0 for drawCntl.
			result = 0L;
			break;
		case testCntl:
			// Find the part code of the control part clicked on, if any.
			result = SliderTestControl( varCode, theControl, param );
			break;
		case calcCRgns:
			SliderCalcCRgns( varCode, theControl, param );
			result = 0L;
			break;
		case initCntl:
			SliderInitControl( varCode, theControl, param );
			result = 0L;
			break;
		case dispCntl:
			SliderDisposeControl( varCode, theControl, param );
			result = 0L;
			break;
		case posCntl:
			SliderPositionControl( varCode, theControl, param );
			result = 0L;
			break;
		case thumbCntl:
			result = 0L;
			break;
		case dragCntl:
			result = SliderDragControl( varCode, theControl, param );
			break;
		case autoTrack:
			result = SliderDragControl( varCode, theControl, param );
			break;
		case calcCntlRgn:
			SliderCalcCntlRgn( varCode, theControl, param );
			result = 0L;
			break;
		case calcThumbRgn:
			SliderCalcThumbRgn( varCode, theControl, param );
			result = 0L;
			break;
		default:
			result = 0L;
	}
	RestoreA4();
	return result;
}