/*
 * Set hiliting value of control.  Differs from HiliteControl() in that
 * the control is not set it if it already has that value, in order to minimize
 * drawing.  Returns true if the value was actually changed, false if not.
 *
 * Returning the value can be useful in conjunction with push buttons surrounded
 * by a heavy outline.  When the button is dimmed, the outline should go dim.
 * When the button is made active, the outline should be black.  By knowing
 * whether the hiliting value actually changed, outline redrawing can be skipped
 * when it's unnecessary.
 */

# include	"TransSkel.h"


pascal Boolean
SkelHiliteControl (ControlHandle ctrl, short hilite)
{
	if ((**ctrl).contrlHilite != hilite)
	{
		HiliteControl (ctrl, hilite);
		return (true);
	}
	return (false);
}
