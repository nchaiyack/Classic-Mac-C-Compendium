/*
 * Set bounding rectangle of a dialog item.
 */

# include	"TransSkel.h"


pascal void
SkelSetDlogRect (DialogPtr d, short item, Rect *r)
{
short	type;
Handle	h;
Rect	rOld;

	GetDItem (d, item, &type, &h, &rOld);
	SetDItem (d, item, type, h, r);
}
