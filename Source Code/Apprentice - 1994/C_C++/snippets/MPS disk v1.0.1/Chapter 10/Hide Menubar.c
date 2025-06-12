#include "Hide Menubar.h"

/*******************************************************************************

	Private constants and variables

*******************************************************************************/

const Boolean	kProhibitClicks	= FALSE;	/* Set to TRUE to prohibit the user
											   from clicking on the menubar
											   while hidden. If FALSE, the menu
											   will still respond to clicks. */

Boolean			gMenuBarHidden	= FALSE;	/* Current state of the menubar. */

short			gOldeMBarHeight;			/* Saves the height of the menubar
											   while we have it hidden. */

RgnHandle		gOldeGrayRgn;				/* Saves the region defining the
											   desktop; we change it when
											   hiding the menubar. */


/*******************************************************************************

	ToggleMenuBar

	Routine that can be called by clients to change the state of the menubar.
	It examines the current state of the menubar (as recorded by
	gMenuBarHidden) and calls either ShowMenuBar or HideMenuBar as
	appropriate.

*******************************************************************************/
void ToggleMenuBar()
{
	if (gMenuBarHidden)
		ShowMenuBar();
	else
		HideMenuBar();
}


/*******************************************************************************

	HideMenuBar

	Hides the menubar if it is visible. This is done by adding the space used
	by the menubar to the desktop region. Normally, the menubar is clipped out
	by the Window Manager, so nothing ever draws on top of it. We make it so
	we can draw on top of it, and then call the Window Manager to refresh the
	desktop.

	If we want to prohibit the user from clicking on the menubar while it’s
	hidden, we set the height of the menubar to 0. Note the instruction we use
	to accomplish this:

		GetMBarHeight() = 0;

	The menubar’s height is stored in a low-memory global at 0x0BAA. Normally
	we use GetMBarHeight to retrieve this value to help determine things like
	window placement. However, GetMBarHeight isn’t really a function; it’s a
	macro:

		#define GetMBarHeight() (*(short*)0x0BAA)

	Because it’s a macro, we can use it on the left side of the equation. The
	whole instruction expands to:

		(*(short*)0x0BAA) = 0;

	Which is just C’s way of saying “store zero into low-memory location
	0x0BAA.”

	Note that if we should be careful about calling DrawMenuBar while the
	menubar is hidden. While DrawMenuBar will faithfully draw the menubar, the
	resulting image is very volatile. Any window dragged over the menubar will
	wipe out chunks of it.

*******************************************************************************/
void HideMenuBar()
{
	RgnHandle	menuRgn;

	if (!gMenuBarHidden) {
		gOldeMBarHeight = GetMBarHeight();

		if (kProhibitClicks)
			GetMBarHeight() = 0;

		if (gOldeGrayRgn == nil)
			gOldeGrayRgn = NewRgn();

		CopyRgn(GetGrayRgn(), gOldeGrayRgn);

		menuRgn = NewRgn();
		SetToMenuRect(menuRgn);
		UnionRgn(GetGrayRgn(), menuRgn, GetGrayRgn());

		PaintBehind((WindowPeek) FrontWindow(), menuRgn);
		CalcVisBehind((WindowPeek) FrontWindow(), menuRgn);

		DisposeRgn(menuRgn);

		gMenuBarHidden = TRUE;
	}
}


/*******************************************************************************

	ShowMenuBar

	Restore the old desktop region, making the world safe for menubars again.
	Call DrawMenuBar to redraw the menubar. If we previously set the menubar
	height to zero in HideMenuBar, restore it.

*******************************************************************************/
void ShowMenuBar()
{
	if (gMenuBarHidden) {
		GetMBarHeight() = gOldeMBarHeight;
		CopyRgn(gOldeGrayRgn, GetGrayRgn());
		SetToMenuRect(gOldeGrayRgn);

		PaintBehind((WindowPeek) FrontWindow(), gOldeGrayRgn);
		CalcVisBehind((WindowPeek) FrontWindow(), gOldeGrayRgn);

		DrawMenuBar();
		gMenuBarHidden = FALSE;
		SetEmptyRgn(gOldeGrayRgn);
	}
}


/*******************************************************************************

	SetToMenuRect

	Handy utility for calculating and returning the bounding rectangle of the
	menubar. Assumes that gOldeMBarHeight holds the correct height of the
	menubar. The resulting rectangle is placed in the region passed in to us.

*******************************************************************************/
void SetToMenuRect(RgnHandle rgn)
{
	Rect		menuRect;

	menuRect = qd.screenBits.bounds;
	menuRect.bottom = gOldeMBarHeight;
	RectRgn(rgn, &menuRect);
}
