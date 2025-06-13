# include   "TransSkel.h"


static pascal void
DoFileMenu (short item)
{
    SkelStopEventLoop ();            /* tell SkelEventLoop() to quit */
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
						nil,		/* mouse click handler */
						nil,		/* key click handler */
						nil,		/* update event handler */
						nil,		/* activate event handler */
						Close,		/* close box click handler */
						Clobber,	/* disposal function */
						nil,		/* idle-time handler */
						true);		/* idle only when frontmost */
	SkelEventLoop ();
	SkelCleanup ();
}
