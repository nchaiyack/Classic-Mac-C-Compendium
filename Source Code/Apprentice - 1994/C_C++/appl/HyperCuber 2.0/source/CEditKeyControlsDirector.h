//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditKeyControlsDirector class.  The
//| CEditKeyControlsDirector class supervises the dialog which is used to edit
//| key controls.
//|________________________________________________________________________________

#pragma once

#include "Keys.h"

#include <CDialogDirector.h>

class CEditKeyControlsDirector : public CDialogDirector
	{

	key_control_struct *theKey;		//  The key command edited
	
  public:

	void	IEditKeyControlsDirector (CDirectorOwner *aSupervisor, key_control_struct *key);
	void	TalkToUser(void);

	};
