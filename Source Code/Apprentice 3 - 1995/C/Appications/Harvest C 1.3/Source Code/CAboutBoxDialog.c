/******************************************************************************
 CAboutBoxDialog.c

		A subclass of CDLOGDirector that displays the about box.
		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CAboutBoxDialog.h"
#include "CApplication.h"
#include "Commands.h"
#include "DemoCommands.h"
#include "CRadioGroupPane.h"
#include "CWindow.h"

#define	kAboutBoxDLOGID	130	// resource ID of the DLOG resource

/******************************************************************************
 IAboutBoxDialog
 
 	Initialize the dialog. This method also assigns help balloon resource
 	indexes for some of the panes in the window.
 	
******************************************************************************/

void CAboutBoxDialog::IAboutBoxDialog( CDirectorOwner *aSupervisor)
{
	
	CDLOGDirector::IDLOGDirector( kAboutBoxDLOGID, aSupervisor);
	
}

/******************************************************************************
 DoAbout
 
******************************************************************************/

long CAboutBoxDialog::DoAbout( void)
{
	long 			theCommand;
		// show the dialog
		
	BeginDialog();
	
		// run the dialog and return the final command.
		
	theCommand = DoModalDialog( cmdOK);
	
	return theCommand;
}
