//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditKeyControlsDirector.cp
//|
//| This implements the director to control the dialog used to edit key controls
//|______________________________________________________________________________

#include "CEditKeyControlsDialog.h"
#include "CEditKeyControlsDirector.h"
#include "HyperCuber Commands.h"


//============================ Globals ============================\\

extern CDesktop 		*gDesktop;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditKeyControlsDirector::IKeyControlsDirector
//|
//| Purpose: Initialize the dialog.
//|
//| Parameters: aSupervisor: the supervisor
//|             key:         the key; if key is changed, receives changed key
//|___________________________________________________________________________

void CEditKeyControlsDirector::IEditKeyControlsDirector (CDirectorOwner *aSupervisor,
															KeyControl *key)
{

#define EDIT_CONTROLS_DIALOG_ID	133

	CDialogDirector::IDialogDirector (aSupervisor);						//  Init superclass

	theKey = key;														//  Save key command pointer

	CEditKeyControlsDialog *dialog = new (CEditKeyControlsDialog);		//  Set up the dialog
	dialog->IEditKeyControlsDialog (EDIT_CONTROLS_DIALOG_ID, gDesktop, this, key);
	itsWindow = dialog;

}	//=== CEditKeyControlsDirector::IEditKeyControlsDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditKeyControlsDirector::TalkToUser
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CEditKeyControlsDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	CEditKeyControlsDialog *dialog = (CEditKeyControlsDialog *) itsWindow;

	if (dismiss == cmdOK)
		((CEditKeyControlsDialog *) itsWindow)->UpdateKey(theKey);	//  Update the key

}	//=== CEditKeyControlsDirector::TalkToUser ===\\
