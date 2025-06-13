#include <MacHeaders>
#include "constants.h"
#include "globals.h"
#include "morphutils.h"
#include "redraw.h"


// ---------------  Function Protos  (alphabetized)
static void AdjustCursor(Point where, RgnHandle cursorRgn);
       void Cleanup(void);
static void DoAbout(void);
static void DoActivate(EventRecord *theEvent);
static void DoCommand(long mResult);
static void DoDialogEvent(EventRecord *theEvent);
static void DoEvent(EventRecord *theEvent);
static void DoKeyDown(EventRecord *theEvent);
static void DoIdle(EventRecord *theEvent, RgnHandle cursorRgn);
static void DoInfo(void);
static void DoMouseDown(EventRecord *theEvent);
static void DoUpdate(EventRecord *theEvent);
static void Init(void);
static void SetupMenus(void);
static void SetupWindows(void);
       void main(void);

// ---------------  Functions

static void AdjustCursor(Point where, RgnHandle cursorRgn)
{
	// if in image window, change cursor to + cursor.
	// else return to normal pointer.
	GrafPtr savedPort;
	Point pt;

#ifdef DEBUG
	DebugStr("\pAdjustCursor");
#endif
	GetPort(&savedPort);
	SetPort(gMainWindow);
	pt = where;
	GlobalToLocal(&pt);  // convert to main window coords
	if ( (gCurrentWindow == gMainWindow) &&
		  PtInRect( pt, &gMainWindow->portRect))
	{	if (gCrossCursor != NULL)
			SetCursor( *gCrossCursor);  // dereference handle to cursor *
										// OK to do, doesn't move heap
	}
	else
		InitCursor();	// restore to arrow
	
	SetPort(savedPort);
	
} // AdjustCursor()


void Cleanup(void)
{
//  Call any cleanup routines we need to to free any storage we alloc'd.
//
	CleanupOffscreen();		// get rid of offscreen bitmap -- offscreen.c
} // Cleanup()


static void DoAbout(void)
{
	short itemHit = -9;
	DialogPtr aboutDlog;
	
	if ((aboutDlog = GetNewDialog(kAboutDLOG, NULL, (void*)(-1))) != NULL)
	{
		ShowWindow((WindowPtr)aboutDlog);
		SelectWindow((WindowPtr)aboutDlog);
		UserItem( aboutDlog, kAboutOKOutline, OutlineButton);
		while (itemHit != kAboutOKButt)
		{
			ModalDialog( NULL, &itemHit);
			if (itemHit == kAboutInfoButt)
				DoInfo();	// display the more info dialog...
		}
		DisposDialog(aboutDlog);
	} else
		SysBeep(5);       /* beep if we can't get dialog box */
} // DoAbout() 


static void DoActivate(EventRecord *theEvent)
{
#ifdef DEBUG
	DebugStr("\pDoActivate");
#endif
	if (gControlDialog == (WindowPtr)theEvent->message)
		DoDialogEvent(theEvent);
} // DoActivate()


static void DoCommand(long mResult)
{
	int theItem = LoWord(mResult);
	int theMenu = HiWord(mResult);
	Str255 name;
	int temp;
	
#ifdef DEBUG
	DebugStr("\pDoCommand");
#endif
	switch (theMenu) {
		case kAppleM:
			if (theItem == kAppleAboutItem) {
				DoAbout();
			} else {
				GetItem( gAppleM, theItem, name);
				temp = OpenDeskAcc(name);
			}
			break;
			
		case kFileM:
			switch (theItem) {
				case kFileNewItem:
					break;
				case kFileOpenItem:
					break;
				case kFileSaveAsItem:
					SavePict();		// savepict.c
					break;
				case kFileQuitItem:
					Cleanup();
					break;
			} /* switch theItem */
			break;
			
		case kEditM:
			break;
		
		case kTypeM:
			switch(theItem)
			{
			case kTypeAddItem:
				MorphAddRes();
				break;
			case kTypeDeleteItem:
				MorphDelRes();
				break;
			
			case kTypeDefaultItem:
				MorphClearProc();
				gMorphProcH = gDefaultProcH;     // use the builtin
				CheckItem( gTypeM, gTypeChkItem, FALSE);  // uncheck prev
				CheckItem( gTypeM, kTypeDefaultItem, TRUE);
				gTypeChkItem = kTypeDefaultItem;
				break;

			default:  // whichever is left...
				MorphClearProc();
				CheckItem( gTypeM, gTypeChkItem, FALSE); // uncheck it
				
				GetItem( gTypeM, theItem, name);
				gMorphProcH =
						(MorphProcH)GetNamedResource( kMorphECR, name);
				
				if (gMorphProcH != NULL)   // got it
				{
					HLock((Handle)gMorphProcH);
					gTypeChkItem = theItem;
				}
				else   // didn't get it so use the default.
				{
					Error( kMorphCodeErr, 0, 0, 0, noteIcon);
					gMorphProcH = gDefaultProcH;
					gTypeChkItem = kTypeDefaultItem;
				}
				CheckItem( gTypeM, gTypeChkItem, TRUE);
				break;
			} // kTypeM switch
			break;
			
	} /* switch theMenu */
	HiliteMenu(0);		/* unhilite the selected menu */
} // DoCommand()


static void DoDialogEvent(EventRecord *theEvent)
{
	DialogPtr whichDialog;
	Boolean btemp;
	int itemHit;
	
#ifdef DEBUG
	DebugStr("\pDoDialogEvent");
#endif
	if (IsDialogEvent(theEvent))
	{
		if (DialogSelect(theEvent, &whichDialog, &itemHit))
			switch(itemHit) {
			case kZoomInB:
				ZoomIn();	// redraw.c
				break;
				
			case kZoomOutB:
				ZoomOut();	// redraw.c
				break;
				
			case kRedrawB:
				Redraw();	// redraw.c
				break;
				
			case kSelectB:
				// blow up the selection
				break;
			} // switch
	} // if

} // DoDialogEvent()


static void DoEvent(EventRecord *theEvent)
{
	switch( theEvent->what)
	{
		case nullEvent:
			DoDialogEvent(theEvent);
			break;
		case mouseDown:
			DoMouseDown(theEvent);
			break;
		case keyDown:
		case autoKey:
			DoKeyDown(theEvent);
			break;
		case activateEvt:
			DoActivate(theEvent);
			break;
		case updateEvt:
			DoUpdate(theEvent);
			break;
		case kHighLevelEvent:
			break;
		case osEvt:		/* old app4Evt -- suspend/resume event */
						/* modifiers contains data */
			{
				Str255 s;
				Print("osEvt, mods=");
				NumToString( (long)theEvent->modifiers, s);
				PtoCstr( s);
				Print( (char*)s);
			}
			break;
	}
		
} // DoEvent()


static void DoIdle(EventRecord *theEvent, RgnHandle cursorRgn)
{
#ifdef DEBUG
	DebugStr("\pDoIdle, before AdjustCursor");
#endif
	AdjustCursor(theEvent->where, cursorRgn);
#ifdef DEBUG
	DebugStr("\pDoIdle, before DoDialogEvent");
#endif
	DoDialogEvent(theEvent);
} // DoIdle()


static void DoInfo(void)
{
	short itemHit = -9;
	DialogPtr infoDlog;
	
	if ((infoDlog = GetNewDialog(kInfoDLOG, NULL, (void*)(-1))) != NULL)
	{
		ShowWindow((WindowPtr)infoDlog);
		SelectWindow((WindowPtr)infoDlog);
		UserItem( infoDlog, kInfoOKOutline, OutlineButton);
		while (itemHit != kInfoOKButt)
			ModalDialog( NULL, &itemHit);
		DisposDialog(infoDlog);
	} else
		SysBeep(5);       /* beep if we can't get dialog box */
} // DoInfo() 


static void DoKeyDown(EventRecord *theEvent)
{
	char theChar = (char)(theEvent->message & charCodeMask);
	if (theEvent->modifiers & cmdKey)
		DoCommand( MenuKey(theChar));
	else
		if (gCurrentWindow == gControlDialog)
			DoDialogEvent(theEvent);
} // DoKeyDown()


static void DoMouseDown(EventRecord *theEvent)
{
	WindowPtr whichWindow;
	Rect dragRect;
	int  part;
	
#ifdef DEBUG
	DebugStr("\pDoMouseDown");
#endif
	dragRect = qd.screenBits.bounds;
	InsetRect( &dragRect, -4, -4);
	
	part = FindWindow(theEvent->where, &whichWindow);
	
	switch(part) {
		case inMenuBar:
			DoCommand( MenuSelect(theEvent->where) );
			break;
		case inContent:
			if (whichWindow != gCurrentWindow)
			{	SelectWindow(whichWindow);		// bring clicked window up
				gCurrentWindow = whichWindow;
			}
			else if (gCurrentWindow == gControlDialog)  // dialog event
				DoDialogEvent(theEvent);
			else if (gCurrentWindow == gMainWindow)	// track mouse coords
				TrackCoords(theEvent->where);
			break;
		case inDrag:
			if (whichWindow != gCurrentWindow)
			{
				if ( theEvent->modifiers & cmdKey)
				{
					// then drag whichWindow around
					DragWindow( whichWindow, theEvent->where, &dragRect);
				}
				else  // no command key, so bring whichWindow to the front
				{
					SelectWindow(whichWindow);
					gCurrentWindow = whichWindow;
				}
			}
			else // a click in the current window's drag region
			{
				// then drag whichWindow around
				DragWindow( whichWindow, theEvent->where, &dragRect);
			}
			break;
		case inGrow:
			break;
		case inGoAway:
			break;
	} // switch
} // DoMouseDown()


static void DoUpdate(EventRecord *theEvent)
{
	GrafPtr savedPort;
	
#ifdef DEBUG
	DebugStr("\pDoUpdate");
#endif
	if ( gControlDialog == (WindowPtr)theEvent->message)
	{
		Print("Dialog update");
		DoDialogEvent(theEvent);
	}
	else if (gMainWindow == (WindowPtr)theEvent->message)
	{
		Print("Main update");
		GetPort(&savedPort);
		SetPort(gMainWindow);
		BeginUpdate(gMainWindow);
		// Restore the entire bitmap image...
		CopyBits( &gOffGP.portBits,  &gMainWindow->portBits,
		  		  &gOffGP.portRect,  &gMainWindow->portRect,
		  		  srcCopy,           NULL);
		EndUpdate(gMainWindow);
		SetPort(savedPort);
	}
	else // must be the select window
	{
		Print("select update");
		BeginUpdate(gSelectWindow);
		EndUpdate(gSelectWindow);
	}
} // DoUpdate()


static void Init(void)
{
	register int i;
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs( (ProcPtr)0L);
	InitCursor();
	for (i=0; i<7; i++) (void)MoreMasters();	
	FlushEvents(everyEvent, 0);
	SetEventMask(everyEvent);
} // Init()


static void SetupMenus(void)
{
	Handle mbarH;
	
	mbarH = (Handle)GetNewMBar(kMenuBar);
	if (mbarH == NULL) {
		Error( kFetchMenuErr, 0, 0, 0, stopIcon);
		ExitToShell();
	}
	SetMenuBar( mbarH);
	
	gAppleM= GetMenu(kAppleM);
	gFileM = GetMenu(kFileM);
	gEditM = GetMenu(kEditM);
	gTypeM = GetMenu(kTypeM);
	
	AddResMenu( gAppleM, 'DRVR');
	AddResMenu( gTypeM, kMorphECR); // Add external code resources
	DrawMenuBar();
} // SetupMenus()


static void SetupWindows(void)
{
	gControlDialog = GetNewDialog(kControlDLOG, NULL, (DialogPtr)-1L);
	gMainWindow    = GetNewWindow(kMainWIND, NULL, (WindowPtr)-1L);
	gSelectWindow  = GetNewWindow(kSelectWIND, NULL, (WindowPtr)-1L);

	if ( (gControlDialog == NULL) ||
		 (gMainWindow == NULL) ||
		 (gSelectWindow == NULL) )
	{
		// Alert the user we couldn't get a window...
		Error( kFetchWINDErr, 0, 0, 0, stopIcon);
		ExitToShell();
	}

	WriteCoords();
	UserItem(gControlDialog, kZoomGroup, BoxItem);
	UserItem(gControlDialog, kRangeGroup, BoxItem);
	
	ShowWindow(gMainWindow);
	ShowWindow(gSelectWindow);   // used for status messages, too.
	SetPort(gSelectWindow);
	TextFont(monaco);
	TextSize(9);
	{ FontInfo finfo;
		GetFontInfo(&finfo);
		gPrintY = finfo.ascent + finfo.descent +
				  finfo.leading;
	}
	ShowWindow(gControlDialog);
	DrawDialog(gControlDialog);  // show the dialog items now.
	SelectWindow(gControlDialog);
	gCurrentWindow = gControlDialog; // which one is selected now.
} // SetupWindows()


void main(void)
{
	RgnHandle cursorRgn;
	Boolean gotEvent;
	EventRecord theEvent;
	
	Init(); // init mac toolbox
	SetupMenus();		// get menubar/menus going, install ECRs
	SetupWindows();		// load the windows, install user items, etc.
	SetupOffscreen();	// get offscreen buffer set up
	
	SetPort(gControlDialog);
	
#ifdef DEBUG
	DebugStr("\pmain, before GetCursor()");
#endif
	gCrossCursor = NULL;
	gCrossCursor = GetCursor( crossCursor);  // load "plus" cursor
	
	gDefaultProcP = Default;		// Built-in function
	gDefaultProcH = &gDefaultProcP;	// simulate a handle.  be careful.
	gMorphProcH = gDefaultProcH;	// set the global to use
	CheckItem( gTypeM, kTypeDefaultItem, TRUE);
	gTypeChkItem = kTypeDefaultItem;
	
	cursorRgn = NewRgn();			// create an empty region
	do
	{
		gotEvent = WaitNextEvent( everyEvent, &theEvent,
								  gSleepTime, cursorRgn);
					
		if (gotEvent)
			DoEvent(&theEvent);
		else
			DoIdle(&theEvent, cursorRgn);  // allow dlog text areas to blink
	} while (1);
} // main()


