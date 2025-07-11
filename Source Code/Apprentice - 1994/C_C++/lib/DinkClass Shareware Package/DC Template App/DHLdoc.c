/*
	This is the class deffinition for the DDocument subclass DHLDoc.
	It is responcible only for the creation of the DHLWindow objects
	needed for the display of the results of the tests implemented in
	the DApplication subclass for this program.
*/

#include "DHLDoc.h"
#include "DHLWindow.h"


DHLDoc::DHLDoc(void)
{
	//stub
}

DHLDoc::~DHLDoc(void)
{
	//stub
}


DDocument*	DHLDoc::Init( Boolean OpenFromFile)
{
	DDocument *inheritedDoc;
	
	inheritedDoc = inherited::Init(FALSE);
		// FALSE makes sure that no files will be opened
		// becuase this application realy has nothing to
		// do with saving or reading data off the disk.

 	return inheritedDoc;
}// end of Init method


DWindow*	DHLDoc::MakeWindow(Boolean hasColorWindows)
{	
	DHLWindow *newWindow;
	
	newWindow = new DHLWindow;
	fDWindow = newWindow;
		
	if (newWindow->Init(this, hasColorWindows))
		return newWindow;
	else
	{
		fDWindow = NULL;
		return fDWindow;
	}	
/*
	This method is a complete overried of the DDocument version of 
	MakeWindow.  It is declared vertual only for the dynamic binding
	of the instance to the method, it dosen't call inherited.
*/
}//end of MakeWindow method



