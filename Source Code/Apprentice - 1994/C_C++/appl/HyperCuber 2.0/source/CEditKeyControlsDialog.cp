//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditKeyControlsDialog.cp
//|
//| This implements the dialog used to edit a key control
//|_________________________________________________________

#include "CEditKeyControlsDialog.h"
#include "CModifierKeyPane.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <CBartender.h>
#include <CButton.h>
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
//| CEditKeyControlsDialog::IEditKeyControlsDialog
//|
//| Purpose: Initialize the dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this DialogDirector
//|             key:        the key to edit
//|______________________________________________________________________________

void CEditKeyControlsDialog::IEditKeyControlsDialog(short WindowID, CDesktop *enclosure,
											CDirector *supervisor, key_control_struct *key)
{

	
#define OKAY_BUTTON_ID		130
#define CANCEL_BUTTON_ID	131

#define FUNCTION_HEIGHT		40
#define DIRECTION_MENU_ID	131
#define DIMENSION_MENU_ID	30000
#define ANGLE_MENU_ID		30001

	CDialog::IDialog(WindowID, enclosure, supervisor);		//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;							//  Link in the Balloon Help

	CButton		*button;
	button = new(CButton);									//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID, this, this);
	button->Offset(320, FUNCTION_HEIGHT + 40, TRUE);
	button->SetClickCmd (cmdOK);
	button->helpResIndex = kKeyEdOkay;
	SetDefaultButton(button);
	
	button = new (CButton);									//  Set up the cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(220, FUNCTION_HEIGHT + 40, TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kKeyEdCancel;
	
	CEditText *text = new(CEditText);						//  Set up the Key text
	text->IEditText(this, this, 65, 16,
					0, 10, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pKey:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	text->SetAlignCmd(cmdAlignRight);
	
	key_pane = new(CModifierKeyPane);						//  Set up the key pane
	key_pane->IModifierKeyPane(this, this, 71, 10,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
	key_pane->SetKey(KeyF1);
	key_pane->SetModifiers(0);
	key_pane->helpResIndex = kKeyEdKey;

	text = new(CEditText);									//  Set up the Function text
	text->IEditText(this, this, 65, 16,
					0, FUNCTION_HEIGHT, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pFunction:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	text->SetAlignCmd(cmdAlignRight);

	short cursor = 70;
	direction_popup = new (CStdPopupPane);					//  Set up the direction menu
	direction_popup->IStdPopupPane(
							DIRECTION_MENU_ID, this, this,
							kAutoSize, kAutoSize, cursor, FUNCTION_HEIGHT);
	direction_popup->helpResIndex = kKeyEdDirection;
	cursor += 105;
	
	text = new(CEditText);									//  Set up the [ text
	text->IEditText(this, this, 100, 16, cursor, FUNCTION_HEIGHT,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\p[");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	cursor += 12;

	dimension_text = new(CIntegerText);						//  Set up the dimension text
	dimension_text->IIntegerText(this, this, 40, 16, 
						cursor, FUNCTION_HEIGHT,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	dimension_text->SetFontNumber(systemFont);
	dimension_text->SetFontSize(12);
	dimension_text->SpecifyDefaultValue(3);
	dimension_text->helpResIndex = kKeyEdDimension;
	cursor += 44;

	text = new(CEditText);									//  Set up the : text
	text->IEditText(this, this, 100, 16, cursor, FUNCTION_HEIGHT,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\p:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	cursor += 11;

	angle_text = new(CDialogText);							//  Set up the angle text
	angle_text->IDialogText(this, this, 40, 16, 
						cursor, FUNCTION_HEIGHT,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	angle_text->SetFontNumber(systemFont);
	angle_text->SetFontSize(12);
	angle_text->helpResIndex = kKeyEdAngle;
	cursor += 44;

	text = new(CEditText);									//  Set up the "] by" text
	text->IEditText(this, this, 100, 16, cursor, FUNCTION_HEIGHT,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\p] by");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);
	cursor += 33;

	increment_text = new(CIntegerText);						//  Set up the editable increment text
	increment_text->IIntegerText(this, this, 60, 16, 
						cursor, FUNCTION_HEIGHT,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	increment_text->SetFontNumber(systemFont);
	increment_text->SetFontSize(12);
	increment_text->SpecifyDefaultValue(1);
	increment_text->helpResIndex = kKeyEdIncrement;

	key_pane->SetKey(key->key_code);								//  Set the key
	key_pane->SetModifiers(key->modifiers);							//  Set the modifiers
	
	dimension_text->SetIntValue(key->dimension);					//  Set the dimension

	if (key->angle == 0)
		angle_text->SetTextString("\pP");							//  Set perspective change
	else
		{
		Str255 angle_string;
		NumToString(key->angle, angle_string);
		angle_text->SetTextString(angle_string);					//  Set the angle text
		}
	
	increment_text->SetIntValue(
						(key->increment >= 0) ?
							key->increment : -key->increment);		//  Set increment

	direction_popup->GetMenu()->SelectItem(
						(key->increment >= 0) ? 1 : 2, pmForceOn);	//  Set direction

	gDecorator->CenterWindow (this);						//  Center the window on the screen

}	//=== CEditKeyControlsDialog::IEditKeyControlsDialog() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CEditKeyControlsDialog::UpdateKey
//|
//| Purpose: Sets the key to match the settings chosen in the dialog.
//|
//| Parameters: key: receives the settings chosen in the dialog
//|______________________________________________________________________________

void CEditKeyControlsDialog::UpdateKey(key_control_struct *key)
{

	key->key_code = key_pane->GetKey();								//  Get the key
	key->modifiers = key_pane->GetModifiers();						//  Get the modifiers

	key->dimension = dimension_text->GetIntValue();					//  Get the dimension
	key->increment = increment_text->GetIntValue();					//  Get the increment
	if (direction_popup->GetMenu()->GetCheckedItem() == 2)
		key->increment = -key->increment;							//  Check for Decrease

	Str255 angle_string;
	angle_text->GetTextString(angle_string);						//  Get the angle as a string
	if ((angle_string[0] == '\1') &&
			(toupper(angle_string[1]) == 'P'))
		key->angle = 0;												//  Perspective change
	else
		{
		long angle_long;
		StringToNum(angle_string, &angle_long);						//  Convert to number
		if (angle_long >= key->dimension)
			angle_long = key->dimension - 1;						//  Clip to dimension
		if (angle_long <= 0)
			angle_long = 1;											//  Clip to angle == 1
		key->angle = angle_long;
		}

}	//==== CEditKeyControlsDialog::UpdateKey() ====\\