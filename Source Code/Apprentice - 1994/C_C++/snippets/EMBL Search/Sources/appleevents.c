/*
*********************************************************************
*	
*	AppleEvents.c
*	Handling of required AppleEvents (based on DTS sample code)
*
*	Rainer Fuchs
*	EMBL Data Library
*	Postfach 10.2209
*	D-6900 Heidelberg, FRG
*	E-mail: fuchs@embl-heidelberg.de
*
*	Copyright � 1992 EMBL Data Library
*		
**********************************************************************
*	
*/ 

#include <AppleEvents.h>

#include "EMBL-Search.h"
#include "EMBL-Search.rsrc.h"


/*
******************************* Prototypes ***************************
*/

#include "appleevents.h"
#include "util.h"
#include "checkapp.h"
#include "window.h"

static Boolean MissedAnyParameters(AppleEvent *message);
static pascal OSErr DoAEOpenApplication(AppleEvent *message, AppleEvent *reply, long refcon);
static pascal OSErr DoAEOpenDocuments(AppleEvent *message, AppleEvent *reply, long refcon);
static pascal OSErr DoAEPrintDocuments(AppleEvent *message, AppleEvent *reply, long refcon);
static pascal OSErr DoAEQuitApplication(AppleEvent *message, AppleEvent *reply, long refcon);


/*
******************************** Global variables *****************
*/

typedef struct {
	AEEventClass	theEventClass;
	AEEventID		theEventID;
	ProcPtr			theHandler;
} triplets;

triplets keywordsToInstall[] = {
	{ kCoreEventClass,	kAEOpenApplication,	(ProcPtr) DoAEOpenApplication },
	{ kCoreEventClass,	kAEOpenDocuments,	(ProcPtr) DoAEOpenDocuments },
	{ kCoreEventClass,	kAEPrintDocuments,	(ProcPtr) DoAEPrintDocuments },
	{ kCoreEventClass,	kAEQuitApplication,	(ProcPtr) DoAEQuitApplication }
		/* The above are the four required AppleEvents. */
};

extern Boolean gHasAppleEvents;
extern Boolean gQuitApplication;



/**************************************
*	Simply calls AEProcessAppleEvent
*/

void DoHighLevelEvent(EventRecord event)
{
	AEProcessAppleEvent(&event);
}


/**************************************
*	Initialize our AppleEvent dispatcher table.
*	For every triplet of entries in keywordsToInstall, we make a call to
*	AEInstallEventHandler().
*/

void InitAppleEvents(void)
{
	OSErr	err;
	long	result;
	short	i;

	if (gHasAppleEvents) {
		for (i = 0; i < (sizeof(keywordsToInstall) / sizeof(triplets)); ++i) {
			err = AEInstallEventHandler(
				keywordsToInstall[i].theEventClass,	/* What class to install.  */
				keywordsToInstall[i].theEventID,	/* Keywords to install.    */
				keywordsToInstall[i].theHandler,	/* The AppleEvent handler. */
				0L,									/* Unused refcon.		   */
				FALSE								/* Only for our app.	   */
			);

			if ( err != noErr ) {
				ErrorMsg(ERR_AEINSTALL);
				return;
			}
		}
	}
}

/**************************************
*	Used to check for any unread required parameters. Returns TRUE if we
*	missed at least one.
*/

static Boolean MissedAnyParameters(AppleEvent *message)
{
	OSErr		err;
	DescType	ignoredActualType;
	AEKeyword	missedKeyword;
	Size		ignoredActualSize;
	EventRecord	event;

	err = AEGetAttributePtr(	/* SEE IF PARAMETERS ARE ALL USED UP.		  */
		message,				/* AppleEvent to check.						  */
		keyMissedKeywordAttr,	/* Look for unread parameters.				  */
		typeKeyword,			/* So we can see what type we missed, if any. */
		&ignoredActualType,		/* What is would have been if not coerced.	  */
		(Ptr)&missedKeyword,	/* Data area.  (Keyword not handled.)		  */
		sizeof(missedKeyword),	/* Size of data area.						  */
		&ignoredActualSize		/* Actual data size.						  */
	);

/* No error means that we found some unused parameters. */

	if (err == noErr) {
		event.message = *(long *) &ignoredActualType;
		event.where = *(Point *) &missedKeyword;
		err = errAEEventNotHandled;
	}

/* errAEDescNotFound means that there are no more parameters.  If we get
an error code other than that, flag it. */

	return(err != errAEDescNotFound);
}


/**************************************
*	Open Application AppleEvent
*/

static pascal OSErr	DoAEOpenApplication(AppleEvent *message, AppleEvent *reply,
										long refcon)
{
	return(noErr);	/* no action required */
}


/**************************************
*	Open Document AppleEvent
*/
static pascal OSErr	DoAEOpenDocuments(AppleEvent *message, AppleEvent *reply,
										long refcon)
{
	OSErr		err,err2;
	AEDescList	theDesc;
	FSSpec		theFSS;
	short		loop;
	long		numFilesToOpen;
	AEKeyword	ignoredKeyWord;
	DescType	ignoredType;
	Size		ignoredSize;
	short		wdRefNum;
	Str255		fName;
	OSType		fType;
	FInfo		fndrInfo;
	WDPtr		dummy;
	
	if ( (err = AEGetParamDesc(message, keyDirectObject, typeAEList, &theDesc))
			!= noErr)
		return(err);

	if (!MissedAnyParameters(message)) {

	/* Got all the parameters we need.  Now, go through the direct object,
		see what type it is, and parse it up. */

		err = AECountItems(&theDesc, &numFilesToOpen);
		if ( err == noErr ) {
			/* We have numFilesToOpen that need opening. Go to it... */

			for (loop = 1; loop <= numFilesToOpen && err == noErr; ++loop) {
				err = AEGetNthPtr(		/* GET NEXT IN THE LIST...		 */
					&theDesc,			/* List of file names.			 */
					loop,				/* Item # in the list.			 */
					typeFSS,			/* Item is of type FSSpec.		 */
					&ignoredKeyWord,	/* Returned keyword -- we know.  */
					&ignoredType,		/* Returned type -- we know.	 */
					(Ptr)&theFSS,		/* Where to put the FSSpec info. */
					sizeof(theFSS),		/* Size of the FSSpec info.		 */
					&ignoredSize		/* Actual size -- we know.		 */
				);
				if (err != noErr) break;
				
				/* Convert FSSpec to HFS working dir and file name */
				if( FSSpecToHFS(&theFSS,&wdRefNum,fName) != noErr) {
					ErrorMsg(ERR_AEOPEN);
					break;
				}
				
				/* get file type */
				if( FSpGetFInfo(&theFSS,&fndrInfo) != noErr ) {
					ErrorMsg(ERR_AEOPEN);
					break;
				}
				else fType = fndrInfo.fdType;
			
				/* open document */
				if( !DoDocOpen(fType, wdRefNum,fName, &dummy)) {
					err=errAEEventNotHandled;
					break;
				}
			}
		}
	}
	err2 = AEDisposeDesc(&theDesc);
	return( (err != noErr) ? err : err2);
}

/**************************************
*	Print Document AppleEvent
*/

static pascal OSErr	DoAEPrintDocuments(AppleEvent *message, AppleEvent *reply, long refcon)
{
	OSErr		err,err2;
	AEDescList	theDesc;
	FSSpec		theFSS;
	short		loop;
	long		numFilesToPrint;
	AEKeyword	ignoredKeyWord;
	DescType	ignoredType;
	Size		ignoredSize;
	short		wdRefNum;
	Str255		fName;
	OSType		fType;
	FInfo		fndrInfo;
	
	if ( (err = AEGetParamDesc(message, keyDirectObject, typeAEList, &theDesc))
			!= noErr)
		return(err);

	if (!MissedAnyParameters(message)) {

	/* Got all the parameters we need.  Now, go through the direct object,
		see what type it is, and parse it up. */

		err = AECountItems(&theDesc, &numFilesToPrint);
		if ( err == noErr ) {
			/* We have numFilesToPrint that need printing. Go to it... */

			for (loop = 1; loop <= numFilesToPrint && err == noErr; ++loop) {
				err = AEGetNthPtr(		/* GET NEXT IN THE LIST...		 */
					&theDesc,			/* List of file names.			 */
					loop,				/* Item # in the list.			 */
					typeFSS,			/* Item is of type FSSpec.		 */
					&ignoredKeyWord,	/* Returned keyword -- we know.  */
					&ignoredType,		/* Returned type -- we know.	 */
					(Ptr)&theFSS,		/* Where to put the FSSpec info. */
					sizeof(theFSS),		/* Size of the FSSpec info.		 */
					&ignoredSize		/* Actual size -- we know.		 */
				);
				if (err != noErr) break;
				
				/* Convert FSSpec to HFS working dir and file name */
				if( FSSpecToHFS(&theFSS,&wdRefNum,fName) != noErr) {
					ErrorMsg(ERR_AEOPEN);
					break;
				}
				
				/* get file type */
				if( FSpGetFInfo(&theFSS,&fndrInfo) != noErr ) {
					ErrorMsg(ERR_AEOPEN);
					break;
				}
				else fType = fndrInfo.fdType;
			
				/* print document */
				if( !DoDocPrint(fType, wdRefNum,fName)) {
					err=errAEEventNotHandled;
					break;
				}
			}
		}
	}
	err2 = AEDisposeDesc(&theDesc);
	return( (err != noErr) ? err : err2);
}


/**************************************
*	Quit Application AppleEvent
*/

static pascal OSErr	DoAEQuitApplication(AppleEvent *message, AppleEvent *reply, long refcon)
{
	if(CloseAllWindows(FALSE)) {
		gQuitApplication=TRUE;
		LoadScrap();
	}
	return(noErr);
}