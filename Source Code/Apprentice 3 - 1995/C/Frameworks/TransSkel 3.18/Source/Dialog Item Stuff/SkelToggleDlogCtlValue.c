/*
 * Toggle value of control associated with a dialog item,
 * return resulting value.
 */

# include	"TransSkel.h"


pascal short
SkelToggleDlogCtlValue (DialogPtr d, short item)
{
	return (SkelToggleCtlValue (SkelGetDlogCtl (d, item)));
}
