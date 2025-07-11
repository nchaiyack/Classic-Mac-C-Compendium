/*
 * Calculate content or structure rectangle for a window. These are
 * core routines because they are called by other core routines,
 * e.g., SkelGetWindowDevice() and SkelGetWindTitleHeight ().
 */

# include	"TransSkel.h"

# define	kTranslate	0x4000


/*
 * Get content rectangle and convert it to global coordinates
 */

pascal void
SkelGetWindContentRect (WindowPtr w, Rect *rp)
{
GrafPtr	oldPort;
Rect	content;

	GetPort (&oldPort);
	SetPort (w);
	*rp = w->portRect;
	LocalToGlobal (&topLeft (*rp));
	LocalToGlobal (&botRight (*rp));
	SetPort (oldPort);
}


/*
 * Get structure rectangle.  This is already in global coordinates, but the
 * tricky part is that it isn't valid if the window is invisible.
 *
 * If window's visible, the structure region's valid, so get the bounding box.
 *
 * If the window's not visible, fling it out into space, make it visible, get
 * the structure region bounding box, make it invisible again and restore it to
 * its normal position.  Use ShowHide() for this since  it doesn't change the
 * window's hiliting or position in the stacking order.  The rectangle
 * calculated this way has to be moved back, too, since it's obtained when the
 * window is in flung position.
 *
 * I have seen similar code that also saves and restored the window's userState,
 * but Inside Macintosh (Toolbox Essentials, p. 4-70) explicitly states that
 * the userState isn't modified when you just move a window, so I don't see the
 * point.
 */

pascal void
SkelGetWindStructureRect (WindowPtr w, Rect *rp)
{
Rect	content;

	if (((WindowPeek) w)->visible)
		*rp = (**(* (WindowPeek) w).strucRgn).rgnBBox;
	else
	{
		SkelGetWindContentRect (w, &content);				/* get upper-left coords */
		MoveWindow (w, kTranslate, content.top, false);		/* fling window */
		ShowHide (w, true);
		*rp = (**(* (WindowPeek) w).strucRgn).rgnBBox;
		ShowHide (w, false);
		MoveWindow (w, content.left, content.top, false);	/* unfling window */
		OffsetRect (rp, content.left - kTranslate, 0);		/* unfling struct rect */
	}
}
