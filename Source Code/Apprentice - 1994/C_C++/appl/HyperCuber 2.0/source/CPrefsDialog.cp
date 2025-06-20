//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefsDialog.cp
//|
//| This implements the preferences dialog
//|_________________________________________________________

#include "CPrefsDialog.h"
#include "CColorPane.h"
#include "CHyperCuberDoc.h"
#include "CHyperCuberPrefs.h"
#include "CPrefsDialogDirector.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <CButton.h>
#include <CDecorator.h>
#include <CEditText.h>
#include <CPaneBorder.h>

#include <string.h>

extern CDecorator 		*gDecorator;
extern CHyperCuberPrefs	*gPrefs;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefsDialog::IPrefsDialog
//|
//| Purpose: Initialize the preferences dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this DialogDirector
//|             doc:        the document (used to get the prefs)
//|______________________________________________________________________________

void CPrefsDialog::IPrefsDialog(short WindowID, CDesktop *enclosure, CDirector *supervisor)
{

	CButton		*button;
	
#define OKAY_BUTTON_ID		130
#define CANCEL_BUTTON_ID	131
#define DEFAULTS_BUTTON_ID	132

	CDialog::IDialog(WindowID, enclosure, supervisor);	//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;					//  Link in the Balloon Help

	button = new(CButton);							//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID, this, this);
	button->Offset(210, 15, TRUE);
	button->SetClickCmd (cmdOK);
	button->helpResIndex = kColorsOkay;
	SetDefaultButton(button);
	
	button = new (CButton);							//  Set up the cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(210, 50, TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kColorsCancel;
	
	button = new (CButton);							//  Set up the defaults button
	button->IButton(DEFAULTS_BUTTON_ID, this, this);
	button->Offset(210, 85, TRUE);
	button->SetClickCmd (cmdDefaults);
	button->helpResIndex = kColorsDefault;
	
	InstallColorEntry(&background_color_pane, 10,	//  Set up the colored squares in the dialog
						"Background Color:",	
						&(gPrefs->prefs.background_color),
						kColorsBackground);
	InstallColorEntry(&left_eye_color_pane, 40,
						"Left Eye Color:",
						&(gPrefs->prefs.left_eye_color),
						kColorsLeftEye);
	InstallColorEntry(&right_eye_color_pane, 70,
						"Right Eye Color:",
						&(gPrefs->prefs.right_eye_color),
						kColorsRightEye);
	
	gDecorator->CenterWindow (this);				//  Center the window on the screen

}	//=== CPrefsDialog::IPrefsDialog() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CPrefsDialog::InstallColorEntry
//|
//| Purpose: Initialize the preferences dialog.
//|
//| Parameters: color_pane: handle which receives newly-created CColorPane
//|             vert:       vertical position of the pane
//|             color_name: string to put to left of pane
//|             color:      color of pane
//|             help_res:   resource for Balloon Help
//|________________________________________________________________________

void CPrefsDialog::InstallColorEntry(CColorPane **color_pane, short vert, char *color_name,
										RGBColor *color, short help_res)
{

	CEditText *text = new(CEditText);						//  Set up the color name pane
	text->IEditText(this, this, 150, 20,
					10, vert + 2, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextPtr(color_name, strlen(color_name));
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	
	*color_pane = new(CColorPane);							//  Set up the colored pane
	(*color_pane)->IColorPane(this, this, 20, 20,
							150, vert, sizFIXEDLEFT, sizFIXEDTOP, color);
	(*color_pane)->SetWantsClicks(TRUE);
	(*color_pane)->helpResIndex = help_res;

	CPaneBorder *border = new(CPaneBorder);					//  Border the color name pane
	border->IPaneBorder(kBorderFrame);
	border->SetPenSize(2, 2);
	(*color_pane)->SetBorder(border);

}	//==== CPrefsDialog::InstallColorEntry() ====\\
