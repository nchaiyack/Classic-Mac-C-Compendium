/* ----------------------------------------------------------------------

	PPPOn
	establishes a PPP session on a Mac with MacTCP and MacPPP installed.
	
	Written by: Paul Celestin
	
	950621 - work begins on the project

---------------------------------------------------------------------- */


/* ----------------------------------------------------------------------
includes
---------------------------------------------------------------------- */

#include	"pppcontrol.h"


/* ----------------------------------------------------------------------
main - here is where it all began...
---------------------------------------------------------------------- */
void main()
{
	OSErr myErr;

	InitToolBox();
	if (!pppup())
	{
		myErr = pppopen();
	}
	ExitToShell();
}


/* ----------------------------------------------------------------------
InitToolBox
---------------------------------------------------------------------- */
void InitToolBox()
{
	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}
