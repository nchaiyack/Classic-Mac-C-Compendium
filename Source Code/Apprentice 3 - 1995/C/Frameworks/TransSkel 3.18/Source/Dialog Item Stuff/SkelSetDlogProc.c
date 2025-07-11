/*
 * Set procedure associated with dialog user item.
 *
 * If you are passing compiling PowerPC code, proc should be a pointer to
 * a routine descriptor.
 */


# include	"TransSkel.h"


pascal void
SkelSetDlogProc (DialogPtr d, short item, UserItemUPP proc)
{
short	type;
Handle	h;
Rect	r;

	GetDItem (d, item, &type, &h, &r);
	SetDItem (d, item, type, (Handle) proc, &r);
}
