/*
 * Position a window according to the given position type, using the
 * given positioning ratios.
 *
 * Position types:
 *	skelPositionNone -- leave window in current position
 *	skelPositionOnMainDevice -- position on main device
 *	skelPositionOnParentWindow -- position on frontmost visible window
 *	skelPositionOnParentScreen -- position on screen of frontmost visible window
 *
 * If there's no frontmost window, positions that use it default to
 * skelPositionOnMainDevice.
 *
 * For best results, window should not be visible.  Otherwise you'll end
 * up moving it while it's visible.
 *
 * 08 Feb 94
 * - Position window using structure rather than content rectangle.
 */

# include	"TransSkel.h"


pascal void
SkelPositionWindow (WindowPtr w, short positionType,
							Fixed hRatio, Fixed vRatio)
{
Rect	contentRect, structRect, refRect;
short	hDiff, vDiff;

	if (positionType == skelPositionNone)	/* leave window as is */
		return;

	/* get rect to use as reference against which to position window rect */
	SkelGetReferenceRect (&refRect, positionType);

	/*
	 * Use structure rect as the rect to be positioned, but when moving window,
	 * offset by difference between upper left of structure and content rects,
	 * since MoveWindow() positions the content rect to the given position.
	 */
	SkelGetWindContentRect (w, &contentRect);
	SkelGetWindStructureRect (w, &structRect);
	hDiff = contentRect.left - structRect.left;
	vDiff = contentRect.top - structRect.top;
	SkelPositionRect (&refRect, &structRect, hRatio, vRatio);
	MoveWindow (w,
				structRect.left + hDiff,
				structRect.top + vDiff,
				false);
}
