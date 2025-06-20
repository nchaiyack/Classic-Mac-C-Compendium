/*
	This is the class declaration for the DHLApp subclass of DApplication.
	It is responcible for creating the correct document subclass, holding 
	the test results and doing the tests whose results get held.
	Mark Gross 10/10/92
*/

#ifndef __DHLAPP__
#define __DHLAPP__

#include <DApplication.h>

class DHLApp : public DApplication
{
public:

	DHLApp(void);
	~DHLApp(void);
		// just stubs for now... (and perhaps always for this application.

	virtual DDocument* MakeDDoc(Boolean OpenFromFile);
		// needed to instaniate the correct document subclass for
		// this application.

	virtual void HandleMenuChoice(short menuID, short menuItem);
		// needed to fire off the tests in responce to the users
		// menu selections...
	virtual void SetUpMenues(void);
		// needed for the design concept, if a test can't be run 
		// becuase something isn't set up (like PPC addresses or 
		// something) then I don't want the menu to be enabled
	
protected:
	
	virtual void ClearMenus(void);
		// needed to simplify the logic in SetUpMenus
		// if this guy clears the DHLEvent spacific items, then
		// SetUpMenus only needs to turn ON items, as aposed to
		// turning the OFF or ON as needed.

};//end of DHLApp class declaration

#endif __DHLAPP__
