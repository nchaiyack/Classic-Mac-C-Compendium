/*
	File:		DApplication.cp

	Contains:	xxx put contents here xxx

	Written by:	Mark Gross

	Copyright:	© 1992 by Applied Technical Software, all rights reserved.

	Change History (most recent first):

		 <6>	12/31/92	MTG		making the code conditionaly compiled so         that I am
									always working with a current         version in either think c
									or MPW C++
		 <5>	11/14/92	MTG		Bringing the C++ version up to date WRT the ThinkC version.
		 <4>	 9/20/92	MTG		Bringing the C++version up to date with the THINK C VERSION
		 <3>	  8/9/92	MTG		merging in changes from the ThinkC version
		 <2>	  8/8/92	MTG		fixing warnings

	To Do:
*/

// This is the Class definition of the DApplication class
//

#include "DApplication.h"
#include "DEventHandler.h"
#include "DDocument.h"


DApplication::DApplication(void)
{
	fIterator  = new DIterator;

}

DApplication::~DApplication(void)
{
	delete fIterator;
}



//Set up the Handler lists and set up the members to safe values

Boolean	DApplication::InitApp(void)
{
	Handle	menuBar;
	
	fEventHandlers = new DList;
	fDeadHandlers = new DList;
	fEventHandlers->Init();
	fDeadHandlers->Init();
	fSleepVal = SLEEPVAL;
	fDone = FALSE;

	fTarget = this;
	fClipData = NULL;
	GetClipFromSystem();
	fInBackground = false;
	fCursorRgn = NewRgn();
	
	menuBar = GetNewMBar(rMenuBarID);
	SetMenuBar(menuBar);
	DisposHandle(menuBar);
	AddResMenu(GetMHandle(rAppleMenu), 'DRVR');
	DrawMenuBar();

	return TRUE; 	// I know that it never will return fals
					// but, I think subclasses may need to have
					// the option of returning FALSE.
}// end of member function InitApp


// Add all the event handlers to the fDeadHandlers by telling them all to 
// kill them selves then Flush 'em!  This method smoked out a lot of side effect
// problems with the event handler maintenance implemented in DinkClass, Its been
// fixed.  If your interested look at the comments in the 
// DApplication::FlushDeadHandlers method.

Boolean DApplication::CleanUp(void)
{
	DEventHandler *nextHandler;
	DIterator *iterator;
		// I'm using an alocated iterator in this function because IT
		// calls functions which result in calls to functions wich use
		// the DApplications fIterator (which messes up its use in THIS
		// function.)
	
	iterator = new DIterator;
	iterator->Init(fEventHandlers);
	
	while( nextHandler = (DEventHandler *)iterator->GetCurrentThenIncrement() )
	{
		nextHandler->KillMeNext();
	}	

	FlushDeadHandlers();
	delete iterator;
		
	return fDone;// if user canceles out of closing a window then 
				// fDone is rest to FALSE within DDocument::WindowClosed.
		
}// end of CleanUp method...


//
// The following section of code is the deffintions of the fuctions
// used in support of the DDocument handling modle used in this class 
// library. 
//

DDocument* DApplication::MakeDDoc(Boolean OpenFromFile )
{
	DDocument *New;
	
	New = new(DDocument);
			// if OpenFromFile == FALSE then it will 
			// not read from any file
	if(New->Init(OpenFromFile)) 
	{
		if ( fTarget = New->MakeWindow(HasColorQD()) )
			((DWindow *) fTarget)->SetWindowTitle();// everything went ok, so set the title
				// I'm having the App tell 
				// the doc to make the window because I don't 
				// like haveing one init function do too much stuff.  
				// Its a reliability thing. BTW FALSE is for no colorQD
		else
			fTarget = New; // window not created
	}
	else
	{
		fTarget = this;
		SelectWindow( FrontWindow());
	}
	return New;
	
}//end member function MakeDDoc



//
// DApplication overides of some of the DEventHandler subclass methods
//

void DApplication::HandleMenuChoice(short menuID, short menuItem)
{
	short	itemHit;
	Str255	daName;
	short	daRefNum;

	if( menuID == rAppleMenu )
	{
		switch (menuItem)
		{
			case iAbout:
				itemHit = Alert(rAboutIDBox, NULL);
				break;
			default:
				GetItem(GetMHandle(rAppleMenu), menuItem, daName);
				daRefNum = OpenDeskAcc( daName);
				break;
		}
	}
				
	if( menuID == rFileMenu)
	{
		switch (menuItem)
		{
			case iNew:
				MakeDDoc(FALSE);
				break;
				
			case iOpen:
				MakeDDoc(TRUE);
				break;
				
			case iQuit:
				fDone = TRUE;
				break;
				
			default:
				break;
				
		}// end switch on menuItem
	}// end if fileMenu
	
	inherited::HandleMenuChoice( menuID, menuItem);
}
			

void DApplication::SetUpMenues(void)
{
	MenuHandle	menu;
	
	menu = GetMHandle(rFileMenu);

	EnableMenuItem( menu, iNew, TRUE);
	//EnableMenuItem( menu, iOpen, TRUE);
	EnableMenuItem( menu, iQuit, TRUE);
	
	inherited::SetUpMenues( );
	
}



// Return the number of types int the typeList and stuff the fMainFileType
// in the first item.  

int DApplication::GetFileType(OSType *typeList)
{
	*typeList = fMainFileType;
	return 1;
}


void DApplication::GiveClipToSystem(void)
{
	long	result, size;
	
	if(fClipData != NULL)
	{
		result = ZeroScrap();
		if(result != noErr)
			return;
		
		size = GetHandleSize(fClipData);
		HLock(fClipData);
		result = PutScrap(size, fClipType, *fClipData);
		HUnlock(fClipData);
	}	
}// end of GiveClipToSystem member fuction



void	DApplication::GetClipFromSystem(void)
{
	long	offset, result;
	Handle 	newData;
	
	newData = NewHandle(0); // Make an initial zero leangth handle
							// it will be sized by GetScrap as needed.

	result = GetScrap(newData, fClipType, &offset);
	if (result > 0)
	{
		if(fClipData != NULL)
			DisposHandle(fClipData);
		SetHandleSize(newData, result);
		fClipData = newData;
	}
}// end of GetClipFromSystem member function


Handle	DApplication::GetClipCopyFromApp(OSType *type)
{
	Handle tempHandle;
	OSErr err;
	
	*type = fClipType;	
	tempHandle = fClipData;
	err = HandToHand(&tempHandle);
	if(err != noErr)
		EnterMBStr("failed on the HandToHand copy");

	return tempHandle;
}

void	DApplication::GiveDataToApp(Handle data, OSType type)
{
	Handle tempHandle;
	OSErr err;

	fClipType = type;
	if (fClipData)
	{
		DisposHandle(fClipData);
	}
	tempHandle = data;
	err = HandToHand(&tempHandle);
	if(err != noErr)
		EnterMBStr("failed on the HandToHand copy");
	
	fClipData = tempHandle;
	
}



