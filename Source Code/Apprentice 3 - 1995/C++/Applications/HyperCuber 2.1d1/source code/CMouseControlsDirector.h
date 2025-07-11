//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CMouseControlsDirector class.  The
//| CMouseControlsDirector class supervises the mouse controls dialog.
//|________________________________________________________________________________

#pragma once
#include <CDialogDirector.h>

class CArray;
class CMouseControlsArrayPane;

class CMouseControlsDirector : public CDialogDirector
	{

	CArray 					*array;
	CMouseControlsArrayPane	*array_pane;

  public:

	void	IMouseControlsDirector(CDirectorOwner *supervisor);
	void	DoCommand(long command);
	void	TalkToUser(void);

	void	BuildArrayFromPrefs(void);

	};
