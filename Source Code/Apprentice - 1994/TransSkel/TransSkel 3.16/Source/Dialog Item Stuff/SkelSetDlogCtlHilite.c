/*
 * Set hiliting value of control associated with a dialog item.
 * Doesn't set it if it already has that value, in order to minimize
 * drawing.  Returns true if the value was actually changed, false if not.
 */

# include	"TransSkel.h"


pascal Boolean
SkelSetDlogCtlHilite (DialogPtr d, short item, short hilite)
{
	return (SkelHiliteControl (SkelGetDlogCtl (d, item), hilite));
}
