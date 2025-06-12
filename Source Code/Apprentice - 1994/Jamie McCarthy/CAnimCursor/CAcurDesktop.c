/*
 * CAcurDesktop.c
 * A useful supplement to CAnimCursor, when using the TCL.
 * Version 1.0b3, 13 May 1992
 *
 * To use, change your application's MakeDesktop() method to:
 *
 *		void CYourApp::MakeDesktop(void)
 *		{
 *			gDesktop = new(CAcurDesktop);
 *			((CAcurDesktop*)gDesktop)->IAcurDesktop(this);
 *		}
 *
 * If you want CAcurDesktop's superclass to be something other than
 * CDesktop (for example, CFWDesktop), well, here's where you
 * gnash your teeth and wish for multiple inheritance.  If Think C
 * had MI, the class hierarchy might look like:
 *
 * 	CDesktop			  \
 * 		MFWDesktop		\
 *			MAcurDesktop	 >->	CMyDesktop
 *			MWizzyDesktop	/
 *
 * As it is, the hierarchy might have to be:
 *
 *		CDesktop -> CFWDesktop -> CHackedUpWizzyDesktop -> CHackedUpAcurDesktop -> CMyDesktop
 *
 * The easiest way, if you can believe it, is to take your current
 * CMyDesktop.c and CMyDesktop.h files and change all occurences
 * of their superclass to "CAcurDesktop";  then, change every
 * reference to CDesktop in CAcurDesktop.h and CAcurDesktop.c to
 * whatever CMyDesktop's superclass used to be.  Don't forget
 * the IDesktop() call--change that too.  It's a pain in the butt,
 * I know.  Sorry.  Don't blame me.
 *
 */



/********************************/

#include "CAcurDesktop.h"

/********************************/

#include "CAnimCursor.h"

extern Boolean CrsrVis : 0x8CC;	// TRUE if cursor is visible

/********************************/



void CAcurDesktop::IAcurDesktop(CBureaucrat *aSupervisor)
{
	inherited::IDesktop(aSupervisor);
}



void CAcurDesktop::DispatchCursor(Point where, RgnHandle mouseRgn)
{
	switch (gAnimCursor->getMode()) {
		
		case kCACModeInterrupted:
			inherited::DispatchCursor(where, mouseRgn);
			break;
			
		case kCACModeContinuous:
			if (gAnimCursor == NULL || gAnimCursor->getIsAnimating()) {
				if (gInBackground || !CrsrVis) {
					return;
				}
				CView::cCurrHelpView = NULL;
				gAnimCursor->animateCursor();
			} else {
				inherited::DispatchCursor(where, mouseRgn);
			}
			break;
			
		default:
				/* What's goin' on? */
			break;
			
	}
}
