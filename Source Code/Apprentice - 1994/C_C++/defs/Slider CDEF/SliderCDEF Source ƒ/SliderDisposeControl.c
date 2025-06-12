#include "SliderCDEF.h"

// SliderDisposeControl
//
// This routine is called in response to the dispCntl message so that the CDEF may
// do additional disposal actions.
void SliderDisposeControl( short varCode, ControlHandle theControl, long param )
{
	// Dispose of the slider's GWorld.
	if (  (**((SliderDataHnd)(**theControl).contrlData)).sliderPort  != nil ) {
		DisposeGWorld( (**((SliderDataHnd)(**theControl).contrlData)).sliderPort );
	}
	
	// Dispose of the slider's private data.
	if ( (**theControl).contrlData != nil ) {
		DisposeHandle( 	(**theControl).contrlData );
	}
}