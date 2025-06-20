// File "main.h" - 

#ifndef ____MAIN_HEADER____
#define ____MAIN_HEADER____

#ifndef __PROCESSES__
#include <Processes.h>
#endif  __PROCESSES__

// * ****************************************************************************** *
// * ****************************************************************************** *

#define kAppleMenuID			128
#define kAppleMenuAboutItem		  1
#define kAppleMenuSeparator1	  2

#define kFileMenuID				129
#define kFileMenuNewItem		  1
#define kFileMenuSeparator1		  2
#define kFileMenuQuitItem		  3

#define kEditMenuID				130
#define kEditMenuUndoItem		  1
#define kEditMenuSeparator1		  2
#define kEditMenuCutItem		  3
#define kEditMenuCopyItem		  4
#define kEditMenuPasteItem		  5
#define kEditMenuClearItem		  6

// * ****************************************************************************** *
// * ****************************************************************************** *

typedef struct {
	Boolean bkgdOnly;		// Is the process an FBA ('appe') or normal app ('APPL')
	Boolean hasColorQD;		// Check once in case we need to know later
	Boolean hasGDevices;	// Check once in case we need to know later
	Boolean hasDragMgr;		// If we have it, we get all kinds of kewl functionality.
	Boolean quitting;		// A simple flag (set from AE handlers) to close up shop

	Boolean hideFloats;		// A user controlled hide flag for showing/hiding floaters
	Boolean blockFloats;	// An overriding hide flag for screen savers/full-screen apps
	Boolean modalFloats;	// An overriding hide flag for our own modal dialogs

	EventRecord theEvent;	// The event currently being processed
	QHdr forwardedEvents;	// A queue of EventRecords that are forwarded from our filter
	QHdr allFloatWindows;	// A queue of WindowPtrs that track each TSM Floater that was
							//   installed, so that the filter can intercept their events.
	
	ProcessSerialNumber myPSN;
	Ptr filterProc;
	} GlobalsRec;

// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

void DoTest(void);
void DoInit(void);
void DoLoop(void);
void DoMenuItem(long theMenuAndItem);
void DoDispose(void);

void main(void);

#endif  ____MAIN_HEADER____

