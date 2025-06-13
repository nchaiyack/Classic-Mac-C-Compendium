/*
 * Should add menu hook routine to dim Close item when no windows are active.
 *
 * MultiSkel - TransSkel multiple-window demonstration main module
 *
 * This module performs setup and termination operations, installs
 * the window and menu handlers, and processes menu item selections.
 *
 * There are four window handlers in this demonstration.  The code
 * for each handler is in its own module.
 *
 * Help Window		Scrollable non-editable text window
 * Edit Window		Non-scrollable editable text window
 * Zoom Window		Non-manipulable graphics display window
 * Region Window	Manipulable graphics display window
 *
 * 21 Apr 88 Paul DuBois
 * 29 Jan 89 Version 1.01
 * - Conversion for TransSkel 2.0.
 * 12 Jan 91 Version 1.02
 * - Conversion for TransSkel 3.00.
 * 05 Jun 93 Version 1.03
 * - Conversion for THINK C 6.0.
 * 15 Feb 94
 * - Numerous minor revisions.
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 * 04 Nov 94
 * - Updated for TransSkel 3.18 (Support for Universal headers, PowerPC,
 * Metrowerks).
 */

# include	"TransSkel.h"

# include	"MultiSkel.h"


/* File menu item numbers */

typedef enum {
	open = 1,
	close,
	/* --- */
	quit = 4
} fileItems;


WindowPtr	helpWind;
WindowPtr	editWind;
WindowPtr	zoomWind;
WindowPtr	rgnWind;


/*
 * Menu handles.  There isn't any Apple menu here, since TransSkel will
 * be told to handle it itself.
 */

static MenuHandle	fileMenu;
MenuHandle			editMenu;

static RgnHandle	oldClip;


/*
 * Miscellaneous routines
 * These take care of drawing the grow box and the line along
 * the right edge of the window, and of setting and resetting the clip
 * region to disallow drawing in that right edge by the other drawing
 * routines.
 */


void
DrawGrowBox (WindowPtr wind)
{
Rect		r;
RgnHandle	oldClip;

	r = wind->portRect;
	r.left = r.right - 15;		/* draw only along right edge */
	oldClip = NewRgn ();
	GetClip (oldClip);
	ClipRect (&r);
	DrawGrowIcon (wind);
	SetClip (oldClip);
	DisposeRgn (oldClip);
}


void
SetWindClip (WindowPtr wind)
{
Rect		r;

	r = wind->portRect;
	r.right -= 15;		/* don't draw along right edge */
	oldClip = NewRgn ();
	GetClip (oldClip);
	ClipRect (&r);
}


void
ResetWindClip (void)
{
	SetClip (oldClip);
	DisposeRgn (oldClip);
}


/*
 * Show a window if it's not visible.  Select the window FIRST, then
 * show it, so that it comes up in front.  Otherwise it will be drawn
 * in back then brought to the front, which is ugly.
 *
 * The test for visibility must be done carefully:  the window manager
 * stores 255 and 0 for true and false, not real boolean values.
 */

static void
MyShowWindow (WindowPtr wind)
{

	if (((WindowPeek) wind)->visible == false)
	{
		SelectWindow (wind);
		ShowWindow (wind);
	}
}


/*
 * Handle selection of About MultiSkel... item from Apple menu
 */

static pascal void
DoAppleMenu (short item)
{
	(void) SkelAlert (aboutAlrtRes, SkelDlogFilter (nil, true),
										skelPositionOnParentDevice);
	SkelRmveDlogFilter ();
}


/*
 * Process selection from File menu.
 *
 * Open		Make all four windows visible
 * Close	Hide the frontmost window.  If it belongs to a desk accessory,
 *			close the accessory.
 * Quit		Request a halt by calling SkelHalt().  This makes SkelMain
 *			return.
 */

static pascal void
DoFileMenu (short item)
{
	switch (item)
	{
		case open:
			MyShowWindow (rgnWind);
			MyShowWindow (zoomWind);
			MyShowWindow (editWind);
			MyShowWindow (helpWind);
			break;

		case close:
			SkelClose (FrontWindow ());
			break;

		case quit:
			SkelStopEventLoop ();		/* request halt */
			break;
	}
}


/*
 * Initialize menus.  Tell TransSkel to process the Apple menu automatically,
 * and associate the proper procedures with the File and Edit menus.
 * The Edit menu is enabled only when the Edit is frontmost.
 *
 * \311 = ellipsis character
 */

static void
SetUpMenus (void)
{
	SkelApple ((StringPtr) "\pAbout MultiSkel\311", DoAppleMenu);
	fileMenu = GetMenu (fileMenuRes);
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, false);
	editMenu = GetMenu (editMenuRes);
	(void) SkelMenu (editMenu, EditWindEditMenu, nil, false, true);
}


int
main (void)
{
long	f, b;

	SkelInit ((SkelInitParamsPtr) nil);
	SkelGetWaitTimes (&f, &b);
	b = f;
	SkelSetWaitTimes (f, b);
	SetUpMenus ();			/* install menu handlers */
	RgnWindInit ();			/* install window handlers  */
	ZoomWindInit ();
	EditWindInit ();
	HelpWindInit ();
	SkelEventLoop ();		/* process events */
	SkelCleanup ();			/* throw away windows and menus */
}
