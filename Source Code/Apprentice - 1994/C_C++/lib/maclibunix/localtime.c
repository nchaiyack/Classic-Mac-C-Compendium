/* Localtime emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987).
*/

#include "macdefs.h"
#include "time.h"

struct tm *
localtime(clock)
	long *clock;
{
	DateTimeRec date;
	static struct tm t;
	
	Secs2Date(*clock + TIMEDIFF, &date);
	t.tm_sec= date.second;
	t.tm_min= date.minute;
	t.tm_hour= date.hour;
	t.tm_mday= date.day;
	t.tm_mon= date.month - 1;
	t.tm_wday= date.dayOfWeek - 1;
	t.tm_year= date.year - 1900;
	return &t;
}
