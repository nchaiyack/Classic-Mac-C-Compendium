//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CKeyControlsDirector class.  The
//| CKeyControlsDirector class supervises the controls dialog.
//|________________________________________________________________________________

#pragma once
#include <CDialogDirector.h>

class CArray;
class CKeyControlsArrayPane;

class CKeyControlsDirector : public CDialogDirector
	{

	CArray 					*array;
	CKeyControlsArrayPane	*array_pane;

  public:

	void	IKeyControlsDirector(CDirectorOwner *supervisor);
	void	DoCommand(long command);
	void	TalkToUser(void);

	void	BuildArrayFromPrefs(void);
	
	};
