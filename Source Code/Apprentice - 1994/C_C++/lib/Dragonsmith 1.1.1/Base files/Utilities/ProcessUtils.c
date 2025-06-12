/*
	ProcessUtils.c
	
	Created	16 Jul 1992
	Modified	08 Aug 1992	Added FSpLaunchApp, LaunchedByFinder, and StartAppInForeground (adapted and
							moved from EventUtils.c, where it didn't really belong)
			11 Sep 1992	Removed FSpLaunchApp, LaunchedByFinder, and GetFinderPSN — they're not needed
							by Dragonsmith
							
	Copyright © 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.

*/

#include	"ProcessUtils.h"

OSErr StartAppInForeground (Boolean *inForeground)
{
	OSErr				err;
	ProcessSerialNumber	myPSN, frontPSN;
	EventRecord			event;

	// According to TechNote #180 —
	//	"Some applications like  to put up a “splash screen” to give the user something to look at while the application
	//	is loading. If your application does this and has the canBackground bit set in the size resource, then it should
	//	call _EventAvail several times (or _WaitNextEvent or _GetNextEvent) before putting up the splash screen, or
	//	the splash screen will come up behind the frontmost layer.  If the canBackground bit is set, MultiFinder will not
	//	not move your layer to the front until you call _GetNextEvent, _WaitNextEvent, or _EventAvail"
	// The magic number "several" apparently == 3

	// When you call WaitNextEvent, it's a use-it-or-lose-it deal (it == the event returned), so we call EventAvail instead
	
	(void) EventAvail (everyEvent, &event);
	(void) EventAvail (everyEvent, &event);
	(void) EventAvail (everyEvent, &event);
	
	// Now check to see if we actually ARE in the foreground…
	if ((err = GetCurrentProcess (&myPSN)) == noErr && (err = GetFrontProcess (&frontPSN)) == noErr)
		err = SameProcess (&myPSN, &frontPSN, inForeground);
	return err;
}

