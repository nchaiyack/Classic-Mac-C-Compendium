//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| This file contains the interface to the CPrefsDialogDirector class.  The
//| CPrefsDialogDirector class supervises the preferences dialog.
//|________________________________________________________________________________

#pragma once
#include <CDialogDirector.h>

class CPrefsDialogDirector : public CDialogDirector
	{

  public:

	void	IPrefsDialogDirector(CDirectorOwner *supervisor);
	void	DoCommand(long command);
	void	TalkToUser(void);

	void	DoKeyDown(char the_char, Byte key_code, EventRecord *event);
	void	DoAutoKey(char the_char, Byte key_code, EventRecord *event);

	};
