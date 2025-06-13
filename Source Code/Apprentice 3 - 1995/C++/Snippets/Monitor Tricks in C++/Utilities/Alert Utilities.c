//
// Alert Utilities.c v1.0
//
// by Kenneth Worley
// Public Domain
//
// Contact me at:   America Online: KNEworley
// 			internet: KNEworley@aol.com or kworley@fascination.com
//
// Routines that make it easier to show a quick error or
// informational dialog.
//

#include "Alert Utilities.h"
#include <QDOffscreen.h>

void	DoAlert( short dialogID, Boolean playAlert )
{
	/*	This routine calls DoAlertStr with an empty error string. It is
	 *	expected that the dialog ID you specify is a dialog that already
	 *	has appropriate error text in it.
	 */
	
	DoAlertStr( dialogID, playAlert, "\p" );
}


void	DoAlertStrID( short dialogID, Boolean playAlert, short errStrID )
{
	/*	This routine loads the 'STR ' resource with ID errStrID and sends
	 *	the string and other parameters on to DoAlertStr.
	 */
	
	StringHandle		errString;
	
	errString = GetString( errStrID );
	HLock( (Handle)errString );
	DoAlertStr( dialogID, playAlert, *errString );
	HUnlock( (Handle)errString );
	ReleaseResource( (Handle)errString );
}

void	DoAlertStrIndexID( short dialogID, Boolean playAlert,
			short errStrID, short index )
{
	/*	This routine loads the string in the 'STR#' resource at the index
	 *	specified and passes the string and other parameters to the
	 *	DoAlertStr routine.
	 */
	
	Str255		errString;
	
	GetIndString( errString, errStrID, index );
	DoAlertStr( dialogID, playAlert, errString );
}

void	DoAlertStr( short dialogID, Boolean playAlert, Str255 errString )
{
	/*	This routine displays an alert whose resource ID number is specified
	 *	in the parameter dialogID.  The dialog specified is expected to have
	 *	at least one button (usually labeled "OK") which will be the default
	 *	button and have an ID if 1.  When the user presses this button, the
	 *	alert will be dismissed. The dialog should also have a static text
	 *	item with an ID of 2. If the string in errString is not zero length,
	 *	that string is displayed in the static text field. If the string
	 *	is zero length, nothing is done with the text in the dialog.
	 */

	CGrafPtr		currentPort;	/* store the current port here */
	GDHandle		currentDev;		// store current device here
	DialogPtr		theDlg;			/* to store our dialog in */
	
	short		itemType;		/* these 3 local variables are used to */
	Handle		itemHandle;		/* manipulate items in the dialog.     */
	Rect		itemRect;

	short		itemHit;		/* use with ModalDialog */

	// Save the current grafPort
	
		GetGWorld( &currentPort, &currentDev );
	
	// Now, load our dialog resource.
	
		theDlg = GetNewDialog( dialogID, NULL, (WindowPtr)-1L );
	
	// make the dialog the current port
	
		SetGWorld( (CGrafPtr)theDlg, NULL );
	
	// if the parameter specifies, do a system beep
	
		if ( playAlert )
			SysBeep( 3 );
	
	// If a string was sent, put that string in the dialog.
	
		if ( errString[0] )
		{
			GetDialogItem( theDlg, 2, &itemType, &itemHandle, &itemRect );
			SetDialogItemText( itemHandle, errString );
		}
	
	// now make sure the dialog is visible
	
		ShowWindow( theDlg );

	//	Get the OK button item and draw a bold border around it to show that
	//	it's the default button.

	 	GetDialogItem( theDlg, 1, &itemType, &itemHandle, &itemRect );
	 	InsetRect( &itemRect, -4, -4 );
	 	PenSize( 3, 3 );
	 	FrameRoundRect( &itemRect, 16, 16 );
	 	PenSize( 1, 1 );
	
	 //	Loop and call ModalDialog until the user presses the OK button
	 // This routine requires that whatever alert is shown, the button with
	 // id number kAlertOKButton is the one that should dismiss the dialog.
	 
	 	ModalDialog( NULL, &itemHit );
	 	while ( itemHit != 1 )
	 		ModalDialog( NULL, &itemHit );
	 	
	 /*	Now get rid of the dialog and set the port back to the control panel */
	 
	 	DisposDialog( theDlg );
	 	SetGWorld( currentPort, currentDev );
}

