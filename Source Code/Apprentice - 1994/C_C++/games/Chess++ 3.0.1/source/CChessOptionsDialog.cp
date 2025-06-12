////////////
//
//	CChessOptionsDialog.cp
//
//	A subclass of CDLOGDirector that displays the Chess Options... dialog, 
//	allowing the user to change whatever in the game.
//
//	SUPERCLASS = CDLOGDirector
//	Copyright © 1993 Steven J. Bushell. All rights reserved.
//
////////////

#include "CChessOptionsDialog.h"
#include "CChessInfoPane.h"
#include <Commands.h>
#include <CRadioGroupPane.h>
#include <CRadioControl.h>
#include <CIntegerText.h>
#include <CWindow.h>
#include <CBartender.h>
#include "CBrain.h"

extern CBartender	*gBartender;
extern CBrain		*gBrain;

#define	kChessOptionsDLOGID	1030	// resource ID of the DLOG resource


/******************************************************************************
 IChessOptionsDialog
 
 	Initialize the dialog. This method also assigns help balloon resource
 	indexes for some of the panes in the window.
 	
******************************************************************************/

void CChessOptionsDialog::IChessOptionsDialog( CDirectorOwner *aSupervisor)
{
	CDLOGDirector::IDLOGDirector( kChessOptionsDLOGID, aSupervisor);
}

/******************************************************************************
 SelectDocType
 
 	Displays the modal dialog and returns a command number corresponding
 	to the user's selection.
 	
******************************************************************************/

void CChessOptionsDialog::DoChessOptionsDialog( void)
{
	long 			theCommand;
	CRadioGroupPane	*radioGroup;
	CButton			*theButton;
	CIntegerText	*theText;
	register		i;
	short			stationSetting;
	
	itsWindow->SetTitle("\pChess Options");

	for (i=kEasyGame;i<=kWayHardGame;i++)
	{
		theButton = (CButton *)itsWindow->FindViewByID(i);
		theButton->SetClickCmd(i);
	}

	//	dummy call to force class reference (otherwise CChessInfoPane will be
	//	smart-linked out of existence
	if (i == kEasyGame)	//	always false
		member(theButton,CChessInfoPane);
	
	for (i=kSoundOnBetterMoves;i<=kDoReallyCoolThings;i++)
	{
		theButton = (CButton *)itsWindow->FindViewByID(i);
		theButton->SetClickCmd(i);
	}
	
	// set initial radio group selection. Since this dialog was created
	// from a resource, we don't have a reference to the radio group pane.
	// Since we know its ID (its the same as the dialog item number), we
	// can get the reference by calling FindViewByID.
	
	radioGroup = (CRadioGroupPane*) itsWindow->FindViewByID( kRadioGroupID);

	selectedSearchDepth = gBrain->searchDepth;
	radioGroup->SetStationID( selectedSearchDepth+2);
	
	selectedSoundOnBetterMoves = gBrain->soundOnBetterMoves;
	theButton = (CButton *)itsWindow->FindViewByID(kSoundOnBetterMoves);
	theButton->SetValue(selectedSoundOnBetterMoves);

	selectedShowThoughts = gBrain->showContemplatedMoves;
	theButton = (CButton *)itsWindow->FindViewByID(kShowThoughts);
	theButton->SetValue(selectedShowThoughts);

	selectedBackPropagation = gBrain->backPropagation;
	theButton = (CButton *)itsWindow->FindViewByID(kDoReallyCoolThings);
	theButton->SetValue(selectedBackPropagation);

	selectedBackgroundTimeInterval = gBrain->backgroundTimeInterval;
	theText = (CIntegerText *)itsWindow->FindViewByID(kBackgroundTimeInterval);
	theText->SetIntValue(selectedBackgroundTimeInterval);

		// show the dialog
		
	BeginDialog();
	
		// run the dialog and return the final command.
		
	theCommand = DoModalDialog( cmdOK);
	
	if (theCommand == cmdOK)
	{
		gBrain->searchDepth = selectedSearchDepth;
		gBrain->soundOnBetterMoves = selectedSoundOnBetterMoves;
		gBrain->showContemplatedMoves = selectedShowThoughts;
		gBrain->backPropagation = selectedBackPropagation;
		selectedBackgroundTimeInterval = theText->GetIntValue();
		gBrain->backgroundTimeInterval = selectedBackgroundTimeInterval;
	}

	return;
}

void CChessOptionsDialog::DoCommand(long theCommand)

{
	Rect			aRect;
	CChessInfoPane	*infoPane = (CChessInfoPane *)itsWindow->FindViewByID(7);
	
	switch (theCommand) {
	
		case kEasyGame:
			infoPane->infoString = kEasyGame;
			selectedSearchDepth = 1;
			break;
		case kHardGame:
			infoPane->infoString = kHardGame;
			selectedSearchDepth = 2;
			break;
		case kWayHardGame:
			infoPane->infoString = kWayHardGame;
			selectedSearchDepth = 3;
			break;
		case kSoundOnBetterMoves:
			infoPane->infoString = kSoundOnBetterMoves;
			if (selectedSoundOnBetterMoves)
				selectedSoundOnBetterMoves = false;
			else
				selectedSoundOnBetterMoves = true;
			break;
		case kShowThoughts:
			infoPane->infoString = kShowThoughts;
			if (selectedShowThoughts)
				selectedShowThoughts = false;
			else
				selectedShowThoughts = true;
			break;
		case kDoReallyCoolThings:
			infoPane->infoString = kDoReallyCoolThings;
			if (selectedBackPropagation)
				selectedBackPropagation = false;
			else
				selectedBackPropagation = true;
			break;
		default:
			inherited::DoCommand(theCommand);
			break;
	}

	infoPane->Prepare();
	infoPane->Draw(&aRect);
}