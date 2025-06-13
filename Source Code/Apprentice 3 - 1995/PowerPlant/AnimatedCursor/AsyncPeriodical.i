// AsyncPeriodical.i
// Copyright ©1994 David Kreindler. All rights reserved.

// inline function definitions for class AsyncPeriodical, VBLPeriodical and TMPeriodical

// revision history
//	940212 DK: initial version

inline Boolean&
AsyncPeriodical::IsRunning() {
	return mTaskIsRunning;
}

inline AsyncPeriodical::Ticks
AsyncPeriodical::Interval() const {
	return mInterval;
}