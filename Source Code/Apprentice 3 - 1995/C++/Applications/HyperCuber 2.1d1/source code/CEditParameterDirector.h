//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditParameterDirector class.  The
//| CEditParameterDirector class supervises the dialog which is used to edit
//| a single parameter.
//|________________________________________________________________________________

#pragma once

#include "Parameter.h"

#include <CDialogDirector.h>

class CEditParameterDirector : public CDialogDirector
	{

	Parameter	*theParameter;					//  Pointer to the parameter being edited
	
  public:

	void	IEditParameterDirector (CDirectorOwner *aSupervisor, Parameter *parameter);
	void	TalkToUser(void);

	};
