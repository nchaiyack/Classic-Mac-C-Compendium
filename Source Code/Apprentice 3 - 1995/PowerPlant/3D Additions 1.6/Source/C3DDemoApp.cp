// ===========================================================================
//	C3DDemoApp.cp
// ===========================================================================


#pragma once

#include "C3DDemoApp.h"
#include "C3DDemoWin.h"

#include <LApplication.h>
#include <LGrowZone.h>
#include <LRadioGroup.h>
#include <LWindow.h>
#include <UMemoryMgr.h>
#include <UDrawingState.h>
#include <URegistrar.h>
#include <UPowerTools.h>

#include <3DobClasses.h>

const ResIDT	WIND_3DDemo	= 200;


// ===========================================================================
//		¥ Main Program
// ===========================================================================

void main(void)
{
									// Set Debugging options
#ifdef Debug_Throw
	gDebugThrow = debugAction_Alert;
#endif

#ifdef Debug_Signal
	gDebugSignal = debugAction_Alert;
#endif

	InitializeHeap(3);				// Initialize Memory Manager
									// Parameter is number of Master Pointer
									//   blocks to allocate
	
									// Initialize standard Toolbox managers
	UQDGlobals::InitializeToolbox(&qd);
	
#ifdef Debug_Signal					// Check for missing MBAR, which
	CheckForInitialMBAR();			// probably means that there is no
#endif								// project resource file
	
	new LGrowZone(20000);			// Install a GrowZone function to catch
									//    low memory situations.
									//    Parameter is size of reserve memory
									//    block to allocated. The first time
									//    the GrowZone function is called,
									//    there will be at least this much
									//    memory left (so you'll have enough
									//    memory to alert the user or finish
									//    what you are doing).
	
	C3DDemoApp	theApp;			// Create instance of your Application
	theApp.Run();					//   class and run it
}


// ===========================================================================
//		¥ C3DDemoApp Class
// ===========================================================================

// ---------------------------------------------------------------------------
//		¥ C3DDemoApp
// ---------------------------------------------------------------------------
//	Constructor

C3DDemoApp::C3DDemoApp()
{
		// Register classes for objects created from 'PPob' resources
		
	URegistrar::RegisterClass(LWindow::class_ID,		LWindow::CreateWindowStream);
	URegistrar::RegisterClass(LRadioGroup::class_ID,	LRadioGroup::CreateRadioGroupStream);

	URegistrar::RegisterClass(C3DDemoWin::class_ID,		C3DDemoWin::CreateFromStream);

	RegisterAll3DClasses();
	
		// A 3DDemo program has a single main Window that is
		// displayed on start up. The "Visible on Creation" option
		// for the Window (in its PPob resource) should be OFF,
		// so that you can adjust the Window's contents before
		// displaying it.
		
	mDisplayWindow = LWindow::CreateWindow(WIND_3DDemo, this);
	
		// +++ Add code here to configure Panes inside the Window
		
	mDisplayWindow->Show();
}


// ---------------------------------------------------------------------------
//		¥ ~C3DDemoApp
// ---------------------------------------------------------------------------
//	Destructor

C3DDemoApp::~C3DDemoApp()
{
		// +++ Add code here to cleanup (if necessary) before quitting
}


// ---------------------------------------------------------------------------
//		¥ ObeyCommand
// ---------------------------------------------------------------------------
//	Respond to commands

Boolean
C3DDemoApp::ObeyCommand(
	CommandT	inCommand,
	void		*ioParam)
{
	Boolean	cmdHandled = true;
	
	switch (inCommand) {
	
		// +++ Add cases here for the commands you handle
		//		Remember to add same cases to FindCommandStatus below
		//		to enable/disable the menu items for the commands
	
		default:
			cmdHandled = LApplication::ObeyCommand(inCommand, ioParam);
			break;
	}
	
	return cmdHandled;
}


// ---------------------------------------------------------------------------
//		¥ FindCommandStatus
// ---------------------------------------------------------------------------
//	Pass back status of a (menu) command

void
C3DDemoApp::FindCommandStatus(
	CommandT	inCommand,
	Boolean		&outEnabled,
	Boolean		&outUsesMark,
	Char16		&outMark,
	Str255		outName)
{
	outUsesMark = false;
	
	switch (inCommand) {
	
		// +++ Add cases here for the commands you handle.
		//
		//		Set outEnabled to TRUE for commands that can be executed at
		//		this time.
		//
		//		If the associated menu items can have check marks, set
		//		outUsesMark and outMark accordingly.
		//
		//		Set outName to change the name of the menu item
	
		default:
			LApplication::FindCommandStatus(inCommand, outEnabled, outUsesMark,
								outMark, outName);
			break;
	}
}