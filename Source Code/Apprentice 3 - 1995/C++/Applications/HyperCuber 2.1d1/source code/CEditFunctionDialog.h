//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEditFunctionDialog class.  The
//| CEditFunctionDialog class implements the dialog used to edit a single
//| parametric function.
//|________________________________________________________________________________

#include <CDialog.h>

class CDialogText;

class CEditFunctionDialog : public CDialog
	{

	CDialogText			*function_text;			//  The field containing the function
	
  public:

	void	IEditFunctionDialog(short WindowID, CDesktop *enclosure,
											CDirector *supervisor, long variable_num,
											char *function);
	
	void	UpdateFunction(char *the_function);
	
	};
