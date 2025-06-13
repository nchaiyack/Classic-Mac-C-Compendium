/*
Copyright й 1993,1994,1995 Fabrizio Oddone
еее еее еее еее еее еее еее еее еее еее
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"TrackThumb.h"

//#pragma segment Main

/*
	TrackThumb must be called as follows:
	partCode = TrackThumb(whichControl, localPoint, actionProc);

	actionProc must be declared as follows:
	pascal void actionProc(ControlHandle control, short value);
*/

short TrackThumb(ControlHandle control, Point start, void (*action)(ControlHandle, short))
{
enum {
kTooDistant = 23
};

EventRecord	dummyEventRec;
Rect	rct;
GrafPtr	savePort;
Point	pos;
register long	val, new;
register short	min, max, old, widthOrheight;

GetPort(&savePort);
SetPort((*control)->contrlOwner);
min	= GetControlMinimum(control);		/* min value */
max	= GetControlMaximum(control);		/* max value */
val	= GetControlValue(control);		/* cur value */
rct	= (*control)->contrlRect;	/* control's rectangle */

widthOrheight = rct.right - rct.left;
old = SHRT_MIN;
do {
	GetMouse(&pos);

	if (pos.v < rct.top + widthOrheight)
		pos.v = rct.top + widthOrheight;
	else if (pos.v > rct.bottom - widthOrheight)
		pos.v = rct.bottom - widthOrheight;
	new = val + (long)(max - min) * (pos.v - start.v) /
								(rct.bottom - rct.top - (widthOrheight * 3));
	if (new < min)
		new = min;
	if (new > max)
		new = max;
	if ((pos.h - rct.right > kTooDistant) || (rct.left - pos.h > kTooDistant))
		new = val;

	if (new != old) {
		SetControlValue(control, new);
		action(control, old = GetControlValue(control));
		}
	else {	/* give some time to other applications */
		SystemTask();
		(void)EventAvail(everyEvent, &dummyEventRec);
		}
	}
while(StillDown());
SetPort(savePort);
return kInIndicatorControlPart;
}

