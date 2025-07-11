/*
 * TinyEdit - Minimal TransEdit Demonstration.
 *
 * This application is about the smallest that can be written using TransEdit
 * It allows for a single edit window, created either blank or with contents
 * read from a file.  It also provides the standard edit operations from the
 * Edit menu.
 *
 * 08 Nov 86 Paul DuBois
 * 02 Feb 89 Version 1.01
 * - Modified to work with TransSkel 2.0 and TransEdit 2.0. 2-byte and 4-byte
 * integer types are typedef'ed to Integer and Longint to ease porting.
 * 16 Jun 92 Version 1.02
 * - Modified for TransEdit 3.00.
 * 06 Jun 93 Version 1.03
 * - Conversion for THINK C 6.0.
 * 04 Jan 94
 * - Undid Integer/LongInt type stuff back to short/long.
 * 20 Jan 94 Version 1.04
 * - Use a menu hook procedure to set menus when a mouse click occurs in the
 * menu bar, rather than setting them after every action that could change
 * menu item state.  This simplifies application logic somewhat.
 * 21 Jan 94
 * - Use GetNewEWindow() rather than NewEWindow().
 * 21 Feb 94
 * - Updated for TransSkel 3.11, TransEdit 3.05.
 */

# include	"TransSkel.h"

# include	"TransEdit.h"


# define	aboutAlrt	1000	/* "About..." alert number */
# define	windRsrc	1000	/* window resource number */

typedef enum		/* File menu item numbers */
{
	newWind = 1,		/* begin new window */
	openWind,			/* open existing file */
	closeWind,			/* close window */
	/* --- */
	quitApp = 5
};


typedef enum			/* Edit menu item numbers */
{
	undo = 1,
	/* --- */
	cut = 3,
	copy,
	paste,
	clear
};


static WindowPtr	editWind = nil;		/* non-nil if edit window open */
static MenuHandle	fileMenu;
static MenuHandle	editMenu;


static pascal void
Close (void)
{
	if (EWindowClose (editWind))
		editWind = (WindowPtr) nil;
}


/*
 * Handle selection of About... item from Apple menu
 */

static pascal void
DoAppleMenu (short item)
{
	(void) SkelAlert (aboutAlrt, SkelDlogFilter (nil, true),
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
	case newWind:
		editWind = GetNewEWindow (windRsrc, (WindowPtr) -1L, false);
		break;
	case openWind:
		editWind = GetNewEWindow (windRsrc, (WindowPtr) -1L, true);
		break;
	case closeWind:
		SkelClose (FrontWindow ());
		break;
	case quitApp:
		if (ClobberEWindows () == true)
			SkelStopEventLoop ();
		break;
	}
}


/*
 * Menu hook, called to adjust the menus when a mouse click occurs in
 * the menu bar.
 *
 * Set File/Edit menu items according to type of front window.
 *
 * The general behavior is:
 *
 * New and Open enabled if an edit window is not open, otherwise they
 * are disabled.
 *
 * Close enabled when an edit or DA window is in front (i.e.,
 * when there's a window at all).
 *
 * Undo disabled when the edit window is in front.
 */

static pascal void
AdjustMenus (void)
{
	DisableItem (fileMenu, closeWind);	/* assume no window at all */
	EnableItem (editMenu, undo);

	if (FrontWindow () != nil)
	{
		EnableItem (fileMenu, closeWind);
		if (IsEWindow (FrontWindow ()))	/* the edit window's in front */
			DisableItem (editMenu, undo);
	}

	if (editWind == nil)
	{
		EnableItem (fileMenu, newWind);
		EnableItem (fileMenu, openWind);
	}
	else
	{
		DisableItem (fileMenu, newWind);
		DisableItem (fileMenu, openWind);
	}
}


int
main (void)
{
	/*
	 * Initialize TransSkel, create menus and install handlers.
	 */

	SkelInit ((SkelInitParamsPtr) nil);

	SkelApple ("\pAbout TinyEdit\311", DoAppleMenu);	/* 311 = ellipsis */

	fileMenu = NewMenu (1000, "\pFile");
	AppendMenu (fileMenu, "\pNew/N;Open\311/O;(Close/W;(-;Quit/Q");
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, false);

	editMenu = NewMenu (1001, "\pEdit");
	AppendMenu (editMenu, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	(void) SkelMenu (editMenu, EWindowEditOp, nil, false, false);

	DrawMenuBar ();
	SkelSetMenuHook (AdjustMenus);

	SetEWindowProcs (nil, nil, nil, Close);

	/*
	 * Process events until user quits,
	 * then clean up and exit
	 */

	SkelEventLoop ();
	SkelCleanup ();
}
