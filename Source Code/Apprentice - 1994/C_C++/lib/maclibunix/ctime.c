/* Ctime emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987).
*/

#include "macdefs.h"

static char dayname[]= "SunMonTueWedThuFriSat";
static char monthname[]= "JanFebMarAprMayJunJulAugSepOctNovDec";

char *
ctime(secs)
	long *secs;
{
	DateTimeRec date;
	static char buffer[26];
	
	Secs2Date(*secs + TIMEDIFF, &date);
	sprintf(buffer, "%.3s %.3s %2d %02d:%02d:%02d %4d\n",
		dayname + 3*(date.dayOfWeek-1),
		monthname + 3*(date.month-1),
		date.day,
		date.hour,
		date.minute,
		date.second,
		date.year);
	return buffer;
}
