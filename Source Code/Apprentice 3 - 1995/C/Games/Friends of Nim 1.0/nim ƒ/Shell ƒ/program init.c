#include "program init.h"
#include "window layer.h"
#if USE_MUSIC
#include "music layer.h"
#endif
#include "cursor layer.h"
#include "nim.h"

void InitTheProgram(void)
{
#if USE_MUSIC
	if (gMusicToggle)
		StartTheMusic();
#endif

	SetupTheAnimatedCursor(1003, 6);
	InitGame();
}

Boolean ShutDownTheProgram(void)
{
	WindowRef		theWindow;
	
	while ((theWindow=GetFrontDocumentWindow())!=0L)
		if (!CloseTheWindow(theWindow))
			return FALSE;
	
	while ((theWindow=FrontWindow())!=0L)
		if (!CloseTheWindow(theWindow))
			return FALSE;
	
	/* ShutdownTheAnimatedCursor(); */
	
	return TRUE;
}
