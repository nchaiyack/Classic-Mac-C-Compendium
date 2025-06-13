/*
	This is the class declarationfor the the DDocument subclass
	DHLDoc, responcible for the only the creation of the correct type of
	Window.  Without this the DDocument class implementation would only
	instaniate a DWindow object and I wouldn't get the Draw method I
	need.
	Mark Gross 10/10/92
*/

#ifndef __DHLDOC__
#define __DHLDOC__

#include <DDocument.h>

class DHLDoc : public DDocument
{
public:

	DHLDoc(void);
	~DHLDoc(void);
		// just stubs for now... (and perhaps always for this application.
	
	virtual 	DDocument* Init( Boolean OpenFromFile);
		// needed just in case I have to over ride the
		// defult Init behavior, and its good OOP style 
		// to always have an Init.  Its better than using
		// constructors, just as SmallTalk OOPers.
	
	virtual DWindow* 	MakeWindow(Boolean hasColorWindows);
		// needed to creat my DHLWindow 

};// end of DHLDoc class declaration...

#endif __DHLDOC__
