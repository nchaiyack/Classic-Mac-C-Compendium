//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CPrefsDialog class.  The
//| CPrefsDialog class implements the preferences dialog.
//|________________________________________________________________________________


#include <CDialog.h>
#include "CHyperCuberPrefs.h"
#include "CColorPane.h"

class CHyperCuberPane;
class CHyperCuberDoc;

class CPrefsDialog : public CDialog
	{


  public:

	CColorPane	*background_color_pane;	
	CColorPane	*left_eye_color_pane;
	CColorPane	*right_eye_color_pane;

	void IPrefsDialog(short WindowID, CDesktop *enclosure, CDirector *supervisor);
	void InstallColorEntry(CColorPane **color_pane, short vert, char *color_name,
										RGBColor *color, short help_res);
	
	};
