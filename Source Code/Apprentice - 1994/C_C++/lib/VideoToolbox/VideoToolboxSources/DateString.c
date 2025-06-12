/*
DateString.c
returns a pointer to a string representing the time in a form suitable for
use as a unique filename extension, e.g. "890625015959", that 
sorts in chronological order. The supplied time should be obtained by
calling Apple's GetDateTime() or Standard C's time(). 
Both return seconds since midnight January 1, 1904.

HISTORY:
1989 dgp wrote it.
2/25/94	dgp	Noted above how to get the time.
3/19/94	dgp Removed the dots because the string was too long, using up nearly
	the maximum length a filename.
*/
#include "VideoToolbox.h"

char *DateString(unsigned long seconds)
{
	DateTimeRec t;
	static char dateString[26];
	
	Secs2Date(seconds,&t);
	sprintf(dateString,"%02d%02d%02d%02d%02d%02d",t.year-1900,t.month,t.day,t.hour,t.minute,t.second);
	return dateString;
}