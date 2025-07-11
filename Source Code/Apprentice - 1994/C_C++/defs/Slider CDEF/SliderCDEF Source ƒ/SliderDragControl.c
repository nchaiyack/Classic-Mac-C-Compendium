#include "SliderCDEF.h"

// The only CDEF globals.
VBLRec gVBLRec;
Boolean gAnimating = false;
static DefVideoRec gVideoInfoRec;


// SliderDragControl
//
// Drags the control's indicator around. Called in response to the dragCntl message.
long SliderDragControl( short varCode, ControlHandle theControl, long param )
{
	Point			startPoint, mouseLoc;
	ControlPtr	theControlPtr;
	Rect			slopRect, thumbRect;
	long			offset;
	long			prevOffset;
	long			range;
	Rect			trackRect;
	short		trackWidth;
	short		thumbWidth;
	short		thumbHalfWidth;
	short		thumbCenter;
	float			ratio;
	OSErr		err;
	
	// Lock the control handle and dereference.
	HLock( (Handle) theControl );
	theControlPtr = *theControl;

	// Calculate the thumb rectangle at the old value.
	thumbRect = CalcThumbRect( theControl );
	
	// Calculate the track's rectangle and its width in pixels.
	thumbWidth = kThumbSize;
	thumbHalfWidth = thumbWidth / 2;
	trackRect = theControlPtr->contrlRect;
	InsetRect( &trackRect,  thumbHalfWidth + 1, 1 );
	trackWidth = trackRect.right - trackRect.left;
	
	// Calculate the range of conrol values allowed.
	range = theControlPtr->contrlMax - theControlPtr->contrlMin;
	
	// Calculate the ratio of control values to track width.
	// This shoul d probably be done using fixed-point numbers.
	ratio = (float) ((float) range) / ((float)trackWidth);

	// Calculate the slopRect for dragging the indicator.
	slopRect = theControlPtr->contrlRect;
	InsetRect( &slopRect, 0, -10 );

	// Get the initial mouse location.
	GetMouse( &startPoint );

	// Install a notification VBL task.
	err = VBLTaskSetup();
	if ( err != noErr ) {
		return;
	}
	
	// Let the draw routine know the slider is animating.
	gAnimating = true;

	while( StillDown() ) {
		GetMouse( &mouseLoc );
		offset = mouseLoc.h - startPoint.h;
		if ( offset != 0 && PtInRect( mouseLoc, &slopRect ) ) {
			// Calculate the new thumb rectangle.
			thumbRect.left += offset;
			thumbRect.right += offset;
			
			// Calculate the center of the thumb.
			thumbCenter = thumbRect.left + thumbHalfWidth;
			
			// Calculate the new control value.
			// (thumbCenter - trackRect.left) is how far into the track the thumbCenter is.
			theControlPtr->contrlValue = (thumbCenter - trackRect.left) * ratio;

			// Make sure the new value does not exceed the control's range.
			if ( theControlPtr->contrlValue > theControlPtr->contrlMax )
				theControlPtr->contrlValue = theControlPtr->contrlMax;
			if ( theControlPtr->contrlValue < theControlPtr->contrlMin )
				theControlPtr->contrlValue = theControlPtr->contrlMin;
			
			// Draw the control with the thumb at the new value only if
			// the control is visible.
			if ( theControlPtr->contrlVis ) {
				SliderDrawControl( varCode, theControl, param );
			}
			
			// Update the mouse location for the next iteration.
			startPoint = mouseLoc;
		}
	}
	
	// Turn off animation.
	gAnimating = false;
	
	// Remove the VBL task.
	err = RemoveVBLTask();

	// Unlock the control handle.
	HUnlock( (Handle) theControl );
	
	// Return a non-zero value so the control manager knows that indicator
	// dragging has been handled by the CDEF.
	return 1L;
}




// The next 3 routines come from Think Reference 2.0.
OSErr VBLTaskSetup( void )
{
	OSErr	err;
	
	GetVideoDefault( &gVideoInfoRec );
	
	gVBLRec.xDoTask = 0;
	
	gVBLRec.xVBLTask.qType = vType;
	gVBLRec.xVBLTask.vblAddr = (ProcPtr) MyVBLTask;
	gVBLRec.xVBLTask.vblCount = 1;
	gVBLRec.xVBLTask.vblPhase = 0;
	gVBLRec.vblA5 = (long) CurrentA5;

	err = SlotVInstall( (QElemPtr) &gVBLRec.xVBLTask, gVideoInfoRec.sdSlot );
	
	return err;
}




OSErr RemoveVBLTask( void )
{
	OSErr	err;
	
	err = SlotVRemove( (QElemPtr) &gVBLRec.xVBLTask, gVideoInfoRec.sdSlot );
	
	return err;
}




void MyVBLTask( void )
{
	VBLRecPtr	recPtr;
	GrafPtr		oldPort;
	long			curA5;

	recPtr = (VBLRecPtr) GetVBLRec();
	curA5 = SetA5( recPtr->vblA5 );

	recPtr->xDoTask = 1;
	// Reset task to the correct time.
	recPtr->xVBLTask.vblCount = kVBLCount;	
	
	curA5 = SetA5(curA5);
}