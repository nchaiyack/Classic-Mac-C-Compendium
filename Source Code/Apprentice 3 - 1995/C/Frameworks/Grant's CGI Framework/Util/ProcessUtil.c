/*****
 *
 *	ProcessUtil.c
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

#include "ProcessUtil.h"


/***  FUNCTIONS  ***/

/*  */
Boolean
ProcessCurrentIsFront ( void )
{
	Boolean					isFront;
	ProcessSerialNumber		myPSN;
	OSErr					theErr;
	
	/* get my process serial number. IM:Processes 2-21 */
	theErr = GetCurrentProcess ( &myPSN );
	
	if( theErr != noErr )
	{
		/* might want to do something else here to handle the error */
		return false;
	}
	
	isFront = ProcessIsFront ( myPSN.lowLongOfPSN );
	
	return isFront;
} /* ProcessCurrentIsFront */


/*  */
Boolean
ProcessIsFront ( long theLowPSN )
{
	OSErr					theErr;
	ProcessSerialNumber		frontPSN;
	
	theErr = GetFrontProcess ( &frontPSN );
	
	if ( theErr != noErr )
	{
		/* might want to do something else here to handle the error */
		
		return false;
	}
	
	return ( frontPSN.lowLongOfPSN == theLowPSN );
} /* ProcessIsFront */


/*****  EOF  *****/
