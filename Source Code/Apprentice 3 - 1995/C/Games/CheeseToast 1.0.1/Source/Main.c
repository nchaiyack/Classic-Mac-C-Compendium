/************************************************************************************
 * Main.c
 *
 * CheeseToast by Jim Bumgardner
 *
 ************************************************************************************/
#include "CToast.h"
#include "ObjectWindow.h"
#include <Traps.h>

#if __option(profile)			// 6/15 Optional profiling support
#include <Console.h>
#include <Profile.h>
#endif

void 	MyInitMacintosh(void);
void	MyInitMultifinder(void);
void 	MyHandleEvent(void);
void 	MyDoEvent(EventRecord *theEvent);
void	IdleObjects(EventRecord *theEvent);
void	CheckEnvironment(void);

Boolean	gWNEImplemented;
Boolean	gDoneFlag;

// Typical Macintosh Initialization Code


// Main Entry Point

main()
{
	// Standard Mac Initialization
	MyInitMacintosh();

	// Check if Multifinder (WaitNextEvent) is implemented
	MyInitMultifinder();

	// Set up the menu bar
	MySetUpMenus();

	// Check if this is the appropriate type of Macintosh
	CheckEnvironment();

	// Open Graphics window
	MyNewWindow();
	BeginAttract();
	
	// Start Profiling
#if __option(profile)			// 6/15 Optional profiling support
	freopen("profile.log","w",stdout);		// If console isn't wanted
	InitProfile(200,200);
	_profile = 0;
	// cecho2file("profile.log",false,stdout);	// If console is wanted
#endif

	// main event loop
	while (!gDoneFlag)			// Till the End of Time...
		MyHandleEvent();		// Get an Event, do something about it
#if __option(profile)
	DumpProfile();
#endif

	CleanUp();
}

// Standard Macintosh Initialization

#define ExtraMasterBlocksNeeded	2
void MyInitMacintosh(void)
{
	short	i;
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();

	MaxApplZone();

	i = ExtraMasterBlocksNeeded;
	while (i--)
		MoreMasters();
}

// Check if WaitNextEvent (Multifinder) is implemented on this Macintosh

void MyInitMultifinder(void)
{
	gWNEImplemented = (NGetTrapAddress(_WaitNextEvent, ToolTrap) != 
					   NGetTrapAddress(_Unimplemented,ToolTrap));
}

#define E_BadEnviron	10000

void CheckEnvironment(void)
{
	SysEnvRec	sEnv;
	OSErr		oe;
	static		StringPtr eStr[7] = 
				{"\pThis Mac is ancient history - sorry!",
				 "\pCheeseToast needs Color Quickdraw - sorry!",
				 "\pCheeseToast needs system 6.0.7 or greater - sorry!",
				 "\pCheeseToast needs a 68020 or better - sorry!",
				 "\pCheeseToast requires a 12\" monitor or larger - sorry!",
				 "\pReset your monitor to 256 colors and try again.",
				 "\pThe CT Resources file is missing!"};

	Boolean		errFlag = false;
	char		eID = 0;

	oe = SysEnvirons(1,&sEnv);
	errFlag = true;

	if (oe != noErr)
		eID = 0;
	else if (!sEnv.hasColorQD)
		eID = 1;
	else if (sEnv.systemVersion < 0x0607)
		eID = 2;
	else if (sEnv.processor < 3)
		eID = 3;
	else if (screenBits.bounds.bottom - screenBits.bounds.top < 384 ||
			 screenBits.bounds.right - screenBits.bounds.left < 512)
		eID = 4;
	else {
		GDHandle	curDevice;
		curDevice = GetGDevice();
		if ((*curDevice)->gdPMap == NULL ||
			(*(*curDevice)->gdPMap)->pixelSize != 8)
			eID = 5;
		else {
			gResFile = OpenResFile("\pCT Resources");
			if (gResFile == -1)
				eID = 6;
			else
				errFlag = false;
		}
	}

	if (errFlag)
	{
		ParamText(eStr[eID],"\p","\p","\p");
		StopAlert(E_BadEnviron, NULL);
		ExitToShell();
	}
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

	if (ok) {
		// Handle the Event
		MyDoEvent(&theEvent);
	}
	else
		// Nothing happened, kick back...
		IdleObjects(&theEvent);
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
			if (theWindow != FrontWindow())
				// Make it so...
				SelectWindow(theWindow);
			else {	

				// Window is already in the front, handle the click
				switch (windowCode) {

				case inContent:		// Content area?
					if (((WindowPeek) theWindow)->refCon == MyWindowID)
						((ObjectWindowPtr) theWindow)->HandleClick(theWindow, theEvent->where);
					break;

				case inDrag:		// Dragbar?
					{
						Rect	dragRect;
						dragRect = screenBits.bounds;
						// Handle the dragging of the window
						DragWindow(theWindow, theEvent->where, &dragRect);
					}
					break;

				 case inGoAway:						// close box?
					// Only Grid Windows can be closed
				  	if (((WindowPeek) theWindow)->refCon == MyWindowID) {

						// Handle the mouse tracking for the close box
				  		if (TrackGoAway(theWindow, theEvent->where))
							// If mouse is released inside the close box
							// Hide or close the window
							((ObjectWindowPtr) theWindow)->Dispose(theWindow);
					}
				  	break;

				case inGrow:						// Grow box?
					{
						long	growResult;
						Rect	growRect;
						SetRect(&growRect,20,20,
								screenBits.bounds.right,screenBits.bounds.bottom);

						// Handle the mouse tracking for the resizing
						growResult = GrowWindow(theWindow,theEvent->where,&growRect);

						// Change the size of the window
						SizeWindow(theWindow,LoWord(growResult),HiWord(growResult),true);

						// Redraw the window
						SetPort(theWindow);
					  	InvalRect(&theWindow->portRect);
					}
					break;
				}
			}
			break;
		}
		break;
		
	// Was a key pressed?
	case keyUp:
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
				((ObjectWindowPtr) theWindow)->ProcessKey(theEvent);
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
			((ObjectWindowPtr) theWindow)->Activate(theWindow);

		break;
    }
}	    

// Do Idle Time Processing

void IdleObjects(EventRecord *theEvent)
{
	WindowPeek	theWin;
	theWin = (WindowPeek) FrontWindow();
	while (theWin) {
		if (theWin->refCon == MyWindowID &&
			((ObjectWindowPtr) theWin)->Idle)
			((ObjectWindowPtr) theWin)->Idle((WindowPtr) theWin,theEvent);
		theWin = theWin->nextWindow;
	}
}
