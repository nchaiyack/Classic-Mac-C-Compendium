/********************************************************/
/*														*/
/*  EventTrigger Code from Chapter Four of 				*/
/*														*/
/*	** The Macintosh C Programming Primer, 2nd Ed. **	*/
/*  													*/
/*	Copyright 1992, Dave Mark and Cartwright Reed   	*/
/*														*/
/*  This program demonstrates specific Mac programming	*/
/*	techniques.											*/
/*														*/
/********************************************************/

#include <AppleEvents.h>
#include <GestaltEqu.h>

#define kGestaltMask	1L

/***************/
/*  Functions  */
/***************/

void	ToolBoxInit( void );
void	EventsInit( void );
void	SendEvent( AEEventID theAEEventID );


/******************************** main *********/

void	main( void )
{
	ToolBoxInit();
	EventsInit();
	
	SendEvent( kAEOpenApplication );
	SendEvent( kAEOpenDocuments );
	SendEvent( kAEPrintDocuments );
	SendEvent( kAEQuitApplication );
}


/*********************************** ToolBoxInit */

void	ToolBoxInit( void )
{
	InitGraf( &qd.thePort );
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( 0L );
	InitCursor();
}


/*********************************** EventsInit */

void	EventsInit( void )
{
	long	feature;
	OSErr	err;
	
	err = Gestalt( gestaltAppleEventsAttr, &feature );
	
	if ( err != noErr )
	{
		SysBeep( 10 );	/*  Error calling Gestalt!!!  */
		ExitToShell();
	}
	
	if ( !( feature & ( kGestaltMask << gestaltAppleEventsPresent ) ) )
	{
		SysBeep( 10 );	/*  AppleEvents not supported!!!  */
		ExitToShell();
	}
	
}


/******************************** SendEvent *********/

void	SendEvent( AEEventID theAEEventID )
{
	OSErr			err;
	AEAddressDesc	address;
	OSType			appSig;
	AppleEvent		appleEvent, reply;
	
	appSig = 'Prmr';
	
	err = AECreateDesc( typeApplSignature, (Ptr)(&appSig),
				(Size)sizeof( appSig ), &address );
	
	err = AECreateAppleEvent( kCoreEventClass, theAEEventID, &address,
			kAutoGenerateReturnID, 1L, &appleEvent );
			
	err = AESend( &appleEvent, &reply, kAENoReply + kAECanInteract,
			kAENormalPriority, kAEDefaultTimeout, nil, nil );
}