// waitfor.c
// Darrell Anderson

#include "waitfor.h"
#include <events.h>
#include <stdio.h>

void WaitFor(short *lock) {
	EventRecord event;
		
	while(*lock > 0) {
		
#if 1		
		// only accept suspend/resume (in the osMask class) events
		if( WaitNextEvent( osMask, &event, SLEEP_TIME, 0) ) {
			// got an event.. for the moment discard it
			;
		}
#endif

	}
}