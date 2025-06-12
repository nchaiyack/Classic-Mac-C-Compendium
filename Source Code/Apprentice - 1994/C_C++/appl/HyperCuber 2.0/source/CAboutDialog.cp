//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CAboutDialog.cp
//|
//| This implements the about... dialog
//|_________________________________________________________

#include "CAboutDialog.h"

#include <CButton.h>
#include <CDecorator.h>
#include <CEditText.h>
#include <CPicture.h>

extern CDecorator *gDecorator;

//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CAboutDialog::IAboutDialog
//|
//| Purpose: Initialize the about dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this DialogDirector
//|             low_memory: TRUE if memory is low, and the PICT should not be
//|                         displayed.
//|______________________________________________________________________________
void CAboutDialog::IAboutDialog(short WindowID, CDesktop *enclosure,
									CDirector *supervisor, short pict_id)
{

#define OKAY_BUTTON_ID		130

#define LOW_MEM_PICT_ID		129

	CPicture	*picture = NULL;
	CEditText	*text = NULL;
	CButton		*button = NULL;

	TRY
	{
	
	CDialog::IDialog(WindowID, enclosure, supervisor);	//  Initialize as CDialog

	if (pict_id)
		{

		CPicture *picture = new(CPicture);				//  Set up the picture
		picture->IPicture(this, this, 500, 400, 10, 10,
							sizFIXEDLEFT, sizFIXEDTOP);
		picture->UsePICT(pict_id);
		picture->SetScaled(FALSE);
		picture->FrameToBounds();
		picture->SetWantsClicks(TRUE);
		picture->CenterWithinEnclosure(TRUE, TRUE);

		CEditText *text = new(CEditText);				//  Set up the version number text
		text->IEditText(this, this, 100, 20,
						307 - 30*(pict_id == LOW_MEM_PICT_ID),
						127,
						sizFIXEDLEFT, sizFIXEDTOP, -1);
		text->SetTextString("\pVersion 2.0");
		text->SetFontNumber(geneva);
		text->SetFontSize(9);
		text->SetWantsClicks(FALSE);
		text->Specify(kNotEditable, kNotSelectable,
						kNotStylable);
		}
	
	else
		SysBeep(0);										//    don't draw anything in the window
	
	CButton *button = new(CButton);						//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID,
							this, this);
	button->Offset(420, 250, TRUE);
	button->SetClickCmd (cmdOK);
	SetDefaultButton(button);
	
	gDecorator->CenterWindow (this);					//  Center the window on the screen

	Select();
	Update();
	
	}
	
	CATCH
	{
	
	ForgetObject(picture);								//  clean up
	ForgetObject(button);
	ForgetObject(text);

	}
	ENDTRY;

	
}	//=== CPrefsDialog::IPrefsDialog() ===\\