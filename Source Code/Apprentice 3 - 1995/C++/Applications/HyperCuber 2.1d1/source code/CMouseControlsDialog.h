//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CMouseControlsDialog class.  The
//| CMouseControlsDialog class implements the mouse controls dialog.
//|________________________________________________________________________________


#include <CDialog.h>

class CArray;
class CMouseControlsArrayPane;

class CMouseControlsDialog : public CDialog
	{

  public:

	void	IMouseControlsDialog(short WindowID, CDesktop *enclosure,
								CDirector *supervisor, CArray *array,
									CMouseControlsArrayPane *array_pane);
	
	void	MatchDialogToPrefs(void);
	
	};
