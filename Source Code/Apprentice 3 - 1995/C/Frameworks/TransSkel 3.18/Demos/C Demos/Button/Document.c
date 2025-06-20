/*
 * The stuff in this file presents a document window with an outlined
 * pushbutton.  It demonstrates:
 * - Button outlining in a document window.
 *
 * Note that button clicks are tracked and return/enter and escape/
 * command-period keyclicks are mapped onto button clicks, but that no
 * other action is associated with those clicks.  This window hander
 * just shows the visible user interface stuff associated with those
 * actions.
 */

# include	"TransSkel.h"

# include	"Button.h"


# define	returnKey	13
# define	enterKey	3
# define	escapeKey	27


static WindowPtr		wind;
static ControlHandle	okBtn;
static ControlHandle	cancelBtn;


static pascal void
Mouse (Point pt, long t, short mods)
{
ControlHandle	ctrl;
short	partNo;

	if ((partNo = FindControl (pt, wind, &ctrl)) != 0)
	{
		if (partNo == inButton)
		{
			if (TrackControl (ctrl, pt, nil))
			{
				/* nothing done here */
			}
		}
	}
}


/*
 * Key handler.  Map return/enter onto clicks in OK button.
 * Note that we check whether the OK button is active or not
 * before flashing the button.  In this application the button
 * is never inactive when the window is active, but that may not
 * be generally true.
 */

static pascal void
Key (short c, short code, short mods)
{
	if (c == returnKey || c == enterKey)
	{
		if ((**okBtn).contrlHilite == normalHilite)
			SkelFlashButton (okBtn);
	}
	else if (c == escapeKey || SkelCmdPeriod (SkelGetCurrentEvent ()))
	{
		if ((**cancelBtn).contrlHilite == normalHilite)
			SkelFlashButton (cancelBtn);
	}
}


/*
 * Update the window.
 */

static pascal void
Update (Boolean resized)
{
WindowPtr	wind;
Rect	r;
short	h;

	GetPort (&wind);

	r = wind->portRect;
	EraseRect (&r);
	DrawControls (wind);
	SkelDrawButtonOutline (okBtn);
}


/*
 * Make the buttons active or inactive as the window becomes active or
 * inactive.  Redraw default button outline to follow state of default
 * button.
 */

static pascal void
Activate (Boolean active)
{
short	hilite;

	hilite = (active ? normalHilite : dimHilite);
	HiliteControl (okBtn, hilite);
	SkelDrawButtonOutline (okBtn);
	HiliteControl (cancelBtn, hilite);
}


static pascal void
Clobber (void)
{
WindowPtr	wind;

	GetPort (&wind);
	HideWindow (wind);
	DisposeWindow (wind);
}


/*
 * Initialize document window
 */

void
SetupDocument (void)
{
Rect	r;

	if (SkelQuery (skelQHasColorQD))
		wind = GetNewCWindow (docWindRes, nil, (WindowPtr) -1L);
	else
		wind = GetNewWindow (docWindRes, nil, (WindowPtr) -1L);
	if (wind == (WindowPtr) nil)
	{
		SysBeep (1);
		return;
	}
	SkelWindow (wind,
				Mouse,
				Key,
				Update,
				Activate,
				nil,			/* no close box, so no close handler */
				Clobber,
				nil,			/* no idle handler */
				true);			/* irrelevant since no idle handler */

	SetRect (&r, 10, 20, 80, 40);
	cancelBtn = NewControl (wind, &r, "\pCancel", true, 0, 0, 1, pushButProc, 0L);
	OffsetRect (&r, 80, 0);
	okBtn = NewControl (wind, &r, "\pOK", true, 0, 0, 1, pushButProc, 0L);

	ShowWindow (wind);
	SkelDoEvents (activMask + updateMask);
}
