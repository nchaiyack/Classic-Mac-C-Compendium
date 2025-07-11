/*******************************************************************************

	PPPop
	Version: 1.2.1
	Date: June 16, 1995
	Author: Rob Friefeld  (friefeld@earthlink.net)
	
	PPPop is a MacPPP 2.01 operator application. It puts up a window with a control
	button. Clicking the button toggles the state of the PPP connection. PPPop also
	puts up a session timer in a separate window.
	
	Think C 6.0 code. Based on Macintosh Programming Secrets framework.

	This program uses source code from MacPPP 2.01, � Merit Network. It will not
	compile unless prototype checking is set to "infer".
	
*******************************************************************************/


#include "PPPop.h"
#include "PPPcontrol.h"
#include "SoundPlay.h"
#include "LaunchFile.h"
#include "Timer.h"
#include "TimerDlg.h"
#include "Prefs.h"
#include "PositionWindow.h"
#include <GestaltEqu.h>
#include <Traps.h>
#include <Resources.h>
#include <Files.h>
#include <Folders.h>
#include <AppleEvents.h>


/*******************************************************************************

	Global variables
	
*******************************************************************************/

Boolean		gHasColorQD;
Boolean		gQuit;
Boolean		gInBackground;		
Boolean		gWillHardClose;		//	If FALSE, then PPP will soft close
Boolean		gSoundOn;
Boolean		gReturnToFinder;
Boolean		gLaunchOK[3];		// Used to enable and disable specific menu items

short		gPPPState;			// PPP up or down

WindowPtr	gAppWindow;			// The window with the button
ControlHandle gControl;			// The button's control

WindowPtr	gTimerWindow;		// The window with the timer
long		gTimerResolution;	// ticks since last update
Boolean		gAutoPositionTimer;


/*******************************************************************************

	main

*******************************************************************************/
void main()
{

	SetUpHeap();
	InitToolbox();
	AEInit();						/* Install required Apple Event handlers */
	InitProgram();					/* Program specific initialization */
	MainEventLoop();
}


/*******************************************************************************

	SetUpHeap

*******************************************************************************/
void SetUpHeap()
{
#define	kXStack		0x1000		//4k extra
#define kXMasters	2

	short	i;

	SetApplLimit(GetApplLimit() - kXStack);
	MaxApplZone();
	for (i = 1; i <= kXMasters; i++)
		MoreMasters();
}


/*******************************************************************************

	InitToolbox

*******************************************************************************/
void InitToolbox()
{
#define kMinHeap	(40 * 1024)		// If user set size < 40k, don't try to run

	Handle		menuBar;
	OSErr		err;
	long		feature;
	EventRecord	event;
	short		count;
	
	gInBackground = FALSE;
	gQuit = FALSE;

	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NIL);
	InitCursor();
	
	for (count == 1; count <= 3; count++)
		EventAvail(everyEvent, &event);
		
	if ((long)GetApplLimit() - (long)ApplicZone() < kMinHeap)
		DeathAlert(errNoMemory);


	err = Gestalt(gestaltQuickdrawFeatures, &feature);
	gHasColorQD = ((err == noErr) && (BitTst(&feature, 31 - gestaltHasColor)));
				
	err = Gestalt(gestaltROMSize, &feature);
	if (err != noErr  || feature < 128000)
		DeathAlert(errWimpyROMs);
	
	err = Gestalt(gestaltSystemVersion, &feature);
	if (err != noErr || feature < 0x0607)		// need 6.07 for sound manager
		DeathAlert(errWimpySystem);
				
	if (!TrapExists(_WaitNextEvent))
		DeathAlert(errWeirdSystem);

	menuBar = GetNewMBar(rMenuBar);			/* Read menus into menu bar */
	if ( menuBar == NIL )
		 DeathAlert(errNoMenuBar);
	SetMenuBar(menuBar);					/* Install menus */
	DisposHandle(menuBar);
	AddResMenu(GetMHandle(mApple), 'DRVR');	/* Add DA names to Apple menu */
	DrawMenuBar();
}


/*******************************************************************************

	Apple Event Routines

*******************************************************************************/
void AEInit()
{
	OSErr	err;
	long	feature;
	
	err = Gestalt(gestaltAppleEventsAttr, &feature);
	
	if (err != noErr)
		return;
		
	if ( !(feature & (kGestaltMask << gestaltAppleEventsPresent)))
		return;
		
	AEInstallHandlers();
}


/*******************************************************************************/
void AEInstallHandlers()
{
	OSErr	err;
	
	err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
				(AEEventHandlerUPP)&DoOpenApp, 0L, false);
	
	err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
				(AEEventHandlerUPP)&DoOpenDoc, 0L, false);

	err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
				(AEEventHandlerUPP)&DoPrintDoc, 0L, false);

	err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
				(AEEventHandlerUPP)&DoQuitApp, 0L, false);
}


/* Event Handlers */

pascal OSErr DoOpenApp(AppleEvent *event, AppleEvent *reply, long refcon)
{
	return noErr;
}

pascal OSErr DoOpenDoc(AppleEvent *event, AppleEvent *reply, long refcon)
{
	return noErr;
}

pascal OSErr DoPrintDoc(AppleEvent *event, AppleEvent *reply, long refcon)
{
	return noErr;
}

pascal OSErr DoQuitApp(AppleEvent *event, AppleEvent *reply, long refcon)
{
	DoQuit();
	return noErr;
}

			
/*******************************************************************************

	PPPop Routines

*******************************************************************************/


void InitProgram()
{	
	WindowPtr		window;
	Ptr				wStorage;
	
	if (pppup() == -1) {
		DoAboutBox();
		DeathAlert(errNoPPP);			// No point in running if no PPP
	}
		
	wStorage = NewPtr(sizeof(WindowRecord));
	if (wStorage == NIL)
		DeathAlert(errNoMemory);
	if (gHasColorQD) {
		if ((window = GetNewCWindow(rWindowID,wStorage,(WindowPtr)-1L)) == NIL)
			DeathAlert(errNoResource);
	}
	else 
		if ((window = GetNewWindow(rWindowID,wStorage,(WindowPtr)-1L)) == NIL)
			DeathAlert(errNoResource);
			
	if ((gControl = GetNewControl(rControlID, window)) == NIL)
		DeathAlert(errNoResource);
		
	gAppWindow = window;
	
	wStorage = NewPtr(sizeof(WindowRecord));
	if (wStorage == NIL)
		DeathAlert(errNoMemory);	
	if ((window = GetNewWindow(rTimerWindID, wStorage, (WindowPtr)-1L)) == NIL)
		DeathAlert(errNoResource);
	gTimerWindow = window;
	
		
	SetUpLaunch();		// Setup cdev launching
					
	DoReadPrefs();
	DoOpenSoundFiles();
	
	AdjustWindowPosition(gAppWindow);	// Verify position is on current monitor
	SetState(gAppWindow);				// Icon for PPP up or PPP down
	ShowWindow(gAppWindow);
	
	if (gAutoPositionTimer)
		AutoPositionTimer();
	else
		AdjustWindowPosition(gTimerWindow);
		
	SetupTimerWindow();					// Set font, size, etc.
	InitTimer();
	AutoReset();
	gTimerResolution = gShowCurrent ? 60 : 600;   // Total time doesn't need 1 sec resolution
	gCounting = gPPPState;		// If PPP up, we are counting time.
	if (gCounting)
		ResetStartTime();
	if (gTimerOn) 
		ShowWindow(gTimerWindow);
}


/*******************************************************************************/
void DoQuit()
{
	gQuit = TRUE;
	CleanUp();
}


/*******************************************************************************/
void	DoAboutBox(void)
{
	short		itemHit;
	DialogPtr	about;
	short		depth, resNum;

	depth = (**((**GetMainDevice()).gdPMap)).pixelSize;
  	resNum = (depth < 4) ? rAboutBoxID_BW : rAboutBoxID;

	about = GetNewDialog(resNum, nil, (WindowPtr) -1);
	
	if (about == nil) {
		SysBeep(5);
		return;
	}

	ShowWindow(about);
	ModalDialog(nil, &itemHit);
	DisposeDialog(about);
}



/*******************************************************************************/
// Tell PICTButton CDEF which icon to display
void SetState(WindowPtr window)
{
	GrafPtr			oldPort;
	
	if (window == NIL) return;
	
	GetPort(&oldPort);
	SetPort(window);
	
		
	gPPPState = pppup();	// 0 if PPPdown, 1 if PPPup

	gPPPState ? SetCtlValue(gControl, rUpID) : SetCtlValue(gControl, rDownID);
	SetPort(oldPort);
}


/*******************************************************************************/
void CleanUp()
{
	DoSavePrefs();
}

/*******************************************************************************/
Point GetWindowPosition(WindowPtr window)
{
	Point		theTopLeft = {0,0};
	GrafPtr		savePort;
	
	if (window != NIL) {
		GetPort(&savePort);
		SetPort(window);
		LocalToGlobal(&theTopLeft);
		SetPort(savePort);
	}
	return theTopLeft;
}

/*******************************************************************************/
void GetWindowSize(WindowPtr window, long *windWidth, long *windHeight)
{
	*windWidth = *windHeight = 0;
	
	if (window != NIL) {
	*windWidth = window->portRect.right - window->portRect.left;
	*windHeight = window->portRect.bottom - window->portRect.top;
	}
}


/*******************************************************************************/
void AutoPositionTimer()
{
	Point	mainPos, timerPos;
	long	width1, height1, width2, height2;
	short	offset = 5;
	

	if (gTimerWindow == nil) return;
	HideWindow(gTimerWindow);
	if (!gTimerOn) return;
	
	mainPos = GetWindowPosition(gAppWindow);
	GetWindowSize(gAppWindow, &width1, &height1); 
	GetWindowSize(gTimerWindow, &width2, &height2);	
	
	timerPos.h = mainPos.h - ((width2 - width1)/2);
	timerPos.v = mainPos.v + height1 + offset;
	
	MoveWindow(gTimerWindow,timerPos.h, timerPos.v, FALSE);
	AdjustWindowPosition(gTimerWindow);
	ShowWindow(gTimerWindow);
}


/*******************************************************************************/
void OperatePPP(short value)
{
	short		pppState;

	pppState = pppup();

	if ( (pppState == 0) && (value == rUpID) ) {
		if (pppopen() != noErr)
			DeathAlert(errPPPOpen);
	}

	else if ( (pppState == 1) && (value == rDownID) ) {
		DoSavePrefs();			// keep total time, in case something crashes
		if ( pppclose((short)gWillHardClose) != noErr)
			DeathAlert(errPPPClose);
	}
	
	if (gReturnToFinder)
		(void) ScheduleFinder();
}


/*******************************************************************************/
// These routines are separated out to facilitate scriptability later

void OpenPPP()
{	
	OperatePPP(rUpID);
}

void ClosePPP()
{
	OperatePPP(rDownID);
}


/*******************************************************************************/
void IdleJobs()
{
	if (pppup() != gPPPState) {		// We are out of phase with icon displayed
		SetState(gAppWindow);
		if (gPPPState)				// We have gone UP
			ResetStartTime();
		ShowTime();
		if (gSoundOn)
			gPPPState ? PlayUp() : PlayDown();
	}
	else 
		if ( gCounting && ((TickCount() - gSessionTime) > gTimerResolution))
			ShowTime();
}

/*******************************************************************************

	MainEventLoop

*******************************************************************************/
void MainEventLoop()
{
	Boolean		gotEvent;
	EventRecord	event;
	Point		mouse;
	long		sleep;
	long		frontSleep = 12L;
	long		backSleep = 12L;

	
	while ( !gQuit ) {
	
//		SetPort(gAppWindow);
		
		gInBackground ? (sleep = backSleep) : (sleep = frontSleep);
		
		gotEvent = WaitNextEvent(everyEvent, &event, sleep, NIL);

		if (pppup()) {
			if (!gCounting)
				ResetStartTime();
			gCounting = TRUE;
			ElapsedTime();
		}
		else {
			gCounting = FALSE;
			AutoReset();
		}

		if ( gotEvent ) 
			HandleEvent(&event);
		else 
			IdleJobs();		// This must be done in background, also
	}
}


/*******************************************************************************

	HandleEvent

*******************************************************************************/
void HandleEvent(EventRecord *event)
{
	switch ( event->what ) {
		case mouseDown:
			HandleMouseDown(event);
			break;
		case keyDown:
		case autoKey:
			HandleKeyPress(event);
			break;
		case activateEvt:
			HandleActivate(event);
			break;
		case updateEvt:
			HandleUpdate(event);
			break;
		case diskEvt:
			HandleDiskInsert(event);
			break;
		case kHighLevelEvent:
			AEProcessAppleEvent(event);
			break;
		case osEvt:
			HandleOSEvent(event);
			break;
	}
}


/*******************************************************************************

	HandleActivate

*******************************************************************************/
void HandleActivate(EventRecord *event)
{
	WindowPtr	window;
	Boolean		becomingActive;

	window = (WindowPtr) event->message;
	becomingActive = (event->modifiers & activeFlag) != 0;
	if ( IsAppWindow(window) ) {
		if (becomingActive) {
			SetPort(window);
			SetCursor(&qd.arrow);
		}
	}
}


/*******************************************************************************

	HandleDiskInsert

*******************************************************************************/
void HandleDiskInsert(EventRecord *event)
{
	Point	aPoint = {100, 100};

	if ( HiWord(event->message) != noErr ) {
		(void) DIBadMount(aPoint, event->message);
	}
}


/*******************************************************************************

	HandleKeyPress

*******************************************************************************/
void HandleKeyPress(EventRecord *event)
{
	char	key;

	key = event->message & charCodeMask;
	if ( event->modifiers & cmdKey ) {		/* Command key down? */
		AdjustMenus();						/* Enable/disable/check menu items properly */
		HandleMenuCommand(MenuKey(key));
	} else {
		DoKeyPress(event);
	}
}


/*******************************************************************************

	HandleMouseDown

*******************************************************************************/
void HandleMouseDown(EventRecord *event)
{
	WindowPtr	window;
	short		part = FindWindow(event->where, &window);
	Rect		limitRect;

	switch ( part ) {
		case inMenuBar:				/* Process a mouse menu command (if any) */
			AdjustMenus();
			HandleMenuCommand(MenuSelect(event->where));
			break;
		case inSysWindow:			/* Let the system handle the mouseDown */
			SystemClick(event, window);
			break;
		case inContent:
			if ( window != FrontWindow() )  {
				SelectWindow(window);
				if (window == gTimerWindow)
					DoContentClick(event, window);
			}
			else
				DoContentClick(event, window);
			break;
		case inDrag:
			DragWindow(window, event->where, &qd.screenBits.bounds);
			if (window == gAppWindow && gAutoPositionTimer)
				AutoPositionTimer();
			break;
		case inGoAway:
			if (TrackGoAway(window, event->where))  {
				CloseAnyWindow(window);
			}
			break;
	}
}

/*******************************************************************************

	DoContentClick

*******************************************************************************/
void	DoContentClick(EventRecord *event, WindowPtr window)
{
	short			value;
	ControlHandle	whichControl;
	Point			oldPlace, newPlace;	

	SetPort(window);
	
	if (window == gAppWindow) {
	
		if (IsDoubleClick(event)) return;
		
		GlobalToLocal(&(event->where));
		if (FindControl(event->where, window, &whichControl)) {
			if (gSoundOn)
				Click(rClick1);
			if (TrackControl(whichControl, event->where, NIL)) {
				if (gSoundOn)
					Click(rClick2);
				value = (pppup() == 0) ? rUpID : rDownID;
				OperatePPP(value);
			}
		}
	}
	else 
	if (window == gTimerWindow) {
	
		oldPlace = GetWindowPosition(window);
		DragWindow(window, event->where, &qd.screenBits.bounds);
		newPlace = GetWindowPosition(window);
		
		if ((newPlace.h == oldPlace.h) && (newPlace.v = oldPlace.v))
			FlipTimeDisplay();		// user clicked in window, but didn't move it
	}
}


// Not rigorous, but good enough for this use
Boolean IsDoubleClick(EventRecord *event)
{
	static	Point	lastWhere = {0,0};
	static	long	lastWhen = 0;
	Boolean			result = false;
			
	if (( (event->when - lastWhen) < LMGetDoubleTime())
			&& (abs(event->where.h - lastWhere.h) < 5)
			&& (abs(event->where.v - lastWhere.v) < 5))
		result = true;
		
	lastWhen = event->when;
	lastWhere = event->where;
	
	return result;
}

short abs(short num)
{
	if (num < 0)
		num = -num;
	return num;
}


/*******************************************************************************

	HandleOSEvent

*******************************************************************************/
void HandleOSEvent(EventRecord *event)
{
	switch ((event->message >> 24) & 0x00FF) {		/* High byte of message */
		case suspendResumeMessage:
				
			gInBackground = (event->message & resumeFlag) == 0;
			if (FrontWindow()) {
				/* DoActivateWindow(FrontWindow(), !gInBackground); */
			}
			break;
		case mouseMovedMessage:
			break;
	}
}


/*******************************************************************************

	HandleUpdate

*******************************************************************************/
void HandleUpdate(EventRecord *event)
{	
	GrafPtr		savePort;
	WindowPtr	window = (WindowPtr) event->message;
	
	if ( IsAppWindow(window) ) {
		GetPort(&savePort);
		SetPort(window);
		BeginUpdate(window);				/* This sets up the visRgn */
		if (!EmptyRgn(window->visRgn)) {	/* Draw if updating needs to be done */
			EraseRgn(window->visRgn);
			if (window == gAppWindow)
				UpdtControl(window, window->visRgn);
			if (window == gTimerWindow)
				ShowTime();
		}
		EndUpdate(window);
		SetPort(savePort);
	}
}


/*******************************************************************************

	AdjustMenus

*******************************************************************************/
void AdjustMenus()
{
	WindowPtr	window;
	MenuHandle	menu;
	Str255		theString;
	short		index;

	window = FrontWindow();	
	
	menu = GetMHandle(mFile);
	if (pppup()) {
		EnableItem(menu, iClosePPP);
		DisableItem(menu, iOpenPPP);
	} else {
		DisableItem(menu, iClosePPP);
		EnableItem(menu, iOpenPPP);
	}
	if (gLaunchOK[1])
		EnableItem(menu, iOpenConfigPPP);
	else
		DisableItem(menu, iOpenConfigPPP);
	if (gLaunchOK[2]) 
		EnableItem(menu, iOpenMacTCP);
	else 
		DisableItem(menu, iOpenMacTCP);
	
			
	index = gWillHardClose ? sHard : sSoft;	
	GetIndString(theString, rWillClose, index);
	menu = GetMHandle(mPrefs);
	SetItem(menu, iWillClose, theString);
	if ((gSoundFileUp == -1) && (gSoundFileDown == -1)) {
		DisableItem(menu, iSoundOn);
		CheckItem(menu, iSoundOn, FALSE);
		gSoundOn = FALSE;
	}
	else {
		EnableItem(menu, iSoundOn);
		CheckItem(menu, iSoundOn, gSoundOn);
	}

	if (gLaunchOK[0]) {
		EnableItem(menu, iReturnToFinder);
		CheckItem(menu, iReturnToFinder, gReturnToFinder);
	} 
	else {
		DisableItem(menu, iReturnToFinder);
		CheckItem(menu, iReturnToFinder, FALSE);
	}
	
	CheckItem(menu, iTimerOn, gTimerOn);
	if (gTimerOn) {
		EnableItem(menu, iTotalTime);
		EnableItem(menu, iAutoPosition);
		CheckItem(menu, iTotalTime, !gShowCurrent);
		CheckItem(menu, iAutoPosition, gAutoPositionTimer);
	}
	else {
		DisableItem(menu, iTotalTime);
		DisableItem(menu, iAutoPosition);
	}

	menu = GetMHandle(mEdit);
	if ( IsDAWindow(window) ) {		/* A desk accessory might need the edit menu� */
		EnableItem(menu, iUndo);
		EnableItem(menu, iCut);
		EnableItem(menu, iCopy);
		EnableItem(menu, iClear);
		EnableItem(menu, iPaste);
	} else {						/* � but we don�t use it. */
		DisableItem(menu, iUndo);
		DisableItem(menu, iCut);
		DisableItem(menu, iCopy);
		DisableItem(menu, iClear);
		DisableItem(menu, iPaste);
	}
}


/*******************************************************************************

	HandleMenuCommand

*******************************************************************************/
void HandleMenuCommand(menuResult)
	long		menuResult;
{
	short		menuID;				/* The resource ID of the selected menu */
	short		menuItem;			/* The item number of the selected menu */
	Str255		daName;

	menuID = HiWord(menuResult);
	menuItem = LoWord(menuResult);
	switch ( menuID ) {
		case mApple:
			switch ( menuItem ) {
				case iAbout:
					CollectGarbage();	// free up mem if needed
					DoAboutBox();
					break;
				default:			/* All non-About items in this menu are DAs */
					GetItem(GetMHandle(mApple), menuItem, daName);
					(void) OpenDeskAcc(daName);
					break;
			}
			break;
			
		case mFile:
			switch ( menuItem ) {
				case iOpenPPP:
					OpenPPP();
					break;
				case iClosePPP:
					ClosePPP();
					break;
				case iOpenConfigPPP:
					OpenConfigPPP();
					break;
				case iOpenMacTCP:
					OpenMacTCP();
					break;
				case iQuit:
					DoQuit();
					break;
			}
			break;
			
		case mEdit:
			switch (menuItem) {
				case iUndo:
				case iCut:
				case iCopy:
				case iPaste:
				case iClear:
					(void) SystemEdit(menuItem-1);
					break;
			}
			break;
			
		case mPrefs:
			switch (menuItem) {
				case iWillClose:
					gWillHardClose = !gWillHardClose;
					break;
				case iSoundOn:
					gSoundOn = !gSoundOn;
					break;
				case iReturnToFinder:
					gReturnToFinder = !gReturnToFinder;
					break;
				case iTimerOn:
					gTimerOn = !gTimerOn;
					if (gAutoPositionTimer)
						AutoPositionTimer();
					else
						AdjustTimerWindow();
					break;
				case iTotalTime:
					gShowCurrent = !gShowCurrent;
					gTimerResolution = gShowCurrent ? 60 : 600;
					ShowTime();
					break;
				case iAutoPosition:
					gAutoPositionTimer = !gAutoPositionTimer;
					if (gAutoPositionTimer)
						AutoPositionTimer();
					break;
				case iResetTime:
					DoTimeDlg();
					break;
			}
			break;
	}
	HiliteMenu(0);		/* Unhighlight what MenuSelect or MenuKey hilited */
}

/*******************************************************************************

	DoKeyPress

*******************************************************************************/
void DoKeyPress(EventRecord *event)
{
#define	kEnter	(char) 0x03
#define kReturn	(char) 0x0D


	char	key;

	key = event->message & charCodeMask;
	if (key == kEnter || key == kReturn)
		FlipTimeDisplay();
}



/*******************************************************************************

	CloseAnyWindow

*******************************************************************************/
void CloseAnyWindow(WindowPtr window)
{
	if (IsDAWindow(window)) {
		CloseDeskAcc( ( (WindowPeek) window )->windowKind );
	} else if (IsDialogWindow(window)) {
		HideWindow(window);
	} else if (IsAppWindow(window)) {
		DisposeWindow(window);
		DoQuit();
	}
}


/*******************************************************************************

	DeathAlert

*******************************************************************************/
void DeathAlert(short errNumber)
{
	short		itemHit;
	Str255		theMessage;

	SetCursor(&qd.arrow);
	GetIndString(theMessage, rErrorStrings, errNumber);
	ParamText(theMessage, NIL, NIL, NIL);
	itemHit = StopAlert(rErrorAlert, NIL);
	ExitToShell();
}


/*******************************************************************************

	IsAppWindow

*******************************************************************************/
Boolean IsAppWindow(WindowPtr window)
{
	short		windowKind;

	if ( window == NIL )
		return false;
	else {
		windowKind = ((WindowPeek) window)->windowKind;
		return ((windowKind >= userKind) || (windowKind == dialogKind));
	}
}


/*******************************************************************************

	IsDAWindow

*******************************************************************************/
Boolean IsDAWindow(WindowPtr window)
{
	if ( window == NIL )
		return false;
	else
		return ( ((WindowPeek) window)->windowKind < 0 );
}


/*******************************************************************************

	IsDialogWindow

*******************************************************************************/
Boolean IsDialogWindow(WindowPtr window)
{
	if ( window == NIL )
		return false;
	else
		return ( ((WindowPeek) window)->windowKind == dialogKind );
}


/*******************************************************************************

	TrapExists

*******************************************************************************/
Boolean	TrapExists(short theTrap)
{
	TrapType	theTrapType;

	theTrapType = GetTrapType(theTrap);
	if ((theTrapType == ToolTrap) && ((theTrap &= 0x07FF) >= NumToolboxTraps()))
		return false;
	else
		return (NGetTrapAddress(_Unimplemented, ToolTrap) !=
				NGetTrapAddress(theTrap, theTrapType));
}


/*******************************************************************************

	GetTrapType

*******************************************************************************/
TrapType	GetTrapType(short theTrap)
{
	if ((theTrap & 0x0800) == 0)					/* Per D.A. */
		return (OSTrap);
	else
		return (ToolTrap);
}


/*******************************************************************************

	NumToolboxTraps

*******************************************************************************/
short	NumToolboxTraps(void)
{
	if (NGetTrapAddress(0xA86E, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
		return (0x200);
	else
		return (0x400);
}


/*******************************************************************************/

void CollectGarbage(void)
{
	Size	avail;
		
	if (MaxBlock() < kBytesWanted) {
		avail = CompactMem((Size)kBytesWanted);
		if ((long)avail < kBytesWanted)
			PurgeMem((Size)kBytesWanted);
	}
}


