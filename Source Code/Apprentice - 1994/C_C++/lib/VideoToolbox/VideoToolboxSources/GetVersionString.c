/*
GetVersionString.c
Gets the version of a program from the "Short version string" field of
its 'vers' resource. The version is returned as a C string allocated by
NewPtr(). Returns "" if there is no 'vers' 1 resource. Use this to display 
your program's current version number.

Copyright Joseph Laffey, 1993. You may use this code snippet in anything you'd
like. It would be cool if you could give me a little credit... Of course, USE AT
YOUR OWN RISK; I'm not responsible for anything, etc., etc. joelaff@aol.com

HISTORY: 
11/93 dgp downloaded VersStr.c from Compuserve
12/19/93 dgp Removed excess whitespace. Renamed function from GetVersStr to 
			GetVersionString. Now allocate and return string instead of copying 
			to user-allocated string argument. Added error checking.
*/
#include <VideoToolbox.h>
#include <Resources.h>
#include <Memory.h>
#define	VERS_RES_ID		1	/* resource ID of 'vers' to use (usually 1 as of Sys7.1)*/
#define	BYTE_OFFSET		6	/* the offset of the pascal-style version string */
							/* from the beginning of the 'vers' resource. */
							/*  Apple MAY change this-- hopefully not!!! */
char *GetVersionString(void);

char *GetVersionString(void)
{
	Handle vers;
	unsigned char *s1;
	char *s2;
	
	vers=Get1Resource('vers',VERS_RES_ID);
	if(vers==NULL)return "";
	HLock(vers);
	s1=(unsigned char *)(*vers + BYTE_OFFSET);
	if(BYTE_OFFSET+*s1+1>SizeResource(vers))
		PrintfExit("GetVersionString: Illegal 'vers' resource.\n");
	s2=NewPtr(*s1+1);
	if(s2==NULL)PrintfExit("GetVersionString: couldn't allocate string.\n");
	PtoCstr(s1);
	strcpy(s2,(char *)s1);
	ReleaseResource(vers);
	return s2;
}