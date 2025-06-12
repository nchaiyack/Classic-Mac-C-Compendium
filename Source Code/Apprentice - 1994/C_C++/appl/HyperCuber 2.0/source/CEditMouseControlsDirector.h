//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditMouseControlsDirector class.  The
//| CEditMouseControlsDirector class supervises the dialog which is used to edit
//| mouse controls.
//|________________________________________________________________________________

#pragma once

#include "Mouse.h"

#include <CDialogDirector.h>

class CEditMouseControlsDirector : public CDialogDirector
	{

	mouse_control_struct *theMouseControl;		//  The mouse command edited
	
  public:

	void	IEditMouseControlsDirector (CDirectorOwner *aSupervisor,
											mouse_control_struct *mouse_control);
	void	TalkToUser(void);

	};
