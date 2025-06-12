/*
	File:		DEventHandler.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <4>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <3>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <2>	 9/20/92	MTG		Bringin this C++ file up to date with the THINK C version

	To Do:
*/

// This is the definition file for the DEventHandler base class
// for the DinkClass library.  It implements the flow of control
// functionality of the typical application.

// Derived classes must call these inherited::"Method"
// at the bottom of the derived functions of the same name

#include "DEventHandler.h"
#include "DApplication.h"

// Define and set the static members for the compiler.
// they will be reset in the main function.

DApplication*	DEventHandler::gApplication = NULL;
Boolean		DEventHandler::gPassItOn = TRUE;


// The DEventHandler constructor supports the list
// mainteance scheam by installing itself into the 
// gApplication::fEventHandlers list.	

DEventHandler::DEventHandler(void)
{
	fNextHandler = NULL;
	fAlive = TRUE;
	if(gApplication)// the gApplication is never kept in the list of Handlers...
		gApplication->InstalHandler(this);
}

DEventHandler::~DEventHandler(void)
{

}

	
Boolean DEventHandler::KillMeNext(void)
{
	Boolean Success;

	if(fAlive) 
	{
		Success = gApplication->RemoveHandler( this);
		fAlive = FALSE; 	// avoid multiple calls to remove handler and other possible sins. 
		return Success;
	}
	return FALSE;
}



void DEventHandler:: HandleNullEvent(EventRecord *theEvent)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleNullEvent(theEvent);
}


void DEventHandler:: HandleActivateEvt(EventRecord *theEvent)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleActivateEvt(theEvent);
}


void DEventHandler:: HandleAutoKey(EventRecord *theEvent)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleAutoKey(theEvent);
}


void DEventHandler:: HandleKeyDown(EventRecord *theEvent)				
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleKeyDown(theEvent);
}


void DEventHandler:: HandleDiskEvt(EventRecord *theEvent)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleDiskEvt(theEvent);
}


void DEventHandler:: HandleHighLevelEvent(EventRecord *theEvent)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleHighLevelEvent(theEvent);
}


void DEventHandler:: HandleOSEvent(EventRecord *theEvent)
{

	if (gPassItOn && fNextHandler)
		fNextHandler->HandleOSEvent(theEvent);
}



void DEventHandler:: HandleUpdateEvt(EventRecord *theEvent)
{

	if (gPassItOn && fNextHandler)
		fNextHandler->HandleUpdateEvt(theEvent);
}
	

void DEventHandler::HandleMouseDown(EventRecord *theEvent, short thePart, WindowPtr theWindow)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleMouseDown(theEvent, thePart, theWindow);
}

void DEventHandler::HandleMenuChoice(short menuID, short menuItem)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->HandleMenuChoice(menuID, menuItem);
}


void	DEventHandler::EnableMenuItem(MenuHandle menu, short item, Boolean enable)
{

	if (enable) 
		EnableItem(menu, item);
	else
		DisableItem(menu, item);
	
}// 
	
void DEventHandler::SetUpMenues(void)
{
	if (gPassItOn && fNextHandler)
		fNextHandler->SetUpMenues();
}
