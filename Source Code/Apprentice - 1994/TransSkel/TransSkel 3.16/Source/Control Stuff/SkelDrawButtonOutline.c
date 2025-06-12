/*
 * Draw a heavy outline around a push button.  Draw the outline in black
 * unless the button is inactive, in which case draw it gray.  When drawing
 * gray outlines, use a true RGB gray if the monitor supports it and the button
 * belongs to a color GrafPort, and pattern gray otherwise.  This matches how
 * the Control Manager draws titles in dimmed buttons.
 *
 * It's absolutely astonishing how much work is necessary to draw a gray
 * outline. This is evidenced not only by all the code below, but also by
 * the efforted expended in SkelGetRectDevice() to make sure the gray is
 * computed using the characteristics of the proper device.  It's a wonder
 * it doesn't take 10 minutes to draw dimmed outlines.
 */

# include	<Palettes.h>		/* for GetGray() */

# include	"TransSkel.h"


# define	normalHilite	0
# define	dimHilite		255


pascal void
SkelDrawButtonOutline (ControlHandle ctrl)
{
GrafPtr		oldPort;
PenState	penState;
Rect		r, r2;
short		curvature;
Boolean		haveRGBGray;
RGBColor	fgColor, newFgColor, bgColor;
GDHandle	gDev;

	GetPort (&oldPort);
	SetPort ((**ctrl).contrlOwner);
	GetPenState (&penState);
	PenNormal ();
	r = (**ctrl).contrlRect;
	InsetRect (&r, -4, -4);
	curvature = (r.bottom - r.top) / 2 + 2;
	PenSize (3, 3);
	if ((**ctrl).contrlHilite == normalHilite)	/* button active, draw black */
		FrameRoundRect (&r, curvature, curvature);
	else										/* button inactive, draw gray */
	{
		/*
		 * Try to get RGB gray value appropriate for the device on which
		 * the button is displayed if have color GrafPort.
		 */
		haveRGBGray = false;
		if (((CGrafPtr) (**ctrl).contrlOwner)->portVersion & 0xc000)
		{
			/* convert rect to global coordinates */
			r2 = r;
			LocalToGlobal (&topLeft (r2));
			LocalToGlobal (&botRight (r2));
			(void) SkelGetRectDevice (&r2, &gDev, (Rect *) nil, (Boolean *) nil);
			/* test unnecessary unless for some reason rect isn't on any device! */
			if (gDev != (GDHandle) nil)
			{
				GetBackColor (&bgColor);
				GetForeColor (&fgColor);
				newFgColor = fgColor;
				haveRGBGray = GetGray (gDev, &bgColor, &newFgColor);
			}
		}
		/*
		 * Draw using colored gray if possible, else using pattern gray
		 */

		if (haveRGBGray)
			RGBForeColor (&newFgColor);
		else
			PenPat (gray);
		FrameRoundRect (&r, curvature, curvature);
		if (haveRGBGray)
			RGBForeColor (&fgColor);
	}
	SetPenState (&penState);
	SetPort (oldPort);
}
