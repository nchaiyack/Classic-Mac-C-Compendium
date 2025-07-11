//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditMouseControlsDirector.cp
//|
//| This implements the director to control the dialog used to edit mouse controls
//|______________________________________________________________________________

#include "CEditMouseControlsDialog.h"
#include "CEditMouseControlsDirector.h"
#include "HyperCuber Commands.h"


//============================ Globals ============================\\

extern CDesktop 		*gDesktop;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditMouseControlsDirector::IMouseControlsDirector
//|
//| Purpose: Initialize the dialog.
//|
//| Parameters: aSupervisor:   the supervisor
//|             mouse_control: the mouse control; if mouse control is changed,
//|                            receives changed mouse control
//|___________________________________________________________________________

void CEditMouseControlsDirector::IEditMouseControlsDirector (CDirectorOwner *aSupervisor,
															MouseControl *mouse_control)
{

#define EDIT_CONTROLS_DIALOG_ID	133

	CDialogDirector::IDialogDirector (aSupervisor);						//  Init superclass

	theMouseControl = mouse_control;									//  Save mouse command pointer

	CEditMouseControlsDialog *dialog = new (CEditMouseControlsDialog);		//  Set up the dialog
	dialog->IEditMouseControlsDialog (EDIT_CONTROLS_DIALOG_ID, gDesktop, this, mouse_control);
	itsWindow = dialog;

}	//=== CEditMouseControlsDirector::IEditMouseControlsDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditMouseControlsDirector::TalkToUser
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CEditMouseControlsDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	if (dismiss == cmdOK)
		((CEditMouseControlsDialog *) itsWindow)->UpdateMouseControl(theMouseControl);	//  Update the mouse control
	
}	//=== CEditMouseControlsDirector::TalkToUser ===\\
