/*
 * Set value of control associated with a dialog item.
 */

# include	"TransSkel.h"


pascal void
SkelSetDlogCtlValue (DialogPtr d, short item, short value)
{
	SetCtlValue (SkelGetDlogCtl (d, item), value);
}
