/*
 * Flash a push button to simulate a click in it.
 */

# include	"TransSkel.h"


# define	hiliteClicks	8

pascal void
SkelFlashButton (ControlHandle ctrl)
{
short	oldHilite;
long	dummy;

	oldHilite = (**ctrl).contrlHilite;
	HiliteControl (ctrl, inButton);		/* flash it */
	Delay ((long) hiliteClicks, &dummy);
	HiliteControl (ctrl, oldHilite);
}
