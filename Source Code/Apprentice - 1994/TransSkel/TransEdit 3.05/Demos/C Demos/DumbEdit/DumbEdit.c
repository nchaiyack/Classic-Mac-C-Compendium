/*
 * DumbEdit - Multiple-window TransEdit Demonstration.
 *
 * 28 Oct 86 Paul DuBois
 *
 * 02 Feb 89
 * - Modified to work with TransSkel 2.0 and TransEdit 2.0.
 * - 2-byte and 4-byte integer types are typedef'ed to Integer and Longint
 * to ease porting.
 * 16 Jun 92
 * - Modified for TransSkel 3.00.
 * - Changed SetTextMenus () to compare new state against last and redraw
 * menu bar on a change. This avoids a subtle bug encountered when trying to
 * figure out whether to redraw the menu bar based on whether current and
 * previous front window were edit windows or not.
 * 05 Jun 93 Version 1.03
 * - Conversion for THINK C 6.0.
 * 04 Jan 94
 * - Undid Integer/LongInt type stuff back to short/long.
 * 20 Jan 94 Version 1.04
 * - Use a menu hook procedure to set menus when a mouse click occurs in the
 * menu bar, rather than setting them after every action that could change
 * menu item state.  This simplifies application logic somewhat.
 * - Added Select All item to Edit menu.
 * 21 Feb 94
 * - Updated for TransSkel 3.11, TransEdit 3.05.
 */

# include	"TransSkel.h"

# include	"TransEdit.h"


# define	maxSize		8		/* no. font sizes made available */
# define	hSize		300		/* horiz, vert size of new windows */
# define	vSize		205
# define	aboutAlrt	1000

# define	dumbCreator	'Dumb'	/* file creator type */


typedef enum		/* File menu item numbers */
{
	new = 1,		/* begin new window */
	open,			/* open existing file */
	close,			/* close file */
	/* --- */
	save = 5,		/* save file */
	saveAs,			/* save under another name */
	saveCopy,		/* save a copy w/o switching file binding */
	revert,			/* revert to version on disk */
	/* --- */
	quit = 10
};


typedef enum			/* Edit menu item numbers */
{
	undo = 1,
	/* --- */
	cut = 3,
	copy,
	paste,
	clear,
	selectAll
};



typedef enum		/* Format menu item numbers */
{
	wordWrap = 1,
	noWrap,
	/* --- */
	leftJust = 4,
	centerJust,
	rightJust
};


static MenuHandle	fileMenu;
static MenuHandle	editMenu;
static MenuHandle	fontMenu;
static MenuHandle	sizeMenu;
static MenuHandle	formatMenu;

static short	sizes[maxSize] = { 9, 10, 12, 14, 18, 20, 24, 48 };


/*
 * Set state of all the items in a menu, and unchecks them all.
 */

static void
SetMenuItems (MenuHandle m, Boolean enable)
{
short	i, nItems;

	nItems = CountMItems (m);

	for (i = 1; i <= nItems; ++i)
	{
		if (enable)
			EnableItem (m, i);
		else
			DisableItem (m, i);
		CheckItem (m, i, false);
		SetItemStyle (m, i, 0);
	}
}


/*
 * Set the Font, Size and Format menus so that the items corresponding
 * to the text characteristics of the window are checked.  If the
 * window isn't an edit window, dim items in all three menus.
 */

static void
SetTextMenus (void)
{
WindowPtr	w;
Str255		wFontName;
Str255		mFontName;
short		i, nItems;
TEHandle	hTE;
Boolean		isEditWind;

	w = FrontWindow ();

	isEditWind = IsEWindow (w);

	/* enable/disable menu items, toss checkmarks, make plain style */

	SetMenuItems (fontMenu, isEditWind);
	SetMenuItems (sizeMenu, isEditWind);
	SetMenuItems (formatMenu, isEditWind);

	if (!isEditWind)
		return;

	hTE = GetEWindowTE (w);

	/* check appropriate word wrap item */

	CheckItem (formatMenu, (**hTE).crOnly < 0 ? noWrap : wordWrap, true);

	/* check appropriate justification item */

	switch ((**hTE).just)
	{
	case teJustLeft:
		i = leftJust;
		break;
	case teJustRight:
		i = rightJust;
		break;
	case teJustCenter:
		i = centerJust;
		break;
	}
	CheckItem (formatMenu, i, true);

	/*
	 * Check appropriate font size item, and outline items for sizes
	 * present in resource files
	 */

	for (i = 0; i < maxSize; ++i)
	{
		if ((**hTE).txSize == sizes[i])
			CheckItem (sizeMenu, i + 1, true);
		if (RealFont ((**hTE).txFont, sizes[i]))
			SetItemStyle (sizeMenu, i + 1, outline);
	}

	/*
	 * Check appropriate font name item
	 */
	
	GetFontName ((**hTE).txFont, wFontName);	/* name of window font */
	nItems = CountMItems (fontMenu);			/* # fonts in menu */
	for (i = 1; i <= nItems; ++i)
	{
		GetItem (fontMenu, i, mFontName);	/* get font name */
		if (EqualString (wFontName, mFontName, false, true))
		{
			CheckItem (fontMenu, i, true);
			break;
		}
	}
}


/*
 * Set File/Edit menu items according to type of front window.
 *
 * The general behavior is:
 *
 * New and Open are always enabled, since a new edit window can always be
 * opened.
 *
 * Close enabled when an edit or DA window in front (i.e., when there's
 * a window at all).
 *
 * Save enabled for edit windows not bound to a file, and edit windows
 * bound to a file when they're dirty (typed into, Edit menu used to
 * do something to them).
 *
 * Save As and Save a Copy As enabled for edit windows.
 *
 * Revert enabled for edit windows bound to a file when they're dirty.
 *
 * Undo disabled when there's an edit window in front.
 */

static void
SetNonTextMenus (void)
{
WindowPtr	w;

	DisableItem (fileMenu, close);	/* assume no window at all */
	DisableItem (fileMenu, save);
	DisableItem (fileMenu, saveAs);
	DisableItem (fileMenu, saveCopy);
	DisableItem (fileMenu, revert);
	EnableItem (editMenu, undo);

	if ((w = FrontWindow ()) != (WindowPtr) nil)
		EnableItem (fileMenu, close);

	if (IsEWindow (w))			/* edit window in front */
	{
		EnableItem (fileMenu, saveAs);
		EnableItem (fileMenu, saveCopy);
		if (GetEWindowFile (w, nil) == false)	/* not bound to file */
		{
			EnableItem (fileMenu, save);
		}
		else if (IsEWindowDirty (w))		/* bound - is it dirty? */
		{
			EnableItem (fileMenu, save);
			EnableItem (fileMenu, revert);
		}
		DisableItem (editMenu, undo);
	}
}


/*
 * Menu hook, called when mouse click occurs in menu bar.
 */

static pascal void
AdjustMenus (void)
{
	SetNonTextMenus ();
	SetTextMenus ();
}


static void
MakeWind (Boolean bindToFile)
{
WindowPtr	w;
Rect		r;
short		offset;
static short	windCount = 0;

	if (FrontWindow () == nil)
		windCount = 0;
	SetRect (&r, 0, 0, hSize, vSize);
	offset = 50 + 25 * (windCount++ % 4);
	OffsetRect (&r, offset, offset);
	w = NewEWindow (&r, nil, true, (WindowPtr) -1L, true, 0L, bindToFile);
}


/*
 * File menu handler
 */

static pascal void
DoFileMenu (short item)
{
WindowPtr	theWind;

	theWind = FrontWindow ();
	switch (item)
	{
	case new:
		MakeWind (false);
		break;
	case open:
		MakeWind (true);
		break;
	case close:
		SkelClose (theWind);
		break;
	case save:
		(void) EWindowSave (theWind);
		break;
	case saveAs:
		(void) EWindowSaveAs (theWind);
		break;
	case saveCopy:
		(void) EWindowSaveCopy (theWind);
		break;
	case revert:
		(void) EWindowRevert (theWind);
		break;
	case quit:
		if (ClobberEWindows () == true)
			SkelStopEventLoop ();
		break;
	}
}


static pascal void
DoEditMenu (short item)
{
TEHandle	hTE;

	if (item == selectAll)
	{
		hTE = GetEWindowTE (FrontWindow ());
		if (hTE != (TEHandle) nil)
			TESetSelect (0L, 32767L, hTE);
	}
	else
		EWindowEditOp (item);
}


/*
 * Handle Font menu items
 */

static pascal void
DoFontMenu (short item)
{
short		font;
TEHandle	te;
WindowPtr	theWind;
Str255		theFontName;

	theWind = FrontWindow ();
	if ((te = GetEWindowTE (theWind)) == nil)
		return;				/* not an edit window */
	GetItem (fontMenu, item, theFontName);
	GetFNum (theFontName, &font);
	SetEWindowStyle (theWind, font, (**te).txSize, (**te).crOnly, (**te).just);
}


/*
 * Handle Size menu items
 */

static pascal void
DoSizeMenu (short item)
{
TEHandle	te;
WindowPtr	theWind;

	theWind = FrontWindow ();
	if ((te = GetEWindowTE (theWind)) == nil)
		return;				/* not an edit window */
	SetEWindowStyle (theWind, (**te).txFont, sizes[item-1], (**te).crOnly, (**te).just);
}


/*
 * Handle Format menu items
 */

static pascal void
DoFormatMenu (short item)
{
short		font, size, just, wrap;
TEHandle	te;
WindowPtr	theWind;

	theWind = FrontWindow ();
	if ((te = GetEWindowTE (theWind)) == nil)
		return;				/* not an edit window */
	font = (**te).txFont;
	size = (**te).txSize;
	just = (**te).just;
	wrap = (**te).crOnly;

	switch (item)
	{
	case wordWrap:
		wrap = 0;
		break;
	case noWrap:
		wrap = -1;
		break;
	case leftJust:
		just = teJustLeft;
		break;
	case centerJust:
		just = teJustCenter;
		break;
	case rightJust:
		just = teJustRight;
		break;
	}
	SetEWindowStyle (theWind, font, size, wrap, just);
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


int
main (void)
{
	/*
	 * Initialize TransSkel, create menus and install handlers.
	 */

	SkelInit ((SkelInitParamsPtr) nil);

	SkelApple ("\pAbout DumbEdit\311", DoAppleMenu);	/* 311 = ellipsis */

	fileMenu = NewMenu (1000, "\pFile");
	AppendMenu (fileMenu, "\pNew/N;Open\311/O;Close/W;(-;Save/S;Save As\311");
	AppendMenu (fileMenu, "\pSave a Copy As\311;Revert/R;(-;Quit/Q");
	(void) SkelMenu (fileMenu, DoFileMenu, nil, false, false);

	editMenu = NewMenu (1001, "\pEdit");
	AppendMenu (editMenu, "\pUndo/Z;(-;Cut/X;Copy/C;Paste/V;Clear");
	AppendMenu (editMenu, "\pSelect All/A");
	(void) SkelMenu (editMenu, DoEditMenu, nil, false, false);

	fontMenu = NewMenu (1002, "\pFont");
	AddResMenu (fontMenu, 'FONT');
	(void) SkelMenu (fontMenu, DoFontMenu, nil, false, false);

	sizeMenu = NewMenu (1003, "\pSize");
	AppendMenu (sizeMenu, "\p9 Point;10 Point;12 Point;14 Point");
	AppendMenu (sizeMenu, "\p18 Point;20 Point;24 Point;48 Point");
	(void) SkelMenu (sizeMenu, DoSizeMenu, nil, false, false);

	formatMenu = NewMenu (1004, "\pFormat");
	AppendMenu (formatMenu, "\pWord Wrap;No Word Wrap;(-;Left;Center;Right");
	(void) SkelMenu (formatMenu, DoFormatMenu, nil, false, false);

	DrawMenuBar ();

	SkelSetMenuHook (AdjustMenus);

	/*
	 * Do TransEdit-specific setup:  set creator for any files created,
	 * set default text style and event notification procedures for
	 * new windows.
	 */

	SetEWindowCreator (dumbCreator);
	SetEWindowStyle (nil, monaco, 9, 0, teJustLeft);
	/*SetEWindowProcs (nil, Key, nil, Close);*/

	/*
	 * Process events until Quit selected from File menu, then clean up.
	 */

	SkelEventLoop ();
	SkelCleanup ();
}
