/*
 * Given a set of contiguous radio button dialog items, turn one of them
 * on and all others off.
 */

# include	"TransSkel.h"


pascal void
SkelSetDlogRadioButtonSet (DialogPtr dlog, short first, short last, short choice)
{
ControlHandle	ctrl;
short			i, value;

	for (i = first; i <= last; i++)
	{
		value = (i == choice ? 1 : 0);
		ctrl = SkelGetDlogCtl (dlog, i);
		if (GetCtlValue (ctrl) != value)	/* don't set unless it's different! */
			SetCtlValue (ctrl, value);
	}
}
