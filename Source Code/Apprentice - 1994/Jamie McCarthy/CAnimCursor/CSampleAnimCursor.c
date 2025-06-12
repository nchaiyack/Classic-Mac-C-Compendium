/*
 * CSampleAnimCursor.c
 * A sample subclass of CAnimCursor.
 *
 * Written by Jamie McCarthy.
 * Internet: k044477@kzoo.edu				AppleLink: j.mccarthy
 * Telephone:  800-421-4157 (9:00-5:00 Eastern time)
 *
 */



/********************************/

#include "CSampleAnimCursor.h"

/********************************/



void CSampleAnimCursor::ISampleAnimCursor(void)
{
	inherited::IQixableCursor(kStripedArrowID);
	setQixing(FALSE);
	setMode(kCACModeInterrupted);
}



void CSampleAnimCursor::useQixCursor(void)
{
	setQixing(TRUE);
	useAnimCursorID(kQixCursorID);
	setMode(kCACModeContinuous);
	setTicksBetweenCursors(6);
}



void CSampleAnimCursor::useStripedArrowCursor(void)
{
	setQixing(FALSE);
	useAnimCursorID(kStripedArrowID);
	setMode(kCACModeInterrupted);
	setTicksBetweenCursors(20);
}



void CSampleAnimCursor::useBeachballCursor(void)
{
	setQixing(FALSE);
	useAnimCursorID(kBeachballID);
	setMode(kCACModeContinuous);
	setTicksBetweenCursors(8);
}



void CSampleAnimCursor::useSpinningEarthCursor(void)
{
	setQixing(FALSE);
	useAnimCursorID(kSpinningEarthID);
	setMode(kCACModeContinuous);
	setTicksBetweenCursors(4);
}

