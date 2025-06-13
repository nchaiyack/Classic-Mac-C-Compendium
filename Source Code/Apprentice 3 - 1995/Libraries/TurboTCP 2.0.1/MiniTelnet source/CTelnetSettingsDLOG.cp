/*
** CTelnetSettingsDLOG.cp
**
**	MiniTelnet application
**	Telnet settings dialog director
**
**	Copyright © 1993-94, FrostByte Design / Eric Scouten
**
*/


#include "CTelnetSettingsDLOG.h"

#include "CApplication.h"
#include "CBartender.h"
#include "CDialog.h"
#include "CButton.h"
#include "CCheckBox.h"
#include "CDataFile.h"
#include "CEditText.h"
#include "CPaneBorder.h"
#include "CRadioGroupPane.h"
#include "Commands.h"
#include "TBUtilities.h"
#include "TCLUtilities.h"

#include "CMiniTelnetApp.h"
#include "CTelnetTerminal.h"


// item numbers in dialog box

#define itemOK			1
#define itemCancel		2
#define itemSave		3
#define itemHostName	4
#define itemBSPane		6
#define itemBS_BS		7
#define itemBS_DEL		8
#define itemGoAway		9
#define itemTitle		10
#define itemShowDebug	14

extern CApplication*	gApplication;
extern CBartender*		gBartender;
extern OSType			gSignature;
 

//	ÑÑÊconstructor ÑÑ

/*______________________________________________________________________
**
** constructor
**
**	Initialize the settings dialog.
**
**		aSupervisor (CDirectorOwner*):	the supervisor for the dialog
**
*/

CTelnetSettingsDLOG::CTelnetSettingsDLOG (CDirectorOwner* aSupervisor)
	: CDLOGDirector(DLOGSettings, aSupervisor)

{
	CButton*		theButton;
	CPane*		thePane;
	CPaneBorder*	theBorder;
	CView*		theView;


	// find important buttons & configure them

	theView = itsWindow->FindViewByID(itemOK);
	theButton = TCL_DYNAMIC_CAST(CButton, theView);
	if (theButton) {
		theButton->SetClickCmd(cmdOK);
		((CDialog*) itsWindow)->SetDefaultButton(theButton);
	}

	theView = itsWindow->FindViewByID(itemCancel);
	theButton = TCL_DYNAMIC_CAST(CButton, theView);
	if (theButton)
		theButton->SetClickCmd(cmdCancel);

	theView = itsWindow->FindViewByID(itemSave);
	theButton = TCL_DYNAMIC_CAST(CButton, theView);
	if (theButton)
		theButton->SetClickCmd(cmdSaveSettings);


	// add a border for the dialog title

	theView = itsWindow->FindViewByID(itemTitle);
	thePane = TCL_DYNAMIC_CAST(CPane, theView);
	if (thePane) {
		theBorder = new CPaneBorder(kBorderBottom);
		thePane->SetBorder(theBorder);
	}


	// get rid of that ugly border around the radio buttons

	theView = itsWindow->FindViewByID(itemBSPane);
	thePane = TCL_DYNAMIC_CAST(CPane, theView);
	if (thePane) {
		theBorder = thePane->GetBorder();
		if (theBorder)
			theBorder->SetBorderFlags(kBorderNone);
	}
	
}


//	ÑÑ settings record functions ÑÑ

/*______________________________________________________________________
**
** DefaultSettings
**
**	Creates a new blank settings record and copies this to the dialog.
**
*/

void CTelnetSettingsDLOG::DefaultSettings()

{
	r.signature = kTSRsignature;
	r.settingsVersion = kTSRversion;
	r.settingsMinVersion = 0;
	r.hostName[0] = '\0';
	BlockMove("UNKNOWN", &r.termEmulation, 8);
//	r.backspaceChar = charBS;
	r.backspaceChar = charDEL;
	r.closeOnSessionEnd = FALSE;
	r.showDebug = FALSE;
	
	PutSettings();
}


/*______________________________________________________________________
**
** GrabSettings
**
**	Get the settings from the dialog box fields and put them into the
**	settings record.
**
*/

void CTelnetSettingsDLOG::GrabSettings()

{
	CCheckBox*		theCheckBox;
	CEditText*		theText;
	CRadioGroupPane*	theRadios;
	CView*			theView;
	short	  		textSize;
	short			i;
	char*			p;


	// blank out the settings record
	
	p = (char*) &r;
	for (i=0; i<sizeof(TelnetSettingsRec); i++)
		*p++ = '\0';


	// recreate signature, version info

	r.signature = kTSRsignature;
	r.settingsVersion = kTSRversion;
	r.settingsMinVersion = 0;


	// get host name

	theView = itsWindow->FindViewByID(itemHostName);
	theText = TCL_DYNAMIC_CAST(CEditText, theView);
	if (theText) {
		textSize = theText->GetLength();
		textSize = textSize > 255 ? 255 : textSize;
		BlockMove(*(theText->GetTextHandle()), &r.hostName, textSize);
		r.hostName[textSize] = '\0';
	}


	// get terminal emulation

	BlockMove("UNKNOWN", &r.termEmulation, 8);


	// get backspace/del configuration

	theView = itsWindow->FindViewByID(itemBSPane);
	theRadios = TCL_DYNAMIC_CAST(CRadioGroupPane, theView);
	if (theRadios)
		r.backspaceChar = (theRadios->GetStationID() == itemBS_DEL ? charDEL : charBS);


	// get window go away config

	theView = itsWindow->FindViewByID(itemGoAway);
	theCheckBox = TCL_DYNAMIC_CAST(CCheckBox, theView);
	if (theCheckBox)
		r.closeOnSessionEnd = theCheckBox->IsChecked();


	// get show debug codes config

	theView = itsWindow->FindViewByID(itemShowDebug);
	theCheckBox = TCL_DYNAMIC_CAST(CCheckBox, theView);
	if (theCheckBox)
		r.showDebug = theCheckBox->IsChecked();

}


/*______________________________________________________________________
**
** PutSettings
**
**	Copy the settings from the settings record to the dialog box fields.
**
*/

void CTelnetSettingsDLOG::PutSettings()

{
	CCheckBox*		theCheckBox;
	CEditText*		theText;
	CPane*			thePane;
	CRadioGroupPane*	theRadios;
	CView*			theView;


	// get host name

	theView = itsWindow->FindViewByID(itemHostName);
	theText = TCL_DYNAMIC_CAST(CEditText, theView);
	if (theText)
		theText->SetTextPtr((char*) &r.hostName, cstrlen(r.hostName));


	// get backspace/del configuration

	theView = itsWindow->FindViewByID(itemBSPane);
	theRadios = TCL_DYNAMIC_CAST(CRadioGroupPane, theView);
	if (theRadios)
		theRadios->SetStationID((r.backspaceChar == charDEL) ? itemBS_DEL : itemBS_BS);


	// get window go away config

	theView = itsWindow->FindViewByID(itemGoAway);
	theCheckBox = TCL_DYNAMIC_CAST(CCheckBox, theView);
	if (theCheckBox)
		theCheckBox->SetValue(r.closeOnSessionEnd);


	// get show debug codes config

	theView = itsWindow->FindViewByID(itemShowDebug);
	theCheckBox = TCL_DYNAMIC_CAST(CCheckBox, theView);
	if (theCheckBox)
		theCheckBox->SetValue(r.showDebug);


	// ignore other parms

	// force redraw

	thePane = TCL_DYNAMIC_CAST(CPane, itsWindow);
	if (thePane)
		thePane->Refresh();

}


//	ÑÑ dialog handling functions ÑÑ

/*______________________________________________________________________
**
** DoCommand
**
**	Handle all commands that the dialog can understand.
**
**		theCommand (long):	the command number which was issued
**
*/

void CTelnetSettingsDLOG::DoCommand(long theCommand)

{
	CButton*			cancelBtn;
	TelnetSettingsRec	rTemp;


	// what command did we get?

	switch (theCommand) {


		// OK, Cancel buttons: respond

		case cmdOK:
			if (EndDialog(cmdOK, TRUE)) {
				GrabSettings();
				BlockMove(&r, &rTemp, sizeof(TelnetSettingsRec));
				if (TCL_DYNAMIC_CAST(CMiniTelnetApp, gApplication))
					((CMiniTelnetApp*) gApplication)->NewSession(&rTemp);
			}
			break;
			
		case cmdCancel:
			EndDialog(cmdCancel, FALSE);
			break;


		// Save Settings: do it

		case cmdSave:
		case cmdSaveAs:
		case cmdSaveSettings:
			GrabSettings();
			DoSaveFile();
			break;


		// File->Close: cancel out of here

		case cmdClose:		
			cancelBtn = ((CDialog*) itsWindow)->FindButton(cmdCancel);
			if (cancelBtn)
				cancelBtn->SimulateClick();
			EndDialog(cmdCancel, FALSE);
			break;


		// not ours, send along the chain

		default:
			CDialogDirector::DoCommand(theCommand);
	}


	// if closing window, use a disposer chore

	if ((dismissCmd != cmdNull) && (theCommand != cmdClose))
		delete this;

}


/*______________________________________________________________________
**
** UpdateMenus
**
**	Enable Telnet-specific commands.
**
*/

void CTelnetSettingsDLOG::UpdateMenus()

{
	CDLOGDirector::UpdateMenus();
	gBartender->EnableCmd(cmdSave);
	gBartender->EnableCmd(cmdSaveAs);
}


//	ÑÑ file interactions ÑÑ

/*______________________________________________________________________
**
** DoSaveFile
**
**	Save a settings record to a file.
**
*/

void CTelnetSettingsDLOG::DoSaveFile()

{
	Point			corner;				// top left corner of dialog box
	Str255		origName;				// default name for file
	short		nameLength;			// length of default name
	StringHandle	prompt;				// prompt string
	Boolean		wasLocked;
	
	CDataFile*	itsFile;				// donÕt keep the file around
	SFReply		macSFReply;			// reply from Std File


	// ask user for settings file name

	FindDlogPosition('DLOG', putDlgID, &corner);
	BlockMove(&r.hostName, &origName[1], 31);
	nameLength = cstrlen(r.hostName);
	origName[0] = (nameLength > 31 ? 31 : nameLength);
	
	prompt = GetString(STR_SettingsPrompt);
	FailNILRes(prompt);
	
	MoveHHi((Handle) prompt);
	HLock((Handle) prompt);
	SFPPutFile(corner, *prompt, origName, NULL, &macSFReply, putDlgID, NULL);
	ReleaseResource((Handle) prompt);


	// create the file

	TRY {
		itsFile = new CDataFile;
		itsFile->SFSpecify(&macSFReply);

		if (itsFile->ExistsOnDisk()) {
			itsFile->Open(fsRdWrPerm);
			itsFile->SetLength(0);
				// TEMPORARY: need to reset file type
		}
		else {
			itsFile->CreateNew(gSignature, kSettingsFileType);
			itsFile->Open(fsRdWrPerm);
		}
		itsFile->WriteSome((Ptr) &r, sizeof(TelnetSettingsRec));
		itsFile->Close();
		delete itsFile;
	}
	
	CATCH {
		ForgetObject(itsFile);
	}
	ENDTRY;

}
