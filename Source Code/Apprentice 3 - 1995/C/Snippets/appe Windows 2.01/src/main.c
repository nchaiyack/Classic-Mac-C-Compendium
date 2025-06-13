// File "main.c" - 

#include <GestaltEqu.h>
#include <TextServices.h>
#include <Traps.h>

#include "main.h"
#include "aevents.h"
#include "filter.h"
#include "floaters.h"
#include "notify.h"
#include "patches.h"
#include "sample win.h"

// * ****************************************************************************** *
// Global Declarations

GlobalsRec glob;

// * ****************************************************************************** *
// * ****************************************************************************** *

void DoTest() {
	short i;
	long response;
	
	for(i=0; i<sizeof(glob); ((char *) &glob)[i++] = 0);
	
	// Simple Tests for compatibility
	if (Gestalt(gestaltSystemVersion, &response) || (response < 0x0604) ||
			Gestalt(gestaltTSMgrVersion, &response))
		UserNotify(kNotifyStringList, kSys71NotifyString, TRUE);
		
	// I could look at my ProcessInfoRec, but this is a single line solution
	glob.bkgdOnly = ((** (short **) GetResource('SIZE', -1)) & 0x0400) ? TRUE : FALSE;
	glob.hasColorQD = (! Gestalt(gestaltQuickdrawFeatures, &response) &&
			(response & (1 << gestaltHasColor))) ? TRUE : FALSE;
	glob.hasGDevices = (NGetTrapAddress(_GetDeviceList, ToolTrap) !=
			NGetTrapAddress(_Unimplemented, ToolTrap)) ? TRUE : FALSE;
	glob.hasDragMgr = (! Gestalt(gestaltDragMgrAttr, &response) &&
			(response & (1 << gestaltDragMgrPresent))) ? TRUE : FALSE;

	// Inform the user how to find the "TSM Fix" if it isn't installed
	if (Gestalt('TSM+', &response))
		UserNotify(kNotifyStringList, kTSMFixNotifyString, FALSE);
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void DoInit() {
	long saveA5;
	
	// 'appe' programs get 8k of Stack Space by default -- we need MORE! 
	if (glob.bkgdOnly) SetApplLimit(GetApplLimit() - 16384);
	MaxApplZone();
	MoreMasters();

	// Basic Initialization 
	InitGraf(&qd.thePort);
	
	// Finish our Initialization - but only if we are a foreground app. Apple
	//   warns loudly that bkgd-only apps should not call InitWindows(), etc.
	if (! glob.bkgdOnly) {
		InitFonts();
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs(0);
		InitCursor();
		
		InsertMenu(GetMenu(kAppleMenuID), 0);
		AddResMenu(GetMHandle(kAppleMenuID), 'DRVR');
		InsertMenu(GetMenu(kFileMenuID), 0);
		InsertMenu(GetMenu(kEditMenuID), 0);
		DisableItem(GetMHandle(kEditMenuID), 0);
		
		DrawMenuBar();
		}
	  else {
		// After reading the above, you know that you can't call any other toolbox
		//   init stuff. However, FindServiceWindow() fails if the MenuList lomem 
		//   global is NIL! This is a moderate (but not recommended) workaround.
		InitFonts();
		InitMenus();
		}
	
	PatchNewWindow();
	PatchExitToShell();

	// Save this information for later
	GetCurrentProcess(&glob.myPSN);	
		
	// Install an event filter or die
	SetA5(saveA5 = SetA5(0));
	glob.filterProc =
			(Ptr) InstallEventFilter((FilterHelperUPP) EventFilterHelper, (Ptr) saveA5);
	if (! glob.filterProc)
		UserNotify(kNotifyStringList, kJGNENotifyString, TRUE);
	
	InitHLEvents();
	NewSampleWindow();
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void DoLoop() {
	Boolean floaterEvent;
	
	while(! glob.quitting) {
		// If there are no internal events pending, then we call WNE() with a
		//   moderate delay. Otherwise, we want to make a fast response to any 
		//   floater events, so we make sure the event filter will wake us up quickly.
		if (! glob.forwardedEvents.qHead) 
			WaitNextEvent(everyEvent, &glob.theEvent, 150, 0);	// Sleep for 2.5 secs
		
		// If nothing to do, then check our private event queue to see if any
		//   clicks or keydowns are pending for our floater windows.
		floaterEvent = (glob.theEvent.what != nullEvent) ? FALSE : 
				GetFloaterEvent(&glob.theEvent);
		
		// Check the states of our floaters, then generate and handle any 
		//   pending Update Events for floaters because the Event Mgr will
		//   not generate them for us.
		ShowHideFloater(0);
		UpdateFloater(0);

		switch(glob.theEvent.what) {
			case nullEvent:
				break;
			case mouseDown: {
				short thePart;
				Rect bounds;
				WindowPtr whichWin, frontWin;
				
				if (floaterEvent) {
					thePart = FindServiceWindow(glob.theEvent.where, &whichWin);
					if (GetFrontServiceWindow(&frontWin)) frontWin = 0;
					}
				  else {
					thePart = FindWindow(glob.theEvent.where, &whichWin);
					frontWin = FrontWindow();
					}

				glob.theEvent.message = thePart;		
				switch(thePart) {
					case inMenuBar:
						DoMenuItem(MenuSelect(glob.theEvent.where));
						break;
						
					case inSysWindow:
						SystemClick(&glob.theEvent, whichWin);
						break;
						
					case inDrag:
						if (whichWin != frontWin) {
							SelectWindow(whichWin);
							UpdateFloater(whichWin);
							}
						bounds = (*GetGrayRgn())->rgnBBox;
						DragWindow(whichWin, glob.theEvent.where, &bounds);
						UpdateFloater(whichWin);
						break;
						
					case inContent:
					case inGrow:
					case inGoAway:
					case inZoomIn:
					case inZoomOut:
						if (whichWin != frontWin) {
							SelectWindow(whichWin);
							UpdateFloater(whichWin);
							}
						EventDispatchFloaters(&glob.theEvent, whichWin);
						break;
					}
				break;
				}
			case keyDown:
			case autoKey: {
				char theKey, theChar;
				
				theChar = glob.theEvent.message & charCodeMask;
				theKey = (glob.theEvent.message & keyCodeMask) >> 8;
				
				if (glob.theEvent.modifiers & cmdKey)
					DoMenuItem(MenuKey(glob.theEvent.message & charCodeMask));
				  else EventDispatchFloaters(&glob.theEvent, 0);
				}
				break;
					
			case updateEvt:
				if (GetOneFloater((WindowPtr) glob.theEvent.message, FALSE))
					EventDispatchFloaters(&glob.theEvent, (WindowPtr) glob.theEvent.message);
				break;
			case activateEvt:
				break;
			case kHighLevelEvent:
				AEProcessAppleEvent(&glob.theEvent);
				break;
			}
		
		// WARNING: Dont set the cursor to arrow just because it is over a floater
		//   window -- most foreground apps are not smart enough to recognize a
		//   the TSM floaters, and the cursor may start to flicker.
		if (! glob.bkgdOnly) SetCursor(&qd.arrow);
		}
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void DoMenuItem(long theMenuAndItem) {
	short theMenu, theItem;
	Str63 theString;
	
	if (! theMenuAndItem) return;
	
	theMenu = (theMenuAndItem & 0xFFFF0000) >> 16;
	theItem = theMenuAndItem & 0x0000FFFF;
	
	switch(theMenu) {
		case kAppleMenuID:
			if (theItem == kAppleMenuAboutItem) {
				// Block out the floaters while we display our about box... 
				//   then enable and update them after we have finished.
				glob.modalFloats = TRUE;
				ShowHideFloater(0);
				
				Alert(128, 0);
				
				glob.modalFloats = FALSE;
				ShowHideFloater(0);
				UpdateFloater(0);
				}
			  else {
				GetItem(GetMHandle(theMenu), theItem, theString);
				OpenDeskAcc(theString);
				}
			break;
			
		case kFileMenuID:
			switch(theItem) {
				case kFileMenuNewItem:
					NewSampleWindow();
					break;
				case kFileMenuQuitItem:
					glob.quitting = TRUE;
					break;
				}
			break;
			
		case kEditMenuID:
			break;

		default:
			break;
		}
	HiliteMenu(0);
	DrawMenuBar();
	}

// * ****************************************************************************** *
// * ****************************************************************************** *

void DoDispose() {
	static Boolean done = FALSE;
	
	// Our ExitToShell() patch may cause re-entrancy problems... so we bracket
	//   the functional calls by checking and setting a one-time flag.
	if (! done) {
		done = TRUE;
		
		CloseRemainingFloaters();
		
		ReleaseEventFilter(glob.filterProc);
		glob.filterProc = 0;
		}
	}
		
// * ****************************************************************************** *
// * ****************************************************************************** *

void main() {
	DoTest();
	DoInit();
	DoLoop();
	DoDispose();
	}
	
