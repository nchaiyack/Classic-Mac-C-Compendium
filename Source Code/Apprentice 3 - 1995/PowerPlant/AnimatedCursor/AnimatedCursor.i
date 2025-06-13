// AnimatedCursor.i
// Copyright ©1994 David Kreindler. All rights reserved.

// inline function definitions for class AnimatedCursor, AsyncAnimCursor and AutoAnimCursor

// revision history
//	940319 DK: initial version
//	940326 DK: added class AutoAnimCursor

inline void
AnimatedCursor::operator ++()  {
	Increment();
}

inline void
AnimatedCursor::operator --()  {
	Increment(-1);
}

inline void
AnimatedCursor::operator +=(short increment)  {
	Increment(increment);
}

inline void
AnimatedCursor::operator -=(short increment)  {
	Increment(-increment);
}

inline ACurHandle
AnimatedCursor::ACurH() {
	return mACurH;
}

inline void
AutoAnimCursor::Postpone() {
	mLastActivity = ::TickCount();
}