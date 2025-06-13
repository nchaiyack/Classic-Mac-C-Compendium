/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CCTBSwitchboard.cp
	
	CommToolbox compatible switchboard.
	
	SUPERCLASS = CSwitchboard.
	
	Original copyright © 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright © 1994-95 Ithran Einhorn. All rights reserved.
	
°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
#include <CApplication.h>
#include <CDesktop.h>
#include <Constants.h>
#include <Global.h>
*/

#include "CCTBApp.h"							/* Other includes */
#include "CCTBSwitchboard.h"
#include "CFileTransfer.h"
#include "CTermPane.h"

/* Application globals */

extern CApplication	*gApplication;
extern CDesktop		*gDesktop;

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * CCTBSwitchboard
 *
 * Switchboard object initialisation
 *
 */
 
CCTBSwitchboard::CCTBSwitchboard() : CSwitchboard()		/* Initialize superclass */
{
}

/******************************************************************************
 DispatchEvent

	Dispatch the event to the appropriate handler.
 ******************************************************************************/

void	CCTBSwitchboard::DispatchEvent(
	EventRecord		*macEvent)
{
	WindowPtr		theWindow;
	Boolean			toolEvent;
	
	toolEvent = FALSE;
	theWindow = NULL;
	
	switch (macEvent->what)
	{				
		case mouseDown:
			FindWindow(macEvent->where,&theWindow);
			break;
			
		case activateEvt:
		case updateEvt:
			theWindow = (WindowPtr) macEvent->message;
			break;
	}
	
	if (theWindow != NULL)
	{
		toolEvent = CFileTransfer::cTestToolEvent(macEvent,theWindow);
		
		if (!toolEvent)
			toolEvent = CConnection::cTestToolEvent(macEvent,theWindow);
			
		if (!toolEvent)
			toolEvent = CTermPane::cTestToolEvent(macEvent,theWindow);
	}
	
	if (!toolEvent)
		inherited::DispatchEvent(macEvent);	/* Send the evt to its superclass */
}

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */