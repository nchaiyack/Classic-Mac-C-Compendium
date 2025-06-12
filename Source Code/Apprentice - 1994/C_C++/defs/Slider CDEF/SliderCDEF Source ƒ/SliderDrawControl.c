#include "SliderCDEF.h"

extern Boolean gAnimating;
extern VBLRec gVBLRec;

// SliderDrawControl
//
// The main drawing routine for the CDEF; called in respone to the drawCtrl message
// if the control is visible.
void SliderDrawControl( short varCode, ControlHandle theControl, long param )
{
	Rect			theRect;
	SliderColors	colors;
	short		width;
	short		height;
	PenState		savedPenState;
	RgnHandle		savedClip;
	
	// Save the pen state to be restored when drawing is finished,
	// and reset the pen to its default state.
	GetPenState( &savedPenState );
	PenNormal();
	
	// Save the clipping region.
	savedClip = NewRgn();
	GetClip( savedClip );
	
	// Lock and dereference the control handle.
	HLock( (Handle) theControl );
	theRect = (**theControl).contrlRect;
	HUnlock( (Handle) theControl );

	// Calculate the width and height of the control.
	width = theRect.right - theRect.left;
	height = theRect.bottom - theRect.top;
	
	// Return if the control's rectangle is invalid.
	if ( width < 0 || height < 0 ) {
		return;
	}
	
	// Draw the control in the appropriate state.
	if ( (**theControl).contrlHilite == kInactive ) {
		SliderDrawControlInactive( varCode, theControl, param );
	}
	
	// Get the colors for the control.
	SliderGetControlColors( theControl, &colors );

	// Draw the control.
	SliderBlitControl( theControl, &colors );

	// Draw the control value.
	SliderDrawControlValue( theControl, &colors );
	
	// Restore the clipping region.
	SetClip( savedClip );
	DisposeRgn( savedClip );
	
	// Restore the pen state.
	SetPenState( &savedPenState );
}




// SliderDrawControlTrack
//
// Draws the track for the slider.
void SliderDrawControlTrack( ControlHandle theControl, SliderColors *colors )
{
	Rect		trackRect;
	RGBColor	savedForeColor;

	// Save the foreground color.
	GetForeColor( &savedForeColor );
	
	// Get a copy of the track rect (GWorld coordinates).
	trackRect = (**((SliderDataHnd)(**theControl).contrlData)).sliderTrackRect;

	// Fill in the track.
	RGBForeColor( &colors->darkColor );
	PaintRect( &trackRect );
	
	// Outline the top and left sides of the track.
	RGBForeColor( &colors->black );
	MoveTo( trackRect.left, trackRect.bottom );
	LineTo( trackRect.left, trackRect.top );
	LineTo( trackRect.right, trackRect.top );
	
	// Outline the bottom and right sides of the track.
	RGBForeColor( &colors->white );
	LineTo( trackRect.right, trackRect.bottom );
	LineTo( trackRect.left, trackRect.bottom );
	
	// Restore the foreground color.
	RGBForeColor( &savedForeColor );
}




// SliderDrawControlThumb
//
// Draws the thumb for the slider.
void SliderDrawControlThumb( ControlHandle theControl, SliderColors *colors )
{
	short	thumbCenter;
	short	thumbWidth;
	Rect		thumbRect;
	RGBColor	savedForeColor;
	
	// Save the foreground color.
	GetForeColor( &savedForeColor );

	// Get a copy of the thumb rect (GWorld coordinates).
	thumbRect = (**((SliderDataHnd)(**theControl).contrlData)).sliderThumbRect;

	// Calculate the thumb width and center.
	thumbWidth = (thumbRect.right - thumbRect.left);
	thumbCenter = thumbRect.left + thumbWidth/2 - 1;

	// Fill in the thumb rect.	
	RGBForeColor( &colors->lightColor );
	PaintRect( &thumbRect );
	
	// Outline the top and left sides of the thumb.
	RGBForeColor( &colors->black );
	MoveTo( thumbRect.left, thumbRect.bottom );
	LineTo( thumbRect.right, thumbRect.bottom );
	LineTo( thumbRect.right, thumbRect.top );

	// Outline the bottom, right, and center line of the thumb.
	RGBForeColor( &colors->darkGray );
	MoveTo( thumbRect.left, thumbRect.top );
	LineTo( thumbRect.left, thumbRect.bottom-1 );
	LineTo( thumbRect.right-1, thumbRect.bottom-1 );
	LineTo( thumbRect.right-1, thumbRect.top );
	MoveTo( thumbCenter, thumbRect.bottom-1 );
	LineTo( thumbCenter, thumbRect.top );
	
	// Add some highlights.
	RGBForeColor( &colors->white );
	MoveTo( thumbRect.left+1, thumbRect.bottom-2 );
	LineTo( thumbRect.left+1, thumbRect.top );
	LineTo( thumbCenter-1, thumbRect.top );
	MoveTo( thumbCenter+1, thumbRect.bottom-2 );
	LineTo( thumbCenter+1, thumbRect.top );
	LineTo( thumbRect.right-2, thumbRect.top );

	// Restore the foreground color.
	RGBForeColor( &savedForeColor );
}




// SliderDrawControlValue
//
// Draws the control value centered over the thumb just above or to the left of the
// slider's rectangle. Note: drawing occurs outside the control's rectangle!
void SliderDrawControlValue( ControlHandle theControl, SliderColors *colors )
{
	long		val;
	short	stringWidth;
	short	stringHalfWidth;
	short	texth, textv;
	short	thumbCenter;
	Rect		thumbRect;
	Rect		textRect;
	FontInfo	fInfo;
	Str255	valueString;
	RGBColor	savedForeColor;
	
	// Save the foreground color.
	GetForeColor( &savedForeColor );

	// Get the control's value.
	val = (long) (**theControl).contrlValue;
	
	// Turn the value into a string.
	NumToString( val , valueString );
	
	// Calculate the string width and half width.
	stringWidth = StringWidth( valueString );
	stringHalfWidth = stringWidth / 2;
	
	// Get the thumb rectangle (window coordinates) and
	// calculate the thumb center.
	thumbRect = CalcThumbRect( theControl );
	thumbCenter = thumbRect.left + (thumbRect.right - thumbRect.left)/2;
	
	// Calculate the horizontal position of the text.
	texth = thumbCenter - stringHalfWidth;
	
	// Set the text rectangle to the control's rectangle.
	textRect = (**theControl).contrlRect;
	
	// Calculate the vertical position of the text.
	textv = textRect.bottom = (**theControl).contrlRect.top - 2;
	
	// Set the text attributes.
	TextFont( geneva );
	TextSize( 9 );
	TextFace( 0 );
	
	// Calculate the top of the text rectangle.
	GetFontInfo( &fInfo );
	textRect.top = textRect.bottom - ( fInfo.ascent + fInfo.descent );
	
	// Erase the text rect.
	EraseRect( &textRect );
	
	// Set the foreground color to black and draw the text.
	RGBForeColor( &colors->black );
	MoveTo( texth, textv );
	DrawString( valueString );

	// Restore the foreground color.
	RGBForeColor( &savedForeColor );
}




// DrawSliderControlInactive
//
// Draws the control in its disabled state.
void SliderDrawControlInactive( short varCode, ControlHandle theControl, long param )
{
	// Nothing here, yet...
}



// SliderBlitControl
//
// Copy the slider to the window.
void SliderBlitControl( ControlHandle theControl, SliderColors *colors )
{
	SliderDataHnd	sliderDataHnd;
	SliderDataPtr	sliderData;
	Rect			thumbRect;
	short		temp;
	CWindowPtr	theWindow;	
	RGBColor		savedForeColor, savedBackColor;

	// Lock the control handle, and get the slider's data.
	HLock( (Handle) theControl );
	sliderDataHnd = (SliderDataHnd) (**theControl).contrlData;
	HLock( (Handle) sliderDataHnd );
	sliderData = *sliderDataHnd;

	// Get the control's parent window.
	theWindow = (CWindowPtr) (**theControl).contrlOwner;
	SetPort( (GrafPtr) theWindow );

	// Calculate the thumb's rectangle (window coordinates).
	thumbRect = CalcThumbRect( theControl );
	
	// Lock down the pixels that we are drawing to.
	LockPixels ( sliderData->sliderPort->portPixMap );
	
	// Save the foreground and background colors.
	GetForeColor( &savedForeColor );
	GetBackColor( &savedBackColor );
	
	// Set the foreground and background colors to 
	// black and white, respectively, so that CopyBits
	// will do the right thing.
	RGBForeColor( &colors->black );
	RGBBackColor( &colors->white );

	// Copy the track to the work area.
	CopyBits ( (BitMap *) (*(sliderData->sliderPort->portPixMap)),
		(BitMap *) (*(sliderData->sliderPort->portPixMap)),
		&sliderData->sliderTrackRect, &sliderData->sliderRect, srcCopy, nil );
	
	// Put the thumb rect into GWorld coordinates.
	// Move the thumb rect up to the top.
	OffsetRect( &thumbRect, 0, -(thumbRect.top-1) );
	// Get the offset of the thumb in the track (GWorld coordinates).
	temp = sliderData->sliderRect.left + (thumbRect.left - (**theControl).contrlRect.left);
	thumbRect.right = temp + (thumbRect.right-thumbRect.left);
	thumbRect.left = temp;
	// Copy the thumb to the work area.
	CopyBits ( (BitMap *) (*(sliderData->sliderPort->portPixMap)),
		(BitMap *) (*(sliderData->sliderPort->portPixMap)),
		&sliderData->sliderThumbRect, &thumbRect, srcCopy, nil );

	// If animating, wait for the VBL task to tell us
	// it's time to go, otherwise, just go.
	if ( gAnimating == true ) {  
		if ( gVBLRec.xDoTask == true ) {
 			gVBLRec.xDoTask = false;
			// Blit the track.
			CopyBits ( (BitMap *) (*(sliderData->sliderPort->portPixMap)),
				&((GrafPtr)theWindow)->portBits, &sliderData->sliderRect,
				&(**theControl).contrlRect, srcCopy, nil );
		}
	} else {
		CopyBits ( (BitMap *) (*(sliderData->sliderPort->portPixMap)),
			&((GrafPtr)theWindow)->portBits, &sliderData->sliderRect,
			&(**theControl).contrlRect, srcCopy, nil );
	}
	
	// Now unlock pixels.
	UnlockPixels( sliderData->sliderPort->portPixMap );
	
	// Unlock data and control handles.
	HUnlock( (Handle) sliderDataHnd );
	HUnlock( (Handle) theControl );
	
	// Restore the foreground and background colors.
	RGBForeColor( &savedForeColor );
	RGBBackColor( &savedBackColor );
}