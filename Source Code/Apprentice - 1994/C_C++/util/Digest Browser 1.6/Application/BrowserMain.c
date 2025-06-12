/******************************************************************************

  BrowserMain.c
 
 	The main file for the Digest Browswer.  It displays a splash
 	screen, initialiazes variables, and gets TCL running.
 
  Copyright © 1991 Manuel A. PŽrez.  All rights reserved.
  
******************************************************************************/

 
#include "CBrowserApp.h"

// Prototypes
void InitToolbox(void);


void main()
{
DialogPtr theDialog;
CBrowserApp	*BrowserApp;					
long dlogDisplay;

	// initialize the toolbox
	InitToolbox();

	dlogDisplay = TickCount();
	theDialog = GetNewDialog(1000, NULL, (WindowPtr)-1);
	if (theDialog)
		DrawDialog(theDialog);				// make sure is drawn

	// Create a new application object, and set everything up
	BrowserApp = new CBrowserApp;
	BrowserApp->IBrowserApp();

	// leave the dialog up for 2 seconds
	if (theDialog) {
		while (TickCount() - dlogDisplay < (2 * 60));
		DisposeDialog(theDialog);
	}

	// ... and go
	BrowserApp->Run();
	BrowserApp->Exit();
}

// InitToolbox, this is the InitToolbox from TCL. I moved it
// out here so that we can display the splash screen.
void InitToolbox(void)
{
	InitGraf(&thePort);					// Standard initialization calls
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(NULL);					// Add a ResumeProc
	InitCursor();
}
