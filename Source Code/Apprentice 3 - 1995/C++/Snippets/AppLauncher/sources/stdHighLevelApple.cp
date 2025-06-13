/* 
*	stdHighLevelApple.cp
*
*	AppLauncher
*	^^^^^^^^^^^
*
*	4 required high-level AE's
*	Uses UPP's for future PowerMac compatibility
*	© Andrew Nemeth (where applicable), Warrimoo Australia 1994, 1995
*
*	File created:		8 Mar 95.
*	Modified:			8 Mar 95.
*/


#include	"stdHighLevelApple.h"							//	Application High-Level AE
#include	"gConstDefines.h"								//	Global #defines & const's
#include	"gVariables.h"									//	Global variable defn & 'externs'

#include 	<AppleEvents.h>								//	High level AE
#include 	<gestaltEqu.h>									//	Gestalt




//	LOCAL function prototypes…
//
static pascal OSErr	doOpenApp		( AppleEvent, AppleEvent, long );
static pascal OSErr	doOpenDoc		( AppleEvent, AppleEvent, long );
static pascal OSErr	doPrintDoc	( AppleEvent, AppleEvent, long );
static pascal OSErr	doQuitApp		( AppleEvent  aeEvent, AppleEvent  aeReply, long refCon );

//	FILE globals…
//
AEEventHandlerUPP	gDoOpenAppUPP, 
				gDoOpenDocUPP, 
				gDoPrintDocUPP, 
				gDoQuitAppUPP;



void		highLevelEventInit( void )
//
//	Function to initialise 
//	Apple Event handlers 
//
{
	const long 	kGestaltMask = 1L;
	OSErr		err = noErr;
	long			feature;
	

	err = ::Gestalt( gestaltAppleEventsAttr, &feature );
	
	if ( err != noErr )
		{
//														no Gestalt!
		return;
		}
	else																				// No Apple events!
		{
		if ( ! ( feature & ( kGestaltMask << gestaltAppleEventsPresent ) ) )
			{
//														high-level AE not supported!
			return;
			}
		}

	//	Now install the High Level event handlers for the 4 main AE's.
	//	If there's a problem with any of them, drop into Jasik Land…

	gDoOpenAppUPP = NewAEEventHandlerProc( doOpenApp );
	err = ::AEInstallEventHandler( kCoreEventClass, kAEOpenApplication,
							gDoOpenAppUPP,  0L, FALSE );
	if ( err  !=  noErr )
		{
		::SysBeep(1);
		::DebugStr( "\pInstall of kAEOpenApplication handler failed" );
		}
	
	gDoOpenDocUPP = NewAEEventHandlerProc( doOpenDoc );
	err = ::AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,
							gDoOpenDocUPP,  0L, FALSE );
	if ( err  !=  noErr )
		{
		::SysBeep(1);
		::DebugStr( "\pInstall of kAEOpenDocuments handler failed" );
		}
	
	gDoPrintDocUPP = NewAEEventHandlerProc( doPrintDoc );
	err = ::AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,
							gDoPrintDocUPP,  0L, FALSE );
	if ( err  !=  noErr )
		{
		::SysBeep(1);
		::DebugStr( "\pInstall of kAEPrintDocuments handler failed" );
		}
	
	gDoQuitAppUPP = NewAEEventHandlerProc( doQuitApp );
	err = ::AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
							gDoQuitAppUPP,  0L, FALSE );
	if ( err  !=  noErr )
		{
		::SysBeep(1);
		::DebugStr( "\pInstall of kAEQuitApplication handler failed" );
		}
}


pascal OSErr 	doOpenApp( AppleEvent 		aeEvent, 
						AppleEvent	aeReply, 
						long			lgRefcon )
//
//	The routine called at startup.
//	Put up splash screen.
//
{
//														silence the compiler warnings!
	if ( lgRefcon || &aeEvent || &aeReply )
		;

	return( noErr );
}



pascal OSErr 	doOpenDoc( AppleEvent 		aeEvent, 
						AppleEvent 	aeReply, 
						long 		lgRefcon )
//
//	Not supported.  Just start app instead.
//	Put up splash screen.
//
{
	return( doOpenApp( aeEvent, aeReply, lgRefcon ) );
}



pascal OSErr 	doPrintDoc( AppleEvent 		aeEvent, 
						AppleEvent 	aeReply, 
						long 		lgRefcon )
//
//	Print not supported by this application
//
{
//														silence the compiler warnings!
	if ( lgRefcon || &aeEvent || &aeReply )
		NULL;
//														exit gracefully
	gptrGlobalsRec->boolDone = TRUE;
	return( noErr );
}



pascal OSErr 	doQuitApp( AppleEvent 		aeEvent, 
						AppleEvent	aeReply, 
						long 		lgRefcon )
//
//	Functions to perform on 'quit' Apple Event.
//
{
//														silence the compiler warnings!
	if ( lgRefcon || &aeEvent || &aeReply )
		NULL;

	gptrGlobalsRec->boolDone = TRUE;
	return( noErr );
}
