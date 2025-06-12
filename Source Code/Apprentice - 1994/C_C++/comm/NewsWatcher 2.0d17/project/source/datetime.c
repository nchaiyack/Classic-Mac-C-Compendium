/*----------------------------------------------------------------------------

	datetime.c

	This module converts RFC822 date/time lines into Mac-format date/time
	strings in local time.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <Script.h>
#include <Packages.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#include "datetime.h"
#include "util.h"



/*----------------------------------------------------------------------------
	ParseNum
	
	Extract a long integer from a string.
	
	Entry:	*str = string.
	
	Exit:	function result = parsed number, or 0 if error.
			*str = pointer to first non-space character following parsed nubmer.
	
	A POINTER to a char* is passed in; it is adjusted to point to the
	next "word" after the number (skipping white space)

	Returns the number that was found, or 0.
----------------------------------------------------------------------------*/

static long ParseNum (char **str)
{
	long temp = 0;
	char *s = *str;
	
	while (*s && isdigit(*s))
		temp = temp * 10 + (*s++ - '0');
	while (*s && isspace(*s)) s++;
	*str = s;
	return temp;
}



/*----------------------------------------------------------------------------
	FindMonth

	Given a string that might be a month abbreviation, return the number
	of the month (starting with January = 1, ...).
	
	Entry:	str = month string.
	
	Exit:	function result = month ordinal, or 0 if can't parse.

	This isn't WorldScript-friendly, but it doesn't matter, since this
	function is only used for parsing RFC822 dates
----------------------------------------------------------------------------*/

static short FindMonth (const char *str)
{
	static const char *months[12] = {
		"JAN", "FEB", "MAR", "APR", "MAY", "JUN",
		"JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
	};
	short i;
	
	for (i = 0; i < 12; i++) {
		if (strcasecmp(str, months[i]) == 0) return i+1;
	}
	return 0;
}



/*----------------------------------------------------------------------------
	Parse822Date

	Parse a date/time that's in RFC822 header format:
	
	[Day, ] dd mmm [yy]yy hh:mm:ss [ GMT | (+|-) hhmm ]
	
	Entry:	date = RFC822 date/time string.
	
	Exit:	function result = number of seconds since January 1, 1904, 
				the standard Mac date convention.
			function result = 0 if the date could not be parsed
----------------------------------------------------------------------------*/

unsigned long Parse822Date (char *date)
{
	DateTimeRec dt;
	unsigned long result;
	char chunk[20];
	char *s, *p;
	short sign;
	long tzDelta;

	memset(&dt, 0, sizeof(dt));

	s = date;
	if (p = strchr(date, ',')) s = p+1;		/* Skip day of week */

	while (*s && isspace(*s)) s++;			/* dd */
	dt.day = ParseNum(&s);

	p = chunk;								/* mmm */
	while (*s && isalnum(*s) && p - chunk < sizeof(chunk)-1) *p++ = *s++;
	*p = 0;
	dt.month = FindMonth(chunk);

	while (*s && isspace(*s)) s++;			/* [yy]yy */
	dt.year = ParseNum(&s);
	if (dt.year < 21) {
		dt.year += 2000;
	} else if (dt.year < 100) {
		dt.year += 1900;
	}

	dt.hour = ParseNum(&s);					/* hh */
	if (*s == ':') {
		s++;
		dt.minute = ParseNum(&s);			/* mm */
	}
	if (*s == ':') {
		s++;
		dt.second = ParseNum(&s);			/* ss */
	} else {
		return 0;
	}
	
	if (strcasecmp(s, "GMT") == 0) {		/* GMT */
		s += 3;
		while (*s && isspace(*s)) s++;
	}

	if ((*s == '-' || *s == '+') && isdigit(*(s+1)) ) {		/* (+|-) hhmm */
		if (*s == '-') {
			/* If the delta is negative, we have to ADD time to get GMT */
			sign = 1;
		} else {
			sign = -1;
		}
		s++;
		tzDelta = ParseNum(&s);
		dt.hour   += sign * (tzDelta / 100);		/* Hours */
		dt.minute += sign * (tzDelta % 100);		/* Minutes */
	}
	
	if (*s != 0) return 0;

	/* Convert to seconds since 1/1/1904 */
	
	Date2Secs(&dt, &result);
	return result;
}


/*----------------------------------------------------------------------------
	Cleanup822Date
	
	Converts an RFC822 date/time string into a Mac-style date/time string
	in local time.
	
	Entry:	date = RFC822 date/time string.
			
	Exit:	date = Mac-style date/time string.
	
	If the string cannot be parsed or converted, or if the Map control
	panel is not installed, the string is unchanged.
----------------------------------------------------------------------------*/

void Cleanup822Date (char *date)
{
	MachineLocation	loc;	/* Location info for this machine */
	long gmtOffset;			/* Offset from GMT, in seconds */
	unsigned long secs;
	char *s, *p;
	Str255 time;

	/* Get our location. Return if Map control panel not installed. */
	
	ReadLocation(&loc);
	if (loc.latitude == 0 && loc.longitude == 0 && loc.gmtFlags.gmtDelta == 0) return;

	/* Get our offset in seconds from GMT. */
	
	gmtOffset = loc.gmtFlags.gmtDelta & 0x00FFFFFF;
	if ((gmtOffset & 0x00800000) != 0) gmtOffset |= 0xFF000000;

	/* Try to parse the date we were passed. */
	
	secs = Parse822Date(date);
	if (secs == 0) return;

	/* Correct for our timezone. */
	
	secs += gmtOffset;

	/* Convert it back into a Mac-style date/time string. */
	
	IUDateString(secs, abbrevDate, (StringPtr)date);
	p2cstr((StringPtr)date);
	strcat(date, " ");
	IUTimeString(secs, true, time);
	strcat(date, p2cstr(time));
}

