// Test.cp
// Copyright ©1994 David Kreindler. All rights reserved.

// demonstration application

// revision history
//	940319 DK: initial version
//	940326 DK: added AutoAnimCursor example

// what happens in main
//	initialize QuickDraw
//	construct a (default) AsyncAnimCursor
//	animate synchronously until the user clicks the mouse button
//	animate asynchronously (and backwards) while the user holds the mouse button down
//	delete the AsyncAnimCursor
//	construct a (default) AutoAnimCursor
//	animate automatically (after the default delay), until the user clicks the mouse button
//	delete the AutoAnimCursor

#ifdef __MWERKS__
#	pragma options align=mac68k
#endif

#include <events.h>		// for Button
#include <osutils.h>	// for Delay

#ifdef __MWERKS__
#	pragma options align=reset
#endif

#include <animatedcursor.h>	// for cursor animation classes

void main() {

	enum {
		kSysHz = 60,											// Delay timebase
		kCursorHz = 15,											// frequency of cursor "frames"
		kCursorDelay = kSysHz / kCursorHz,						// cycles/frame for the Delay timebase
		kCursorSpeed = AsyncAnimCursor::kTimerHz / kCursorHz	// cycles/frame for the AsyncAnimCursor timebase
	};

	::InitGraf(&qd.thePort);

	// do some synchronous cursor animation until the user clicks the mouse button
	
	AsyncAnimCursor* cursor = new AsyncAnimCursor;
	
	do {
		++*cursor;
		
		long delay;						// unused parameter to Delay
		::Delay(kCursorDelay, &delay);
	} while (!Button());
	
	// do some asynchronous cursor animation until the user releases the mouse button
	
	cursor->Start(-kCursorSpeed);
	
	while (Button()) {
		// let the VBL task do the animation
	}
	
	delete cursor;		// the destructor calls Stop, if necessary
	
	// show the arrow cursor to clarify the AutoAnimCursor's delay
	
	::InitCursor();
	
	// do some automatic cursor animation until the user clicks the mouse button
	
	cursor = new AutoAnimCursor(kCursorSpeed);
	
	while (!Button()) {
		// let the VBL task do the animation, after the specified delay
	}
	
	delete cursor;	
}