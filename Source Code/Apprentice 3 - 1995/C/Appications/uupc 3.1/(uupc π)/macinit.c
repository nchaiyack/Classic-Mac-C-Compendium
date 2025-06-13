/*			macinit.c

			Portions Copyright © David Platt, 1992, 1991.  All Rights Reserved
			Worldwide.

*/

#ifdef THINK_C
# include "unixlibproto.h"
#endif THINK_C

#include "dcp.h"

#include	<stdio.h>

#include "macinit.proto.h"

macinit(void)
{
	WindowPeek theWindow;
	int consoleDriverRefNum;
	DCtlHandle consoleDriverHandle;
	/* standard initialization required by real Mac applications */
	InitGraf(&thePort);
    InitWindows();
    InitFonts();
    FlushEvents(everyEvent,0);
    InitMenus();				/* init menu manager */
    InitDialogs((ProcPtr) 0L);	/* init dialog manager */
    TEInit();					/* init text edit */
    InitCursor();
    SetApplLimit(GetApplLimit() - MAXPACK * 4); /* room enough for big protocol buffers */
    MaxApplZone();
    OpenResFile("\pUUPC sounds");
/*
	That which follows, makes my skin crawl and my gorge rise.  I
	hate doing things like this.  Ne'ertheless, it seems to be
	necessary to keep the THINK C console driver from eating all
	keyclicks which take place when its window is foremost.  The
	behavior of the driver is appropriate for I/O consoles, but
	is inappropriate for output-only consoles such as the one
	that UUPC uses.
*/
	printf("Welcome to uupc!\n\n");
    theWindow = stdout->window;
    consoleDriverRefNum = theWindow->windowKind;
    consoleDriverHandle = GetDCtlEntry(consoleDriverRefNum);
    if (consoleDriverHandle) {
    	(**consoleDriverHandle).dCtlEMask &= 0xFFF7; /* turn off key-down event grab */
    }
/*
	Forgive me, for I have sinned.
			dplatt@snulbug.mtview.ca.us
*/
}

