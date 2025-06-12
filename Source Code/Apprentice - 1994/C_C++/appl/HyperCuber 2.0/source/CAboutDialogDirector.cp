//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CAboutDialogDirector.cp
//|
//| This implements the "About HyperCuber" dialog
//|_________________________________________________________

#include "CAboutDialog.h"
#include "CAboutDialogDirector.h"
#include "CHyperCuberDoc.h"

extern CDesktop *gDesktop;


//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CAboutDialogDirector::IAboutDialogDirector
//|
//| Purpose: Initialize the preferences dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CAboutDialogDirector::IAboutDialogDirector (CDirectorOwner *aSupervisor)
{

#define ABOUT_WINDOW_ID	129
#define PICT_ID				128
#define LOW_MEM_PICT_ID		129

	Boolean failed = FALSE;

	CDialogDirector::IDialogDirector (aSupervisor);				//  Init superclass

	CAboutDialog *dialog;

	TRY
	{
	dialog = new (CAboutDialog);
	dialog->IAboutDialog (ABOUT_WINDOW_ID, gDesktop, this, PICT_ID);
	itsWindow = dialog;
	}
	
	CATCH
	{
	ForgetObject(dialog);
	failed = TRUE;
	NO_PROPAGATE;
	}
	ENDTRY;
	
	if (!failed) return;
	
	failed = FALSE;

	TRY
	{
	dialog = new(CAboutDialog);
	dialog->IAboutDialog (ABOUT_WINDOW_ID, gDesktop, this, LOW_MEM_PICT_ID);
	itsWindow = dialog;
	}
	
	CATCH
	{
	ForgetObject(dialog);
	failed = TRUE;
	NO_PROPAGATE;
	}
	ENDTRY;
	
	if (!failed) return;

	TRY
	{
	dialog = new(CAboutDialog);
	dialog->IAboutDialog (ABOUT_WINDOW_ID, gDesktop, this, 0);
	itsWindow = dialog;
	}
	
	CATCH
	{
	ForgetObject(dialog);
	}
	ENDTRY;
	
}	//=== CAboutDialogDirector::IAboutDialogDirector() ===\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CAboutDialogDirector::DoKeyDown / CAboutDialogDirector::DoAutoKey
//|
//| Purpose: These procedure handles a key down event.  Their sole purpose is to
//|          prevent the keydown from getting to the Controls Director.
//|
//| Parameters: ignored
//|______________________________________________________________________________

void	CAboutDialogDirector::DoKeyDown(char the_char, Byte key_code, EventRecord *event)
{

}	//==== CAboutDialogDirector::DoKeyDown() ====\\


void	CAboutDialogDirector::DoAutoKey(char the_char, Byte key_code, EventRecord *event)
{

}	//==== CAboutDialogDirector::DoAutoKey() ====\\



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CAboutDialogDirector::TalkToUser
//|
//| Purpose: Handle commands for the dialog.
//|
//| Parameters: none
//|_________________________________________________________

void CAboutDialogDirector::TalkToUser(void)
{

	long dismiss_command;
	
	BeginModalDialog();
	long dismiss = DoModalDialog(cmdOK);
	long memory = MaxBlock();
	
}	//=== CAboutDialogDirector::TalkToUser ===\\
