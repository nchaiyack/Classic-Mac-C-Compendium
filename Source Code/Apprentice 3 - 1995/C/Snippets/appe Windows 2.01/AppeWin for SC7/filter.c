// File "filter.c" -

#include <TextServices.h>

#include "main.h"
#include "filter.h"
#include "floaters.h"

// ***********************************************************************************
// Global Declarations 

extern GlobalsRec glob;

// ***********************************************************************************
// ***********************************************************************************

/*
	The Not-So-Simple FAT jGNEFilter 
		Original Code by Matt Slot (fprefect@umich.edu), 6/2/95 
			with help and criticism (lots!) from Ed Wynne (arwyn@umich.edu).
	
	Quick Intro
		Since jGNEFilters are nasty things in 68k, and pretty much impossible 
		in PPC, writing simple and cross-compiling handler code is also. To 
		facilitate use of this, I have written some interface routines to hide
		some of the complexity from the application programmer.
		
	/	Ptr InstallEventFilter(FilterHelperProcPtr helperProc, Ptr helperData);
	|		Installs a jGNEFilter which can properly call a callback from within
	|		the application. The callback handles incoming events wither either 
	|		68k or PPC code, and gets passed the data pointer you send from here.
	|		Install() returns a pointer the jGNEFilter (which has been loaded from
	|		a resource) installed in the system heap -- or NIL to indicate an error.

	/	Ptr ReleaseEventFilter(Ptr filterProc);
	|		Pass in the pointer to the jGNEFilter, and this routine will disable 
	|		the handling and try to dispose of its storage in the System Heap.
	|		You *must* do this if your helper function is going to disappear when
	|		the application closes. 
	
	/	Code Resource 'jGNE', #128: the jGNEFilter
	|		Since there is no way to get the same routine to compile on both 68K
	|		and PPC, I have compiled the jGNEFilter into an executable resource.
	|		The real code of this resource has been compiled from the additional
	|		source file you can find in the project folder. Note that the code simply  
	|		calls the ProcPtr for the Helper function blindly... whether 68k or PPC. 
	|		See below for more details as to how the filter is loaded, unloaded,
	|		and called.

	/	void EventFilterHelper(EventRecord *theEvent, Ptr helperData);
	|		This callback is the workhorse of the event filter. Once installed, this  
	|		routine sees every event that gets harvested and has an opportunity to
	|		modify the record before the front application gets to see it. Ideally
	|		this function can do the necessary work itself or pass off the event info
	|		to the home application.
	|		WARNING: For 68k code, the routine will be called from the current app's
	|		context (A5/Globals, Rsrc File, HeapZone). PPC code will have a valid RTOC
	|		(Globals access) but not Rsrc File, HeapZone, etc.
	

	How this all works:
		
		Since I didn't want to write the jGNEFilter in C (OK, I couldn't figure out
		a good way), the code is carried around in a resource. Originally, I had 
		saved the routine a constant string that was StuffHex()'ed into the System 
		Heap pointer... but I got so many questions and comments I decided to do it 
		the normal way.
		
		The jGNEFilter keeps 3 pieces of data inline: the next filter in the chain,
		a pointer to the helper routine, and some extra data to pass to the helper.
		Most importantly, we may not be able to remove the filter from the calling
		chain...  the architecture just doesn't permit it! If we are able to safely
		pull the filter out, we do. Otherwise the next best solution is to keep a flag,
		that we can clear when we want to disable the functionality -- in fact, we
		set or clear the pointer to the Helper Proc as the flag.
		
		Finally, the helper function is the meat of our jGNEFilter; it does the work
		of the active filter. In the case of a 68K helper, it is accessed via a 
		simple ProcPtr. In the case of a PPC helper, the installer sets up a valid
		RoutineDescriptor (in the System Heap with the jGNEFilter) to invoke a 
		MixedMode switch between the 68K caller (filter) and PPC routine (helper).
		Again, when releasing the filter the handler disposes the descriptor and 
		clear the inline ProcPtr/flag, since the helper function will probably be
		disappearing when the application quits.
		
		If you are picking out events to handle within your app, my suggestion is to
		keep a secondary Queue of events in the System Heap -- remember, you must 
		allocate new EventRecords (since the current event belongs to the calling
		app) into the System Heap (you need a heap that both the current process and
		your own process can access). Given these events, your main event loop can 
		suck out clicks or keydowns for dispatching internally and safely within your
		own context.
		
		Also, Text Service windows don't receive Activate or Update events... you 
		must check for those manually within your own event loop and handle them.
	
*/

// ***********************************************************************************
// ***********************************************************************************

Ptr InstallEventFilter(FilterHelperProcPtr helperProc, Ptr helperData) {
	Handle filterRsrc;
	Ptr filterProc, data;
	
	// Create a duplicate function in the System Heap (so its *alway* there) and
	//   copy the data across. Note: even though it is technically a function, we
	//   can still treat it as data safely until it has been installed and called.
	filterRsrc = Get1Resource(kJGNEFilterResType, kJGNEFilterResID);
	filterProc = NewPtrSys(GetHandleSize(filterRsrc));
	// If either is nil, there is no need to free.. we are quitting anyway
	if (! filterRsrc || ! filterProc) return(0); 
	BlockMove(*filterRsrc, filterProc, GetHandleSize(filterRsrc));
	ReleaseResource(filterRsrc);
	
	// Get and install the current filter as the next filter in the chain.
	data = (Ptr) GetJGNEFilter();
	BlockMove(&data, filterProc + kNextFilterOffset, sizeof(data));

	// Get and install the Helper function to do the real work (and as a flag to
	//   indicate we are in business and accepting events). Remember that if we
	//   generating PPC code, it is necessary to establish a Routine Descriptor.
	SetZone(SystemZone());
	data = (Ptr) helperProc;
	if (! data) return(0);
	BlockMove(&data, filterProc + kEventHelperOffset, sizeof(data));
	SetZone(ApplicZone());

	// If the caller wants to pass data to the jGNEFilter Helper function. This
	//   pointer (or handle if desired) *must* be allocated in the System Heap
	//   if you don't plan on releasing the Filter before quitting. If you plan
	//   on releasing the filter, then either the App or Sys heap will suffice.
	data = helperData;
	BlockMove(&helperData, filterProc + kEventHelperDataOffset, sizeof(data));
	
	// Install us, we are ready to do some work!
	SetJGNEFilter((ProcPtr) filterProc);
	
	return(filterProc);
	}

// ***********************************************************************************
// ***********************************************************************************

Ptr ReleaseEventFilter(Ptr filterProc) {
	Ptr data;
	
	if (! filterProc) return(0);

	// Clear the Helper location as an indicator that we have closed up shop. The
	//   filter itself may lingers in the System Heap until shutdown unless we can
	//   find a way to extract it from the chain (see below). On the other hand, the 
	//   filter has been written so that if the Helper function pointer is NIL, the 
	//   filter will do nothing at all. Let's zero it out for that (hopeful) case.
	BlockMove(filterProc + kEventHelperOffset, &data, sizeof(data));
	data = 0;
	BlockMove(&data, filterProc + kEventHelperOffset, sizeof(data));
	

	// If the installed filterProc is the first one in the chain, then we should
	//    be able remove it and replace it with the next one (the one we would
	//    normally jump to). If we can dispose the filterProc buffer, then we can
	//    can recover those 50 bytes that remain in the System Heap.
	// Thanks to HoverBar's Guy Fullerton (hedgeboy@realm.net) for the suggestion.
	if (filterProc == (Ptr) GetJGNEFilter()) {
		// Remove our filterProc from the chain.
		BlockMove(filterProc + kNextFilterOffset, &data, sizeof(data));
		SetJGNEFilter((ProcPtr) data);
		
		BlockMove(filterProc + kEventHelperDataOffset, &data, sizeof(data));
		DisposePtr(filterProc);
		}
	  else {
		// Grab the data that was passed when initialized or as set in the Helper
		//   function. The caller can then deallocate it if desired or necessary.
		
		BlockMove(filterProc + kEventHelperDataOffset, &data, sizeof(data));
		}
	
	return(data);
	}

// ***********************************************************************************
// ***********************************************************************************

void EventFilterHelper(EventRecord *theEvent, Ptr helperData) {
	Boolean fwdThisEvent = FALSE, filterThisEvent = FALSE;
	long saveA5;
	EvQEl *fwdEvent;
	
	// This only does something in 68K code. We now have access to globals,
	//   which PPC get for free from CFM; however, we won't have access to 
	//   our application's Resource file/chain or HeapZone. Be careful!
	saveA5 = SetA5((long) helperData);
	
	// Check to see if the floaters should all be hidden, then hide or show any
	//   window's as necessary. Note: Update events will be entered/handled in
	//   our context, so let our app get the CPU to handle the event quickly
	TestScreenSaver();
	if (ShowHideFloater(0)) WakeUpProcess(&glob.myPSN);
	
	switch(theEvent->what) {
		case nullEvent:
			break;
		case mouseDown: {
			short thePart, index;
			Boolean found;
			WindowPtr whichWin;
			FloaterQElemPtr floatQElem;
			
			// The following tests shouldnt be necessary... but sometimes there are
			//   problems with the FindServiceWindow() call. To get around that,
			//   eliminate a few checks in some cases that we know are wrong.
		
			for(found = FALSE, floatQElem = GetIndFloater(index = 1, FALSE);
					floatQElem && !found; floatQElem = GetIndFloater(++index, FALSE)) {
				if (PtInRgn(theEvent->where,
						((WindowPeek) floatQElem->floatWindow)->strucRgn)) found = TRUE;
				}
			if (! found) break;
			
			thePart = FindServiceWindow(theEvent->where, &whichWin);
			theEvent->message = thePart;
			switch(thePart) {
				case inMenuBar:
				case inSysWindow:
					break;
				case inDrag:
				case inContent:
				case inGrow:
				case inGoAway:
				case inZoomIn:
				case inZoomOut:
					fwdThisEvent = TRUE;
					filterThisEvent = TRUE;
					break;
				}
			}
			break;
		case keyDown:
		case autoKey: {
			char theKey, theChar;
			
			theChar = theEvent->message & charCodeMask;
			theKey = (theEvent->message & keyCodeMask) >> 8;
			
			// Add your selected tests for key-shortcuts
			if ((theEvent->modifiers & cmdKey) && (theKey == 0x35)) {
				// Cmd-Escape is the sequence to temporarily hide floaters
				//   and a wakeup to handle any pending update events
				glob.hideFloats = (glob.hideFloats) ? FALSE : TRUE;
				if (ShowHideFloater(0)) WakeUpProcess(&glob.myPSN);

				fwdThisEvent = filterThisEvent = FALSE;
				}
			}
			break;
		default:
			break;
		}
	

	if (fwdThisEvent) {
		// We have discovered that this event deserves full attention. Forward it to
		//   our home application by Q'ing it up as a pointer in the System Heap
		fwdEvent = (EvQEl *) NewPtrSys(sizeof(*fwdEvent));
		fwdEvent->qLink = 0;
		fwdEvent->qType = evType;
		BlockMove(&theEvent->what, &fwdEvent->evtQWhat, sizeof(EventRecord));
		Enqueue((QElem *) fwdEvent, &glob.forwardedEvents);
		
		// Let our app get the CPU to handle the event quickly
		WakeUpProcess(&glob.myPSN);
		}
		
	// Does intercepting the event mean no one else should have it?
	if (filterThisEvent) theEvent->what = nullEvent;

	// Restore the (68K) context before leaving
	SetA5(saveA5);
	}

