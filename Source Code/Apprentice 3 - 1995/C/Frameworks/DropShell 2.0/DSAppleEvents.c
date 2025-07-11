/******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DSAppleEvents.c
**
**   Description:	Generic AppleEvent handling routines
**					
**					This is the set of routines for handling the required Apple events.
**					You should NEVER have to modify this file!!!
**					Simply add code in DSUserProcs to the routines called by these.
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**	MTC			Marshall Clow
**	SCS			Stephan Somogyi
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Author	Description
**	---------	------	---------------------------------------------
**	20 Feb 94	LDR		Modified _HandleDocs to pass item count to preflight & postflight
**	11 Dec 93	SCS		Universal Headers/UPPs (Phoenix 68k/PPC & PPCC)
**						Skipped System 6 compatible rev of DropShell source
**	11/24/91	LDR		Added a handler for 'pdoc' as per DTS recommendation
**							This caused some reorg & userProc routine changes
**							I also created a new common AEVT doc extractor
**						Cleaned up error handling by adding FailErr
**						Cleaned up the placement of braces
**						Added the passing of a userDataHandle to the odoc/pdoc routines
**	10/29/91	SCS		Changes for THINK C 5
**	10/28/91	LDR		Officially renamed DropShell (from QuickShell)
**						Added a bunch of comments for clarification
**	10/06/91	MTC		Converted to MPW C
**	04/09/91	LDR		Added to Projector
**
******************************************************************************/

#include "DSGlobals.h"
#include "DSUserProcs.h"

#include "DSAppleEvents.h"

AEEventHandlerUPP	OAPPHandlerUPP, ODOCHandlerUPP, PDOCHandlerUPP, QUITHandlerUPP;

/*
	This routine does all initialization for AEM, including the
	creation and then population of the dispatch table.
*/
#pragma segment Initialize
pascal void InitAEVTStuff ()  {
	OSErr aevtErr;

	aevtErr = noErr;
	
	if ( aevtErr == noErr )	{
		OAPPHandlerUPP = NewAEEventHandlerProc(HandleOAPP);
		aevtErr = AEInstallEventHandler ( kCoreEventClass, kAEOpenApplication, OAPPHandlerUPP, 0, false );
	}


	if ( aevtErr == noErr )	{
		ODOCHandlerUPP = NewAEEventHandlerProc(HandleODOC);
		aevtErr = AEInstallEventHandler ( kCoreEventClass, kAEOpenDocuments, ODOCHandlerUPP, 0, false );
	}

	if ( aevtErr == noErr )	{
		PDOCHandlerUPP = NewAEEventHandlerProc(HandlePDOC);
		aevtErr = AEInstallEventHandler ( kCoreEventClass, kAEPrintDocuments, PDOCHandlerUPP, 0, false );
	}

	if ( aevtErr == noErr )	{
		QUITHandlerUPP = NewAEEventHandlerProc(HandleQuit);
		aevtErr = AEInstallEventHandler ( kCoreEventClass, kAEQuitApplication, QUITHandlerUPP, 0, false );
	}

	if ( aevtErr == noErr )
		InstallOtherEvents ();

		
	if ( aevtErr != noErr )
		;		// report an error if you are so included
}



/*	
	This routine is a utility routine for checking that all required 
	parameters in the Apple event have been used.
*/
#pragma segment Main
OSErr GotRequiredParams ( AppleEvent *theAppleEvent ) {
	DescType	typeCode;
	Size		actualSize;
	OSErr		retErr, err;

	err = AEGetAttributePtr ( theAppleEvent, keyMissedKeywordAttr,
					typeWildCard, &typeCode, NULL, 0, &actualSize );
	
	if ( err == errAEDescNotFound )	// we got all the required params: all is ok
		retErr = noErr;
	else if ( err == noErr )
		retErr = errAEEventNotHandled;
	else 
		retErr = err;
	
	return retErr;
}

/*
	This is another routine useful for showing debugging info.
	It calls the ErrorAlert routine from DSUtils to put up the 
	error message.

*/
void FailErr(OSErr err) {

	if (err != noErr)
		ErrorAlert(kErrStringID, kAEVTErr, err);
}

/*	
	This routine is the handler for the oapp (Open Application) event.
	
	It first checks the number of parameters to make sure we got them all 
	(even though we don't want any) and then calls the OpenApp userProc in QSUserProcs.
	Finally it checks to see if the caller wanted a reply & sends one, setting any error.
*/
#pragma segment Main
pascal OSErr HandleOAPP ( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon ) {
#pragma unused ( handlerRefcon )
	OSErr err;

	FailErr(err = GotRequiredParams ( theAppleEvent ));

	// let's show the user the splash screen
	if ( gSplashScreen != NULL )
		ShowWindow ( gSplashScreen );

	OpenApp ();		// pass it on to the app specific routine

	if ( reply->dataHandle != NULL )	/*	a reply is sought */
		FailErr(err = AEPutParamPtr ( reply, 'errs', 'TEXT', "Opening", 7 ));
	
	return err;
}


/*	
	This routine is the handler for the quit (Quit Application) event.
	
	It first checks the number of parameters to make sure we got them all 
	(even though we don't want any) and then calls the QuitApp userProc in QSUserProcs.
	Finally it checks to see if the caller wanted a reply & sends one, setting any error.
*/

#pragma segment Main
pascal OSErr HandleQuit ( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon ) {
#pragma unused ( handlerRefcon )
	OSErr err;
	
	FailErr( err = GotRequiredParams ( theAppleEvent ));

	QuitApp ();		// pass it on to the app specific routine

	if ( reply->dataHandle != NULL )	/*	a reply is sought */
		FailErr(err = AEPutParamPtr ( reply, 'errs', 'TEXT', "Qutting", 7 ));
	
	return err;
}


/*	
	This routine is the low level processing routine for both the 
	odoc (Open Document) and pdoc (Print Document) events.
	
	This routine is the key one, since this is how we get the list of
	files/folders/disks to process.  The first thing to do is the get the
	list of files, and then make sure that's all the parameters (should be!).
	We then send call the PreflightDocs routine (from DSUserProcs), process
	each file in the list by calling OpenDoc (again in DSUserProcs), and finally
	call PostflightDocs (you know where) and setting a return value.
*/
#pragma segment Main
pascal OSErr _HandleDocs ( AppleEvent *theAppleEvent, AppleEvent *reply, Boolean opening ) {
#pragma unused ( reply )
#pragma unused ( handlerRefcon )
	OSErr		err;
	FSSpec		myFSS;
	AEDescList	docList;
	long		index, itemsInList;
	Size		actualSize;
	AEKeyword	keywd;
	DescType	typeCode;
	Handle		userDataHandle;
	

	FailErr(err = AEGetParamDesc ( theAppleEvent, keyDirectObject, typeAEList, &docList ));
	FailErr(err = GotRequiredParams ( theAppleEvent ));

	/*	How many items do we have?. */
	/* NOTE: Moved here in DS 2.0 due to requests for this info in preflighter */
	FailErr(err = AECountItems ( &docList, &itemsInList ));
	
	if (PreFlightDocs (opening, itemsInList, &userDataHandle))	{	// let the app do any preflighting it might need

		for ( index = 1; index <= itemsInList; index++ ) {
			FailErr(err = AEGetNthPtr ( &docList, index, typeFSS, &keywd, &typeCode,
					(Ptr) &myFSS, sizeof ( myFSS ), &actualSize ));
	
			OpenDoc( &myFSS, opening, userDataHandle );	// call the userProc
		}
	
		PostFlightDocs (opening, itemsInList, userDataHandle);	// cleanup time
	}
	else
		err = errAEEventNotHandled;	// tells AEM that we didn't handle it!
		
	FailErr(AEDisposeDesc ( &docList ));

	return err;
}

/*
	This routine is the handler for the odoc (Open Document) event.
	
	The odoc event simply calls the common _HandleDocs routines, which will
	do the dirty work of parsing the AEVT & calling the userProcs.
*/
#pragma segment Main
pascal OSErr HandleODOC ( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon ) {
#pragma unused ( handlerRefcon )
	
	return (_HandleDocs(theAppleEvent, reply, true));	// call the low level routine
}

/*
	This routine is the handler for the pdoc (Print Document) event.
	
	The pdoc event like the odoc simply calls the common _HandleDocs routines
*/
#pragma segment Main
pascal OSErr HandlePDOC ( AppleEvent *theAppleEvent, AppleEvent *reply, long handlerRefcon ) {
#pragma unused ( handlerRefcon )
	
	return (_HandleDocs(theAppleEvent, reply, false));	// call the low level routine
}

/*	
	This is the routine called by the main event loop, when a high level
	event is found.  Since we only deal with Apple events, and not other
	high level events, we just pass everything onto the AEM via AEProcessAppleEvent
*/
#pragma segment Main
pascal void DoHighLevelEvent ( EventRecord *event ) {

	FailErr ( AEProcessAppleEvent ( event ) );
}
