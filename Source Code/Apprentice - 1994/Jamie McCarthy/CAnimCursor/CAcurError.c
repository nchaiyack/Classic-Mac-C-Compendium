/*
 * CAcurError.c
 * A useful supplement to CAnimCursor, when using the TCL.
 * Version 1.0b3, 18 May 1992
 *
 * To use, change your application's MakeDesktop() method to:
 *
 *		void CYourApp::MakeError(void)
 *		{
 *			gError = new(CAcurError);
 *		}
 *
 */



/********************************/

#include "CAcurError.h"

/********************************/

#include "CAnimCursor.h"

/********************************/



void CAcurError::SevereMacError(OSErr macErr)
{
	if (gAnimCursor != NULL) {
		gAnimCursor->stopAnimating();
	}
	inherited::SevereMacError(macErr);
}



Boolean CAcurError::CheckOSError(OSErr macErr)
{
	if (gAnimCursor != NULL) {
		gAnimCursor->stopAnimating();
	}
	return inherited::CheckOSError(macErr);
}



void CAcurError::PostAlert(short STRid, short index)
{
	if (gAnimCursor != NULL) {
		gAnimCursor->stopAnimating();
	}
	inherited::PostAlert(STRid, index);
}



