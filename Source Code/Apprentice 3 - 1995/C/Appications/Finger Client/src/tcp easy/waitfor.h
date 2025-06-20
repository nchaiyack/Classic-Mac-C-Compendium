// waitfor.h
// Darrell Anderson

/*

Though the TCP routines in "tcp easy" claim to be synchronous, they in fact aren't.
The program that calls them blocks until the routine completes, but instead of blocking 
the whole system, this routine is called, spinning until the condition is met, yielding
time to other processes while we wait.

(synchronous pbio calls lock up the machine until they complete.. icky!)

*/

// minimum number of ticks the process should sleep between lock checks
#define SLEEP_TIME 15

// wait until lock becomes false, giving cycles out to other programs in the meantime
void WaitFor(short *lock);