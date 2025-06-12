/*
GetTimeDateString

Return a malloced string with the time and date in a nice format:
	"10:43 PM, Monday, September 13, 1993"
You may supply a calendar time, as returned by time(), or you may supply 0 to
use the current time. 

HISTORY:
8/93	dhb,jms	wrote it, using Apple toolbox.
9/13/93	dgp	rewrote it using Standard C library.
9/16/93 dhb Fixed name of routine in PrintfExit.
*/
#include <VideoToolbox.h>
#include <time.h>	/* Standard C library */

char *GetTimeDateString(time_t t)
{
	char *s;
	
	s=malloc(64);
	if(s==NULL)PrintfExit("GetTimeDateString: malloc(64) failed.\n");
	if(t==0)t=time(NULL);
	strftime(s,64,"%I:%M %p, %A, %B %d, %Y",localtime(&t));
	return(s);
}
