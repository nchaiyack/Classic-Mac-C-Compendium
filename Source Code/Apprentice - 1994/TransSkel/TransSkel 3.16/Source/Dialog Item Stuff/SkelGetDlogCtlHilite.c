/*
 * Get hiliting value of control associated with a dialog item.
 */

# include	"TransSkel.h"


pascal short
SkelGetDlogCtlHilite (DialogPtr d, short item)
{
	return ((**SkelGetDlogCtl (d, item)).contrlHilite);
}
