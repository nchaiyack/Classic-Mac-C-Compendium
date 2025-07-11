/*
 * Bug: doesn't properly keep bottom of window from going off screen.
 *
 * ManyWind -- TransSkel demonstration
 *
 * This application allows up to twenty windows to be created at once,
 * with the New item under the File menu.  The name of each window
 * appears under the Window menu (which is not created until at least
 * one window exists).  Selecting the window name from the Window menu
 * brings the window to the front.  For every window created, Skel is
 * told to create a new handler.  If the window's close box is clicked,
 * the handler removes the window name from the Window menu, disposes
 * of the window, and removes itself from the window handler list.  If
 * the window was the last window, the Window menu handler removes
 * itself from the menu handler list.
 *
 * When the first window is created, a Color menu also appears.  This
 * allows the color of the content region of the frontmost window to
 * be changed.  It goes away when the last window is closed.
 *
 * To quit, select Quit from the File menu or type command-Q.
 *
 * ManyWind demonstrates dynamic window and menu creation and disposal.
 * It also shows how handler procedures may be shared among handlers
 * for different windows.
 *
 * 21 Apr 88 Version 1.00
 * - Created.  Paul DuBois.
 * 29 Jan 89 Version 1.01
 * - Conversion for TransSkel 2.0.
 * 30 Jan 91 Version 1.02
 * - Conversion for TransSkel 3.00.
 * 05 Jun 93 Version 1.03
 * - Conversion for THINK C 6.0.
 * 13 Nov 93
 * - Added Close item to file menu.
 * 11 Feb 94
 * - Minor revisions and bug fixes.
 * 15 Feb 94
 * - Added item checking for Color and Window menus.
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 */

# include	"TransSkel.h"


# define	maxWind	20	/* maximum number of windows existing at once */


enum				/* menu numbers */
{
	aMenuNum = skelAppleMenuID,	/* Apple menu */
	fMenuNum,					/* File menu */
	wMenuNum,					/* Window menu */
	cMenuNum					/* Color menu */
};


enum				/* File menu item numbers */
{
	newWind = 1,
	closeWind,
	/* --- */
	quitApp = 4
};

enum				/* Color menu items numbers */
{
	cWhite = 1,
	cLtGray,
	cGray,
	cDkGray,
	cBlack
};


static void MakeWindow(void);


MenuHandle	fileMenu;
MenuHandle	windowMenu;
MenuHandle	colorMenu;

short	windCount = 0;	/* number of currently existing windows */
long	windNum = 0;	/* id of last window created */


/* ------------- */
/* Menu handling */
/* ------------- */


static pascal void
DoFileMenu (short item)
{
WindowPtr	w;

	switch (item)
	{
		case newWind:					/* make a new window */
			MakeWindow ();
			break;
		case closeWind:
			SkelClose (FrontWindow ());
			break;
		case quitApp:					/* tell SkelEventLoop() to quit */
			SkelStopEventLoop ();
			break;
	}
}


static pascal void
DoWindowMenu (short item)
{
Str255		iTitle, wTitle;
WindowPtr	w;

	GetItem (windowMenu, item, iTitle);	/* get window name */
	for (w = FrontWindow (); w != nil; w = (WindowPtr) ((WindowPeek)w)->nextWindow)
	{
		GetWTitle (w, wTitle);
		if (EqualString (iTitle, wTitle, false, true))
		{
			SelectWindow (w);
			break;
		}
	}
}


/*
 * Change the background pattern of the frontmost window.
 * Ignore if the front window is a DA window.
 */

static pascal void
DoColorMenu (short item)
{
WindowPtr	w;

	w = FrontWindow ();
	if (((WindowPeek) w)->windowKind < 0) return;	/* front is DA window */
	switch (item)
	{
		case cWhite:	BackPat (white); break;
		case cLtGray:	BackPat (ltGray); break;
		case cGray:		BackPat (gray); break;
		case cDkGray:	BackPat (dkGray); break;
		case cBlack:	BackPat (black); break;
	}
	SetWRefCon (w, item);		/* save item number for menu checkmarking */
	EraseRect (&w->portRect);
}


static pascal void
DoMClobber (MenuHandle m)
{
	DisposeMenu (m);
}


static void
SetItemEnableState (MenuHandle m, short item, Boolean state)
{
	if (state)
		EnableItem (m, item);
	else
		DisableItem (m, item);
}


/*
 * Adjust menus when mouse click occurs in menu bar.
 * File menu:
 * - New is enabled if window count hasn't exceeded limit.
 * - Close is enabled if there is a window visible.
 * Color menu:
 * - Check item corresponding to color of frontmost window.
 * Window menu:
 * - Check item corresponding to frontmost window.
 */
 
static pascal void
AdjustMenus (void)
{
short	nItems, i;
Str255	iTitle, wTitle;
short	mark;

	SetItemEnableState(fileMenu, newWind, windCount < maxWind);
	SetItemEnableState(fileMenu, closeWind, FrontWindow() != (WindowPtr) nil);
	if (windCount > 0)
	{
		for (i = cWhite; i <= cBlack; i++)
		{
			mark = (GetWRefCon (FrontWindow ()) == i ? checkMark : noMark);
			SetItemMark (colorMenu, i, mark);
		}
		GetWTitle (FrontWindow(), wTitle);
		nItems = CountMItems (windowMenu);
		for (i = 1; i <= nItems; i++)
		{
			GetItem (windowMenu, i, iTitle);
			mark = (EqualString (iTitle, wTitle, false, true) ? checkMark : noMark);
			SetItemMark (windowMenu, i, mark);
		}
	}
}


/* --------------- */
/* Window handling */
/* --------------- */


static pascal void
DoWUpdate (Boolean resized)
{
WindowPtr	w;

	GetPort (&w);
	EraseRect (&w->portRect);	/* repaint w/background pattern */
}


/*
 * Mouse was clicked in close box.  Remove the window handler (which
 * causes the window to be disposed of), and delete the window title
 * from the Window menu.  If the window was the last one, delete the
 * Window and Color menus entirely.
 *
 * Skel makes sure the port is pointing to the appropriate window, so
 * this procedure can determine which window had its close box clicked,
 * without being told explicitly.
 */

static pascal void
DoWClose (void)
{
WindowPtr	w;

	GetPort (&w);			/* window to be closed */
	SkelRmveWind (w);
}


/*
 * Dispose of window.  Skel makes sure the port is pointing to the
 * appropriate window, so this procedure can determine which window
 * is to be disposed, of without being told explicitly.
 *
 * Also delete the window title from the Window menu.  If the window
 * was the last one, delete the Window and Color menus entirely.
 */

static pascal void
DoWClobber (void)
{
WindowPtr	w;
short		i, mItems;
Str255		iTitle, wTitle;

	GetPort (&w);			/* window to be closed */
	GetWTitle (w, wTitle);
	DisposeWindow (w);
	if (--windCount == 0)
	{
		SkelRmveMenu (windowMenu);	/* last window - clobber menus */
		SkelRmveMenu (colorMenu);
	}
	else
	{
		for (i = 1, mItems = CountMItems (windowMenu); i <= mItems; ++i)
		{
			GetItem (windowMenu, i, iTitle);
			if (EqualString (iTitle, wTitle, false, true))
			{
				DelMenuItem (windowMenu, i);
				break;
			}
		}
	}
}


/*
 * Make new window.  Locate at (100, 100) if no other windows, else
 * offset slightly from front window.  The window title is the next
 * window number (1, 2, 3, ...).  If this is the first window, create
 * the Window and Color menus.  Add the window title as the last item
 * of the Window menu.
 *
 * If the maximum window count is reached, disable New in the
 * File menu.
 */

static void
MakeWindow (void)
{
WindowPtr	w;
Rect		r;
Str255		s;

	if ((w = FrontWindow ()) == (WindowPtr) nil)
		SetRect (&r, 100, 100, 300, 250);
	else
	{
		SkelGetWindContentRect (w, &r);
		OffsetRect (&r, 20, 20);
		if (r.left > 480 || r.top > 300)	/* keep on screen */
			OffsetRect (&r, 40 - r.left, 40 - r.top);
	}
	NumToString (++windNum, s);
	if (SkelQuery (skelQHasColorQD))
		w = NewCWindow (nil, &r, s, true, noGrowDocProc, (WindowPtr) -1L, true, 0L);
	else
		w = NewWindow (nil, &r, s, true, noGrowDocProc, (WindowPtr) -1L, true, 0L);
	(void) SkelWindow (w,
				nil,		/* mouseclicks */
				nil,		/* key clicks */
				DoWUpdate,	/* updates */
				nil,		/* activate/deactivate events */
				DoWClose,	/* close window, remove from menu */
				DoWClobber,	/* dispose of window */
				nil,		/* idle proc */
				false);		/* irrelevant, since no idle proc */

	if (windCount++ == 0)	/* if first window, create new menus */
	{
		colorMenu = NewMenu (cMenuNum, "\pColor");
		AppendMenu (colorMenu, "\pWhite;Light Gray;Gray;Dark Gray;Black");
		(void) SkelMenu (colorMenu, DoColorMenu, DoMClobber, false, false);
		windowMenu = NewMenu (wMenuNum, "\pWindow");
		(void) SkelMenu (windowMenu, DoWindowMenu, DoMClobber, false, true);
	}
	AppendMenu (windowMenu, s);
	SetWRefCon (w, cWhite);
}


static void
SetupMenus (void)
{
	SkelApple (nil, nil);						/* initialize Apple menu */
	fileMenu = NewMenu (fMenuNum, "\pFile");	/* make File menu handler */
	AppendMenu (fileMenu, "\pNew/N;Close/W;(-;Quit/Q");
	(void) SkelMenu (fileMenu, DoFileMenu, DoMClobber, false, true);
	SkelSetMenuHook (AdjustMenus);
}


/* ------------ */
/* Main program */
/* ------------ */

int
main (void)
{
	SkelInit ((SkelInitParamsPtr) nil);			/* initialize */
	SetupMenus ();
	SkelEventLoop ();							/* loop 'til Quit selected */
	SkelCleanup ();								/* clean up */
}
