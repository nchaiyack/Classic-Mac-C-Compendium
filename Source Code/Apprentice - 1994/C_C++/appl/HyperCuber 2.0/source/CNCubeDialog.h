//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CNCubeDialog class.  The
//| CNCubeDialog class implements the n-cube dialog.
//|________________________________________________________________________________


#include <CDialog.h>

CLASS CIntegerText;

class CNCubeDialog : public CDialog
	{


  public:

	CIntegerText	*dimension;	

	void INCubeDialog(short WindowID, CDesktop *enclosure, CDirector *supervisor);
	
	};
