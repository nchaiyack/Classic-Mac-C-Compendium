/*
 * Get reference constant of control associated with a dialog item.
 */

# include	"TransSkel.h"


pascal long
SkelGetDlogCtlRefCon (DialogPtr d, short item)
{
	return (GetCRefCon (SkelGetDlogCtl (d, item)));
}
