//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditParameterDirector.cp
//|
//| This implements the director to control the dialog used to edit a single
//| parameter.
//|______________________________________________________________________________

#include "CEditParameterDialog.h"
#include "CEditParameterDirector.h"
#include "HyperCuber Commands.h"


//============================ Globals ============================\\

extern CDesktop 		*gDesktop;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditParameterDirector::IEditParameterDirector
//|
//| Purpose: Initialize the parameter editing dialog.
//|
//| Parameters: aSupervisor: the supervisor
//|             parameter:   the parameter; if parameter is changed, receives changed parameter
//|__________________________________________________________________________________________

void CEditParameterDirector::IEditParameterDirector (CDirectorOwner *aSupervisor,
															Parameter *parameter)
{

#define EDIT_PARAMETER_DIALOG_ID	137

	CDialogDirector::IDialogDirector (aSupervisor);						//  Init superclass

	theParameter = parameter;											//  Save parameter pointer

	CEditParameterDialog *dialog = new (CEditParameterDialog);			//  Set up the dialog
	dialog->IEditParameterDialog (EDIT_PARAMETER_DIALOG_ID, gDesktop, this, parameter);
	itsWindow = dialog;

}	//=== CEditParameterDirector::IEditParameterDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditParameterDirector::TalkToUser
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CEditParameterDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	CEditParameterDialog *dialog = (CEditParameterDialog *) itsWindow;

	if (dismiss == cmdOK)
		((CEditParameterDialog *) itsWindow)->
						UpdateParameter(theParameter);		//  Update the parameter

}	//=== CEditParameterDirector::TalkToUser ===\\
