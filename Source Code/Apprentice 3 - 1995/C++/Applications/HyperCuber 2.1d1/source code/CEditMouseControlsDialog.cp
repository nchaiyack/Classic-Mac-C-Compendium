//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditMouseControlsDialog.cp
//|
//| This implements the dialog used to edit a mouse control
//|_________________________________________________________

#include "CEditMouseControlsDialog.h"
#include "CHyperCuberPrefs.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <CBartender.h>
#include <CButton.h>
#include <CCheckBox.h>
#include <CDecorator.h>
#include <CEditText.h>
#include <CIntegerText.h>
#include <CPaneBorder.h>
#include <CPopupMenu.h>
#include <CStdPopupPane.h>

#include <ctype.h>
#include <string.h>

//============================ Globals ===============================\\

extern CDecorator 		*gDecorator;
extern CBartender 		*gBartender;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditMouseControlsDialog::IEditMouseControlsDialog
//|
//| Purpose: Initialize the dialog.
//|
//| Parameters: WindowID:      ID of WIND resource to use
//|             enclosure:     the Desktop
//|             supervisor:    the supervisor of this DialogDirector
//|             mouse_control: the mouse control to edit
//|______________________________________________________________________________

void CEditMouseControlsDialog::IEditMouseControlsDialog(short WindowID, CDesktop *enclosure,
											CDirector *supervisor, MouseControl *mouse_control)
{

	
#define OKAY_BUTTON_ID		130
#define CANCEL_BUTTON_ID	131

#define FUNCTION_HEIGHT		40
#define DIRECTION_MENU_ID	133
#define DIMENSION_MENU_ID	30000
#define ANGLE_MENU_ID		30001
#define REVERSE_CHECKBOX_ID	129

#define COMMAND_CHECKBOX_ID	136
#define OPTION_CHECKBOX_ID	137
#define SHIFT_CHECKBOX_ID	138
#define CONTROL_CHECKBOX_ID	128

	CDialog::IDialog(WindowID, enclosure, supervisor);		//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;							//  Link in the Balloon Help

	CButton		*button;
	button = new(CButton);									//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID, this, this);
	button->Offset(320, FUNCTION_HEIGHT + 40, TRUE);
	button->SetClickCmd (cmdOK);
	button->helpResIndex = kMouseEdOkay;
	SetDefaultButton(button);
	
	button = new (CButton);									//  Set up the cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(220, FUNCTION_HEIGHT + 40, TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kMouseEdCancel;
	
	CEditText *text = new(CEditText);						//  Set up the Modifiers text
	text->IEditText(this, this, 70, 16,
					0, 10, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pModifiers:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	text->SetAlignCmd(cmdAlignRight);
	
	command_checkbox = new(CCheckBox);						//  Set up the command checkbox
	command_checkbox->ICheckBox(COMMAND_CHECKBOX_ID, this, this);
	command_checkbox->Offset(80, 10, TRUE);
	command_checkbox->helpResIndex = kMouseEdCommand;

	option_checkbox = new(CCheckBox);						//  Set up the option checkbox
	option_checkbox->ICheckBox(OPTION_CHECKBOX_ID, this, this);
	option_checkbox->Offset(170, 10, TRUE);
	option_checkbox->helpResIndex = kMouseEdOption;

	shift_checkbox = new(CCheckBox);						//  Set up the shift checkbox
	shift_checkbox->ICheckBox(SHIFT_CHECKBOX_ID, this, this);
	shift_checkbox->Offset(240, 10, TRUE);
	shift_checkbox->helpResIndex = kMouseEdShift;

	control_checkbox = new(CCheckBox);						//  Set up the control checkbox
	control_checkbox->ICheckBox(CONTROL_CHECKBOX_ID, this, this);
	control_checkbox->Offset(300, 10, TRUE);
	control_checkbox->helpResIndex = kMouseEdControl;

	text = new(CEditText);									//  Set up the "Function:" text
	text->IEditText(this, this, 70, 16,
					0, FUNCTION_HEIGHT, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pFunction: Track");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	text->SetAlignCmd(cmdAlignRight);

	short cursor = 79;
	text = new(CEditText);									//  Set up the "Track [" text
	text->IEditText(this, this, 60, 16, cursor, FUNCTION_HEIGHT,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pTrack [");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	cursor += 50;

	dimension_text = new(CIntegerText);						//  Set up the dimension text
	dimension_text->IIntegerText(this, this, 40, 16, 
						cursor, FUNCTION_HEIGHT,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	dimension_text->SetFontNumber(systemFont);
	dimension_text->SetFontSize(12);
	dimension_text->SpecifyDefaultValue(3);
	dimension_text->SpecifyRange(3, MAX_DIMENSION);
	dimension_text->helpResIndex = kMouseEdDimension;
	cursor += 44;

	text = new(CEditText);									//  Set up the : text
	text->IEditText(this, this, 100, 16, cursor, FUNCTION_HEIGHT,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\p:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	cursor += 10;

	angle_text = new(CDialogText);							//  Set up the angle text
	angle_text->IDialogText(this, this, 40, 16, 
						cursor, FUNCTION_HEIGHT,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	angle_text->SetFontNumber(systemFont);
	angle_text->SetFontSize(12);
	angle_text->helpResIndex = kMouseEdAngle;
	cursor += 44;

	text = new(CEditText);									//  Set up the "]" text
	text->IEditText(this, this, 100, 16, cursor, FUNCTION_HEIGHT,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\p]");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	cursor += 20;

	direction_popup = new (CStdPopupPane);					//  Set up the direction menu
	direction_popup->IStdPopupPane(
							DIRECTION_MENU_ID, this, this,
							kAutoSize, kAutoSize, cursor, FUNCTION_HEIGHT);
	direction_popup->helpResIndex = kMouseEdDirection;

	multiplier_text = new(CIntegerText);					//  Set up the editable multiplier text
	multiplier_text->IIntegerText(this, this, 60, 16, 
						130, FUNCTION_HEIGHT+30,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	multiplier_text->SetFontNumber(systemFont);
	multiplier_text->SetFontSize(12);
	multiplier_text->SpecifyDefaultValue(1);
	multiplier_text->helpResIndex = kMouseEdMultiplier;

	text = new(CEditText);									//  Set up the x text
	text->IEditText(this, this, 10, 16,
					115, FUNCTION_HEIGHT+32, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetFontNumber(monaco);
	text->SetFontSize(9);
	text->SetTextString("\px");
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);

	direction_popup->GetMenu()->SelectItem(mouse_control->horiz + 1,
											pmForceOn);				//  Set the direction checkbox

	multiplier_text->SetIntValue(mouse_control->multiplier);		//  Set increment
	dimension_text->SetIntValue(mouse_control->dimension);			//  Set the dimension

	if (mouse_control->angle == 0)
		angle_text->SetTextString("\pP");							//  Set perspective change
	else
		{
		Str255 angle_string;
		NumToString(mouse_control->angle, angle_string);
		angle_text->SetTextString(angle_string);					//  Set the angle text
		}

	command_checkbox->SetValue(mouse_control->modifiers & cmdKey);		//  Set the command checkbox
	control_checkbox->SetValue(mouse_control->modifiers & controlKey);	//  Set the control checkbox
	option_checkbox->SetValue(mouse_control->modifiers & optionKey);	//  Set the option checkbox
	shift_checkbox->SetValue(mouse_control->modifiers & shiftKey);		//  Set the shift checkbox

	gDecorator->CenterWindow (this);								//  Center the window on the screen

}	//=== CEditMouseControlsDialog::IEditMouseControlsDialog() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditMouseControlsDialog::UpdateMouseControl
//|
//| Purpose: Sets the mouse control to match the settings chosen in the dialog.
//|
//| Parameters: mouse_control: receives the settings chosen in the dialog
//|______________________________________________________________________________

void CEditMouseControlsDialog::UpdateMouseControl(MouseControl *mouse_control)
{

	mouse_control->modifiers = 0;											//  Build the modifiers
	if (command_checkbox->GetValue()) mouse_control->modifiers |= cmdKey;
	if (option_checkbox->GetValue()) mouse_control->modifiers |= optionKey;
	if (shift_checkbox->GetValue()) mouse_control->modifiers |= shiftKey;
	if (control_checkbox->GetValue()) mouse_control->modifiers |= controlKey;

	mouse_control->horiz =
				(direction_popup->GetMenu()->GetCheckedItem() - 1);			//  Get the direction

	mouse_control->dimension = dimension_text->GetIntValue();				//  Get the dimension
	mouse_control->multiplier = multiplier_text->GetIntValue();				//  Get the multiplier

	Str255 angle_string;
	angle_text->GetTextString(angle_string);						//  Get the angle as a string
	if ((angle_string[0] == '\1') &&
			(toupper(angle_string[1]) == 'P'))
		mouse_control->angle = 0;									//  Perspective change
	else
		{
		long angle_long;
		StringToNum(angle_string, &angle_long);						//  Convert to number
		if (angle_long >= mouse_control->dimension)
			angle_long = mouse_control->dimension - 1;				//  Clip to dimension
		if (angle_long <= 0)
			angle_long = 1;											//  Clip to angle == 1
		mouse_control->angle = angle_long;
		}

}	//==== CEditMouseControlsDialog::UpdateMouse() ====\\