/**********************************************************************************
TestNotifierINIT.c

	Simple extension that demonstrates the Notifier self-disposing Notification
	alert code resource.

**********************************************************************************/

#include <SetupA4.h>

//Prototypes
void main( void );
void CallNotifier( void );

/**********************************************************************************
main

	All this does is call the routine to install the Notifier.  Note that this
	test extension doesn't detach itself.  This is what most extensions would
	do if they encounter a fatal error at INIT time.

**********************************************************************************/

void 
main(void)
{
	RememberA0();
	SetUpA4();

	CallNotifier();
	
	RestoreA4();

}

/**********************************************************************************
CallNotifier

	This routine reads in the Notifier Code resource.  It then jumps to the 
	resource, passing the address of a Pascal string.  That's it.  The Notifier
	code resource does the rest.

**********************************************************************************/

void
CallNotifier( void )
{
	Str255		theString = "\pThis is the error string from the Test Notifier INIT.";
	
	Handle		NotifierResource;
	
	pascal void (*Notifier)( StringPtr );


	NotifierResource = GetResource( 'Code', 9898 );
	if ( NotifierResource )
	{
		Notifier = ( pascal void (*) ( StringPtr ) )
					StripAddress( *NotifierResource );

		Notifier( theString );
	}

}