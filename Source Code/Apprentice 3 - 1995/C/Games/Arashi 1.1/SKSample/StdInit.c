/*/
     Project Arashi/STORM: StdInit.c
     Major release: 9/9/92

     Last modification: Wednesday, September 9, 1992, 21:46
     Created: Thursday, April 23, 1987, 17:38

     Copyright © 1987-1992, Juri Munkki
/*/

#define	STACKSPACE	32768L

void	DoInits()
{
	InitGraf(&thePort);
	InitCursor();
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
	SetApplLimit(CurStackBase-STACKSPACE);	/*	Allocate some memory for the stack.	*/
	MaxApplZone();
}
