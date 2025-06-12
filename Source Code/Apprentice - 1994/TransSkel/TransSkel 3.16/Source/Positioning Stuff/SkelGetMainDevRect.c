/*
 * Get usable area on main device.  Does not include menu bar; if that's
 * of interest, do this:
 *
 *		SkelGetMainDeviceRect (&r);
 *		r.top -= SkelQuery (skelQMBarHeight);
 */

# include	"TransSkel.h"


pascal void
SkelGetMainDeviceRect (Rect *r)
{
	if (!SkelQuery (skelQHasColorQD))	/* no devices, use screenBits */
		*r = screenBits.bounds;
	else
		*r = (**GetMainDevice ()).gdRect;
	r->top += SkelQuery (skelQMBarHeight);
}
