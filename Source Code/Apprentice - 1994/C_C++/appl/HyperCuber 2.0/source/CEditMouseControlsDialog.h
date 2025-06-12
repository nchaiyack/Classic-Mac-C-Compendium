//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditMouseControlsDialog class.  The
//| CEditMouseControlsDialog class implements the dialog used to edit mouse controls.
//|________________________________________________________________________________

#include "Mouse.h"

#include <CDialog.h>

CLASS CStdPopupPane;
CLASS CCheckBox;
CLASS CIntegerText;
CLASS CDialogText;

class CEditMouseControlsDialog : public CDialog
	{

	CStdPopupPane		*direction_popup;	//  Popup menu; the direction to track the mouse

	CCheckBox			*command_checkbox;	//  Checkbox indicating use of command key
	CCheckBox			*option_checkbox;	//  Checkbox indicating use of option key
	CCheckBox			*shift_checkbox;	//  Checkbox indicating use of shift key
	CCheckBox			*control_checkbox;	//  Checkbox indicating use of control key

	CIntegerText		*multiplier_text;	//  The field containing the multiplier
	CIntegerText		*dimension_text;	//  The dimension of angle to change
	CDialogText			*angle_text;		//  The angle to change

  public:

	void	IEditMouseControlsDialog(short WindowID, CDesktop *enclosure,
								CDirector *supervisor, mouse_control_struct *mouse_control);
	
	void	UpdateMouseControl(mouse_control_struct *mouse_control);
	
	};
