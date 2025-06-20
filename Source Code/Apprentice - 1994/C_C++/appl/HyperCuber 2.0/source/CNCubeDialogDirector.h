//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CNCubeDialogDirector class.  The
//| CNCubeDialogDirector class supervises the n-cube dialog.
//|________________________________________________________________________________

#pragma once
#include <CDialogDirector.h>

class CNCubeDialogDirector : public CDialogDirector
	{

  public:

	void	INCubeDialogDirector(CDirectorOwner *supervisor);
	long	TalkToUser(void);

	};
