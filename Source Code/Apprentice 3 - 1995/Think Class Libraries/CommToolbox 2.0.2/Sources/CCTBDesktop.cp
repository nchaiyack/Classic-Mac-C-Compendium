/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°

	CCTBDesktop.cp
	
	CommToolbox compatible Desktop.

	SUPERCLASS = CDesktop / CFWDesktop.
	
	Original copyright © 1992-93 Romain Vignes. All rights reserved.
	Modifications copyright © 1994-95 Ithran Einhorn. All rights reserved.

°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

#include <CBartender.h>						/* TCL includes */
#include <Constants.h>
#include <CWindow.h>
#include <Global.h>

#include "CCTBApp.h"						/* Other includes */
#include "CCTBDesktop.h"
#include "CFileTransfer.h"
#include "CTermPane.h"

/* Application globals */

extern CApplication	*gApplication;
extern CBartender	*gBartender;
extern CBureaucrat	*gGopher;

#if USE_FLOATING_WINDOWS
TCL_DEFINE_CLASS_M1(CCTBDesktop,  CFWDesktop);
#else
TCL_DEFINE_CLASS_M1(CCTBDesktop,  CDesktop);
#endif

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * CCTBDesktop
 *
 * Initialization of the Desktop object
 *
 * aSupervisor:	desktop supervisor in the chain of command
 *
 */
 
CCTBDesktop::CCTBDesktop(CBureaucrat *aSupervisor) : CDesktop()
{
}

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */

/*
 * DispatchClick
 *
 * Mouse clicks dispatching
 *
 * macEvent: 	Pointer on the mouse event record
 *
 */
 	
void	CCTBDesktop::DispatchClick(
	register EventRecord	*macEvent)	// Mouse down event record
{
	short					thePart,theMenu,theItem;	// Location of mouse click
	WindowPeek				macWindow;					// Window where click occurred
	register CWindow		*theWindow;					// Corresponding window object
	register long			menuChoice;					// Selection from a menu

	thePart = FindWindow(macEvent->where, (WindowPtr*)&macWindow);

	if (macWindow != NULL && macWindow->windowKind == OBJ_WINDOW_KIND)	// TCL 1.1.3 11/30/92 BF
	{
		theWindow = (CWindow*) GetWRefCon((WindowPtr) macWindow);

		if (topWindow && theWindow != topWindow && topWindow->IsModal()
				&& thePart != inMenuBar)
		{
			SysBeep(3);
			return;
		}
	}
	else theWindow = NULL;

	switch (thePart)	// Take appropriate action
	{
		case inMenuBar:					// Pull-down menus
			Boolean toolMenu;
			
			gBartender->UpdateAllMenus();
			menuChoice = MenuSelect(macEvent->where);	/* Element selected */
			
			theMenu = HiShort(menuChoice);			
			theItem = LoShort(menuChoice);
			
			toolMenu = CFileTransfer::cTestToolMenu(theMenu,theItem);
			
			if (!toolMenu)
				toolMenu = CTermPane::cTestToolMenu(theMenu, theItem);
						
			if (theMenu && !toolMenu)
			{						
				gGopher->DoCommand(gBartender->FindCmdNumber(theMenu, theItem));
							/* Send the command to the gopher */
			}
			HiliteMenu(0);	
			break;

		default:
			inherited::DispatchClick(macEvent);
			return;
			break;
	}
}

/* °°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°°° */