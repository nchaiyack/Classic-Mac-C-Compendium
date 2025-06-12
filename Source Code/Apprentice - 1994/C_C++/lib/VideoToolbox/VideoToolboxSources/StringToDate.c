/*
StringToDate.c
Reads date such as "6/30/91", loads date structure, and returns secs since 1/1/1904.
Returns NAN if date could not be read.

HISTORY:
7/27/91 dgp	wrote it
8/5/91	dgp	made compatible with MPW C 3.2
8/24/91	dgp	Made compatible with THINK C 5.0.
12/13/92 dgp Removed obsolete support for THINK C 4.
*/
#include "VideoToolbox.h"
#include <Script.h>

double StringToDate(char *string,DateTimeRec *datePtr);

double StringToDate(char *string,DateTimeRec *datePtr)
{
	LongDateRec longDate;
	LongDateCvt longSecs;
	double secs;
	static DateCacheRecord theCache;
	static Boolean firstTime=1;
	int error;
	long used;
	
	longDate.ld.era=longDate.ld.year=longDate.ld.month=longDate.ld.day=0;
	longDate.ld.hour=longDate.ld.minute=longDate.ld.second=longDate.ld.dayOfWeek=0;
	if(firstTime){
		InitDateCache(&theCache);
		firstTime=0;
	}
	error=String2Date(string,strlen(string),&theCache,&used,&longDate);
	*datePtr=longDate.od.oldDate;
	if(error)return 0./0.;
	LongDate2Secs(&longDate,(LongDateTime *)&longSecs);
	secs=HiWord(longSecs.hl.lHigh);
	secs*=(double)0x10000;
	secs+=(unsigned)LoWord(longSecs.hl.lHigh);
	secs*=(double)0x10000;
	secs+=(unsigned)HiWord(longSecs.hl.lLow);
	secs*=(double)0x10000;
	secs+=(unsigned)LoWord(longSecs.hl.lLow);
	return secs;
}