/*
 * SkelPositionRect()
 *
 * Position a rectangle relative to reference rectangle so that space above/below
 * and to left/right of rectangle that's moved maintains ratio given by hRatio and
 * vRatio.  This is useful for establishing initial window positions or positioning
 * alerts/dialogs.
 *
 * "inside" isn't quite the right word, since the moved rectangle need not
 * actually be smaller than the reference rectangle.
 *
 * Examples:
 *
 * Center a rectangle inside the reference rectangle:
 *	SkelPositionRect (&ref, &r, FixRatio (1, 2), FixRatio (1, 2));
 *
 * Leave 1/3 of vertical space above positioned rectangle, 2/3 of space below:
 *	SkelPositionRect (&ref, &r, FixRatio (1, 2), FixRatio (1, 3));
 *
 * Algorithm may not work correctly if rects have negative coordinates.
 */

# include	<FixMath.h>

# include	"TransSkel.h"


pascal void
SkelPositionRect (Rect *refRect, Rect *r, Fixed hRatio, Fixed vRatio)
{
short	hOff, vOff;

	/* align topleft of rects (simplifies calculations) */

	OffsetRect (r, refRect->left - r->left, refRect->top - r->top);

	/* calculate offsets in each direction for given ratios */

	hOff = Fix2Long (FixMul (Long2Fix ((long) (refRect->right - r->right)), hRatio));
	vOff = Fix2Long (FixMul (Long2Fix ((long) (refRect->bottom - r->bottom)), vRatio));

	/* move rect by appropriate amount */

	OffsetRect (r, hOff, vOff);
}
