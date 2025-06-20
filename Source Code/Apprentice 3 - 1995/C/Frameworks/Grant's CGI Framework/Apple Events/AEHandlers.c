/*****
 *
 *	AEHandlers.c
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

#include <AppleEvents.h>

#include "constants.h"
#include "globals.h"

#include "AEFunc.h"
#include "CGI.h"
#include "ErrorUtil.h"
#include "Quit.h"

#include "AEHandlers.h"


/***  REQUIRED SUITE  ***/

/* Sent if the application is opened by the Finder with no documents. */
pascal OSErr
doAEOpenApp ( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon )
{
	/* You may want to add code here to handle the case where the application
		is just opened with no other startup tasks (such as opening documents). */
	
	return noErr;
} /* doAEOpenApp */


/* The application is expected to open the given documents, if possible */
pascal OSErr
doAEOpenDoc ( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon )
{
	OSErr		theErr;
	OSErr		docErr;
	AEDescList	docList;
	long		numDocs;
	long		counter;
	AEKeyword	keywd;
	DescType	returnedType;
	FSSpec		fileSpec;
	Size		actualSize;
	
	docList.descriptorType	= typeNull;
	docList.dataHandle		= nil;
	
	/* get the direct parameter - a descriptor list - and put it into docList */
	theErr = AEGetParamDesc ( theAppleEvent, keyDirectObject, typeAEList, &docList );
	
	if ( theErr == noErr )
	{
		/* check for missing required parameters */
		theErr = MyGotRequiredParams ( theAppleEvent );
		
		if ( theErr != noErr )
		{
			AEDisposeDesc ( &docList );
		}
	}
	
	if ( theErr == noErr )
	{
		/* count the number of descriptor records in the list.
			Should be at least 1 since we got called and no error */
		theErr = AECountItems ( &docList, &numDocs );
		
		if ( theErr != noErr )
		{
			AEDisposeDesc ( &docList );
		}
	}
	
	if ( theErr == noErr )
	{
		/* now get each descriptor record from the list, coerce the returned data to an
			FSSpec record, and open the associated file */
		for ( counter = 1; counter <= numDocs; counter++ )
		{
			docErr = AEGetNthPtr ( &docList, counter, typeFSS, &keywd, &returnedType,
				(Ptr)&fileSpec, sizeof(fileSpec), &actualSize );
			
			if ( docErr == noErr )
			{
				/* ��� you may want to add code, for handling document opening, here */
			}
			
			if ( docErr != noErr )
			{
				theErr = docErr;
			}
		}
	}
	
	if ( theErr == noErr )
	{
		theErr = AEDisposeDesc ( &docList );
	}
	
	return theErr;
} /* doAEOpenDoc */


/* Handle document printing */
pascal OSErr
doAEPrintDoc (AppleEvent *theAppleEvent, AppleEvent *reply, long refCon)
{
	
	return noErr;
} /*  */


/* Set application to quit. IM:IAC 9-8 */
pascal OSErr
doAEQuitApp ( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon )
{
	OSErr		theErr;
	Boolean		success;
	
	/* check for missing required parameters */
	theErr = MyGotRequiredParams ( theAppleEvent );
	
	if ( theErr == noErr)
	{
		success = QuitPrepare ( true );
	}
	else
	{
		success = false;
	}
	
	/* return error result */
	if ( success == true )
	{
		return noErr;
	}
	else if ( theErr == noErr )
	{
		return userCanceledErr;
	}
	else
	{
		return theErr;
	}
} /* doAEQuitApp */


/**  INITIALIZATION **/
#pragma mark -

#pragma segment Startup
/*  */
OSErr
myInstallAEHandlers ( void )
{
	OSErr				resultErr;
	OSErr				theErr;
	AEEventHandlerUPP	theUPP;

	resultErr = noErr;
	
	/* NewAEEventHandlerProc as discussed in
		"Odds and Ends" by Dave Mark, in 'MacTech' Aug94.
		Revised usage in "The Required Apple Events" by Dave Mark, in 'MacTech' Dec94. */
	theUPP	= NewAEEventHandlerProc ( doAEOpenApp );
	theErr	= AEInstallEventHandler ( kCoreEventClass, kAEOpenApplication, theUPP, 0L, false );
	if ( theErr != noErr )
	{
		/* openapp AE install failed */
		resultErr = theErr;
	}
	
	theUPP	= NewAEEventHandlerProc ( doAEOpenDoc );
	theErr	= AEInstallEventHandler ( kCoreEventClass, kAEOpenDocuments, theUPP, 0L, false );
	if ( theErr != noErr )
	{
		/* opendoc AE install failed */
		resultErr = theErr;
	}

	theUPP	= NewAEEventHandlerProc ( doAEPrintDoc );
	theErr	= AEInstallEventHandler ( kCoreEventClass, kAEPrintDocuments, theUPP, 0L, false );
	if ( theErr != noErr )
	{
		/* printdoc AE install failed */
		resultErr = theErr;
	}

	theUPP	= NewAEEventHandlerProc ( doAEQuitApp );
	theErr	= AEInstallEventHandler ( kCoreEventClass, kAEQuitApplication, theUPP, 0L, false );
	if ( theErr != noErr )
	{
		/* quit app AE install failed */
		resultErr = theErr;
	}

	theUPP	= NewAEEventHandlerProc ( CGIAESearchDoc );
	theErr	= AEInstallEventHandler ( kAEClassCGI, kAEIDSearchDoc, theUPP, 0L, false );
	if ( theErr != noErr )
	{
		/* quit app AE install failed */
		resultErr = theErr;
	}

	return resultErr;
} /* myInstallAEHandlers */
#pragma segment AppleEvents


/*****  EOF  *****/
