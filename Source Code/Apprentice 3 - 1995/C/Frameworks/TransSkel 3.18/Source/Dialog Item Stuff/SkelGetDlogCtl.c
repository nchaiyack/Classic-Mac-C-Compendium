/*
 * Return a handle to the control associated with a dialog item.
 */

# include	"TransSkel.h"


pascal ControlHandle
SkelGetDlogCtl (DialogPtr d, short item)
{
short	type;
Handle	h;
Rect	r;

	GetDItem (d, item, &type, &h, &r);
	return ((ControlHandle) h);
}
