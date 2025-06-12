# include   "TransSkel.h"


static pascal void
DoFileMenu (short item)
{
    SkelStopEventLoop ();            /* tell SkelEventLoop() to quit */
}


int
main (void)
{
MenuHandle	m;

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
	SkelEventLoop ();			/* loop 'til Quit selected */
	SkelCleanup ();				/* clean up */
}
