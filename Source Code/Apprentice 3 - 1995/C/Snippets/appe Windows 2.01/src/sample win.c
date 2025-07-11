// File "sample win.c" -

#include "main.h"
#include "floaters.h"
#include "prefs.h"
#include "sample drag.h"
#include "sample win.h"

// ***********************************************************************************
// Global Declarations 

extern GlobalsRec glob;

// ***********************************************************************************
// ***********************************************************************************

WindowPtr NewSampleWindow() {
	static short count = 0, **prefCount;
	Rect bounds; 
	WindowPtr win;
	Str255 textBuff = "\pSample Window ", altBuff;
	StringPtr winTextPtr;

	// Demo the prefs functions by incrementing the Window #
	if (prefCount = (short **) GetPrefs('WCnt', 1)) {
		count = (**prefCount < 999) ? ++(**prefCount) : (**prefCount = 0);
		WritePrefs((Handle) prefCount, 'WCnt', 1);
		DisposeHandle((Handle) prefCount);
		}
	  else if (prefCount = (short **) NewHandle(sizeof(**prefCount))) {
		**prefCount = ++count;
		WritePrefs((Handle) prefCount, 'WCnt', 1);
		DisposeHandle((Handle) prefCount);
		}
	  else count++;

	// Setup the Window Title String
	NumToString(count, altBuff);
	BlockMove(altBuff+1, textBuff + textBuff[0] + 1, altBuff[0]);
	textBuff[0] += altBuff[0];

	SetRect(&bounds, 100, 100, 320, 202);
	OffsetRect(&bounds, (count-1) % 3 * 40, (count-1) % 3 * 40);
	win = NewFloater(0, &bounds, textBuff, TRUE, 2048, (WindowPtr) -1, TRUE, 0, 0,
			SampleWindowEventHandler, DisposeSampleWindow);

	// Apply some text to draw for update routines
	if (win) SetSampleWindowText(win,
"\pAppeWin 2.0: A free demo program and source code by Matt Slot (fprefect@umich.edu) \
that shows off TSM Floaters.\r\rThe latest version is free to look at or use, and can \
be downloaded from your local Sumex or Mac-Archives mirror site.");
		
	if (win) {
		UpdateFloater(win);	// Force an immediate draw
		if (glob.hasDragMgr) SetupDragHandlers(win);
		}
	
	return(win);
	}

// ***********************************************************************************
// ***********************************************************************************

void DisposeSampleWindow(WindowPtr win) {
	StringPtr winTextPtr;
	
	if (winTextPtr = (StringPtr) GetWRefCon(win)) DisposePtr((Ptr) winTextPtr);
	DisposeFloater(win);
	
	// For a background only app, this will cause us to quit when the user
	//   closes the last floater. Its your choice if you want this to happen.
	if (glob.bkgdOnly && ! GetIndFloater(1, FALSE)) glob.quitting = TRUE;
	}

// ***********************************************************************************
// ***********************************************************************************

void SampleWindowEventHandler(EventRecord *theEvent, WindowPtr win) {
	Rect destRect;
	GrafPtr savePort;
	StringPtr winTextPtr;

	switch(theEvent->what) {
		case nullEvent:
			break;
		case mouseDown: {
			// The Window part has already been found and forwarded to us
			short thePart = theEvent->message;	
			
			switch(thePart) {
				case inContent:
					GetPort(&savePort);
					SetPort(win);

					if (glob.hasColorQD)
						LMSetHiliteMode(LMGetHiliteMode() ^ (1L << hiliteBit));
					InvertRect(&win->portRect);

					if (glob.hasDragMgr) SampleWindowDrag(theEvent, win);
					  else while(StillDown());

					if (glob.hasColorQD)
						LMSetHiliteMode(LMGetHiliteMode() ^ (1L << hiliteBit));
					InvertRect(&win->portRect);
					
					SetPort(savePort);
					break;
				case inGrow:
					break;
				case inGoAway:
					if (TrackGoAway(win, theEvent->where))
						DisposeSampleWindow(win);
					break;
				case inZoomIn:
					break;
				case inZoomOut:
					break;
				}
			break;
			}
		case keyDown:
		case autoKey:
			break;
		case updateEvt: {
			GetPort(&savePort);
			SetPort(win);
			BeginUpdate(win);
			
			// Draw something cool!
			destRect = win->portRect;
			EraseRect(&destRect);
			InsetRect(&destRect, 3, 2);
			TextFont(1); TextSize(9);
			if (winTextPtr = GetSampleWindowText(win)) 
				TextBox(winTextPtr+1, winTextPtr[0], &destRect, teFlushDefault);
			TextFont(0); TextSize(0);
			
			EndUpdate(win);
			SetPort(savePort);
			break;
			}
		}
	}

// ***********************************************************************************
// ***********************************************************************************

void SetSampleWindowText(WindowPtr win, StringPtr winText) {
	Boolean dirty = FALSE;
	Ptr storage = (Ptr) GetWRefCon(win);
	GrafPtr savePort;
	
	if (! storage) { storage = NewPtr(sizeof(Str255)); dirty = TRUE; }
	if (! storage) return;
	
	if (winText && ! EqualString(winText, (StringPtr) storage, 0, 0)) dirty = TRUE;
	BlockMove((winText && winText[0]) ? winText : "\p", storage, sizeof(Str255));
	SetWRefCon(win, (long) storage);
	
	// Force a redraw after changing content
	if (dirty) {
		GetPort(&savePort);
		SetPort(win);
		InvalRect(&win->portRect);
		SetPort(savePort);
		}
	}

// ***********************************************************************************
// ***********************************************************************************

StringPtr GetSampleWindowText(WindowPtr win) {
	Ptr storage = (Ptr) GetWRefCon(win);
	StringPtr winText;

	winText = (storage && ((StringPtr) storage[0])) ? ((StringPtr) storage) : "\p<No Data>";
	return(winText); 
	}
	