/*
 * Get text of a dialog item.  The value is passed or returned
 * as a string, not a buffer/length combination.
 */

# include	"TransSkel.h"


pascal void
SkelGetDlogStr (DialogPtr d, short item, StringPtr str)
{
Handle	h;
Rect	r;
short	type;

	GetDItem (d, item, &type, &h, &r);
	GetIText (h, str);
}
