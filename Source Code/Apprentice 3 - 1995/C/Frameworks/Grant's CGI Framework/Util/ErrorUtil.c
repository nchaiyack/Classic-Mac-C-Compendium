/*****
 *
 *	ErrorUtil.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright �1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include <stdio.h>
#include <string.h>

#include "constants.h"
#include "globals.h"

#include "DebugUtil.h"
#include "ErrorInt.h"
#include "MemoryUtil.h"
#include "Quit.h"
#include "StringUtil.h"

#include "ErrorUtil.h"


/***  LOCAL CONSTANTS ***/

#define kstrErrorMissing		"\pUnknown! Note the id number and contact the developer of this software."

/* the size of the above string + 1 for size byte */
#define kstrErrorMissingSize	72


/***  LOCAL PROTOTYPES ***/

	StringHandle	errorSystemGetString		( OSErr, Str255 * );
	void			errorSystemDisposeString	( StringHandle );
	void			errorStartupGetString		( short, Str255 * );


/***  FUNCTIONS  ***/

/*  */
void
ErrorSystem ( OSErr errNum )
{
	StringHandle	errStrHdl;	/* the human readable error string */
	Str255			errNameStr;	/* the name of the error */
	
	errStrHdl = errorSystemGetString ( errNum, &errNameStr );
	
	if ( (errStrHdl != nil) && (*errStrHdl != nil) )
	{
		/* errStrHdl was successfully retreived */
		/* Display the alert. */
		ErrorAlertSystemString ( errNum, errStrHdl, &errNameStr );
		
		errorSystemDisposeString ( errStrHdl );
	}
	else
	{
		/* can't get system error string */
		SysBeep ( 5 );
	}
} /* ErrorSystem */


#pragma segment Startup
/*  */
void
ErrorStartup ( short errNum )
{
	Str255	errStr;	/* the retrieved error string */
	
	errorStartupGetString ( errNum, &errStr );
	
	/* Display the alert. */
	ErrorAlertStartupString ( (OSErr)errNum, &errStr );
	
	ForceQuit	();
} /* ErrorStartup */
#pragma segment Main


/**  String Retreival  **/

/* return the system error's human readable string and it's constant name (errNameStr) */
StringHandle
errorSystemGetString ( OSErr errNum, Str255 *errNameStr )
{
	StringHandle	errStrHdl;	/* the retrieved error string */
	StringHandle	theString;	/* the string to return */
	short			errID;		/* just used for GetResInfo call */
	ResType			errType;	/* just used for GetResInfo call */
	
	/* Get message string for the error number. */
	errStrHdl = (StringHandle) GetResource ( (ResType)krtErrStr, (short)errNum );
	theString = nil;
	
	if ( errStrHdl != nil )
	{
		theString = (StringHandle) MyNewHandle ( sizeof(Str255), nil );
	}
	
	if ( theString == nil )
	{
		/* if string isn't available, use generic error string */
		theString			= gSystemErrorStr;
		(*errNameStr)[0]	= nil;
	}
	else
	{
		HLockHi ( (Handle)errStrHdl );
		HLockHi ( (Handle)theString );
		
		StringPascalCopy ( (char *)(*errStrHdl), (char *)(*theString) );
		
		HUnlock ( (Handle)errStrHdl );
		HUnlock ( (Handle)theString );
		
		ReleaseResource ( (Handle)errStrHdl );
		
		/* get the error name from the resource */
		GetResInfo ( (Handle)errStrHdl, &errID, &errType, *errNameStr );
	}
	
	return theString;
} /* errorSystemGetString */


/* dispose of the error string handle - don't do anything if it is the default string */
void
errorSystemDisposeString ( StringHandle theString )
{
	my_assert ( theString != nil, "\perrorSystemDisposeString: theString is nil" );
	
	if ( theString != gSystemErrorStr )
	{
		DisposeHandle ( (Handle)theString );
	}
} /* errorSystemDisposeString */


#pragma segment Startup
/*  */
void
errorStartupGetString ( short errNum, Str255 *errStr )
{
	/* Get message string for the error number. */
	GetIndString ( *errStr, krStartupErrorStrs, errNum );

	if ( *errStr == nil )
	{
		/* if string isn't available, use generic error string */
		StringPascalCopy ( (char *)kstrErrorMissing, (char *)(*errStr) );
	}
} /* errorStartupGetString */
#pragma segment Main


/*****  EOF  *****/
