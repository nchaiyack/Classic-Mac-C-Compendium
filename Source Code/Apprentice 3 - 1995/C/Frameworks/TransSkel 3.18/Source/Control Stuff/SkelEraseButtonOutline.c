/*
 * Erase the heavy outline around a push button.  This simply draws
 * the outline in white.  Apparently this works correctly even when
 * the background is a different color.  Maybe "white pen" really
 * means "background color pen"?
 */

# include	"TransSkel.h"


# define	normalHilite	0
# define	dimHilite		255


pascal void
SkelEraseButtonOutline (ControlHandle ctrl)
{
GrafPtr		oldPort;
PenState	penState;
Rect		r, r2;
short		curvature;

	GetPort (&oldPort);
	SetPort ((**ctrl).contrlOwner);
	r = (**ctrl).contrlRect;
	InsetRect (&r, -4, -4);
	curvature = (r.bottom - r.top) / 2 + 2;
	GetPenState (&penState);
	PenNormal ();
	PenPat ((ConstPatternParam) &SkelQD (white));
	PenSize (3, 3);
	FrameRoundRect (&r, curvature, curvature);
	SetPenState (&penState);
	SetPort (oldPort);
}
