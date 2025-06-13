# include   "TransSkel.h"


static char lastChar = '\0';
static Point lastLocation = { 20, 20 };


static pascal void
DoFileMenu (short item)
{
    SkelStopEventLoop ();            /* tell SkelEventLoop() to quit */
}


static pascal void
Mouse (Point where, long when, short modifiers)
{
WindowPtr	w;

	lastLocation = where;
	GetPort (&w);
	InvalRect (&w->portRect);
}


static pascal void
Key (short c, short code, short modifiers)
{
WindowPtr	w;

	lastChar = c;
	GetPort (&w);
	InvalRect (&w->portRect);
}


static void
DrawGrowBox (WindowPtr w)
{
RgnHandle	oldClip;
Rect		r;

	r = w->portRect;
	r.left = r.right - 15;		/* draw only in corner */
	r.top = r.bottom - 15;
	oldClip = NewRgn ();
	GetClip (oldClip);
	ClipRect (&r);
	DrawGrowIcon (w);
	SetClip (oldClip);
	DisposeRgn (oldClip);
}


static pascal void
Update (Boolean resized)
{
WindowPtr	w;

	GetPort (&w);
	EraseRect (&w->portRect);
	if (lastChar != '\0')
	{
		MoveTo (lastLocation.h, lastLocation.v);
		DrawChar (lastChar);
	}
	DrawGrowBox (w);
}


static pascal void
Activate (Boolean active)
{
WindowPtr	w;

	GetPort (&w);
	DrawGrowBox (w);
}


static pascal void
Close (void)
{
	SkelStopEventLoop ();
}


static pascal void
Clobber (void)
{
WindowPtr	w;

	GetPort (&w);
	DisposeWindow (w);
}


static pascal void
Idle (void)
{
WindowPtr	w;

	GetPort (&w);
}


int
main (void)
{
MenuHandle	m;
WindowPtr	w;
Rect		r;

	SkelInit (nil);				/* initialize */
	SkelApple (nil, nil);		/* handle Apple menu */
	/* create File menu, install handler */
	m = NewMenu (skelAppleMenuID + 1, "\pFile");
	AppendMenu (m, "\pQuit/Q");
	(void) SkelMenu (m,					/* menu handle */
						DoFileMenu,		/* item selection function */
						nil,			/* menu disposal function */
						false,			/* not a submenu */
						true);			/* draw menu bar */

	SetRect (&r, 40, 40, 200, 120);
	w = NewWindow (nil, &r, "\pA Window", true,
						documentProc+8, (WindowPtr) -1, true, 0L);
	(void) SkelWindow (w,
						Mouse,		/* mouse click handler */
						Key,		/* key click handler */
						Update,		/* update event handler */
						Activate,	/* activate event handler */
						Close,		/* close box click handler */
						Clobber,	/* disposal function */
						Idle,		/* idle-time handler */
						true);		/* idle only when frontmost */
	SkelEventLoop ();
	SkelCleanup ();
}
