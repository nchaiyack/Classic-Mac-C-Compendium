//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditParameterDialog.cp
//|
//| This implements the dialog used to edit a single parameter
//|____________________________________________________________

#include "CEditParameterDialog.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"
#include "CNumberText.h"

#include <CBartender.h>
#include <CButton.h>
#include <CDecorator.h>
#include <CDialogText.h>

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//============================ Globals ===============================\\

extern CDecorator 		*gDecorator;
extern CBartender 		*gBartender;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditParameterDialog::IEditParameterDialog
//|
//| Purpose: Initialize the dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this DialogDirector
//|             key:        the key to edit
//|______________________________________________________________________________

void CEditParameterDialog::IEditParameterDialog(short WindowID, CDesktop *enclosure,
											CDirector *supervisor, Parameter *parameter)
{

	
#define OKAY_BUTTON_ID		130
#define CANCEL_BUTTON_ID	131

#define BOTTOM_HEIGHT		40

	CDialog::IDialog(WindowID, enclosure, supervisor);	//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;						//  Link in the Balloon Help

	CButton		*button;
	button = new(CButton);								//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID, this, this);
	button->Offset(260, BOTTOM_HEIGHT + 40, TRUE);
	button->SetClickCmd (cmdOK);
	button->helpResIndex = kKeyEdOkay;
	SetDefaultButton(button);
	
	button = new (CButton);								//  Set up the cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(160, BOTTOM_HEIGHT + 40, TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kKeyEdCancel;
	
	CEditText *text = new(CEditText);					//  Set up the Name text
	text->IEditText(this, this,
					65, 16, 0, 10,
					sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pName:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable,
					kNotStylable);
	text->SetAlignCmd(cmdAlignRight);
	
	name_text = new(CDialogText);						//  Set up the editable parameter name text
	name_text->IDialogText(this, this,
							100, 16, 71, 10,
							sizFIXEDLEFT,
							sizFIXEDTOP, -1);
	name_text->SetFontNumber(systemFont);
	name_text->SetFontSize(12);
	name_text->helpResIndex = kKeyEdIncrement;

	short cursor = 20;
	text = new(CEditText);								//  Set up the Min: text
	text->IEditText(this, this, 100, 16,
					cursor, BOTTOM_HEIGHT,
					sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pMin:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable,
					kNotStylable);
	cursor += 34;

	min_text = new(CNumberText);						//  Set up the editable minimum text
	min_text->INumberText(this, this,
							60, 16, 
							cursor, BOTTOM_HEIGHT,
							sizFIXEDLEFT,
							sizFIXEDTOP, -1);
	min_text->SetFontNumber(systemFont);
	min_text->SetFontSize(12);
	min_text->helpResIndex = kKeyEdIncrement;
	cursor += 70;
	
	text = new(CEditText);								//  Set up the Max: text
	text->IEditText(this, this, 100, 16,
						cursor, BOTTOM_HEIGHT,
						sizFIXEDLEFT,
						sizFIXEDTOP, -1);
	text->SetTextString("\pMax:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable,
					kNotStylable);
	cursor += 38;

	max_text = new(CNumberText);						//  Set up the editable maximum text
	max_text->INumberText(this, this,
							60, 16, 
							cursor, BOTTOM_HEIGHT,
							sizFIXEDLEFT,
							sizFIXEDTOP, -1);
	max_text->SetFontNumber(systemFont);
	max_text->SetFontSize(12);
	max_text->helpResIndex = kKeyEdIncrement;
	cursor += 70;
	
	text = new(CEditText);								//  Set up the Step: text
	text->IEditText(this, this, 100, 16,
					cursor, BOTTOM_HEIGHT,
					sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pStep:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable,
					kNotStylable);
	cursor += 39;

	step_text = new(CNumberText);						//  Set up the editable step text
	step_text->INumberText(this, this, 60, 16, 
							cursor, BOTTOM_HEIGHT,
							sizFIXEDLEFT,
							sizFIXEDTOP, -1);
	step_text->SetFontNumber(systemFont);
	step_text->SetFontSize(12);
	step_text->helpResIndex = kKeyEdIncrement;
	
	char double_string[50];
	sprintf(double_string, "%g", parameter->start);
	min_text->SetTextString(CtoPstr(double_string));	//  Set the minimum text
	sprintf(double_string, "%g", parameter->end);
	max_text->SetTextString(CtoPstr(double_string));	//  Set the maximum text
	sprintf(double_string, "%g", parameter->step);
	step_text->SetTextString(CtoPstr(double_string));	//  Set the step text
	
	name_text->SetTextString(CtoPstr(parameter->name));	//  Set the name
	PtoCstr((unsigned char *) parameter->name);

	gDecorator->CenterWindow (this);					//  Center the window on the screen

}	//=== CEditParameterDialog::IEditParameterDialog() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditParameterDialog::UpdateParameter
//|
//| Purpose: Sets the parameter to match the settings chosen in the dialog.
//|
//| Parameters: parameter: receives the settings chosen in the dialog
//|______________________________________________________________________________

void CEditParameterDialog::UpdateParameter(Parameter *parameter)
{

	name_text->GetTextString((unsigned char *) parameter->name);
	PtoCstr((unsigned char *) parameter->name);						//  Get the name

	char *end;
	char double_string[50];
	min_text->GetTextString(
			(unsigned char *) double_string);
	parameter->start = strtod(PtoCstr(
			(unsigned char *) double_string), &end);	//  Get the minimum
	max_text->GetTextString(
					(unsigned char *) double_string);
	parameter->end = strtod(PtoCstr(
			(unsigned char *) double_string), &end);	//  Get the maximum
	step_text->GetTextString(
			(unsigned char *) double_string);
	parameter->step = strtod(PtoCstr(
			(unsigned char *) double_string), &end);	//  Get the step
	
	if ((parameter->end > parameter->start) && (parameter->step < 0) ||
		(parameter->end < parameter->start) && (parameter->step > 0))
		parameter->step = -parameter->step;				//  Make sure step goes in the right direction
	
	if (parameter->step == 0)							//  Don't allow zero step size
		{
		
		if (parameter->end == parameter->start)
			parameter->step = 1;						//  Handle case where max == min, step == 0
		
		else
			parameter->step =
				(parameter->end - parameter->start)/10;	//  Ten steps
		
		}

}	//==== CEditParameterDialog::UpdateParameter() ====\\