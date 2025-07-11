/*/
     Project Arashi: StdInit.c
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, September 9, 1992, 21:39
     Created: Thursday, April 23, 1987, 17:38

     Copyright � 1987-1992, Juri Munkki
/*/

#define	STACKSPACE	32768L

void	DoInits()
{
	SetApplLimit(GetApplLimit() - (STACKSPACE - DefltStack));

	InitGraf(&thePort);
	InitCursor();
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	MaxApplZone();
}
