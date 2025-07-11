/*
 * Set text of a dialog item.  The value is passed or returned
 * as a string, not a buffer/length combination.
 */

# include	"TransSkel.h"


pascal void
SkelSetDlogStr (DialogPtr d, short item, StringPtr str)
{
short	type;
Handle	h;
Rect	r;

	GetDItem (d, item, &type, &h, &r);
	SetIText (h, str);
}
