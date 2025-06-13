

/*			dcpsys.c

			Revised edition of dcp

			Stuart Lynne May/87

			Copyright (c) Richard H. Lamb 1985, 1986, 1987
			Changes Copyright (c) Stuart Lynne 1987
			Portions Copyright © David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

*/
/* "DCP" a uucp clone. Copyright Richard H. Lamb 1985,1986,1987 */
/* Get the next system, and other support routines  */
#include "dcp.h"
#include <string.h>
#include <Serial.h>
/*#define PROTOS  "trkg"*/
#define PROTOS  "gfG"
#define MAXLOGTRY       3

Proto Protolst[] = {
		'g', ggetpkt, gsendpkt, gopenpk, gclosepk, gfilepkt, geofpkt, grdmsg,  gwrmsg, gsendresp,
		'f', fgetpkt, fsendpkt, fopenpk, fclosepk, ffilepkt, feofpkt, frdmsg,  fwrmsg, fsendresp,
		'G', ggetpkt, gsendpkt, gopenpk, gclosepk, gfilepkt, geofpkt, grdmsg,  gwrmsg, gsendresp,
#ifdef PROTA
		'a', agetpkt, asendpkt, aopenpk, aclosepk, afilepkt, aeofpkt, ardmsg,  awrmsg, asendresp,
#endif
/*
	    'k', kgetpkt, ksendpkt, kopenpk, kclosepk,
	    'r', rgetpkt, rsendpkt, ropenpk, rclosepk,
	    't', tgetpkt, tsendpkt, topenpk, tclosepk,
*/
		 '0'};

#define EOTMSG "\004\r\004\r"

procref         getpkt, sendpkt, openpk, closepk, filepkt, eofpkt, getmsg, sendmsg, sendresp;

#ifdef THINK_C
int checkcron(void);
#endif

#include "dcpsys.proto.h"

void hangup()
{
	if ( useHardwareFlowControl ) {
		printmsg(2, "HAYES attention/hangup");
		sendexpect("\\d\\p+++\\c","OK",5);
		sendexpect("\\pAAATH","OK",2);
	}
}

void zzz(int nsecs)
{
#ifdef MULTIFINDER
	long int endTime = TickCount() + nsecs * 60L;
	while (TickCount() < endTime) {
	  if (Check_Events(10)) 
	    if (Main_State == Abort_Program) {
	    	hangup();
	    	(*currentConnection->Close)(TRUE);
	    	exit(-1);
	    }
	  }
#else
	sleep(nsecs);
#endif
}


/**/

/***************************************************************/
/***            Sub Systems             */
/*
**
**getsystem
** Process an "systems" file entry (like L.sys)
*/
getsystem(char *rmthost)
{
	int	i,blankline;
	char tmpline[BUFSIZ],*p;
	
	*sysline = 0;	/* empty out sysline */
	while(fgets( tmpline, BUFSIZ, fsys) != (char *)NULL) {
		for(blankline=TRUE, p=tmpline; *p; p++) {
			if(!isspace(*p)) {
				blankline = FALSE;
				break;
			}
		}
		if(blankline)			/* skip blank lines */
			continue;
		if(*p == '#')			/* skip comment lines */
			continue;
		
		if(tmpline[strlen(tmpline)-2] == '\\') {
			tmpline[strlen(tmpline)-2] = ' ';	/* got continuation line */
			tmpline[strlen(tmpline)-1] = (char)NULL;	/* throw away the newline char */
			strcat(sysline, tmpline);
			continue;
		}
		else {
			strcat(sysline, tmpline);		/* not continued, take it and move on */
			break;
		}
	}
	
	if(strlen(sysline) == 0)
		return('A');
		
	for (i = 0; i < NFLDS; i++) flds[i] = NULL;

	printmsg( 2, "%s", sysline );

	kflds = getargs( sysline, flds );
	
	/* if this system isn't the one we're calling or we're not calling
	   a special set of systems then return 'I' and try the next one */
	if ( strcmp(rmthost, flds[FLD_REMOTE] ) != SAME &&
		 strcmp(rmthost, "list") != SAME &&
		 strcmp(rmthost, "any")  != SAME &&
		 strcmp(rmthost, "all")  != SAME &&
		 strcmp(rmthost, "cron") != SAME &&
		 strcmp(rmthost, "need") != SAME &&
		 strcmp(rmthost, "auto") != SAME)
		return( 'I' );
	
	if (strcmp(rmthost, "INCOMING") == SAME && remote == MASTER) {
		return ('I');
	}
		
	strcpy( rmtname, flds[FLD_REMOTE] );
	cctime = flds[FLD_CCTIME];
	strcpy( device, flds[FLD_DEVICE] );

	/* strcpy( type, flds[FLD_TYPE] ); */
	strcpy( speed, flds[FLD_SPEED] );
	strcpy( proto, flds[FLD_PROTO] );
	strcpy( phone, flds[FLD_PHONE] );

	if (debuglevel > 3)
		for (i = FLD_EXPECT; i < kflds; i += 2)
			fprintf( stderr, "expect[%02d]:\t%s\nsend  [%02d]:\t%s\n",
				i, flds[i], i+1, flds[i+1] );

	printmsg( 2, "rmt= %s ctm= %s", rmtname, flds[FLD_CCTIME] );
	printmsg( 2, "dev= %s ", device );
	printmsg( 2, "spd= %s pro= %s", speed, proto );
	printmsg( 2, "phn= %s ", phone );
	
	if (fw != (FILE *)NULL) {
		fclose(fw);
		fw = (FILE *)NULL;
	}
	
	if (strcmp(rmthost, "list") == SAME) {
		return 'I';
	}
	
	/* first make sure it's ok to call the system at this time, if so,
	   then make sure it's a system we are supposed to call */
	strcpy(callcause, "user request");
	if ( ((strcmp(rmthost, "all")   == SAME && strcmp(rmtname, "INCOMING") != SAME) ||
		  (strcmp(rmthost, rmtname) == SAME) ||
	 	  ((strcmp(rmthost, "any") == SAME) && scandir() == 'S') ||
	 	  ((strcmp(rmthost, "cron") == SAME) && checkcron() == 'S') ||
	 	  ((strcmp(rmthost, "need") == SAME || strcmp(rmthost, "auto") == SAME) && (scandir() == 'S' || checkcron() == 'S')))
	   )
	{
		if (fw != (FILE *)NULL) {
			fclose(fw);
			fw = (FILE *)NULL;
		}
		if ( checktime(cctime) ) {
			return('S');  	/* startup this system */
		} else {
			return('T');  	/* wrong time */
		}
	} else {
		if (fw != (FILE *)NULL) {
			fclose(fw);
			fw = (FILE *)NULL;
		}
		return('I');	/* don't call this system, try the next one */
	}
}


/**/
/*
**
**checkname
** Do we know the guy ?
*/
checkname(char name[])
{
	FILE *ff;
	char line[BUFSIZ], tmp[20]; /* can change to 8 if %8s works */

	if ( ( ff = FOPEN( s_systems, "r", 't' )) == (FILE *)NULL ) {
		printmsg(0, "Error %d, can't open %s file!", errno, s_systems);
		return( FAILED );
	}

	while ( fgets( line, BUFSIZ, ff ) != (char *)NULL ){
		sscanf( line, "%8s ", tmp );
		printmsg( 3, "rmt= %s sys= %s", name, tmp );
		if (strncmp(line, "ANONYMOUS", 9) == 0 || strncmp( tmp, name, 7 ) == 0 ) {
			fclose( ff );
			return ( OK ); /*OK I like you */
		}
	}
	fclose( ff );
	return( FAILED ); /* Who are you ? */

}


void Schedule_To_List(char *Schedule, datalist Schedule_List, int Min, int Max);
void Compute_Next_Call_Time(status_record Last, schedule_record Schedule, struct tm *Next );
int Time_Less_Than_Or_Equal(struct tm *Left, struct tm *Right);

int Time_Less_Than_Or_Equal(struct tm *Left, struct tm *Right)
{
	/* check the months first */
	if (Left->tm_mon < Right->tm_mon) return TRUE;
	if (Left->tm_mon > Right->tm_mon) return FALSE;
	
	/* months are the same, check the day */
	if (Left->tm_mday < Right->tm_mday) return TRUE;
	if (Left->tm_mday > Right->tm_mday) return FALSE;
	
	/* days are the same, check the hour */
	if (Left->tm_hour < Right->tm_hour) return TRUE;
	if (Left->tm_hour > Right->tm_hour) return FALSE;
	
	/* hours are the same, check the minute */
	if (Left->tm_min > Right->tm_min) return FALSE;
	return TRUE;
}


void Schedule_To_List(char *Schedule, datalist Schedule_List, int Min, int Max)
{
	/* take a character string describing this schedule entry and
	   convert it to a list of times that are acceptable, see format
	   and examples below.
	   
	   Format (no spaces allowed in actual string):
		   number :== digit{digit}
		   range  :== number '-' number
		   item   :== range | number	
		   entry  :==  '*' | item {, item}
	   Example:
	       "*"  "1-5,8"  "1,3,5,7,9,11" "0-7,17-23"
	*/
	int i,k,m;
	char *p;
	
	if (strcmp(Schedule,"*")==SAME) {
		for (i=0, k=Min; k<=Max; k++, i++) 
			Schedule_List[i] = k;
		return; 
	}
	
	/* scan for list entries */
	p = Schedule;
	i = 0;
	while (*p) {
		if (isdigit(*p)) {
			/* got a number */
			sscanf(p, "%d", &k);		/* got a list entry */
			Schedule_List[i++] = k;		/* put it in the list */
			while (isdigit(*p)) p++;	/* skip past number */
			
		} else if (*p == ',') {
			/* got a comma */
			p++;		/* skip the comma */
		} else if (*p == '-') {
			/* got a dash, must be a range */
			p++;		/* skip the dash */
			sscanf(p, "%d", &m);		/* get the end of the range */
			while (k <= m) {			/* put entries for the range in the list */
				Schedule_List[i++] = k++;
			}
			while (isdigit(*p)) p++;	/* skip past number */
		} else {
			p++;
		}
		Schedule_List[i] = -1;
	}
}	

int Validate_Value(int Value, datalist Valid_List);
int Validate_Value(int Value, datalist Valid_List)
{
	int i = 0;

	while ((Valid_List[i] < Value) && (Valid_List[i] >= 0))
		i++;
	
	if (Valid_List[i] < 0) {
		/* didn't find the value, use the first entry */
		return Valid_List[0];
	} else {
		/* found the value or the next larger one, it's ok to use it */
		return Valid_List[i];
	}
}

int Add_Field(int Last, int *Next, datalist Valid_List, int *Carry);
int Add_Field(int Last, int *Next, datalist Valid_List, int *Carry)
{
	*Next = Last + *Carry;		/* add the carry in (may be 0) */
	
	/* make sure the time is a valid value for this field */
	*Next = Validate_Value(*Next, Valid_List);
	
	/* if the value is lower than the last time, set the carry flag (overflow) */
	if (*Next < Last) {
		*Carry = 1;		/* to make it less we had to wrap around, causing a carry */
	} else if (*Next == Last && *Carry == 1) {
		/* we had a carry but it didn't increment; it must have wrapped all the
		   way back around to where we started, this should cause a carry */
		*Carry = 1;
	} else {
		*Carry = 0;
	}
		
	/* if the value is different from last time, then return 1 to indicate it
	   must reset the less significant fields to their minimum values */
	if (*Next != Last || *Carry == 1) return 1;
	return 0;
}

int Day_Of_Week(int Month, int Day, int Year);
int Day_Of_Week(int Month, int Day, int Year)		/* 1904-2040 */
/* Returns Day_Of_Week where 1=Mon...5=Fri, 6=Sat, 7=Sun */
{
	DateTimeRec Tmp_DTR;
	long Secs;
	
	Tmp_DTR.year   = (Year >= 80 ? 1900+Year : 2000+Year);
	Tmp_DTR.month  = Month;
	Tmp_DTR.day    = Day;
	Tmp_DTR.hour   = 0;
	Tmp_DTR.minute = 0;
	Tmp_DTR.second = 0;
	Tmp_DTR.dayOfWeek = 1;
	
	Date2Secs(&Tmp_DTR, &Secs);	/* day of week not needed */
	Secs2Date(Secs, &Tmp_DTR);	/* day of week return in conversion */
	
	if (Tmp_DTR.dayOfWeek == 1) 
		return 7;  /* Sunday */
	else
		return Tmp_DTR.dayOfWeek - 1;
}

void Compute_Next_Call_Time(status_record Last, schedule_record Schedule, struct tm *Next)
{
	schedule_list_record Valid_Lists;	
	int carry;				/* start with a carry to increment to next time value */
	int field = 0;
	struct tm Last_Time;
	
	Last_Time = *localtime(&Last.time_secs);
	Last_Time.tm_mon++;		/* make months 1 based */
	*Next = Last_Time;
	
	Schedule_To_List(Schedule.min,     Valid_Lists.min,     0, 59);
	Schedule_To_List(Schedule.hour,    Valid_Lists.hour,    0, 23);
	Schedule_To_List(Schedule.day,     Valid_Lists.day,     1, 31);
	Schedule_To_List(Schedule.month,   Valid_Lists.month,   1, 12);
	Schedule_To_List(Schedule.weekday, Valid_Lists.weekday, 1, 7);

	/* find the next larger valid time for the field */
	carry = 1;
	if (Add_Field(Last_Time.tm_min, &Next->tm_min, Valid_Lists.min, &carry)) {
		/* field was set backwards, we must update the less significant fields
			to their minimum values */
		/* there are no less significant fields (we don't use seconds at all) */
	}

	if (Add_Field(Last_Time.tm_hour, &Next->tm_hour, Valid_Lists.hour, &carry)) {
		/* field was set backwards, we must update the less significant fields to their
		   minimum values */
		Next->tm_min = Valid_Lists.min[0];
	}

  try_the_next_day:
  	
	if (Add_Field(Last_Time.tm_mday, &Next->tm_mday, Valid_Lists.day, &carry)) {
		/* field was set backwards, we must update the less significant fields to their
		   minimum values */
		Next->tm_hour = Valid_Lists.hour[0];
		Next->tm_min  = Valid_Lists.min[0];
	}
	
	if (Add_Field(Last_Time.tm_mon, &Next->tm_mon, Valid_Lists.month, &carry)) {
		/* field was set backwards, we must update the less significant fields to their
		   minimum values */
		Next->tm_mday = Valid_Lists.day[0];
		Next->tm_hour = Valid_Lists.hour[0];
		Next->tm_min  = Valid_Lists.min[0];
	}
	
	/* see if day of week of the computed next call time is a valid 
	   day of the week given the days that are allowed */
	Next->tm_wday = Day_Of_Week(Next->tm_mon, Next->tm_mday, Next->tm_year);
	if (Validate_Value(Next->tm_wday, Valid_Lists.weekday) != Next->tm_wday) {
	  /* weekday doesn't match list of valid weekdays, so try the next 
	     possible day that we could call and see if it falls on the right
	     weekday.  Keep doing this until we find a valid day that is on a valid
	     weekday. */
	  Last_Time = *Next;
	  carry = 1;		/* so day will get incremented */
	  goto try_the_next_day;
	}
	
	Next->tm_mon--;		/* make months 0 based again */
	
	/*** weekday restrictions are not being handled yet ***/
}

/* get status information from file.  Returns TRUE on success, FALSE on failure. */

int GetStatus(void)
{
	FILE *fp;
	char line[BUFSIZ];
	char stfile[80];			/* constructed status file name */
	int n;
	if (strcmp(Last.name, rmtname) == SAME) {
		return TRUE;
	}
	sprintf(stfile, "%s%s", s_statfile, rmtname);
	
	/* if there is no status file, assume it has never been called */
	if ((fp=FOPEN(stfile, "r", 't')) == (FILE *)NULL) {
		printmsg( 1, "No status file for system %s", rmtname );
		goto fakeit;
	}
	
	n = 0;
	if (fgets(line, BUFSIZ, fp) != (char *)NULL) {
		/* got status line into buffer */
		/* line format:
				ctime system status [failures]
		   ctime is epoch time in seconds from time function
		   status may be OK or FAILED */
		Last.failures = 0;
		n = sscanf(line, "%ld %s %s %d",
				   &Last.time_secs, Last.name, Last.status, &Last.failures);
		if (n < 3 ||
			strcmp(rmtname, Last.name) != SAME) n = 0;
	}
	fclose(fp);
	if (n == 0) {
		printmsg( 1, "Unable to read status line" );
fakeit: strcpy(Last.name, rmtname);
		strcpy(Last.status, "OK");
		Last.failures = 0;
		Last.time_secs = 0;
		return FALSE;
	}
	return TRUE;
}

/**/
/*
**
** checkcron()
** check if we should call a system at this time.
**
** returns 'S' if system needs be called
** returns 'I' if system doesn't need to be called
**
** "need" is based on schedule file not by what is spooled, use "any"
** if you want to send spooled data out.
**
** system needs to be called if any of the following conditions are met:
**
** (1) status file "ST.<system>" doesn't exist
** (2) status file indicates last call failed and the elapsed time since
**     the failure exceeds the retrytime entry in the schedule file.
** (3) status file indicates last call was ok and according to the
**     schedule file there should have been a call during the time when
**     the last called occurred and the current time.
*/
int checkcron(void)
{
	FILE *fp;
	char line[BUFSIZ];
	char stfile[80];			/* constructed status file name */
	int n;
	time_t Cur_Time_Secs, Retry_Expiration;
	
	schedule_record Schedule;
	struct tm Next, Current_Time;
	
	/* got a real problem if schedule file is missing */
	if ((fp=FOPEN(s_schedule, "r", 't')) == (FILE *)NULL) {
		printmsg( 1, "Schedule file '%s' missing", s_schedule );
		return('I');		/* no schedule, so don't call anyone */
	}
	
	n = 0;
	while (n==0 && fgets(line, BUFSIZ, fp) != (char *)NULL){
		/* line format:
			min hour day month weekday system [retrytime] */
		if (line[0] != '#') {
			Schedule.retrytime = 1;
			n = sscanf(line, "%s %s %s %s %s %s %d",
					   Schedule.min, Schedule.hour, Schedule.day,
					   Schedule.month, Schedule.weekday, Schedule.name,
					   &Schedule.retrytime);
			if (n < 6 ||
				strcmp(rmtname, Schedule.name) != SAME) n = 0;
		}
	}
	fclose(fp);
	if (n == 0) {
		printmsg( 1, "System %s not in schedule", rmtname );
		return ('I');	/* didn't find the system in schedule, don't call it */
	}
	
	/* we have the schedule now, now we need the current status */
	if (!GetStatus()) {
		strcpy(callcause, "scheduled call");
		return('S');
	}
	
	if (strcmp(Last.status,"OK") != SAME  && Schedule.retrytime) {
		/* no retries if retrytime is 0, just the scheduled calls */
		
		/* last call failed, see if failed during current run.  If so,
		   try again (additional phone number) */
		if (Last.time_secs >= runStartSecs) {
			return ('S');
		}
		/* Failed during previous run.  See if the retry time has expired */
		Retry_Expiration = Last.time_secs + Schedule.retrytime * 60;
		time(&Cur_Time_Secs);
		if (Cur_Time_Secs >= Retry_Expiration) {
			printmsg( 1, "RETRY time for %s", rmtname );
			strcpy(callcause, "retry of scheduled call");
			return ('S');		/* retry time past, call system again */
		} else {
			printmsg( 1, "Retry time for %s not reached yet", rmtname );
			return ('I');		/* not time yet, don't call system */
		}
	}
	
	/* last call was successful, see if it's time to call this system again */
	Compute_Next_Call_Time( Last,		/* time of last successful call */
							Schedule,	/* time schedule pattern */
							&Next);		/* next time it should be called */
							
	time(&Cur_Time_Secs);
	Current_Time = *localtime(&Cur_Time_Secs);
	
	if (Time_Less_Than_Or_Equal(&Next, &Current_Time) == TRUE) {
		/* the Next time has past (it's less than the current time) so that means
		   we should call this system */
		strcpy(callcause, "scheduled call");
		return ('S');		/* call this system */
	} else {
		printmsg( 1, "Scheduled call time for %s not reached yet",
					 rmtname );
		return ('I');		/* not time to call yet, don't call system */
	}
}


void Update_Status(int New_Status)
{
	/* rewrite status file with latest results */
	char stfile[80];
	time_t cur_time;
	FILE *fp;
	
	sprintf(stfile, "%s%s", s_statfile, rmtname);
	
	if ((fp=FOPEN(stfile, "w", 't')) == (FILE *)NULL) return;
	
	time(&cur_time);
	if (New_Status == OK) {
		fprintf(fp, "%ld %s %s\r",
				cur_time, rmtname, "OK");
		Last.time_secs = cur_time;
	} else {
		fprintf(fp, "%ld %s %s %d\r",
			cur_time, rmtname, "FAILED", failureCount);
	}
	fclose(fp);
}

/**/
/*
**
**checktime
** check if we may make a call at this time
**
** acceptable formats:  "Any", "Evening", "Night", "hhmm-hhmm"
** hhmm-hhmm may be a inclusive range, ie: 0000-0600,
** or it can be an range that specifies morning and evening hours,
** ie: 1700-0700 (means 0000-0700 and 1700-2359)
**
** Evening includes the standard AT&T long distance hours from 1700 to
** 0800 plus all day Saturday and all day Sunday except evening hours.
**
** Time may also include a retry interval (e.g. "evening;10") to specify
** the minimum retry interval.  If a retry interval is specified, it will
** be used precisely as given.  If no retry interval is specified, a
** HoneyDanBer-style exponential backoff algorithm is used... the retry
** time will be 5 minutes after the first failed attempt, 10 minutes after
** the second, 20 after the third, and so forth... up to a maximum backoff
** of 23 hours.
**
** The retry interval is applied during calls to "any", "all", "cron", and
** "auto".  It is _not_ applied during a call directed to a specific site...
** this allows the user to force a call without having to manually trash
** the ST.sitename file.
*/
checktime(char xtime[])
{
	struct tm	*t;
	int Cur_Time = 0;
	int Left,Right;
	int Is_OK = FALSE;
	char mytime[256],retrystring[256];
	char line[BUFSIZ];
	long int backoff;
	char *p;
	int i, n;
	char stfile[80];			/* constructed status file name */
	time_t Retry_Expiration;
	FILE *fp;
	
	if ((p = strchr(xtime, ';')) == NULL) {
		strcpy(mytime, xtime);
		backoff = -1;
	} else {
		strncpy(mytime, xtime, p - xtime);
		mytime[p-xtime] = '\0';
		if (sscanf(p+1, "%ld", &backoff) != 1 || backoff <= 0 || backoff > 60*24*7) {
			backoff = -1;
		}
	}
	time(&theTime);
	t = localtime(&theTime);
	Cur_Time = t->tm_hour*100 + t->tm_min;
	
	for (p=mytime; *p; p++) *p = tolower(*p);	/* convert to lower */
	
	printmsg( 2, "curtime= %d mytime= %s", Cur_Time, mytime );

	if (strcmp(mytime,"any")==SAME) {
		/* any time allowed */
		Is_OK = TRUE;
		
	} else if (isdigit(*mytime) &&
	           (sscanf(mytime,"%d-%d",&Left,&Right) == 2)) {
		/* time range */
		if (Left <= Right) {
			/* inclusive range, ie: 1700-2300 */
			if (Cur_Time >= Left && Cur_Time < Right) Is_OK = TRUE;
		} else {
			/* complex range, ie: 2300-0800 */
			/* ranges are: 0000-Right + Left-2359 */
			if (Cur_Time < Right || Cur_Time >= Left) Is_OK = TRUE;
		}
		
	} else if (strcmp(mytime,"evening")==SAME) {
		/* evening time period (includes night rates, basicall anytime
		   except weekday daytime rates) */
		if (Cur_Time <  800  ||		/* early morning is night rate */
		    Cur_Time >= 1700 ||		/* evening is evening or night rate */
		    t->tm_wday == 0  || 	/* sunday is all evening or night */
		    t->tm_wday == 6) 		/* saturday is night rates */
		  Is_OK = TRUE;
		
	} else if (strcmp(mytime,"night")==SAME) {
		/* night rate time period */
		if (Cur_Time <  800 ||		/* early morning is night rates */
			Cur_Time >= 2300 ||		/* late night rates */
			t->tm_wday == 6  ||     /* saturday is night rates */
									/* sunday daytime is night rates, evening
									   night rates already handled */
			(t->tm_wday == 0 &&	Cur_Time < 1700) ||
			                        /* friday evening is night rates */
			(t->tm_wday == 5 && Cur_Time >= 1700))
		  Is_OK = TRUE;
		
	} else if (strcmp(mytime,"never")==SAME) {
	    Is_OK = FALSE;
	    
	} else {
		/* didn't match known formats, flag an format error */
		printmsg( 0, "bad time format in System file: %s", mytime );
	}

	if (debuglevel > 0 && !Is_OK) {
		printmsg( 1, "wrong time to call %s, allowed at: %s",
					 rmtname, mytime);
	}
	if (!Is_OK) {
		return FALSE;
	}
	
	/* it's the right time of day. now we need the current status */
	if (!GetStatus()) {
		failureCount = 0;
		return('S');
	}
	
	failureCount = Last.failures;
	
	if (strcmp(Last.status,"OK") == SAME || strcmp(rmtname, Rmtname) == SAME) {
		return TRUE;
	}

	/* Failed during previous run.  See if the retry time has expired */
	if (backoff == -1) { /* HDB-style exponential backoff */
		if (failureCount == 0) {
			backoff = 5;
		} else if (failureCount >= 10) {
			backoff = 23 * 60;
		} else {
			backoff = 5 << (failureCount - 1);
		}
	}
	Retry_Expiration = Last.time_secs + backoff * 60;
	if (theTime >= Retry_Expiration) {
		return (TRUE);		/* retry time past, call system again */
	} else {
		printmsg( 1, "Retry time for %s not reached yet, %ld minutes to go", rmtname,
					(Retry_Expiration - theTime + 59) / 60 );
		return (FALSE);		/* not time yet, don't call system */
	}
}

/**/
/*
**
**      delay
**
*/
/*ddelay(dtime)
int	dtime;
{
	int	i, j;
	for (i = 0; i < dtime; i++) {
	}
}

*/

paceit (void)
{
	EventRecord	theEvent;
	long timea;

	timea = Ticks+4; /* 1/15 - 1/20 second spin */

	while (Ticks < timea) {
#ifdef MULTIFINDER
		Check_Events(MF_DELAY);
#else
		(void) EventAvail(everyEvent, &theEvent);
		SystemTask();
#endif
	}
}


/**/
/*
**
**sysend
** end UUCP session negotiation
*/
sysend(void)
{
	char	msg[80];

	msg[1] = '\0';
	msgtime = 5; /* dplatt, don't hang around for so long... */
/* 	msgtime = 2 * MSGTIME; */
	wmsg("OOOOOO", 2);
	if (rmsg(msg, 2) == -1)
			goto hang;
	/*}*/
hang:
	wmsg("OOOOOO", 2);
	
	closeline();
	printmsg( 0, "Over and out");
	if ( remote == MASTER )
		return('I');
	return('A');
}


/*
**
**wmsg
** write a ^P type msg to the remote uucp
*/
wmsg(char msg[], int syn)
{
	int	len;
	len = strlen(msg);
	if (syn == 2)
		swrite("\0\020", 2);
	swrite(msg, len);
	if (syn == 2)
		swrite("\0", 1);
}


/*
**
**rmsg
** read a ^P msg from UUCP
*/
rmsg(char msg[], int syn)
{
	int	ii;
	char	c, cc[5];
	/* *msg0;*/
	/*msg0 = msg;*/
	
	c = 'a';
	if (syn == 2) {
		while ((c & 0x7f) != '\020') {
			if (sread(cc, 1, msgtime) < 1)
				return(-1);
			c = cc[0]; /* Dont ask. MSC needs more than a byte to breathe */
			/*		printf("Hello im in rmsg c=%x\n",c); */
		}
	}
	for (ii = 0; ii < 132 && c ; ii++) {
		if (sread(cc, 1, msgtime) < 1)
			return(-1);
		c = cc[0] & 0x7f;
		if (c == '\r' || c == '\n')
			c = '\0';
		msg[ii] = c;
		/*if(c == '\020') msg = msg0; */
	}

	return(strlen(msg));
}

void cleanmsg(char *msg)
{
	char c;
	while ((c = *msg)) {
		if (!isprint(c)) {
			c = '?';
		}
		*msg++ = c;
	}
}



/**/
/*
**
**
**startup
**
**
*/
startup(void)
{
	char	msg[255], tmp1[255], tmp2[255];
	memset((void *) &remote_stats, 0, (size_t) sizeof remote_stats);
	if ( remote == MASTER ) {
		msgtime = 2 * MSGTIME;
		if (rmsg(msg, 2) == -1) {
			printmsg( 0, "1st msg (\"Shere\") never arrived" );
			return('X');
		}
		cleanmsg(msg);
		if (strncmp(msg, "Shere", 5) != 0) {
			printmsg(0, "1st msg not Shere: %s", msg);
			return('X');
		}
		printmsg( 2, "1st msg: %s", msg );
		if (msg[5] == '=' && strncmp(&msg[6], rmtname, 7)) {
			printmsg( 0, "Not who we expected it to be: %s", msg );
			return('Y');
		}


		/*sprintf(msg, "S%.7s -Q0 -x%d", nodename, debuglevel);*/  /* -Q0 -x16 remote debuglevel set */

		sprintf(msg, "S%s", shortname);

		wmsg(msg, 2);
		if (rmsg(msg, 2) == -1) {
			printmsg( 0, "2nd msg (responce to \"S%s\") never arrived", shortname );
			return('X');
		}
		cleanmsg(msg);
		printmsg( 2, "2nd msg: %s", msg );
		if (strncmp(msg, "RLCK", 4) == 0) {
			printmsg(0, "%s says it has a lock for you", rmtname);
			return ('Y');
		} else if (strncmp(msg, "RCB", 3) == 0) {
			printmsg(0, "%s says it will call you back", rmtname);
			return ('Y');
		} else if (strncmp(msg, "RLOGIN", 6) == 0) {
			printmsg(0, "%s says you are using an incorrect login name", rmtname);
			return ('X');
		} else  if (strncmp(&msg[1], "OK", 2)) {
			printmsg( 2, "2nd msg not OK" );
			printmsg( 0, "2nd msg: %s", msg);
			return('X');
		}
		if (rmsg(msg, 2) == -1) {
			printmsg( 2, "3rd msg (protocol list) never arrived" );
			return('X');
		}
		cleanmsg(msg);
		printmsg( 2, "3rd msg = %s", msg );
		if (msg[0] != 'P' || index(&msg[1], proto[0]) == (char *)NULL) {
			printmsg(0, "Protocol disagreement");
			wmsg("UN", 2);
			return('X');
		}
		sprintf(msg, "U%c", proto[0]);
		wmsg(msg, 2);
		setproto(proto[0]);
		printmsg( 0, "OK Startup '%c'", proto[0]);
		return('D');
	} else {
		msgtime = 2 * MSGTIME;
		sprintf(msg, "Shere=%s", shortname);
		wmsg(msg, 2);
		if (rmsg(msg, 2) == -1)
			return('Y');
		cleanmsg(msg);
		sscanf(&msg[1], "%s %s %s", rmtname, tmp1, tmp2);
		sscanf(tmp2, "-x%d", &debuglevel);
		printmsg( 1, "debuglevel level = %d", debuglevel );
		printmsg( 2, "1st msg from remote = %s", msg );
		if (checkname(rmtname)) {
			wmsg("RYou are unknown to me", 2);
			printmsg(0, "Rejected a connection from %s (unknown)", rmtname);
			zzz(1);
			return('Y');
		}
		wmsg("ROK", 2);
		sprintf(msg, "P%s", PROTOS);
		wmsg(msg, 2);
		if (rmsg(msg, 2) == -1) {
			printmsg(0, "Never got Use-protocol message from %s", rmtname);
			return('Y');
		}
		if (msg[0] != 'U' || index(PROTOS, msg[1]) == (char *)NULL ) {
			printmsg(0, "Protocol disagreement with %s", rmtname);
			return('Y');
		}
		proto[0] = msg[1];
		setproto(proto[0]);
		printmsg( 0, "Call from %s connected", rmtname);
		return('R');
	}
}


/******* set the protocol **********/
setproto(char pr)
{
	int	i;
	Proto * tproto;
	for (tproto = Protolst; tproto->type != '\0' && pr != tproto->type; tproto++) {
		printmsg( 3, "setproto: %c %c", pr, tproto->type );
	}
	if (tproto->type == '\0') {
		printmsg( 0, "setproto:You said I had it but I cant find it" );
	    hangup();
	    (*currentConnection->Close)(TRUE);
		exit(1);
	}
	getpkt = tproto->a;
	sendpkt = tproto->b;
	openpk = tproto->c;
	closepk = tproto->d;
	filepkt = tproto->e;
	eofpkt = tproto->f;
	getmsg = tproto->g;
	sendmsg = tproto->h;
	sendresp = tproto->i;
}



/**/
int prefix(char *sh, char *lg)
{
	return( strncmp(sh,lg,strlen(sh)) == SAME);
}

int notin(char *sh, char *lg)
{
	while (*lg) {
		if (prefix(sh,lg++))
			return( FALSE );
	}
	return( TRUE );
}

#define MAXR 300
int
expectstr(char *str, long int timeout)
{
	static char rdvec[MAXR];
	char *rp = rdvec;
	char expectstr[MAXR];
	int kr;
	int totlen, strip, iter, i, special;
	int totReceived;
	char nextch;
	int nbreaks, breakno;
	char *breaks[10], *breakout;
	
	nbreaks = 0;
	totReceived = 0;

	printmsg( 1, "wanted %s", str );

	if ( strcmp(str, "\"\"") == SAME || strlen(str) == 0) {
		return( TRUE );
	}
	i = 0;
	totlen = 0;
	special = 0;
	while (i < strlen(str)) {
		nextch = str[i++];
		if (special) {
			switch (nextch) {
			case 's':
				expectstr[totlen++] = ' ';
				break;
			case 'r':
				expectstr[totlen++] = '\r';
				break;
			case 'n':
				expectstr[totlen++] = '\n';
				break;
			case 't':
				expectstr[totlen++] = '\t';
				break;
			default:
				expectstr[totlen++] = nextch;
				break;
			}
			special = 0;
		} else if (nextch == '\\') {
			special = 1;
		} else {
			expectstr[totlen++] = nextch;
		}
	}
	expectstr[totlen] = '\0';
	if (breakout = strchr(expectstr, '¥')) {
		do {
			*breakout++ = '\0';
			breaks[nbreaks++] = breakout;
			breakout = strchr(breakout, '¥');
		} while (breakout);
	}
	*rp = 0;
	while ( notin( expectstr,rdvec ) ) {
		printmsg(9, "Got '%s', match on '%s''", rdvec, expectstr);
		for (breakno = nbreaks-1 ; breakno >= 0; breakno--) {
			if (!notin(breaks[breakno], rdvec)) {
				printmsg(0, breaks[breakno]);
				return( FALSE );
			}
		}
#ifdef MULTIFINDER
		if (Main_State != Call_Systems) {
			return ( FALSE );
		}
#endif
		/* fprintf(stderr, "---------->%s<------\n", rdvec);/**/
		kr = sread(&nextch, 1, timeout);
		if (kr <= 0) {
			return( FALSE );
		}
		if (debuglevel >= 7) {
			fputc(nextch & 0177, stdout);
		}
		totReceived++;
		if (totReceived > MAXR * 10) { /* "Enough, already!" gibberish blocker */
			printmsg(2, "He seems to be babbling...");
			return (FALSE);
		}
		if ((*rp = nextch & 0177) != '\0') {
			rp++;
		}
		if (rp >= rdvec + MAXR) { /* off end, need to toss old stuff */
			strip = MAXR - totlen - 2;
			iter = strip;
			while (iter < MAXR) {
				rdvec[iter-strip] = rdvec[iter];
				iter++;
			}
			rp -= strip;
		}
		*rp = '\0';
	}
	return( TRUE );
}

int
writestr(char *s) {
	register char last;
	register char * m;
	char ctlchar;
#ifdef MULTIFINDER
	long Last_Ticks;
#endif
	int nocr, pace;
	last = '\0';
	nocr = pace = FALSE;
	while (*s) {
		if (last == '\\') {
			switch (*s) {
			case 'd':
			case 'D': /* delay */
#ifdef MULTIFINDER
				Last_Ticks = Ticks;
				while (Ticks-Last_Ticks < 120) 
				  if (Check_Events(30))
				    if (Main_State == Abort_Program)  {
	    				hangup();
				    	(*currentConnection->Close)(TRUE);
						exit(-1);
					}
#else
				delay(80000);
#endif
               break;
            case 'c':
            case 'C': /* end string don't output CR */
               nocr = TRUE;
               break;
            case 'r':
            case 'R': /* carriage return */
            case 'm':
            case 'M':
      			swrite( "\r", 1 );
      			if (pace) paceit();
               	break;
            case 'n':
            case 'N':
      			swrite( "\n", 1 );
      			if (pace) paceit();
               	break;
            case 'b':
            case 'B':
      			swrite( "\b", 1 );
      			if (pace) paceit();
               	break;
            case 't':
            case 'T':
      			swrite( "\t", 1 );
      			if (pace) paceit();
               	break;
            case 's':
            case 'S':
      			swrite( " ", 1 );
      			if (pace) paceit();
               	break;
            case 'z':
            case 'Z':
            	(*currentConnection->Speed)( ++s );
            	while ( *s != '\0' && *s != '\\' )
            		s++;
            	if ( *s == '\\' )
            		s++;
            	break;
#ifdef Upgrade
            case 'p':
            case 'P':
      			if (pace = ! pace) paceit();
            	break;
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            	for(systimeout = *s++ - '0' ; *s < '9' && *s >= '0' ; ) {
            		systimeout *= 10;
            		systimeout += *s++ - '0';
            	}
            	s--; /* incremented later - before next time round loop */
				printmsg(3, "expect timeout set to %hd", systimeout);
            	break;
#endif Upgrade
            default:
      			swrite( s, 1 );
      			if (pace) paceit();
			}
			last = '\0';
		}
		else if (last == '^') {
			ctlchar = *s & 0x1f;
			swrite(&ctlchar, 1);
      		if (pace) paceit();
			last = '\0';
		}	
		else if (*s != '\\' && *s != '^') {
      		swrite( s, 1 );
      		if (pace) paceit();
		}
		else {
			last = *s;
		}
		s++;
	}
	return( nocr );
}

/***
 *   void sendthem(str)   send line of login sequence
 *         char *str;
 *
 *   return codes:  none
 */

void sendstr(char *str)
{
	int nw, ns;
	int nulls = 3;

	printmsg( 2, "sending %s", str );

#ifdef BREAK
	if (prefix("BREAK", str)) {
		sscanf(&str[5], "%1d", &nulls);
		if (nulls <= 0 || nulls > 10)
			nulls = 3;
		/* send break */
		(*currentConnection->Break)(nulls);
		return;
	}
#endif BREAK

	if ( strcmp(str, "EOT") == SAME ) {
		swrite(EOTMSG, strlen(EOTMSG));
		return;
	} else if (strcmp(str, "P_EVEN") == SAME) {
		(*currentConnection->SetParity)(data7, evenParity);
		return;
	} else if (strcmp(str, "P_ODD") == SAME) {
		(*currentConnection->SetParity)(data7, oddParity);
		return;
	} else if (strcmp(str, "P_NONE") == SAME) {
		(*currentConnection->SetParity)(data8, noParity);
		return;
	}		

	if ( strcmp(str,"\"\"") == SAME )
		*str = '\0';
		/*fprintf(stderr,"'%s'\n",str);*/

	if ( strcmp(str,"") != SAME ) {
		if (!writestr(str)) {
			swrite ("\r", 1);
		}
	}
	else {
		swrite("\r", 1);
	}
	return;
}

int
sendexpect(char *s, char *e, long int timeout) {

	sendstr(s);
	
#ifdef Upgrade
	if(systimeout > 0)
		return(expectstr(e, systimeout));
#endif Upgrade

	return(expectstr(e, timeout));
}

dial(void)
{
	int	flg, kk, jj, ll, firstflg;
#define SPEEDBUF 10
	char	buf[10], *prsend;

	char *exp;
	char *alternate;
	int	ok;
	int i;
	int dialpause;
	static char HayesInitString[] = "\\p\\dAAATQ0V1E0";
	long minutes_to_sleep;

#ifdef Upgrade
	char number[128];
	char dialstring[256];

	if ( strcmp( flds[FLD_TYPE], "VADIC" ) == SAME ) {
		if (rmtname[0] != 0) printmsg( 0, "Dialing host %s", rmtname );
		if (line(device, speed, NULL ))
			return( FALSE );
	
		(*currentConnection->AllowInterrupts)(TRUE);
		printmsg( 2, "vadic: autobauding" );
		if ( sendexpect( "\005\\d", "*", 2 ) != TRUE &&
		 sendexpect( "\005\\d", "*", 2 ) != TRUE ) {
			printmsg( 0, "vadic: no response to autobaud" );
			return( FALSE);
		}
		printmsg( 1, "vadic: got modem response" );
	
		if (sendexpect ("dial", "NUMBER?", 5) != TRUE ) {
			printmsg( 2, "vadic: dial command rejected" );
			sendstr("I\dIDLE\\r");
			return( FALSE);
		}
		
		strcpy(number, "\\d");
		strcat(number, phone);
		strcat(number, "\\r\\d");
		if ( sendexpect(number, "DIALING...¥NO DIAL TONE", 5) != TRUE ) {
			printmsg( 0, "vadic: could not initiate dial" );
			sendstr("I\dIDLE\\r");
			return(FALSE);
		}
	
		if ( sendexpect( "\\d\\c", "ON LINE¥BUSY¥FAILED CALL", 40 ) == TRUE ) {
			printmsg( 3, "vadic: ON LINE" );
			return( TRUE );
		}
		else {
			sendstr("I\dIDLE\\r");
			return( FALSE );
		}
	}

#endif

	if ( strncmp( flds[FLD_TYPE], "HAYES", 5 ) != SAME ) {
		printmsg( 0, "dial: unsupported dialer %s", flds[FLD_TYPE] );
		return( FALSE );
	}

	if (rmtname[0] != 0) printmsg( 0, "Dialing host %s", rmtname );
	if (openline(device, speed, NULL ))
		return( FALSE );
	
	if (flds[FLD_TYPE][5] == '*') {
		useHardwareFlowControl = TRUE;
		(*currentConnection->SetFlowCtl)(FALSE, TRUE);
	}

	(*currentConnection->AllowInterrupts)(TRUE);
	
	strcpy(dialstring, HayesInitString);
	if (flds[FLD_TYPE][5] != '\0' && flds[FLD_TYPE][6] != '\0') { /* for HAYES+ and HAYES! */
		strcat(dialstring, flds[FLD_TYPE]+6);
	}
	printmsg( 2, "hayes: autobauding" );
	if ( sendexpect( dialstring, "OK", 2 ) != TRUE &&
		 sendexpect( dialstring, "OK", 2 ) != TRUE) {
		sendexpect( "\\d\\p+++\\c", "OK", 2 );
		if ( sendexpect( "\\pAAATZ", "OK", 2 ) != TRUE ||
			 sendexpect( dialstring, "OK", 2 ) != TRUE) {
			printmsg(0, "hayes: could not get modem's attention");
			return( FALSE);
		}
	}
	printmsg( 1, "hayes: got modem response" );
/*	zzz(1); */
	
	strcpy(dialstring, "\\pAAAT");	
	
	if (strcmp(phone, "-") == SAME | strcmp(phone, "@") == SAME) {
		strcat(dialstring, "S0=1");
		sscanf(sleeptime, "%ld", &minutes_to_sleep);
		if (minutes_to_sleep < 1) minutes_to_sleep = 1;
		dialpause = minutes_to_sleep * 60;
	} else {
		if (isdigit(*phone)) {
			strcat(dialstring, "DT");
		}
		strcat(dialstring, phone);
		dialpause = 120;
	}

	if ( sendexpect( dialstring, "CONNECT¥BUSY¥NO CARRIER¥NO DIAL TONE¥NO DIALTONE", dialpause ) == TRUE ) {
		printmsg( 3, "hayes: got CONNECT" );

		if ( flds[FLD_TYPE][5] != '!' && flds[FLD_TYPE][5] != '*') {
			if (sread(buf, 1, 1) == 1 && *buf == ' ') {
				i = 0;
				while (i < SPEEDBUF) {
					if (sread( buf+i, 1, 1 ) != 1 || ! isdigit(buf[i]) ) {
						break;
					}
					i++;
				}
				if (i > 0) {
					buf[i] = '\0';
					printmsg( 3, "hayes: speed select %s", buf );
					/* set speed appropriately */
					(*currentConnection->Speed) ( buf );
				}
			}
		}
		return( TRUE );
	}
	else {
		(*currentConnection->AllowInterrupts)(FALSE);
		sendexpect("\\r\\d\\pAAATZ", "OK", 4);
		return( FALSE );
	}

}


/*
**
**callup
** script processor - nothing fancy!
*/
callup(void)
{
	int	flg, kk, jj, ll, firstflg, err;
	char	*prsend;

	char *exp;
	char *alternate;
	int	ok;
	int i;

	useHardwareFlowControl = FALSE;

	if (rmtname[0] != 0) printmsg( 0, "Calling %s (%s)", rmtname, callcause );
	
	if ( strcmp( flds[FLD_TYPE], "DIR" ) == SAME ) {
		if (openline(device, speed, phone)) {
			return ('X');
		}
#ifdef NOTDEF
	} else if (strcmp(flds[FLD_TYPE], "TCP") == SAME) {
		if ((err = open_tcp_uucp()) != noErr) {
			if (rmtname[0] != 0) printmsg(0, "TCP open failed, code %d", err);
			return( 'X' );
		}
#endif
	} else {
		if ( dial() == FALSE ) {
			if (rmtname[0] != 0) printmsg(0, "Dial failed");
			return( 'X' );
		}
	}

	(*currentConnection->AllowInterrupts)(TRUE);

	for (i = FLD_EXPECT; i < kflds; i+=2) {

		exp = flds[i];
		printmsg( 2, "callup: expect %d of %d  \"%s\"", i, kflds, exp );

		ok = FALSE;
		while (ok != TRUE) {

			alternate = index( exp, '-' );
			if (alternate != (char *)NULL)
				*alternate++ = '\0';
#ifdef Upgrade
			ok = expectstr(exp, (systimeout > 0 ? systimeout : (10+2*strlen(exp))));
#else Upgrade
			ok = expectstr( exp, 10+2*strlen(exp) );
#endif Upgrade

			printmsg( 2, "got %s", ok != TRUE ? "?" : "that" );

			if ( ok == TRUE ) {
				printmsg( 2, "got that" );
				break;
			}

			if ( alternate == (char *)NULL ) {
				printmsg( 0, "Login failed" );
				return( 'X' );
			}

			exp = index( alternate, '-' );
			if ( exp != (char *)NULL )
				*exp++ = '\0';

			printmsg( 1, "send alternate" );

			sendstr( alternate );
		}

	if (i+1 < kflds) {
		printmsg( 2, "callup: send %d of %d  \"%s\"", i+1, kflds, flds[i+1] );
#ifdef MULTIFINDER
		{
		  long Last_Ticks;
		  int status = 0;
		
		  Last_Ticks = Ticks;
		  while (Ticks-Last_Ticks < 60)
			if (Check_Events(15)) {
			  printmsg( 0, "Call cancelled by operator" );
	    	  hangup();
			  (*currentConnection->Close)(TRUE);
			  return( 'X' );
		    }
		}
#else
		zzz(1);
#endif
		sendstr(flds[i+1]);
	}
	}
	printmsg( 0, "Connected");
	return('P');

}

/**/
/*
**
**      slowrite
** comunication slow write. needed for auto-baud modems
*/
/*slowrite(st)
register char	*st;
{
	int	len, j;
	char	c;
	len = strlen(st);
	printmsg( 2, "sent %s", st );
	for (j = 0; j < len; j++) {
		swrite(&st[j], 1);
		ddelay(80000);
	}
}
*/

/**/
/*
**
**scandir
**
*/

#include "ndir.h"


/*	scandir

	scan work dir for C. files matching current remote host (rmtname)

	return

		A	- abort
		Y	- can't open file
		S	- ok
		Q	- no files

*/
scandir(void)
{
	int	fn, len, i;
	char	cname[40], mappedname[40], tmp[132];

	DIR *dirp;
	struct direct *dp;

	if ((dirp = opendir( spooldir )) == (DIR *)NULL ) {
		fprintf( stderr, "couldn't open dir %s\n", spooldir );
		return( 'A' );
	}
#ifdef Upgrade
	sprintf(cname, "%s%s", CALLFILE, rmtname);
#else Upgrade
	sprintf(cname, CALLFILE, rmtname);
#endif Upgrade
	len = strlen(cname);
	if (len > 9) len = 9;
	while ((dp = readdir(dirp)) != (struct direct *)NULL) {
		printmsg( 4, "scandir: %s", dp->d_name );
		strcpy(mappedname, dp->d_name);
		unmapMacCaseness(mappedname);
		if ( strncmp( cname, dp->d_name, len ) == SAME ||
			 strncmp( cname, mappedname, len) == SAME) {
			printmsg( 4, "scandir: match!!" );
			strcpy(cfile, dp->d_name);
#ifdef Upgrade
			/* don't forget to close the last cfile */
			if (fw != (FILE *)NULL) {
				fclose(fw);
				fw = (FILE *)NULL;
			}
#endif Upgrade
			if ((fw = FOPEN( cfile, "r", 't' )) == (FILE *)NULL ) {
				printmsg(4, "scandir: could not open file '%s'", cfile );
				getcwd(tmp, sizeof tmp);
				printmsg(4, "scandir: current working directory was '%s'", tmp );
				closedir( dirp );
				return('Y');
			}
			closedir( dirp );
			strcpy(callcause, "work pending");
			return('S');
		}
	}
	closedir( dirp );
	printmsg(4, "scandir: no work files");
	return('Q');

}


/**/
/*
**
**dscandir
** scan the directory
*/

dscandir(void)
{
	int	fn, len, i;
	char	cname[40], tmp[132], gotname[40];

	DIR *dirp;
	struct direct *dp;


	if ((dirp = opendir( spooldir )) == (DIR *)NULL ) {
		printmsg( 0, "** couldn't open dir %s\n", spooldir );
		return(-1);
	}
	gotname[0] = 0;
#ifdef Upgrade

	/* we probably want ALL X. files to be processed every chance we get
	 * in order to pick up any left behind by crashes before they could
	 * be dealt with. In any case the #else Upgrade code was incorrect
	 * and this fixes that A.S.
	 */
	 
# ifndef TOORESTRICTIVE
	strcpy(cname, XQTFILE);
# else TOORESTRICTIVE
	sprintf(cname, "%s%s", XQTFILE, rmtname); /* may be too restrictive */
# endif TOORESTRICTIVE

#else Upgrade
	sprintf(cname, XQTFILE, rmtname); /* sprintf(cname,"c%.4s",rmtname); */
#endif Upgrade
/*
	Modified this loop for 3.0d3 to take into account the fact that the
	names of inbound X. files have gone through a reversable name-conversion
	process which may change their sort ordering.  The loop now runs through
	all filenames in the directory, reverses the name-conversion, and picks
	the X. file with the lowest-sorting name after the reverse conversion.
*/
	len = strlen(cname);
	while ((dp = readdir(dirp)) != (struct direct *)NULL) {
		printmsg( 4, "dscandir: file = %s cname = %s", dp->d_name, cname );
		unmapMacCaseness(cname);
		if ( strncmp( cname, dp->d_name, len ) == SAME ) {
			if (gotname[0] == 0 || strcmp(dp->d_name, gotname) < 0) {
				strcpy(gotname, dp->d_name);
			}
		}
	}
	closedir( dirp );
	if (gotname[0]) {
		printmsg( 4, "dscandir: match!!" );
		strcpy(cfile, gotname);
		return( -1 );
	} else {
		return( 0 );
	}
}
