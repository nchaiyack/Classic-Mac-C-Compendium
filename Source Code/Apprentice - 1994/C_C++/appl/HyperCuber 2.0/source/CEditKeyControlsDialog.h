//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditKeyControlsDialog class.  The
//| CEditKeyControlsDialog class implements the dialog used to edit key controls.
//|________________________________________________________________________________

#include "Keys.h"

#include <CDialog.h>

CLASS CStdPopupPane;
CLASS CModifierKeyPane;
CLASS CIntegerText;
CLASS CDialogText;

class CEditKeyControlsDialog : public CDialog
	{

	CModifierKeyPane	*key_pane;			//  Pane containing the key and its modifiers

	CIntegerText		*dimension_text;	//  The field containing the dimension
	CDialogText			*angle_text;		//  The field containing the angle
	CIntegerText		*increment_text;	//  The field containing the increment

	CStdPopupPane		*direction_popup;	//  Popup menu indicating the direction of change
	
  public:

	void	IEditKeyControlsDialog(short WindowID, CDesktop *enclosure,
								CDirector *supervisor, key_control_struct *key);
	
	void	UpdateKey(key_control_struct *key);
	
	};
