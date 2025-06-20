/**********************************************************************************
NotifierUnlock.c

	This is a small code resource that implements a self-disposing Notification
	Manager Alert.  Its purpose is to be called from an extension that has 
	decided not to install itself but which wants to report the reason to
	the user by way of a Notification Manager alert.
	
	The extension needs to load this code resource into memory and pass the address
	of a Pascal String to the code resource when jumping to it.  Notifier
	makes a copy of the string so it may be disposed of after Notifier returns.
	
	Do not make this code resource of type 'INIT',  make it of type 'Code' or
	something else.  It must be marked System Heap and Locked.
	
	This code was written for Think C 7.

**********************************************************************************/

#include <SetupA4.h>

/*  Prototypes  */
pascal void main( StringPtr theString );
Boolean InstallUs( StringPtr aString );
pascal void RemoveResponse( NMRecPtr thePtr );

pascal void*		GetA0( void ) = { 0x2E88 };

/*  Global Variables  */
Handle				ourHand;
NMRec				theRec;
Str255				msgString;

/**********************************************************************************
main

	Called from the extension.  This function tries to install the Notification 
	request.  If successful it detaches itself.

**********************************************************************************/

pascal void 
main( StringPtr theString )
{
	Ptr		pToUs = GetA0();

	RememberA0();
	
	SetUpA4();

	if ( InstallUs( theString ) )
	{
		ourHand = RecoverHandle( pToUs );
		DetachResource( ourHand );
	}

	RestoreA4();

}

/**********************************************************************************
InstallUs

	Set up the Notification record.  Makes a copy of the String and
	installs the NM request. 

**********************************************************************************/

Boolean
InstallUs( StringPtr aString )
{	
	//Set up the notification record
	theRec.qType = nmType;
	theRec.nmMark = 0;
	theRec.nmIcon = NULL;
	theRec.nmSound = NULL;
	theRec.nmStr = msgString;
	theRec.nmResp = (void *) RemoveResponse;
	theRec.nmRefCon = 0;
	
	//Make a local copy of the string
	BlockMove( aString, msgString, aString[0] + 1 );

	if ( noErr == NMInstall( &theRec ) )
		return TRUE;
	else
		return FALSE;

}

/**********************************************************************************
RemoveResponse

	Removes the NM request.  Works by unlocking us and marking us as purgeable.
	When space is required in the system heap then this block will be purged.
	All that will remain will be a single used handle entry in the system heap.
	This is not much.

**********************************************************************************/

pascal void 
RemoveResponse( NMRecPtr thePtr )
{
	SetUpA4();

	//Uninstall the request
	(void) NMRemove( thePtr );
	
	HPurge( ourHand );
	HUnlock( ourHand );

	RestoreA4();

}