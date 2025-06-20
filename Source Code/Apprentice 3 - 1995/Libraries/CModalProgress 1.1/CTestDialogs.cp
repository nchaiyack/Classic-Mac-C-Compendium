// ===========================================================================
//	CTestDialogs.cp
//
//	This file contains the definition information for the example code to
//	demonstrate the functionality of the CModalProgress class.
//
//	NOTE:  This is just an example of how to use the CModalProgress class. Use
//		   this code as a base for your own specific uses.
//
//	Copyright 1994, 1995 Alysoft Solutions. All rights reserved.
//
// ===========================================================================

#include "CTestDialogs.h"

/*
 *  TEST 1
 *	This test dialog simply shows a piece of text, a standard one state progress bar and a 
 *	stop, or abort button.
 */
 
void TestDialog1()
{
	CModalProgress			*dlg ;
	long					total = 0 ;
	short					progress = kDialogContinues ;
	long					delay ;
	short					stateSpace ;
	
	dlg = new CModalProgress ;
	if (dlg->SetupDialog(kTest1DialogResID) != noErr)
	{
		SysBeep(1) ;
		return ;
	}
	dlg->SetProgressBar(kTest1ProgressBarItem) ;

	dlg->SetCurrentState(100) ;
	dlg->SetStateSpace(300) ;
	dlg->BeginModal() ;

	while (progress == kDialogContinues)
	{
		progress = dlg->ProcessModal() ;
		Delay(1, &delay) ;
		stateSpace = dlg->SetCurrentStateValue(total++) ;
	}
	
	dlg->EndModal() ;
	delete dlg ;

}

/*
 *	TEST 2
 *	This test shows a more complex scenario, there is some changing text in the dialog which
 *	is modified at state change time, there is a progress bar and there are multiple state
 *	spaces.
 */
	
void CDialog2::SetCurrentState(float statePercent)
{
	short					itemType ;
	Handle					theItem ;
	Rect					box ;
	
	GetDialogItem(fDialog, kTest2ItemText, &itemType, &theItem, &box) ;
	
	CModalProgress::SetCurrentState(statePercent) ;

	if (statePercent == kTest2State1)
	{
		SetIText(theItem, kTest2State1String) ;
		SetStateSpace(kTest2State1StateSpace) ;
	}
		
	if (statePercent == kTest2State2)
	{
		SetIText(theItem, kTest2State2String) ;
		SetStateSpace(kTest2State2StateSpace) ;
	}
	
	if (statePercent == kTest2State3)
	{
		SetIText(theItem, kTest2State3String) ;
		SetStateSpace(kTest2State3StateSpace) ;
	}
}

void TestDialog2()
{
	CDialog2				*dlg ;
	long					total = 0 ;
	short					progress = kDialogContinues ;
	long					delay ;
	short					stateSpace = kStateSpaceWithinLimit ;
	
	dlg = new CDialog2 ;
	if (dlg->SetupDialog(kTest2DialogResID) != noErr)
	{
		SysBeep(1) ;
		return ;
	}
	dlg->SetProgressBar(kTest2ProgressBarItem) ;

	dlg->SetCurrentState(kTest2State1) ;
	dlg->BeginModal() ;

	while ((progress == kDialogContinues) && (stateSpace == kStateSpaceWithinLimit))
	{
		progress = dlg->ProcessModal() ;
		Delay(1, &delay) ;
		stateSpace = dlg->SetCurrentStateValue(total++) ;
	}
	
	if (progress == kDialogContinues)
	{
		dlg->SetCurrentState(kTest2State2) ;
		total = 0 ;
		stateSpace = kStateSpaceWithinLimit ;
		while ((progress == kDialogContinues) && (stateSpace == kStateSpaceWithinLimit))
		{
			progress = dlg->ProcessModal() ;
			Delay(1, &delay) ;
			stateSpace = dlg->SetCurrentStateValue(total++) ;
		}
		
		if (progress == kDialogContinues)
		{
			dlg->SetCurrentState(kTest2State3) ;
			total = 0 ;
			while (progress == kDialogContinues)
			{
				progress = dlg->ProcessModal() ;
				Delay(1, &delay) ;
				stateSpace = dlg->SetCurrentStateValue(total++) ;
			}
		}
	}
	
	dlg->EndModal() ;
	delete dlg ;

}

/*
 *  TEST 3
 *	A simple search type dialog with the infinite progress bar.
 *
 *  This test also makes use of the alternative event handling method. Note the
 *	use of the CanProcessEvent() and the ProcessIdle() method calls.
 */
 
void TestDialog3()
{
	CModalProgress			*dlg ;
	short					progress = kDialogContinues ;
	
	HiliteMenu(0) ;
	
	dlg = new CModalProgress ;
	if (dlg->SetupDialog(kTest3DialogResID) != noErr)
	{
		SysBeep(1) ;
		return ;
	}
	dlg->SetInfiniteBar(kTest3InfiniteBarItem) ;

	dlg->BeginModal() ;

	while (progress == kDialogContinues)
	{
		EventRecord theEvent ;
		
		if (GetNextEvent(everyEvent, &theEvent))
		{
			if (!dlg->CanProcessEvent(&theEvent, &progress))
				switch (theEvent.what)
				{
					case inMenuBar:
						MenuSelect(theEvent.where) ;
						HiliteMenu(0) ;
						break ;
				}
		}
		else
			progress = dlg->ProcessIdle() ;
	}
	
	dlg->EndModal() ;
	delete dlg ;
}

/*
 *	TEST 4
 *	This is the most detailed of all the test. It comprises of a dialog which depicts a 
 *	situation where the user is connecting to a remote server, downloading several files,
 *	setting some parameter data on the server and then disconnecting from the server.
 *	The dialog shows fields for the textual current state, the files being downloaded and
 *	a progress bar to show time left.
 */

void CDialog4::SetCurrentState(float statePercent)
{
	short					itemType ;
	Handle					theItem ;
	Rect					box ;
	
	GetDialogItem(fDialog, kTest4StateTextItem, &itemType, &theItem, &box) ;
	
	CModalProgress::SetCurrentState(statePercent) ;

	switch ((long)statePercent)
	{
		case kTest4ConnectState:
			SetIText(theItem, kTest4ConnectStateString) ;
			SetStateSpace(kTest4StandardStateSpace) ;
			break ;
			
		case kTest4LoginState:
			SetIText(theItem, kTest4LoginStateString) ;
			SetStateSpace(kTest4StandardStateSpace) ;
			break ;
			
		case kTest4DownloadState:
			SetIText(theItem, kTest4DownloadStateString) ;
			SetStateSpace(kTest4DownloadStateSpace) ;
			break ;
			
		case kTest4SetParamsState:
			SetIText(theItem, kTest4SetParamsStateString) ;
			SetStateSpace(kTest4StandardStateSpace) ;
			break ;
			
		case kTest4LogoffState:
			SetIText(theItem, kTest4LogoffStateString) ;
			SetStateSpace(kTest4StandardStateSpace) ;
			break ;
			
		case kTest4DisconnectState:
			SetIText(theItem, kTest4DisconnectStateString) ;
			SetStateSpace(kTest4StandardStateSpace) ;
			break ;
			
		case kTest4CleanupState:
			SetIText(theItem, kTest4CleanupStateString) ;
			SetStateSpace(kTest4StandardStateSpace) ;
			break ;
			
	}
}

void CDialog4::SetFileName(Str255 filename)
{
	short					itemType ;
	Handle					theItem ;
	Rect					box ;
	
	GetDialogItem(fDialog, kTest4FileNameTextItem, &itemType, &theItem, &box) ;
	SetIText(theItem, filename) ;
}

short PerformAState(CDialog4 *dlg, short whichState)
{
	short					progress = kDialogContinues ;
	short					stateSpace = kStateSpaceWithinLimit ;
	long					total = 0 ;
	
	dlg->SetCurrentState(whichState) ;

	while ((stateSpace == kStateSpaceWithinLimit) && (progress == kDialogContinues))
	{
		progress = dlg->ProcessModal() ;
		stateSpace = dlg->SetCurrentStateValue(total++) ;
	}

	return progress ;
}

void TestDialog4()
{
	CDialog4				*dlg ;
	short					progress = kDialogContinues ;
	long					total ;
	
	dlg = new CDialog4 ;
	if (dlg->SetupDialog(kTest4DialogResID) != noErr)
	{
		SysBeep(1) ;
		return ;
	}
	
	dlg->SetProgressBar(kTest4ProgressBarItem) ;
	dlg->SetPercentText(kTest4PercentTextItem, 5) ;
	
	dlg->SetFileName(kTest4DownloadFileNone) ;
	
	dlg->BeginModal() ;
	
	/* Process the connect state */
	
	if (PerformAState(dlg, kTest4ConnectState) != kDialogContinues)
		goto ALL_DONE ;
	
	/* Process the login state */
	
	if (PerformAState(dlg, kTest4LoginState) != kDialogContinues)
		goto ALL_DONE ;
	
	/* Perform the file downloads */
	
	dlg->SetCurrentState(kTest4DownloadState) ;
	dlg->SetFileName(kTest4DownloadFile1) ;
	
	total = 0 ;
	while ((progress == kDialogContinues) && (total < kTest4File1StateSpace))
	{
		progress = dlg->ProcessModal() ;
		dlg->SetCurrentStateValue(total++) ;
	}
	if (progress != kDialogContinues)
		goto ALL_DONE ;
	
	/* File 2 */
	
	dlg->SetFileName(kTest4DownloadFile2) ;
	
	while ((progress == kDialogContinues) && (total < (kTest4File2StateSpace + kTest4File1StateSpace)))
	{
		progress = dlg->ProcessModal() ;
		dlg->SetCurrentStateValue(total++) ;
	}
	if (progress != kDialogContinues)
		goto ALL_DONE ;

	/* File 3 */
	
	dlg->SetFileName(kTest4DownloadFile3) ;
	
	while ((progress == kDialogContinues) && (total < (kTest4File2StateSpace + kTest4File1StateSpace + kTest4File3StateSpace)))
	{
		progress = dlg->ProcessModal() ;
		dlg->SetCurrentStateValue(total++) ;
	}
	if (progress != kDialogContinues)
		goto ALL_DONE ;

	dlg->SetFileName(kTest4DownloadFileNone) ;

	/* Perfrom the Parameter setting */

	if (PerformAState(dlg, kTest4SetParamsState) != kDialogContinues)
		goto ALL_DONE ;
	
	/* Perfrom the Logoff state */

	if (PerformAState(dlg, kTest4LogoffState) != kDialogContinues)
		goto ALL_DONE ;

	/* Perfrom the disconnect state */

	if (PerformAState(dlg, kTest4DisconnectState) != kDialogContinues)
		goto ALL_DONE ;
	
	/* Perfrom the cleanup state */

	if (PerformAState(dlg, kTest4CleanupState) != kDialogContinues)
		goto ALL_DONE ;

ALL_DONE:
	dlg->EndModal() ;
	delete dlg ;
}
