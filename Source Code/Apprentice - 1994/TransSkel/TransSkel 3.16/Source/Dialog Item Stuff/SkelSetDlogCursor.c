/*
 * Set cursor to I-beam if it's in an edit text item of the given dialog.
 *
 * Does nothing if the FindDItem() trap doesn't exist (which it doesn't in
 * versions of system software prior to 3.2).
 */

# include	<Traps.h>

# include	"TransSkel.h"



pascal void
SkelSetDlogCursor (DialogPtr d)
{
GrafPtr	savePort;
Point	pt;
short	i;

	if (SkelTrapAvailable (_FindDItem))
	{
		/*
		 * Get cursor local coordinates.  One assumes the port will be set
		 * to the dialog, but you never know...
		 */
		GetPort (&savePort);
		SetPort (d);
		GetMouse (&pt);
		SetPort (savePort);
		i = FindDItem (d, pt) + 1;
		if (i > 0 && (SkelGetDlogType (d, i) & editText) == 0)
				i = 0;
		if (i > 0)
			SetCursor (*GetCursor (iBeamCursor));
		else
			InitCursor ();
	}
}
