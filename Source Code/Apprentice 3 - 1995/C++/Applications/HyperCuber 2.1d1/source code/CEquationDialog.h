//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CEquationDialog class.  The
//| CEquationDialog class implements the dialog used to define equations.
//|________________________________________________________________________________

#include <CDialog.h>

class CParametersArrayPane;
class CFunctionsArrayPane;

class CEquationDialog : public CDialog
	{

  public:

	void	IEquationDialog(short WindowID, CDesktop *enclosure, CDirector *supervisor,
							CArray *parameters_array, CParametersArrayPane *parameters_array_pane,
							CArray *functions_array, CFunctionsArrayPane *functions_array_pane);

	void	UpdatePrefs(void);
	
	};
