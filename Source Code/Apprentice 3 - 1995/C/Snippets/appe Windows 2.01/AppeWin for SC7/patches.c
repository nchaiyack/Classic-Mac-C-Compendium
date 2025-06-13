// File "patches.c" -

#include <Traps.h>

#include "main.h"
#include "patches.h"

// ***********************************************************************************
// Global Declarations 

ExitToShellProcPtr gSaveExitToShell;

ExitToShellProcPtr gExitToShellPatch=0;
NewWindowProcPtr gNewWindowPatch=0;

// ***********************************************************************************
// ***********************************************************************************

void PatchExitToShell() {
	short trap = _ExitToShell; 
	
	if (! gExitToShellPatch) gExitToShellPatch = MyExitToShell;
	if (! gExitToShellPatch) return;
	
	gSaveExitToShell = (ExitToShellProcPtr) 
			NGetTrapAddress(trap, (trap & 0x0800) ? ToolTrap : OSTrap);
	NSetTrapAddress((UniversalProcPtr)gExitToShellPatch, trap, (trap & 0x0800) ? ToolTrap : OSTrap);
	}
	
// ***********************************************************************************
// ***********************************************************************************

pascal void MyExitToShell() {
	static Boolean done = FALSE;
	long saveA5;
	ExitToShellProcPtr tempSaveExitToShell;
	
	// Setup (68k) globals in case we are ES-ing from deep within Macsbug.
	saveA5 = SetCurrentA5();

	// Our ExitToShell() patch may cause re-entrancy problems... so we bracket
	//   the functional calls by checking and setting a one-time flag.
	if (! done) {
		done = TRUE;
		DoDispose();
		}

	// Save off the global pointer while we still have our 68K A5-World set up
	tempSaveExitToShell = gSaveExitToShell;
	SetA5(saveA5);
	
	(*tempSaveExitToShell)();
	}

// ***********************************************************************************
// ***********************************************************************************

void PatchNewWindow() {
	short trap = _NewWindow; 

	if (! gNewWindowPatch) gNewWindowPatch = MyNewWindow;
	if (! gNewWindowPatch) return;
	
	NSetTrapAddress((UniversalProcPtr)gNewWindowPatch, trap, (trap & 0x0800) ? ToolTrap : OSTrap);
	}
	
// ***********************************************************************************
// ***********************************************************************************

pascal WindowPtr MyNewWindow(Ptr wStorage, Rect *bounds, StringPtr title,
		Boolean vis, short procID, WindowPtr behind, Boolean close, long refCon) {
	short trap = _NewCWindow;
	WindowPtr win;
	NewWindowProcPtr myNewCWindow;
	
	// Call NewCWindow in place of NewWindow!
	myNewCWindow = (NewWindowProcPtr) 
			NGetTrapAddress(trap, (trap & 0x0800) ? ToolTrap : OSTrap);
	
	win = (*myNewCWindow)(wStorage, bounds, title, vis, procID, behind, close, refCon);

	return(win);
	}

