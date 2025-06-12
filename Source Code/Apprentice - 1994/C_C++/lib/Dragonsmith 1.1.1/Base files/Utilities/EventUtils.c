/*
	EventUtils.c
	
	Event-related utility functions
	
	Created:	15 Jul 1992	YieldCPUTime
	Modified:	03 Sep 1992	Added IsCancelEvent
			
	Copyright © 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"EventUtils.h"
#include	<Script.h>

void YieldCPUTime (unsigned short ticks)
{
	EventRecord	event;
	register long	ticksEnd;
	
	ticksEnd = TickCount () + ticks;
	
	do
		(void) EventAvail (everyEvent, &event);
	while (TickCount () < ticksEnd);
			
}

Boolean IsCancelEvent (EventRecord *theEvent)
{
	// Adapted from Tech Note 263 ÒInternational CancelingÓ and Inside Macintosh Vol. IV, page 14Ð106
	
#define	kMaskModifiers	0xFE00		// Mask out the command key
#define	kMaskVirtualKey	0x0000FF00	// Mask for the virtual key
#define	kMaskASCII1		0x00FF0000	// Get the key out of the ASCII1 byte
#define	kMaskASCII2		0x000000FF	// Get the key out of the ASCII2 byte
#define	cPERIOD			'.'

	Ptr			kchrPtr;
	short		keyCode;
	long			virtualKey, keyInfo, lowChar, highChar, deadState;
	
	if (theEvent->modifiers & cmdKey) {
		virtualKey = (theEvent->message & kMaskVirtualKey) >> 8;
		keyCode = (theEvent->modifiers & kMaskModifiers) | virtualKey;
		// Get a pointer to the current 'KCHR' Ñ this will work only in System 7.0 or above
		kchrPtr = (Ptr) GetEnvirons (smKCHRCache);
		deadState = 0;
		keyInfo = KeyTrans (kchrPtr, keyCode, &deadState);
		lowChar = keyInfo & kMaskASCII2;
		highChar = (keyInfo & kMaskASCII1) >> 16;
		return lowChar == cPERIOD || highChar == cPERIOD;
	} else
		return FALSE;
}

