/* Cell Proj 1.0 */

#include "Cell_Proto.h"
#include "Cell_Definitions.h"
#include "Cell_Variables.h"

Boolean gZoomed = false;

void HandleMouseDown(EventRecord *theEvent)
{
	WindowPtr	whichWindow;
	short		thePart;
	long		menuChoice;
	
	thePart = FindWindow( theEvent->where, &whichWindow );
	switch ( thePart ) {
		case inMenuBar:
			menuChoice = MenuSelect(theEvent->where);
			HandleMenuChoice( menuChoice );
			break;
		case inSysWindow:
			SystemClick( theEvent, whichWindow );
			break;
		case inDrag:
			// because I haven't figured out how to handle the dragging of the
			// zoomed state, I just won't allow it. -DaveP
			if (!gZoomed) {
				Rect dragRect = qd.screenBits.bounds;
				InsetRect(&dragRect,DRAG_THRESHOLD,DRAG_THRESHOLD);
				DragWindow( whichWindow, theEvent->where, &dragRect );
			}
			SetPort( whichWindow );
			break;
		case inGoAway:
			HideWindow( whichWindow );
			break;
		case inContent:
			SelectWindow( whichWindow );
			break;
		case inGrow:
			break;
		case inZoomIn:
		case inZoomOut:
			// clicking the zoom box now makes the window 2x its normal size.
			// handy for zooming in, no? -DaveP
			if (TrackBox(whichWindow,theEvent->where,thePart)) {
				// reset the counters so I can track speeds for different sizes -DaveP
				startTime = TickCount();
				generations = 0;
				// handle the zooming
				gZoomed = (thePart == inZoomOut);
				ZoomWindow(whichWindow, thePart, true);
				EraseRect(&(whichWindow->portRect));
			}
			break;
	}
}

void HandleMenuChoice(long menuChoice )
{
	short	theMenu;
	short	theItem;
	
	if ( menuChoice != 0 ) {
		theMenu = HiWord( menuChoice );
		theItem = LoWord( menuChoice );
		switch ( theMenu ) {
			case APPLE_MENU_ID:
				HandleAppleChoice( theItem );
				break;
			case FILE_MENU_ID:
				HandleFileChoice( theItem );
				break;
		}
		HiliteMenu( 0 );
	}
}

void HandleAppleChoice(short theItem )
{
	Str255		accName;
	short		accNumber;

	switch ( theItem ) {
		case ABOUT_ITEM:
			NoteAlert( ABOUT_ALERT, nil );
			break;
		default:
			GetItem( gAppleMenu, theItem, accName );
			accNumber = OpenDeskAcc( accName );
			break;
	}
}

void HandleFileChoice(short theItem )
{
	switch( theItem ) {
		case RESTART_ITEM:
			PlaceRandomCells();
			break;
		case QUIT_ITEM:
			gDone = TRUE;
			break;
	}
}
