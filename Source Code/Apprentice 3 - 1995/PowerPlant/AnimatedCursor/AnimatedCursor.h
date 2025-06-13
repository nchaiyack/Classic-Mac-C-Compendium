// AnimatedCursor.h
// Copyright ©1994 David Kreindler. All rights reserved.

// declarations of class AnimatedCursor, AsyncAnimCursor and AutoAnimCursor

// theory of operation
//	these classes provide cursor animation by cycling through an arbitrary-length sequence of cursor images stored as 'CURS' resources referenced from an 'acur' resource
//	class AnimatedCursor requires the user to call an increment function to advance the cursor sequence
//	class AsyncAnimCursor provides asynchronous animation by means of a vertical retrace interrupt task
//	class AutoAnimCursor provides asynchronous animation triggered by an inactivity timer

// warning
//	the incrementing and drawing functions of these classes might be called at interrupt time from a VBL task (inherited from class VBLPeriodical),
//	so those functions cannot use the Memory Manager, either directly or indirectly, and cannot rely on unlocked Handles;
//	for this reason, the AnimatedCursor constructor makes its ACurHandle and CursHandles nonpurgeable,
//	and AsyncAnimCursor's Start function locks them in memory (the Stop function unlocks them)

// revision history
//	940319 DK: initial version
//	940326 DK: added class AutoAnimCursor

// to do
//	the client should be able to specify a default frame frequency in the 'acur' resource; perhaps 'index' could be used
//	color cursors ('crsr' resources) should be supported
//	class AutoAnimCursor (or a subclass of it) should patch EventAvail, GetNextEvent and WaitNextEvent instead of requiring the client to call Postpone periodically

#ifndef ANIMATEDCURSOR_H
#	define ANIMATEDCURSOR_H

#	include "asyncperiodical.h"

//
// 'acur' resource structure
//

#	ifdef __MWERKS__
#		pragma options align=mac68k
#	endif

typedef struct {
	short numCursors;		// the number of 'CURS' resources in the sequence
	short index;			// 'CURS' currently being displayed (not used in resource)
	
	//	the sequence of 'CURS' resource IDs (on disk) or CursHandles (in memory)
	
	union {
		struct {
			short resID;	// 'CURS' resource id while on disk
			short filler;	// padding for the other word of the CursHandle part of the union
		} onDisk;
		CursHandle cursH;	// CursHandle of 'CURS' (resID) resource after being read into memory
	} curs[];
} **ACurHandle;

#	ifdef __MWERKS__
#		pragma options align=reset
#	endif

//
// class AnimatedCursor
// provides synchronous cursor animation
//

class AnimatedCursor {

	public:

		enum {
			acur_default = 128					// symbolic name for constructor parameter default (resource ID)
		};
		
		AnimatedCursor(short acurResID = acur_default);
		virtual ~AnimatedCursor();

	// animation methods
	
		void Increment(short increment = 1);	// 'increment' is the number of frames to advance the cursor; negative values are acceptable

		void operator ++();
		void operator --();
		
		void operator +=(short increment);
		void operator -=(short increment);
		
	protected:
	
	// drawing method; called by Increment and the increment/decrement operator functions

		virtual void Draw();
	
	// access to private ACurHandle member variable
	
		ACurHandle ACurH();

	private:
	
		ACurHandle mACurH;
};

//
// class AsyncAnimCursor
// adds asynchronous cursor animation functions to class AnimatedCursor by mixing in class VBLPeriodical
//

class AsyncAnimCursor: public AnimatedCursor, public VBLPeriodical {

	public:
	
		AsyncAnimCursor(short acurResID = acur_default);
		virtual ~AsyncAnimCursor();

	// asynchronous animation methods

		void Start(Ticks animTicks);	// virtual from VBLPeriodical
		void Stop();					// virtual from VBLPeriodical
	
	protected:
	
		void DoTask();					// virtual from VBLPeriodical
};

//
// class AutoAnimCursor
// automatically animates the cursor after a specified period of inactivity (activity is defined as a call to the Postpone function)
//

class AutoAnimCursor: public AsyncAnimCursor {
	
	public:
	
		typedef unsigned long SystemTicks;
	
	// inactivity timer's timebase and default delay
	
		enum {
			kDelayHz = 60,
			kDelayDefault = 2 * kDelayHz
		};
		
		AutoAnimCursor(Ticks animTicks, SystemTicks delay = kDelayDefault, short acurResID = acur_default);	// 'delay' is the period of inactivity before animation commences; 'animTicks' is the parameter to Start
		virtual ~AutoAnimCursor();
	
	// 'Postpone' resets the inactivity timer and stops any animation that is in progress
	// the client should call it whenever it calls EventAvail, GetNextEvent or WaitNextEvent, including from dialog/alert filter procs
	
		void Postpone();
		
	protected:
	
		void DoTask();						// virtual from VBLPeriodical
	
	private:
	
		volatile SystemTicks mDelay;
		volatile SystemTicks mLastActivity;
};

//
// inline function definitions
//

#	ifndef OUTLINE_INLINES
#		include "animatedcursor.i"
#	endif

#endif	// !defined (ANIMATEDCURSOR_H)