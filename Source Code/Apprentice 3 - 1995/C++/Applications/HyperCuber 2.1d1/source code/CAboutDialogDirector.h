//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CAboutDialogDirector class.  The
//| CAboutDialogDirector class supervises the "About HyperCuber..." dialog.
//|________________________________________________________________________________

#pragma once
#include <CDialogDirector.h>

class CAboutDialogDirector : public CDialogDirector
	{

  public:

	void	IAboutDialogDirector(CDirectorOwner *supervisor);
	void	TalkToUser(void);

	void	DoKeyDown(char the_char, Byte key_code, EventRecord *event);
	void	DoAutoKey(char the_char, Byte key_code, EventRecord *event);

	};
