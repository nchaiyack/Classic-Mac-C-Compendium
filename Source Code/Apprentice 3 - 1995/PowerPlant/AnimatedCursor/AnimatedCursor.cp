// AnimatedCursor.cp
// Copyright ©1994 David Kreindler. All rights reserved.

// definitions for classes AnimatedCursor, AsyncAnimCursor and AutoAnimCursor

// revision history
//	940319 DK: initial version
//	940326 DK: added class AutoAnimCursor

#ifdef __MWERKS__
#	pragma options align=mac68k
#endif

#include <events.h>
#include <lowmem.h>		// for LMGetCrsrBusy
#include <toolutils.h>	// for GetCursor

#ifdef __MWERKS__
#	pragma options align=reset
#endif

#include "animatedcursor.h"

#ifdef OUTLINE_INLINES
#	define inline
#	include "animatedcursor.i"
#	undef inline
#endif

#include "animatedcursor_utils.cp"

//
// AnimatedCursor
//

AnimatedCursor::AnimatedCursor(short acurResID) {

	// get the ACurHandle

	mACurH = GetACur(acurResID);
	if (mACurH != nil) {	// make sure that we have the ACurHandle

	// we modify the content of the ACurHandle in memory,
	// so we must not let the Resource Manager give the same handle to anybody else,
	// and we need to keep it in memory until we are done with it
	
		DetachResource(mACurH);
		HNoPurge(mACurH);
		HLockHi(mACurH);	// the following GetResource calls might move or purge memory

	// get the CursHandles and make them nonpurgeable so that we do not have to call the Memory Manager from the incrementing or drawing methods, which might be called at interrupt time
			
		for (short i = (**mACurH).numCursors - 1; i >= 0; --i) {
			(**mACurH).curs[i].cursH = GetCursor((**mACurH).curs[i].onDisk.resID);
			if ((**mACurH).curs[i].cursH != nil) {
				HNoPurge((**mACurH).curs[i].cursH);
			}
		}

	// let the ACurHandle float again until we need to use it

		HUnlock(mACurH);

	// range-check the starting index value, which the client might not have initialized in the resource file
	// note that (**mACurH).index values greater than (**mACurH).numCursors are probably unintentional;
	// but because they are not dangerous (Increment calculates modulo (**mACurH).numCursors), we leave it

		if ((**mACurH).index < 0 /* || (**mACurH).index >= (**mACurH).numCursors */) {
			(**mACurH).index = 0;
		}
	}
}

AnimatedCursor::~AnimatedCursor() {

	if (mACurH != nil) {											// make sure that we have the ACurHandle
	
	// dispose of the CursHandles

		for (short i = (**mACurH).numCursors - 1; i >= 0; --i) {
			if ((**mACurH).curs[i].cursH != nil) {					// make sure that we have the CursHandle
				ReleaseResource((**mACurH).curs[i].cursH);
			}
		}

	// dispose of the ACurHandle
	// note that we have called DetachResource on mACurH, so it is treated as a plain (non-resource) handle

		DisposeHandle(mACurH);
	}
}

void
AnimatedCursor::Increment(short increment) {

	if (mACurH != nil) {	// make sure that we have the ACurHandle
		
	// if 'increment' is negative, make it positive

		if (increment < 0) {
			increment = (**mACurH).numCursors - -increment % (**mACurH).numCursors;
		}

	// increment the index
		
		(**mACurH).index = ((**mACurH).index + increment) % (**mACurH).numCursors;

	// draw the cursor image

		Draw();
	}
}

void
AnimatedCursor::Draw() {

	if (mACurH != nil) {											// make sure that we have the ACurHandle
		if ((**mACurH).curs[(**mACurH).index].cursH != nil) {		// make sure that we have the CursHandle
			SetCursor(*(**mACurH).curs[(**mACurH).index].cursH);
		}
	}
}

//
// AsyncAnimCursor
//

AsyncAnimCursor::AsyncAnimCursor(short acurResID): AnimatedCursor(acurResID) {
}

AsyncAnimCursor::~AsyncAnimCursor() {

	// make sure that we are not still running when we go away
	
	Stop();
}

void
AsyncAnimCursor::Start(Ticks animTicks) {

	if (ACurH() != nil) {								// make sure that we have the ACurHandle
	
	// lock the ACurHandle and CursHandles, because the VBL task will be executing at interrupt time

		HLockHi(ACurH());
		for (short i = (**ACurH()).numCursors - 1; i >= 0; --i) {
			if ((**ACurH()).curs[i].cursH != nil) {		// make sure that we have each CursHandle
				HLockHi((**ACurH()).curs[i].cursH);
			}
		}
		
	// install the VBL task
		
		VBLPeriodical::Start(animTicks);
	}
}

void
AsyncAnimCursor::Stop() {

	if (ACurH() != nil) {								// make sure that we have the ACurHandle

	// remove the VBL task
	
		VBLPeriodical::Stop();

	// unlock the ACurHandle and CursHandles

		for (short i = (**ACurH()).numCursors - 1; i >= 0; --i) {
			if ((**ACurH()).curs[i].cursH != nil) {		// make sure that we have each CursHandle
				HUnlock((**ACurH()).curs[i].cursH);
			}
		}
		HUnlock(ACurH());
	}
}

void
AsyncAnimCursor::DoTask() {

	// check that the cursor is not busy and then draw the next frame

	if (!LMGetCrsrBusy()) {
		Increment(Interval() >= 0 ? 1 : -1);	// if the interval is negative, animate backwards
	}
}

//
// AutoAnimCursor
//

AutoAnimCursor::AutoAnimCursor(Ticks animTicks, SystemTicks delay, short acurResID): AsyncAnimCursor(acurResID), mDelay(delay) {

	Postpone();			// reset the activity timer
	Start(animTicks);
}
	
AutoAnimCursor::~AutoAnimCursor() {

	// make sure that we are not still running when we go away
	
	Stop();
}

void
AutoAnimCursor::DoTask() {

	// animate the cursor only after the specified period of inactivity and only if the mouse button is not down
	
	if (TickCount() >= mLastActivity + mDelay && !Button()) {
		AsyncAnimCursor::DoTask();
	}
}