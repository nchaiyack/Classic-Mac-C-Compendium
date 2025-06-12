/*
 * Need to dispose of the Apple Event handlers?
 *
 * Need to check for new Std Get File routines.
 */

/*
 * EventLog - TransDisplay event-logging demonstration program
 *
 * The project should include EventLog.c (this file), TransDisplay.c
 * (or a project made from TransDisplay.c), TransSkel.c (or a project
 * made from TransSkel.c), and MacTraps.
 *
 * 08 Nov 86 Paul DuBois
 * 28 Dec 86 Removed window zooming constants, since LightspeedC
 * v. 2.01 now supports them directly.
 * 01 Feb 89 Modified for TransSkel 2.0.  2-byte and 4-byte integer
 * types are typedef'ed to Integer and Longint to ease porting.
 * 05 Jun 93
 * - Use SkelDlogFilter() in DoAbout() so that update events for document
 * windows are seen and logged.
 * - Conversion for THINK C 6.0.  Also conversion for TransSkel 3.00.
 * 16 Oct 93
 * - Added some support for OS and high-level events.  Open Application,
 * Quit Application, Open Documents, and Print Documents are reported
 * (i.e., the four required core Apple Events).
 * Apple Event logging is a little different than other events, since not
 * all event information is in the EventRecord.  They're just reported through
 * the regular Apple Event routing mechanism.
 * 19 Oct 93
 * - Add Launch item to File menu to prepare for allowing app launches so
 * can catch app-died events.  Add handler for Application Died Apple Event.
 * 06 Nov 93
 * - ReportKey() now reports the key code in addition to the character, in
 * keeping with a change made some time ago to TransSkel's key handler that
 * passes both.
 * 04 Jan 94
 * - Undid Integer/LongInt type stuff back to short/long.
 * 18 Jan 94
 * - Updated for TransDisplay 3.04.
 * 21 Feb 94
 * - Updated for TransSkel 3.11, TransDisplay 3.05.
 */


#include	<GestaltEqu.h>
#include	<AppleEvents.h>


#include	"TransSkel.h"

#include	"TransDisplay.h"


#define PtToLong(pt)	(* (long *) &pt)

#define	maxButton	19


#define	helpTextRes		1000	/* help text resource number */
#define	aboutAlrtRes	1000	/* About... alert resource number */


typedef enum					/* Menu resource numbers */
{
	fileMenuRes = 1000,
	editMenuRes,
	logMenuRes
};


typedef enum					/* Window resource numbers */
{
	logWindRes = 1000,
	helpWindRes,
	selectWindRes
};


typedef enum			/* File menu item numbers */
{
	showLog = 1,	/* make windows visible/bring to front */
	showHelp,
	showSelect,
	/* --- */
	launchApp = 5,
	/* --- */
	quit = 7
};


typedef enum				/* Edit menu item numbers */
{
	undo = 1,
	/* --- */
	cut = 3,
	copy,
	paste,
	clear
};


typedef enum				/* Log menu item numbers */
{
	logEvents = 1,		/* whether events are logged */
	excludeLWind,
	/* --- */
	flushLog = 4,		/* flush log output */
	/* --- */
	wrapStyle = 6,		/* word wrap or not */
	/* --- */
	leftJust = 8,		/* justification */
	centerJust,
	rightJust,
	/* --- */
	small = 12,			/* text size */
	medium,
	large,
	/* --- */
	top = 16,			/* scroll home */
	bottom				/* scroll to bottom */
};


typedef struct CtrlInfo
{
	Point			loc;		/* upper left of control */
	Str255			title;		/* control title */
	Boolean			*flagAddr;	/* associated boolean */
	ControlHandle	ctrl;		/* associated control */
	struct CtrlInfo	*subInfo;	/* subsidiary control */

} CtrlInfo;


WindowPtr	selectWind;			/* event selection window */
WindowPtr	helpWind;			/* help text window */
WindowPtr	logWind;			/* log output window */
MenuHandle	fileMenu;
MenuHandle	editMenu;
MenuHandle	logMenu;
Boolean		reportEvents;		/* report events or not */
Boolean		excludeLog;			/* exclude log window events or not */
short		logFont;
short		logSize;
short		logWrap;
short		logJust;


Boolean							/* event type selection flags */
		rMouseDown = true,
			rMouseMods = false,
			rMouseWind = true,
			rMouseLoc = false,
			rMousePart = true,
			rMouseSys = false,
		rMouseUp = false,
		rKeyDown = true,
			rKDMods = false,
		rAutoKey = true,
			rAKMods = false,
		rUpdate = true,
		rActivate = true,
		rDisk = true,
		rOS = true,
			rOSSuspendResume = true,
			rOSMouseMoved = true,
			rOSChildDied = true,
		rHighLevel = true;


/*
	Control information.  The last field is used to tell which controls
	are "owned" by another.  When the owner is unchecked, all the owned
	controls go dim.
*/

CtrlInfo	ctrlInfo [maxButton] =
{
	{ { 5, 10}, "\pMouse Down", &rMouseDown, nil, nil },
	{ { 25, 30}, "\pModifiers", &rMouseMods, nil, &ctrlInfo[0] },
	{ { 45, 30}, "\pWindow", &rMouseWind, nil, &ctrlInfo[0] },
	{ { 65, 30}, "\pLocation", &rMouseLoc, nil, &ctrlInfo[0] },
	{ { 85, 30}, "\pPart Code", &rMousePart, nil, &ctrlInfo[0] },
	{ { 105, 30}, "\pSystem Clicks", &rMouseSys, nil, &ctrlInfo[0] },
	{ { 125, 10}, "\pMouse Up", &rMouseUp, nil, nil },
	{ { 5, 150}, "\pKey Down", &rKeyDown, nil, nil },
	{ { 25, 170}, "\pModifiers", &rKDMods, nil, &ctrlInfo[7] },
	{ { 45, 150}, "\pAutoKey", &rAutoKey, nil, nil },
	{ { 65, 170}, "\pModifiers", &rAKMods, nil, &ctrlInfo[9] },
	{ { 85, 150}, "\pUpdate", &rUpdate, nil, nil },
	{ { 105, 150}, "\pActivate", &rActivate, nil, nil },
	{ { 125, 150}, "\pDisk", &rDisk, nil, nil },
	{ { 5, 265}, "\pOS Event", &rOS, nil, nil },
	{ { 25, 285}, "\pSuspend", &rOSSuspendResume, nil, &ctrlInfo[14] },
	{ { 45, 285}, "\pMouse Moved", &rOSMouseMoved, nil, &ctrlInfo[14] },
	{ { 65, 285}, "\pChild Died", &rOSChildDied, nil, &ctrlInfo[14] },
	{ { 85, 265}, "\pHigh Level", &rHighLevel, nil, nil }
};

/*	Window that was in front last time checked	*/

WindowPtr	lastFront = nil;

static Boolean	inForeground = true;


/*
	Print information about a window.  If it's a window with a title,
	print the title.  Print whether it's a desk accessory window.
*/

static void
WindowInfo (theWind)
WindowPeek	theWind;
{
Str255	title;

	GetWTitle ((WindowPtr) theWind, title);
	if (title[0] != 0)			/* window has title */
	{
		DisplayChar (' ');
		DisplayString (title);
	}

	if (theWind->windowKind < 0)
		DisplayString ("\p (DA)");
}


static void
Modifiers (mods)
short	mods;
{
	DisplayString ("\p modifiers 0x");
	DisplayHexShort (mods);
}


static void
MouseLoc (thePt, thePort)
Point	thePt;
GrafPtr	thePort;
{
GrafPtr	savePort;

	GetPort (&savePort);
	SetPort (thePort);
	GlobalToLocal (&thePt);
	SetPort (savePort);
	if (rMouseLoc)
	{
		DisplayString ("\p loc (");
		DisplayShort (thePt.h);
		DisplayString ("\p, ");
		DisplayShort (thePt.v);
		DisplayChar (')');
	}
}


/*
	Mouse click.  Get the window that the click occurred in, and the
	part of the window.

	Make sure to get all the part codes!  (incl. zoom box stuff)
*/

static void
ReportMouse (EventRecord *theEvent)
{
Point	evtPt;
short	evtPart;
GrafPtr	evtPort;

	evtPt = theEvent->where;
	evtPart = FindWindow (evtPt, &evtPort);
	if (excludeLog && evtPort == logWind)
		return;
	DisplayString ("\pMouse click");

	switch (evtPart)
	{
	default:
		if (rMousePart)
		{
			DisplayString ("\p in unknown part code ");
			DisplayShort (evtPart);
		}
		break;

	/*
	 * Click in a desk accessory window.
	 */
	case inSysWindow:
		if (rMouseSys)
		{
			if (rMousePart)
				DisplayString ("\p in system window:");
			if (rMouseWind)
				WindowInfo ((WindowPeek) evtPort);
			MouseLoc (evtPt, evtPort);
		}
		break;

	/*
	 * Click in desk top.
	 */
	case inDesk:
		if (rMousePart)
			DisplayString ("\p in desktop");
		break;

	/*
	 * Click in menu bar.
	 */
	case inMenuBar:
		if (rMousePart)
			DisplayString ("\p in menu bar");
		break;

	/*
	 * Click in grow box.
	 */
	case inGrow:
		if (rMousePart)
			DisplayString ("\p in grow region:");
		if (rMouseWind)
			WindowInfo ((WindowPeek) evtPort);
		MouseLoc (evtPt, evtPort);
		break;

	/*
	 * Click in title bar.
	 */
	case inDrag:
		if (rMousePart)
			DisplayString ("\p in drag region:");
		if (rMouseWind)
			WindowInfo ((WindowPeek) evtPort);
		break;

	/*
	 * Click in close box.
	 */
	case inGoAway:
		if (rMousePart)
			DisplayString ("\p in close box:");
		if (rMouseWind)
			WindowInfo ((WindowPeek) evtPort);
		break;

	/*
	 * Click in zoom-in box.
	 */
	case inZoomIn:
		if (rMousePart)
			DisplayString ("\p in zoom-in box:");
		if (rMouseWind)
			WindowInfo ((WindowPeek) evtPort);
		break;

	/*
	 * Click in zoom-out box.
	 */
	case inZoomOut:
		if (rMousePart)
			DisplayString ("\p in zoom-out box:");
		if (rMouseWind)
			WindowInfo ((WindowPeek) evtPort);
		break;

	/*
	 * Click in content region.
	 *
	 * (some day - check if in control, and if so, print control information)
	 */
	case inContent:
		if (rMousePart)
			DisplayString ("\p in content region:");
		if (rMouseWind)
			WindowInfo ((WindowPeek) evtPort);
		MouseLoc (evtPt, evtPort);
		break;
	}

	if (rMouseMods)
		Modifiers (theEvent->modifiers);
	DisplayLn ();
}


/*
 * The high bit of the key code is set if the event is a key up event.
 * Actually, this will never be reported since the default event mask
 * excludes key ups.  I'm not sure the system reports them anymore anyway,
 * even if you change the event mask...
 */

static void
ReportKey (short what, char c, unsigned char code, short mods, Boolean modFlag)
{
	if (what == keyDown)
		DisplayString ("\pKey down: char '");
	else
		DisplayString ("\pAutokey: char '");
	DisplayChar (c);
	DisplayString ("\p', code 0x");
	DisplayHexShort ((short) code);
	if (modFlag)
	{
		DisplayString ("\p,");
		Modifiers (mods);
	}
	DisplayLn ();
}


static void
ReportActivate (WindowPtr theWind, short mods)
{
	if ((mods & activeFlag) != 0)
		DisplayString ("\pActivate:");
	else
		DisplayString ("\pDeactivate:");
	WindowInfo ((WindowPeek) theWind);
	DisplayLn ();
}


static  void
ReportUpdate (WindowPtr theWind)
{
	DisplayString ("\pUpdate:");
	WindowInfo ((WindowPeek) (WindowPeek) theWind);
	DisplayLn ();
}


/*
 * General event logger.  This is essentially a modified copy of
 * SkelRouteEvent().  Null events are not recorded because they occur
 * too frequently.
 */

static pascal Boolean
LogEvent (EventRecord *theEvent)
{
Point			evtPt;
long			evtMsge;
GrafPtr			tmpPort;
short			evtPart;
char			evtChar;
unsigned char	evtCode;
short			evtMods;
Rect			r;
short			osMsge;
Boolean			osSuspend;
Boolean			osClipCvt;

/*
if (theEvent->what != 0)
{
	DisplayString ("\pEvent number: ");
	DisplayLong ((long) theEvent->what);
	DisplayLn ();
}
*/
	if (reportEvents == false)
		return (false);			/* don't do anything */
	evtPt = theEvent->where;
	evtMsge = theEvent->message;
	evtMods = theEvent->modifiers;

	switch (theEvent->what)
	{

/*
	Mouse click.
*/
		case mouseDown:
			if (rMouseDown)
				ReportMouse (theEvent);
			break;

		case mouseUp:
			if (rMouseUp)
				DisplayString ("\pMouse up\r");
			break;

/*
	Key event.
*/
		case keyDown:
			if (excludeLog && FrontWindow () == logWind)
				break;
			if (rKeyDown)
			{
				evtChar = evtMsge & charCodeMask;
				evtCode = (evtMsge & keyCodeMask) >> 8;
				ReportKey (keyDown, evtChar, evtCode, evtMods, rKDMods);
			}
			break;

		case autoKey:
			if (excludeLog && FrontWindow () == logWind)
				break;
			if (rKeyDown)
			{
				evtChar = evtMsge & charCodeMask;
				evtCode = (evtMsge & keyCodeMask) >> 8;
				ReportKey (autoKey, evtChar, evtCode, evtMods, rAKMods);
			}
			break;

/*
	Update a window.  If it's an update for the log window, invalidate
	it, because the message is written and will cause a scroll BEFORE
	the window actually gets updated.  This means that part of what
	needs redrawing will be scrolled out of the update region and won't
	be redrawn properly.  Invalidating the entire port is wasteful but
	makes sure the whole window can be drawn properly.
*/
		case updateEvt:
			if ((WindowPtr) evtMsge == logWind)
			{
				GetPort (&tmpPort);
				SetPort (logWind);
				InvalRect (&logWind->portRect);
				SetPort (tmpPort);
			}
			if (excludeLog && (WindowPtr) evtMsge == logWind)
				break;
			if (rUpdate)
				ReportUpdate ((WindowPtr) evtMsge);
			break;

/*
	Activate or deactivate a window.
*/
		case activateEvt:
			if (excludeLog && (WindowPtr) evtMsge == logWind)
				break;
			if (rActivate)
				ReportActivate ((WindowPtr) evtMsge, theEvent->modifiers);
			break;

/*
	handle inserts of uninitialized disks
*/
		case diskEvt:
			if (rDisk)
			{
				DisplayString ("\pDisk insertion");
				if (HiWord (evtMsge) != noErr)
				{
					DisplayString ("\p (needs initializing)");
				}
				DisplayLn ();
			}
			break;

	
		case osEvt:				/* aka app4Evt aka MultiFinder event */
			if (!rOS)
				break;
			DisplayString ("\pOS event:");
			/* rip the message field into constituent parts */
			osMsge = ((evtMsge >> 24) & 0xff);			/* high byte */
			osSuspend = (Boolean) ((evtMsge & 1) == 0);	/* suspend application */
			osClipCvt = (Boolean) ((evtMsge & 2) != 0);	/* convert clipboard ? */

			switch (osMsge)
			{
			case	1:				/* suspend or resume event */
				inForeground = !osSuspend;
				if (osSuspend)			/* always convert on suspend */
					osClipCvt = true;
				if (inForeground)
					DisplayString ("\p resume");
				else
					DisplayString ("\p suspend");
				if (osClipCvt)
					DisplayString ("\p (convert clipboard)");
				else
					DisplayString ("\p (do not convert clipboard)");
				break;

			case 0xfa:				/* mouse-moved event */
				/*
				 * Not likely to be executed since EventLog doesn't 
				 * do anything about changing cursor
				 */
				DisplayString ("\pmouse moved");
				break;

			case 0xfd:				/* child-died event */
				/* SADE-only event */
				/* child pid is byte 2 (evtMsge >> 16) & 0xffL) */
				DisplayString ("\p sade (pid ");
				DisplayShort ((evtMsge >> 16) & 0xff);
				DisplayString ("\p)");
				break;

			default:				/* other event */
				DisplayString ("\p unknown type");
				break;
			}
			DisplayLn ();
			break;

		case kHighLevelEvent:
			/* Handled by LogAppleEvent() */
			break;

	}
	return (false);		/* never tell TransSkel the event was handled */
}


static pascal void
LogAppleEvent (EventRecord *theEvent)
{
	if (!rHighLevel)
		return;

	DisplayString ("\pHigh level event '");
	DisplayOSType ((OSType) theEvent->message);
	DisplayString ("\p' '");
	DisplayOSType ((OSType) PtToLong (theEvent->where));
	DisplayString ("\p'");
	DisplayLn ();
	AEProcessAppleEvent (theEvent);
}


/*
 * Handler for Open Application or Quit Application Apple Events.
 * Reference constant is 0 for Open, 1 for Quit.
 * Note that it must be a pascal function!
 */

static pascal OSErr
AEOpenOrQuitApp (AppleEvent *theAEvt, AppleEvent *replyEvt, long refCon)
{
long	items;

	if (refCon == 0L)
		DisplayString ("\pOpen Application Apple event");
	else
		DisplayString ("\pQuit Application Apple event");
	DisplayLn ();
	return (noErr);
}


/*
 * Handler for Open Document or Print Document Apple Events.
 * Reference constant is 0 for Open, 1 for Print.
 * Note that it must be a pascal function!
 */

static pascal OSErr
AEOpenOrPrintDoc (AppleEvent *theAEvt, AppleEvent *replyEvt, long refCon)
{
long		items, index;
FSSpec		myFSS;
AEDescList	docList;
AEKeyword	keywd;
DescType	returnedType;
long		actualSize;
OSErr		myErr;

	if (refCon == 0L)
		DisplayString ("\pOpen Document Apple event");
	else
		DisplayString ("\pPrint Document Apple event");
	DisplayLn ();
	myErr = AEGetParamDesc (theAEvt, keyDirectObject, typeAEList, &docList);
	if (myErr != noErr)
		return (myErr);

	/* should check for missing required parameters here */

	DisplayString ("\pNumber of items: ");
	if ((myErr = AECountItems (&docList, &items)) != noErr)
	{
		DisplayString ("\p(couldn't tell - error occurred)");
		DisplayLn ();
		return (myErr);
	}
	DisplayLong (items);
	DisplayLn ();
	for (index = 1; index <= items; index++)
	{
		myErr = AEGetNthPtr (&docList, index, typeFSS, &keywd, &returnedType,
						&myFSS, sizeof (myFSS), &actualSize);
		if (myErr != noErr)
			return (myErr);
		DisplayString ("\pItem ");
		DisplayLong (index);
		DisplayString ("\p: name = ");
		DisplayString ((StringPtr) myFSS.name);
		DisplayString ("\p, vRefNum = ");
		DisplayShort (myFSS.vRefNum);
		DisplayString ("\p, parID = ");
		DisplayLong (myFSS.parID);
		DisplayLn ();
	}
	myErr = AEDisposeDesc (&docList);
	return (myErr);
}


/*
 * Handler for Application Died Apple Event.
 * This will be called if/when any application launched by selecting
 * File/Launch exits.
 * Note that it must be a pascal function!
 */

static pascal OSErr
AEAppDied (AppleEvent *theAEvt, AppleEvent *replyEvt, long refCon)
{
OSErr		myErr;
OSErr		appDiedErr;
DescType	returnedType;
long		actualSize;
ProcessSerialNumber	psn;

	DisplayString ("\pApplication Died Apple event");
	DisplayLn ();

	DisplayString ("\pProcess serial number: ");
	myErr = AEGetParamPtr (theAEvt, keyProcessSerialNumber, typeProcessSerialNumber,
								&returnedType, &psn, sizeof (psn), &actualSize);
	if (myErr == noErr)
	{
		DisplayLong (psn.highLongOfPSN);
		DisplayChar ('/');
		DisplayLong (psn.lowLongOfPSN);
	}
	else
		DisplayString ("\pcould not get psn.");
	DisplayLn ();

	DisplayString ("\pReturn status: ");
	myErr = AEGetParamPtr (theAEvt, keyErrorNumber, typeLongInteger,
						&returnedType, &appDiedErr, sizeof (appDiedErr), &actualSize);
	if (myErr == noErr)
		DisplayLong (appDiedErr);
	else
		DisplayString ("\pcould not get return status.");
	DisplayLn ();

	/* should check for missing required parameters here */

	return (noErr);
}


/* ------------------------------------------------------------ */
/*			Event Selection Window Handler Routines				*/
/* ------------------------------------------------------------ */


/*
	Activate event procedure for both display windows and the checkbox
	window.
*/

static pascal void
Activate (Boolean active)
{
}

/*
	Update window.  This is easy, just draw the controls.
*/

static pascal void
Update (Boolean resized)
{
	DrawControls (selectWind);
}


/*
	Handle hits in check boxes of selection window:
	Toggle check box, sync the associated flag, and enable or disable
	any subsidiary check boxes accordingly.  (Subsidiaries have
	information in the control structure that points back to the owner
	check box.)

*/

static pascal void
Mouse (Point thePt, long t, short mods)
{
ControlHandle	ctl;
CtrlInfo		*ci;
Boolean			val;
short			i;

	if (FindControl (thePt, selectWind, &ctl))
	{
		if (TrackControl (ctl, thePt, nil))
		{
			ci = (CtrlInfo *) GetCRefCon (ctl);
			val = !GetCtlValue (ctl);
			*(ci->flagAddr) = val;
			SetCtlValue (ctl, val);
		
			/* enable/disable any subsidiaries */
		
			for (i = 0; i < maxButton; ++i)
			{
				if (ctrlInfo[i].subInfo->ctrl == ci->ctrl)
					HiliteControl (ctrlInfo[i].ctrl, val ? 0 : 255);
			}
		}
	}
}


/* ------------------------------------------------------------ */
/*			Menu Hook and Menu Handler Routines					*/
/* ------------------------------------------------------------ */


/*
 * Menu hook procedure.  Check front window, reset edit menu depending on
 * whether front window is an application window or a DA window (items are
 * only active for a DA window).
 */

static pascal void
MyMenuHook (void)
{
WindowPtr	w;
short		theKind;

	DisableItem (editMenu, undo);
	DisableItem (editMenu, cut);
	DisableItem (editMenu, copy);
	DisableItem (editMenu, paste);
	DisableItem (editMenu, clear);

	if ((w = FrontWindow ()) != (WindowPtr) nil)
	{
		if (((WindowPeek) w)->windowKind < 0)	/* DA in front */
		{
			EnableItem (editMenu, undo);
			EnableItem (editMenu, cut);
			EnableItem (editMenu, copy);
			EnableItem (editMenu, paste);
			EnableItem (editMenu, clear);
		}
	}
}


/*
 * Handle selection of About... item from Apple menu.
 * Use of SkelDlogFilter() is important here because it makes
 * sure the event hook is called even while the alert is up.
 */

static pascal void
DoAbout (short item)
{
	(void) SkelAlert (aboutAlrtRes, SkelDlogFilter (nil, true),
												skelPositionOnParentDevice);
	SkelRmveDlogFilter ();
}


/*
 * File menu handler.
 *
 * The launchApp item will be disable and cannot be selected unless
 * the system supports the new standard file dialogs and application
 * launching.
 */

static pascal void
DoFileMenu (short item)
{
LaunchParamBlockRec	lParams;
StandardFileReply	appSFR;
SFTypeList			types;
OSErr				myErr;

	switch (item)
	{
	case showHelp:
		SelectWindow (helpWind);
		ShowWindow (helpWind);
		break;
	
	case showSelect:
		SelectWindow (selectWind);
		ShowWindow (selectWind);
		break;
	
	case showLog:
		SelectWindow (logWind);
		ShowWindow (logWind);
		break;
	
	case launchApp:
		types[0] = 'APPL';    /* to launch any application */
		StandardGetFile (nil, 1, types, &appSFR);		
		if (appSFR.sfGood)
		{ 
			
			lParams.launchBlockID = extendedBlock;
			lParams.launchEPBLength = extendedBlockLen;
			lParams.launchFileFlags = 0;
			lParams.launchControlFlags = launchContinue + launchNoFileFlags;
			lParams.launchAppSpec = &(appSFR.sfFile);
			lParams.launchAppParameters = nil;
			
			if ((myErr = LaunchApplication (&lParams)) != noErr)
			{
				DisplayString ("\pLaunch failed, error = \r");
				DisplayLong (myErr);
				DisplayLn ();
			}
		}
		break;
	
	case quit:
		SkelStopEventLoop ();
		break;
	}
}

/*
	Put the right check marks in the Log menu
*/

static pascal void
SetLogMenu ()
{
	CheckItem (logMenu, logEvents, reportEvents);
	CheckItem (logMenu, excludeLWind, excludeLog);
	CheckItem (logMenu, wrapStyle, logWrap >= 0);
	CheckItem (logMenu, leftJust, logJust == teJustLeft);
	CheckItem (logMenu, centerJust, logJust == teJustCenter);
	CheckItem (logMenu, rightJust, logJust == teJustRight);
	CheckItem (logMenu, small, logSize == 9);
	CheckItem (logMenu, medium, logSize == 12);
	CheckItem (logMenu, large, logSize == 24);
}


/*
	Set display style of log window
*/

static void
SetStyle ()
{
	SetDWindowStyle (logWind, logFont, logSize, logWrap, logJust);
	SetLogMenu ();
}


/*
	Log menu handler
*/

static pascal void
DoLogMenu (short item)
{
	switch (item)
	{
	case logEvents:
		reportEvents = !reportEvents;
		SetLogMenu ();
		break;

	case excludeLWind:
		excludeLog = !excludeLog;
		SetLogMenu ();
		break;

	case flushLog:
		FlushDWindow (logWind, 32767L);
		break;
	
	case wrapStyle:
		logWrap = (logWrap >= 0 ? -1 : 0);
		SetStyle ();
		break;

	case leftJust:
		logJust = teJustLeft;
		SetStyle ();
		break;

	case centerJust:
		logJust = teJustCenter;
		SetStyle ();
		break;

	case rightJust:
		logJust = teJustRight;
		SetStyle ();
		break;

	case small:
		logFont = monaco;
		logSize = 9;
		SetStyle ();
		break;

	case medium:
		logFont = systemFont;
		logSize = 12;
		SetStyle ();
		break;

	case large:
		logFont = geneva;
		logSize = 24;
		SetStyle ();
		break;

	case top:
		SetDWindowPos (logWind, 0);
		break;

	case bottom:
		SetDWindowPos (logWind, 32767);
		break;
	}
}


/*
	Dispose of event selection window (and controls)
*/

static pascal void
WClobber (void)
{
	DisposeWindow (selectWind);
}



/*
	Create controls
*/

static void
MakeControls (theWind)
WindowPtr	theWind;
{
short		i;
CtrlInfo	*ci;
Rect		r;

	for (i = 0; i < maxButton; ++i)
	{
		ci = &ctrlInfo[i];
		SetRect (&r, ci->loc.h, ci->loc.v,
					ci->loc.h + StringWidth (ci->title) + 30,
					ci->loc.v + 20);
		ci->ctrl = NewControl (theWind, &r, ci->title, true,
							*(ci->flagAddr), 0, 1,
							checkBoxProc, (long) ci);
	}
	ValidRect (&theWind->portRect);

	/*
	 * Disable high-level button if high-level events aren't present.
	 * (Assumes high-level button is last)
	 */
	if (!rHighLevel)
		HiliteControl (ctrlInfo[maxButton-1].ctrl, 255);
}

/*
 * Handle suspend/resume events.  This function is called only if the
 * "Suspend & Resume Events" bit is set in the 'SIZE' -1 resource.  It
 * assumes that the "MultiFinder-Aware" bit is also set, which means that
 * the application handles activate/deactivate of frontmost window on
 * suspend/resume:
 * - On a suspend, deactivate the frontmost window.
 * - On a resume, don't just blindly activate the frontmost window.  The
 * user may have brought the application to the front by clicking in some
 * other window.  That click will bring the other window to the front when
 * it's processed, so the frontmost window will just immediately be deactivated
 * after being activated.  This is ugly.  Instead, check whether or not a
 * mouse click is in the event queue.  If it is, and it's a click in the
 * frontmost window, activate that window.  Otherwise, don't activate anything.
 * When the click is processed, the right window will be activated.
 */

static pascal void
DoSuspendResume (Boolean inForeground)
{
EventRecord	event;
WindowPtr	w = FrontWindow ();
GrafPtr		eventPort;
Boolean		doActivate;

	if (!inForeground)		/* suspend is imminent */
	{
		SkelActivate (w, false);
	}
	else					/* resume is imminent */
	{
		doActivate = true;
		if (EventAvail (mDownMask, &event))
		{
			(void) FindWindow (event.where, &eventPort);
			if (eventPort != w)
				doActivate = false;
		}
		if (doActivate)
			SkelActivate (w, true);
	}
}


int
main ()
{
Handle	h;
long	result;

	SkelInit ((SkelInitParamsPtr) nil);

	/*
	 * If Apple Events are available, install handlers for the core required
	 * events, and the Application Died event.
	 */

	if (!SkelQuery (skelQHasAppleEvents))
		rHighLevel = false;
	else
	{
		SkelSetAEHandler (LogAppleEvent);
		if (AEInstallEventHandler (kCoreEventClass, kAEOpenApplication,
										AEOpenOrQuitApp, 0L, false) != noErr)
			DisplayString ("\pAEInstallEventHandler failed (Open Application)\r");
		if (AEInstallEventHandler (kCoreEventClass, kAEQuitApplication,
										AEOpenOrQuitApp, 1L, false) != noErr)
			DisplayString ("\pAEInstallEventHandler failed (Quit Application)\r");
		if (AEInstallEventHandler (kCoreEventClass, kAEOpenDocuments,
										AEOpenOrPrintDoc, 0L, false) != noErr)
			DisplayString ("\pAEInstallEventHandler failed (Open Documents)\r");
		if (AEInstallEventHandler (kCoreEventClass, kAEPrintDocuments,
										AEOpenOrPrintDoc, 1L, false) != noErr)
			DisplayString ("\pAEInstallEventHandler failed (Print Documents)\r");
		if (AEInstallEventHandler (kCoreEventClass, kAEApplicationDied,
										AEAppDied, 0L, false) != noErr)
			DisplayString ("\pAEInstallEventHandler failed (Application died)\r");
	}

	/*
	 * Install menus and menu handlers
	 *
	 * Enable File menu Launch item only if System 7 launching available
	 * and can be done without terminating current application.
	 */

	SkelApple ("\pAbout EventLog\311", DoAbout);

	fileMenu = GetMenu (fileMenuRes);
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, false);

	 DisableItem (fileMenu, launchApp);
	 if (SkelQuery (skelQHasGestalt) && Gestalt (gestaltOSAttr, &result) == noErr)
	 {
	 	if ((result & (1L << gestaltLaunchFullFileSpec))
	 		&& (result & (1L << gestaltLaunchCanReturn)))
	 		EnableItem (fileMenu, launchApp);
	 }

	editMenu = GetMenu (editMenuRes);
	(void) SkelMenu (editMenu, nil, nil, false, false);

	logMenu = GetMenu (logMenuRes);
	(void) SkelMenu (logMenu, DoLogMenu, nil, false, true);

	SkelSetMenuHook (MyMenuHook);

	/*
	 * Create windows and install handlers.
	 */

	SetDWindowNotify (nil, Activate);

	helpWind = GetNewDWindow (helpWindRes, (WindowPtr) -1L);
	SetDWindowStyle (helpWind, 0, 0, 0, teJustLeft);

	h = GetResource ('TEXT', helpTextRes);	/* read help text */
	HLock (h);						/* lock it and write to window */
	DisplayText (*h, GetHandleSize (h));
	HUnlock (h);
	ReleaseResource (h);			/* done with it, so goodbye */
	SetDWindowPos (helpWind, 0);	/* scroll back to top */
	/*ShowWindow (helpWind);*/

	logWind = GetNewDWindow (logWindRes, (WindowPtr) -1L);

	SkelSetEventHook (LogEvent);
	reportEvents = true;
	excludeLog = false;

	logFont = monaco;
	logSize = 9;
	logWrap = 0;
	logJust = teJustLeft;
	SetStyle ();
	ShowWindow (logWind);

	if (SkelQuery (skelQHasColorQD))
		selectWind = GetNewCWindow (selectWindRes, nil, (WindowPtr) -1L);
	else
		selectWind = GetNewWindow (selectWindRes, nil, (WindowPtr) -1L);

	(void) SkelWindow (selectWind,	/* the window */
				Mouse,		/* mouse click handler */
				nil,		/* key clicks are ignored */
				Update,		/* window updating procedure */
				Activate,	/* window activate/deactivate procedure */
				nil,		/* hide window */
				WClobber,	/* window disposal procedure */
				nil,		/* idle proc */
				false);		/* irrelevant */

	MakeControls (selectWind);

	/*
	 * Process events until user quits, then clean up and exit.
	 */

/*
	SetEventMask(everyEvent);
	SkelSetEventMask (SkelGetEventMask () | keyUpMask);
*/

	SkelSetSuspendResume (DoSuspendResume);

	SkelEventLoop ();

/*
	SetEventMask(everyEvent - keyUpMask);
*/

	SkelCleanup ();
}
