/*
 * Toggle value of control, return resulting value.
 */

# include	"TransSkel.h"


pascal short
SkelToggleCtlValue (ControlHandle ctrl)
{
short	value;

	value = (GetCtlValue (ctrl) ? 0 : 1);
	SetCtlValue (ctrl, value);
	return (value);
}
