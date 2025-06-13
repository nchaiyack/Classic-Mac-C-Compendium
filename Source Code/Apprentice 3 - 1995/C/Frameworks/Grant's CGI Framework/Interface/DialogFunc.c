/*****
 *
 *	DialogFunc.c
 *
 *	This is a support file for "Grant's CGI Framework".
 *	Please see the license agreement that accompanies the distribution package
 *	for licensing details.
 *
 *	Copyright ©1995 by Grant Neufeld
 *	grant@acm.com
 *	http://arpp1.carleton.ca/grant/
 *
 *****/

#include "constants.h"

#include "Events.h"
#include "WindowInt.h"

#include "DialogFunc.h"


/***  FUNCTIONS  ***/

/* This function responds to update, and activate events, and key presses.
	return, enter, Command-. and esc select the 'OK' button. (No Cancel button) */
pascal Boolean
defaultAlert1ButtonEventFilter ( DialogPtr theDialog, EventRecord *theEvent, short *itemHit)
{
	Boolean		result;
	char		key;
	short		itemType;
	Handle		itemHandle;
	Rect		itemRect;
	long		finalTicks;
	
	result = false;
	
	switch ( theEvent->what )
	{
		case updateEvt :
			if ( (WindowPtr)theEvent->message != theDialog )
			{
				doUpdateEvent ( theEvent );
			}
			break;
		
		case activateEvt :
			if ( (WindowPtr)theEvent->message != theDialog )
				WindowActivate ( (WindowPtr)theEvent->message,
					( (theEvent->modifiers & activeFlag) != 0 ), theEvent );
			break;
		
		case keyDown :
		case autoKey :
			key = (char)( theEvent->message & charCodeMask );
			
			if ( (key == (char)kReturnKey) || (key == (char)kEnterKey) ||
				(key == (char)kEscapeKey) ||
				((key == (char)kPeriodKey) && (theEvent->modifiers & cmdKey)) )
			{
				GetDialogItem	( theDialog, kDefaultButtonID, &itemType, &itemHandle, &itemRect );
				HiliteControl	( (ControlHandle)itemHandle, nil );
				Delay			( (long)kVisualDelay, &finalTicks );
				HiliteControl	( (ControlHandle)itemHandle, nil );
				
				result		= true;
				*itemHit	= (short)kDefaultButtonID;
			}
			break;
	}
	
	return result;
} /* defaultAlert1ButtonEventFilter */


/*****  EOF  *****/
