//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefsDialogDirector.cp
//|
//| This implements the preferences dialog director
//|_________________________________________________________

#include "CColorPane.h"
#include "CHyperCuberDoc.h"
#include "CHyperCuberPrefs.h"
#include "CPrefsDialog.h"
#include "CPrefsDialogDirector.h"

#include "HyperCuber Commands.h"



//============================ Globals ============================\\

extern CDesktop 		*gDesktop;
extern CHyperCuberPrefs	*gPrefs;



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CPrefsDialogDirector::IPrefsDialogDirector
//
// Purpose: Initialize the preferences dialog.
//
// Parameters: none
//_________________________________________________________

void CPrefsDialogDirector::IPrefsDialogDirector (CDirectorOwner *aSupervisor)
{

#define PREFS_WINDOW_ID	128

	CDialogDirector::IDialogDirector (aSupervisor);			//  Init superclass

	CPrefsDialog *dialog = new (CPrefsDialog);				//  Set up the prefs dialog
	dialog->IPrefsDialog (PREFS_WINDOW_ID, gDesktop, this);
	itsWindow = dialog;

}	//=== CPrefsDialogDirector::CPrefsDialogDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefsDialogDirector::DoKeyDown / CPrefsDialogDirector::DoAutoKey
//|
//| Purpose: These procedure handles a key down event.  Their sole purpose is to
//|          prevent the keydown from getting to the Controls Director.
//|
//| Parameters: ignored
//|______________________________________________________________________________

void	CPrefsDialogDirector::DoKeyDown(char the_char, Byte key_code, EventRecord *event)
{

}	//==== CPrefsDialogDirector::DoKeyDown() ====\\


void	CPrefsDialogDirector::DoAutoKey(char the_char, Byte key_code, EventRecord *event)
{

}	//==== CPrefsDialogDirector::DoAutoKey() ====\\



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CPrefsDialogDirector::DoCommand
//
// Purpose: Handle commands for the dialog.
//
// Parameters: none
//_________________________________________________________

void CPrefsDialogDirector::DoCommand( long aCmd)
{

	switch (aCmd)
	{
	
		case cmdDefaults:
		
			PrefsStruct prefs;
			prefs = gPrefs->prefs;					//  Save the current prefs

			gPrefs->SetDefaults();					//  Set all preferences to default values
			
			CPrefsDialog *dialog = (CPrefsDialog *) itsWindow;			//  Change the colors
			dialog->background_color_pane->SetPaneColor(&(gPrefs->prefs.background_color));
			dialog->left_eye_color_pane->SetPaneColor(&(gPrefs->prefs.left_eye_color));
			dialog->right_eye_color_pane->SetPaneColor(&(gPrefs->prefs.right_eye_color));

			gPrefs->prefs = prefs;					//  Restore the current prefs

			break;
	
		default:
			inherited::DoCommand( aCmd);
			break;
	}
	
}	//=== CPrefsDialogDirector::DoCommand ===\\



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// CPrefsDialogDirector::TalkToUser
//
// Purpose: Handle commands for the dialog.
//
// Parameters: none
//_________________________________________________________

void CPrefsDialogDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	
	if (dismiss == cmdOK)
		{

		PrefsStruct prefs = gPrefs->prefs;				//  Get the current prefs

		CPrefsDialog *dialog = (CPrefsDialog *) itsWindow;			//  Add the new colors
		dialog->background_color_pane->GetPaneColor(&(prefs.background_color));
		dialog->left_eye_color_pane->GetPaneColor(&(prefs.left_eye_color));
		dialog->right_eye_color_pane->GetPaneColor(&(prefs.right_eye_color));

		gPrefs->prefs = prefs;							// update prefs to reflect changes
		
		}
	
}	//=== CPrefsDialogDirector::TalkToUser ===\\
