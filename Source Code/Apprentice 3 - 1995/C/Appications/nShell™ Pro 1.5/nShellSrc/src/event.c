/* ==========================================

	event.c
	
	Copyright (c) 1993,1994,1995 Newport Software Development

   ========================================== */

#define EVENT_DEBUG 0

#include <AppleEvents.h>

#include "nsh.h"
#include "multi.h"

#include "event.proto.h"
#include "multi.proto.h"
#include "utility.proto.h"

#if ! FULL_APP

#include "drop.proto.h"

extern int		gTooLate;

#endif

extern Boolean	gHasAppleEvents;
extern Boolean	gEventOpen;
extern Boolean	gEventQuit;

/* ========================================== */

int event_install_core( AEEventID event, EventHandlerProcPtr handler)
{
    OSErr error;
    
	error = AEInstallEventHandler(kCoreEventClass, event, handler, 0, false);
    
    if (error) {
    	error_note("\pcould not install Apple Event handler");
    	gHasAppleEvents = 0;
    	}
    	
    return(!error);
}

void event_init(void)
{
    if (gHasAppleEvents)
    
		if ( event_install_core( kAEOpenApplication, event_open_app ) )
		
			if ( event_install_core( kAEQuitApplication, event_quit_app ) )
				
				if ( event_install_core( kAEOpenDocuments, event_open_docs ) )
			
					event_install_core( kAEPrintDocuments, event_print_docs );
}

void event_do(EventRecord *AERecord)
{
    OSErr error;
	error = AEProcessAppleEvent(AERecord);
}

pascal OSErr event_open_app(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#if EVENT_DEBUG
	Notify("\pGot Run AEvent", 0);
#endif
	
	gEventOpen = 1;
    return(noErr);
}

pascal OSErr event_quit_app(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#if EVENT_DEBUG
	Notify("\pGot Quit AEvent", 0);
#endif
	
	gEventQuit = 1;
    return(noErr);
}

pascal OSErr event_open_docs(AppleEvent *theAppleEvent, AppleEvent *reply, long refIn)
{
	int			GotSome;
	int			myRefNum;
	FInfo		myInfo;
	FSSpec		myFSS;
	AEDescList	docList;
	int			myErr;
	long		index, itemsInList;
	long		actualSize;
	AEKeyword	keywd;
	DescType	returnedType;
	
#if EVENT_DEBUG
	Notify("\pGot ODOC AEvent", 0);
#endif
	
#if !FULL_APP

	if (gTooLate)
 		 return(errAEEventNotHandled);

#endif
	
	GotSome = 0;

	// get the direct parameter--a descriptor list--and put it into docList
	
	myErr = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList);
	
	if (myErr) 
		error_note("\pfailed AEGetParamDesc");

	// check for missing required parameters
	
	myErr = event_got_parms(theAppleEvent);
	
	if (myErr) {
		error_note("\pmissing Apple Event parameters");
		myErr = AEDisposeDesc(&docList);
		return	myErr;
	}

	// count the number of descriptor records in the list
	myErr = AECountItems (&docList,&itemsInList);

	// now get each descriptor record from the list, coerce
	// the returned data to an FSSpec record, and open the
	// associated file
	for (index=1; index<=itemsInList; index++) {
		myErr = AEGetNthPtr(&docList, index, typeFSS, &keywd,
							&returnedType, (Ptr)&myFSS,
							sizeof(myFSS), &actualSize);
		if (myErr)
			error_note("\pfailed AEGetNthPtr");
		else {
#if FULL_APP
			myRefNum = event_vRefNum( &myFSS );
			GetFInfo( myFSS.name, myRefNum, &myInfo );
			if ( myInfo.fdType == TRANSCRIPT_FILE ) {
				mf_open_file( &myFSS );
				GotSome = 1;
				}
			if ( myInfo.fdType == SCRIPT_FILE ) {
				mf_open_script( &myFSS );
				GotSome = 1;
				}
#else
			drop_fss( &myFSS );
#endif
			}
		}

	if (!GotSome) 						// if we couldn't open anything else,
		gEventOpen = 1;					// at least open an empty shell

	myErr = AEDisposeDesc(&docList);


	return(noErr);
}

pascal OSErr event_print_docs(AppleEvent *messagein, AppleEvent *reply, long refIn)
{
#if EVENT_DEBUG
	Notify("\pGot Print AEvent", 0);
#endif
	
  return(errAEEventNotHandled);
}

int	event_got_parms (AppleEvent *theAppleEvent)
{
	DescType	returnedType;
	Size		actualSize;
	OSErr		myErr;

	myErr = AEGetAttributePtr(	theAppleEvent,
							  	keyMissedKeywordAttr,
							  	typeWildCard,
							  	&returnedType,
							  	nil,
							  	0,
							  	&actualSize );

	if (myErr == errAEDescNotFound)		// you got all the required parameters
		return(noErr);

	if (myErr == noErr)					// you missed a required parameter
		return(errAEParamMissed);

	return(myErr);						// the call to AEGetAttributePtr failed
}

int event_vRefNum( FSSpec *specPtr )
{
	WDPBRec			wPB;
	OSErr			err;
	
	wPB.ioCompletion	= (ProcPtr)NULL;
	wPB.ioVRefNum		= specPtr->vRefNum;
	wPB.ioWDDirID		= specPtr->parID;
	wPB.ioWDProcID		= APPLICATION_SIGNATURE;
	wPB.ioNamePtr		= (StringPtr)NULL;

	err = PBOpenWD( &wPB, false );
	
	if ( err )
		return( 0 );
	else
		return( wPB.ioVRefNum );
}

