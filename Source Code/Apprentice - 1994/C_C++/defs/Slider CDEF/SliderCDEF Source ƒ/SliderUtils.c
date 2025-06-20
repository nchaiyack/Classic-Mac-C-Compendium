#include "SliderCDEF.h"

// CalcThumbRect
//
// Calculates the thumb rectangle in window (local) coordinates.
// Should be changed to use fixed-point math.
Rect CalcThumbRect( ControlHandle theControl )
{
	Rect		trackRect;
	short	trackWidth;
	Rect		thumbRect;
	short	thumbWidth;
	short	thumbHalfWidth;
	short	thumbCenter;
	short	val;
	short	max;
	short	min;
	float		ratio;
	
	// Lock the control handle.
	HLock( (Handle) theControl );
	
	// Calculate the thumb width and half-width;
	thumbWidth = kThumbSize;
	thumbHalfWidth = thumbWidth / 2;
	
	// Get a copy of the control's rectangle.
	trackRect = (**theControl).contrlRect;
	
	// Shrink it to the thumb rectangle.
	InsetRect( &trackRect,  thumbHalfWidth + 1, 1 );
	trackWidth = trackRect.right - trackRect.left;
	
	// Get the control's values.
	val = (**theControl).contrlValue;
	max = (**theControl).contrlMax;
	min = (**theControl).contrlMin;
	
	// Unlock the control handle.
	HUnlock( (Handle) theControl );
	
	// Calculate the control's fractional value.
	ratio = ((float)val) / ((float)(max-min));
		
	// Calculate the thumb center.
	thumbCenter = trackRect.left + (short) (ratio * trackWidth);

	// Setup the thumb rectangle.
	SetRect( &thumbRect, thumbCenter - thumbHalfWidth,
		trackRect.top, thumbCenter + thumbHalfWidth, trackRect.bottom );
	
	return thumbRect;
}




// SliderGetControlColors
//
// Fills in a structure full of colors used by the CDEF to draw the control.
// This routine should use the colors defined by the 'Colors' control panel,
// obtainable through the window's auxilliary color table.
void SliderGetControlColors( ControlHandle theControl, SliderColors *colors )
{
	colors->black.red = colors->black.green = colors->black.blue = 0x0000;
	colors->white.red = colors->white.green = colors->white.blue = 0xFFFF;

	colors->darkGray.red = colors->darkGray.green = colors->darkGray.blue = 0x4444;
	colors->lightGray.red = colors->lightGray.green = colors->lightGray.blue = 0xCCCC;

	colors->lightColor.red = 0xCCCC;
	colors->lightColor.green = 0xCCCC;
	colors->lightColor.blue = 0xFFFF;
	
	colors->darkColor.red = 0x9999;
	colors->darkColor.green = 0x9999;
	colors->darkColor.blue = 0xCCCC;
}




// IsHorizontal
//
// Looks at the control's contrlRect to see whether its width is greater than its height.
// If true, the control is horizontal, otherwise it's vertical.
Boolean IsHorizontal( ControlHandle theControl )
{
	// Make a local copy of the control's rect.
	Rect	theRect = (**theControl).contrlRect;
	
	// Compare the control's width to its height.
	// A perfect place for a conditional expression...
	return ( (theRect.right-theRect.left) > (theRect.bottom-theRect.top) ) ? true : false;
}