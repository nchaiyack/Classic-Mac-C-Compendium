// File "floaters.c" - 

#include <GestaltEqu.h>
#include <TextServices.h>

#include "floaters.h"
#include "main.h"

// * ****************************************************************************** *
// Global Declarations

extern GlobalsRec glob;

// * ****************************************************************************** *
// * ****************************************************************************** *

WindowPtr NewFloater(Ptr wStorage, Rect *bounds, Str255 title, Boolean visFlag,
		short wdefProc, WindowPtr behind, Boolean closeFlag, long refCon, long hotApp,
		FloaterEventProcPtr eventHandler, FloaterCloseProcPtr closeHandler) {
	short err=0;
	Boolean myStorage = (wStorage) ? TRUE : FALSE;
	WindowPtr win=0;
	FloaterQElemPtr myQElem;
	
	if (myStorage) wStorage = NewPtr(sizeof(WindowRecord));
	if (myStorage && !wStorage) return(0);
	
	err = NewServiceWindow(wStorage, bounds, title, FALSE, wdefProc, (WindowPtr) -1,
			closeFlag, (ComponentInstance) kCurrentProcess, &win);
	if (err) {
		if (myStorage) DisposePtr(wStorage);
		return(0);
		}
		
	myQElem = (FloaterQElemPtr) NewPtrSys(sizeof(* myQElem));
	if (! myQElem) {
		if (win) CloseServiceWindow(win);
		if (myStorage) DisposePtr(wStorage);
		return(0);
		}
	myQElem->qLink = 0;
	myQElem->qType = 0;
	myQElem->hideMe = FALSE;
	myQElem->hotApplication = hotApp;
	myQElem->floatWindow = win;
	myQElem->eventHandler = eventHandler;
	myQElem->closeHandler = closeHandler;
	Enqueue((QElem *) myQElem, &glob.allFloatWindows);

	SetWRefCon(win, refCon);
	HiliteWindow(win, -1);
	if (visFlag) ShowHideFloater(win);

	return(win);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void CloseFloater(WindowPtr win) {
	FloaterQElemPtr myQElem, qTail;
	
	// Get the window's QElem and release its memory
	myQElem = GetOneFloater(win, TRUE);
	if (myQElem) DisposePtr((Ptr) myQElem);
	
	ShowHide(win, 0);
	CloseServiceWindow(win);

	UpdateFloater(0);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void DisposeFloater(WindowPtr win) {
	FloaterQElemPtr myQElem;
	
	// Get the window's QElem and release its memory
	myQElem = GetOneFloater(win, TRUE);
	if (myQElem) DisposePtr((Ptr) myQElem);
	
	ShowHide(win, 0);
	CloseServiceWindow(win);
	DisposePtr((Ptr) win);
	
	UpdateFloater(0);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

FloaterQElemPtr GetOneFloater(WindowPtr win, Boolean dequeueIt) {
	FloaterQElemPtr myQElem;
	
	// Get the window's QElem and release its memory
	if (glob.allFloatWindows.qTail) glob.allFloatWindows.qTail->qLink = 0;
	for(myQElem = (FloaterQElemPtr) glob.allFloatWindows.qHead; myQElem && 
			(myQElem->floatWindow != win); myQElem = myQElem->qLink);

	if (dequeueIt && myQElem) Dequeue((QElem *) myQElem, &glob.allFloatWindows);
	return(myQElem);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

FloaterQElemPtr GetIndFloater(short index, Boolean dequeueIt) {
	FloaterQElemPtr myQElem;
	
	if (index <= 0) return(0);
	
	// Get the window's QElem and release its memory
	if (glob.allFloatWindows.qTail) glob.allFloatWindows.qTail->qLink = 0;
	for(myQElem = (FloaterQElemPtr) glob.allFloatWindows.qHead, index--;
			myQElem && index; myQElem = myQElem->qLink, index--);

	if (dequeueIt && myQElem) Dequeue((QElem *) myQElem, &glob.allFloatWindows);
	return(myQElem);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

// If you have set the Hot Application, then this function checks whether that
//   process is the front one and hides or shows the window as appropriate.
Boolean ShowHideFloater(WindowPtr win) {
	Boolean toHide, anyChanges=FALSE;
	short index;
	ProcessSerialNumber psn;
	ProcessInfoRec pInfo;
	FloaterQElemPtr myQElem;
	
	pInfo.processInfoLength = sizeof(pInfo);
	pInfo.processName = 0;
	pInfo.processAppSpec = 0;
	
	if (GetFrontProcess(&psn) || GetProcessInformation(&psn, &pInfo)) 
		pInfo.processSignature = -1;

	if (! win) {
		for(myQElem = GetIndFloater(index = 1, FALSE); myQElem; 
				myQElem = GetIndFloater(++index, FALSE)) {
			win = myQElem->floatWindow;
			if (! win) continue;

			toHide = glob.hideFloats || glob.blockFloats || glob.modalFloats || 
					myQElem->hideMe || (myQElem->hotApplication && 
					(pInfo.processSignature != myQElem->hotApplication));
			
			if (! toHide && ! ((WindowPeek) win)->visible) {
				ShowHide(win, TRUE);
				anyChanges = TRUE;
				}
			  else if (toHide && ((WindowPeek) win)->visible) {
				ShowHide(win, FALSE);
				anyChanges = TRUE;
				}
			}
		}
	  else if (myQElem = GetOneFloater(win, FALSE)) {
		toHide = glob.hideFloats || glob.blockFloats || glob.modalFloats || 
				myQElem->hideMe || (myQElem->hotApplication && 
				(pInfo.processSignature != myQElem->hotApplication));
		
		if (! toHide && ! ((WindowPeek) win)->visible) {
			ShowHide(win, TRUE);
			anyChanges = TRUE;
			}
		  else if (toHide && ((WindowPeek) win)->visible) {
			ShowHide(win, FALSE);
			anyChanges = TRUE;
			}
		}
	return(anyChanges);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void UpdateFloater(WindowPtr win) {
	short index;
	EventRecord theEvent;
	FloaterQElemPtr myQElem;

	theEvent.what = updateEvt;
	theEvent.when = TickCount();
	theEvent.message = (long) win;
	
	if (! win) {
		// Call the event handler for every floating window
		for(myQElem = GetIndFloater(index = 1, FALSE); myQElem; 
				myQElem = GetIndFloater(++index, FALSE)) {
			win = myQElem->floatWindow;
			if (win && ! EmptyRgn(((WindowPeek) win)->updateRgn)) {
				theEvent.message = (long) win;
				if (myQElem->eventHandler) (*myQElem->eventHandler)(&theEvent, win);
				}
			}
		}
	  else if (myQElem = GetOneFloater(win, FALSE))
		// Call the event handler for the indicated window
		if (! EmptyRgn(((WindowPeek) win)->updateRgn))
			if (myQElem->eventHandler) (*myQElem->eventHandler)(&theEvent, win);

	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void EventDispatchFloaters(EventRecord *theEvent, WindowPtr win) {
	short index;
	FloaterQElemPtr myQElem;

	if (! win) {
		// Call the event handler for every floating window
		for(myQElem = GetIndFloater(index = 1, FALSE); myQElem; 
				myQElem = GetIndFloater(++index, FALSE)) 
			if (myQElem->eventHandler) 
				(*myQElem->eventHandler)(theEvent, myQElem->floatWindow);
		}
	  else if (myQElem = GetOneFloater(win, FALSE))
		// Call the event handler for the indicated window
		if (myQElem->eventHandler) 
			(*myQElem->eventHandler)(theEvent, myQElem->floatWindow);

	}


// * ****************************************************************************** *
// * ****************************************************************************** *

void CloseRemainingFloaters() {
	FloaterQElemPtr myQElem;

	// For each tracked window, we call its close procedure - WE'RE QUITTING
	//   Of course, you may find it necessary to add a "Save?" dialog, and escape
	//   the quit sequence on a user cancel -- this code would need to be fixed.
	while(myQElem = GetIndFloater(1, FALSE)) 
		if (myQElem->closeHandler) (*myQElem->closeHandler)(myQElem->floatWindow);
		  else DisposeFloater(myQElem->floatWindow);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

Boolean GetFloaterEvent(EventRecord *theEvent) {
	Boolean floaterEvent = FALSE;
	short err=0, index;
	EvQEl *fwdEvent=0;
	FloaterQElemPtr myQElem;
	
	// Extract any forwarded events... and treat them as normal
	if (fwdEvent = (EvQEl *) glob.forwardedEvents.qHead)
		err = Dequeue((QElemPtr) fwdEvent, &glob.forwardedEvents);

	if (fwdEvent && !err) {
		BlockMove(&fwdEvent->evtQWhat, theEvent, sizeof(EventRecord));
		DisposePtr((Ptr) fwdEvent);
		
		// DONT ZERO IT OUT - The pointer indicates that we are
		//   handling an event for a Floater window. 

		floaterEvent = TRUE;
		}
		
	return(floaterEvent);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

// Thanks to Dair Grant (dair.grant@ucl.ac.uk) - I now check the MBarHeight
//   of the *Front* process, and remember (via globals) when executing in bkgd.

// Remember that this function must be called from within that app's context!
Boolean TestScreenSaver() {
	Boolean inFront=FALSE;
	long result;
	ProcessSerialNumber frontPSN, curPSN;

	glob.blockFloats = FALSE;

	// Most screensavers should register themselves this way
	if (!Gestalt('SAVR', &result) && (result & 0x02)) glob.blockFloats = TRUE;
	  else {
		// We also want to hide ourselves if the foreground application
		// has hidden the menu bar, since this implies they're taking
		// over the whole screen (and they probably don't want us visible).
		
		// Only when we are executing in the foreground app can we check the
		//   height of the menubar. When the front process has hidden its
		//   menubar, then we set a global flag to enact our desires.
		if (!GetFrontProcess(&frontPSN) && !GetCurrentProcess(&curPSN))
			if (SameProcess(&frontPSN, &curPSN, &inFront)) inFront = FALSE;
		
		if (inFront && (GetMBarHeight() == 0)) glob.blockFloats = TRUE;
		}
	
	return(glob.blockFloats);
	}


