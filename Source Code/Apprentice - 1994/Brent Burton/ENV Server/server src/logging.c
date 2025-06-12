/*************************
** logging.c
**
** Module to do event logging
**************************/

#include "MacHeaders"
#include <Packages.h>
#include <stdio.h>
#include "logging.h"

#if LOGGING

static FILE *logfile;   /* module var for open logfile */
static short logging_on;

/********************************************/

/*****************************
** LogInit
**
** Initialize the logfile, etc.
******************************/
void LogInit(const char *name)
{
	short isinited=0;
	
	if (!isinited)
	{
		logfile = fopen( name, "w");
		isinited++;
	}
	logging_on = 1;
} /* LogInit */



/*****************
** LogStop  and  LogStart
**
** These small functions are for turning logging on or off.  Functions
** used to preserve module boundaries...
******************/
void LogStop(void)
{
	logging_on = 0;    /* turn off logging */
}

void LogStart(void)
{
	logging_on = 1;
}



/*******************
** LogClose
**
** Stop logging and close the log file.
********************/
void LogClose(void)
{
	if ( logfile != NULL)
	{
		fclose( logfile);
		logging_on = 0;
		logfile = NULL;
	}
}


/*******************
** LogEntry
**
** print an entry to the logfile, with a date & timestamp.
**
** LogError
** print an entry via LogEntry only if its a valid error.
********************/
void LogEntry( const char *s)
{
	if (logging_on)
	{
		unsigned long dateTime;
		Str255 dstring, tstring;
		
		GetDateTime(&dateTime);
		IUDateString(dateTime, shortDate, dstring);
		IUTimeString(dateTime, TRUE, tstring);
		PtoCstr(dstring);
		PtoCstr(tstring);
		fprintf(logfile, "%s %s: %s\n",dstring, tstring, s);
		fflush(logfile);
	}
}


void LogError(OSErr err, const char *s)
{
	if ( err != noErr)
		LogEntry(s);
}



#endif   /* LOGGING */

