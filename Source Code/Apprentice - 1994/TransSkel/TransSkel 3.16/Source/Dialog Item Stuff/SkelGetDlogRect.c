/*
 * Get bounding rectangle of a dialog item.
 */

# include	"TransSkel.h"


pascal void
SkelGetDlogRect (DialogPtr d, short item, Rect *r)
{
short	type;
Handle	h;

	GetDItem (d, item, &type, &h, r);
}
