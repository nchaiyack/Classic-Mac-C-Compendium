/*
 * Set type of a dialog item.
 */

# include	"TransSkel.h"


pascal void
SkelSetDlogType (DialogPtr d, short item, short type)
{
short	oldType;
Handle	h;
Rect	r;

	GetDItem (d, item, &oldType, &h, &r);
	SetDItem (d, item, type, h, &r);
}
