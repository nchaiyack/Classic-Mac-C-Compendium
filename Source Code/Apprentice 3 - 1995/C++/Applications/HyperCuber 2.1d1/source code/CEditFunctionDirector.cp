//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditFunctionDirector.cp
//|
//| This implements the director to control the dialog used to edit a single
//| parametric function
//|______________________________________________________________________________

#include "CEditFunctionDialog.h"
#include "CEditFunctionDirector.h"
#include "HyperCuber Commands.h"


//============================ Globals ============================\\

extern CDesktop 		*gDesktop;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditFunctionDirector::IEditFunctionDirector
//|
//| Purpose: Initialize the function editing dialog.
//|
//| Parameters: aSupervisor: the supervisor
//|             function:    the function; if function is changed, receives changed function
//|__________________________________________________________________________________________

void CEditFunctionDirector::IEditFunctionDirector (CDirectorOwner *aSupervisor,
															long variable_num, char *function)
{

#define EDIT_FUNCTION_DIALOG_ID	136

	CDialogDirector::IDialogDirector (aSupervisor);						//  Init superclass

	theFunction = function;												//  Save function pointer

	CEditFunctionDialog *dialog = new (CEditFunctionDialog);			//  Set up the dialog
	dialog->IEditFunctionDialog (EDIT_FUNCTION_DIALOG_ID, gDesktop,
									this, variable_num, function);
	itsWindow = dialog;

}	//=== CEditFunctionDirector::IEditFunctionDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditFunctionDirector::TalkToUser
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CEditFunctionDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	CEditFunctionDialog *dialog = (CEditFunctionDialog *) itsWindow;

	if (dismiss == cmdOK)
		((CEditFunctionDialog *) itsWindow)->UpdateFunction(theFunction);	//  Update the function

}	//=== CEditFunctionDirector::TalkToUser ===\\
