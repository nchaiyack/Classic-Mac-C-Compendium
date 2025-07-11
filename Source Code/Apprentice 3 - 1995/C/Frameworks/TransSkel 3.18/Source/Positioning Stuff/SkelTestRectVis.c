/*
 * SkelTestRectVisible ()
 *
 * Test whether or not the given rectangle is completely visible on the
 * desktop.  Done by converting the rectangle to a region, intersecting
 * it with the desktop region, and checking whether or not the intersection
 * is the same as the rectangle's region.
 *
 * Result is false if the regions needed for the calculation can't be
 * allocated.
 */

# include	"TransSkel.h"


pascal Boolean
SkelTestRectVisible (Rect *r)
{
Boolean		result = false;
RgnHandle	rectRgn;
RgnHandle	rgn;

	rectRgn = NewRgn ();
	if (rectRgn != (RgnHandle) nil)
	{
		rgn = (RgnHandle) SkelQuery (skelQGrayRgn);
		if (rgn != (RgnHandle) nil)
		{
			RectRgn (rectRgn, r);				/* convert rect to region */
			SectRgn (rectRgn, rgn, rgn);		/* intersect with desktop */
			result = EqualRgn (rectRgn, rgn);	/* true if completely within */
			DisposeRgn (rgn);
		}
		DisposeRgn (rectRgn);
	}
	return (result);
}
