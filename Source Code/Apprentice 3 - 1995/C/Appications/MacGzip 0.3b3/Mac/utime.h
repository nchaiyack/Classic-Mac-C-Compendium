/* utime.h - set file modification time */

/*
modification history
--------------------
01a,07nov94,ejo  written.
*/

#ifndef INC_utime_h
#define INC_utime_h

#include <time.h>

struct utimbuf
	{
	time_t actime;
	time_t modtime;
	};

int utime (const char *, const struct utimbuf *);

#endif /* INC_utime_h */
