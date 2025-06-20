/* Cell Proj 1.0 */

/********************************************************************
 Ported to Metrowerks CodeWarrior by Paul Celestin on 4 November 1994
 ********************************************************************/

#include "Cell_Proto.h"
#include "Cell_Definitions.h"
#include "Cell_Variables.h"

#if __profile__
#include <Profiler.h>
#endif

Boolean			gDone;
MenuHandle		gAppleMenu;
WindowPtr		gCellWindow;
char			gCellStatus[ NUMBER_OF_CELLS ];
long			startTime,generations = 0;

void main(void)
{
	gDone = FALSE;
	ToolBoxInit();
	WindowInit();
	MenuBarInit();
	DisplayCellWindow();
	PlaceRandomCells();
	startTime = TickCount();
#if __profile__
	if (!ProfilerInit(collectDetailed, bestTimeBase, 20, 5)) {
#endif
	MainLoop();
#if __profile__
		ProfilerDump("\pLifeSim.PPC.prof");
		ProfilerTerm();
	}
#endif
}

void MainLoop(void)
{
	while ( gDone == FALSE ) {
		EventRecord	theEvent;

		WaitNextEvent(everyEvent,&theEvent,MIN_SLEEP,nil);
		HandleEvent(&theEvent);
	}
}