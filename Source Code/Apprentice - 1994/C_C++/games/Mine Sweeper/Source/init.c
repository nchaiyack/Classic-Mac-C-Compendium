/*	init.c
 *
 *		How to initialize the basic Macintosh application
 */

#include <stdio.h>
#include "event.h"

/****************************************************************/
/*																*/
/*						Forward Declarations					*/
/*																*/
/****************************************************************/

void	InitMacOS(void);				/* Initialize Macintosh */
void	InitMenuBar(void);				/* Init Menu Bar		*/

/****************************************************************/
/*																*/
/*						Primary Entry Point						*/
/*																*/
/****************************************************************/

/*	InitApp
 *
 *		The primary entry point for initializing the entire
 *	application program
 */

void InitApp()
{
	InitMacOS();						/* Init Macintosh		*/
	InitMenuBar();						/* Get My Menu Bar		*/
	LoadPreferences();					/* Load prefs file		*/
}

/*	EndApp
 *
 *		Entry point for saving stuff
 */

void EndApp()
{
	SavePreferences();					/* Save preferences		*/
}

/****************************************************************/
/*																*/
/*					Local Initialization Code					*/
/*																*/
/****************************************************************/

/*	InitMacOS
 *
 *		Initialize the various Macintosh Operating System
 *	Managers
 */

static void InitMacOS()
{
	int i;
	EventRecord e;
	long l;
	
	InitGraf(&thePort);					/* Initialize quickdraw	*/
	InitCursor();						/* Initialize my cursor */
	InitFonts();						/* Initialize fonts		*/
	InitWindows();						/* Initialize windows	*/
	InitMenus();						/* Initialize menu mgr.	*/
	TEInit();							/* Init Text Manager	*/
	InitDialogs(NULL);					/* Init dialogs			*/
	FlushEvents(everyEvent,0);			/* Init Event Manager	*/
	
	for (i = 0; i < 10; i++)			/* Force me to front	*/
		GetNextEvent(everyEvent,&e);	/* in multifinder...	*/
	
	GetDateTime((unsigned long *)&l);
	randSeed = l;						/* Randomize this		*/
}


/*	InitMenuBar
 *
 *		Initialize the menu bar
 */

static void InitMenuBar(void)
{
	Handle h;
	MenuHandle mh;
	
	h = GetNewMBar(128);
	SetMenuBar(h);
	mh = GetMHandle(128);
	AddResMenu(mh,'DRVR');
	DrawMenuBar();
}

