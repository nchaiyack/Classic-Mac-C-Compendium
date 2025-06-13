//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditParameterDialog class.  The
//| CEditParameterDialog class implements the dialog used to edit a single parameter.
//|________________________________________________________________________________

#include "Parameter.h"

#include <CDialog.h>

class CDialogText;
class CNumberText;

class CEditParameterDialog : public CDialog
	{

	CDialogText			*name_text;				//  The field containing the name of the parameter
	CNumberText			*min_text;				//  The field containing the minimum value
	CNumberText			*max_text;				//  The field containing the maximum value
	CNumberText			*step_text;				//  The field containing the increment
	
  public:

	void	IEditParameterDialog(short WindowID, CDesktop *enclosure,
								CDirector *supervisor, Parameter *parameter);
	
	void	UpdateParameter(Parameter *parameter);
	
	};
