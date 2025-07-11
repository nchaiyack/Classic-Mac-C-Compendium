#include "program init.h"
#include "graphics.h"
#include "window layer.h"

void InitTheProgram(void)
{
}

Boolean ShutDownTheProgram(void)
{
	WindowPtr		theWindow;
	
	while ((theWindow=GetFrontDocumentWindow())!=0L)
		if (!CloseTheWindow(theWindow))
			return FALSE;
	
	while ((theWindow=FrontWindow())!=0L)
		if (!CloseTheWindow(theWindow))
			return FALSE;
	
	return TRUE;
}
