//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsDialog.cp
//|
//| This implements the dialog used to edit key controls
//|_________________________________________________________

#include "CKeyControlsArrayPane.h"
#include "CKeyControlsDialog.h"
#include "CHyperCuberPrefs.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <CButton.h>
#include <CPaneBorder.h>
#include <CDecorator.h>
#include <CEditText.h>

#include <string.h>

//============================ Globals ===============================\\

extern CDecorator 		*gDecorator;
extern CHyperCuberPrefs *gPrefs;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsDialog::IKeyControlsDialog
//|
//| Purpose: Initialize the controls dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this DialogDirector
//|             array:      the array of key controls
//|______________________________________________________________________________

void CKeyControlsDialog::IKeyControlsDialog(short WindowID, CDesktop *enclosure,
											CDirector *supervisor, CArray *array,
												CKeyControlsArrayPane *array_pane)
{

	
#define OKAY_BUTTON_ID		130
#define CANCEL_BUTTON_ID	131
#define DEFAULTS_BUTTON_ID	132
#define ADD_BUTTON_ID		133
#define REMOVE_BUTTON_ID	134
#define EDIT_BUTTON_ID		135


	CDialog::IDialog(WindowID, enclosure, supervisor);	//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;					//  Link in the Balloon Help

	CButton		*button;
	button = new(CButton);							//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID, this, this);
	button->Offset(350, 35, TRUE);
	button->SetClickCmd (cmdOK);
	button->helpResIndex = kKeyOkay;
	SetDefaultButton(button);
	
	button = new (CButton);							//  Set up the cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(350, 70, TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kKeyCancel;
	
	button = new (CButton);							//  Set up the Defaults button
	button->IButton(DEFAULTS_BUTTON_ID, this, this);
	button->Offset(350, 105, TRUE);
	button->SetClickCmd (cmdDefaults);
	button->helpResIndex = kKeyDefaults;
	
	button = new (CButton);							//  Set up the Add button
	button->IButton(ADD_BUTTON_ID, this, this);
	button->Offset(350, 140, TRUE);
	button->SetClickCmd (cmdAdd);
	button->helpResIndex = kKeyAdd;
	
	button = new (CButton);							//  Set up the Remove button
	button->IButton(REMOVE_BUTTON_ID, this, this);
	button->Offset(350, 175, TRUE);
	button->SetClickCmd (cmdRemove);
	button->helpResIndex = kKeyRemove;
	
	button = new (CButton);							//  Set up the Edit button
	button->IButton(EDIT_BUTTON_ID, this, this);
	button->Offset(350, 210, TRUE);
	button->SetClickCmd (cmdEdit);
	button->helpResIndex = kKeyEdit;
	
	CScrollPane *scroll_pane;
	scroll_pane = new(CScrollPane);					//  Set up the scrolling pane
	scroll_pane->IScrollPane(this, this, 300, 254,
							20, 20,
							sizFIXEDLEFT, sizFIXEDTOP,
							FALSE, TRUE, FALSE);
	
	CPaneBorder *border;
	border = new(CPaneBorder);						//  Add a box around the scroll pane
	border->IPaneBorder(kBorderFrame);
	Rect margin = {1, 1, -1, -1};
	border->SetMargin(&margin);
	scroll_pane->SetBorder(border);

	array_pane->IKeyControlsArrayPane(scroll_pane, scroll_pane,
						10, 10, 0, 0,
						sizFIXEDLEFT, sizFIXEDTOP);	//  Set up the array pane
	array_pane->FitToEnclosure(TRUE, TRUE);
	array_pane->SetArray(array, TRUE);
	array_pane->SetDblClickCmd(cmdEdit);
	
	scroll_pane->InstallPanorama(array_pane);

	CEditText *text;
	text = new(CEditText);									//  Set up the Function text
	text->IEditText(this, this, 100, 16,
					23, 3, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pFunction");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	
	text = new(CEditText);									//  Set up the Key text
	text->IEditText(this, this, 100, 16,
					239, 3, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pKey");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	

	gDecorator->CenterWindow (this);				//  Center the window on the screen

}	//=== CKeyControlsDialog::IKeyControlsDialog() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CKeyControlsDialog::MatchDialogToPrefs
//|
//| Purpose: Sets the values of the keys in the dialog to match the prefs
//|______________________________________________________________________________

void CKeyControlsDialog::MatchDialogToPrefs(void)
{

	PrefsStruct prefs = gPrefs->prefs;		// Get current prefs

#if 0
	increase_theta_keypane->SetKey(prefs.increase_theta_key);
	decrease_theta_keypane->SetKey(prefs.decrease_theta_key);
	increase_phi_keypane->SetKey(prefs.increase_phi_key);
	decrease_phi_keypane->SetKey(prefs.decrease_phi_key);
	increase_rho_keypane->SetKey(prefs.increase_rho_key);
	decrease_rho_keypane->SetKey(prefs.decrease_rho_key);
	increase_alpha_keypane->SetKey(prefs.increase_alpha_key);
	decrease_alpha_keypane->SetKey(prefs.decrease_alpha_key);
	increase_beta_keypane->SetKey(prefs.increase_beta_key);
	decrease_beta_keypane->SetKey(prefs.decrease_beta_key);
	increase_gamma_keypane->SetKey(prefs.increase_gamma_key);
	decrease_gamma_keypane->SetKey(prefs.decrease_gamma_key);
	increase_delta_keypane->SetKey(prefs.increase_delta_key);
	decrease_delta_keypane->SetKey(prefs.decrease_delta_key);

	times_ten_mod_menu->GetMenu()->SelectItem(prefs.times_ten_modifier, pmForceOn);
#endif
	
}	//==== CKeyControlsDialog::MatchDialogToPrefs() ====\\