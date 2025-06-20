/*
GetClicks.c
waits for a mouse click, and then counts clicks (e.g. double-click, triple-click). Each
click must arrive within the acceptable double-click time of the previous,
as set in the Control Panel. Returns the number of clicks, 1 or more.
HISTORY:
4/30/88 dgp	wrote it
3/31/90	dgp	cleaned up code and documentation.
8/24/91	dgp	Made compatible with THINK C 5.0.
3/30/91	dgp	use SndStop1() instead of obsolete Sound Driver.
1/25/93 dgp removed obsolete support for THINK C 4.
*/
#include "VideoToolbox.h"

short GetClicks(void)
{
	long ticks;
	EventRecord myEvent;
	short clicks;

	clicks=0;
	while(!GetNextEvent(mDownMask,&myEvent));
	SndStop1();	/* Stop sound on first click */
	clicks++;
	ticks=TickCount()+GetDblTime();
	while(!GetNextEvent(mUpMask,&myEvent));
	while(TickCount() < ticks)	/* wait as long a possible for another click */
		if(GetNextEvent(mDownMask,&myEvent)){
			clicks++;
			ticks=TickCount()+GetDblTime();
			while(!GetNextEvent(mUpMask,&myEvent));
		}
	return clicks;
}