/* apple_event_support.c */

#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Values.h>
#include "apple_event_support.h"
#include "TransSkel.h"
#include "load_files.h"
#include "error.h"
#include "windows.h"
#include "dil.h"
#include "globals.h"
	
void myEventInit( void )
{
	OSErr 		err;
	
	// make sure Apple Events exist before trying to set them up.
	
	if( SkelQuery( skelQHasAppleEvents ))
	{
		err = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication,
									DoOpenApp, 0L, false );
		
		err = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,
									DoOpenDoc, 0L, false );
			
		err = AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,
									DoPrintDoc, 0L, false );
			
		err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
									DoQuitApp, 0L, false );
	}
	
	// register my event handling function
	SkelSetAEHandler( myHandleHighLevelEvent );
}

pascal void myHandleHighLevelEvent( EventRecord *theEvent )
{
	OSErr err;

	// let MacOS do the work for me (convert it to an AE, then route it to the right function)
	err = AEProcessAppleEvent( theEvent );

	if( err != noErr )
		myError( "Failed to deal with Apple Event, sorry!", false );
}

pascal OSErr DoOpenApp( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon )
{
	; // nothing special.
	
	return( noErr );
}

pascal OSErr DoOpenDoc( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon )
{
	// I love think reference!

	FSSpec spec;
	AEDescList docList;
	OSErr err;
	long index, itemsinlist;
	Size actualSize;
	AEKeyword keywd;
	DescType returnedType;
	dil_rec *temp;

	// we may have been brought from the background, and need to refresh (ie from the finder
	// for a drag & drop)	
	myUpdate(false);

	err = AEGetParamDesc( theAppleEvent, keyDirectObject, typeAEList, &docList );
	
	err = AECountItems( &docList, &itemsinlist );
	
	for( index=1 ; index <= itemsinlist ; index++ )
	{
		err = AEGetNthPtr( &docList, index, typeFSS, &keywd, &returnedType, (Ptr) &spec,
							sizeof(spec), &actualSize );
							
		// check for 'GIFf' type?
		
		// store/restore CurDil if none are open yet (probly b/c drag & drop start up)					
		temp = GetCurDil();
		
		LoadAndAddGIF( &spec );
		
		if( temp == nil )
			SetCurDil( temp );
	}
		
	err = AEDisposeDesc( &docList );
	
	// get rid of the 'processing..' message
	myUpdate(false);

	return( noErr );
}

pascal OSErr DoPrintDoc( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon )
{
	;
	
	return( noErr );
}

pascal OSErr DoQuitApp( AppleEvent *theAppleEvent, AppleEvent *reply, long refCon )
{
	// quit peacefully
	SkelStopEventLoop();
	
	return( noErr );
}
