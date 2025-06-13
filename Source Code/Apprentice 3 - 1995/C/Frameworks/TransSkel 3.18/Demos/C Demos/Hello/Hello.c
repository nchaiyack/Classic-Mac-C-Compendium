/*
 * TransSkel version of "hello, world."
 *
 * Presents a window with "Hello, world." displayed centered in the window
 * when the window is active, and "Goodbye, world." when the window is
 * inactive (e.g., when the application is suspended, or when the "About"
 * alert comes up).
 *
 * 04 Feb 94 Version 1.00, Paul DuBois
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 */

# include	"TransSkel.h"

/* strings that appear in window */

# define	hello	"\pHello, world."
# define	goodbye	"\pGoodbye, world."

/* window positioning ratios */

# define	hRatio	FixRatio (1, 2)
# define	vRatio	FixRatio (1, 5)

/*
 * Resource numbers
 */

# define	aboutAlrtRes	1000					/* About box */
# define	fileMenuNum		(skelAppleMenuID + 1)	/* File menu */


/* file menu item numbers */

typedef enum
{
	quit = 1
};


static MenuHandle	fileMenu;

/* current phrase */

static StringPtr	phrase = hello;


/* -------------------------------------------------------------------- */
/*						Menu handling procedures						*/
/* -------------------------------------------------------------------- */


/*
 * Handle selection of "About Hello..." item from Apple menu
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
 */

static pascal void
DoFileMenu (short item)
{
	switch (item)
	{
	case quit:
		SkelStopEventLoop ();
		break;
	}
}


/*
 * Initialize menus.  Tell TransSkel to process the Apple menu
 * automatically, and associate the proper procedure with the
 * File menu.
 *
 * \311 is the ellipsis character.
 */

static void
SetupMenus (void)
{
	SkelApple ("\pAbout Hello\311", DoAppleMenu);
	fileMenu = NewMenu (fileMenuNum, "\pFile");
	AppendMenu (fileMenu, "\pQuit/Q");
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, false);

	DrawMenuBar ();
}


/* -------------------------------------------------------------------- */
/*					Window handling procedures							*/
/* -------------------------------------------------------------------- */


/*
 * Draw grow box of window in lower right hand corner
 */


static void
DrawGrowBox (WindowPtr w)
{
RgnHandle	oldClip;
Rect		r;

	r = w->portRect;
	r.left = r.right - 15;		/* draw only in corner */
	r.top = r.bottom - 15;
	oldClip = NewRgn ();
	GetClip (oldClip);
	ClipRect (&r);
	DrawGrowIcon (w);
	SetClip (oldClip);
	DisposeRgn (oldClip);
}


static pascal void
Update (Boolean resized)
{
WindowPtr	w;
Rect		r;
short		h, v;

	GetPort (&w);
	r = w->portRect;
	EraseRect (&r);
	h = (r.left + r.right - StringWidth (phrase)) / 2;
	v = (r.top + r.bottom) / 2;
	MoveTo (h, v);
	DrawString (phrase);
	DrawGrowBox (w);
}


/*
 * Normally the grow icon is drawn on a change of activation state, but since
 * the entire portRect is invalidated here, everything will be redrawn by
 * Update(), anyway, including the grow box.  So don't bother here.
 */

static pascal void
Activate (Boolean active)
{
WindowPtr	w;

	GetPort (&w);
	phrase = (active ? hello : goodbye);
	InvalRect (&w->portRect);
}


static pascal void
Clobber (void)
{
WindowPtr	w;

	GetPort (&w);
	DisposeWindow (w);
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
WindowPtr	w;
Rect	bounds;

	SetRect (&bounds, 0, 0, 200, 100);
	if (SkelQuery (skelQHasColorQD))
	{
		w = NewCWindow (nil, &bounds, "\pHowdy", false,
						documentProc+8, (WindowPtr) -1, false, 0L);
	}
	else
	{
		w = NewWindow (nil, &bounds, "\pHowdy", false,
						documentProc+8, (WindowPtr) -1, false, 0L);
	}
	SkelPositionWindow (w, skelPositionOnMainDevice,
									FixRatio (1, 2), FixRatio (1,5));
	(void) SkelWindow (w, nil, nil, Update, Activate, nil,
					Clobber, nil, false);
	TextFont (0);	/* select system font in default size */
	TextSize (0);
	SelectWindow (w);
	ShowWindow (w);
}


/* -------------------------------------------------------------------- */
/*									Main								*/
/* -------------------------------------------------------------------- */


int
main (void)
{
	SkelInit ((SkelInitParamsPtr) nil);	/* initialize */
	SetupMenus ();						/* install menu handlers */
	WindInit();							/* install window handler */
	SkelEventLoop ();					/* loop 'til Quit selected */
	SkelCleanup ();						/* clean up */
}
