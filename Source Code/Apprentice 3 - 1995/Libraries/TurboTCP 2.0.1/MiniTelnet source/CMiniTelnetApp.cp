/*
** CMiniTelnetApp.cp
**
**	MiniTelnet application
**	Application subclass
**
**	Copyright © 1993-94, FrostByte Design / Eric Scouten
**
*/


#include "CMiniTelnetApp.h"

#include "CArrowPopupPane.h"
#include "CBartender.h"
#include "CDataFile.h"
#include "CDialogText.h"
#include "CIconPane.h"
#include "CPopupMenu.h"
#include "CPopupPane.h"
#include "CRadioControl.h"
#include "CRadioGroupPane.h"
#include "CStdPopupPane.h"
#include "TBUtilities.h"
#include "TCLForceReferences.h"

//#include "CAboutDirector.h"
#include "CTelnetTerminal.h"
#include "CTelnetSettingsDLOG.h"


// global TCL objects

extern CApplication*	gApplication;
extern CBartender*		gBartender;
extern OSType			gSignature;
extern CursHandle		gWatchCursor;
extern OSType			gSignature;

TCL_DEFINE_CLASS_M1(CMiniTelnetApp, CTCPApplication);


//	ÑÑ application startup ÑÑ

/*______________________________________________________________________
**
** main()
**
**	The big one. Set up the OOP stuff and let it go.
**
*/

void main()

{
	CMiniTelnetApp* theApp = new CMiniTelnetApp;
	theApp->Run();
	theApp->Exit();
}


/*______________________________________________________________________
**
** constructor
**
**	Initialize the application.
**
*/

CMiniTelnetApp::CMiniTelnetApp()
	: CTCPApplication(kExtraMasters, kRainyDayFund, kCriticalBalance, kToolboxBalance)

{
	gSignature = kAppSignature;
}

/*______________________________________________________________________
**
** ForceClassReferences
**
**	Reference classes used by new_by_name.
**
*/

void CMiniTelnetApp::ForceClassReferences()

{
	TCL_FORCE_REFERENCE(CArrowPopupPane);
	TCL_FORCE_REFERENCE(CDialogText);
	TCL_FORCE_REFERENCE(CIconPane);
	TCL_FORCE_REFERENCE(CPopupMenu);
//	TCL_FORCE_REFERENCE(CPopupPane);
	TCL_FORCE_REFERENCE(CRadioControl);
	TCL_FORCE_REFERENCE(CRadioGroupPane);
	TCL_FORCE_REFERENCE(CStdPopupPane);
}


//	ÑÑ creation of Telnet sessions (documents) ÑÑ

/*______________________________________________________________________
**
** CreateDocument
**
**	What happens to File->New from the menu. Creates a settings dialog rather than a session
**	document. The dialog will open a session if the user so desires.
**
*/

void CMiniTelnetApp::CreateDocument()

{
	CTelnetSettingsDLOG* theDialog = NULL;
	
	TRY {
		theDialog = new CTelnetSettingsDLOG(this);
		theDialog->DefaultSettings();
		theDialog->BeginDialog();
	}
	
	CATCH {
		ForgetObject(theDialog);
	}
	ENDTRY;
}


/*______________________________________________________________________
**
** OpenDocument
**
**	The user chose OpenÉ from the File menu. Creates a new session
**	with the contents of the settings file.
**
**		macSFReply (SFReply*):	the settings file document to open
**
*/

void CMiniTelnetApp::OpenDocument(SFReply* macSFReply)

{
	TelnetSettingsRec newSettings;

	if (macSFReply->fType == kSettingsFileType) {
		OpenSettingsFile(macSFReply, &newSettings);
		NewSession(&newSettings);
	}
}


/*______________________________________________________________________
**
** OpenSettings
**
**	What happens to File->Open Settings from the menu. Creates a new settings dialog with the
**	contents of the settings file.
**
**		macSFReply (SFReply *):	the settings file document to open
**
*/

void CMiniTelnetApp::OpenSettings(SFReply* macSFReply)

{
	CTelnetSettingsDLOG*	theDialog = NULL;
	TelnetSettingsRec		newSettings;
	
	OpenSettingsFile(macSFReply, &newSettings);

	TRY {
		theDialog = new CTelnetSettingsDLOG(this);
		BlockMove(&newSettings, &(theDialog->r), sizeof(TelnetSettingsRec));
		theDialog->PutSettings();
		theDialog->BeginDialog();
	}
	
	CATCH {
		ForgetObject(theDialog);
	}
	ENDTRY;
	
}


/*______________________________________________________________________
**
** OpenSettingsFile
**
**	Read a Telnet settings document.
**
**		macSFReply (SFReply*):			the settings file document to read
**		theSettings (TelnetSettingsRec*):	where to put the settings record
**
*/

void CMiniTelnetApp::OpenSettingsFile(SFReply* macSFReply, TelnetSettingsRec* theSettings)

{
	CDataFile*	itsFile = NULL;				// donÕt need to keep the file around


	// read the file
	
	TRY {
		itsFile = new CDataFile;
		itsFile->SFSpecify(macSFReply);
		itsFile->Open(fsRdPerm);
		
		itsFile->ReadSome((Ptr) theSettings, sizeof (TelnetSettingsRec));
		itsFile->Close();
		delete itsFile;
	}
	
	CATCH {
		ForgetObject(itsFile);
	}
	ENDTRY;
}


/*______________________________________________________________________
**
** NewSession
**
**	Create a new session from the settings record given.
**
**		newSettings (TelnetSettingsRec*):	the settings record which was opened
**
*/

void CMiniTelnetApp::NewSession(TelnetSettingsRec* newSettings)

{
	CTelnetTerminal* newTerminal = new CTelnetTerminal(kTelnetPort, kTelnetRecBufferSize,
												kTelnetAutoRecSize, kTelnetAutoRecNum);
	newTerminal->NewSession(newSettings);
			// terminal session doc will dispose of itself if it fails
}


/*______________________________________________________________________
**
** SetUpFileParameters
**
**	What kind of files can we handle?
**
*/

void CMiniTelnetApp::SetUpFileParameters()

{
	CApplication::SetUpFileParameters();

	sfNumTypes = 1;
	sfFileTypes[0] = kSettingsFileType;
	gSignature = kAppSignature;
}


//	ÑÑ menu/command handling ÑÑ

/*______________________________________________________________________
**
** DoCommand
**
**	Handle application-specific commands.
**
**		theCommand (long):	the command number which was issued
**
*/

void CMiniTelnetApp::DoCommand(long theCommand)

{
	Point				corner;
	SFTypeList		fileTypes;
	SFReply			macSFReply;
/*
	CAboutDirector*	theAboutDirector = NULL;
*/

	switch (theCommand) {

/*
		// AboutÉ command

		case cmdAbout:
			theAboutDirector = new CAboutDirector(DLOGAboutBox, this);
			theAboutDirector->DoAbout(FALSE, 0);
			break;
*/		


		// open settings file

		case cmdOpenSettings:
								// canÕt use ChooseFile since subclass might
								// have specified additional file types
			fileTypes[0] = kSettingsFileType;
			FindDlogPosition('DLOG', sfGetDLOGid, &corner);
			SFPGetFile(corner, "\p", sfFileFilter, 1, fileTypes,
						sfGetDLOGHook, &macSFReply, sfGetDLOGid, sfGetDLOGFilter);

			if (macSFReply.good) {
				SetCursor(*gWatchCursor);
				OpenSettings(&macSFReply);
			}
			break;
		
		default:
			CTCPApplication::DoCommand(theCommand);
			break;
	}
}


/*______________________________________________________________________
**
** SetUpMenus
**
**	Set up menus which must be created at run time.
**
*/

void CMiniTelnetApp::SetUpMenus()

{
	CTCPApplication::SetUpMenus();
	gBartender->SetUnchecking(MENUTelnet, TRUE);
}


/*______________________________________________________________________
**
** UpdateMenus
**
**	Adjust menus on mouse-down event.
**
*/

void CMiniTelnetApp::UpdateMenus()

{
	CApplication::UpdateMenus();
	gBartender->EnableCmd(cmdOpenSettings);
}


/*______________________________________________________________________
**
** Run
**
**	Run the application. Overriden to provide splash screen.
**
*/

void CMiniTelnetApp::Run()

{
/*
	CAboutDirector*	theAboutDirector = NULL;
*/

	// show a splash screen (modified aboutÉ box), then run the app
	
/*
	theAboutDirector = new CAboutDirector(DLOGAboutBox, this);
	theAboutDirector->DoAbout(TRUE, kSplashScreenTicks);
*/

	CApplication::Run();

}