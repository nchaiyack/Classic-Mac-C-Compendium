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

	MouseControl *theMouseControl;		//  The mouse command edited
	
  public:

	void	IEditMouseControlsDirector (CDirectorOwner *aSupervisor,
											MouseControl *mouse_control);
	void	TalkToUser(void);

	};
