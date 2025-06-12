/* Time emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

#include "macdefs.h"
#include <OSUtils.h>

long
time(p)
	long *p;
{
	long secs;
	
	GetDateTime(&secs);
	secs -= TIMEDIFF;
	if (p != NULL)
		*p= secs;
	return secs;
}
