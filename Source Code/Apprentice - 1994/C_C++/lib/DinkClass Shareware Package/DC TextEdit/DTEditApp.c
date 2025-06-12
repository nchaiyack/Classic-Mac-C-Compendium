/*
	File:		DTEditApp.c

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.
	Use at your own risk.

*/

//This is the class deffinition of the DScribbleApplication

#include "DTEditApp.h"
#include "DTEditDoc.h"



DTEditApp::DTEditApp(void)
{
	//Stub
}


DTEditApp::~DTEditApp()
{
	//Stub
}


DDocument* DTEditApp::MakeDDoc(Boolean OpenFromFile)
{
	DTEditDoc *New;
	
	New = new DTEditDoc;

			// if OpenFromFile == FALSE then it will 
			// not read from any file
	if(New->Init(OpenFromFile)) 
	{
		if ( fTarget = New->MakeWindow(HasColorQD()) )
			((DWindow *) fTarget)->SetWindowTitle();// everything went ok, so set the title
				// I'm having the App tell 
				// the doc to make the window because I don't 
				// like haveing one init function do too much stuff.  
				// Its a reliability thing. BTW FALSE is for no collorQD
		else
			fTarget = New; // window not created
	}
	else
	{
		fTarget = this;
		SelectWindow( FrontWindow());
	}
	return New;
}// end of MakeDDoc method


void DTEditApp::SetUpMenues(void)
{
	MenuHandle	menu;
	
	menu = GetMHandle(rFileMenu);
	EnableMenuItem( menu, iOpen, TRUE);
	
	inherited::SetUpMenues( );
	
}// end of SetUpMenus method



