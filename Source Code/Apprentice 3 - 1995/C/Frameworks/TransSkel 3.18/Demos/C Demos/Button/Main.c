/*
 * Button - Demonstration of TransSkel button outlining for document windows,
 * modeless dialogs, movable modal dialogs, and modal dialogs.  Accompanies
 * discussion in TransSkel Programmer's Note 10.
 *
 * Also demonstrates key-to-button mapping for document windows, modeless
 * dialogs, movable modal dialogs, and modal dialogs.
 *
 * Modal Dialog 1 demonstrates how to install an outliner for the button that's
 * indicated as the default in the dialog template, and how to make the
 * outline change state when the button does.
 *
 * Modal Dialog 2 demonstrates how to install an outliner when the button to
 * be outlined isn't necessarily the default, and how to change the button
 * with which the outline is associated.
 *
 * The document window demonstrates simple use of SkelDrawButtonOutline()
 * in a document window.
 *
 * 11 Jan 94 Paul DuBois
 *
 * 11 Jan 94 Release 1.00
 * 21 Feb 94
 * - Updated for TransSkel 3.11.
 * 22 Apr 94
 * - Tracks cursor in dialogs so it becomes an I-beam in edit text items.
 * 25 Apr 94
 * - Escape and command-period in document window cause Cancel button to flash.
 * 27 Apr 94 Release 1.01
 * - Added modeless dialog.  It acts just like the document window, but is
 * implemented differently.
 * 28 Apr 94
 * - Document window was wrong type (it had a grow region but shouldn't).
 * Fixed.
 * 30 Apr 94
 * - Added movable modal dialog (not available unless running at least System 7).
 * 01 May 94
 * - Adjust menus when movable modal dialog is in front so a modal dialog can't
 * be selected.
 * 18 Aug 94
 * - Updated for TransSkel 3.18 (Support for Universal headers, PowerPC,
 * Metrowerks).
 */

# include	"TransSkel.h"

# include	"Button.h"


/*
 * Handle selection of "About Button..." item from Apple menu
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
	case doModal1:
		DoModal1 ();
		break;
	case doModal2:
		DoModal2 ();
		break;
	case doModal3:
		DoModal3 ();
		break;
	case doMovable:
		DoMovableModal ();
		break;
	case quitApp:
		SkelStopEventLoop ();
		break;
	}
}


void
AdjustMenus (void)
{
MenuHandle	m;

	m = GetMHandle (skelAppleMenuID);
	if (SkelIsMMDlog (FrontWindow ()))	/* disable "About..." item */
		DisableItem (m, 1);
	else
		EnableItem (m, 1);
	m = GetMHandle (fileMenuRes);
	if (SkelIsMMDlog (FrontWindow ()))
	{
		DisableItem (m, doModal1);
		DisableItem (m, doModal2);
		DisableItem (m, doModal3);
		DisableItem (m, doMovable);
	}
	else
	{
		EnableItem (m, doModal1);
		EnableItem (m, doModal2);
		EnableItem (m, doModal3);
		EnableItem (m, doMovable);
	}
}


int
main (void)
{
MenuHandle	m;
long	result;

	SkelInit ((SkelInitParamsPtr) nil);	/* initialize */

	SkelApple ("\pAbout Button\311", DoAppleMenu);
	m = GetMenu (fileMenuRes);
	(void) SkelMenu (m, DoFileMenu, nil, false, true);
	if (SkelQuery (skelQSysVersion) < 0x00000700)
		DisableItem (m, doMovable);

	SetupDocument ();
	SetupModeless ();

	SkelEventLoop ();

	SkelCleanup ();						/* clean up */
}
