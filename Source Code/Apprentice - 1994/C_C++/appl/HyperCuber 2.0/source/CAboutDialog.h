//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CAboutDialog class.  The
//| CAboutDialog class implements the "About HyperCuber..." dialog.
//|________________________________________________________________________________


#include <CDialog.h>

class CAboutDialog : public CDialog
	{

  public:

	void	IAboutDialog(short WindowID, CDesktop *enclosure, CDirector *supervisor,
							short pict_id);
	
	};
