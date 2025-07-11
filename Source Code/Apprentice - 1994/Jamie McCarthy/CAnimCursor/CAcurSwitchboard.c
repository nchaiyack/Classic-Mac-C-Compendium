/*
 * CAcurSwitchboard.c
 * A useful supplement to CAnimCursor, when using the TCL.
 * Version 1.0b3, 13 May 1992
 *
 * To use, change your application's MakeSwitchboard() method to:
 *
 *		void CYourApp::MakeSwitchboard(void)
 *		{
 *			itsSwitchboard = new(CAcurSwitchboard);
 *			((CAcurSwitchboard*)itsSwitchboard)->IAcurSwitchboard();
 *		}
 *
 * If the switchboard you're presently using is not the standard
 * CSwitchboard, see the note about multiple inheritance in
 * CDesktop.c.
 *
 */



/********************************/

#include "CAcurSwitchboard.h"

/********************************/

#include <CWindow.h>

#include "CAnimCursor.h"

/********************************/



void CAcurSwitchboard::IAcurSwitchboard(void)
{
	inherited::ISwitchboard();
}



void CAcurSwitchboard::ProcessEvent(void)
{
	if (gAnimCursor != NULL) {
		
		Boolean shouldStopAnimating = FALSE;
		
		if (gAnimCursor->getMode() == kCACModeInterrupted) {
			
			shouldStopAnimating = TRUE;
			
		} else {
			
			WindowPeek theFrontWindow = (WindowPeek) FrontWindow();
			
			switch (theFrontWindow->windowKind) {
			
			case dialogKind: // a dialog or alert
				shouldStopAnimating = TRUE;
				break;
				
			case OBJ_WINDOW_KIND: { // a TCL window
				CWindow *theWindow;
				theWindow = (CWindow*) GetWRefCon((WindowPtr) theFrontWindow);
				if (theWindow->IsModal()) {
					shouldStopAnimating = TRUE;
				}
			}	break;
				
			default:
				break;
			}
			
		}
		
		if (shouldStopAnimating) {
			gAnimCursor->stopAnimating();
		}
	}
	
	inherited::ProcessEvent();
}



void CAcurSwitchboard::DispatchEvent(EventRecord *macEvent)
{
	if (gAnimCursor == NULL || !gAnimCursor->getIsAnimating()) {
		
		inherited::DispatchEvent(macEvent);
		
	} else {
		
		switch (macEvent->what) {
			case mouseUp:
			case mouseDown:
			case keyDown:
			case keyUp:
			case autoKey:
			case diskEvt:
			case osEvt:
				gAnimCursor->stopAnimating();
			default:
				inherited::DispatchEvent(macEvent);
				break;
		}
		
	}
}
