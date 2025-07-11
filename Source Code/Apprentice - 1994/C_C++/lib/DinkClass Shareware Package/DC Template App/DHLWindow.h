/*
	This is the class declarations for the DWindow subclass for the
	DHLEvents demo.  This class is only responcible for the presenting
	of the results measuered in the HL and AE event bandwidth tests.
	Mark Gross 10/10/92
*/
#ifndef _DHLWINDOW__
#define _DHLWINDOW__

////#include <DWindow.h>
#include <DScrollWindow.h>


////class DHLWindow : public DWindow
class DHLWindow : public DScrollWindow
{

public:

	DHLWindow(void);
	~DHLWindow(void);
		// just stubs for now... (and perhaps always for this application.
	
	virtual Boolean Init(DDocument *doc, Boolean hasColorWindows);
		//just a simple init wich passes through to the
		// inherited method...for the moment.
		
	virtual void	Draw(Rect *r);
		// simply draws the data held in the Application
		// subclass after the computation is finished.

	//virtual Boolean KillMeNext(void);
		// as stub to indicate how to support closing a window
		// while alowing the user the opertunity to cancel out of
		// the close.  You can use it or the destructor to free 
		// space allocated.  Sometimes its better to use one over
		// the other depending on how you want your application
		// to behave as you delete objects.
		
}; // end of DHLWindow class declaration

#endif _DHLWINDOW__
