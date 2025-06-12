/*
 * Simple application showing how to create hierarchical menus with TransSkel.
 *
 * Nothing is actually done with any of the menu selections except Quit from
 * the File menu.
 *
 * 16 Feb 94 Version 1.00, Paul DuBois
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 */

# include	"TransSkel.h"


# define	aboutAlrtRes	1000		/* About box resource number*/


# define	fileMenuNum		1000		/* File menu */
# define	demoMenuNum		1001		/* Demo menu */
# define	sub1MenuNum		231			/* Demo menu submenus */
# define	sub2MenuNum		232
# define	sub3MenuNum		233
# define	demo2MenuNum	1005		/* Demo2 menu resource number */
# define	sub21MenuNum	241			/* Demo2 menu submenus */
# define	sub22MenuNum	242
# define	sub23MenuNum	243


/* file menu item numbers */

typedef enum
{
	quit = 1
};



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
MenuHandle	m;

	SkelApple ("\pAbout HierMenu\311", DoAppleMenu);
	m = NewMenu (fileMenuNum, "\pFile");
	AppendMenu (m, "\pQuit/Q");
	(void) SkelMenu (m, DoFileMenu, nil, false, false);

	/*
	 * Create hierarchical menu using NewMenu().  The parent menu
	 * items are specified as having submenus by indicating 033 (0x1b)
	 * as the command-key equivalent.  The submenus are attached by
	 * specifying the submenu numbers as the item mark numbers for
	 * items in the parent menu.
	 */

	m = NewMenu (demoMenuNum, "\pDemo");
	AppendMenu (m, "\pItem 1/\033;Item 2/\033;Item 3/\033");
	SetItemMark (m, 1, sub1MenuNum);
	SetItemMark (m, 2, sub2MenuNum);
	SetItemMark (m, 3, sub3MenuNum);
	(void) SkelMenu (m, nil, nil, false, false);
	m = NewMenu (sub1MenuNum, "\p");
	AppendMenu (m, "\pSubitem 1.1;Subitem 1.2;Subitem 1.3");
	(void) SkelMenu (m, nil, nil, true, false);
	m = NewMenu (sub2MenuNum, "\p");
	AppendMenu (m, "\pSubitem 2.1;Subitem 2.2;Subitem 2.3");
	(void) SkelMenu (m, nil, nil, true, false);
	m = NewMenu (sub3MenuNum, "\p");
	AppendMenu (m, "\pSubitem 3.1;Subitem 3.2;Subitem 3.3");
	(void) SkelMenu (m, nil, nil, true, false);

	/*
	 * Create hierarchical menu from resources using GetMenu().
	 * All the submenu information is specified in the resources.
	 */

	m = GetMenu (demo2MenuNum);
	(void) SkelMenu (m, nil, nil, false, false);
	m = GetMenu (sub21MenuNum);
	(void) SkelMenu (m, nil, nil, true, false);
	m = GetMenu (sub22MenuNum);
	(void) SkelMenu (m, nil, nil, true, false);
	m = GetMenu (sub23MenuNum);
	(void) SkelMenu (m, nil, nil, true, false);

	DrawMenuBar ();
}


/* -------------------------------------------------------------------- */
/*									Main								*/
/* -------------------------------------------------------------------- */


int
main (void)
{
	SkelInit ((SkelInitParamsPtr) nil);	/* initialize */
	SetupMenus ();						/* install menu handlers */
	SkelEventLoop ();					/* loop 'til Quit selected */
	SkelCleanup ();						/* clean up */
}
