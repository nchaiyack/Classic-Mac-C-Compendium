/*
 * Get procedure associated with dialog user item.
 */


# include	"TransSkel.h"


pascal UserItemUPP
SkelGetDlogProc (DialogPtr d, short item)
{
short	type;
Handle	h;
Rect	r;

	GetDItem (d, item, &type, &h, &r);
	return ((UserItemUPP) h);
}
