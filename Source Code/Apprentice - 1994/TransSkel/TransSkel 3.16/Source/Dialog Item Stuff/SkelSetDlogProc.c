/*
 * Set procedure associated with dialog user item.
 */


# include	"TransSkel.h"


pascal void
SkelSetDlogProc (DialogPtr d, short item, SkelDlogItemProcPtr p)
{
short	type;
Handle	h;
Rect	r;

	GetDItem (d, item, &type, &h, &r);
	SetDItem (d, item, type, (Handle) p, &r);
}
