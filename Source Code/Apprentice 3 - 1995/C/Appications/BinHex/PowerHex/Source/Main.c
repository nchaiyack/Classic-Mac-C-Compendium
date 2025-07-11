#include "Event.h"
#include "Menu.h"
#include "Status.h"
#include "Main.h"





int main(void)
{
	short	timeToQuit;
	
	
	InitApplication();
	
	do
	{
		timeToQuit = doEventLoop(60L);
	}while(!timeToQuit);

	return noErr;
}





void InitApplication(void)
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	InitEvents();
	InitAppMenus();
	InitStatusWindow();
}