/*
	File:		DTEditApp.h

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

// This is the class declaration for the DTEditApp subclass to 
// DApp

#ifndef __DTEDITAPP__
#define __DTEDITAPP__

#include"DApplication.h"

class DTEditApp : public DApplication
{
public:

	DTEditApp(void);
	~DTEditApp(void);
		// stub con/de structors... here just for style

	virtual DDocument* MakeDDoc(Boolean OpenFromFile);
		// creats the application spacific DTEditDoc objects

	virtual void SetUpMenues(void);
		// enables the open menu item.  This may be a break in 
		// oop style, but I know that the DApplication HandleMenuChoice
		// will respond correctly (calls MakeDDoc)to an Open menu selection 
		// so I don't need to have a HandleMenuChoice method in this class.
};


#endif 

	