#include "SimpleWindowClass.h"
#include "LinkedLists.h"

#define rMenuBar 128
#define rWindow 128
#define mApple 128
#define mFile 129
#define rAboutDialog 128
#define	iAbout 1
#define iNewWindow 1
#define	iCloseWindow 2
#define iQuit 4

void	InitMac();
void	DoCommand(long	theCommand);
void	DoMouseDown(EventRecord *theEvent);
void	EventLoop();


/*
	Create a ComparisonList List with a SimpleWindowClass class and a WindowPtr to use
	for comparisons
	
	tempWindow is used throughout the program to create a new window
	quit is used to know when to quit the program
	windowNumber is the number of the window created
*/


ComparisonList<SimpleWindowClass, WindowPtr> myWindowList;
SimpleWindowClass	*tempWindow;
short				windowNumber=0;
Boolean				quit=0;

void main()
{
	extern	ComparisonList<SimpleWindowClass, WindowPtr> myWindowList;
	extern	SimpleWindowClass	*tempWindow;
	extern	Boolean	quit;
	extern	short	windowNumber;
	
	
	InitMac();		//	Do standard Mac Initializations, plus set up menu bar
	
	
	//	Create new window, then push it on the list
	
	windowNumber++;
	tempWindow=new SimpleWindowClass();
	tempWindow->CreateWindow(windowNumber, rWindow);
	myWindowList.Push(*tempWindow);
	
	
	//	Enter Event Loop
	while (!quit)
		EventLoop();
	
	
	//	When quitting, delete all entries in list.  This is not actually necessary, because
	//	the destructor in the ComparisonList will call the function, too
		
	myWindowList.DeleteAll();

}



void	InitMac()
{
	InitGraf(&thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
	
	
	//	Get menu bar, add apple menu items, draw menu bar
	
	Handle	hmenuBar;
	MenuHandle hmenu;
	
	hmenuBar=GetNewMBar(rMenuBar);
	SetMenuBar(hmenuBar);
	DisposeHandle(hmenuBar);
	AppendResMenu(GetMenuHandle(mApple), 'DRVR');
	DrawMenuBar();

}

void	EventLoop()
{
	EventRecord	theEvent;
	
	//	This event loop only processes keyDown, mouseDown, and update and activate events
	//	No time is needed for background processing, (i.e., null events), so sleep time is
	//	set to 0xFFFFFFFF
	
	if (WaitNextEvent(everyEvent, &theEvent, 300, nil)) {
	
		switch (theEvent.what) {
			case keyDown:
				if (theEvent.modifiers & cmdKey) {
					DoCommand(MenuKey(theEvent.message));
					HiliteMenu(0);
				}
			break;
			
			case mouseDown:
				DoMouseDown(&theEvent);
			break;
			
	/*****
		The update and activate events are a good example of how ComparisonList's XInList
		function is used.  Passed a WindowPtr and a pointer to a SimpleWindowClass pointer, it
		will return whether the WindowPtr is in the list and a pointer to the object containing
		the WindowPtr in the second paramater.
	*****/
			case updateEvt:
			case activateEvt:
			
				SetCursor(&arrow);
			
				SimpleWindowClass	*thisObject;
				
				if (myWindowList.XInList((WindowPtr) theEvent.message, thisObject)) {
					if (updateEvt)
						thisObject->UpdateWindow();
					else
						thisObject->ActivateWindow(theEvent.modifiers & activeFlag);
				}
			break;
			
			
		}
	}
}

void	DoCommand(long theCommand)
{
	short	menu=HiWord(theCommand), item=LoWord(theCommand);
	extern	short windowNumber;
	extern	Boolean quit;
	extern	SimpleWindowClass *tempWindow;
	
	HiliteMenu(0);
	
	switch (menu) {
		case mApple:
			if (item==iAbout) {
				DialogPtr	aboutDialog;
				
				aboutDialog=(DialogPtr) NewPtr(sizeof (DialogRecord));
				aboutDialog=GetNewDialog(rAboutDialog, aboutDialog, (WindowPtr) -1L);
				ModalDialog(nil, nil);
				CloseDialog(aboutDialog);
				DisposePtr((Ptr) aboutDialog);
			}
			
			else {
				Str255 sName;
				GetItem(GetMenu(mApple), item, sName);
				OpenDeskAcc(sName);
			}
		break;
		
		case mFile:
			switch (item) {
				case iNewWindow:
					windowNumber++;
					tempWindow=new SimpleWindowClass();
					tempWindow->CreateWindow(windowNumber, rWindow);
					myWindowList.Push(*tempWindow);
				break;
				
				case iCloseWindow:
					WindowPtr	theWindow=FrontWindow();
					myWindowList.XInList(theWindow, tempWindow);
					tempWindow->Hide();
				break;
				
				case iQuit:
					quit=1;
				break;
			}
		break;
	}
}

void	DoMouseDown(EventRecord *theEvent)
{
	WindowPtr	theWindow;
	short		windowPart;
	extern		ComparisonList<SimpleWindowClass, WindowPtr> myWindowList;
	
	windowPart=FindWindow(theEvent->where, &theWindow);
	
	
	/*****
		After finding which window is involved in the mouseDown event, retrieve the object
		containing the WindowPtr and call the proper function.  To make things simple, and
		because this ListClass doesn't have a delete function for a specific object, we'll
		just hide the window if the user closes it.
	*****/
	
	myWindowList.XInList(theWindow, tempWindow);
	
	
	switch (windowPart) {
		case inGoAway:
			if (TrackGoAway(theWindow, theEvent->where))
				tempWindow->Hide();
		break;
		
		case inDrag:
			tempWindow->Drag(theEvent->where);
		break;
		
		case inContent:
			if (FrontWindow()!=theWindow)
				SelectWindow(theWindow);
		break;
		
		case inMenuBar:
			DoCommand(MenuSelect(theEvent->where));
		break;
	}
}