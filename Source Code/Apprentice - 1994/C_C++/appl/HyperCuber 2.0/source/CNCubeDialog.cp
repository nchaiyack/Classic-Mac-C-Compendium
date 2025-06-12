//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNCubeDialog.cp
//|
//| This implements the n-cube dialog
//|_________________________________________________________

#include "CNCubeDialog.h"
#include "CHyperCuberDoc.h"
#include "CNCubeDialogDirector.h"

#include "HyperCuber Balloons.h"
#include "HyperCuber Commands.h"

#include <CButton.h>
#include <CDecorator.h>
#include <CEditText.h>
#include <CIntegerText.h>
#include <CPaneBorder.h>

#include <string.h>

extern CDecorator 		*gDecorator;



//|~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//| CNCubeDialog::INCubeDialog
//|
//| Purpose: Initialize the n-cube dialog.
//|
//| Parameters: WindowID:   ID of WIND resource to use
//|             enclosure:  the Desktop
//|             supervisor: the supervisor of this dialog
//|______________________________________________________________________________

void CNCubeDialog::INCubeDialog(short WindowID, CDesktop *enclosure, CDirector *supervisor)
{

	CButton		*button;
	
#define OKAY_BUTTON_ID		130
#define CANCEL_BUTTON_ID	131

	CDialog::IDialog(WindowID, enclosure, supervisor);	//  Initialize window from WIND resource

	helpResID = WINDOW_HELP_RES;					//  Link in the Balloon Help

	button = new(CButton);							//  Set up the okay button
	button->IButton(OKAY_BUTTON_ID, this, this);
	button->Offset(200, 50, TRUE);
	button->SetClickCmd (cmdOK);
	button->helpResIndex = kNCubeOkay;
	SetDefaultButton(button);
	
	button = new (CButton);							//  Set up the cancel button
	button->IButton(CANCEL_BUTTON_ID, this, this);
	button->Offset(100, 50, TRUE);
	button->SetClickCmd (cmdCancel);
	button->helpResIndex = kNCubeCancel;

	CEditText *text = new(CEditText);					//  Set up the prompt text
	text->IEditText(this, this, 200, 16,
					10, 10, sizFIXEDLEFT, sizFIXEDTOP, -1);
	text->SetTextString("\pDimension of new n-cube:");
	text->SetFontNumber(systemFont);
	text->SetFontSize(12);
	text->SetWantsClicks(FALSE);
	text->Specify(kNotEditable, kNotSelectable, kNotStylable);

	dimension = new(CIntegerText);						//  Set up the dimension text
	dimension->IIntegerText(this, this, 40, 16, 
						200, 10,
							sizFIXEDLEFT, sizFIXEDTOP, -1);
	dimension->SetFontNumber(systemFont);
	dimension->SetFontSize(12);
	dimension->SpecifyRange(3, 14);
	dimension->helpResIndex = kNCubeDimension;

	gDecorator->CenterWindow(this);						//  Center the window on the screen

}	//=== CNCubeDialog::INCubeDialog() ===\\



