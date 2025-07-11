/*
 * TransSkel demonstration of use of SkelDlogFilter(), SkelRmveDlogFilter().
 *
 * A dialog is presented which can be made to jump around on the screen,
 * erasing parts of the background (accomplice) window.  The user has the
 * option of running the jumpy dialog with or without the filter, allowing
 * inspection of the different effect on the accomplice.
 *
 * As an added bonus :-), this demonstration also illustrates how to write
 * a really ugly dialog-specific filter, and how to plot SICN resources.
 *
 * A mouse warp option is included in the dialog, which allows the cursor
 * to be warped around on the screen to follow the dialog movement.  This
 * makes it a lot more convenient to press the same movement button several
 * times in a row.  Really fast successive clicks seem not to allow the
 * warp to be detected in time, though, so don't press your luck.
 *
 * This program should not be construed as hewing closely to the Macintosh
 * user interface guidelines, precisely because the modal dialog does jump
 * around on the screen.  The mouse warp option is also bad, since it relies
 * on undocumented low-memory globals.  Presumably it might crash on some
 * machines, e.g., those running A/UX.
 *
 * The "About" box can be shown either with or without the filter, too.
 * About the only difference is that without the filter, the grow box in
 * the document window doesn't get deactivated properly.
 *
 * 24 Jan 91 Paul DuBois
 * 24 Jan 91
 * - Written.  Version 1.00
 * 05 Jun 93 PD Version 1.01
 * - Fixed to compile under THINK C 6.0.
 * 01 Jan 94
 * - Use color GrafPort if available.
 * - Use TransSkel convenience routines to handle dialog items and button
 * outlining/flashing.
 * 13 Feb 94
 * - Minor revisions to use TransSkel convenience routines in lieu of
 * calculations that were done "by hand" here.
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 * 29 Apr 94
 * - Updated for TransSkel 3.13.
 * 04 Nov 94
 * - Updated for TransSkel 3.18 (Support for Universal headers, PowerPC,
 * Metrowerks).
 */

# include	"TransSkel.h"
# include	"Sicn.h"

/*
 * Resource numbers
 */

# define	aboutAlrtRes	1000	/* About box */
# define	jumpyDlogRes	1001	/* jumping dialog box */
# define	dirSicnRes		1001	/* direction SICNs */



/* Apple menu item numbers */

typedef enum
{
	aboutFilter = 1,
	aboutNoFilter
};


/* File menu and item numbers */

# define	fileMenuNum	1001

typedef enum
{
	jumpyFilter = 1,
	jumpyNoFilter,
	/* --- */
	quit = 4
};



static WindowPtr	theWind;

/*
 * Menu handles.  There isn't any apple menu here, since TransSkel will
 * be told to handle it itself.
 */

static MenuHandle	fileMenu;


/* -------------------------------------------------------- */
/*					Process Jumpy Dialog					*/
/* -------------------------------------------------------- */


static SicnHandle	hSicn;
static ControlHandle	warpBox;
static short		warpVal = 0;	/* mouse warp is off initially */


/*
 * Draw a SICN user item.
 */

static pascal void
DrawDlogUserItem (DialogPtr dlog, short item)
{
Rect	r;

	SkelGetDlogRect (dlog, item, &r);
	PlotSicn (&r, hSicn, item - 2);
}


static Boolean
TrackRect (Rect *rp)
{
short	i;
Boolean	inRect;
Point	pt;

	InvertRect (rp);
	inRect = true;
	while (StillDown ())
	{
		GetMouse (&pt);
		i = PtInRect (pt, rp);
		if (inRect && !i)				/* was in, now out */
		{
			InvertRect (rp);
			inRect = false;
		}
		else if (!inRect && i)			/* was out, now in */
		{
			InvertRect (rp);
			inRect = true;
		}
	}
	if (inRect)							/* mouse ended up in rect */
		InvertRect (rp);
	return (inRect);
}


/*
 * Dialog filter.  When there's a hit in one of the sicn items, track
 * it and return true if mouse-up occurs in same item.
 */

static pascal Boolean
DlogFilter (DialogPtr d, EventRecord *e, short *item)
{
Point	pt;
Rect	r;
short	i;

	if (e->what == mouseDown)
	{
		pt = e->where;
		GlobalToLocal (&pt);
		for (i = 2; i <= 18; i++)
		{
			SkelGetDlogRect (d, i, &r);
			if (PtInRect (pt, &r))
			{
				if (TrackRect (&r))
				{
					*item = i;
					return (true);
				}
			}
		}
	}
	return (false);
}


/*
 * Dialog filter 2.  This is used if the TransSkel filter isn't used.
 * It does the key mapping manually (which is normally done by the
 * TransSkel filter), then calls DlogFilter.
 */

static pascal Boolean
DlogFilter2 (DialogPtr d, EventRecord *e, short *item)
{
	if (SkelDlogMapKeyToButton (d, e, item, 1, 0))
		return (true);
	return (DlogFilter (d, e, item));
}


/*
 * Put up a dialog box that can be bounced around with the buttons.
 * When it comes up, a deactivate event for the background window
 * should be generated and processed.
 * When it is moved, an update event for the background window will
 * be generated.
 *
 * Item 1 is the ok button, the next 17 items are SICN items
 * which draw directional arrows.
 */

# define	delta	20	/* size of relative jumps */


/*
 * Set up a variable to point to the drawing procedure.  For 68K code this
 * is just a direct pointers to OutlineButton().  For PowerPC code it is a
 * routine descriptor into which is stuffed the address of OutlineButton().
 */

# if skelPPC		/* PowerPC code */

static RoutineDescriptor	drawDesc =
		BUILD_ROUTINE_DESCRIPTOR(uppUserItemProcInfo, DrawDlogUserItem);
static UserItemUPP	drawProc = (UserItemUPP) &drawDesc;

# else				/* 68K code */

static UserItemUPP	drawProc = DrawDlogUserItem;

# endif


static void
JumpyDialog (Boolean useSkelFilter)
{
ModalFilterUPP	filter;
DialogPtr	dlog;
GrafPtr	savePort;
short	itemHit;
Point	pt, mousePt;
Rect	scrnRect;
Rect	dlogRect;
short	dlogHt, dlogWid;
short	culh, culv;
short	i;

	if ((hSicn = (SicnHandle) GetResource ('SICN', dirSicnRes)) == nil)
	{
		SysBeep (1);
		return;
	}
	LoadResource ((Handle) hSicn);

	if (useSkelFilter)
		filter = NewModalFilterProc (DlogFilter);
	else
		filter = NewModalFilterProc (DlogFilter2);

	if (filter == (ModalFilterUPP) nil)
	{
		ReleaseResource ((Handle) hSicn);
		SysBeep (1);
		return;
	}

	GetPort (&savePort);
	dlog = GetNewDialog (jumpyDlogRes, nil, (WindowPtr) -1L);
	if (dlog == (DialogPtr) nil)
	{
		DisposeRoutineDescriptor (filter);
		ReleaseResource ((Handle) hSicn);
		SysBeep (1);
		return;
	}

	SkelPositionWindow (dlog, skelPositionOnMainDevice,
								FixRatio (1, 2), FixRatio (1, 5));

	/*
	 * Set drawing proc for button outline.
	 */
	SkelSetDlogButtonOutliner (dlog, 19);

	/* set warp box value */
	warpBox = SkelGetDlogCtl (dlog, 20);
	SetCtlValue (warpBox, warpVal);

	/* set drawing proc for sicn items */
	for (i = 2; i <= 18; i++)
		SkelSetDlogProc (dlog, i, drawProc);
	
	/* get size of dialog window */
	dlogRect = dlog->portRect;
	dlogWid = dlogRect.right - dlogRect.left;
	dlogHt = dlogRect.bottom - dlogRect.top;

	/* get max display rect for device on which dialog is located */
	(void) SkelGetWindowDevice (dlog, nil, &scrnRect);
	/* figure dlog border is 8 pixels - this keeps on screen */
	InsetRect (&scrnRect, 8, 8);

	/* coordinates of upper-left of dlog to center horizontally/vertically */
	culh = scrnRect.left + (scrnRect.right - scrnRect.left - dlogWid) / 2;
	culv = scrnRect.top + (scrnRect.bottom - scrnRect.top - dlogHt) / 2;

	SetPort (dlog);
	ShowWindow (dlog);
	for (;;)
	{
		if (useSkelFilter)
		{
			ModalDialog (SkelDlogFilter (filter, true), &itemHit);
			SkelRmveDlogFilter ();
		}
		else
			ModalDialog (filter, &itemHit);

		if (itemHit == 1)
			break;
		if (itemHit == 20)
		{
			warpVal = SkelToggleDlogCtlValue (dlog, 20);
			continue;
		}
		pt.h = dlog->portRect.left;
		pt.v = dlog->portRect.top;
		LocalToGlobal (&pt);
		GetMouse (&mousePt);
		LocalToGlobal (&mousePt);
		mousePt.h -= pt.h;
		mousePt.v -= pt.v;
		switch (itemHit)
		{
		case 2:				/* to center */
			pt.h = culh;
			pt.v = culv;
			break;
		case 3:				/* up a little */
			pt.v -= delta;
			break;
		case 4:				/* up/right a little */
			pt.h += delta;
			pt.v -= delta;
			break;
		case 5:				/* right a little */
			pt.h += delta;
			break;
		case 6:				/* down/right a little */
			pt.h += delta;
			pt.v += delta;
			break;
		case 7:				/* down a little */
			pt.v += delta;
			break;
		case 8:				/* down/left a little */
			pt.h -= delta;
			pt.v += delta;
			break;
		case 9:				/* left a little */
			pt.h -= delta;
			break;
		case 10:			/* up/left a little */
			pt.h -= delta;
			pt.v -= delta;
			break;
		case 11:			/* top center */
			pt.h = culh;
			pt.v = scrnRect.top;
			break;
		case 12:			/* upper right corner */
			pt.h = scrnRect.right - dlogWid;
			pt.v = scrnRect.top;
			break;
		case 13:			/* right center */
			pt.h = scrnRect.right - dlogWid;
			pt.v = culv;
			break;
		case 14:			/* lower right corner */
			pt.h = scrnRect.right - dlogWid;
			pt.v = scrnRect.bottom - dlogHt;
			break;
		case 15:			/* bottom center */
			pt.h = culh;
			pt.v = scrnRect.bottom - dlogHt;
			break;
		case 16:			/* lower left corner */
			pt.h = scrnRect.left;
			pt.v = scrnRect.bottom - dlogHt;
			break;
		case 17:			/* left center */
			pt.h = scrnRect.left;
			pt.v = culv;
			break;
		case 18:			/* upper left corner */
			pt.h = scrnRect.left;
			pt.v = scrnRect.top;
			break;
		}
		/*
		 * Clip coords to keep dialog on screen.  That's not so great if there
		 * are multiple monitors, but this is just a demonstration program
		 * anyway.
		 */
		if (pt.h < scrnRect.left)
			pt.h = scrnRect.left;
		if (pt.h > scrnRect.right - dlogWid)
			pt.h = scrnRect.right - dlogWid;
		if (pt.v < scrnRect.top)
			pt.v = scrnRect.top;
		if (pt.v > scrnRect.bottom - dlogHt)
			pt.v = scrnRect.bottom - dlogHt;
		if (warpVal)
		{
			HideCursor ();
			mousePt.h += pt.h;
			mousePt.v += pt.v;
			/* naughty use of undocumented low-memory globals */
			*(long *) 0x82c = *(long *) &mousePt;
			*(long *) 0x828 = *(long *) &mousePt;
			*(short *) 0x8ce = 0xffff;
		}
		MoveWindow (dlog, pt.h, pt.v, false);
		if (warpVal)
			ShowCursor ();
	}
	SetPort (savePort);
	DisposeDialog (dlog);
	DisposeRoutineDescriptor (filter);
	ReleaseResource ((Handle) hSicn);
}


/* -------------------------------------------------------------------- */
/*						Menu handling procedures						*/
/* -------------------------------------------------------------------- */


/*
 * Handle selection of "About Filter..." item from Apple menu
 */

static pascal void
DoAppleMenu (short item)
{
	switch (item)
	{
	case aboutFilter:
		(void) SkelAlert (aboutAlrtRes, SkelDlogFilter (nil, true),
											skelPositionOnParentDevice);
		SkelRmveDlogFilter ();
		break;
	case aboutNoFilter:
		(void) SkelAlert (aboutAlrtRes, nil, skelPositionOnParentDevice);
		break;
	}
}


/*
 * Process selection from File menu.
 */

static pascal void
DoFileMenu (short item)
{
	switch (item)
	{
		case jumpyFilter:	JumpyDialog (true); break;
		case jumpyNoFilter:	JumpyDialog (false); break;
		case quit:			SkelStopEventLoop (); break;
	}
}


/*
 * Initialize menus.  Tell TransSkel to process the Apple menu
 * automatically, and associate the proper procedures with the
 * File and Edit menus.
 *
 * \311 is the ellipsis character.
 */

static void
SetUpMenus (void)
{
	SkelApple ((StringPtr) "\pFiltered \322About Filter\311\323;Unfiltered \322About Filter\311\323",
					DoAppleMenu);
	fileMenu = NewMenu (fileMenuNum, "\pFile");
	AppendMenu (fileMenu, "\pFiltered Jump Dialog\311/F");
	AppendMenu (fileMenu, "\pUnfiltered Jump Dialog\311/U");
	AppendMenu (fileMenu, "\p(-;Quit/Q");
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, true);
}


/* -------------------------------------------------------------------- */
/*					Window handling procedures							*/
/* -------------------------------------------------------------------- */


static pascal void
Activate (Boolean active)
{
	DrawGrowIcon (theWind);	/* make grow box reflect new window state */
}


/*
 * On update event, can ignore the resizing information, since the whole
 * window is always redrawn in terms of the current size, anyway.
 * Content area is dark gray except scroll bar areas, which are white.
 * Draw grow box as well.
 *
 * Note that we can assume the current port is set to theWind.
 */

static pascal void
Update (Boolean resized)
{
Rect	r;

	r = theWind->portRect;		/* paint window dark gray */
	r.bottom -= 15;				/* don't bother painting the */
	r.right -= 15;				/* scroll bar areas */
	FillRect (&r, (ConstPatternParam) &qd.ltGray);
	r = theWind->portRect;		/* paint scroll bar areas white */
	r.left = r.right - 15;
	FillRect (&r, (ConstPatternParam) &qd.white);
	r = theWind->portRect;
	r.top = r.bottom - 15;
	FillRect (&r, (ConstPatternParam) &qd.white);
	DrawGrowIcon (theWind);
}


static pascal void
Clobber (void)
{
	DisposeWindow (theWind);
}


/*
 * Read window from resource file and install handler for it.  Mouse
 * and key clicks are ignored.  There is no close proc since the window
 * doesn't have a close box.  There is no idle proc since nothing is
 * done while the window is in front (all the things that are done are
 * handled by TransSkel).
 */

static void
WindInit (void)
{
Rect	bounds;

	SetRect (&bounds, 0, 0, 500, 282);
	OffsetRect (&bounds, 4, 40);
	if (SkelQuery (skelQHasColorQD))
	{
		theWind = NewCWindow (nil, &bounds, "\pThe Dialog�s Accomplice", true,
						documentProc+8, (WindowPtr) -1, false, 0L);
	}
	else
	{
		theWind = NewWindow (nil, &bounds, "\pThe Dialog�s Accomplice", true,
						documentProc+8, (WindowPtr) -1, false, 0L);
	}
	(void) SkelWindow (theWind, nil, nil, Update, Activate, nil,
					Clobber, nil, false);
}


/* -------------------------------------------------------------------- */
/*									Main								*/
/* -------------------------------------------------------------------- */


int
main (void)
{
	SkelInit ((SkelInitParamsPtr) nil);	/* initialize */
	SetUpMenus ();						/* install menu handlers */
	WindInit();							/* install window handler */
	SkelEventLoop ();					/* loop 'til Quit selected */
	SkelCleanup ();						/* clean up */
}
