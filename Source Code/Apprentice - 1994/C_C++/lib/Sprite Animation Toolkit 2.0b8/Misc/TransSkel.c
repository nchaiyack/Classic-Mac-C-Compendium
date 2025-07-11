/*
	TransSkel - Transportable application skeleton
	TransSkel is public domain.
	Version 2.6 of 8/17/90

	This version by:
			Bob Schumaker and Dan Hite
			The AMIX Corporation
			2345 Yale Street
			Palo Alto, CA 94306

	UUCP:	{sun, uunet}!markets![bob,dan]
	ARPA:	[bob,dan]@amix.com

	 Originally written by:
			Paul DuBois
			Wisconsin Regional Primate Research Center
			1220 Capital Court
			Madison WI  53706  USA

	UUCP:	{allegra,ihnp4,seismo}!uwvax!uwmacc!dubois
	ARPA:	dubois@unix.macc.wisc.edu
			dubois@rhesus.primate.wisc.edu
*/

/*
	The following symbol controls support for dialogs.
	Changing #define to #undef disables the support.
*/

#include "TransSkel.h"
//#include "TransSkull.h"
#ifdef applec
#include <Desk.h>
#include <DiskInit.h>
#include <Fonts.h>
#include <Memory.h>
#include <OSEvents.h>
#include <Resources.h>
#include <Script.h>
#include <ToolUtils.h>
#else
#define curSysEnvVers	1
#define zoomDocProc 8
#define	DragGrayRgnProcPtr	ProcPtr
#endif

/*
	New(TypeName) returns handle to new object, for any TypeName.
	If there is insufficient memory, the result is nil.
*/

#define	New(x)	(x **) NewHandle (sizeof (x))


/*
	Window and Menu handler types, constants, variables.

	whList and mhList are the lists of window and menu handlers.
	whClobOnRmve and mhClobOnRmve are true if the handler disposal proc
	is to be called when a handler is removed.  They are temporarily set
	false when handlers are installed for windows or menus that already
	have handlers - the old handler is removed WITHOUT calling the
	disposal proc.

	Default lower limits on window sizing of 80 pixels both directions is
	sufficient to allow text windows room to draw a grow box and scroll
	bars without having the thumb and arrows overlap.  These values may
	be changed if such a constraint is undesirable with SkelGrowBounds.
	Default upper limits are for the Macintosh, not the Lisa, but are set
	per machine in SkelInit.
*/

typedef struct WHandler
{
	WindowPtr	whWind;						/* window/dialog to be handled	*/
#ifdef applec
	void	(*whClobber)(void);				/* data structure disposal proc */
	void	(*whMouse)(Point, long, short);	/* mouse-click handler proc		*/
	void	(*whKey)(char, unsigned char, short);	/* key-click handler proc */
	void	(*whUpdate)(Boolean, short);	/* update handler proc			*/
	void	(*whActivate)(Boolean);			/* activate event handler proc	*/
	void	(*whClose)(void);				/* close "event" handler proc	*/
	void	(*whIdle)(void);				/* main loop proc				*/
#ifdef	supportDialogs
	Boolean	(*whEvent)(short *, EventRecord *);/* event hook				*/
	Boolean	(*whCheck)(short);				/* hit notify 					*/
#endif
#else
	void	(*whClobber)();		/* data structure disposal proc */
	void	(*whMouse)();		/* mouse-click handler proc		*/
	void	(*whKey)();			/* key-click handler proc		*/
	void	(*whUpdate)();		/* update handler proc			*/
	void	(*whActivate)();	/* activate event handler proc	*/
	void	(*whClose)();		/* close "event" handler proc	*/
	void	(*whIdle)();		/* main loop proc				*/
#ifdef	supportDialogs
	Boolean	(*whEvent)();		/* event hook					*/
	Boolean	(*whCheck)();		/* hit notify					*/
#endif
#endif
	Rect		whGrow;			/* limits on window sizing			*/
	RgnHandle	whCursorRgn;	/* cursor region when front window	*/
	CursHandle	whCursor;		/* cursor to show while in region	*/
	long		userReference;	/* a user specified longword		*/
	short		whSizedMods;	/* keys held down while window was resized */
#ifdef supportDialogs
	short		whItemHit;		/* the last item hit in this dialog */
	Boolean		whModal;		/* treat this as a modal dialog?	*/
#endif
	Boolean		whCanGrow;		/* this window *can* be grown!!		*/
	Boolean		whSized;		/* true = window was resized		*/
	Boolean		whFrontOnly;	/* true = idle only when active		*/
	Boolean		whHaveCursor;	/* cursor associated with this window */
	struct WHandler	**whNext;	/* next window handler				*/
} WHandler;

static WHandler	**whList = nil;
static Boolean	whClobOnRmve = true;
static Rect		growRect;
static short mBarHeight;	/* menu bar height.  All window sizing
							  code takes this into account */

static RgnHandle cursorRgn;		/* use this to manage the cursor */
static RgnHandle screensRgn;
static Boolean useWaitNextEvent;
static SysEnvRec skelEnvRec;

typedef struct MHandler
{
	short			mhID;						/* menu id                     */
#ifdef applec
	void			(*mhSelect)(short);			/* item selection handler proc */
	void			(*mhClobber)(MenuHandle);	/* menu disposal handler proc  */
#else
	void			(*mhSelect)();			/* item selection handler proc */
	void			(*mhClobber)();			/* menu disposal handler proc  */
#endif
	struct MHandler	**mhNext;					/* next menu handler           */
} MHandler;


static MHandler	**mhList = nil;			/* list of menu handlers */
static Boolean	mhClobOnRmve = true;


/*
	Variables for default Apple menu handler.  appleID is set to 1 if
	SkelApple is called and is the id of the Apple menu, appleAboutProc
	is the procedure to execute if there is an About... item and it's
	chosen from the Apple menu.  If doAbout is true, then the menu
	contains the About... item, otherwise it's just desk accessories.
*/

static MenuHandle	appleMenu;
static short		appleID = 0;
#ifdef applec
static void			(*appleAboutProc)(void) = nil;
static void			(*appleHelpProc)(void) = nil;
#else
static void			(*appleAboutProc)() = nil;
static void			(*appleHelpProc)() = nil;
#endif
static Boolean		doAbout = false;
static Boolean		doHelp = false;
/*
	Miscellaneous

	screenPort points to the window manager port.
	
	doneFlag determines when SkelMain returns.  It is set by calling
	SkelWhoa(), which the host does to request a halt.

	pBkgnd points to a background procedure, to be run during event
	processing.  Set it with SkelBackground.  If nil, there's no
	procedure.

	pEvent points to an event-inspecting hook, to be run whenever an
	event occurs.  Set it with SkelEventHook.  If nil, there's no
	procedure.

	eventMask controls the event types requested in the GetNextEvent
	call in SkelMain.

	diskInitPt is the location at which the disk initialization dialog
	appears, if an uninitialized disk is inserted.
*/

static GrafPtr	screenPort;
static Boolean	doneFlag = false;

#ifdef applec
static Boolean	(*pEvent)(EventRecord *) = nil;
static void		(*pBkgnd)(void) = nil;
#else
static Boolean	(*pEvent)() = nil;
static void		(*pBkgnd)() = nil;
#endif
static long 	pFGTime = 6L;			/* tenth of a second */
static long		pBGTime = 300L;			/* 5 seconds */
static Boolean	fgApp = true;			/* handle suspend/resume events */

static short	eventMask = everyEvent ^ keyUpMask;
static Point	diskInitPt = { /* v = */ 120, /* h = */ 100 };

#ifdef applec
static void (*pScrapConvert)(Boolean) = nil;
static void (*pSuspendResume)(Boolean) = nil;
static void (*pZoomProc)(WindowPtr, short, short, Boolean) = nil;
static void (*pNetworkProc)(EventRecord *) = nil;
static void (*pChildDied)(EventRecord *) = nil;
static void (*pHandleApp4)(EventRecord *) = nil;
#else
static void (*pScrapConvert)() = nil;
static void (*pSuspendResume)() = nil;
static void (*pZoomProc)() = nil;
static void (*pNetworkProc)() = nil;
static void (*pChildDied)() = nil;
static void (*pHandleApp4)() = nil;
#endif

#ifdef	supportDialogs

/*
	Events that are passed to dialogs.  Others are ignored.
	Standard mask passes , mousedown, keydown, autokey, update,
	activate and null events.  Null events are controlled by bit 0.
*/

static short	dlogEventMask = 0x16b;

#endif

/* -------------------------------------------------------------------- */
/*						Internal (private) Routines						*/
/* -------------------------------------------------------------------- */

/*
	Get handler associated with user or dialog window.
	Return nil if window doesn't belong to any known handler.
	This routine is absolutely fundamental to TransSkel.
*/

static WHandler	**GetWDHandler(WindowPtr theWind)
{
	register WHandler	**h;

	if (theWind) {
		for (h = whList; h != nil; h = (**h).whNext) {
			if ((**h).whWind == theWind) {
				return (h);
			}
		}
	}
	return (nil);
}


/*
	Get handler associated with user window.
	Return nil if window doesn't belong to any known handler.
	The order of the two tests is critical:  theWind might be nil.
*/

static WHandler	**GetWHandler(WindowPtr theWind)
{
	register WHandler	**h;

	if ((h = GetWDHandler (theWind)) != nil
		&& ((WindowPeek) theWind)->windowKind != dialogKind) {
		return (h);
	}
	return (nil);
}

#ifdef	supportDialogs

/*
	Get handler associated with dialog window.
	Return nil if window doesn't belong to any known handler.
	The order of the two tests is critical:  theDialog might be nil.
*/

static WHandler	**GetDHandler(DialogPtr theDialog)
{
	register WHandler	**h;

	if ((h = GetWDHandler (theDialog)) != nil
		&& ((WindowPeek) theDialog)->windowKind == dialogKind)
	{
			return (h);
	}
	return (nil);
}

#endif

static GetPortBounds(WindowPeek winPeek, Rect *bounds)
{
	if ((winPeek->port.portBits.rowBytes & 0xC000) == 0xC000) {
		/* it's a color window!! */
		*bounds = (**((CGrafPtr)(&winPeek->port))->portPixMap).bounds;
	}
	else {
		*bounds = winPeek->port.portBits.bounds;
	}
}

static WHandler **SkelNullHandler(WindowPtr theWin)
{
	register WHandler	**h;
	if (h = New (WHandler)) 
	{
		(**h).whWind = theWin;
		(**h).whMouse = nil;
		(**h).whKey = nil;
		(**h).whUpdate = nil;
		(**h).whActivate = nil;
		(**h).whClose = nil;
		(**h).whClobber = nil;
		(**h).whIdle = nil;
		(**h).whFrontOnly = true;
		(**h).whSized = false;
		(**h).whSizedMods = 0;
		(**h).whGrow = growRect;
		(**h).whCanGrow = true;
		(**h).whHaveCursor = false;
		(**h).whCursorRgn = (RgnHandle) 0;
		(**h).whCursor = (CursHandle) 0;
		(**h).userReference = 0L;
#ifdef supportDialogs
		(**h).whEvent = nil;
		(**h).whCheck = nil;
		(**h).whItemHit = 0;
		(**h).whModal = false;
#endif
		/* add this into the window list! */
		(**h).whNext = whList;
		whList = h;
	}
	return (h);
}

void SkelDragWindow (WindowPtr theWin, Point pt, short mods, Rect *bounds)
{
	if (!(mods & cmdKey)) {
		SelectWindow (theWin);
		SetPort (theWin);
		if (!StillDown ())
			return;
	}
	DragWindow (theWin, pt, bounds);
}

/*
	Remove a menu handler.  This calls the handler's disposal routine
	and then takes the handler out of the handler list and disposes
	of it.

	Note that the menu MUST be deleted from the menu bar before calling
	the clobber proc, because the menu bar will end up filled with
	garbage if the menu was allocated with NewMenu (see discussion of
	DisposeMenu in Menu Manager section of Inside Macintosh).

	The menu bar is only redrawn if redrawBar is true--this removes
	the flicker from the menu bar when the application terminates.
*/

static void RemoveMenu(MenuHandle theMenu, Boolean redrawBar)
{
	register short		mID;
	register MHandler	**h, **h2;
#ifdef applec
	register void		(*p)(MenuHandle);
#else
	register void		(*p)();
#endif

	mID = (**theMenu).menuID;
	/* if list empty, ignore */
	if (mhList != nil) {
		/* is it the first element? */
		if ((**mhList).mhID == mID) {
			h2 = mhList;
			mhList = (**mhList).mhNext;
		}
		else {
			for (h = mhList; h != nil; h = h2) {
				h2 = (**h).mhNext;
				if (h2 == nil)
					return;						/* menu not in list! */
				if ((**h2).mhID == mID) {
					/* found it */
					(**h).mhNext = (**h2).mhNext;
					break;
				}
			}
		}
		DeleteMenu (mID);
		if (redrawBar) {
			DrawMenuBar ();
		}
		if (mhClobOnRmve && (p = (**h2).mhClobber) != nil) {
			(*p) (theMenu);						/* call disposal routine */
		}
		DisposHandle ((Handle) h2);				/* get rid of handler record */
	}
}

/*
	General menu-handler.  Just passes selection to the handler's
	select routine.  If the select routine is nil, selecting items from
	the menu is a nop.
*/

static void DoMenuCommand(long command)
{
	register short		menu;
	register short		item;
	register MHandler	**mh;
#ifdef applec
	register void		(*p)(short);
#else
	register void		(*p)();
#endif

	menu = HiWord (command);
	item = LoWord (command);
	for (mh = mhList; mh != nil; mh = (**mh).mhNext) {
		if ((menu == (**mh).mhID) && ((p = (**mh).mhSelect) != nil)) {
			(*p) (item);
			break;
		}
	}
	HiliteMenu (0);		/* command done, turn off menu hiliting */
}


/*
	Apple menu handler
	
	DoAppleItem:  If the first item was chosen, and there's an "About..."
	item, call the procedure associated with it (if not nil).  If there
	is no "About..." item or the item was not the first one, then open
	the associated desk accessory.  The port is saved and restored
	because OpenDeskAcc does not always preserve it correctly.
	
	DoAppleClobber disposes of the Apple menu.
*/

static void DoAppleItem(short item)
{
	GrafPtr		curPort;
	Str255		str;

	if (doAbout && item == 1)
	{
		if (appleAboutProc != nil)
			(*appleAboutProc) ();
	}
	else 
		if (doHelp && item == 2) 
		{
			if (appleHelpProc != nil) 
			{
				(*appleHelpProc) ();
			}
		}
		else
		{
			GetPort (&curPort);
			GetItem (appleMenu, item, str);		/* get DA name */
			(void) OpenDeskAcc (str);			/* open it */
			SetPort (curPort);
		}
}

static void DoAppleClobber ()
{
	DisposeMenu (appleMenu);
}


/* -------------------------------------------------------------------- */
/*						Window-handler routing routines					*/
/*																		*/
/*	Each routine sets the port to the handler's window before executing	*/
/*	the handler procedure.												*/
/* -------------------------------------------------------------------- */


/*
	Pass local mouse coordinates, click time, and the modifiers flag
	word to the handler.
*/

static void DoMouse(WHandler **h, EventRecord *theEvent)
{
#ifdef applec
	register void	(*p)(Point, long, short);
#else
	register void	(*p)();
#endif
	Point			thePt;

	if (h != nil) {
		SetPort ((**h).whWind);
		if ((p = (**h).whMouse) != nil) {
			thePt = theEvent->where;	/* make local copy */
			GlobalToLocal (&thePt);
			(*p) (thePt, theEvent->when, theEvent->modifiers);
		}
	}
}


/*
	Pass the character and the modifiers flag word to the handler.
*/

static void DoKey(WHandler **h, char ch, unsigned char code, short mods)
{
#ifdef applec
	register void (*p)(char, unsigned char, short);
#else
	register void (*p)();
#endif
	if (h != nil) {
		SetPort ((**h).whWind);
		if ((p = (**h).whKey) != nil)
			(*p) (ch, code, mods);
	}
}


/*
	Call the window updating procedure, passing to it an indicator whether
	the window has been resized or not.  Then clear the flag, assuming
	the update proc took whatever action was necessary to respond to
	resizing.

	If the handler doesn't have any update proc, the Begin/EndUpdate
	stuff is still done, to clear the update region.  Otherwise the
	Window Manager will keep generating update events for the window,
	stalling updates of other windows.

	Make sure to save and restore the port, as it's not always the
	active window that is updated.
*/

static void DoUpdate (WHandler	**h)
{
#ifdef applec
	register void		(*p)(Boolean, short);
#else
	register void		(*p)();
#endif
	register GrafPtr	updPort;
	GrafPtr				tmpPort;

	if (h != nil) {
		GetPort (&tmpPort);
		SetPort (updPort = (**h).whWind);
		BeginUpdate (updPort);
		if ((p = (**h).whUpdate) != nil) {
			(*p) ((**h).whSized, (**h).whSizedMods);
			(**h).whSized = false;
			(**h).whSizedMods = 0;
		}
		EndUpdate (updPort);
		SetPort (tmpPort);
	}
}


/*
	Pass activate/deactivate notification to handler.
*/

static void DoActivate(WindowPtr theWin, Boolean active)
{
#ifdef applec
	register void (*p)(Boolean);
#else
	register void (*p)();
#endif
	WHandler	**h;

	if ((h = GetWDHandler (theWin)) != nil) {
		SetPort ((**h).whWind);
		if ((p = (**h).whActivate) != nil) {
			(*p) (active);
		}
		/* reset the cursor region */
		if (active) {
			SetEmptyRgn (cursorRgn);
		}
	}
}


/*
	Execute a window handler's close proc.  This may be used by handlers
	for temp windows that want to remove themselves when the window
	is closed:  they can call SkelRmveWind to dispose of the window
	and remove the handler from the window handler list.  Thus, windows
	may be dynamically created and destroyed without filling up the
	handler list with a bunch of invalid handlers.
	
	If the handler doesn't have a close proc, just hide the window.
	The host should provide some way of reopening the window (perhaps
	a menu selection).  Otherwise the window will be lost from user
	control if it is hidden, since it won't receive user events.

	The port is set to the window manager port after calling the
	handler proc, to avoid a dangling port.

	This is called both for regular and dialog windows.
*/

static void DoClose (WHandler **h)
{
#ifdef applec
	register void		(*p)(void);
#else
	register void		(*p)();
#endif
	GrafPtr savePort;

	if (h != nil) {
		if ((p = (**h).whClose) != nil) {
			GetPort (&savePort);
			SetPort ((**h).whWind);
			(*p) ();
			SetPort (savePort);
		}
		else {
			HideWindow ((**h).whWind);
		}
	}
}


/*
	Execute a window handler's clobber proc.

	The port is set to the window manager port after calling the
	handler proc, to avoid a dangling port.

	This is called both for regular and dialog windows.
*/

static void DoClobber (WHandler **h)
{
#ifdef applec
	register void (*p)(void);
#else
	register void (*p)();
#endif
	GrafPtr	tmpPort;

	GetPort (&tmpPort);
	SetPort ((**h).whWind);
	if ((p = (**h).whClobber) != nil)
		(*p) ();
	SetPort (tmpPort);
}


/*
	Execute handler's idle proc.

	Make sure to save and restore the port, since idle procs may be
	called for any window, not just the active one.
*/

static void DoIdle (WHandler **h)
{
#ifdef applec
	register void (*p)(void);
#else
	register void (*p)();
#endif
	GrafPtr			tmpPort;

	if (h != nil) 
	{
		if (!useWaitNextEvent) 
		{
			SystemTask ();
		}
		GetPort (&tmpPort);
		SetPort ((**h).whWind);
		if ((p = (**h).whIdle) != nil)
			(*p) ();
		SetPort (tmpPort);
	}
}


#ifdef	supportDialogs

/* -------------------------------------------------------------------- */
/*							Dialog-handling routines					*/
/* -------------------------------------------------------------------- */


/*
	Handle event if it's for a dialog.  The event must be one of
	those that is passed to dialogs according to dlogEventMask.
	This mask can be set so that disk-inserts, for instance, don't
	get eaten up.
*/

static Boolean DoDialog (EventRecord *evt)
{
	register WHandler	**dh;
	DialogPtr			theDialog;
	register short		what;
	short				item;
	Boolean				handledIt = false;
#ifdef applec
	Boolean				(*pKeys)(short *, EventRecord *);
#else
	Boolean				(*pKeys)();
#endif

	theDialog = (DialogPtr) FrontWindow ();
	if ((dh = GetDHandler (theDialog)) != nil) {
#ifdef applec
		pKeys = (Boolean (*)(short *, EventRecord *)) (**dh).whKey;
#else
		pKeys = (Boolean (*)()) (**dh).whKey;
#endif
	}
	else {
		pKeys = nil;
	}
	what = evt->what;
/* handle command keys before they get to IsDialogEvent */
	if ((what == keyDown || what == autoKey) && (evt->modifiers & cmdKey)) {
		if (!(pKeys && (handledIt = (*pKeys) (&item, evt)))) {
			if (!(dh && (**dh).whModal)) {
				DoMenuCommand (MenuKey (evt->message & charCodeMask));
			}
			return (true);
		}
	}
	else if (what == app4Evt || what == updateEvt) {
		/* do multifinder events, updates are handled in the main loop */
		if (dh && (**dh).whEvent) {
			handledIt = (*(**dh).whEvent)(&item, evt);
		}
		return (handledIt);		   	/* main section handles these */
	}
	else if (((1 << what) & dlogEventMask) && IsDialogEvent (evt)) {
		item = 0;
		switch (what) {
			case keyDown:
			case autoKey:
				if (!(pKeys && (handledIt = (*pKeys) (&item, evt)))) {
					handledIt = DialogSelect (evt, &theDialog, &item);
				}
				break;
			case activateEvt:						/* if activate */
				theDialog = (DialogPtr) evt->message;
				dh = GetDHandler ((WindowPtr) theDialog);
				if ((evt->modifiers & activeFlag)	/* and coming active */
					&& ((WindowPeek) theDialog)->windowKind == dialogKind) {
					SetPort ((GrafPtr) theDialog);
				}
			/* fall through */
			default:
				if (!(dh && (**dh).whEvent &&
						(handledIt = (*(**dh).whEvent)(&item, evt))	)) {
					handledIt = DialogSelect (evt, &theDialog, &item);
				}
				break;
		}
		if ((dh = GetDHandler ((WindowPtr) theDialog)) != nil) {
			(**dh).whItemHit = item;
			/* if this is a modal dialog, whCheck will be called elsewhere */
			if ((**dh).whCheck && !(**dh).whModal) {
				(*(**dh).whCheck) (item);
			}
		}
	}
	return (handledIt);
}

#endif

/* -------------------------------------------------------------------- */
/*							Event-handling routines						*/
/* -------------------------------------------------------------------- */


static void SkelCheckCursor(WHandler **wh, Point mouse, RgnHandle region)
{
	Point pt;
	RgnHandle arrowRgn, localRgn;

	if (wh && (**wh).whHaveCursor) 
	{
		arrowRgn = NewRgn ();
		localRgn = NewRgn ();
		CopyRgn ((**wh).whCursorRgn, localRgn);
		CopyRgn (screensRgn, arrowRgn);
		SetPt (&pt, 0, 0);
		SetPort ((**wh).whWind);
		LocalToGlobal (&pt);
		OffsetRgn (localRgn, pt.h, pt.v);
		DiffRgn (arrowRgn, localRgn, arrowRgn);

		if (PtInRgn (mouse, localRgn)) 
		{
			SetCursor (*(**wh).whCursor);
			CopyRgn (localRgn, region);
		}
		else {
			InitCursor ();
			CopyRgn (arrowRgn, region);
		}
		DisposeRgn (localRgn);
		DisposeRgn (arrowRgn);
	}
	else {
		InitCursor ();
		CopyRgn (screensRgn, region);
	}
}

/*
	Have either zoomed a window or sized it manually.  Invalidate
	it to force an update and set the 'resized' flag in the window
	handler true.  The port is assumed to be set to the port that changed
	size.
*/

static void TriggerUpdate(WHandler **h, GrafPtr grownPort, short mods)
{
	GrafPtr savePort;

	GetPort (&savePort);
	SetPort (grownPort);
	InvalRect (&grownPort->portRect);
	if (h != nil) {
		(**h).whSized = true;
		(**h).whSizedMods = mods;
		if ((**h).whHaveCursor && grownPort == FrontWindow ()) 
		{
			SetEmptyRgn (cursorRgn);
		}
	}
	SetPort (savePort);
}

/*
	Size a window.  If the window has a handler, use the grow limits
	in the handler record, otherwise use the defaults.

	The portRect is invalidated to force an update event.  (The port
	must be set first, as it could be pointing anywhere.)  The handler's
	update procedure should check the parameter passed to it to check
	whether the window has changed size, if it needs to adjust itself to
	the new size.  THIS IS A CONVENTION.  Update procs must notice grow
	"events", there is no procedure specifically for such events.
	
	The clipping rectangle is not reset.  If the host application
	keeps the clipping set equal to the portRect or something similar,
	then it will have to arrange to treat window growing with more
	care.

	Since the grow region of only the active window may be clicked,
	it should not be necessary to set the port.
*/

static void DoGrow(WHandler **h, GrafPtr thePort, Point startPt, short mods)
{
	Rect				r;
	register long		growRes;

	r = (**h).whGrow;
	/* grow result non-zero if size change	*/
	if (growRes = GrowWindow (thePort, startPt, &r)) {
		SizeWindow (thePort, LoWord (growRes), HiWord (growRes), false);
		TriggerUpdate (h, thePort, mods);
	}
}


/*
	Zoom the current window.  Very similar to DoGrow

	Since the zoombox of only the active window may be clicked,
	it should not be necessary to set the port.
*/

static void DoZoom(WHandler **h, GrafPtr zoomPort, short partCode, short mods)
{
	if (pZoomProc) 
	{
		/* for a custom zoom proc, allow the user to modify the behaviour */
		(*pZoomProc) (zoomPort, partCode, mods, false);
	}
	else 
		{
			ZoomWindow (zoomPort, partCode, false);
	}
	TriggerUpdate (h, zoomPort, mods);
}

/*
	General event handler
*/

static void DoEvent (EventRecord *theEvt)
{
	register EventRecord	*theEvent;
	Point					evtPt;
	GrafPtr					evtPort;
	register short			evtPart;
	register char			evtChar;
	register unsigned char	evtCode;
	register short			evtMods = 0;
	register WHandler		**h, **t;
	Rect					r;
	WindowPtr				frontWindow = FrontWindow ();

	theEvent = theEvt;

#ifdef	supportDialogs
	if(DoDialog (theEvent))
		return;
#endif

	evtPt = theEvent->where;
	evtMods = theEvent->modifiers;

	switch (theEvent->what) 
	{
		case app4Evt:		/* multifinder event */
			{
				short flag = ((theEvent->message >> 24) & 0xff);
				if (flag == 0x01) {
					fgApp = ((theEvent->message & 0x01) != 0);
					DoActivate (frontWindow, fgApp);
					/* does the app want to know about suspend/resume? */
					if (pSuspendResume) {
						(*pSuspendResume)(fgApp);
					}
					/* should we convert the scrap?  Always convert if suspend
					   but only convert on resume if "changed scrap" flag bit is set */
					if ((!fgApp || (theEvent->message & 0x02)) && pScrapConvert) {
						(*pScrapConvert)(fgApp);
					}
				}
				else if (flag == 0xfa && fgApp) {
					SkelCheckCursor (GetWDHandler (frontWindow), evtPt, cursorRgn);
				}
				else if (flag == 0xfd) {
					if (pChildDied) {
						(*pChildDied) (theEvent);
					}
				}
				else if (pHandleApp4) {
					(*pHandleApp4) (theEvent);
				}
			}
			/* fall through to do "idle" processing */
		case nullEvent:
			if (pBkgnd != nil) {
				(*pBkgnd) ();
			}
			if (!useWaitNextEvent && !PtInRgn (evtPt, cursorRgn)) {
				SkelCheckCursor (GetWDHandler (frontWindow), theEvent->where, cursorRgn);
			}
			break;
/*
	Mouse click.  Get the window that the click occurred in, and the
	part of the window.
*/
		case mouseDown:
		{
			evtPart = FindWindow (evtPt, &evtPort);
			h = GetWHandler (evtPort);
#ifdef supportDialogs
			/* don't allow mouse clicks outside a modal dialog */
			if (evtPort != frontWindow &&
					(t = GetWDHandler(FrontWindow())) != nil &&
					(**t).whModal) {
				break;
			}
#endif
			switch (evtPart)
			{
/*
	Click in a desk accessory window.  Pass back to the system.
*/
				case inSysWindow:
				{
					SystemClick (theEvent, evtPort);
					break;
				}
/*
	Click in menu bar.  Track the mouse and execute selected command,
	if any.
*/
				case inMenuBar:
				{
					DoMenuCommand (MenuSelect (evtPt));
					break;
				}
/*
	Click in grow box.  Resize window.
*/
				case inGrow:
				{
					if (h) {
						if ((**h).whCanGrow) {
							DoGrow (h, evtPort, evtPt, evtMods);
						}
						else {
							DoMouse (h, theEvent);
						}
					}
					break;
				}
/*
	Click in title bar.  Drag the window around.  Leave at least
	4 pixels visible in both directions.
	Bug fix:  The window is selected first to make sure it's at least
	activated (unless the command key is down-see Inside Macintosh).
	DragWindow seems to call StillDown first, so that clicks in drag
	regions while machine is busy don't otherwise bring window to front if
	the mouse is already up by the time DragWindow is called.
*/
				case inDrag:
				{
					r = screenPort->portRect;
					r.top += mBarHeight;			/* skip down past menu bar */
					InsetRect (&r, 4, 4);
					SkelDragWindow (evtPort, evtPt, evtMods, &r);
					break;
				}
/*
	Click in close box.  Call the close proc if the window has one.
*/
				case inGoAway:
				{
					if (TrackGoAway (evtPort, evtPt))
						DoClose (h);
					break;
				}
/*
	Click in content region.  If the window wasn't frontmost (active),
	just select it, otherwise pass the click to the window's mouse
	click handler.
*/
				case inContent:
				{
					if (evtPort != frontWindow) {
						SelectWindow (evtPort);
						SetPort (evtPort);
					}
					else
						DoMouse (h, theEvent);
					break;
				}

/*
	Click in zoom box.  Track the click and then zoom the window if
	necessary
*/
				case inZoomIn:
				case inZoomOut:
				{
					if(TrackBox(evtPort, evtPt, evtPart))
						DoZoom (h, evtPort, evtPart, evtMods);
					break;
				}

			}
			break;	/* mouseDown */
		}
/*
	Key event.  If the command key was down, process as menu item
	selection, otherwise pass the character and the modifiers flags
	to the active window's key handler.

	If dialogs are supported, there's no check for command-key
	equivalents, since that would have been checked in DoDialog.
*/
		case keyDown:
		case autoKey:
		{
			evtCode = (theEvent->message & keyCodeMask) >> 8;
			evtChar = theEvent->message & charCodeMask;
#ifndef supportDialogs
			if (evtMods & cmdKey) {
				/* try menu equivalent */
				DoMenuCommand (MenuKey (evtChar));
				break;
			}
#endif
			/* frontWindow is at least frontDocWindow--
				tools can't get keys! (Yet!!) */
			DoKey (GetWHandler (frontWindow), evtChar, evtCode, evtMods);
			break;
		}

		case keyUp:
		{
			evtCode = 0x80 | ( (theEvent->message & keyCodeMask) >> 8 );
			evtChar = theEvent->message & charCodeMask;

			DoKey (GetWHandler (frontWindow), evtChar, evtCode, evtMods);
			break;
		}
		
/*
	Update a window.
*/
		case updateEvt:
		{
			DoUpdate (GetWDHandler ((WindowPtr) theEvent->message));
			break;
		}
/*
	Activate or deactivate a window.
*/
		case activateEvt:
		{
			DoActivate ((WindowPtr) theEvent->message,
						((theEvent->modifiers & activeFlag) != 0));
			break;
		}
/*
	handle inserts of uninitialized disks
*/
		case diskEvt:
		{
			if (HiWord (theEvent->message) != noErr) {
				DILoad ();
				(void) DIBadMount (diskInitPt, theEvent->message);
				DIUnload ();
			}
			break;
		}
/*
	handle network events
*/
		case networkEvt:
		{
			if (pNetworkProc) {
				(*pNetworkProc) (theEvent);
			}
			break;
		}
	}
}


/* -------------------------------------------------------------------- */
/*						Interface (public) Routines						*/
/* -------------------------------------------------------------------- */


/*
	Initialize the various Macintosh Managers.
	Set default upper limits on window sizing.
	FlushEvents does NOT toss disk insert events, so that disks
	inserted while the application is starting up don't result
	in dead drives.
	Determine whether to use WaitNextEvent or not.
	Set up big rectangle for cursor management.
*/
#define _WaitNextEvent 0xA860

void SkelInit(void (*resume)(), int extra)
{
	Rect r;

	MaxApplZone ();
	FlushEvents (everyEvent - diskMask, 0 );
#ifdef applec
	InitGraf (&qd.thePort);
#else
	InitGraf (&thePort);
#endif
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
#ifdef applec
	InitDialogs ((ResumeProcPtr) resume);		/* no restart proc */
#else
	InitDialogs ((ProcPtr) resume);		/* no restart proc */
#endif
	InitCursor ();

	while (extra-- > 0)
		MoreMasters ();

/*
	Set upper limits of window sizing to machine screen size.  Allow
	for the menu bar (use the glue from the Script Manager).
*/
	GetWMgrPort (&screenPort);
	growRect.top = 80;
	growRect.left = 80;
	growRect.right = screenPort->portRect.right;
	growRect.bottom = screenPort->portRect.bottom - (mBarHeight = GetMBarHeight ());

	/* this is (at this time anyway) pointless */
	SkelEventMask (eventMask);

	/* throw away any error return */
	(void) SysEnvirons (curSysEnvVers, &skelEnvRec);
	useWaitNextEvent = SkelCheckTrap (_WaitNextEvent, ToolTrap);

	/* build a region that covers all of the displays available */
	screensRgn = NewRgn ();
	SetRect (&r, -32768, -32768, 32767, 32767);
	/* account for bug in 128K ROMs */
	if (skelEnvRec.machineType < envMacII)
		InsetRect (&r, 1, 1);
	RectRgn (screensRgn, &r);
	/* a place to watch the cursor in */
	cursorRgn = NewRgn ();
}

/*
	Main loop.

	Take care of DA's with SystemTask (not needed with WaitNextEvent).
	Run background task if there is one. (now at nullEvents in DoEvent)
	If there is an event, check for an event hook.  If there isn't
	one defined, or if there is but it returns false, call the
	general event handler.  (Hook returns true if TransSkel should
	ignore the event.)
	If no event, call the "no-event" handler for the front window and for
	any other windows with idle procedures that are always supposed
	to run.  This is done in such a way that it is safe for idle procs
	to remove the handler for their own window if they want (unlikely,
	but...)  This loop doesn't check whether the window is really
	a dialog window or not, but it doesn't have to, because such
	things always have a nil idle proc.
	
	doneFlag is reset upon exit.  This allows it to be called
	repeatedly, or recursively.

	If dialogs are supported, null events are looked at (in SkelMain)
	and passed to the event handler.  This is necessary to make sure
	DialogSelect gets called repeatedly, or the caret won't blink if
	a dialog has any editText items.

	In order to be more multi-finder compatible, null-events are passed
	through to doEvent--where the appropriate idle processing is done.
*/

Boolean SkelWaitNextEvent(short evtMask, EventRecord *evt, 
					long timeOut, RgnHandle checkRgn)
{
	if (useWaitNextEvent) {
		return (WaitNextEvent (evtMask, evt, timeOut, checkRgn));
	}
	else {
		SystemTask ();
		return (GetNextEvent (evtMask, evt));
	}
}

/*
	if checkRgn is nil then mouse-moved events *won't* be generated
*/
void SkelOnePass (RgnHandle checkRgn)
{
	EventRecord			theEvent;
	register WHandler	**wh, **wh2;
	Boolean				haveEvent;
	WindowPtr			frontWindow;
	long				timeOut = pBGTime;

/*
	Now watch carefully.  GetNextEvent calls SystemEvent to handle some
	DA events, and returns false if the event was handled.  However, in
	such cases the event record will still have the event that occurred,
	*not* a null event, as you might reasonably expect.  So it's not
	enough to look at haveEvent.

	Previous versions figured (wrongly) that haveEvent==false meant a null
	event had occurred, and passed it through to DoEvent and DoDialog, so
	that caret-blinking in dialog TextEdit items would occur.  But cmd-key
	equivalents while DA windows were in front, in particular, allowed
	already-processed DA events to get into DoEvent (because haveEvent
	was false), and they got handled twice because when the event record
	was examined, lo and behold, it had a cmd-key event!  So now this
	logic is used:

	If have a real event, and there's no event hook or there is but it
	doesn't handle the event, OR if the "non-event" is a true nullEvent,
	then process it. (modified to allow nullEvents to go to event hook)
*/
		
	if (fgApp) {
		timeOut = pFGTime;
	}
	haveEvent = SkelWaitNextEvent (eventMask, &theEvent, timeOut, checkRgn);

	/* ignore haveEvent here so that event hook will get null events
	   a check for a nullEvent is pretty silly in this case */
	if ((pEvent == nil || (*pEvent)(&theEvent) == false))
		DoEvent(&theEvent);

/*
	Run applicable idle procs.  Make sure to save and restore the port,
	since idle procs for the non-active window may be run.
*/
	if (!haveEvent || theEvent.what == nullEvent) {
		frontWindow = FrontWindow ();
		for (wh = whList; wh != nil; wh = wh2) {
			wh2 = (**wh).whNext;
			if ((**wh).whWind == frontWindow || !(**wh).whFrontOnly) {
				DoIdle (wh);
			}
		}
	}
}

void SkelMain ()
{
#ifdef	CHECK_STACK
	long stackFree;
#endif
	/* create a region to do cursor handling in */
	while (!doneFlag) {	
		SkelOnePass (cursorRgn);
#ifdef	CHECK_STACK
		stackFree = StackSpace ();
#endif
	}
	doneFlag = false;
	/* ditch the cursor control region */
}


/*
	Tell SkelMain to stop
*/

void SkelWhoa ()
{
	doneFlag = true;
}


/*
	Clobber all the menu, window and dialog handlers
*/

void SkelClobber ()
{
	while (whList != nil)
		SkelRmveWind ((**whList).whWind);

	/* 'false' removes the annoying flicker as menus are deleted */
	while (mhList != nil)
		RemoveMenu (GetMHandle((**mhList).mhID), false);
	/* draw the menu bar (now empty) in case there's some system level cleanup */
	DrawMenuBar ();
	DisposeRgn (cursorRgn);
	/* make the event mask innocuous for uniFinder */
	SetEventMask (everyEvent ^ keyUpMask);
}


/* -------------------------------------------------------------------- */
/*						Menu-handler interface routines					*/
/* -------------------------------------------------------------------- */


/*
	Install handler for a menu.  Remove any previous handler for it.
	Pass the following parameters:

	theMenu	Handle to the menu to be handled.  Must be created by host.
	pSelect	Proc that handles selection of items from menu.  If this is
			nil, the menu is installed, but nothing happens when items
			are selected from it.
	pClobber Proc for disposal of handler's data structures.  Usually
			nil for menus that remain in menu bar until program
			termination.
	isPull	Should this be installed in the menubar?
	drawBar	draw the menu bar?
*/

void SkelMenu(MenuHandle theMenu, 
	void (*pSelect)(short), 
	void (*pClobber)(MenuHandle), 
	Boolean isPull, 
	Boolean drawBar)
{
	register MHandler	**mh;

	mhClobOnRmve = false;
	SkelRmveMenu (theMenu);
	mhClobOnRmve = true;

	mh = New (MHandler);
	(**mh).mhNext = mhList;
	mhList = mh;
	(**mh).mhID = (**theMenu).menuID;			/* get menu id number */
	(**mh).mhSelect = pSelect;					/* install selection handler */
	(**mh).mhClobber = pClobber;				/* install disposal handler */
	InsertMenu (theMenu, (isPull) ? -1 : 0);	/* insert menu in menulist */
	if (drawBar)
		DrawMenuBar ();
}

/*
	Remove a menu from the menu list for the user
*/

void SkelRmveMenu (MenuHandle theMenu)
{
	/* 'true' forces the menubar to be redrawn */
	RemoveMenu (theMenu, true);
}

/*
	Install a handler for the Apple menu.
	
	SkelApple is called if TransSkel is supposed to handle the apple
	menu itself.  aboutTitle is the title of the first item.  If nil,
	then only desk accessories are put into the menu.  If not nil, then
	the title is entered as the first item, followed by a gray line,
	then the desk accessories.

	SkelApple does not cause the menubar to be drawn, so if the Apple
	menu is the only menu, DrawMenuBar must be called afterward.

	No value is returned, unlike SkelMenu.  It is assumed that SkelApple
	will be called so early in the application that the call the SkelMenu
	is virtually certain to succeed.  If it doesn't, there's probably
	little hope for the application anyway.
*/

void SkelApple(char *aboutTitle, void (*aboutProc)(void))
{
	appleID = 1;
	appleMenu = NewMenu (appleID, "\p\024");	/* 024 = apple character */
	if (aboutTitle != nil) 
	{
		doAbout = true;
		AppendMenu (appleMenu, aboutTitle);	/* add About... item title */
		AppendMenu (appleMenu, "\p(-");		/* add gray line */
		appleAboutProc = aboutProc;
	}
	AddResMenu (appleMenu, 'DRVR');		/* add desk accessories */
	SkelMenu (appleMenu, DoAppleItem, DoAppleClobber, false, false);
}

void SkelHelp(char *helpTitle, void (*helpProc)(void))
{
	if (appleID == 1 && helpTitle != nil) {
		doHelp = true;
		InsMenuItem (appleMenu, helpTitle, (doAbout) ? 1 : 0);
		appleHelpProc = helpProc;
	}
}

void SkelEnableMenu(short mID, Boolean redraw)
{
	MenuHandle m = GetMHandle (mID);
	if (m) {
		EnableItem (m, 0);
	}
	if (redraw)
		DrawMenuBar ();
}

void SkelDisableMenu (short mID, Boolean redraw)
{
	MenuHandle m = GetMHandle (mID);
	if (m) 
	{
		DisableItem (m, 0);
	}
	if (redraw)
		DrawMenuBar ();
}

void SkelEnableMenus ()
{
	register MHandler **mh = mhList;
	while (mh) {
		SkelEnableMenu ((**mh).mhID, false);
		mh = (**mh).mhNext;
	}
	DrawMenuBar ();
}

void SkelDisableMenus ()
{
	register MHandler **mh = mhList;
	while (mh) {
		SkelDisableMenu ((**mh).mhID, false);
		mh = (**mh).mhNext;
	}
	DrawMenuBar ();
}

/* -------------------------------------------------------------------- */
/*					Window-handler interface routines					*/
/* -------------------------------------------------------------------- */

/*
	Install handler for a window.  Remove any previous handler for it.
	Pass the following parameters:

	theWind	Pointer to the window to be handled.  Must be created by host.
	pMouse	Proc to handle mouse clicks in window.  The proc will be
			passed the point (in local coordinates), the time of the
			click, and the modifier flags word.
	pKey	Proc to handle key clicks in window.  The proc will be passed
			the character and the modifier flags word.
	pUpdate	Proc for updating window.  TransSkel brackets calls to update
			procs with calls to BeginUpdate and EndUpdate, so the visRgn
			is set up correctly.  A flag is passed indicating whether the
			window was resized or not.  BY CONVENTION, the entire portRect
			is invalidated when the window is resized.  That way, the
			handler's update proc can redraw the entire content region
			without interference from BeginUpdate/EndUpdate.  The flag
			is set to false after the update proc is called; the
			assumption is made that it will notice the resizing and
			respond appropriately.
	pActivate Proc to execute when window is activated or deactivated.
			A boolean is passed to it which is true if the window is
			coming active, false if it's going inactive.
	pClose	Proc to execute when mouse clicked in close box.  Useful
			mainly to temp window handlers that want to know when to
			self-destruct (with SkelRmveWind).
	pClobber Proc for disposal of handler's data structures
	pIdle	Proc to execute when no events are pending.
	frontOnly True if pIdle should execute on no events only when
			theWind is frontmost, false if executes all the time.  Note
			that if it always goes, everything else may be slowed down!

	If a particular procedure is not needed (e.g., key events are
	not processed by a handler), pass nil in place of the appropriate
	procedure address.

	All handler procedures may assume that the port is set correctly
	at the time they are called.
*/

Boolean SkelWindow(
	WindowPtr theWind, 
	void (*pMouse)(Point,long,short),
	void (*pKey)(char,unsigned char,short),
	void (*pUpdate)(Boolean,short), 
	void (*pActivate)(Boolean),
	void (*pClose)(void), 
	void (*pClobber)(void), 
	void (*pIdle)(void), 
	Boolean frontOnly)
{
	register WHandler	**hHand, *hPtr;

	if ((hHand = GetWDHandler (theWind)) != nil) {
		/* remove the window, don't clobber it! */
		whClobOnRmve = false;
		SkelRmveWind (theWind);
		whClobOnRmve = true;
	}
/*
	Get new handler, attach to list of handlers.  It is attached to the
	beginning of the list, which is simpler; the order is presumably
	irrelevant to the host, anyway.
*/
	hHand = SkelNullHandler (theWind);
	if (hHand) {
/*
	Fill in handler fields
*/
		hPtr = *hHand;
		hPtr->whMouse = pMouse;
		hPtr->whKey = pKey;
		hPtr->whUpdate = pUpdate;
		hPtr->whActivate = pActivate;
		hPtr->whClose = pClose;
		hPtr->whClobber = pClobber;
		hPtr->whIdle = pIdle;
		hPtr->whFrontOnly = frontOnly;
		hPtr->whSized = false;
		hPtr->whSizedMods = 0;
	}
	return (hHand != nil);
}


/*
	Remove a window handler.  This calls the handler's disposal routine
	and then takes the handler out of the handler list and disposes
	of it.

	SkelRmveWind is also called by SkelRmveDlog.
*/

void SkelRmveWind (WindowPtr theWind)
{
register WHandler	**h, **h2;

	if (whList != nil) {
		/* if list empty, ignore */
		if ((**whList).whWind == theWind) 
		{
			/* is it the first element? */
			h2 = whList;
			whList = (**whList).whNext;
		}
		else {
			for (h = whList; h != nil; h = h2) 
			{
				h2 = (**h).whNext;
				if (h2 == nil) {
					/* theWind not in list! */
					return;
				}
				if ((**h2).whWind == theWind) 
				{
					/* found it */
					(**h).whNext = (**h2).whNext;
					break;
				}
			}
		}
		if (whClobOnRmve)
			DoClobber (h2);				/* call disposal routine */
		if ((**h2).whCursorRgn) 
		{
			DisposeRgn ((**h2).whCursorRgn);
		}
		DisposHandle ((Handle) h2);		/* get rid of handler record */
	}
}

/*
	Set a cursor and a region to allow WaitNextEvent to return mouse-moved
	events.  This routine should probably also allow a routine to be passed
	in that would be called when the cursor changed�allowing multiple regions
	(and multiple cursors) per window, or perhaps a list of regions and
	cursors.  At the moment the most economical and useful way of doing this
	is unclear.  -RTS
*/

void SkelSetCursor(WindowPtr theWind, CursHandle theCursor, RgnHandle theCursorRgn)
{
	register WHandler		**h;
	if ((h = GetWDHandler (theWind)) != nil) 
	{
		if ((**h).whHaveCursor = (theCursorRgn != nil)) 
		{
			if (!(**h).whCursorRgn) 
			{
				(**h).whCursorRgn = NewRgn ();
			}
			CopyRgn (theCursorRgn, (**h).whCursorRgn);
		}
		else {
			if ((**h).whCursorRgn) 
			{
				DisposeRgn ((**h).whCursorRgn);
			}
			(**h).whCursorRgn = theCursorRgn;
		}
		(**h).whCursor = theCursor;
	}
}

/*
	Return something about the state of cursor management for this
	window.  Allows the resize code to do something like:
	
	if (SkelGetCursor (nowWin, &cursorHand, &cursorRgn)) {
		cursorRgn = calcUpdatedCursorRegion (nowWin);
		SkelSetCursor (nowWin, cursorHand, cursorRgn);
	}
*/

Boolean SkelGetCursor(WindowPtr theWind, CursHandle *theCursor, 
					RgnHandle *theCursorRgn)
{
	register WHandler **h;
	if ((h = GetWDHandler (theWind)) != nil) 
	{
		*theCursor = (**h).whCursor;
		*theCursorRgn = (**h).whCursorRgn;
		return ((**h).whHaveCursor);
	}
	else 
		{
			return (false);
	}
}

/*
	allow the user to associate an arbitrary long integer with
	a window
*/

void SkelSetReference(WindowPtr theWind, long refCon)
{
	register WHandler **h;
	if ((h = GetWDHandler (theWind)) != nil) {
		(**h).userReference = refCon;
	}
}

long SkelGetReference(WindowPtr theWind)
{
	register WHandler **h;
	if ((h = GetWDHandler (theWind)) != nil) {
		return ((**h).userReference);
	}
	else {
		return (0L);
	}
}

#ifdef	supportDialogs

/* -------------------------------------------------------------------- */
/*					Dialog-handler interface routines					*/
/* -------------------------------------------------------------------- */

/*
	Install a dialog handler.  Remove any previous handler for it.
	SkelDialog calls SkelWindow as a subsidiary to install a window
	handler, then sets the event procedure on return.

	Pass the following parameters:

	theDialog	Pointer to the dialog to be handled.  Must be created
			by host.
	pEvent	Event-handling proc for dialog events.
	pClose	Proc to execute when mouse clicked in close box.  Useful
			mainly to dialog handlers that want to know when to
			self-destruct (with SkelRmveDlog).
	pClobber Proc for disposal of handler's data structures

	If a particular procedure is not needed, pass nil in place of
	the appropriate procedure address.

	All handler procedures may assume that the port is set correctly
	at the time they are called.
*/

static void SkelDefaultDialogUpdate (Boolean resized, short mods)
{
#ifdef applec
#pragma unused (resized, mods)
#endif
	DialogPtr dLog;
	GetPort ((WindowPtr *) &dLog);
	UpdtDialog(dLog, dLog->visRgn);
}

void SkelDialog(DialogPtr theDialog, 
	Boolean (*pEvent)(short *, EventRecord *), 
	Boolean (*pCheck)(short), 
	Boolean (*pKeys)(short *, EventRecord *), 
	void (*pUpdate)(Boolean,short), 
	void (*pClose)(void), 
	void (*pClobber)(void))
{
	register WHandler **h;
	if (pUpdate == nil)
		pUpdate = SkelDefaultDialogUpdate;
#ifdef applec
	SkelWindow (theDialog, nil,
				(void (*)(char, unsigned char, short)) pKeys,
				pUpdate, nil, pClose, pClobber,
				nil, false);
#else
	SkelWindow (theDialog, nil,
				(void (*)())pKeys,
				pUpdate, nil, pClose, pClobber,
				nil, false);
#endif
	h = GetWDHandler (theDialog);
	(**h).whEvent = pEvent;
	(**h).whCheck = pCheck;
}

/*
	Remove a dialog and its handler
*/

void SkelRmveDlog (DialogPtr theDialog)
{
	SkelRmveWind (theDialog);
}

/*
	Act like modal dialog--but do all the other stuff, too
*/

short SkelModalDialog(
	DialogPtr dLog, 
	Boolean (*pEvent)(short *, EventRecord *), 
	Boolean (*pCheck)(short), 
	Boolean (*pKeys)(short *, EventRecord *), 
	void (*pUpdate)(Boolean,short), 
	long refCon);
{
	register WHandler **h;
	short itemHit;

	SkelDialog (dLog, pEvent, pCheck, pKeys, pUpdate, nil, nil);
	h = GetDHandler (dLog);
	SkelSetReference (dLog, refCon);
	(**h).whModal = true;
	/* default to 0 for check routine */
	(**h).whItemHit = 0;
	while (pCheck && (*pCheck) ((**h).whItemHit)) {
		(**h).whItemHit = 0;
		SkelOnePass (0);
		/* in case this got changed by the main loop */
		SetPort ((GrafPtr) dLog);
	}
	itemHit = (**h).whItemHit;
	SkelRmveDlog (dLog);
	return (itemHit);
}
#endif


/* -------------------------------------------------------------------- */
/*					Miscellaneous interface routines					*/
/* -------------------------------------------------------------------- */

/*
	Override the default sizing limits for a window, or, if theWind
	is nil, reset the default limits used by SkelWindow.
*/

void SkelGrowBounds(WindowPtr theWind, short hLo, short vLo, short hHi, short vHi)
{
	register WHandler	**h;
	Rect				r;

	if (theWind == nil) {
		SetRect (&growRect, hLo, vLo, hHi, vHi);
	}
	else if ((h = GetWDHandler (theWind)) != nil) {
		SetRect (&r, hLo, vLo, hHi, vHi);
		(**h).whGrow = r;
		(**h).whCanGrow = ((hLo != hHi) || (vLo != vHi));
	}
}


/*
	Set the event mask.  Allow keyup events. -DTH
*/

void SkelEventMask (short mask)
{
	eventMask = mask;
	if (everyEvent == (eventMask | keyUpMask))
		SetEventMask (eventMask);
}


/*
	Return the event mask.
*/

void SkelGetEventMask (short *mask)
{
	*mask = eventMask;
}


/*
	Install a background task.  If p is nil, the current task is
	disabled.  fgTime and bgTime are the timeOut values when
	WaitNextEvent is being used. -RTS
*/

void SkelBackground(void (*p)(void), long fgTime, long bgTime)
{
	pBkgnd = p;
	pFGTime = fgTime;
	pBGTime = bgTime;
}


/*
	Return the current background task.  Return nil if none.
	Also return the current foreground and background pause times
	for use with WaitNextEvent.
*/

void SkelGetBackground(void (**p)(void), long *fgTime, long *bgTime)
{
	*p = pBkgnd;
	*fgTime = pFGTime;
	*bgTime = pBGTime;
}


/*
	Install an event-inspecting hook.  If p is nil, the hook is
	disabled.
*/

void SkelEventHook(Boolean (*p)(EventRecord *))
{
	pEvent = p;
}


/*
	Return the current event-inspecting hook.  Return nil if none.
*/

void SkelGetEventHook(Boolean (**p)(EventRecord *))
{
	*p = pEvent;
}

/*
	Install a scrap conversion hook.  If p is nil, the hook is disabled.
*/

void SkelScrapConvert(void (*p)(Boolean))
{
	pScrapConvert = p;
}

/*
	Return the current scrap conversion hook.  Return nil if none.
*/

void SkelGetScrapConvert(void (**p)(Boolean))
{
	*p = pScrapConvert;
}

/*
	Install a multifinder suspend/resume hook.  If p is nil, the hook is disabled.
*/

void SkelSuspendResumeProc(void (*p)(Boolean))
{
	pSuspendResume = p;
}

/*
	Return the current multifinder suspend/resume hook.  Return nil if none.
*/

void SkelGetSuspendResume(void (**p)(Boolean))
{
	*p = pSuspendResume;
}

/*
	Install a multifinder childDied event hook.
*/

void SkelChildDiedProc(void (*p)(EventRecord *))
{
	pChildDied = p;
}

/*
	Return the current multifinder childDied event hook.
*/

void SkelGetChildDiedProc(void (**p)(EventRecord *))
{
	*p = pChildDied;
}

/*
	Install a multifinder childDied event hook.
*/

void SkelHandleApp4Proc(void (*p)(EventRecord *))
{
	pHandleApp4 = p;
}

/*
	Return the current multifinder childDied event hook.
*/

void SkelGetHandleApp4Proc(void (**p)(EventRecord *))
{
	*p = pHandleApp4;
}

/*
	Install a handler for network events

	This is not really useful anymore under the new systems as networkEvts
	are be guaranteed to arrive.  The suggested way now is to use callbacks.
	
*/

void SkelNetworkProc(void (*p)(EventRecord *))
{
	pNetworkProc = p;
}

void SkelGetNetworkProc(void (**p)(EventRecord *))
{
	*p = pNetworkProc;
}

/*
	Install a zoom proc hook.  If p is nil, the hook is disabled.
*/

void SkelZoomProc(void (*p)(WindowPtr,short,short,Boolean))
{
	pZoomProc = p;
}

/*
	Return the zoom proc hook.  Return nil if none.
*/

void SkelGetZoomProc(void (**p)(WindowPtr,short,short,Boolean))
{
	*p = pZoomProc;
}

/*
	Return the environment record that TransSkel is using -RTS
*/

SysEnvRec *SkelGetEnvirons ()
{
	return (&skelEnvRec);
}

/*
	Check for the existence of a trap
*/
#define _Unimplemented                  0xA89F


Boolean	SkelCheckTrap(short trapNumber, TrapType trapKind)
{
	if (trapKind == ToolTrap && (skelEnvRec.machineType > envMachUnknown &&
			skelEnvRec.machineType < envMacII)) {
		trapNumber &= 0x03FF;
		if (trapNumber > 0x01FF) {
			/* ToolTraps don't exist on these machines */
			trapNumber = _Unimplemented;
		}
	}
	return (NGetTrapAddress (trapNumber, trapKind) !=
							GetTrapAddress (_Unimplemented));
}

#ifdef	supportDialogs

/*
	Set the mask for event types that will be passed to dialogs.
	Bit 1 is always set, so that null events will be passed.
*/

void SkelDlogMask (short mask)
{
	dlogEventMask = mask | 1;
}


/*
	Return the current dialog event mask.
*/

void SkelGetDlogMask (short *mask)
{
	*mask = dlogEventMask;
}

#endif

/* attempt to stop crashing from nested SkelOnePasses */

RgnHandle SkelUseCursorRgn (RgnHandle newRgn)
{
	RgnHandle tempRgn;
	tempRgn = cursorRgn;
	cursorRgn = newRgn;
	return (tempRgn);
}
