//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditFunctionDialog.cp
//|
//| This implements the dialog used to edit a single parametric function
//|______________________________________________________________________

#include "CEditFunctionDialog.h"
#include "CHyperCuberPrefs.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <CBartender.h>
#include <CButton.h>
#include <CDecorator.h>
#include <CDialogText.h>
#include <CEditText.h>

#include <ctype.h>
#include <stdio.h>
#include <string.h>

//============================ Globals ===============================\\

extern CDecorator 		*gDecorator;
extern CBartender 		*gBartender;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditFunctionDialog::IEditFunctionDialog
//|
//| Purpose: Initialize the dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this DialogDirector
//|             function:   the default function text
//|______________________________________________________________________________

void CEditFunctionDialog::IEditFunctionDialog(short WindowID, CDesktop *enclosure,
											CDirector *supervisor, long variable_num,
											char *function)
{

	
#define OKAY_BUTTON_ID		130
#define CANCEL_BUTTON_ID	131

	CDialog::IDialog(WindowID, enclosure, supervisor);		//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;							//  Link in the Balloon Help

	CButton	*button;
	button = new(CButton);									//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID, this, this);
	button->Offset(310, 60, TRUE);
	button->SetClickCmd (cmdOK);
	button->helpResIndex = kKeyEdOkay;
	SetDefaultButton(button);
	
	button = new (CButton);									//  Set up the cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(210, 60, TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kKeyEdCancel;
	
	char variable_text[20];
	sprintf(variable_text, "x%d =", variable_num);
	CEditText *text = new(CEditText);						//  Set up the variable text
	text->IEditText(this, this, 45, 16,
					0, 10, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString(CtoPstr(variable_text));
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	text->SetAlignCmd(cmdAlignRight);
	
	function_text = new(CDialogText);						//  Set up the editable function text
	function_text->IDialogText(this, this, 330, 32, 
							50, 10,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	function_text->SetFontNumber(systemFont);
	function_text->SetFontSize(12);
	function_text->SetConstraints(TRUE,
									MAX_FUNCTION_LENGTH);	//  Make function required, but no more
															//   than MAX_FUNCTION_LENGTH
															//   characters long.
	function_text->helpResIndex = kKeyEdAngle;

	function_text->SetTextString(CtoPstr(function));		//  Set the function text
	PtoCstr((unsigned char *) function);

	gDecorator->CenterWindow (this);						//  Center the window on the screen

}	//=== CEditFunctionDialog::IEditFunctionDialog() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditFunctionDialog::UpdateFunction
//|
//| Purpose: Sets the function to match the function text in the dialog.
//|
//| Parameters: function: receives the function text in the dialog
//|______________________________________________________________________________

void CEditFunctionDialog::UpdateFunction(char *function)
{

	function_text->GetTextString((unsigned char *) function);	//  Get the function text
	PtoCstr((unsigned char *) function);

}	//==== CEditFunctionDialog::UpdateFunction() ====\\