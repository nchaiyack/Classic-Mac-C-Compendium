//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditFunctionDirector class.  The
//| CEditFunctionDirector class supervises the dialog which is used to edit
//| a single parametric function.
//|________________________________________________________________________________

#pragma once

#include "Parameter.h"

#include <CDialogDirector.h>

class CEditFunctionDirector : public CDialogDirector
	{

	char	*theFunction;					//  Pointer to the function being edited
	
  public:

	void	IEditFunctionDirector (CDirectorOwner *aSupervisor,
									long variable_num, char *the_function);
	void	TalkToUser(void);

	};
