/*
 * TransSkel - Transportable Macintosh application skeleton
 * Release 3.18
 *
 * Please report problems to Paul DuBois.
 *
 *
 * TransSkel is in the public domain and was originally written by:
 *
 * 			Paul DuBois
 * 			Wisconsin Regional Primate Research Center
 * 			1220 Capitol Court
 * 			Madison, WI  53715-1299  USA
 *
 * Internet:	dubois@primate.wisc.edu
 *			
 * Additional changes were made by:
 *	
 * 			Owen Hartnett
 * 			OHM Software Company
 * 			163 Richard Drive
 * 			Tiverton, RI 02878  USA
 * 		
 * Internet:	omh@cs.brown.edu
 * UUCP:		uunet!brunix!omh	
 *
 * Owen is also responsible for the port to THINK Pascal.
 *
 * Bob Schumaker joined the cast between versions 2.0 and 3.0.
 *
 * 			Bob Schumaker
 * 			The AMIX Corporation
 * 			1881 Landings Drive
 * 			Mountain View, CA 94043-0848
 *
 * Internet:	bob@markets.amix.com
 * UUCP:		{sun, uunet, netcom}!markets!bob
 * CIS:			72227,2103
 * AOL:			BSchumaker
 *
 * This version of TransSkel is written for THINK C 6.0.1.
 * THINK C is a trademark of:
 *
 * 	Symantec Corporation
 * 	10201 Torre Avenue
 * 	Cupertino, CA 95014  USA
 *
 * Reference key:
 * IM		Inside Macintosh
 * TN		Macintosh Technical Notes
 * MHIG		Macintosh Human Interface Guidelines
 * HIN		Human Interface Notes
 * PGMF		Programmer's Guide to MultiFinder (APDA)
 * TPN		TransSkel Programmer's Notes
 *
 * Recent history is given below.  Earlier change history is in TSHistory.
 * If you've been writing applications with an earlier release, READ THAT FILE!
 *
 * 15 May 94 Release 3.17
 * - Changed references to QuickDraw globals so they're written in terms of the
 * qd struct.  E.g., thePort -> qd.thePort, gray -> qd.gray.
 * - Redid some pattern references so they'll compile whether or not
 * dangerousPattern is defined, and if universal headers are used.
 * 25 May 94
 * - Try to cast the argument to InitDialogs() properly depending on whether or
 * not the universal headers are used.
 *
 * 10 Aug 94 Release 3.18
 * - Began adding full support for Universal Headers so TransSkel can be
 * compiled in native PowerPC mode.  I'm doing this under Metrowerks C, so
 * for the moment this also involves porting to Metrowerks.  (For compiling
 * M680x0 code under Metrowerks, no changes were necessary from release 3.17,
 * which was a pleasant surprise.)
 * - skelUnivHeaders invented.  If value is 1, the Universal Headers are
 * available in the header files(and thus UPP definitions are also available),
 * otherwise workarounds are necessary for UPP types.
 * - skelPPC invented.  If value is 1, PPC code is being compiled.  Simpler
 * check than checking for powerc and __powerc.
 * 05 Sep 94
 * - Fixed bug in SkelDialog() with not checking whether the window property
 * type was already attached to the handler.  That could happen when replacing
 * the handler for an existing window, and trying to add the property when one
 * is already there returns an error.
 * 30 Oct 94
 * - Release 3.17 went to the qd form of QuickDraw global access, but that
 * broke THINK Pascal support since THINK Pascal doesn't define the qd struct.
 * References to the QuickDraw globals are now made using SkelQD(global), which
 * can be defined to use the qd or non-qd access forms.  The qd form is still
 * the default, but the THINK Pascal version of the TransSkel library can be
 * compiled easily by supplying a non-qd definition of SkelQD().
 */


# include	<Traps.h>
# include	<GestaltEqu.h>
# include	<EPPC.h>


/*
 * TransSkel.h contains defines, typedefs, and public function
 * prototypes
 */

# include	"TransSkel.h"


/*
 * New(TypeName) returns handle to new object, for any TypeName.
 * If there is insufficient memory, the result is nil.
 */

# define	New(type)	(type **) NewHandle ((Size) sizeof (type))


/* -------------- */
/* Internal types */
/* -------------- */


/*
 * Private data types for window and menu handlers
 */

typedef struct WHandler	WHandler, *WHPtr, **WHHandle;

struct WHandler
{
	WindowPtr				whWind;		/* window/dialog to handle */
	SkelWindMouseProcPtr	whMouse;	/* mouse-click handler */
	SkelWindKeyProcPtr		whKey;		/* key-click handler */
	SkelWindUpdateProcPtr	whUpdate;	/* update handler */
	SkelWindActivateProcPtr	whActivate;	/* activate event handler */
	SkelWindCloseProcPtr	whClose;	/* close "event" handler */
	SkelWindClobberProcPtr	whClobber;	/* window disposal proc */
	SkelWindIdleProcPtr		whIdle;		/* main loop idle proc */
	SkelWindZoomProcPtr		whZoom;		/* zoom proc */
	SkelWindSelectProcPtr	whSelect;	/* item selection proc (dialog) */
	ModalFilterProcPtr			whFilter;	/* event filter proc (dialog) */
	Rect		whGrow;					/* limits on window sizing */
	Boolean		whSized;				/* true = window was resized */
	Boolean		whFrontOnly;			/* idle only when window active */
	short		whFlags;				/* various flags */
	SkelWindPropHandle	whProperties;	/* property list */
	WHHandle	whNext;					/* next window handler */
};

typedef struct MHandler	MHandler, *MHPtr, **MHHandle;

struct MHandler
{
	short					mhID;		/* menu id */
	SkelMenuSelectProcPtr	mhSelect;	/* item selection handler */
	SkelMenuClobberProcPtr	mhClobber;	/* menu disposal proc */
	Boolean		mhSubMenu;				/* whether submenu */
	MHHandle	mhNext;					/* next menu handler */
};


/* ------------------------------------------- */
/* Prototypes for internal (private) functions */
/* ------------------------------------------- */

static WHHandle GetWHandler (WindowPtr w);
static void DetachWHandler (WHHandle wh);

static void RouteEvent (EventRecord *evt);

static void DoMenuCommand (long command);
static void DoMenuHook (void);

static void DoMouse (WHHandle h, EventRecord *evt);
static void DoKey (WHHandle h, char ch, unsigned char code, short mods);
static void DoUpdate (EventRecord *evt);
static void DoActivate (EventRecord *evt);
static void DoClose (WHHandle h);
static void DoClobber (WHHandle h);
static void DoDlogEvt (DialogPtr dlog, EventRecord *evt);
static Boolean DoDlogFilter (DialogPtr dlog, EventRecord *evt);

static void DoGrow (WHHandle h, Point startPt);
static void DoZoom (WHHandle h, short partCode);


/* ------------------ */
/* Internal variables */
/* ------------------ */


/*
 * Window and menu handler variables.
 *
 * whList and mhList are the lists of window and menu handlers.
 * mhClobOnRmve is true if the menu handler disposal proc
 * is to be called when a handler is removed.  It is temporarily set
 * false when handlers are installed for menus that already
 * have handlers - the old handler is removed WITHOUT calling the
 * disposal proc.  The effect is to replace the handler for the menu
 * without destroying the menu itself.
 *
 * dragRect determines the limits on window dragging.  It is set in
 * SkelInit() to the bounding box of the desktop region inset by 4 pixels.
 *
 * growRect contains the default limits on window sizing.  It is set in
 * SkelInit().  The lower limits on window sizing of 80 pixels both directions
 * is sufficient to allow text windows room to draw a grow box and scroll
 * bars without having the thumb and arrows overlap.  The upper limits are
 * determined from the screen size. (Probably incorrectly for the case of > 1
 * screen.)
 * These default values may be changed if with SkelGrowBounds if they are
 * not appropriate.
 *
 * zoomProc is the default zoom procedure to use if the window does not have
 * one of its own.  zoomProc may be nil, in which case the default is to zoom
 * to just about full window size.
 *
 * mhDrawBarOnRmve determines whether the menu bar is redrawn by
 * SkelRmveMenu() after taking a menu out of the menu bar.  Normally
 * it's true, but SkelClobber() sets it false temporarily to avoid
 * flicker as each menu is removed.
 */


static WHHandle	whList = (WHHandle) nil;
static Rect		dragRect;
static Rect		growRect;
static SkelWindZoomProcPtr	zoomProc = (SkelWindZoomProcPtr) nil;


static MHHandle	mhList = (MHHandle) nil;
static Boolean	mhClobOnRmve = true;
static Boolean	mhDrawBarOnRmve = true;


/*
 * Miscellaneous
 *
 * - skelEnv contains SysEnvirons() information.
 * - sysVersion contains the system software version.
 * - hasGestalt is true if Gestalt() is supported.
 * - has64KROM is true if the current machine has the 64K ROM.
 * - hasGetWVariant is true if GetWVariant() is supported.
 * - mBarHeight is menu bar height.  Window sizing, zooming and dragging
 * code takes this into account.  Initialized in SkelInit(), which see
 * for teeth-gnashing over such a simple thing.
 * - doneFlag determines when SkelEventLoop() returns.  It is set by calling
 * SkelStopEventLoop(), which is how the host requests a halt.
 * - pIdle points to a background procedure, to be run during event
 * processing.  Set it with SkelSetIdle().  If nil, there's no
 * procedure.
 * - pEvent points to an event-inspecting hook, to be run whenever an
 * event occurs.  Set it with SkelSetEventHook().  If nil, there's no
 * procedure.
 * - eventMask controls the event types requested by GetNextEvent() or
 * WaitNextEvent() in SkelEventLoop().
 * - pMenuHook points to a procedure called whenever a menu selection is about
 * to be executed.  nil if no hook.
 * - diskInitPt is the location at which the disk initialization dialog
 * appears, if an uninitialized disk is inserted.
 * - eventModifiers is the value of the modifiers field of the current event.
 * - eventPtr points to the current event (nil if none seen yet).
 * - defInitParams contains the default SkelInit() parameters if caller passes
 * nil.
 */

static SysEnvRec	skelEnv;
static long	sysVersion = 0;
static Boolean	hasGestalt;
static Boolean	has64KROM;
static Boolean	hasGetWVariant;
static short	mBarHeight;
static short	doneFlag;
static short	eventMask = everyEvent ^ keyUpMask;
static short	eventModifiers = 0;
static EventRecord	*eventPtr = (EventRecord *) nil;
static Point	diskInitPt = { /* v = */ 120, /* h = */ 100 };

static SkelIdleProcPtr			pIdle = (SkelIdleProcPtr) nil;
static SkelEventHookProcPtr		pEvent = (SkelEventHookProcPtr) nil;
static SkelMenuHookProcPtr		pMenuHook = (SkelMenuHookProcPtr) nil;

static SkelInitParams	defInitParams =
{
	6,							/* no. of times to call MoreMasters() */
	(GrowZoneUPP) nil,			/* GrowZone proc */
	(SkelResumeProcPtr) nil,	/* resume proc */
	0L							/* stack adjustment */
};

/*
 * Multitasking support stuff
 *
 * hasWNE is true if WaitNextEvent() is available.
 *
 * inForeground is true if application is running in foreground (not
 * suspended).  Initially true, per PGMF 3-1.
 *
 * getFrontClicks indicates whether the application wants to receive
 * content-area clicks that bring it to the foreground.
 *
 * fgWaitTime and bgWaitTime are WaitNextEvent() times for foreground and
 * background states.
 */

static Boolean	hasWNE;
static Boolean	inForeground = true;
static long	fgWaitTime = 6L;			/* 0.1 seconds */
static long	bgWaitTime = 300L;			/* 5.0 seconds */
static Boolean	getFrontClicks = false;
static SkelSuspendResumeProcPtr	pSuspendResume = (SkelSuspendResumeProcPtr) nil;
static SkelClipCvtProcPtr	pClipCvt = (SkelClipCvtProcPtr) nil;

static WindowPtr	oldWindow = (WindowPtr) nil; 	
static WHHandle		oldWHandler = (WHHandle) nil;

/*
 * Apple Event support
 */

static Boolean	hasAppleEvents = 0;
static SkelAEHandlerProcPtr	pAEHandler = (SkelAEHandlerProcPtr) nil;


/* --------------------------- */
/* Initialization and shutdown */
/* --------------------------- */

/*
 * Initialize the various Macintosh Managers and lots of other stuff.
 *
 * FlushEvents does NOT toss disk insert events; this is so disks
 * inserted while the application is starting up don't result
 * in dead drives.
 *
 * initParams contains initialization parameters:
 * - the number of times to call MoreMasters
 * - the address of a grow zone procedure to call if memory allocation
 * problems occur (nil if none to be used)
 * - the address of a resume procedure to pass to InitDialogs()
 * (nil if none is to be used)
 * - amount to adjust the application stack size by (default 0; no adjustment)
 *
 * if initParams is nil, defaults are used.
 */

pascal void
SkelInit (SkelInitParamsPtr initParams)
{
EventRecord	dummyEvent;
Handle		h;
long		result;
short		i;

	if (initParams == (SkelInitParams *) nil)
		initParams = &defInitParams;

	if (initParams->skelGzProc != (GrowZoneUPP) nil)
		SetGrowZone (initParams->skelGzProc);

	SetApplLimit (GetApplLimit () - initParams->skelStackAdjust);

	MaxApplZone ();

	for (i = 0; i < initParams->skelMoreMasters; i++)
		MoreMasters ();

	FlushEvents (everyEvent - diskMask, 0 );
	InitGraf (&SkelQD (thePort));
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
/*
 * Cast argument according to whether or not universal headers are used
 */
# if skelUnivHeaders
	InitDialogs ((long) initParams->skelResumeProc);
# else
	InitDialogs (initParams->skelResumeProc);
# endif
	InitCursor ();

	(void) SysEnvirons (1, &skelEnv);
	
	sysVersion = (long) skelEnv.systemVersion;

	has64KROM = (skelEnv.machineType == envMac || skelEnv.machineType == envXL);

	/*
	 * If 64K ROM machine, use hard-coded value of 20.  Otherwise use
	 * Script Manager routine GetMBarHeight().  (This assumes, just to be
	 * safe, that GetMBarHeight() glue doesn't return 20 on 64K ROM systems,
	 * which it very well may.  The low memory variable MBarHeight (0x0BAA)
	 * isn't used because it doesn't exist on 64K ROM machines (TN OV 4, p.7).
	 */

	mBarHeight = (has64KROM ? 20 : GetMBarHeight ());

	/*
	 * Determine whether WaitNextEvent() is implemented (TN's OV 16 and TN TB 14)
	 */

	if (has64KROM)
		hasWNE = false;
	else
		hasWNE = SkelTrapAvailable (_WaitNextEvent);

	hasGestalt = SkelTrapAvailable (_Gestalt);
	hasAppleEvents = hasGestalt
					&& Gestalt (gestaltAppleEventsAttr, &result) == noErr
					&& (result & (1 << gestaltAppleEventsPresent));

	/*
	 * Determine whether GetWVariant() exists for checking whether a dialog is
	 * a movable modal or not.  The variant code can be gotten other ways, but
	 * the existence of trap precedes the existence of movalable modal windows,
	 * so if the trap doesn't exist, movable modals aren't likely to, either.
	 */

	hasGetWVariant = SkelTrapAvailable (_GetWVariant);

	/*
	 * Check whether application wants to get "bring to front" clicks.
	 */

	if ((h = GetResource ('SIZE', -1)) != (Handle) nil)
	{
		getFrontClicks = (((**(short **) h) & 0x200) != 0);
		ReleaseResource (h);
	}

	/*
	 * Window dragging limits are determined from bounding box of desktop.
	 * Upper limits of window sizing are related to that.  Both can involve
	 * multiple monitors, and should allow for menu bar.  dragRect is inset
	 * so as to leave at least 4 pixels of window title bar visible in both
	 * directions (IM I-289).
	 *
	 * GetGrayRgn() bounding box gives desktop extents.  On 64K ROM
	 * machines, GetGrayRgn() might not be present; could use GrayRgn
	 * bounding box, but use screenBits.bounds - menu bar, to avoid
	 * low memory access.  The two should be equivalent.
	 */

	if (has64KROM)
	{
		dragRect = SkelQD (screenBits.bounds);
		dragRect.top += mBarHeight;
	}
	else
	{
		/* GetGrayRgn () already takes menu bar into account */
		dragRect = (**GetGrayRgn ()).rgnBBox;
	}

	SetRect (&growRect, 80, 80,
				dragRect.right - dragRect.left,
				dragRect.bottom - dragRect.top);

	InsetRect (&dragRect, 4, 4);

	/* let application come to front in multitasking environment, TN TB 35, p.8 */

	(void) EventAvail (everyEvent, &dummyEvent);
	(void) EventAvail (everyEvent, &dummyEvent);
	(void) EventAvail (everyEvent, &dummyEvent);
	(void) EventAvail (everyEvent, &dummyEvent);
}


/*
 * Copy the default initialization parameters into the structure
 * pointed to by initParams.
 */

pascal void
SkelGetInitParams (SkelInitParamsPtr initParams)
{
	*initParams = defInitParams;
}


/*
 * Clobber all the menu, window and dialog handlers.  Tell SkelRmveMenu()
 * not to redraw menu bar so it doesn't flicker as menus are removed,
 * then redraw it manually.
 *
 * Before removing window handlers, hide all the windows.  Do this from
 * back to front (more esthetic and speedier).  If a window belongs to a DA,
 * close the DA.  (For early systems (e.g., 4.1), if you leave a DA open,
 * the system crashes the next time you try to open that DA.)
 */

pascal void
SkelCleanup (void)
{
Boolean	oldFlag;
short	theKind;
WindowPeek	w;
WindowPtr	lastVis;

	for (;;)
	{
		lastVis = (WindowPtr) nil;
		for (w = (WindowPeek) FrontWindow (); w != (WindowPeek) nil; w = w->nextWindow)
		{
			if (w->visible)
				lastVis = (WindowPtr) w;
		}
		if (lastVis == (WindowPtr) nil)		/* no more visible windows */
			break;
		if (lastVis != (WindowPtr) nil)
		{
			theKind = ((WindowPeek) lastVis)->windowKind;
			if (theKind < 0)				/* DA, close it */
				CloseDeskAcc (theKind);
			else
				HideWindow (lastVis);
		}
	}

	while (whList != (WHHandle) nil)
		SkelRmveWind ((**whList).whWind);

	oldFlag = mhDrawBarOnRmve;
	mhDrawBarOnRmve = false;
	while (mhList != (MHHandle) nil)
		SkelRmveMenu (GetMHandle((**mhList).mhID));
	mhDrawBarOnRmve = oldFlag;
	DrawMenuBar ();
}


/* ----------------------------------- */
/* Execution environment interrogation */
/* ----------------------------------- */



#define trapMask	0x0800

static short
NumToolboxTraps (void)
{
	if (NGetTrapAddress (_InitGraf, ToolTrap)
		== NGetTrapAddress (0xaa6e, ToolTrap))
		return (0x200);
	return (0x400);
}


static TrapType
GetTrapType (short theTrap)
{
	return ((theTrap & trapMask) ? ToolTrap : OSTrap);
}


pascal Boolean
SkelTrapAvailable (short theTrap)
{
TrapType	tType;

	if ((tType = GetTrapType (theTrap)) == ToolTrap)
	{
		theTrap &= 0x07ff;
		if (theTrap >= NumToolboxTraps ())
			theTrap = _Unimplemented;
	}
	return (NGetTrapAddress (theTrap, tType)
				!= NGetTrapAddress (_Unimplemented, ToolTrap));
}


/*
 * Query the TransSkel execution environment.  Shouldn't be called until
 * after SkelInit() has been called.  Result is undefined if selector isn't
 * legal.
 */

pascal long
SkelQuery (short selector)
{
long	result;
Rect	r;
RgnHandle	rgn;

	switch (selector)
	{
	case skelQVersion:
		result = ((long) skelMajorRelease << 16) | skelMinorRelease;
		break;
	case skelQSysVersion:
		result = sysVersion;
		break;
	case skelQHasWNE:
		result = hasWNE ? 1 : 0;
		break;
	case skelQHas64KROM:
		result = has64KROM ? 1 : 0;
		break;
	case skelQMBarHeight:
		result = mBarHeight;
		break;
	case skelQHasColorQD:
		result = skelEnv.hasColorQD ? 1 : 0;
		break;
	case skelQQDVersion:
		/* get QuickDraw version number */
		if (!hasGestalt
			|| Gestalt (gestaltQuickdrawVersion, &result) != noErr)
			result = 0;					/* assume original QuickDraw */
		break;
	case skelQInForeground:
		result = inForeground ? 1 : 0;
		break;
	case skelQHasGestalt:
		result = hasGestalt ? 1 : 0;
		break;
	case skelQHasAppleEvents:
		result = hasAppleEvents ? 1 : 0;
		break;
	case skelQGrayRgn:
		rgn = NewRgn ();
		if (rgn != (RgnHandle) nil)
		{
			if (has64KROM)
			{
				r = SkelQD (screenBits.bounds);
				r.top += mBarHeight;
				RectRgn (rgn, &r);
			}
			else
			{
				/* GetGrayRgn () already takes menu bar into account */
				CopyRgn (GetGrayRgn (), rgn);
			}
		}
		result = (long) rgn;
		break;
	default:
		/* result is undefined! */
		break;
	}
	return (result);
}

/* ------------------------------------- */
/* Event loop initiation and termination */
/* ------------------------------------- */


/*
 * Main event loop.
 *
 * - Take care of DA's with SystemTask() if necessary.
 * - Get an event.
 * - Pass event to event router.
 *	
 * doneFlag is restored to its previous value upon exit.  This allows
 * SkelEventLoop() to be called recursively.
 */

pascal void
SkelEventLoop (void)
{
EventRecord	evt;
Boolean		oldDoneFlag;
long		waitTime;

	oldDoneFlag = doneFlag;		/* save in case this is a recursive call */
	doneFlag = false;			/* set for this call */
	while (!doneFlag)
	{
		if (hasWNE)
		{
			waitTime = (inForeground ? fgWaitTime : bgWaitTime);
			(void) WaitNextEvent (eventMask, &evt, waitTime, nil);
		}
		else
		{
			/*
			 * On some early versions of the system software, it cannot
			 * be assumed that the event contains a null event if the
			 * GetNextEvent() return value is false.  GetNextEvent() calls
			 * SystemEvent() to handle some DA events, and returns false
			 * if the event was handled.  However, in such cases the event
			 * record may still have the event that occurred, *not* a null
			 * event.  To avoid problems later with misinterpreting the
			 * event as non-null, force it to look like a null event.
			 */
			SystemTask ();
			if (!GetNextEvent (eventMask, &evt))
				evt.what = nullEvent;
		}

			SkelRouteEvent (&evt);
	}
	doneFlag = oldDoneFlag;	/* restore in case this was a recursive call */
}


/*
 * Tell current instance of SkelEventLoop() to drop dead
 */

pascal void
SkelStopEventLoop (void)
{
	doneFlag = true;
}


/* ----------------- */
/* Event dispatching */
/* ----------------- */


/*
 * Route a single event and run window idle procedures.
 *
 * If the event is a null-event, call the "no-event" handler for the front
 * window and for any other windows with idle procedures that are always
 * supposed to run.  This is done in such a way that it is safe for idle
 * procs to remove the window handler for their own window if they want
 * (unlikely, but...).
 */

pascal void
SkelRouteEvent (EventRecord *evt)
{
WHHandle	wh, wh2;
GrafPtr		tmpPort;
WindowPtr	w;
SkelWindIdleProcPtr	p;

	RouteEvent (evt);

	/*
	 * Run applicable window idle procs.  Make sure to save and restore
	 * the port, since idle procs for the non-active window may be run.
	 */

	if (evt->what == nullEvent)
	{
		GetPort (&tmpPort);
		for (wh = whList; wh != (WHHandle) nil; wh = wh2)
		{
			wh2 = (**wh).whNext;
			w = (**wh).whWind;
			if (w == FrontWindow () || !(**wh).whFrontOnly)
			{
				if ((p = (**wh).whIdle) != (SkelWindIdleProcPtr) nil)
				{
					if (!hasWNE)
						SystemTask ();
					SetPort (w);
					(*p) ();
				}
			}
		}
		SetPort (tmpPort);
	}
}


/*
 * General event dispatch routine.
 *
 * If there is an event-handling hook and it handles the event, the
 * event is not further processed here.  Otherwise, run the application's idle
 * time procedure if the event is a null event, then process the event.
 *
 * Null events are sent through DialogSelect() if a dialog is active.  This
 * is necessary to make sure  the caret blinks if a dialog has any editText
 * items.
 *
 * Network events are not supported as per the deprecation in TN NW 07.
 * Application-defined events 1, 2 and 3 are not handled, either.
 */

static void
RouteEvent (EventRecord *evt)
{
Point		evtPt;
GrafPtr		evtPort;
short		evtPart;
short		evtMods;
char		evtChar;
long		evtMsge;
unsigned char evtCode;
WHHandle	wh;
WindowPtr	frontWind;
Boolean		frontIsDlog;
short		osMsge;
Boolean		osResume;
Boolean		osClipCvt;
Rect		r1, r2;
WStateData	**wdh;
SignedByte	state;

	/* save values for SkelGetCurrentEvent() and SkelGetModifiers() */

	eventPtr = evt;
	eventModifiers = evt->modifiers;

	/* don't bother handling event below if event hook does so here */
	
	if (pEvent != (SkelEventHookProcPtr) nil && (*pEvent) (evt))
		return;

	frontWind = FrontWindow ();
	frontIsDlog = SkelIsDlog (frontWind);

	evtPt = evt->where;
	evtMods = evt->modifiers;
	evtMsge = evt->message;

	switch (evt->what)
	{
	case nullEvent:
		/*
		 * Run the application idle-time function.  If the front window is
		 * a dialog window, pass the event to the dialog event handler; this
		 * is necessary to make the caret blink if it has an edit text item.
		 * Don't use frontWind after calling the idle-time function, since
		 * the function might change the front window!
		 */
		if (pIdle != (SkelIdleProcPtr) nil)
			(*pIdle) ();
		if (SkelIsDlog (FrontWindow ()))
			DoDlogEvt (FrontWindow (), evt);
		break;

	/*
	 * Mouse click.  Get the window in which the click occurred, and
	 * the part of the window.
	 */
	case mouseDown:
		evtPart = FindWindow (evtPt, &evtPort);
		wh = GetWHandler (evtPort);

		/*
		 * Beep if a click occurs outside of a movable modal dialog box.
		 * Exceptions: allow clicks in menu bar, and command-clicks in
		 * drag region of underlying windows.
		 */

		if (SkelIsMMDlog (frontWind)
			&& !PtInRgn (evtPt, ((WindowPeek) frontWind)->strucRgn))
		{
			if (evtPart != inMenuBar
				&& !(evtPart == inDrag && evtPort != frontWind && (evtMods & cmdKey)))
			{
				SysBeep (1);
				break;
			}
		}

		switch (evtPart)
		{

		/*
		 * Click in desk accessory window.  Pass back to the system.
		 */
		case inSysWindow:
			SystemClick (evt, evtPort);
			break;

		/*
		 * Click in menu bar.  Track the mouse and execute
		 * selected command, if any.
		 */
		case inMenuBar:
			DoMenuHook ();
			DoMenuCommand (MenuSelect (evtPt));
			break;

		/*
		 * Click in grow box.  Resize window.
		 */
		case inGrow:
			DoGrow (wh, evtPt);
			break;

		/*
		 * Click in title bar.  Drag the window around.
		 * Problem fix:  DragWindow() seems to call StillDown()
		 * first, so that clicks in drag regions while machine is
		 * busy don't otherwise bring window to front if the mouse
		 * is already up by the time DragWindow() is called.  So the
		 * window is selected first to make sure it's at least
		 * activated (unless the command key is down, IM I-289).
		 *
		 * Also offset the window's userState by the amount of the drag
		 * (it'd be simpler to set it to the final content rect but the
		 * window might be in zoomed state rather than user state).
		 */
		case inDrag:
			if (evtPort != frontWind && (evtMods & cmdKey) == 0)
				SelectWindow (evtPort);
			SkelGetWindContentRect (evtPort, &r1);		/* post-drag position */
			DragWindow (evtPort, evtPt, &dragRect);
			SkelGetWindContentRect (evtPort, &r2);		/* post-drag position */
			wdh = (WStateData **)(((WindowPeek) evtPort)->dataHandle);
			state = HGetState ((Handle) wdh);
			HLock ((Handle) wdh);
			OffsetRect (&(**wdh).userState, r2.left - r1.left, r2.top - r1.top);
			HSetState ((Handle) wdh, state);
			break;

		/*
		 * Click in close box.  Call the close proc if the window
		 * has one.
		 */
		case inGoAway:
			if (TrackGoAway (evtPort, evtPt))
				DoClose (wh);
			break;

		/*
		 * Click in zoom box.  Track the click and then zoom the
		 * window if necessary.
		 */
		case inZoomIn:
		case inZoomOut:
			if (TrackBox (evtPort, evtPt, evtPart))
				DoZoom (wh, evtPart);
			break;

		/*
		 * Click in content region.  If the window wasn't frontmost
		 * (active), just select it, otherwise pass the click to the
		 * window's mouse click handler.  Exception: if the application
		 * wants to receive content clicks event in non-frontmost windows,
		 * select the window and "repeat" the click.
		 */
		case inContent:
			if (evtPort != frontWind)
			{
				SelectWindow (evtPort);
				if (!getFrontClicks)	/* don't pass click to handler */
					break;
				SetPort (evtPort);
			}
			if (frontIsDlog)
				DoDlogEvt (evtPort, evt);
			else
				DoMouse (wh, evt);
			break;

		}
		break;	/* mouseDown */

	/*
	 * Key down event.  If the command key was down, process as menu
	 * item selection, otherwise pass the character and the modifiers
	 * flags to the active window's key handler.
	 *
	 * Command-period is not supposed to be used as a menu-item equivalent.
	 * Consequently, that's noticed as a special case and not passed to
	 * the menu routines.
	 */
	case keyDown:
	case autoKey:
		evtChar = evtMsge & charCodeMask;
		evtCode = (evtMsge & keyCodeMask) >> 8;	/* hope bit 7 isn't set! */

		if ((evtMods & cmdKey) && !SkelCmdPeriod (evt))	/* try menu equivalent */
		{
			DoMenuHook ();
			DoMenuCommand (MenuKey (evtChar));
			break;
		}

		if (frontIsDlog)
			DoDlogEvt (frontWind, evt);
		else
			DoKey (GetWHandler (frontWind), evtChar, evtCode, evtMods);
		break;

	/*
	 * Key up event.  Key-ups are signified by setting the high bit
	 * of the key code.  This never executes unless the application
	 * changes the system event mask *and* the TransSkel event mask.
	 */
	case keyUp:
		evtChar = evtMsge & charCodeMask;			/* probably 0? */
		evtCode = ((evtMsge & keyCodeMask) >> 8) | 0x80;

		if (frontIsDlog)
			DoDlogEvt (frontWind, evt);
		else
			DoKey (GetWHandler (frontWind), evtChar, evtCode, evtMods);
		break;

	/*
	 * Update a window.
	 */
	case updateEvt:
		DoUpdate (evt);
		break;

	/*
	 * Activate or deactivate a window.
	 */
	case activateEvt:
		DoActivate (evt);
		break;

	/*
	 * handle inserts of uninitialized disks.  Deactivate the frontmost
	 * window since the disk-init dialog doesn't do anything with
	 * activate events for other windows.
	 */
	case diskEvt:
		if (HiWord (evtMsge) != noErr)
		{
			SkelActivate (FrontWindow (), false);
			DILoad ();
			(void) DIBadMount (diskInitPt, evtMsge);
			DIUnload ();
		}
		break;

	case osEvt:				/* aka app4Evt aka MultiFinder event */
		/* rip the message field into constituent parts */
		osMsge = ((evtMsge >> 24) & 0xff);			/* high byte */
		osResume = (Boolean) ((evtMsge & resumeFlag) != 0);
		osClipCvt = (Boolean) ((evtMsge & convertClipboardFlag) != 0);

		switch (osMsge)
		{
		case suspendResumeMessage:
			/*
			 * Tell application it's being suspended or resumed
			 * Tell application to convert scrap if necessary
			 */
		
			inForeground = osResume;
			if (pSuspendResume != (SkelSuspendResumeProcPtr) nil)
				(*pSuspendResume) (inForeground);
			if (!osResume)			/* always convert on suspend */
				osClipCvt = true;
			if (osClipCvt && pClipCvt != (SkelClipCvtProcPtr) nil)
				(*pClipCvt) (inForeground);
			break;

		case mouseMovedMessage:
			/* recompute mouse region -- not implemented */
			break;

		/*
		 * 0xfd is a child-died event -- not implemented here since it's
		 * only had limited use, e.g., by certain debuggers.  The child pid
		 * is byte 2 ((evtMsge >> 16) & 0xff)
		case 0xfd:
			break;
		 */

		default:				/* other OS event */
			/* pass event to catch-all handler -- not implemented */
			break;
		}
		break;

	case kHighLevelEvent:
		if (pAEHandler != (SkelAEHandlerProcPtr) nil)
			(*pAEHandler) (evt);
		break;
	}
}


/*
 * Activate or deactivate a window by synthesizing a fake
 * activate event and sending it through the event router.
 * Useful for activating a window when you don't know its
 * activate function.
 */

pascal void
SkelActivate (WindowPtr w, Boolean active)
{
EventRecord	evt;

	if (w != (WindowPtr) nil)
	{
		evt.what = activateEvt;
		evt.modifiers = active ? activeFlag : 0;
		evt.when = TickCount ();
		SetPt (&evt.where, 0, 0);
		evt.message = (long) w;
		SkelRouteEvent (&evt);
	}
}


/*
 * Call a window's close procedure.  Useful for closing a window when you
 * don't know its close function.
 *
 * This function knows how to close Desk Accessories.
 */

pascal void
SkelClose (WindowPtr w)
{
	if (w != (WindowPtr) nil)
	{
		if (((WindowPeek) w)->windowKind < 0)		/* DA window */
			CloseDeskAcc (((WindowPeek) w)->windowKind);
		else
			DoClose (GetWHandler (w));
	}
}


/*
 * Set the TransSkel event mask.  Does not have anything to do with the
 * system event mask.  See TPN 3.
 */

pascal void
SkelSetEventMask (short mask)
{
	eventMask = mask;
}


/*
 * Return the event mask.
 */

pascal short
SkelGetEventMask (void)
{
	return (eventMask);
}


/*
 * Install an idle-time task.  If p is nil, the current task is
 * disabled.
 */

pascal void
SkelSetIdle (SkelIdleProcPtr p)
{
	pIdle = p;
}


/*
 * Return the current idle-time task.  Return nil if none.
 */

pascal SkelIdleProcPtr
SkelGetIdle (void)
{
	return (pIdle);
}


/*
 * Install an event-inspecting hook.  If p is nil, the hook is
 * disabled.
 */

pascal void
SkelSetEventHook (SkelEventHookProcPtr p)
{
	pEvent = p;
}


/*
 * Return the current event-inspecting hook.  Return nil if none.
 */

pascal SkelEventHookProcPtr
SkelGetEventHook (void)
{
	return (pEvent);
}


pascal void
SkelSetSuspendResume (SkelSuspendResumeProcPtr p)
{
	pSuspendResume = p;
}


pascal SkelSuspendResumeProcPtr
SkelGetSuspendResume (void)
{
	return (pSuspendResume);
}


pascal void
SkelSetClipCvt (SkelClipCvtProcPtr p)
{
	pClipCvt = p;
}


pascal SkelClipCvtProcPtr
SkelGetClipCvt (void)
{
	return (pClipCvt);
}


pascal void
SkelSetWaitTimes (long fgTime, long bgTime)
{
	fgWaitTime = fgTime;
	bgWaitTime = bgTime;
}


pascal void
SkelGetWaitTimes (long *pFgTime, long *pBgTime)
{
	if (pFgTime != (long) nil)
		*pFgTime = fgWaitTime;
	if (pBgTime != (long) nil)
		*pBgTime = bgWaitTime;
}


pascal EventRecord *
SkelGetCurrentEvent (void)
{
	return (eventPtr);
}


pascal short
SkelGetModifiers (void)
{
	return (eventModifiers);
}


pascal void
SkelSetAEHandler (SkelAEHandlerProcPtr p)
{
	pAEHandler = p;
}


pascal SkelAEHandlerProcPtr
SkelGetAEHandler (void)
{
	return (pAEHandler);
}


/* -------------------------------------------------------------------- */
/*					Window-handler event routing routines				*/
/*																		*/
/*	See manual for discussion of port-setting behavior: the current		*/
/*	port is set to a window when it becomes active in DoActivate().		*/
/* -------------------------------------------------------------------- */


/*
 * Process dialog event.  dlog is the dialog to which the event applies.
 * Give the filter a chance at the event first.  If the filter doesn't
 * handle it, pass the event to DialogSelect().  If DialogSelect() selects
 * an item, pass the item to the window's item selection function, if
 * there is one.  This is used to dispose of dialog events that aren't
 * handled in some other more direct fashion.
 */


static void
DoDlogEvt (DialogPtr dlog, EventRecord *evt)
{
short		item;
WHHandle	wh;
SkelWindSelectProcPtr	select;

	if (DoDlogFilter (dlog, evt))
		return;

	if (DialogSelect (evt, &dlog, &item)
	   && (wh = GetWHandler (dlog)) != (WHHandle) nil
	   && (select = (**wh).whSelect) != (SkelWindSelectProcPtr) nil)
	{
		(*select) (dlog, item);
	}
}

/*
 * Run a dialog's filter function to give the filter first chance
 * at the event.
 *
 * The filter function returns false if it doesn't handle the event.
 * It returns true if it handled the event, in which case it should
 * set the item parameter.  The item will be passed to the dialog's
 * item selection function.
 *
 * If the filter function returns true, look up the handler again
 * just in case the filter function also called SkelRmveDlog().
 * If it did, the handler will have become invalid.  Looking it
 * up again avoids disaster.
 */

static Boolean
DoDlogFilter (DialogPtr dlog, EventRecord *evt)
{
short		item;
WHHandle	wh;
SkelWindSelectProcPtr	select;
ModalFilterProcPtr		filter;
Boolean	result = false;

	if ((wh = GetWHandler (dlog)) != (WHHandle) nil
		&& (filter = (**wh).whFilter) != (ModalFilterProcPtr) nil)
	{
		if ((*(ModalFilterProcPtr) filter) (dlog, evt, &item))
		{
			if ((wh = GetWHandler (dlog)) != (WHHandle) nil
				&& (select = (**wh).whSelect) != (SkelWindSelectProcPtr) nil)
			{
				(*select) (dlog, item);
			}
			result = true;
		}
	}
	return (result);
}


/*
 * Pass local mouse coordinates, click time, and the modifiers flag
 * word to the handler.  Should not be necessary to set the port, as
 * the click is passed to the active window's handler.
 */

static void
DoMouse (WHHandle h, EventRecord *evt)
{
Point	thePt;

	if (h != (WHHandle) nil && (**h).whMouse != (SkelWindMouseProcPtr) nil)
	{
		thePt = evt->where;	/* make local copy */
		GlobalToLocal (&thePt);
		(*(**h).whMouse) (thePt, evt->when, evt->modifiers);
	}
}


/*
 * Pass the character code, key code and the modifiers flag word to
 * the handler. Should not be necessary to set the port, as the click
 * is passed to the active window's handler.
 */

static void
DoKey (WHHandle h, char c, unsigned char code, short mods)
{
	if (h != (WHHandle) nil && (**h).whKey != (SkelWindKeyProcPtr) nil)
		(*(**h).whKey) ((short) c, (short) code, mods);
}


/*
 * Call the window updating procedure, passing to it an indicator whether
 * the window has been resized or not.  Then clear the flag, assuming
 * the update proc took whatever action was necessary to respond to
 * resizing.
 *
 * The Begin/EndUpdate stuff is done to clear the update region even if
 * the handler doesn't have any update proc.  Otherwise the Window
 * Manager will keep generating update events for the window, stalling  
 * updates of other windows.
 *
 * For dialog windows, UpdtDialog() does the normal item updating.  The
 * filter procedure can take care of non-item drawing, such as a bold
 * outline around a default button.
 *
 * Saves, sets, and restore the port, since it's not always the
 * active window that is updated.
 */

static void
DoUpdate (EventRecord *evt)
{
WHHandle	h;
GrafPtr	port;
GrafPtr	tmpPort;

	port = (WindowPtr) evt->message;

	GetPort (&tmpPort);
	SetPort (port);
	BeginUpdate (port);
	if (SkelIsDlog (port))
	{
		if (!DoDlogFilter (port, evt))
			UpdtDialog (port, port->visRgn);	/* let Dialog Manager finish update */
	}
	else
	{
		h = GetWHandler (port);
		if (h != (WHHandle) nil)
		{
			if ((**h).whUpdate != (SkelWindUpdateProcPtr) nil)
				(*(**h).whUpdate) ((**h).whSized);
			(**h).whSized = false;
		}
	}
	EndUpdate (port);
	SetPort (tmpPort);
}


/*
 * Pass activate/deactivate notification to handler.  On activate,
 * set the port to the window coming active.  Normally this is done by
 * the user clicking in a window.
 *
 * *** BUT ***
 * Under certain conditions, a deactivate may be generated for a window
 * that has never had the port set to it by a preceding activate.  For
 * instance, if an application puts up window A, then window B in front
 * of A, then starts processing events, the first events will be a
 * deactivate for A and an activate for B.  Therefore, since it can't be
 * assumed that an activate ever set the port to A, the port needs to be
 * set for deactivates as well so drawing occurs in the correct port.
 *
 * This matters not a whit for the more usual cases that occur.  If a
 * deactivate for one window is followed by an activate for another, the
 * port will still be switched properly to the newly active window.  If
 * no activate follows the deactivate, the deactivated window is the last
 * one, and it doesn't matter what the port ends up set to, anyway.
 *
 * On deactivate, port is saved and restored in case deactivate is due to
 * a modal dialog having been brought in front and port changed to it
 * explicitly by the application.  The deactivate shouldn't leave the port
 * changed away from the dialog!
 *
 * For dialogs, DoDlogEvt() is called, allowing DialogSelect() to do
 * whatever it does for dialog activates.  The handler's activate procedure
 * is called in addition to this (e.g., to hilite controls or text selections,
 * adjust menus).
 */

static void
DoActivate (EventRecord *evt)
{
WHHandle	h;
GrafPtr	port;
GrafPtr	tmpPort;
Boolean	active;

	active = (evt->modifiers & activeFlag);
	port = (WindowPtr) evt->message;


	GetPort (&tmpPort);	/* save so can restore if deactivate */
	SetPort (port);
	if (SkelIsDlog (port))
		DoDlogEvt (port, evt);
	else
	{
		h = GetWHandler (port);
		if (h != (WHHandle) nil)
		{
			if ((**h).whActivate != (SkelWindActivateProcPtr) nil)
				(*(**h).whActivate) (active);
		}
	}
	if (!active)
		SetPort (tmpPort);
}


/*
 * Execute a window handler's close box proc.  The close proc for
 * handlers for temp windows that want to remove themselves when the
 * window is closed can call SkelRmveWind to dispose of the window
 * and remove the handler from the window handler list.  Thus, windows
 * may be dynamically created and destroyed without filling up the
 * handler list with a bunch of invalid handlers.
 *
 * If the handler doesn't have a close proc, just hide the window.
 * The host should provide some way of reopening the window (perhaps
 * a menu selection).  Otherwise the window will be lost from user
 * control if it is hidden, since it won't receive user-initiated
 * events.
 *
 * This is called both for regular and dialog windows.
 *
 * Normally this is invoked because the close box of the active window
 * is clicked, in which case the port will be set to the window.  However,
 * SkelClose() allows the application to close an aritrary window, not just
 * the frontmost one -- so the port is saved and restored.
  */

static void
DoClose (WHHandle h)
{
GrafPtr	tmpPort;

	if (h != (WHHandle) nil)
	{
		GetPort (&tmpPort);
		SetPort ((**h).whWind);
		if ((**h).whClose != (SkelWindCloseProcPtr) nil)
			(*(**h).whClose) ();
		else
			HideWindow ((**h).whWind);
		SetPort (tmpPort);
	}
}


/*
 * Execute a window handler's clobber proc.  This is called both
 * for regular and dialog windows.
 *
 * Saves sets, and restores the port, since any window (not just active
 * one) may be clobbered at any time.
 *
 * Don't need to check whether handler is nil, as in other handler
 * procedures, since this is only called by SkelRmveWind with a
 * known-valid handler.
 */

static void
DoClobber (WHHandle h)
{
GrafPtr	tmpPort;

	GetPort (&tmpPort);
	SetPort ((**h).whWind);
	if ((**h).whClobber != (SkelWindClobberProcPtr) nil)
		(*(**h).whClobber) ();
	SetPort (tmpPort);
}


/*
 * Handlers for window events not requiring application handler routines
 * to be called.
 */


/*
 * Have either zoomed a window or sized it manually.  Invalidate
 * it to force an update and set the 'resized' flag in the window
 * handler true.  The port is assumed to be set to the port that changed
 * size.  Handler is assumed non-nil.
 */

static void
TriggerUpdate (WHHandle h)
{
GrafPtr	port = (**h).whWind;

	InvalRect (&port->portRect);
	(**h).whSized = true;
}


/*
 * Size a window, using the grow limits in the handler record.
 *
 * The portRect is invalidated to force an update event.  The window's
 * update handler procedure should check the parameter passed to it to
 * check whether the window has changed size, if it needs to adjust
 * itself to the new size.  THIS IS A CONVENTION.  Update procs must
 * notice grow "events", there is no procedure specifically for that.
 *
 * The clipping rectangle is not reset.  If the host application
 * keeps the clipping set equal to the portRect or something similar,
 * then it will have to arrange to treat window growing with more
 * care.
 *
 * Since the grow region of only the active window may be clicked,
 * it should not be necessary to set the port.
 */

static void
DoGrow (WHHandle h, Point startPt)
{
GrafPtr	growPort;
Rect	growRect;
long	growRes;

	if (h != (WHHandle) nil)
	{
		growPort = (**h).whWind;
		growRect = (**h).whGrow;

		/* growRes will be zero if the size was not actually changed */
	
		if (growRes = GrowWindow (growPort, startPt, &growRect))
		{
			SizeWindow (growPort, LoWord (growRes), HiWord (growRes), false);
			TriggerUpdate (h);
		}
	}
}


/*
 * Zoom the current window.  Very similar to DoGrow, but window
 * is erased before zooming for nicer visual effect (per IM IV-50,
 * TN TB 30, p.4).
 * 
 * Normally, since only the active window has a visible zoom box and
 * TransSkel sets the port to active window, this routine is triggered
 * by user-initiated clicks in zoom box and the port will be set to
 * the zoomed window.
 *
 * However, it is possible for zooms to be software initiated by the
 * application itself on any window; for such cases the port needs
 * to be saved and set before the zoom and restored afterward.
 */

static void
DoZoom (WHHandle h, short zoomDir)
{
GrafPtr	w;
GrafPtr	tmpPort;
Rect	r, growRect;

	if (h != (WHHandle) nil)
	{
		w = (**h).whWind;
		GetPort (&tmpPort);					/* save port and set to */
		SetPort (w);						/* zoomed window */
		if ((**h).whZoom != (SkelWindZoomProcPtr) nil)
			((**h).whZoom) (w, zoomDir);	/* custom zoom proc */
		else if (zoomProc != (SkelWindZoomProcPtr) nil)
			(*zoomProc) (w, zoomDir);		/* custom default zoom proc */
		else								/* default zooming */
		{
			EraseRect (&w->portRect);
			if (zoomDir == inZoomOut)	/* zooming to default state */
			{
				/*
				 * Get the usable area of the device containing most of the
				 * window.  (Can ignore the result because the rect is always
				 * correct.  Pass nil for device parameter because it's
				 * irrelevant.)  Then adjust rect for title bar height, and
				 * inset it slightly.
				 */
				(void) SkelGetWindowDevice (w, (GDHandle *) nil, &r);
				r.top += SkelGetWindTitleHeight (w) - 1;
				/* leave 3-pixel border */
				InsetRect (&r, 3, 3);
				/* clip to grow limits */
				growRect = (**h).whGrow;
				growRect.left = growRect.top = 0;
				OffsetRect (&growRect, r.left, r.top);
				SectRect (&r, &growRect, &r);
				(**(WStateData **)(((WindowPeek)w)->dataHandle)).stdState = r;
			}
			ZoomWindow (w, zoomDir, false);
		}
		SetPort (tmpPort);		/* restore original port */
		TriggerUpdate (h);
	}
}


/* --------------------------------------------------------- */
/* Window handler installation/removal/modification routines */
/* --------------------------------------------------------- */


/*
 * Install handler for a window and set current port to it.  Remove
 * any previous handler for it.  Pass the following parameters:
 *
 * w
 *		Pointer to the window to be handled.  Must be created by host.
 * doMouse
 *		Proc to handle mouse clicks in window.  The proc will be
 * 		passed the point (in local coordinates), the time of the
 * 		click, and the modifier flags word.
 * doKey
 *		Proc to handle key clicks in window.  The proc will be passed
 * 		the character and the modifier flags word.
 * doUpdate
 *		Proc for updating window.  TransSkel brackets calls to update
 * 		procs with calls to BeginUpdate and EndUpdate, so the visRgn
 * 		is set up correctly.  A flag is passed indicating whether the
 * 		window was resized or not.  BY CONVENTION, the entire portRect
 * 		is invalidated when the window is resized or zoomed.  That way,
 * 		the handler's update proc can redraw the entire content region
 * 		without interference from BeginUpdate/EndUpdate.  The flag
 * 		is set to false after the update proc is called; the
 * 		assumption is made that the proc will notice the resizing and
 * 		respond appropriately.
 * doActivate
 *		Proc to execute when window is activated or deactivated.
 * 		A boolean is passed to it which is true if the window is
 * 		coming active, false if it's going inactive.
 * doClose
 *		Proc to execute when mouse clicked in close box.  Useful
 * 		mainly to temp window handlers that want to know when to
 * 		self-destruct (with SkelRmveWind).
 * doClobber
 *		Proc for disposal of handler's data structures
 * doWIdle
 *		Proc to execute when no events are pending.
 * idleFrontOnly
 *		True if doWIdle should execute on no events only when
 * 		w is frontmost, false if executes all the time.  Note
 * 		that if it always goes, everything else may be slowed down!
 *
 * If a particular procedure is not needed (e.g., key events are
 * not processed by a handler), pass nil in place of the appropriate
 * procedure address.
 *
 * Return true if successful, false if no handler could be allocated.
 * If false is returned, the port will not have been changed.
 */
 
pascal Boolean
SkelWindow (WindowPtr w,
			SkelWindMouseProcPtr doMouse,
			SkelWindKeyProcPtr doKey,
			SkelWindUpdateProcPtr doUpdate,
			SkelWindActivateProcPtr doActivate,
			SkelWindCloseProcPtr doClose,
			SkelWindClobberProcPtr doClobber,
			SkelWindIdleProcPtr doWIdle,
			Boolean idleFrontOnly)
{
WHHandle	whNew, whCur;
SkelWindPropHandle	wph = (SkelWindPropHandle) nil;

	/* Get new handler immediately, fail if can't allocate */

	if ((whNew = New (WHandler)) == (WHHandle) nil)
		return (false);

	/*
	 * If there's a current handler for the window, remove it, but first
	 * grab the property list from it so it can be transferred to the new
	 * handler.
	 */
	
	if ((whCur = GetWHandler (w)) != (WHHandle) nil)
	{
		wph = (**whCur).whProperties;
		(**whCur).whProperties = (SkelWindPropHandle) nil;
		DetachWHandler (whCur);
	}

	/*
	 * Attach new handler to list of handlers.  It is attached to the
	 * beginning of the list, which is simpler; the order is presumably
	 * irrelevant to the host, anyway.
	 *
	 * Then fill in handler fields (including properties attached to any
	 * previous handler).
	 */

	(**whNew).whNext = whList;
	whList = whNew;

	(**whNew).whWind = w;
	(**whNew).whMouse = doMouse;
	(**whNew).whKey = doKey;
	(**whNew).whUpdate = doUpdate;
	(**whNew).whActivate = doActivate;
	(**whNew).whClose = doClose;
	(**whNew).whClobber = doClobber;
	(**whNew).whZoom = (SkelWindZoomProcPtr) nil;
	(**whNew).whIdle = doWIdle;
	(**whNew).whGrow = growRect;
	(**whNew).whSized = false;
	(**whNew).whFrontOnly = idleFrontOnly;
	(**whNew).whFlags = 0;
	(**whNew).whProperties = wph;
	(**whNew).whSelect = (SkelWindSelectProcPtr) nil;
	(**whNew).whFilter = (ModalFilterProcPtr) nil;
	SetPort (w);

	return (true);
}


/*
 * Remove a window handler.  This calls the handler's window disposal
 * routine and then takes the handler out of the handler list and
 * disposes of it (including its property list).
 *
 * SkelRmveWind is also called by SkelRmveDlog.
 */

pascal void
SkelRmveWind (WindowPtr w)
{
WHHandle	h;

	if ((h = GetWHandler (w)) == (WHHandle) nil)
		return;

	DoClobber (h);								/* call disposal routine */
	SkelRmveWindProp (w, skelWPropAll);	/* toss properties */

	DetachWHandler (h);	/* remove handler for window from list */
}


/*
 * Install a handler for a modeless or movable modal dialog window and set
 * the port to it.  Remove any previous handler for it. SkelDialog calls
 * SkelWindow as a subsidiary to install a window handler, then sets
 * the event procedure on return.  A property is also added to the window
 * to indicate that it's a modeless or movable modal dialog.
 *
 * Pass the following parameters:
 *
 * dlog
 *		Pointer to the dialog to be handled.  Must be created by host.
 * doFilter
 *		Filter procedure to look at events before they are otherwise
 *		processed.
 * doSelect
 *		Procedure to execute when an item is "selected" (e.g., a mouse
 *		click occurs in it).
 * doClose
 *		Procedure to execute when mouse clicked in close box.  Useful
 * 		mainly to dialog handlers that want to know when to
 * 		self-destruct (with SkelRmveDlog).
 * doClobber
 *		Procedure for disposal of handler's data structures
 *
 * If a particular procedure is not needed, pass nil in place of
 * the appropriate procedure address.
 *
 * Return true if successful, false if no handler could be allocated.
 * If false is returned, the port will not have been changed.
 */

pascal Boolean
SkelDialog (DialogPtr dlog,
			ModalFilterProcPtr doFilter,
			SkelWindSelectProcPtr doSelect,
			SkelWindCloseProcPtr doClose,
			SkelWindClobberProcPtr doClobber)
{
WHHandle	wh;
short	propType;

	if (!SkelWindow (dlog, nil, nil, nil, nil, doClose, doClobber, nil, true))
		return (false);

	/*
	 * Determine dialog property type to add to window and add the property,
	 * unless it already has one.  (The property might already exist if
	 * the handler for an existing window is being replaced.)
	 */
	propType = (SkelIsMMDlog (dlog) ? skelWPropMovableModal : skelWPropModeless);
	if (SkelGetWindProp (dlog, propType) == (SkelWindPropHandle) nil
		&& !SkelAddWindProp (dlog, propType, (long) 0))
	{
		SkelRmveDlog (dlog);
		return (false);
	}

	wh = GetWHandler (dlog);
	(**wh).whSelect = doSelect;
	(**wh).whFilter = doFilter;
	return (true);
}


/*
 * Remove a dialog and its handler
 */

pascal void
SkelRmveDlog (DialogPtr dlog)
{
	SkelRmveWind (dlog);
}


/*
 * Override the default sizing limits for a window, or, if w
 * is nil, reset the default limits used by SkelWindow.
 */

pascal void
SkelSetGrowBounds (WindowPtr w, short hLo, short vLo, short hHi, short vHi)
{
WHHandle	wh;
Rect		r;

	if (w == (WindowPtr) nil)
		SetRect (&growRect, hLo, vLo, hHi, vHi);
	else if ((wh = GetWHandler (w)) != (WHHandle) nil)
	{
		SetRect (&r, hLo, vLo, hHi, vHi);
		(**wh).whGrow = r;
	}
}


pascal void 
SkelSetZoom (WindowPtr w, SkelWindZoomProcPtr pZoom)
{
WHHandle	h;

	if (w == (WindowPtr) nil)
		zoomProc = pZoom;
	else if ((h = GetWHandler (w)) != (WHHandle) nil)
		(**h).whZoom = pZoom;
}


/*
 * Return zoom proc associated with window, nil if there isn't one.
 * Return default zoom proc if window is nil.
 */

pascal SkelWindZoomProcPtr
SkelGetZoom (WindowPtr w)
{
WHHandle	h;

	if (w == (WindowPtr) nil)
		return (zoomProc);
	if ((h = GetWHandler (w)) != (WHHandle) nil)
		return ((**h).whZoom);
	return ((SkelWindZoomProcPtr) nil);
}


pascal Boolean
SkelWindowRegistered (WindowPtr w)
{
	return ((Boolean) (GetWHandler (w) != (WHHandle) nil));
}


/*
 * Routines to determine whether a given window is a dialog, or a movable
 * modal dialog.  Safe to pass nil.
 */

pascal Boolean
SkelIsDlog (WindowPtr w)
{
	return (w != (WindowPtr) nil && ((WindowPeek)w)->windowKind == dialogKind);
}


pascal Boolean
SkelIsMMDlog (WindowPtr w)
{
	return (SkelIsDlog (w) && hasGetWVariant && GetWVariant (w) == movableDBoxProc);
}


/* ------------------------ */
/* Handler finders/removers */
/* ------------------------ */

/*
 * Get handler associated with a window.
 *
 * Return nil if window doesn't belong to any known handler.
 *
 * This routine is absolutely fundamental to TransSkel.
 */


static WHHandle
GetWHandler (WindowPtr w)
{
WHHandle	h;

	if (w == (WindowPtr) nil)
		return ((WHHandle) nil);

	if (w == oldWindow) 
		return (oldWHandler);		/* return handler of cached window */

	for (h = whList; h != (WHHandle) nil; h = (**h).whNext)
	{
		if ((**h).whWind == w)
		{
			oldWindow = w;	/* set cached window and handler */
			oldWHandler = h;
			return (h);
		}
	}
	return ((WHHandle) nil);
}


/*
 * Detach a handler from the handler list and destroy it.
 *
 * Clear window cache variable, just in case it points to the window
 * whose hander is being destroyed (and thus has become invalid).
 */

static void
DetachWHandler (WHHandle wh)
{
WHHandle	h, h2;

	if (whList != (WHHandle) nil)		/* if list empty, ignore */
	{
		if (whList == wh)				/* is it the first element? */
		{
			h2 = whList;
			whList = (**whList).whNext;
		}
		else for (h = whList; h != (WHHandle) nil; h = h2)
		{
			h2 = (**h).whNext;
			if (h2 == (WHHandle) nil)
				return;					/* handler not in list! (huh?) */
			if (h2 == wh)				/* found it */
			{
				(**h).whNext = (**h2).whNext;
				break;
			}
		}
		DisposeHandle ((Handle) h2);		/* get rid of handler record */
	}

	oldWindow = (WindowPtr) nil;	/* clear window cache variables */
	oldWHandler = (WHHandle) nil;
}


/* ------------------------------------------------------- */
/* Menu handler installation/removal/modification routines */
/* ------------------------------------------------------- */


/*
 * Install handler for a menu.  Remove any previous handler for it.
 * Pass the following parameters:
 *
 * theMenu
 *		Handle to the menu to be handled.  Must be created by host.
 * doSelect
 *		Proc that handles selection of items from menu.  If this is
 * 		nil, the menu is installed, but nothing happens when items
 * 		are selected from it.
 * doClobber
 *		Proc for disposal of handler's data structures.  Usually
 * 		nil for menus that remain in menu bar until program
 * 		termination.
 * subMenu
 *		True if the menu is a submenu (not installed in menu bar).
 * drawBar
 *		True if menu bar is to be drawn after menu is installed.
 * 		(Ignored if the menu is a submenu.)
 *
 * Return true if successful, false if no handler could be allocated.
 */

pascal Boolean
SkelMenu (MenuHandle m,
			SkelMenuSelectProcPtr doSelect,
			SkelMenuClobberProcPtr doClobber,
			Boolean subMenu,
			Boolean drawBar)
{
MHHandle	mh;
Boolean		oldFlag;

	oldFlag = mhClobOnRmve;		/* remove any previous handler for */
	mhClobOnRmve = false;		/* menu, without redrawing menu bar */
	SkelRmveMenu (m);
	mhClobOnRmve = oldFlag;

	if ((mh = New (MHandler)) != (MHHandle) nil)
	{
		(**mh).mhNext = mhList;
		mhList = mh;
		(**mh).mhID = (**m).menuID;	/* get menu id number */
		(**mh).mhSelect = doSelect;			/* install selection handler */
		(**mh).mhClobber = doClobber;		/* install disposal handler */
		(**mh).mhSubMenu = subMenu;			/* set submenu flag */
		/* install menu in menu bar if not a submenu */
		InsertMenu (m, subMenu ? -1 : 0);
	}
	if (drawBar && !subMenu)
		DrawMenuBar ();
	return ((Boolean) (mh != (MHHandle) nil));
}


/*
 * Remove a menu handler.  This calls the handler's menu disposal
 * routine and then takes the handler out of the handler list and
 * disposes of it.  The menu bar is redrawn if the menu was not a
 * submenu and the global redraw flag hasn't been cleared.
 *
 * The menu MUST be deleted from the menu bar before calling the
 * clobber proc.  Otherwise the menu bar will end up filled with
 * garbage if the menu was allocated with NewMenu (IM I-352).
 */

pascal void
SkelRmveMenu (MenuHandle m)
{
short	mID;
MHHandle	h, h2;
SkelMenuClobberProcPtr	p;

	mID = (**m).menuID;
	if (mhList != (MHHandle) nil)			/* if list empty, ignore */
	{
		if ((**mhList).mhID == mID)	/* is it the first element? */
		{
			h2 = mhList;
			mhList = (**mhList).mhNext;
		}
		else
		{
			for (h = mhList; h != (MHHandle) nil; h = h2)
			{
				h2 = (**h).mhNext;
				if (h2 == (MHHandle) nil)
					return;						/* menu not in list! */
				if ((**h2).mhID == mID)			/* found it */
				{
					(**h).mhNext = (**h2).mhNext;
					break;
				}
			}
		}
		DeleteMenu (mID);
		if (mhDrawBarOnRmve && !(**h2).mhSubMenu)
			DrawMenuBar ();
		if (mhClobOnRmve
			&& (p = (**h2).mhClobber) != (SkelMenuClobberProcPtr) nil)
				(*p) (m);					/* call disposal routine */
		DisposeHandle ((Handle) h2);		/* get rid of handler record */
	}
}


/*
 * General menu-selection handler.  Just passes selection to the handler's
 * select routine.  If the select routine is nil, selecting items from
 * the menu is a nop.
 */

static void
DoMenuCommand (long command)
{
short		menu;
short		item;
MHHandle	mh;

	menu = HiWord (command);
	item = LoWord (command);
	for (mh = mhList; mh != (MHHandle) nil; mh = (**mh).mhNext)
	{
		if (menu == (**mh).mhID && (**mh).mhSelect != (SkelMenuSelectProcPtr) nil)
		{
			(*(**mh).mhSelect) (item);
			break;
		}
	}
	HiliteMenu (0);		/* command done, turn off menu hiliting */
}


/*
 * Menu is about to be pulled down or command-key executed.  Call menu
 * hook if there is one so application can set menus/items appropriately.
 */

static void
DoMenuHook (void)
{
	if (pMenuHook != (SkelMenuHookProcPtr) nil)
		(*pMenuHook) ();
}


pascal void
SkelSetMenuHook (SkelMenuHookProcPtr p)
{
	pMenuHook = p;
}


pascal SkelMenuHookProcPtr
SkelGetMenuHook (void)
{
	return (pMenuHook);
}


/* ------------------------ */
/* Window property routines */
/* ------------------------ */


/*
 * Add a property to a window.  Fail if the window is unregistered
 * or can't allocate memory for a new property structure.  If the
 * window already has such a property, fail.
 *
 * Returns a handle to the new property for success, nil for failure.
 */

pascal Boolean
SkelAddWindProp (WindowPtr w, short propType, long propData)
{
WHHandle		wh;
SkelWindPropHandle	ph;

	if (propType == skelWPropAll)
		return (false);
	if ((ph = SkelGetWindProp (w, propType)) != (SkelWindPropHandle) nil)
		return (false);
	/* if window is unregistered, or can't allocate structure, fail */
	if ((wh = GetWHandler (w)) == (WHHandle) nil
		|| (ph = New (SkelWindProperty)) == (SkelWindPropHandle) nil)
		return (false);
	(**ph).skelWPropType = propType;
	(**ph).skelWPropData = propData;
	(**ph).skelWPropNext = (**wh).whProperties;
	(**wh).whProperties = ph;
	return (true);
}


/*
 * Remove a window property.  Does nothing if the window isn't
 * registered or if the window doesn't have the given property.
 *
 * If propType is skelWPropAll, SkelRmveWindProp() calls itself
 * recursively to remove all the properties on a window.  This
 * means that if you put skelWPropAll into the skelWPropType field
 * of a property, you'll get an infinite loop here.
 */

pascal void
SkelRmveWindProp (WindowPtr w, short propType)
{
WHHandle		wh;
SkelWindPropHandle	ph, ph2, pNext;

	if ((wh = GetWHandler (w)) == (WHHandle) nil
		|| (ph = SkelGetWindProp (w, propType)) == (SkelWindPropHandle) nil)
		return;

	if (propType == skelWPropAll)	/* remove all properties */
	{
		while ((ph = (**wh).whProperties) != (SkelWindPropHandle) nil)
			SkelRmveWindProp (w, (**ph).skelWPropType);
		return;
	}

	/* remove particular property */
	if ((ph2 = (**wh).whProperties) == ph)	/* remove first in list */
		(**wh).whProperties = (**ph).skelWPropNext;
	else
	{
		while ((pNext = (**ph2).skelWPropNext) != (SkelWindPropHandle) nil)
		{
			if (pNext == ph)
			{
				(**ph2).skelWPropNext = (**ph).skelWPropNext;
				break;
			}
			ph2 = pNext;
		}
	}
	DisposeHandle ((Handle) ph);
}


/*
 * Find the given property for the window.  Fail if window is
 * unregistered or has no such property.
 */

pascal SkelWindPropHandle
SkelGetWindProp (WindowPtr w, short propType)
{
WHHandle			wh;
SkelWindPropHandle	ph = (SkelWindPropHandle) nil;

	if ((wh = GetWHandler (w)) != (WHHandle) nil)
	{
		if (propType == skelWPropAll)	/* return head of list */
			ph = (**wh).whProperties;
		else for (ph = (**wh).whProperties; ph != (SkelWindPropHandle) nil; ph = (**ph).skelWPropNext)
		{
			if ((**ph).skelWPropType == propType)
				break;
		}
	}
	return (ph);
}


/*
 * Find the data value for a given property for the window.  Return 0 if window
 * is unregistered or has no such property.
 *
 * If you need to be able to distinquish an error return from a valid zero-value
 * data value, you should call SkelGetWindProp() instead, check for an error,
 * and extract the data value if there was no error.
 *
 * skelWPropAll is not a valid properly type for this call.
 */

pascal long
SkelGetWindPropData (WindowPtr w, short propType)
{
SkelWindPropHandle	ph;

	ph = SkelGetWindProp (w, propType);
	if (ph != (SkelWindPropHandle) nil)
		return ((**ph).skelWPropData);
	return (0);
}
