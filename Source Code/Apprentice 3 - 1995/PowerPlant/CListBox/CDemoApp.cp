// =================================================================================
//	CDemoApp.cp		©1994 Harold Ekstrom, AG Group, Inc. All rights reserved.
// =================================================================================
//	CDemoApp.h

#include <LWindow.h>
#include <UScreenPort.h>
#include <String_Utils.h>
#include <PP_Messages.h>
#include <PP_Resources.h>
#include <PPobClasses.h>
#include <URegistrar.h>
#include <UReanimator.h>
#include <UDesktop.h>
#include <LGrowZone.h>
#include <LMenuBar.h>
#include <UMemoryMgr.h>
#include <UPowerTools.h>
#include <UException.h>
#include <UDebugging.h>

#include "CDemoList.h"
#include "CDemoApp.h"


// =================================================================================
//		¥ Main Program
// =================================================================================

void main(void)
{
	MaxApplZone();
	InitializeHeap(4);
	InitializeToolbox();
	new LGrowZone(20000);

	CDemoApp	theApp;
	theApp.Run();
}


// ---------------------------------------------------------------------------------
//		¥ CDemoApp
// ---------------------------------------------------------------------------------

CDemoApp::CDemoApp()
{
	UScreenPort::Initialize();

	//	Register the functions to create our listbox classes
	URegistrar::RegisterClass( 'DemL', (ClassCreatorFunc) CDemoList::CreateDemoListStream );
	URegistrar::RegisterClass( 'wind', (ClassCreatorFunc) LWindow::CreateWindowStream );

	gDebugThrow = debugAction_Alert;
}


// ---------------------------------------------------------------------------------
//		¥ ~CDemoApp
// ---------------------------------------------------------------------------------

CDemoApp::~CDemoApp()
{
}


// ---------------------------------------------------------------------------------
//		¥ ObeyCommand
// ---------------------------------------------------------------------------------

Boolean
CDemoApp::ObeyCommand( CommandT inCommand, void *ioParam )
{
	Boolean	cmdHandled = true;
	LWindow	*theWindow;

	switch (inCommand) {

		case cmd_New:
			theWindow = LWindow::CreateWindow( 128, this );
			break;

		default:
			cmdHandled = LApplication::ObeyCommand( inCommand, ioParam );
			break;
	}

	return cmdHandled;
}



// ---------------------------------------------------------------------------------
//		¥ FindCommandStatus
// ---------------------------------------------------------------------------------

void
CDemoApp::FindCommandStatus( CommandT inCommand, Boolean &outEnabled,
	Boolean &outUsesMark, Char16 &outMark, Str255 outName )
{
	ResIDT			menuID;
	Int16			menuItem;

	outEnabled = outUsesMark = false;

	switch ( inCommand ) {
	
		case cmd_New:
			outEnabled = true;
			outUsesMark = false;
			break;
			
		default:
			LApplication::FindCommandStatus( inCommand, outEnabled,
				outUsesMark, outMark, outName );
			break;
	}
}

