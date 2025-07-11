
typedef struct {
	char min[40];
	char hour[40];
	char day[40];
	char month[40];
	char weekday[40];
	char name[40];
	int retrytime;	/* minutes between retries */
} schedule_record;
	
typedef int datalist[61];

typedef struct {
	datalist min;
	datalist hour;
	datalist day;
	datalist month;
	datalist weekday;
} schedule_list_record;

typedef struct {
	time_t time_secs;
	char   name[40];
	char   status[40];
} status_record;

void Schedule_To_List(char *Schedule, datalist Schedule_List, int Min, int Max);
void Compute_Next_Call_Time(status_record Last, schedule_record Schedule, struct tm *Next );
int Time_Less_Than_Or_Equal(struct tm *Left, struct tm *Right);

int Time_Less_Than_Or_Equal(Left, Right)
struct tm *Left;
struct tm *Right;
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


void Schedule_To_List(Schedule, Schedule_List, Min, Max)
char *Schedule;
datalist Schedule_List;
int Min, Max;
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

int Next_Valid_Value(int Last, datalist Valid_List);
int Next_Valid_Value(Last, Valid_List)
int Last;
datalist Valid_List;
{
	int i = 0;
	
	while ((Valid_List[i] <= Last) && (Valid_List[i] >= 0))
		i++;
		
	if (Valid_List[i] < 0) {
		/* didn't find one larger, use the first entry */
		return Valid_List[0];
	} else {
		/* take the larger one we found */
		return Valid_List[i];
	}
}

void Compute_Next_Call_Time(Last, Schedule, Next)
status_record Last;
schedule_record Schedule;
struct tm *Next;
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
	Next->tm_min = Last_Time.tm_min++;
	Next->tm_min = Next_Valid_Value(Next->tm_min, Valid_Lists.min);
	if (Next->tm_min < Last_Time.tm_min) {
		/* field was set backwards, we must update the less significant fields to their
		   minimum values */
		/* field was set backwards, we must carry 1 to the next more significant field */
		carry = 1;
	} else {
		carry = 0;
	}

	Next->tm_hour = Last_Time.tm_hour + carry;
	Next->tm_hour = Next_Valid_Value(Next->tm_hour, Valid_Lists.hour);
	if (Next->tm_hour < Last_Time.tm_hour) {
		/* field was set backwards, we must update the less significant fields to their
		   minimum values */
		Next->tm_min = Valid_Lists.min[0];
		/* field was set backwards, we must carry 1 to the next more significant field */
		carry = 1;
	} else {
		carry = 0;
	}
	
	Next->tm_mday = Last_Time.tm_mday + carry;
	Next->tm_mday = Next_Valid_Value(Next->tm_mday, Valid_Lists.day);
	if (Next->tm_mday < Last_Time.tm_mday) {
		/* field was set backwards, we must update the less significant fields to their
		   minimum values */
		Next->tm_hour = Valid_Lists.hour[0];
		Next->tm_min  = Valid_Lists.min[0];
		/* field was set backwards, we must carry 1 to the next more significant field */
		carry = 1;
	} else {
		carry = 0;
	}
	
	Next->tm_mon = Last_Time.tm_mon + carry;
	Next->tm_mon = Next_Valid_Value(Next->tm_mon, Valid_Lists.month);
	if (Next->tm_mon < Last_Time.tm_mon) {
		/* field was set backwards, we must update the less significant fields to their
		   minimum values */
		Next->tm_mday = Valid_Lists.day[0];
		Next->tm_hour = Valid_Lists.hour[0];
		Next->tm_min  = Valid_Lists.min[0];
		/* field was set backwards, we must carry 1 to the next more significant field */
		carry = 1;
	} else {
		carry = 0;
	}
	
	Next->tm_mon--;		/* make months 0 based again */
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
checkcron()
{
	FILE *fp;
	char line[BUFSIZ];
	char stfile[80];			/* constructed status file name */
	int n;
	time_t Cur_Time_Secs, Retry_Expiration;
	
	schedule_record Schedule;
	status_record Last;
	struct tm Next, Current_Time;
	
	/* got a real problem if schedule file is missing */
	if ((fp=FOPEN(s_schedule, "r", 't')) == (FILE *)NULL)
		return('I');		/* no schedule, so don't call anyone */

	n = 0;
	while (n==0 && fgets(line, BUFSIZ, fp) != (char *)NULL){
		/* line format:
			min hour day month weekday system retrytime */
		n = sscanf(line, "%s %s %s %s %s %s %d",
				   Schedule.min, Schedule.hour, Schedule.day,
				   Schedule.month, Schedule.weekday, Schedule.name,
				   &Schedule.retrytime);
		if (n != 7 ||
			strcmp(rmtname, Schedule.name) != SAME) n = 0;
	}
	fclose(fp);
	if (n == 0) return ('I');	/* didn't find the system in schedule, don't call it */
	
	/* we have the schedule now, now we need the current status */
	sprintf(stfile, "%s%s", s_statfile, rmtname);
	
	/* if there is no status file, assume it has never been called */
	if ((fp=FOPEN(stfile, "r", 't')) == (FILE *)NULL) return('S');
	
	n = 0;
	if (fgets(line, BUFSIZ, fp) != (char *)NULL) {
		/* got status line into buffer */
		/* line format:
				ctime system status
		   ctime is epoch time in seconds from time function
		   status may be OK or FAILED */
		n = sscanf(line, "%ld %s %s",
				   &Last.time_secs, Last.name, Last.status);
		if (n != 3 ||
			strcmp(rmtname, Last.name) != SAME) n = 0;
	}
	fclose(fp);
	if (n == 0) return ('S');	/* didn't get it's status line, call it */
		
	if (strcmp(Last.status,"OK") != SAME) {
		/* last call failed, see if the retry time has expired */
		Retry_Expiration = Last.time_secs + Schedule.retrytime * 60;
		time(&Cur_Time_Secs);
		if (Cur_Time_Secs >= Retry_Expiration)
			return ('S');		/* retry time past, call system again */
		else
			return ('I');		/* not time yet, don't call system */
	
	} else {
		/* last call was successful, see if it's time to call this system again */
		Compute_Next_Call_Time( Last,		/* time of last successful call */
								Schedule,	/* time schedule pattern */
								&Next);		/* next time it should be called */
		if (Time_Less_Than_Or_Equal(&Next, &Current_Time) == TRUE)
			/* the Next time has past (it's less than the current time) so that means
			   we should call this system */
			return ('S');		/* call this system */
		else
			return ('I');		/* not time to call yet, don't call system */
	}
}

