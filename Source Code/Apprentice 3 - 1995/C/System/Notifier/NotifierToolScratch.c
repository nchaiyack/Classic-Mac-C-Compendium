/**********************************************************************************
NotifierToolScratch.c

	This is a small code resource that implements a self-disposing Notification
	Manager Alert.  Its purpose is to be called from an extension that has 
	decided not to install itself but which wants to report the reason to
	the user by way of a Notification Manager alert.
	
	The extension needs to load this code resource into memory and pass the address
	of a Pascal String to the code resource when jumping to it.  Notifier
	makes a copy of the string so it may be disposed of after Notifier returns.
	
	Do not make this code resource of type 'INIT',  make it of type 'Code' or
	something else.  It must be marked System Heap and Locked.

**********************************************************************************/

#include <SetupA4.h>

#define ToolScratch ( (long *) 0x09CE )

/*  Prototypes  */
pascal void main( StringPtr theString );
Boolean InstallUs( StringPtr aString );
pascal void RemoveResponse( NMRecPtr thePtr );

pascal void			SetA0( void* ) = { 0x205F };
pascal void			SetA1( void* ) = { 0x225F };
void*				GetA0( void ) = { 0x2008 };
void*				GetA7( void ) = { 0x200F };

/*  Global Variables  */
Handle				ourHand;
NMRec				theRec;
unsigned long		*retAddressPtr;
Str255				msgString;

/**********************************************************************************
main

	Called from the extension.  It tries to install the Notification request.
	If it's successful it detaches itself.

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

	Sets up the Notification record.  Makes a copy of the String and
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

	Removes the NM request.  Moves a couple of instructions to ToolScratch.
	Sets up the registers so those instructions will dispose this handle.  And
	jumps to ToolScratch. The only thing that could screw up this process is
	if some interrupt time code were to mess with ToolScratch between the time we
	poke our instructions in there and the time we jump there.

**********************************************************************************/

#define kRetOffset 8L

pascal void 
RemoveResponse( NMRecPtr thePtr )
{
	SetUpA4();

	//Uninstall the request
	(void) NMRemove( thePtr );

	//Move the dispose code to ToolScratch
	*ToolScratch = 0xA0234ED1;	//DisposeHandle, jmp (A1)
	FlushInstructionCache();	//Flush the cache
	
	//Get the current return address off the stack
	retAddressPtr = (void *) ( (unsigned long) GetA7() + kRetOffset );
	
	//Save the return address in A1
	SetA1( (void *) *retAddressPtr );
	
	//Change the return address to point to ToolScratch
	*retAddressPtr = (unsigned long) ToolScratch;

	//Save ourHand in A0 so DisposeHandle can find it
	SetA0( ourHand );

	RestoreA4();
	
	//We exit by RTSing to ToolScratch where the DisposeHandle disposes
	//of this handle and then we jmp to the return address saved in A1

}