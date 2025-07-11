/*
	Main.c
	
	Entry point for the application.
	
*/

#include "Main.h"
#include "SpinLib.h"

/*
	InitToolbox
	
	Initializes the Mac's toolbox.
*/
void InitToolbox(){
	
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	FlushEvents(everyEvent,0);
	TEInit();
	InitDialogs(0L);
	InitCursor();
}

/*
	main
	
	Main entry point for the application.
*/
void main(){
	short i;
	long lt;
	
	InitToolbox();
	
	if (SpinInit())
		SysBeep(10);
	
	if (SpinStart(1))
		SysBeep(10);

	FlushEvents(everyEvent,0);
	while(!Button())
		/* Do Nothing */;

	if (SpinStop())
		for (i=0;i<3;i++)
		SysBeep(10);
	
	Delay(180,&lt);
	
	if (SpinStart(-1)){
		SysBeep(10);
		SysBeep(10);
	}

	FlushEvents(everyEvent,0);
	while(!Button())
		/* Do Nothing */;

	if (SpinStop())
		SysBeep(10);
	
	if (SpinCleanup())
		SysBeep(10);
}
