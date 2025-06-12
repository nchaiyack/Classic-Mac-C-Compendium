//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CKeyControlsDialog class.  The
//| CKeyControlsDialog class implements the controls dialog.
//|________________________________________________________________________________


#include <CDialog.h>

CLASS CArray;
CLASS CKeyControlsArrayPane;

class CKeyControlsDialog : public CDialog
	{

  public:

	void	IKeyControlsDialog(short WindowID, CDesktop *enclosure,
								CDirector *supervisor, CArray *array,
									CKeyControlsArrayPane *array_pane);
	
	void	MatchDialogToPrefs(void);
	
	};
