/*
 * Get value of control associated with a dialog item.
 */

# include	"TransSkel.h"


pascal short
SkelGetDlogCtlValue (DialogPtr d, short item)
{
	return (GetCtlValue (SkelGetDlogCtl (d, item)));
}
