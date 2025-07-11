/*
	File:		DScribbleApp.h

	Written by:	Mark Gross

	Copyright:	� 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

// This is the Class declaration of theDScribbleApp subclass to DApplciaton

#ifndef __DSCRIBBLEAPP__
#define __DSCRIBBLEAPP__

#include "DApplication.h"

class DScribbleApp : public DApplication
{
public:

	DScribbleApp(void);
	~DScribbleApp(void);
		// stub con/de structors... here just for style

	virtual DDocument* MakeDDoc(Boolean OpenFromFile);
		// creats the application spacific DScribbleWind objects
		
	virtual void SetUpMenues(void);
		// enables the open menu item.  This may be a break in 
		// oop style, but I know that the DApplication HandleMenuChoice
		// will respond correctly (calls MakeDDoc)to an Open menu selection 
		// so I don't need to have a HandleMenuChoice method in this class.
};


#endif 

