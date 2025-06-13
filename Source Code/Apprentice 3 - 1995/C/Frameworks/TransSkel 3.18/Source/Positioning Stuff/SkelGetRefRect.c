/*
 * Get a reference rectangle for window positioning.
 *
 * Reference rect for position types:
 *	skelPositionNone -- no reference rect
 *	skelPositionOnMainDevice -- usable area on main device
 *	skelPositionOnParentWindow -- content rect of frontmost visible window
 *	skelPositionOnParentDevice -- usable area on screen of frontmost visible window
 *
 * If there's no frontmost window, positions that use it default to
 * skelPositionOnMainDevice.
 *
 * Result for position skelPositionNone is same as for skelPositionOnMainDevice
 * just so that result isn't undefined, but caller would be better off to handle
 * skelPositionNone case itself.
 *
 * 18 Feb 94
 * - Return structure rect rather than content rect of parent window when
 * position type is skelPositionOnParentWindow.
 */

# include	"TransSkel.h"


pascal void
SkelGetReferenceRect (Rect *r, short positionType)
{
WindowPtr	frontWind = FrontWindow ();
GrafPtr	tmpPort;
Rect	floatRect, refRect;

	/*
	 * Assume default positioning will be with reference to main device.
	 * This will also be used as the fallback for positionings that use
	 * FrontWindow() if FrontWindow() is nil.
	 */

	SkelGetMainDeviceRect (r);

	if (positionType == skelPositionNone)	/* leave window as is */
		return;

	/*
	 * Find frontmost visible window
	 */
	frontWind = FrontWindow ();
	while (frontWind != (WindowPtr) nil && !((WindowPeek) frontWind)->visible)
		frontWind = (WindowPtr) ((WindowPeek) frontWind)->nextWindow;

	switch (positionType)
	{
	case skelPositionOnParentWindow:
		if (frontWind != (WindowPtr) nil)
			SkelGetWindStructureRect (frontWind, r);
		break;
	case skelPositionOnParentDevice:
		if (frontWind != (WindowPtr) nil)
			(void) SkelGetWindowDevice (frontWind, (GDHandle *) nil, r);
		break;
	}
}
