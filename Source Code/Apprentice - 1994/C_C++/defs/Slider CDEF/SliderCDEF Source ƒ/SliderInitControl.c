#include "SliderCDEF.h"

// SliderInitControl
//
// Performs initialization for the control definition function such as putting some
// data in the contrlData or contrlRfCon fields or setting the contrlAction field to
// (Ptr) -1L so that the control manager will call the CDEF with the autoTrack
// message.
void SliderInitControl( short varCode, ControlHandle theControl, long param )
{
	// Allocate the slider's data and place a handle to the data structure
	// in the contrlData field.
	HLock( (Handle) theControl );
	(**theControl).contrlData = NewHandle( sizeof( SliderDataRec ) );
	if ( (**theControl).contrlData != nil ) {
		// Create the offscreen graphics world for the slider.
		SliderCreateOffscreenWorld( theControl );
	}
	HUnlock( (Handle) theControl );
}



// SliderCreateOffscreenWorld
//
// Create the offscreen graphics world for the slider control.
void SliderCreateOffscreenWorld( ControlHandle theControl )
{
	SliderDataHnd	sliderData;
	short		err;
	
	// Lock down the control handle.
	HLock( (Handle) theControl );
	
	// Get the handle to the slider's data and lock it.
	sliderData = (SliderDataHnd) (**theControl).contrlData;	
	HLock( (Handle) sliderData );
	
	// Set up the slider's offscreen graphics worlds for drawing.
	// Get the current port and device.
	GetGWorld( &(**sliderData).currPort, &(**sliderData).currDev );

	// Calculate the size of the offscreen graphics world. Start with size
	// of the control's rect.
	(**sliderData).sliderPortRect = (**theControl).contrlRect;
	
	// Unlock the control handle, we're done with it.
	HUnlock( (Handle) theControl );

	// Move the rect to the top-left corner, (0,0).
	OffsetRect( &(**sliderData).sliderPortRect, -(**sliderData).sliderPortRect.left,
		-(**sliderData).sliderPortRect.top );

	// Make this the rectangle for the slider track.
	//	-----------------------------
	//	|		track			|
	//	-----------------------------
	(**sliderData).sliderTrackRect = (**sliderData).sliderPortRect;
	
	// Add room at the end for an entire image of the
	// slider (track with thumb in it).
	//	----------------------------------------------------------
	//	|		track			|	entire slider image		|
	//	----------------------------------------------------------
	(**sliderData).sliderPortRect.right += (**sliderData).sliderPortRect.right;
	(**sliderData).sliderRect = (**sliderData).sliderPortRect;
	(**sliderData).sliderRect.left = (**sliderData).sliderTrackRect.right;

	// Add room at the end of the track for an image of the
	// slider's thumb.
	//	------------------------------------------------------------------------
	//	|		track			|	entire slider image		|	thumb	|
	//	------------------------------------------------------------------------
	(**sliderData).sliderPortRect.right += kThumbSize;
	(**sliderData).sliderThumbRect = (**sliderData).sliderPortRect;
	(**sliderData).sliderThumbRect.left = (**sliderData).sliderRect.right;

	// Create offscreen graphics world.
	err = NewGWorld( &(**sliderData).sliderPort, 0, &(**sliderData).sliderPortRect, nil, nil, 0 );

	// Unlock the slider's data.
	HUnlock( (Handle) sliderData );

	// Draw the control's track and the control's indicator in the offscreen graphics world.
	SliderDrawControlParts( theControl );	
}
	



// SliderDrawControlParts
//
// Draw each of the slider's parts into the slider's GWorld.
void SliderDrawControlParts( ControlHandle theControl )
{
	SliderDataHnd	sliderDataHnd;
	SliderDataPtr	sliderData;
	short		err;
	SliderColors	colors;
	
	// Get a ptr to the slider's data.
	sliderDataHnd = (SliderDataHnd) (**theControl).contrlData;
	HLock( (Handle) sliderDataHnd );
	sliderData = *sliderDataHnd;

	// Lock down Pixels that we are drawing to so that memory will not  move
	LockPixels ( sliderData->sliderPort->portPixMap );
	
	// Setup drawing area to be the offscreen graphics world
	SetGWorld ( sliderData->sliderPort, nil );
	
	// Clear Rectangle, so that CopyBits will not copy extra background
	EraseRect ( &sliderData->sliderPortRect );

	// Draw the track.
	SliderGetControlColors( theControl, &colors );
	SliderDrawControlTrack( theControl, &colors );
	SliderDrawControlThumb( theControl, &colors );

	// Done drawing, now reset port etc.
	SetGWorld ( sliderData->currPort, sliderData->currDev );

	// Now unlock pixels.
	UnlockPixels( sliderData->sliderPort->portPixMap );
	
	// Unlock the slider's data.
	HUnlock( (Handle) sliderDataHnd );
}