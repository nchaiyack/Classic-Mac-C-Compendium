/*
 * SkelPause ()
 *
 * Pause process execution for the given length of time, measured in ticks
 * (60ths of a second).  If running in a multitasking environment, gives
 * time to other processes, using the wait value that's appropriate for
 * whether the application's in the foreground or background.
 */

# include	"TransSkel.h"


pascal void
SkelPause (long ticks)
{
long	current;
long	waitTime;
EventRecord	event;

	if (SkelQuery (skelQInForeground))
		SkelGetWaitTimes (&waitTime, (long *) nil);
	else
		SkelGetWaitTimes ((long *) nil, &waitTime);
	current = TickCount ();
	while (TickCount () < current + ticks)
	{
		if (SkelQuery (skelQHasWNE))
			(void) WaitNextEvent (0, &event, waitTime, nil);
	}
}
