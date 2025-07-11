/*
 * SkelAlert ()
 *
 * Present an alert, positioned according to the given positioning type
 * using the current positioning ratios.  The trick is to grab the alert
 * template, make sure it stays in memory, and position its window
 * bounding rectangle before the Alert() call is made.
 *
 * SkelAlert() returns whatever value the alert would normally return,
 * except that an error value is returned if the alert cannot be presented.
 */

# include	"TransSkel.h"


/* dialog box had 8 pixel border */

# define	dlogBoxBorder	8

/*
 * The initial (default) horizontal and vertical positioning ratios
 * are those given by Apple's (current) Human Interface Guidelines.
 */

# define	defHorizRatio	FixRatio (1, 2)
# define	defVertRatio	FixRatio (1, 5)

static Fixed	horizRatio = -1;
static Fixed	vertRatio = -1;


/*
 * Get current positioning ratios.  Make sure to initialize them if
 * they haven't been set yet.  This is necessary because horizRatio
 * and vertRatio can't be initialized to the non-constant values
 * defHorizRatio and defVertRatio.
 *
 * Either parameter can be nil if the caller is not interested in
 * the corresponding value.
 */

pascal void
SkelGetAlertPosRatios (Fixed *hRatio, Fixed *vRatio)
{
	if (hRatio != (Fixed *) nil)
	{
		if (horizRatio == (Fixed) -1)
			horizRatio = defHorizRatio;
		*hRatio = horizRatio;
	}
	if (vRatio != (Fixed *) nil)
	{
		if (vertRatio == (Fixed) -1)
			vertRatio = defVertRatio;
		*vRatio = vertRatio;
	}
}


/*
 * Set current positioning ratios
 */

pascal void
SkelSetAlertPosRatios (Fixed hRatio, Fixed vRatio)
{
	horizRatio = hRatio;
	vertRatio = vRatio;
}


/*
 * Position and present an alert.
 *
 * The alert bounding box is saved before postioning it and restored
 * afterward, in case the resource is used outside of this function.
 */

pascal short
SkelAlert (short alrtResNum, ModalFilterUPP filter, short positionType)
{
GrafPtr		oldPort;
AlertTHndl	h;
Rect		origRect;
Rect		refRect;
Rect		*rp;
SignedByte	state;
short		result;
Fixed		hRatio, vRatio;

	GetPort (&oldPort);
	h = (AlertTHndl) GetResource ('ALRT', alrtResNum);
	if (h == (AlertTHndl) nil)
	{
		SysBeep (1);
		return (ResError ());
	}
	state = HGetState ((Handle) h);
	HNoPurge ((Handle) h);				/* make sure it's around */
	LoadResource ((Handle) h);			/* and stays around */
	MoveHHi ((Handle) h);
	HLock ((Handle) h);
	origRect = (**h).boundsRect;		/* save original position */
	if (positionType != skelPositionNone)
	{
		/*
		 * Inset bounding rectangle so position is effectively done using
		 * the structure rectangle rather than the content rectangle.
		 */
		InsetRect (&(**h).boundsRect, -dlogBoxBorder, -dlogBoxBorder);

		SkelGetReferenceRect (&refRect, positionType);
		SkelGetAlertPosRatios (&hRatio, &vRatio);
		SkelPositionRect (&refRect, &(**h).boundsRect, hRatio, vRatio);
		/*
		 * If alert is to be positioned on parent window, alert might be
		 * partly invisible if parent is moved partly off the desktop.  If
		 * so, reposition alert on parent device.
		 */
		if (positionType == skelPositionOnParentWindow
				&& !SkelTestRectVisible (&(**h).boundsRect))
		{
			SkelGetReferenceRect (&refRect, skelPositionOnParentDevice);
			SkelPositionRect (&refRect, &(**h).boundsRect, hRatio, vRatio);
		}

		InsetRect (&(**h).boundsRect, dlogBoxBorder, dlogBoxBorder);
	}
	InitCursor ();			/* in case it comes up while cursor is different */
	result = Alert (alrtResNum, filter);
	(**h).boundsRect = origRect;		/* restore original position */
	HUnlock ((Handle) h);
	HSetState ((Handle) h, state);		/* get rid of resource */
	ReleaseResource ((Handle) h);
	SetPort (oldPort);
	return (result);
}
