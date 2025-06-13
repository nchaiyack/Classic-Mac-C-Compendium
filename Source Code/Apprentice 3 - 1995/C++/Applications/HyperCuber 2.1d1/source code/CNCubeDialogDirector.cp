//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNCubeDialogDirector.cp
//|
//| This implements the preferences dialog director
//|_________________________________________________________

#include "CIntegerText.h"
#include "CNCubeDialog.h"
#include "CNCubeDialogDirector.h"

#include "HyperCuber Commands.h"



//============================ Globals ============================\\

extern CDesktop 		*gDesktop;



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CNCubeDialogDirector::INCubeDialogDirector
//
// Purpose: Initialize the n-cube dialog.
//
// Parameters: none
//_________________________________________________________

void CNCubeDialogDirector::INCubeDialogDirector (CDirectorOwner *aSupervisor)
{

#define NCUBE_WINDOW_ID	134

	CDialogDirector::IDialogDirector (aSupervisor);			//  Init superclass

	CNCubeDialog *dialog = new (CNCubeDialog);				//  Set up the n-cube dialog
	dialog->INCubeDialog (NCUBE_WINDOW_ID, gDesktop, this);
	itsWindow = dialog;

}	//=== CNCubeDialogDirector::CNCubeDialogDirector() ===\\



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CNCubeDialogDirector::TalkToUser
//
// Purpose: Handle commands for the dialog.
//
// Parameters: returns dimension of n-cube, 0 if cancelled
//_________________________________________________________

long CNCubeDialogDirector::TalkToUser(void)
{

	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	if (dismiss == cmdOK)
		return ((CNCubeDialog *) itsWindow)->dimension->GetIntValue();

	else
		return 0;
			
}	//=== CNCubeDialogDirector::TalkToUser ===\\
