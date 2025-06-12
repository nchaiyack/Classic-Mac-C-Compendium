/* Set the date and time -- Amoeba version */

#include <amtools.h>
#include <module/tod.h>
#include <ampolicy.h>

#include <time.h>

/* Set the date and time from a struct tm.
   The input time is in local time.
   If 'minchange' is zero, minutes and seconds are not taken
   from the input but from the current system time. */

int
setdatetime(tp, minchange)
	struct tm *tp;
	int minchange; /* nonzero if we must reset minutes and seconds, too */
{
	time_t new;
	errstat err;
	
	new = mktime(tp);
	if (!minchange)
		new = new/3600*3600 + time((time_t *)0)%3600;
	if (tod_settime(new, 0) != STD_OK) {
		return -1;
	}
	else
		return 0;
}
