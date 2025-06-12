/*
 * TransSkel demonstration:  Traditional Skel
 *
 * This program mimics the original Skel application:  one sizable,
 * dragable, non-closable dark gray window, an "About" alert and two
 * dialogs.  Desk accessories supported.
 *
 * 21 Apr 88 Paul DuBois
 * 29 Jan 89 Version 1.01
 * - Conversion for TransSkel 2.0.
 * 12 Jan 91 Version 1.02
 * - Conversion for TransSkel 3.00.
 * 05 Jun 93 Version 1.03
 * - Conversion for THINK C 6.0.
 * 18 Jan 94 Version 1.04
 * - Used TransSkel positioning routines to position About alert and the
 * dialog box on the screen.  The dialog resource also was made initially
 * invisible, since otherwise it appears and then moves.  Also added
 * a user item for outlining the default button.
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 */

# include	"TransSkel.h"


/*
 * Resource numbers
 */

# define	fileMenuRes		129		/* File menu */
# define	aboutAlrtRes	1000	/* About box */
# define	theWindRes		260		/* window */
# define	reportDlog		257		/* message dialog box */
# define	aboutStr		128		/* message strings */
# define	rattleStr		129
# define	frightStr		130


/* dialog item numbers */

typedef enum
{
	okayItem = 1,
	messageItem,
	titleItem,
	outlineItem
};


/* file menu item numbers */

typedef enum
{
	rattle = 1,
	frighten,
	/* --- */
	quit = 4
} fileItems;


static WindowPtr	theWind;

/*
 * Menu handles.  There isn't any apple menu here, since TransSkel will
 * be told to handle it itself.
 */

static MenuHandle	fileMenu;


/* -------------------------------------------------------------------- */
/*						Menu handling procedures						*/
/* -------------------------------------------------------------------- */


/*
 * Read a string resource and put into the Alert/Dialog paramtext
 * values
 */

static void
SetParamText (short strNum)
{
StringHandle	h;
SignedByte		flags;

	h = GetString (strNum);
	flags = HGetState ((Handle) h);
	HLock ((Handle) h);
	ParamText (*h, "\p", "\p", "\p");
	HSetState ((Handle) h, flags);
}


/*
 * Put up a dialog box with a message and an OK button.  The message
 * is stored in the 'STR ' resource whose number is passed as strNum.
 */

static void
Report (short strNum)
{
DialogPtr	theDialog;
GrafPtr		savePort;
short		itemHit;

	SetParamText (strNum);
	theDialog = GetNewDialog (reportDlog, nil, (WindowPtr) -1L);
	SkelPositionWindow (theDialog, skelPositionOnParentDevice,
								FixRatio (1, 2), FixRatio (1, 5));
	GetPort (&savePort);
	SetPort (theDialog);
	SkelSetDlogButtonOutliner (theDialog, outlineItem);
	ShowWindow (theDialog);
	ModalDialog (SkelDlogFilter (nil, true), &itemHit);
	SkelRmveDlogFilter ();
	DisposeDialog (theDialog);
	SetPort (savePort);
}


/*
 * Handle selection of "About Skel..." item from Apple menu
 */

static pascal void
DoAppleMenu (short item)
{
	SetParamText (aboutStr);
	(void) SkelAlert (aboutAlrtRes, SkelDlogFilter (nil, true),
											skelPositionOnParentDevice);
	SkelRmveDlogFilter ();
}


/*
 * Process selection from File menu.
 *
 * Rattle, Frighten	A dialog box with message
 * Quit	Request a halt by calling SkelStopEventLoop().  This
 *		makes SkelMain return.
 */

static pascal void
DoFileMenu (short item)
{
	switch (item)
	{
		case rattle:	Report (rattleStr); break;
		case frighten:	Report (frightStr); break;
		case quit:		SkelStopEventLoop (); break;	/* request halt */
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
	SkelApple ("\pAbout Skel\311", DoAppleMenu);
	fileMenu = GetMenu (fileMenuRes);
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, true);
}


/* -------------------------------------------------------------------- */
/*					Window handling procedures							*/
/* -------------------------------------------------------------------- */


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
	FillRect (&r, dkGray);
	r = theWind->portRect;		/* paint scroll bar areas white */
	r.left = r.right - 15;
	FillRect (&r, white);
	r = theWind->portRect;
	r.top = r.bottom - 15;
	FillRect (&r, white);
	DrawGrowIcon (theWind);
}


static pascal void
Activate (Boolean active)
{
	DrawGrowIcon (theWind);	/* make grow box reflect new window state */
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
	if (SkelQuery (skelQHasColorQD))
		theWind = GetNewCWindow (theWindRes, nil, (WindowPtr) -1L);
	else
		theWind = GetNewWindow (theWindRes, nil, (WindowPtr) -1L);
	if (theWind == (WindowPtr) nil)
		return;
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
