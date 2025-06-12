/* Include file for localtime emulator.
   Public domain by Guido van Rossum, CWI, Amsterdam (July 1987). */

struct tm {
	int tm_sec;
	int tm_min;
	int tm_hour;
	int tm_mday;
	int tm_mon;
	int tm_year;
	int tm_wday;
	/*
	int tm_yday;
	int tm_isdst;
	*/
};

struct tm *gmtime(), *localtime();
