/*
 * DialogSkel - TransSkel modeless dialog demonstration.
 *
 * 21 Apr 88 Version 1.00 Paul DuBois
 * 29 Jan 89 Version 1.01
 * - Conversion for TransSkel 2.0.
 * 07 Jan 91 Version 1.02
 * - Conversion for TransSkel 3.00.
 * 05 Jun 93 Version 1.03
 * - Fixed to compile under THINK C 6.0.
 * 09 Oct 93
 * - Catches suspend/resume events, hides/shows dialogs when these occur.
 * 19 Nov 93
 * - Does better job on suspend/resume.  Rather than using HideWindow() and
 * ShowWindow(), uses ShowHide() to preserve stacking order.
 * - Added regular document window to provide contrast in handling during
 * suspend/resume and to provide a way to bring the application forward by
 * clicking in a window.  (With just dialogs, all the windows are hidden
 * on a suspend.)
 * - Added menu hook to disable Edit menu items when document window is
 * frontmost, since editing doesn't apply to it.
 * - Added Close item to File menu.
 * 09 Dec 93
 * - Updated for TransSkel 3.05.
 * - Flush mouse down on activate when document window wasn't frontmost
 * to preserve stacking order.
 * 30 Dec 93
 * - Fixed bug where closing modeless window by clicking close box caused
 * Visible checkbox to be unchecked in partner, but closing by selecting
 * Close from File menu didn't.
 * 31 Dec 93
 * - Junked all the dialog item manipulation routines and replaced by calls
 * to the equivalent routines that now make up part of the auxiliary
 * component of TransSkel (these routines are new in TS 3.06).
 * 18 Dec 93
 * - Ditto for new 3.07 routines.
 * 11 Feb 94
 * - Minor revisions.
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 * 23 Apr 94
 * - Updated for TransSkel 3.13.
 * - Since the document window is growable, draw the grow region, which wasn't
 * being done before.
 * 27 Apr 94
 * - Added filter function for modeless dialogs.
 * - Track cursor in edit text item and change to I-beam when inside.
 * - Map return/enter to hits in the Accept button.  Draw bold outline around
 * button.
 * - Controls in dialogs go inactive when window is deactivated.  Added
 * 'dctb' resource for the dialog windows so inactive controls draw in better
 * gray on color monitors.
 * 04 Nov 94
 * - Updated for TransSkel 3.18 (Support for Universal headers, PowerPC,
 * Metrowerks).
 */

# include	"TransSkel.h"


# define	normalHilite	0
# define	dimHilite		255



typedef enum			/* menu ID numbers */
{
	fileMenuID = skelAppleMenuID + 1,
	editMenuID
};


typedef enum
{
	mDlogRes = 1000,
	aboutAlrtRes,		/* About... alert resource number */
	docWindRes = 1000
};


typedef enum			/* File menu item numbers */
{
	showDlog1 = 1,
	showDlog2,
	showDoc,
	closeWind,
	fGrayLine,
	quit
};


typedef enum 			/* Edit menu item numbers */
{
	undo = 1,
	eGrayLine,
	cut,
	copy,
	paste,
	clear
};


typedef enum				/* dialog item numbers */
{
	button1 = 1,
	edit1,
	static1,
	radio1,
	radio2,
	radio3,
	check1,
	check2,
	user1
};

static pascal void	DrawIcon (DialogPtr dlog, short item);


static DialogPtr	mDlog1;
static DialogPtr	mDlog2;
static WindowPtr	docWind;
static short		iconNum1 = 0;
static short		iconNum2 = 0;

static MenuHandle	fileMenu;
static MenuHandle	editMenu;


/*
 * Set up a variable to point to the icon drawing procedure.  For 68K code this
 * is just a direct pointers to DrawIcon().  For PowerPC code it is a
 * routine descriptor into which is stuffed the address of DrawIcon().
 */

# if skelPPC		/* PowerPC code */

static RoutineDescriptor	drawDesc =
		BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, DrawIcon);
static UserItemUPP	drawIconProc = (UserItemUPP) &drawDesc;

# else				/* 68K code */

static UserItemUPP	drawIconProc = DrawIcon;

# endif


/* ------------------- */
/* Miscellaneous stuff */
/* ------------------- */


static pascal void
DrawIcon (DialogPtr dlog, short item)
{
Handle	h;
Rect	r;

	SkelGetDlogRect (dlog, item, &r);
	h = GetIcon (dlog == mDlog1 ? iconNum1 : iconNum2);
	PlotIcon (&r, h);
}


static void
SetDlogRadio (DialogPtr dlog, short item)
{
DialogPtr	partner;
GrafPtr		tmpPort;
Rect		r;

	partner = (DialogPtr) GetWRefCon (dlog);
	SkelSetDlogRadioButtonSet (dlog, radio1, radio3, item);

	if (partner == mDlog1)
		iconNum1 = item - radio1;
	else
		iconNum2 = item - radio1;

	SkelGetDlogRect (partner, user1, &r);
	GetPort (&tmpPort);
	SetPort (partner);
	InvalRect (&r);	/* invalidate item rect to generate update event */
	SetPort (tmpPort);
}


/* ---------------------------------------- */
/* Dialog window setup and handler routines */
/* ---------------------------------------- */


static pascal Boolean
DlogFilter (DialogPtr dlog, EventRecord *evt, short *item)
{
Boolean	result = false;
short	hilite;
char	c;
Str255	str;

	/*
	 * Dim Accept button if edit string is empty.  This must be checked
	 * on every event, not just null events.  Otherwise, if the user
	 * clears the string and immediately clicks Accept, there would be
	 * no intervening null event and the button would be active for the
	 * click.
	 *
	 * Always dim button if this is a deactivate or the dialog isn't
	 * frontmost.
	 */

	SkelGetDlogStr (dlog, edit1, str);
	hilite = (str[0] == 0 ? dimHilite : normalHilite);
	if (evt->what == activateEvt && (evt->modifiers & activeFlag) == 0)
		hilite = dimHilite;
	if (dlog != FrontWindow ())
		hilite = dimHilite;
	if (SkelSetDlogCtlHilite (dlog, button1, hilite))	/* did button change state? */
		SkelDrawButtonOutline (SkelGetDlogCtl (dlog, button1));

	switch (evt->what)
	{
	case nullEvent:
		SkelSetDlogCursor (dlog);
		break;
	case keyDown:
		if (SkelDlogMapKeyToButton (dlog, evt, item, button1, 0))
			result = true;
		break;
	case updateEvt:
		SkelDrawButtonOutline (SkelGetDlogCtl (dlog, button1));
		break;
	case activateEvt:
		/* Accept button and outline are set above.  Set other controls here. */
		hilite = (evt->modifiers & activeFlag ? normalHilite : dimHilite);
		(void) SkelSetDlogCtlHilite (dlog, radio1, hilite);
		(void) SkelSetDlogCtlHilite (dlog, radio2, hilite);
		(void) SkelSetDlogCtlHilite (dlog, radio3, hilite);
		(void) SkelSetDlogCtlHilite (dlog, check1, hilite);
		(void) SkelSetDlogCtlHilite (dlog, check2, hilite);
		break;
	}
	return (result);
}


static pascal void
DlogSelect (DialogPtr actor, short item)
{
DialogPtr	partner;
Str255		title;
short		value;

	partner = (DialogPtr) GetWRefCon (actor);
	switch (item)
	{
	case button1:
		SkelGetDlogStr (actor, edit1, title);
		SetWTitle (partner, title);
		break;

	/* set radio buttons */

	case radio1:
	case radio2:
	case radio3:
		SetDlogRadio (actor, item);
		break;

	/* flip check boxes */

	case check1:
		if (SkelToggleDlogCtlValue (actor, item))
			ShowWindow (partner);
		else
			HideWindow (partner);
		break;

	case check2:
		value = SkelToggleDlogCtlValue (actor, item);
		((WindowPeek) partner)->goAwayFlag = (char) (value ? 255 : 0);
		break;
	}
}


static pascal void
DlogClose (void)
{
DialogPtr	actor, partner;

	GetPort (&actor);
	partner = (DialogPtr) GetWRefCon (actor);
	HideWindow (actor);
	SkelSetDlogCtlValue (partner, check1, 0);
}


static pascal void
DlogClobber (void)
{
DialogPtr	dlog;

	GetPort (&dlog);
	DisposeDialog (dlog);
}


static DialogPtr
DemoDialog (StringPtr title, short h, short v)
{
DialogPtr	dlog;

	dlog = GetNewDialog (mDlogRes, nil, (WindowPtr) -1L);
	MoveWindow (dlog, h, v, false);
	SetWTitle (dlog, title);
	(void) SkelDialog (dlog, DlogFilter, DlogSelect, DlogClose, DlogClobber);
	return (dlog);
}


/* ------------------------------------------ */
/* Document window setup and handler routines */
/* ------------------------------------------ */


static pascal void
DocUpdate (Boolean resized)
{
}


static pascal void
DocActivate (Boolean active)
{
}


static pascal void
DocClobber (void)
{
	HideWindow (docWind);
	DisposeWindow (docWind);
}


static void
DocWindow (short h, short v)
{
	if (SkelQuery (skelQHasColorQD))
		docWind = GetNewCWindow (docWindRes, nil, (WindowPtr) -1L);
	else
		docWind = GetNewWindow (docWindRes, nil, (WindowPtr) -1L);
	(void) SkelWindow (docWind, nil, nil, DocUpdate, DocActivate, nil,
					DocClobber, nil, false);
	MoveWindow (docWind, h, v, false);
}


/* ------------- */
/* Menu handlers */
/* ------------- */


/*
 * Handle selection of About... item from Apple menu
 */

static pascal void
DoAppleMenu (short item)
{
	(void) SkelAlert (aboutAlrtRes, SkelDlogFilter (nil, true),
											skelPositionOnParentDevice);
	SkelRmveDlogFilter ();
}


/*
 * File menu handler
 */

static pascal void
DoFileMenu (short item)
{
	switch (item)
	{
	case showDlog1:
		SelectWindow (mDlog1);
		ShowWindow (mDlog1);
		SkelSetDlogCtlValue (mDlog2, check1, 1);
		break;
	
	case showDlog2:
		SelectWindow (mDlog2);
		ShowWindow (mDlog2);
		SkelSetDlogCtlValue (mDlog1, check1, 1);
		break;
	
	case showDoc:
		SelectWindow (docWind);
		ShowWindow (docWind);
		break;

	case closeWind:
		SkelClose (FrontWindow ());
		break;

	case quit:
		SkelStopEventLoop ();
		break;
	}
}


/*
 * Handle Edit menu
 */

static pascal void
DoEditMenu (short item)
{
DialogPtr	dlog;

	if (SystemEdit (item - 1))		/* if DA handled operation, return */
		return;

	/* if front window is document window, do nothing */
	dlog = (DialogPtr) FrontWindow ();
	if (((WindowPeek) dlog)->windowKind != dialogKind)
		return;

	switch (item)
	{
	case cut:
		DlgCut (dlog);
		(void) ZeroScrap ();
		(void) TEToScrap ();
		break;

	case copy:
		DlgCopy (dlog);
		(void) ZeroScrap ();
		(void) TEToScrap ();
		break;

	case paste:
		(void) TEFromScrap ();
		DlgPaste (dlog);
		break;

	case clear:
		DlgDelete (dlog);
		break;
	}
}


/*
 * Adjust menus when mouse click occurs in menu bar, before
 * menus are shown.
 */

static pascal void
AdjustMenus (void)
{
WindowPtr	w = FrontWindow ();

	if (w == (WindowPtr) nil)
		DisableItem (fileMenu, closeWind);
	else
		EnableItem (fileMenu, closeWind);

	if (w == docWind)
	{
		DisableItem (editMenu, undo);
		DisableItem (editMenu, cut);
		DisableItem (editMenu, copy);
		DisableItem (editMenu, paste);
		DisableItem (editMenu, clear);
	}
	else
	{
		/* modeless dialog or DA -- dim undo for dialogs */
		if (((WindowPeek) w)->windowKind == dialogKind)
			DisableItem (editMenu, undo);
		else
			EnableItem (editMenu, undo);
		EnableItem (editMenu, cut);
		EnableItem (editMenu, copy);
		EnableItem (editMenu, paste);
		EnableItem (editMenu, clear);
	}
}


/* ---------------------- */
/* Suspend/resume handler */
/* ---------------------- */


/*
 * On a suspend, hide the dialog windows and deactivate the frontmost window
 * if there is one.  Normally the system will unhilite the front window, but
 * since hiding the dialogs may cause the document window to be hilited, it's
 * necessary to unhilite whatever window's frontmost after hiding the windows.
 *
 * Similarly, on an activate, the system may hilite the frontmost window, but
 * but showing the dialogs may result in a new frontmost window, which then
 * needs to be hilited.  If the document window was not frontmost when the
 * suspend occurred, the system will also generate a mouse click to bring it
 * forward.  To preserve the stacking order, suck the mouse click out of the
 * event queue.
 */

static pascal void
SuspendResume (Boolean inForeground)
{
WindowPtr	w;
EventRecord	event;
static Boolean	hidden1;
static Boolean	hidden2;

	if (!inForeground)
	{
		hidden1 = hidden2 = false;
		if (((WindowPeek) mDlog1)->visible)
		{
			ShowHide (mDlog1, false);
			hidden1 = true;
		}
		if (((WindowPeek) mDlog2)->visible)
		{
			ShowHide (mDlog2, false);
			hidden2 = true;
		}
		if ((w = FrontWindow ()) != (WindowPtr) nil)
		{
			HiliteWindow (w, false);
			SkelActivate (w, false);
		}
	}
	else
	{
		if ((w = FrontWindow ()) != (WindowPtr) nil)
			HiliteWindow (w, false);
		if (hidden1)
			ShowHide (mDlog1, true);
		if (hidden2)
			ShowHide (mDlog2, true);
		if ((w = FrontWindow ()) != (WindowPtr) nil)
		{
			HiliteWindow (w, true);
			SkelActivate (w, true);
		}
		if (EventAvail (mDownMask, &event))
			(void) GetNextEvent (mDownMask, &event);
	}
}


/* ------------ */
/* Main program */
/* ------------ */


int
main (void)
{
	SkelInit ((SkelInitParamsPtr) nil);

	SkelSetSuspendResume (SuspendResume);

	/* 311 = ellipsis */
	SkelApple ((StringPtr) "\pAbout DialogSkel\311", DoAppleMenu);

	fileMenu = NewMenu (fileMenuID, (StringPtr) "\pFile");
	AppendMenu (fileMenu, (StringPtr) "\pShow Dialog 1;Show Dialog 2;Show Doc Window");
	AppendMenu (fileMenu, (StringPtr) "\pClose/W;(-;Quit/Q");
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, false);

	editMenu = NewMenu (editMenuID, (StringPtr) "\pEdit");
	AppendMenu (editMenu, (StringPtr) "\p(Undo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	(void) SkelMenu (editMenu, DoEditMenu, nil, false, false);
	
	DrawMenuBar ();
	SkelSetMenuHook (AdjustMenus);
	
	DocWindow (100, 125);
	
	mDlog1 = DemoDialog ((StringPtr) "\pModeless Dialog 1", 50, 50);
	mDlog2 = DemoDialog ((StringPtr) "\pModeless Dialog 2", 150, 200);
	SetWRefCon (mDlog1, (long) mDlog2);
	SetWRefCon (mDlog2, (long) mDlog1);
	SkelSetDlogStr (mDlog1, edit1, (StringPtr) "\pModeless Dialog 2");
	SkelSetDlogStr (mDlog2, edit1, (StringPtr) "\pModeless Dialog 1");
	SkelSetDlogProc (mDlog1, user1, drawIconProc);
	SkelSetDlogProc (mDlog2, user1, drawIconProc);
	SkelSetDlogRadioButtonSet (mDlog1, radio1, radio3, radio1);
	SkelSetDlogRadioButtonSet (mDlog2, radio1, radio3, radio1);
	SkelSetDlogCtlValue (mDlog1, check1, 1);
	SkelSetDlogCtlValue (mDlog2, check1, 1);
	SkelSetDlogCtlValue (mDlog1, check2, 1);
	SkelSetDlogCtlValue (mDlog2, check2, 1);

	SelectWindow (docWind);
	ShowWindow (docWind);
	SkelDoEvents (activMask + updateMask);
	SelectWindow (mDlog1);
	ShowWindow (mDlog1);
	SkelDoEvents (activMask + updateMask);
	SelectWindow (mDlog2);
	ShowWindow (mDlog2);
	SkelDoEvents (activMask + updateMask);

	SkelEventLoop ();
	SkelCleanup ();
}

