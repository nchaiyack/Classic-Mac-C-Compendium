#include "Standard Stuff.h"
#include "Traps.h"

/*******************************************************************************

	The �g� prefix is used to emphasize that a variable is global.

*******************************************************************************/

SysEnvRec	gMac;				/*	gMac is used to hold the result of a
									SysEnvirons call. This makes it convenient
									for any routine to check the environment. */

Boolean		gQuit;				/*	We set this to TRUE when the user selects
									Quit from the File menu. Our main event
									loop exists when gQuit is TRUE. */

Boolean		gInBackground;		/*	gInBackground is maintained by our osEvent
									handling routines. Any part of the program
									can check it to find out if it is currently
									in the background. */


/*******************************************************************************

	Define HiWrd and LoWrd macros for efficiency.

*******************************************************************************/

#define HiWrd(aLong)	(((aLong) >> 16) & 0xFFFF)
#define LoWrd(aLong)	((aLong) & 0xFFFF)


/*******************************************************************************

	main

	Entry point for our program. We initialize the toolbox, make sure we are
	running on a sufficiently studly machine, and put up the menubar. Finally,
	we start polling for events and handling them by entering our main event
	loop.

*******************************************************************************/
main()
{
	/*	If you have stack requirements that differ from the default,
		then you could use SetApplLimit to increase StackSpace at
		this point, before calling MaxApplZone. */

	MaxApplZone();					/* Expand the heap so code segments load
									   at the top */
	InitToolbox();					/* Initialize the program */
	MainEventLoop();				/* Call the main event loop */
}


/*******************************************************************************

	InitToolbox

	Set up the whole world, including global variables, Toolbox managers, and
	menus.

*******************************************************************************/
void InitToolbox()
{
	Handle		menuBar;
	EventRecord event;
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

	/*	This next bit of code waits until MultiFinder brings our application
		to the front. This gives us a better effect if we open a window at
		startup. */

	for (count = 1; count <= 3; ++count)
		EventAvail(everyEvent, &event);

	SysEnvirons(curSysEnvVers, &gMac);

	if (gMac.machineType < 0)
		DeathAlert(errWimpyROMs);

	if (gMac.systemVersion < 0x0600)
		DeathAlert(errWimpySystem);

	if (!TrapExists(_WaitNextEvent))
		DeathAlert(errWeirdSystem);

	menuBar = GetNewMBar(rMenuBar);			/* Read menus into menu bar */
	if ( menuBar == NIL )
		 DeathAlert(errNoMenuBar);
	SetMenuBar(menuBar);					/* Install menus */
	DisposHandle(menuBar);
	AddResMenu(GetMHandle(mApple), 'DRVR');	/* Add DA names to Apple menu */
	AdjustMenus();
	DrawMenuBar();
}


/*******************************************************************************

	MainEventLoop

	Get events forever, and handle them by calling HandleEvent. First, call
	DoAdjustCursor to set our cursor shape, and to set the cursor region. We
	then call WaitNextEvent() to get the event. This is OK, because we know
	we�re running on System 6.0 or later by this time. If we got an event, we
	handle it by calling HandleEvent(). But before doing that, we call
	DoAdjustCursor again in case our application had fallen asleep under
	MultiFinder.

*******************************************************************************/
void MainEventLoop()
{
	RgnHandle	cursorRgn;
	Boolean		gotEvent;
	EventRecord	event;
	Point		mouse;

	cursorRgn = NIL;
	while ( !gQuit ) {
		gotEvent = WaitNextEvent(everyEvent, &event, MAXLONG, cursorRgn);
		if ( gotEvent ) {
			HandleEvent(&event);
		}
	}
}


/*******************************************************************************

	HandleEvent

	Do the right thing for an event. Determine what kind of event it is, and
	call the appropriate routines.

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
		case osEvt:
			HandleOSEvent(event);
			break;
	}
}


/*******************************************************************************

	HandleActivate

	This is called when a window is activated or deactivated. In this sample,
	the Window Manager�s handling of activate and deactivate events is
	sufficient. Other applications may have TextEdit records, controls, lists,
	etc., to activate/deactivate.

*******************************************************************************/
void HandleActivate(EventRecord *event)
{
	WindowPtr	theWindow;
	Boolean		becomingActive;

	theWindow = (WindowPtr) event->message;
	becomingActive = (event->modifiers & activeFlag) != 0;
	if ( IsAppWindow(theWindow) ) {
		DrawGrowIcon(theWindow);
		/* DoActivateWindow(theWindow, becomingActive) */;
	}
}


/*******************************************************************************

	HandleDiskInsert

	Called when we get a disk inserted event. Check the upper word of the
	event message; if it�s non-zero, then a bad disk was inserted, and it
	needs to be formatted.

*******************************************************************************/
void HandleDiskInsert(EventRecord *event)
{
	Point	aPoint = {100, 100};

	if ( HiWrd(event->message) != noErr ) {
		(void) DIBadMount(aPoint, event->message);
	}
}


/*******************************************************************************

	HandleKeyPress

	The user pressed a key. What are you going to do about it?

*******************************************************************************/
void HandleKeyPress(EventRecord *event)
{
	char	key;

	key = event->message & charCodeMask;
	if ( event->modifiers & cmdKey ) {		/* Command key down? */
		AdjustMenus();						/* Enable/disable/check menu items properly */
		HandleMenuCommand(MenuKey(key));
	} else {
		/* DoKeyPress(event) */;
	}
}


/*******************************************************************************

	HandleMouseDown

	Called to handle mouse clicks. The user could have clicked anywhere, so
	let�s first find out where by calling FindWindow. That returns a number
	indicating where in the screen the mouse was clicked. �switch� on that
	number and call the appropriate routine.

*******************************************************************************/
void HandleMouseDown(EventRecord *event)
{
	long		newSize;
	Rect		growRect;
	WindowPtr	theWindow;
	short		part = FindWindow(event->where, &theWindow);

	switch ( part ) {
		case inMenuBar:				/* Process a mouse menu command (if any) */
			AdjustMenus();
			HandleMenuCommand(MenuSelect(event->where));
			break;
		case inSysWindow:			/* Let the system handle the mouseDown */
			SystemClick(event, theWindow);
			break;
		case inContent:
			if ( theWindow != FrontWindow() )
				SelectWindow(theWindow);
			else
				/* DoContentClick(event, theWindow) */;
			break;
		case inDrag:				/* Pass screenBits.bounds to get all gDevices */
			DragWindow(theWindow, event->where, &qd.screenBits.bounds);
			break;
		case inGrow:
			growRect = qd.screenBits.bounds;
			growRect.top = growRect.left = 80;		/* Arbitrary minimum size. */
			newSize = GrowWindow(theWindow, event->where, &growRect);
			if (newSize != 0) {
				InvalidateScrollbars(theWindow);
				SizeWindow(theWindow, LoWrd(newSize), HiWrd(newSize), TRUE);
				InvalidateScrollbars(theWindow);
			}
			break;
		case inGoAway:
			if (TrackGoAway(theWindow, event->where))  {
				CloseAnyWindow(theWindow);
			}
			break;
		case inZoomIn:
		case inZoomOut:
			if (TrackBox(theWindow, event->where, part)) {
				SetPort(theWindow);
				EraseRect(&theWindow->portRect);
				ZoomWindow(theWindow, part, TRUE);
				InvalRect(&theWindow->portRect);
			}
			break;
	}
}


/*******************************************************************************

	HandleOSEvent

	Deal with OSEvents (formerly, app4Events). These are message that
	MultiFinder -- known as the Process Manager under System 7.0 -- sends to
	us. Here, we deal with suspend and resume message.

*******************************************************************************/
void HandleOSEvent(EventRecord *event)
{
	switch ((event->message >> 24) & 0x00FF) {		/* High byte of message */
		case suspendResumeMessage:

			/*	In our SIZE resource, we say that we are MultiFinder aware.
				This means that we take on the responsibility of activating
				and deactivating our own windows on suspend/resume events. */

			gInBackground = (event->message & resumeFlag) == 0;
			if (FrontWindow()) {
				DrawGrowIcon(FrontWindow());
				/* DoActivateWindow(FrontWindow(), !gInBackground); */
			}
			break;
		case mouseMovedMessage:
			break;
	}
}


/*******************************************************************************

	HandleUpdate

	This is called when an update event is received for a window. It calls
	DoUpdateWindow to draw the contents of an application window. As an
	efficiency measure that does not have to be followed, it calls the drawing
	routine only if the visRgn is non-empty. This will handle situations where
	calculations for drawing or drawing itself is very time-consuming.

*******************************************************************************/
void HandleUpdate(EventRecord *event)
{
	WindowPtr	theWindow = (WindowPtr) event->message;
	if ( IsAppWindow(theWindow) ) {
		BeginUpdate(theWindow);				/* This sets up the visRgn */
		if (!EmptyRgn(theWindow->visRgn)) {	/* Draw if updating needs to be done */
			SetPort(theWindow);
			EraseRgn(theWindow->visRgn);
			/* DoUpdateWindow(event) */;
			DrawGrowIcon(theWindow);
		}
		EndUpdate(theWindow);
	}
}


/*******************************************************************************

	AdjustMenus

	Enable and disable menus based on the current state. The user can only
	select enabled menu items. We set up all the menu items before calling
	MenuSelect or MenuKey, since these are the only times that a menu item can
	be selected. Note that MenuSelect is also the only time the user will see
	menu items. This approach to deciding what enable/disable state a menu
	item has the advantage of concentrating all the decision-making in one
	routine, as opposed to being spread throughout the application. Other
	application designs may take a different approach that is just as valid.

*******************************************************************************/
void AdjustMenus()
{
	WindowPtr	window;
	MenuHandle	menu;

	window = FrontWindow();

	menu = GetMHandle(mFile);
	if ( window != NIL )
		EnableItem(menu, iClose);
	else
		DisableItem(menu, iClose);

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

	This is called when an item is chosen from the menu bar (after calling
	MenuSelect or MenuKey). It performs the right operation for each command.
	It is good to have both the result of MenuSelect and MenuKey go to one
	routine like this to keep everything organized.

*******************************************************************************/
void HandleMenuCommand(menuResult)
	long		menuResult;
{
	short		menuID;				/* The resource ID of the selected menu */
	short		menuItem;			/* The item number of the selected menu */
	Str255		daName;

	menuID = HiWrd(menuResult);
	menuItem = LoWrd(menuResult);
	switch ( menuID ) {
		case mApple:
			switch ( menuItem ) {
				case iAbout:
					(void) Alert(rAboutAlert, NIL);
					break;
				default:			/* All non-About items in this menu are DAs */
					GetItem(GetMHandle(mApple), menuItem, daName);
					(void) OpenDeskAcc(daName);
					break;
			}
			break;
		case mFile:
			switch ( menuItem ) {
				case iNew:
					/* DoNewWindow(); */
					break;
				case iClose:
					CloseAnyWindow(FrontWindow());
					break;
				case iQuit:
					gQuit = TRUE;
					break;
			}
			break;
		case mEdit:
			switch (menuItem) {
				/* Call SystemEdit for DA editing & MultiFinder */
				/* since we don�t do any Editing */
				case iUndo:
				case iCut:
				case iCopy:
				case iPaste:
				case iClear:
					(void) SystemEdit(menuItem-1);
					break;
			}
			break;
	}
	HiliteMenu(0);		/* Unhighlight what MenuSelect or MenuKey hilited */
}


/*******************************************************************************

	CloseAnyWindow

	Close the given window in a manner appropriate for that window. If the
	window belongs to a DA, we call CloseDeskAcc. For dialogs, we simply hide
	the window. If we had any document windows, we would probably call either
	DisposeWindow or CloseWindow after disposing of any document data and/or
	controls.

*******************************************************************************/
void CloseAnyWindow(WindowPtr window)
{
	if (IsDAWindow(window)) {
		CloseDeskAcc( ( (WindowPeek) window )->windowKind );
	} else if (IsDialogWindow(window)) {
		HideWindow(window);
	} else if (IsAppWindow(window)) {
		/* Do something significant for document windows. */
	}
}


/*******************************************************************************

	DeathAlert

	Display an alert that tells the user an error occurred, then exit the
	program. This routine is used as an ultimate bail-out for serious errors
	that prohibit the continuation of the application. The error number is
	used to index an 'STR#' resource so that a relevant message can be
	displayed.

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

	Check to see if a window belongs to the application. If the window pointer
	passed was NIL, then it could not be an application window. WindowKinds
	that are negative belong to the system and windowKinds less than userKind
	are reserved by Apple except for windowKinds equal to dialogKind, which
	mean it is a dialog.

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

	Check to see if a window belongs to a desk accessory. It belongs to a DA
	if the windowKind field of the window record is negative.

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

	Check to see if a window is a dialog window. We can determine this be
	checking to see if the windowKind field is equal to dialogKind.

*******************************************************************************/
Boolean IsDialogWindow(WindowPtr window)
{
	if ( window == NIL )
		return false;
	else
		return ( ((WindowPeek) window)->windowKind == dialogKind );
}


/*******************************************************************************

	InvalidateScrollbars

	Call InvalRect on the right and bottom edges of a window. This routine is
	called during the resizing of a window to take care of the scrollbar lines
	and grow icon.

*******************************************************************************/
void InvalidateScrollbars(WindowPtr theWindow)
{
	Rect	tempRect;

	SetPort(theWindow);

	tempRect = theWindow->portRect;
	tempRect.left = tempRect.right - 15;
	InvalRect(&tempRect);
	EraseRect(&tempRect);

	tempRect = theWindow->portRect;
	tempRect.top = tempRect.bottom - 15;
	InvalRect(&tempRect);
	EraseRect(&tempRect);
}


/*******************************************************************************

	TrapExists

	Check to see if a given trap is implemented. The recommended approach to
	see if a trap is implemented is to see if the address of the trap routine
	is the same as the address of the _Unimplemented trap. However, we must
	also make sure that the trap is contained in the trap table on the machine
	we�re running on. Not all Macintoshes have the same sized trap tables. We
	call NumToolboxTraps to find out the size of the table. If the trap we are
	examining falls off the end, then we treat it as automatically being
	unimplemented.

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

	Check the bits of a trap number to determine its type. If bit 11 is set,
	it�s a toolbox trap. Otherwise, it�s an OS trap.

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

	Find the size of the Toolbox trap table. This can be either 0x0200 or
	0x0400 bytes, depending on which Macintosh we are running on. We determine
	the size by taking advantage of an anomaly of the smaller trap table: any
	entries that fall beyond the end of the table are mirrored back down into
	the lower part. For example, on a large table, trap numbers A86E and AA6E
	correspond to different routines. However, on a small table, they
	correspond to the same routine. By checking the addresses of these
	routines, we can determine the size of the table.

*******************************************************************************/
short	NumToolboxTraps(void)
{
	if (NGetTrapAddress(0xA86E, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
		return (0x200);
	else
		return (0x400);
}
