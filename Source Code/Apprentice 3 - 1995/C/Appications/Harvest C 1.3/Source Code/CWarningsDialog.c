/*
	Harvest C
	Copyright 1992 Eric W. Sink.  All rights reserved.
	
	This file is part of Harvest C.
	
	Harvest C is free software; you can redistribute it and/or modify
	it under the terms of the GNU Generic Public License as published by
	the Free Software Foundation; either version 2, or (at your option)
	any later version.
	
	Harvest C is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with Harvest C; see the file COPYING.  If not, write to
	the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
	
	Harvest C is not in any way a product of the Free Software Foundation.
	Harvest C is not GNU software.
	Harvest C is not public domain.

	This file may have other copyrights which are applicable as well.

*/

/******************************************************************************
 CWarningsDialog.c

		
	SUPERCLASS = CDLOGDirector
	
	Copyright © 1991 Symantec Corporation. All rights reserved.
	

 ******************************************************************************/

#include "CWarningsDialog.h"
#include "CWarningsPane.h"
#include "CWarningsArray.h"
#include "CScrollPane.h"
#include "CDialogText.h"
#include "CDialog.h"
#include "CPaneBorder.h"
#include "CApplication.h"
#include "CHarvestApp.h"
#include "CHarvestDoc.h"
#include "CHarvestOptions.h"
#include "CDataFile.h"
#include "CSourceFile.h"
#include "Commands.h"
#include "HarvestCommands.h"
#include "AppleEvents.h"
#include "Aliases.h"
#include "stdarg.h"

#define	kDemoDlgID			1050

enum		/* window item numbers	*/
{
	kListScrollPaneItem = 1
};

extern CHarvestApp	*gApplication;
extern CHarvestDoc	*gProject;
extern tSystem gSystem;
extern CSourceFile *gCurSourceFile;
extern CWarningsDialog *gErrs;

/******************************************************************************
 IWarningsDialog
******************************************************************************/

#define maxStringLen 255

void CWarningsDialog::IWarningsDialog( CHarvestOptions *theOpts,CWarningsArray *theWarnings)
{
	CScrollPane	*scrollPane;
	CPaneBorder *listBorder;
	Rect	margin;
	Cell	aCell;
	

	CDLOGDirector::IDLOGDirector( kDemoDlgID, gApplication);
	
	itsOptions = theOpts;

	scrollPane = (CScrollPane*) itsWindow->FindViewByID( kListScrollPaneItem);
	warningOn = (CCheckBox *) itsWindow->FindViewByID( 2);
	allOn = (CRadioControl *) itsWindow->FindViewByID( 4);
	allOff = (CRadioControl *) itsWindow->FindViewByID( 5);
	indiv = (CRadioControl *) itsWindow->FindViewByID( 6);
	radioPane = (CRadioGroupPane *) itsWindow->FindViewByID( 3);

	if (scrollPane)
	{				
		itsWarningsPane = new( CWarningsPane);
		itsWarningsPane->IWarningsPane( scrollPane, itsWindow, 0, 0, 0, 0,
			sizELASTIC, sizELASTIC);
		itsWarningsPane->FitToEnclosure( TRUE, TRUE);
		itsWarningsPane->SetDrawActiveBorder( TRUE);
		itsWarningsPane->SetDblClickCmd(cmdToggleWarning);
		
		itsWarningsPane->SetID( 10);

		listBorder = new( CPaneBorder);
		listBorder->IPaneBorder( kBorderFrame);
		itsWarningsPane->SetBorder( listBorder);
				
		itsWarningsPane->SetSelectionFlags( selOnlyOne);
		
		scrollPane->InstallPanorama( itsWarningsPane);
		
		itsWarnings = theWarnings;
		itsWarningsPane->SetArray( itsWarnings, FALSE);
		
		SetCell( aCell, 0, 0);
		itsWarningsPane->SelectCell( aCell, FALSE, FALSE);
	}
	
	warningOn->SetClickCmd(cmdToggleWarning);
	allOn->SetClickCmd(cmdAllWarningsOn);
	allOff->SetClickCmd(cmdAllWarningsOff);
	indiv->SetClickCmd(cmdIndividuallySet);

	radioPane->SetStationID(6);
	
	SetupItems();
}	/* CWarningsDialog::IWarningsDialog */

/******************************************************************************
 DoCommand
******************************************************************************/

void CWarningsDialog::DoCommand( long aCmd)
{
	Cell	selectedCell;
	Str255	string;
	Boolean haveSelection;
	struct Warning w;
	
	SetCell( selectedCell, 0, 0);
	haveSelection = itsWarningsPane->GetSelect( TRUE, &selectedCell);
	
	switch (aCmd)
	{
		case cmdToggleWarning:
			itsWarnings->GetItem((Ptr) &w, selectedCell.v+1);
			itsOptions->warnings[w.num] = !itsOptions->warnings[w.num];
			if (itsOptions->warnings[w.num]) {
				warningOn->SetValue(BUTTON_ON);
			}
			else {
				warningOn->SetValue(BUTTON_OFF);
			}
			itsWarningsPane->RefreshCell(selectedCell);
			break;
		case cmdAllWarningsOn:
			itsOptions->allWarnings = TRUE;
			itsOptions->noWarnings = FALSE;
			break;
		case cmdAllWarningsOff:
			itsOptions->allWarnings = FALSE;
			itsOptions->noWarnings = TRUE;
			break;
		case cmdIndividuallySet:
			itsOptions->allWarnings = FALSE;
			itsOptions->noWarnings = FALSE;
			break;
		default: 
				inherited::DoCommand( aCmd);
				break;
	}
				
}	/* CWarningsDialog::DoCommand */

/******************************************************************************
 SetupItems
******************************************************************************/

void CWarningsDialog::SetupItems( void)
{
	Cell	selectedCell;
	Boolean haveSelection;
	CDialog	*dialog;
	struct Warning w;

	/* determine if any cells are now selected	*/
	
	SetCell( selectedCell, 0, 0);
	haveSelection = itsWarningsPane->GetSelect( TRUE, &selectedCell);
	
	dialog = (CDialog*) itsWindow;
	
	if (haveSelection) {
		itsWarnings->GetItem((Ptr) &w, selectedCell.v+1);
		if (itsOptions->warnings[w.num]) {
			warningOn->SetValue(BUTTON_ON);
		}
		else {
			warningOn->SetValue(BUTTON_OFF);
		}
	}
	else {
	}
}	/* CWarningsDialog::SetupItems */

/******************************************************************************
 ProviderChanged
******************************************************************************/

void CWarningsDialog::ProviderChanged( CCollaborator *aProvider, long reason,
										void *info)
{
	Cell	selectedCell;
	Boolean haveSelection;

	if ((aProvider == itsWarningsPane) && (reason == tableSelectionChanged))
	{
		SetupItems();
	}
	else
		inherited::ProviderChanged( aProvider, reason, info);

}	/* CWarningsDialog::ProviderChanged */

/******************************************************************************
 Dispose
******************************************************************************/

void CWarningsDialog::Dispose( void)
{
	inherited::Dispose();
}

