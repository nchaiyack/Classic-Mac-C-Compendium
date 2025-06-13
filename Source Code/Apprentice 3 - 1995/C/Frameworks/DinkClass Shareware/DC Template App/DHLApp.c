/*
	This is the class definition file for the DHLApp subclass of DApplication.
	This class is needed to creat the correct DDocument subclass and handle
	the menu options
	Mark Gross 10/10/92
*/

#include "DHLApp.h"
#include "DHLDoc.h"


DHLApp::DHLApp(void)
{
	// stubb
}

DHLApp::~DHLApp(void)
{
	// stubb
}


DDocument* DHLApp::MakeDDoc(Boolean OpenFromFile)
{
	DHLDoc *New;
	
	New = new DHLDoc;

			// if OpenFromFile == FALSE then it will 
			// not read from any file
	if(New->Init(OpenFromFile)) 
	{
		if ( fTarget = New->MakeWindow(HasColorQD()) )
			((DWindow *) fTarget)->SetWindowTitle();// everything went ok, so set the title
				// I'm having the App tell 
				// the doc to make the window because I don't 
				// like haveing one init function do too much stuff.  
				// Its a reliability thing. 
		else
			fTarget = New; // window not created
	}
	else
	{
		fTarget = this;
		SelectWindow( FrontWindow());
	}
	return New;
}// end of MakeDDoc member function


void DHLApp::HandleMenuChoice(short menuID, short menuItem)
 {
/*
PUT IN MY TEST methods when I get them implemented!!!!!!
 	if(menuID == rPenMenu)
	{
		switch(menuItem)
		{
			case i???:
				DoTesti???();
				break;
			case i????:
				DoTesti????();
				break;
			default:
				break;
		}//end switch menuitem
	}// end if rPenMenu
*/		
	inherited::HandleMenuChoice(menuID, menuItem);
	
}// end of HandleMenuChoice member fuction
			

void DHLApp::SetUpMenues(void)
 {
	MenuHandle	menu;
	
/*
	menu = GetMHandle(rPenMenu);

	EnableMenuItem( menu, i1X1, TRUE);
	EnableMenuItem( menu, iWhite, TRUE);

	CheckItem(menu,i1X1, ((DScribbleWind *)fDWindow)->fPenSize == 1);
	CheckItem(menu, iWhite, ((DScribbleWind *)fDWindow)->fPenPat == patWhite);
*/
	
	inherited::SetUpMenues();
}// end of SetUpMenues function


void DHLApp::ClearMenus(void)
{
	MenuHandle	menu;
	int menuItem;
	
/*
	menu = GetMHandle(rCommMenu);
	for (menuItem=1; menuItem<=CountMItems(menu); menuItem++)
	{
		EnableMenuItem( menu, menuItem, FALSE);
	}
*/
	inherited::ClearMenus();
	
}// end of clear menus member function...
