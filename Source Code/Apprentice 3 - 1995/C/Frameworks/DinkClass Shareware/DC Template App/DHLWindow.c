/*
	This is the class implementation for the DWindow subclass
	DHLWindow, responcible for rendering the data on the screen
	Mark Gross 10/10/92
*/

#include "DHLWindow.h"



DHLWindow::DHLWindow(void)
{
	//stub
}

DHLWindow::~DHLWindow(void)
{
	//stub
}


Boolean DHLWindow::Init(DDocument *doc, Boolean hasColorWindows)
{
	Boolean inheritedSuccess;
	
	inheritedSuccess = inherited::Init(doc, hasColorWindows);
	if( inheritedSuccess)
	{
		;//do something....
	}

	return (inheritedSuccess);
	
}// end of Init method


void	DHLWindow::Draw(Rect *r)
{
	inherited::Draw(r);
		// just pass it through untill I
		// get the other classes implemented....
}// end of do Draw Method...

/*
//
// Use the following stubb as a template on how to support applications which
// allow the user to cancel out of a quit command.  This application doesn't 
// need this method (thats why its commented out) but its included here 
// to indicate how you support the "save cancel" feature of the Macintosh user interface.
// This same function could be used in your Document subclass, but I tend to not 
// use it there because its safer to use the Document's destructor.  As a rule of 
// thumb, choose KillMeNext over the destructor in cases where the data to be freeed is
// only referenced via the DWindow::fWindowPtr (which is deleted in DWinodw::KillMeNext).
//

Boolean DHLWindow::KillMeNext(void)
{
	Boolean inheritedSuccess = FALSE;

	if(fAlive)// only do stuff if obect is NOT in the DeadHandler list
	{
		if(inheritedSuccess = inherited::KillMeNext())// inherited returns FALSE if user cancels...
		{
			;// Kill your stuff here if your DWindow has things to
			//  get rid of
		}
	}
	return inheritedSuccess;
}// end of KillMeNext method...
*/

