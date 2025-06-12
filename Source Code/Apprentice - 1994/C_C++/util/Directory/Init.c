//-- INIT.C

// This contains the code which initializes the sample application.

#include <stdio.h>
#include "res.h"
#include "struct.h"
#include "error.h"


MenuHandle applMenu,fileMenu,editMenu;
unsigned char ColorQD,MultiWidget;		/* State flags */

#define TRAP_WNE	0x060
#define TRAP_UNIMP	0x09F


//-- Initialize Application --//

// The entry point for application initialization.

InitMacintosh()
{
	int i;

	InitToolbox();		// Step 1:  Initialize the macintosh.

	if (i = Catch())	// Step 2:	Trap errors; if a problem comes 
	{					//			up, simply exit to shell and
		PostError(i);
		ExitToShell();	//			forget it.
	}

	InitEnv();			// Step 2:  Get environment stuff.
	InitMenuBar();		// Step 3:  Initialize the menu bar.
	InitGlobals();		// Step 4:  Initialize globals.
	
	Uncatch();			//Step 5:	All done.  Forget the error trap.
}

//-- InitToolbox --//

// Bring up the Macintosh operating system as per Inside Macintosh.

InitToolbox()
{
	InitGraf(&qd.thePort);
	InitCursor();
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);
	FlushEvents(everyEvent,0);
}

//-- InitEnv --//

// Get environment state.  The various things to test for include:

// ¥	Multifinder environment?
// ¥	Color quickdraw present?.

InitEnv()
{
	SysEnvRec theWorld;
	OSErr err;
	short i;
	Handle h;

	err = SysEnvirons(1,&theWorld);
	if (theWorld.hasColorQD) ColorQD = 1;
	else ColorQD = 0;

	if ((theWorld.machineType >= 0) && 
		(NGetTrapAddress(TRAP_WNE,ToolTrap) != NGetTrapAddress(TRAP_UNIMP,ToolTrap)))
		MultiWidget = 1;
	else MultiWidget = 0;
}


//-- InitMenuBar --//

// Turn on the menu bar.

InitMenuBar()
{
	applMenu = GetMenu(APPLMENU);
	AddResMenu(applMenu,'DRVR');
	fileMenu = GetMenu(FILEMENU);
	editMenu = GetMenu(EDITMENU);

	InsertMenu(applMenu,0);
	InsertMenu(fileMenu,0);
	InsertMenu(editMenu,0);

	DrawMenuBar();
}



//-- InitGlobals --//

// Initialize any application globals:

// ¥	struct DrawWindow drawList.

InitGlobals()
{
	int i;
	
	drawList = (struct DrawWindow *)NewPtr(MAXWINDOWS * sizeof(struct DrawWindow));
	if (drawList == NULL) Throw(INITOUTMEM);
	for (i = 0; i < MAXWINDOWS; i++) drawList[i].inuse = 0;
	
	GetMounted();
}
