//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDialog.cp
//|
//| This implements the dialog used to define the equation.
//|_________________________________________________________

#include "CEquationDialog.h"
#include "CFunctionsArrayPane.h"
#include "CHyperCuberPrefs.h"
#include "CParametersArrayPane.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <CArrayPane.h>
#include <CButton.h>
#include <CPaneBorder.h>
#include <CDecorator.h>
#include <CEditText.h>

#include <string.h>

//============================ Globals ===============================\\

extern CDecorator 		*gDecorator;
extern CHyperCuberPrefs *gPrefs;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDialog::IEquationDialog
//|
//| Purpose: Initialize the equation dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this DialogDirector
//|______________________________________________________________________________

void CEquationDialog::IEquationDialog(short WindowID, CDesktop *enclosure, CDirector *supervisor,
							CArray *parameters_array, CParametersArrayPane *parameters_array_pane,
							CArray *functions_array, CFunctionsArrayPane *functions_array_pane)
{

	
#define CANCEL_BUTTON_ID	131
#define DEFAULTS_BUTTON_ID	132
#define ADD_BUTTON_ID		133
#define REMOVE_BUTTON_ID	134
#define EDIT_BUTTON_ID		135
#define ACCEPT_BUTTON_ID	139
#define GRAPH_BUTTON_ID		140
#define ADD_BUTTON2_ID		141
#define REMOVE_BUTTON2_ID	142
#define EDIT_BUTTON2_ID		143

#define PARAMETERS_HEIGHT	135
#define FUNCTIONS_HEIGHT	140

	CDialog::IDialog(WindowID, enclosure, supervisor);	//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;					//  Link in the Balloon Help

	CButton		*button;
	button = new(CButton);							//  Set up the Graph button
	button->IButton(GRAPH_BUTTON_ID, this, this);
	button->Offset(300,
			PARAMETERS_HEIGHT+PARAMETERS_HEIGHT+20,
			TRUE);
	button->SetClickCmd (cmdGraph);
	button->helpResIndex = kKeyOkay;
	SetDefaultButton(button);
	
	button = new(CButton);							//  Set up the Accept button
	button->IButton(ACCEPT_BUTTON_ID, this, this);
	button->Offset(200,
			PARAMETERS_HEIGHT+PARAMETERS_HEIGHT+20,
			TRUE);
	button->SetClickCmd (cmdAccept);
	button->helpResIndex = kKeyOkay;
	
	button = new (CButton);							//  Set up the Cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(100,
			PARAMETERS_HEIGHT+PARAMETERS_HEIGHT+20,
			TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kKeyCancel;

	CPane *parameters_pane;
	parameters_pane = new(CPane);
	parameters_pane->IPane(this, this,
							10, PARAMETERS_HEIGHT,
							0, 0,
							sizELASTIC, sizELASTIC);//  Create the pane containing parameter controls
	parameters_pane->FitToEnclosure(TRUE, FALSE);	//  Expand it to fill the window horizontally.
	parameters_pane->SetWantsClicks(TRUE);			//  Accept mouse clicks in this pane
	
	CPaneBorder *border;
	border = new(CPaneBorder);						//  Add a line at the bottom of the pane
	border->IPaneBorder(kBorderBottom);
	parameters_pane->SetBorder(border);
	
	CPane *functions_pane;
	functions_pane = new(CPane);
	functions_pane->IPane(this, this,
							10, FUNCTIONS_HEIGHT,
							0, PARAMETERS_HEIGHT,
							sizELASTIC, sizELASTIC);//  Create the pane containing functions controls
	functions_pane->FitToEnclosure(TRUE, FALSE);	//  Expand it to fill the window horizontally.
	functions_pane->SetWantsClicks(TRUE);			//  Accept mouse clicks in this pane

	border = new(CPaneBorder);						//  Add a line at the bottom of the pane
	border->IPaneBorder(kBorderBottom);
	functions_pane->SetBorder(border);
	
	button = new (CButton);							//  Set up the Add button (for parameters)
	button->IButton(ADD_BUTTON_ID, parameters_pane,
					this);
	button->Offset(350, 25, TRUE);
	button->SetClickCmd (cmdAddParameter);
	button->helpResIndex = kKeyAdd;
	
	button = new (CButton);							//  Set up the Remove button (for parameters)
	button->IButton(REMOVE_BUTTON_ID, parameters_pane,
					this);
	button->Offset(350, 60, TRUE);
	button->SetClickCmd (cmdRemoveParameter);
	button->helpResIndex = kKeyRemove;
	
	button = new (CButton);							//  Set up the Edit button (for parameters)
	button->IButton(EDIT_BUTTON_ID, parameters_pane,
					this);
	button->Offset(350, 95, TRUE);
	button->SetClickCmd (cmdEditParameter);
	button->helpResIndex = kKeyEdit;
	
	CScrollPane *scroll_pane;
	scroll_pane = new(CScrollPane);					//  Set up the scrolling pane for the parameters
	scroll_pane->IScrollPane(parameters_pane, this, 300, 100,
							20, 20,
							sizFIXEDLEFT, sizFIXEDTOP,
							FALSE, TRUE, FALSE);
	
	border = new(CPaneBorder);						//  Add a box around the scroll pane
	border->IPaneBorder(kBorderFrame);
	Rect margin = {1, 1, -1, -1};
	border->SetMargin(&margin);
	scroll_pane->SetBorder(border);

	parameters_array_pane->IParametersArrayPane(
						scroll_pane, scroll_pane,
						10, 10, 0, 0,
						sizFIXEDLEFT, sizFIXEDTOP);	//  Set up the parameters array pane
	parameters_array_pane->FitToEnclosure(TRUE, TRUE);
	parameters_array_pane->SetArray(parameters_array, TRUE);
	parameters_array_pane->SetDblClickCmd(cmdEditParameter);
	
	scroll_pane->InstallPanorama(parameters_array_pane);

	button = new (CButton);							//  Set up the Add button (for functions)
	button->IButton(ADD_BUTTON2_ID, functions_pane,
					this);
	button->Offset(350, 30+PARAMETERS_HEIGHT, TRUE);
	button->SetClickCmd (cmdAddFunction);
	button->helpResIndex = kKeyAdd;
	
	button = new (CButton);							//  Set up the Remove button (for functions)
	button->IButton(REMOVE_BUTTON2_ID, functions_pane,
					this);
	button->Offset(350, 65+PARAMETERS_HEIGHT, TRUE);
	button->SetClickCmd (cmdRemoveFunction);
	button->helpResIndex = kKeyRemove;
	
	button = new (CButton);							//  Set up the Edit button (for functions)
	button->IButton(EDIT_BUTTON2_ID, functions_pane,
					this);
	button->Offset(350, 100+PARAMETERS_HEIGHT, TRUE);
	button->SetClickCmd (cmdEditFunction);
	button->helpResIndex = kKeyEdit;
	
	scroll_pane = new(CScrollPane);					//  Set up the scrolling pane for the functions
	scroll_pane->IScrollPane(functions_pane, this,
							300, 100,
							20, 25,
							sizFIXEDLEFT, sizFIXEDTOP,
							FALSE, TRUE, FALSE);
	
	border = new(CPaneBorder);						//  Add a box around the scroll pane
	border->IPaneBorder(kBorderFrame);
	border->SetMargin(&margin);
	scroll_pane->SetBorder(border);

	functions_array_pane->IFunctionsArrayPane(
						scroll_pane, scroll_pane,
						10, 10, 0, 0,
						sizFIXEDLEFT, sizFIXEDTOP);	//  Set up the functions array pane
	functions_array_pane->FitToEnclosure(TRUE, TRUE);
	functions_array_pane->SetArray(functions_array, TRUE);
	functions_array_pane->SetDblClickCmd(cmdEditFunction);
	
	scroll_pane->InstallPanorama(functions_array_pane);

	CEditText *text;
	text = new(CEditText);									//  Set up the Functions text
	text->IEditText(functions_pane, this, 100, 16,
					23, 8, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pFunctions");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	
	text = new(CEditText);									//  Set up the Parameters text
	text->IEditText(parameters_pane, this, 100, 16,
					23, 3, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pParameters");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	

	gDecorator->CenterWindow (this);				//  Center the window on the screen

}	//=== CEquationDialog::IEquationDialog() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEquationDialog::UpdatePrefs
//|
//| Purpose: Sets the preferences to match what is displayed in the dialog.
//|______________________________________________________________________________

void CEquationDialog::UpdatePrefs(void)
{

//	PrefsStruct prefs = gPrefs->prefs;		// Get current prefs

}	//==== CEquationDialog::UpdatePrefs() ====\\