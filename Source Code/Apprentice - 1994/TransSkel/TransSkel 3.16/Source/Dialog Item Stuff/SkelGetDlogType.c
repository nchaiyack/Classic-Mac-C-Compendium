/*
 * Get type of dialog item.
 */


# include	"TransSkel.h"


pascal short
SkelGetDlogType (DialogPtr d, short item)
{
short	type;
Handle	h;
Rect	r;

	GetDItem (d, item, &type, &h, &r);
	return (type);
}
