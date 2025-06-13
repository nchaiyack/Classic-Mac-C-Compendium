/*****
 *
 *	ErrorInt.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include "DebugUtil.h"
#include "DialogFunc.h"

#include "ErrorInt.h"


/***  FUNCTIONS  ***/

/** Report System Errors **/

/* Display a system error alert with the given string. */
void
ErrorAlertSystemString (
	OSErr			errNum,
	StringHandle	errStrHdl,
	Str255 *		errNameStrPtr
	)
{
	Str255			errNumStr;	/* used for string version of the error number */
	short			alertItem;
	
	my_assert ( (errStrHdl != nil) && (*errStrHdl != nil),
		"\pErrorIntSystemString: null error string" );
	
	/* Create a pascal format string of the error number. */
	NumToString ( errNum, errNumStr );
	
	HLockHi ( (Handle)errStrHdl );
	
	if ( errNameStrPtr == nil )
	{
		/* Set the dialog to display the string and error number strings. */
		ParamText ( **((Str255 **)errStrHdl), errNumStr, nil, nil );
	}
	else
	{
		/* Set the dialog to display the string and error number strings. */
		ParamText ( **((Str255 **)errStrHdl), errNumStr, *errNameStrPtr, nil );
	}
	
	/* Display the alert. */
	alertItem = StopAlert ( krErrorDialogSystem, defaultAlert1ButtonEventFilter );
	
	HUnlock ( (Handle)errStrHdl );
} /* ErrorAlertSystemString */


/** Report Startup Errors **/

#pragma segment Startup
/* Display a startup error alert with the given string. */
void
ErrorAlertStartupString ( short errNum, Str255 *errStrHdl )
{
	Str255		errNumStr;	/* used for string version of the error number */
	short		alertItem;
	
	/* Create a pascal format string of the error number. */
	NumToString ( errNum, errNumStr );
	
	/* Set the dialog to display the string and error number strings. */
	ParamText ( *errStrHdl, errNumStr, nil, nil );
	
	/* Display the alert. */
	alertItem = StopAlert ( krErrorDialogStartup, defaultAlert1ButtonEventFilter );
} /* ErrorAlertStartupString */
#pragma segment Main


/*****  EOF  *****/
