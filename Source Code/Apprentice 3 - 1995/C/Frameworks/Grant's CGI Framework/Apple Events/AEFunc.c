/*****
 *
 *	AEFunc.c
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

#include "globals.h"

#include "Events.h"
#include "ErrorUtil.h"

#include "AEFunc.h"


/***  FUNCTIONS  ***/

/* IM:IAC 4-35 */
OSErr
MyGotRequiredParams ( AppleEvent *theAppleEvent )
{
	OSErr		theErr;
	DescType	returnedType;
	Size		actualSize;
	
	theErr = AEGetAttributePtr ( theAppleEvent, keyMissedKeywordAttr, typeWildCard,
		&returnedType, nil, nil, &actualSize );
	
	if ( theErr == errAEDescNotFound )
	{
		/* all required parameters were retrieved */
		return noErr;
	}
	else if ( theErr == noErr )
	{
		/* required parameter was missed */
		return errAEParamMissed;
	}
	else
	{
		return theErr;
	}
} /* MyGotRequiredParams */


/* IM:IAC 5-23 */
pascal Boolean
MyAEIdleFunc ( const EventRecord *theEvent, long *sleepTime, RgnHandle *mouseRgn )
{
	Boolean		theResult = false;
	
	switch ( theEvent->what )
	{
		case updateEvt :
		case activateEvt :
			break;
		
		case osEvt :
			/* MyAdjustCursor ( theEvent->where, theMouseRgn ); */
			doOsEvt ( theEvent );
			break;
		
		case nullEvent :
			/* *mouseRgn = theMouseRgn; */
			*sleepTime = gSleepTicks;	/* may need to change to use correct value for app */
			break;
	}
	
	return theResult;
} /* MyAEIdleFunc */


/*****  EOF  *****/
