/*
 * Set reference constant of control associated with a dialog item.
 */

# include	"TransSkel.h"


pascal void
SkelSetDlogCtlRefCon (DialogPtr d, short item, long value)
{
	SetCRefCon (SkelGetDlogCtl (d, item), value);
}
