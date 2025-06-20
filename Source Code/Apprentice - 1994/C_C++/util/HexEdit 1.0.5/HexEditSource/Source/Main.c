/************************************************************************************
 * Main.c
 *
 * from HexEdit, a simple hex editor
 * copyright 1993, Jim Bumgardner
 *
 * Revision History is in History.note
 ************************************************************************************/
#include "HexEdit.h"
#include "AppleEvents.h"
#include <Traps.h>


void 	MyInitMacintosh(void);
void	MyInitMultifinder(void);
void 	InitAppleEvents(void);
void	CheckEnvironment(void);
void 	MyHandleEvent(void);
void 	MyDoEvent(EventRecord *theEvent);
void	IdleObjects(EventRecord *er);

Boolean	gWNEImplemented,gQuitFlag,gSys7Flag,gColorQDFlag;
// Typical Macintosh Initialization Code


// Main Entry Point

main()
{
	// Standard Mac Initialization
	MyInitMacintosh();

	// Check if Multifinder (WaitNextEvent) is implemented
	MyInitMultifinder();

	// Check if System 7
	CheckEnvironment();

	// Init Apple Events
	InitAppleEvents();

	// Set up the menu bar
	MySetUpMenus();

	InitializeEditor();

	if (!gSys7Flag)
		AskEditWindow();

	// main event loop
	while (!gQuitFlag)			// Till the End of Time...
		MyHandleEvent();		// Get an Event, do something about it

	CleanupEditor();
}

// Standard Macintosh Initialization

void MyInitMacintosh(void)
{
	MaxApplZone();
	
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

// Check if WaitNextEvent (Multifinder) is implemented on this Macintosh

void MyInitMultifinder(void)
{
	gWNEImplemented = (NGetTrapAddress(_WaitNextEvent, ToolTrap) != 
					   NGetTrapAddress(_Unimplemented,ToolTrap));
}

// The Main Event Dispatcher - this routine should be called repeatedly

void MyHandleEvent(void)

{
	EventRecord	theEvent;
	Boolean		ok;

	// If the more modern WaitNextEvent is implemented, use it 

	if (gWNEImplemented)
		// We don't have to call SystemTask because WaitNextEvent calls it for us
		// Get the next event
		ok = WaitNextEvent(everyEvent,&theEvent,0L,NULL);

	else {
		// we are running in (Single) Finder under system 6 or less
		// Give Desk Accessories some processing time
		SystemTask ();

		// Get the next event
		ok = GetNextEvent (everyEvent, &theEvent);
	}

	if (IsDialogEvent(&theEvent)) {
		DoModelessDialogEvent(&theEvent);
	}
	else if (ok) {
			// Handle the Event
			MyDoEvent(&theEvent);
	}
	else {
		// Nothing happened, kick back...
		IdleObjects(&theEvent);
	}
}


void MyDoEvent(EventRecord *theEvent)
{
	short 		windowCode;
	WindowPtr	theWindow;


	switch (theEvent->what) {
	//
	// Was the mouse button pressed?
	case mouseDown:
		// Find out where the mouse went down
		windowCode = FindWindow (theEvent->where, &theWindow);

	  	switch (windowCode) {
		case inSysWindow: 	// Desk Accessory?
		    SystemClick (theEvent, theWindow);
		    break;
		    
		case inMenuBar:		// Menu Bar?
		  	MyAdjustMenus();
		    MyHandleMenu(MenuSelect(theEvent->where));
		    break;

		default:			// Cursor was inside our window
			// If the window isn't in the front
			if (theWindow != FrontWindow()) {
				// Make it so...
				SelectWindow(theWindow);
				MyAdjustMenus();
			}
			else {
				// Window is already in the front, handle the click
				switch (windowCode) {

				case inContent:		// Content area?
					if (((WindowPeek) theWindow)->refCon == MyWindowID)
						((ObjectWindowPtr) theWindow)->HandleClick(theWindow, theEvent->where, theEvent);
					break;

				case inDrag:		// Dragbar?
					{
						Rect	dragRect;
						dragRect = screenBits.bounds;
						// Handle the dragging of the window
						DragWindow(theWindow, theEvent->where, &dragRect);
						if (!((ObjectWindowPtr) theWindow)->floating)
							SelectWindow(theWindow);
					}
					break;

				 case inGoAway:						// close box?
			  		if (TrackGoAway(theWindow, theEvent->where)) {
						// If mouse is released inside the close box
						// Hide or close the window
					  	if (((WindowPeek) theWindow)->refCon == MyWindowID)
							CloseEditWindow(theWindow);
						else if (theWindow == gSearchWin) {
							DisposDialog(gSearchWin);
							gSearchWin = NULL;
						}
						MyAdjustMenus();
					}
				  	break;

				case inGrow:						// Grow box?
					{
						long	growResult;
						Rect	growRect;

						SelectWindow(theWindow);

						SetRect(&growRect,MaxWindowWidth+SBarSize-1,64,
								MaxWindowWidth+SBarSize-1,gMaxHeight);

						// Handle the mouse tracking for the resizing
						growResult = GrowWindow(theWindow,theEvent->where,&growRect);

						// Change the size of the window
						SizeWindow(theWindow,LoWord(growResult),HiWord(growResult),true);

						AdjustScrollBars(theWindow, true);
						DrawPage((EditWindowPtr) theWindow);

						// Redraw the window
						SetPort(theWindow);
					  	InvalRect(&theWindow->portRect);
					}
					break;
				case inZoomIn:
				case inZoomOut:
					if (TrackBox(theWindow, theEvent->where, windowCode)) {
						SetPort(theWindow);
						EraseRect(&theWindow->portRect);
						ZoomWindow(theWindow, windowCode, true);
						AdjustScrollBars(theWindow, true);
						DrawPage((EditWindowPtr) theWindow);
						// Redraw the window
						SetPort(theWindow);
					  	InvalRect(&theWindow->portRect);
					}
				}
			}
			break;
		}
		break;
		
	// Was a key pressed?
	case keyDown: 
	case autoKey:
		// Was the cmd-key being held down?  If so, process menu bar short cuts.
	    if ((theEvent->modifiers & cmdKey) != 0) {
	      MyAdjustMenus();
		  MyHandleMenu(MenuKey((char) (theEvent->message & charCodeMask)));
		}
		else {
			theWindow = FrontWindow();
			if (((WindowPeek) theWindow)->refCon == MyWindowID &&
				((ObjectWindowPtr) theWindow)->ProcessKey != NULL)
				((ObjectWindowPtr) theWindow)->ProcessKey(theWindow, theEvent);
		}
		break;

	// Does a window need to be redrawn?
	case updateEvt:
		theWindow = (WindowPtr) theEvent->message;
		if (((WindowPeek) theWindow)->refCon == MyWindowID)
			((ObjectWindowPtr) theWindow)->Update(theWindow);
	    break;

	// Has a window been activated or deactivated?
	case activateEvt:
		theWindow = (WindowPtr) theEvent->message;

		// Force it to be redrawn
		if (((WindowPeek) theWindow)->refCon == MyWindowID)
			((ObjectWindowPtr) theWindow)->Activate(theWindow,(theEvent->modifiers & activeFlag) > 0);

		break;
	case osEvt:
		// Force it to be redrawn
		switch (theEvent->message >> 24) {
		case suspendResumeMessage:
			theWindow = FrontWindow();
			if (theWindow && ((WindowPeek) theWindow)->refCon == MyWindowID)
				((ObjectWindowPtr) theWindow)->Activate(theWindow,(theEvent->message & resumeFlag) > 0);
			break;
		}
		break;		
	case kHighLevelEvent:
		if (gSys7Flag)
			AEProcessAppleEvent(theEvent);
		break;
    }
}	    

// Do Idle Time Processing

void IdleObjects(EventRecord *er)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin) {
		if (theWin->refCon == MyWindowID &&
			((ObjectWindowPtr) theWin)->Idle)
			((ObjectWindowPtr) theWin)->Idle((WindowPtr) theWin, er);
		theWin = theWin->nextWindow;
	}
}

Boolean GotRequiredParams(AppleEvent *theEvent)
{
   DescType returnedType;
   Size 	actualSize;
   OSErr	err;
   err = AEGetAttributePtr ( theEvent, keyMissedKeywordAttr, 
						typeWildCard, &returnedType, NULL, 0, 
						&actualSize);
   
   return err == errAEDescNotFound;
   
 }	/* CAppleEvent::GotRequiredParams */


void DoOpenEvent(AppleEvent *theEvent)
{
	Handle		docList = NULL;
	long		i, numDocs;
	FSSpec		myFSS;
	DescType	eventID;
	AEDescList	theList;
	AEKeyword	aeKeyword=keyDirectObject;
	long		itemCount;
	DescType	actualType;
	Size		actualSize;
	OSErr		oe;

	if ((oe = AEGetParamDesc( theEvent, keyDirectObject, typeAEList, &theList)) != noErr) {
		DebugStr("\pAEGetParamDesc");
		return;
	}


	if (!GotRequiredParams(theEvent)) {
		DebugStr("\pGotRequiredParams");
		return;
	}

	if ((oe = AECountItems( &theList, &itemCount)) != noErr) {
		DebugStr("\pAECountItems");
		return;
	}


	for (i = 1; i <= itemCount; i++)
	{
		oe = AEGetNthPtr( &theList, i, typeFSS, &aeKeyword, &actualType,
						(Ptr) &myFSS, sizeof( FSSpec), &actualSize);

		if (oe == noErr) {
			OpenEditWindow(&myFSS);
		}
	}
	AEDisposeDesc(&theList);
	// event was handled successfully
}

pascal OSErr AppleEventHandler(AppleEvent *theEvent,AppleEvent *reply, long refCon)
{	
	OSErr		err;
	DescType	actualType;
	Size		actualSize;
	DescType	eventClass, eventID;
	OSErr		oe;

	if ((oe = AEGetAttributePtr( (AppleEvent*) theEvent, keyEventClassAttr,
					typeType, &actualType, (Ptr) &eventClass, 
					sizeof(eventClass), &actualSize)) != noErr)
			return oe;
							
	
	if ((oe = AEGetAttributePtr(  (AppleEvent*) theEvent, keyEventIDAttr,
					typeType, &actualType, (Ptr) &eventID, 
					sizeof(eventID), &actualSize)) != noErr)
			return oe;
									
	if (eventClass == kCoreEventClass)
	{
		switch (eventID)
		{
		case kAEOpenApplication:
			if (GotRequiredParams(theEvent))
			{
				// gGopher->DoCommand( cmdNew);
				// anAppleEvent->SetErrorResult( noErr);
				AskEditWindow();
			}
			break;
				
		case kAEOpenDocuments:
			DoOpenEvent( theEvent);
			break;
				
		case kAEPrintDocuments:
			break;
			
		case kAEQuitApplication:
			if (GotRequiredParams(theEvent))
			{
				gQuitFlag = true;
			}
			break;
		}		
	}
		
	return noErr;
}



void InitAppleEvents(void)
{
	if (gSys7Flag)
		AEInstallEventHandler(typeWildCard, typeWildCard,
								(EventHandlerProcPtr) AppleEventHandler,
								0,FALSE);
}

void CheckEnvironment(void)
{
	SysEnvRec	sEnv;
	OSErr		oe;

	oe = SysEnvirons(1,&sEnv);

	gSys7Flag = sEnv.systemVersion >= 0x0700;
	gColorQDFlag = sEnv.hasColorQD;
}

/* end Evtlab.c */