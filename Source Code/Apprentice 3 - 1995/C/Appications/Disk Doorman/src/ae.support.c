#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Values.h>

#define kGestaltMask	1L

/* FUNCTION HEADERS */

void			EventInit( void );
pascal OSErr	DoOpenApp( AppleEvent theAppleEvent, AppleEvent reply,
							long refCon );
pascal OSErr	DoOpenDoc( AppleEvent theAppleEvent, AppleEvent reply,
							long refCon );
pascal OSErr	DoPrintDoc( AppleEvent theAppleEvent, AppleEvent reply,
							long refCon );
pascal OSErr	DoQuitApp( AppleEvent theAppleEvent, AppleEvent reply,
							long refCon );

/* FUNCTIONS */

//void main( void )
//{
//	ToolBoxInit();
//	WindowInit();
//	EventInit();
//	EventLoop();
//}

void EventInit( void )
{
	OSErr 		err;
	long		feature;
	
	err = Gestalt( gestaltAppleEventsAttr, &feature );
	
	if( err != noErr )
	{
		;
		return;
	}
	else
	{
		if( ! ( feature & ( kGestaltMask << gestaltAppleEventsPresent )))
		{
			;
			return;
		}
	}
	
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenApplication,
								DoOpenApp, 0L, false );
	if( err != noErr ) 
		;//DrawEventString( "\pkAEOpenApplication AE not available!" );
	
	err = AEInstallEventHandler( kCoreEventClass, kAEOpenDocuments,
								DoOpenDoc, 0L, false );
	if( err != noErr ) 
		;//DrawEventString( "\pkAEOpenDocuments AE not available!" );
		
	err = AEInstallEventHandler( kCoreEventClass, kAEPrintDocuments,
								DoPrintDoc, 0L, false );
	if( err != noErr ) 
		;//DrawEventString( "\pkAEPrintDocuments AE not available!" );
		
	err = AEInstallEventHandler( kCoreEventClass, kAEQuitApplication,
								DoQuitApp, 0L, false );
	if( err != noErr ) 
		;//DrawEventString( "\pkAEQuitApplication AE not available!" );
}

pascal OSErr DoOpenApp( AppleEvent theAppleEvent, AppleEvent reply,
						long refCon )
{
	;
}

pascal OSErr DoOpenDoc( AppleEvent theAppleEvent, AppleEvent reply,
						long refCon )
{
	;
}

pascal OSErr DoPrintDoc( AppleEvent theAppleEvent, AppleEvent reply,
						long refCon )
{
	;
}

pascal OSErr DoQuitApp( AppleEvent theAppleEvent, AppleEvent reply,
						long refCon )
{
	ExitToShell();
}
	
	
	