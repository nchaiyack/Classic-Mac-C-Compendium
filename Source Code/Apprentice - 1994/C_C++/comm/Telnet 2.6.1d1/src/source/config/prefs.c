/****************************************************************
*	NCSA Telnet for the Macintosh								*
*																*
*	National Center for Supercomputing Applications				*
*	Software Development Group									*
*	152 Computing Applications Building							*
*	605 E. Springfield Ave.										*
*	Champaign, IL  61820										*
*																*
*	Copyright (c) 1993,											*
*	Board of Trustees of the University of Illinois				*
*****************************************************************
*	Revisions:
*	5/93	Telnet 2.6:	Rewritten entirely -- Jim Browne
*/

#ifdef MPW
#pragma segment Configure
#endif

#include "TelnetHeader.h"
#include "debug.h"
#include "telneterrors.h"
#include "prefs.proto.h"
#include "mainseg.proto.h"		// For quit proto

FTPServerPrefs*		gFTPServerPrefs=NULL;
ApplicationPrefs*	gApplicationPrefs=NULL;

OSErr OpenPreferencesFile( void)
{
	Str255	**PreferencesFileNameHdl;
	OSErr 	err;
	
	PreferencesFileNameHdl = (Str255**)GetString(PREFERENCES_FILE_NAME_STR_ID);
	if (PreferencesFileNameHdl == NULL) {
		DebugStr("\pSomeone has deleted the prefs file name resource!");
		forcequit();
		}
	
	HLock((Handle)PreferencesFileNameHdl);
		
	/* look for an existing settings file. */
	/* first, try the application folder */
	TelInfo->SettingsFile=HOpenResFile(TelInfo->ApFolder.vRefNum, TelInfo->ApFolder.parID,
											(StringPtr)*PreferencesFileNameHdl,fsRdWrShPerm);

	if (TelInfo->SettingsFile == -1) /* Look in System Folder */
		{
		TelInfo->SettingsFile = HOpenResFile(TelInfo->SysFolder.vRefNum, 
												TelInfo->SysFolder.parID, 
											(StringPtr)*PreferencesFileNameHdl, fsRdWrShPerm);
		}

	if (TelInfo->SettingsFile == -1) /* Look in Preferences Folder */
		{
		TelInfo->SettingsFile = HOpenResFile(TelInfo->PrefFolder.vRefNum, 
												TelInfo->PrefFolder.parID,
											(StringPtr)*PreferencesFileNameHdl, fsRdWrShPerm);
		}

	if (TelInfo->SettingsFile == -1)
	  	{
		if (err = HCreate(TelInfo->PrefFolder.vRefNum, TelInfo->PrefFolder.parID,
						(StringPtr)*PreferencesFileNameHdl, 'NCSA', 'pref') != noErr)
			FatalAlert(CANT_CREATE_PREFS, 120, err);

		HCreateResFile(TelInfo->PrefFolder.vRefNum, TelInfo->PrefFolder.parID,
					(StringPtr)*PreferencesFileNameHdl);

		if (ResError() != noErr)
			FatalAlert(CANT_CREATE_PREFS, 120, ResError());

		TelInfo->SettingsFile = HOpenResFile(TelInfo->PrefFolder.vRefNum, 
												TelInfo->PrefFolder.parID,
											(StringPtr)*PreferencesFileNameHdl, fsRdWrShPerm);

		if (TelInfo->SettingsFile == -1)
			FatalAlert(CANT_CREATE_PREFS, 120, ResError());		// Give up
			
		if ((err = NewPreferences()) != noErr) return(err);
	  	}
		
	UseResFile(TelInfo->SettingsFile);
	ReleaseResource((Handle)PreferencesFileNameHdl);
}

OSErr NewPreferences(void)
{
	ApplicationPrefs	**AppPrefsHdl;
	FTPServerPrefs		**FTPPrefsHdl;
	SessionPrefs		**DefaultSessionPrefs;
	TerminalPrefs		**DefaultTerminalPrefs;
		
	// Get the master copies from the application's resource fork
	
	AppPrefsHdl = (ApplicationPrefs **)GetResource(APPLICATIONPREFS_RESTYPE, APPLICATIONPREFS_APPID);
	if ((ResError() != noErr) || (AppPrefsHdl == NULL)) return(ResError());
	DetachResource((Handle)AppPrefsHdl);
	
	FTPPrefsHdl = (FTPServerPrefs **)GetResource(FTPSERVERPREFS_RESTYPE, FTPSERVERPREFS_APPID);
	if ((ResError() != noErr) || (FTPPrefsHdl == NULL)) return(ResError());
	DetachResource((Handle)FTPPrefsHdl);

	DefaultSessionPrefs = (SessionPrefs **)GetResource(SESSIONPREFS_RESTYPE, SESSIONPREFS_APPID);
	if ((ResError() != noErr) || (DefaultSessionPrefs == NULL)) return(ResError());
	DetachResource((Handle)DefaultSessionPrefs);
	
	DefaultTerminalPrefs = (TerminalPrefs **)GetResource(TERMINALPREFS_RESTYPE, TERMINALPREFS_APPID);
	if ((ResError() != noErr) || (DefaultTerminalPrefs == NULL)) return(ResError());
	DetachResource((Handle)DefaultTerminalPrefs);

	// Add them to the Preferences file
	
	UseResFile(TelInfo->SettingsFile);
	if (ResError() != noErr) return(ResError());
	AddResource((Handle)AppPrefsHdl, APPLICATIONPREFS_RESTYPE, APPLICATIONPREFS_ID, "\p");
	if (ResError() != noErr) return(ResError());
	AddResource((Handle)FTPPrefsHdl,FTPSERVERPREFS_RESTYPE, FTPSERVERPREFS_ID, "\p");
	if (ResError() != noErr) return(ResError());
	AddResource((Handle)DefaultSessionPrefs,SESSIONPREFS_RESTYPE, SESSIONPREFS_APPID, "\p<Default>");
	if (ResError() != noErr) return(ResError());
	AddResource((Handle)DefaultTerminalPrefs,TERMINALPREFS_RESTYPE, TERMINALPREFS_APPID, "\p<Default>");
	if (ResError() != noErr) return(ResError());

	// Update the preferences file and release the resources
	
	UpdateResFile(TelInfo->SettingsFile);
	ReleaseResource((Handle)AppPrefsHdl);
	ReleaseResource((Handle)FTPPrefsHdl);
	ReleaseResource((Handle)DefaultSessionPrefs);
	ReleaseResource((Handle)DefaultTerminalPrefs);
	return(ResError());
}

OSErr LoadPreferences(void)
{
	ApplicationPrefs	**AppPrefsHdl;
	FTPServerPrefs		**FTPPrefsHdl;
	
	UseResFile(TelInfo->SettingsFile);
	if (ResError() != noErr) return(ResError());
	
	AppPrefsHdl = (ApplicationPrefs **)GetResource(APPLICATIONPREFS_RESTYPE, APPLICATIONPREFS_ID);
	if ((ResError() != noErr) || (AppPrefsHdl == NULL)) return(ResError());

	HLock((Handle) AppPrefsHdl);
	BlockMove(*AppPrefsHdl, gApplicationPrefs, sizeof(ApplicationPrefs));
	ReleaseResource((Handle) AppPrefsHdl);
	
	FTPPrefsHdl = (FTPServerPrefs **)GetResource(FTPSERVERPREFS_RESTYPE, FTPSERVERPREFS_ID);
	if ((ResError() != noErr) || (FTPPrefsHdl == NULL)) return(ResError());

	HLock((Handle) FTPPrefsHdl);
	BlockMove(*FTPPrefsHdl, gFTPServerPrefs, sizeof(FTPServerPrefs));
	ReleaseResource((Handle) FTPPrefsHdl);
	
	return(noErr);
}

OSErr SavePreferences(void)
{
	ApplicationPrefs	**AppPrefsHdl;
	FTPServerPrefs		**FTPPrefsHdl;
	Boolean				UserHasBeenAlerted = FALSE, UserResponse;
	
	UseResFile(TelInfo->SettingsFile);
	if (ResError() != noErr) return(ResError());
	
	AppPrefsHdl = (ApplicationPrefs **)GetResource(APPLICATIONPREFS_RESTYPE, APPLICATIONPREFS_ID);
	if ((ResError() != noErr) || (AppPrefsHdl == NULL)) return(ResError());

	HLock((Handle) AppPrefsHdl);
	if (gApplicationPrefs->version < (*AppPrefsHdl)->version) {
		UserResponse = AskUserAlert(PREFS_ARE_NEWER_ID, TRUE);		// Cancel is default
		if (UserResponse == TRUE) return(noErr);	// User doesn't want to destroy prefs
		UserHasBeenAlerted = TRUE;					// Don't ask the user twice!
		}
		
	BlockMove(gApplicationPrefs, *AppPrefsHdl, sizeof(ApplicationPrefs));
	ChangedResource((Handle) AppPrefsHdl);
	
	FTPPrefsHdl = (FTPServerPrefs **)GetResource(FTPSERVERPREFS_RESTYPE, FTPSERVERPREFS_ID);
	if ((ResError() != noErr) || (FTPPrefsHdl == NULL)) return(ResError());

	HLock((Handle) FTPPrefsHdl);
	if ((gFTPServerPrefs->version < (*FTPPrefsHdl)->version) && (!UserHasBeenAlerted)) {
		UserResponse = AskUserAlert(PREFS_ARE_NEWER_ID, TRUE);		// Cancel is default
		if (UserResponse == TRUE) return(noErr);	// User doesn't want to destroy prefs
		}
				
	BlockMove(gFTPServerPrefs, *FTPPrefsHdl, sizeof(FTPServerPrefs));
	ChangedResource((Handle) FTPPrefsHdl);

	UpdateResFile(TelInfo->SettingsFile);
	ReleaseResource((Handle) AppPrefsHdl);
	ReleaseResource((Handle) FTPPrefsHdl);
	return(noErr);
}

TerminalPrefs	**GetDefaultTerminal(void)
{
	TerminalPrefs	**theTerminalHdl;
	short			scratchshort;
	
	UseResFile(TelInfo->SettingsFile);
	theTerminalHdl = (TerminalPrefs **)Get1NamedResource(TERMINALPREFS_RESTYPE, "\p<Default>");
	
	// If there is an error here, we put up a dialog box about the prefs
	// being messed up and then we restore the default from the master copy
	// in the application's resource fork.
	
	if (theTerminalHdl == NULL) {
		FatalCancelAlert(PREFERENCES_PROBLEM, "\pRepair", 110, ResError());
		UseResFile(TelInfo->ApplicationFile);
		theTerminalHdl = (TerminalPrefs **)Get1NamedResource(TERMINALPREFS_RESTYPE, "\p<Default>");
		
		// If the master copy is not there, it's a fatal error!
		if (theTerminalHdl == NULL)
			FatalAlert(RESOURCE_PROBLEM, 111, ResError());	// Doesn't return
		
		DetachResource((Handle)theTerminalHdl);	
		UseResFile(TelInfo->SettingsFile);
		scratchshort = Unique1ID(TERMINALPREFS_RESTYPE);
		AddResource((Handle)theTerminalHdl, TERMINALPREFS_RESTYPE, scratchshort, "\p<Default>");

		// If an error ocurred fixing the prefs file, it's a fatal error!
		if (ResError() != noErr) 
			FatalAlert(NUKED_PREFS, 112, ResError());	// Doesn't return
			
		UpdateResFile(TelInfo->SettingsFile);
		}
		
	DetachResource((Handle)theTerminalHdl);
	return(theTerminalHdl);
}

SessionPrefs	**GetDefaultSession(void)
{
	SessionPrefs	**theSessionHdl;
	short			scratchshort;
	
	UseResFile(TelInfo->SettingsFile);
	theSessionHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, "\p<Default>");

	// If there is an error here, we put up a dialog box about the prefs
	// being messed up and then we restore the default from the master copy
	// in the application's resource fork.
	
	if (theSessionHdl == NULL) {
		FatalCancelAlert(PREFERENCES_PROBLEM, "\pRepair", 100, ResError());
		UseResFile(TelInfo->ApplicationFile);
		theSessionHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, "\p<Default>");
		
		// If the master copy is not there, it's a fatal error!
		if (theSessionHdl == NULL)
			FatalAlert(RESOURCE_PROBLEM, 101, ResError());	// Doesn't return
			
		DetachResource((Handle)theSessionHdl);
		UseResFile(TelInfo->SettingsFile);
		scratchshort = Unique1ID(SESSIONPREFS_RESTYPE);
		AddResource((Handle)theSessionHdl, SESSIONPREFS_RESTYPE, scratchshort, "\p<Default>");

		// If an error ocurred fixing the prefs file, it's a fatal error!
		if (ResError() != noErr) 
			FatalAlert(NUKED_PREFS, 102, ResError());	// Doesn't return
			
		UpdateResFile(TelInfo->SettingsFile);
		}
		
	DetachResource((Handle)theSessionHdl);
	return(theSessionHdl);
}

void	GetHostNameFromSession(StringPtr string)
{
	SessionPrefs	**sessHdl;
	
	UseResFile(TelInfo->SettingsFile);
	sessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, string);
	HLock((Handle)sessHdl);
	
	BlockMove((**sessHdl).hostname, string, Length((**sessHdl).hostname)+1);
	
	ReleaseResource((Handle)sessHdl);
}

//	We recognize the following input string: "xxxx yyyy"
//	If "xxxx" matches a session name, that session record is used.  Otherwise, the default
//	session record is used with "xxxx" as the DNS hostname.   "yyyy", if extant, is
//	converted to a number.  If it is a valid number, it is used as the port to connect to.
//	WARNING: Do not pass this routing a blank string.  (We check this in PresentOpenConnectionDialog.)
ConnInitParams	**NameToConnInitParams(StringPtr InputString)
{
	ConnInitParams	**theHdl;
	SessionPrefs	**sessHdl;
	TerminalPrefs	**termHdl;
	short			portRequested;
	Boolean			foundPort;
	
	theHdl = (ConnInitParams **)NewHandleClear(sizeof(ConnInitParams));
	
	foundPort = ProcessHostnameString(InputString, &portRequested);		
		
	UseResFile(TelInfo->SettingsFile);
	sessHdl = (SessionPrefs **)Get1NamedResource(SESSIONPREFS_RESTYPE, InputString);
	if (sessHdl == NULL) {				// Connect to host xxxx w/default session.
		sessHdl = GetDefaultSession();
		DetachResource((Handle) sessHdl);
		HLock((Handle)sessHdl);
		BlockMove(InputString, (**sessHdl).hostname, 64);
		}
	else {	
		DetachResource((Handle) sessHdl);
		HLock((Handle)sessHdl);
		}

	(**theHdl).session = sessHdl;
	
	UseResFile(TelInfo->SettingsFile);
	termHdl = (TerminalPrefs **)Get1NamedResource(TERMINALPREFS_RESTYPE, (**sessHdl).TerminalEmulation);
	if (termHdl == NULL) termHdl = GetDefaultTerminal();
	DetachResource((Handle) termHdl);
	(**theHdl).terminal = termHdl;
	HUnlock((Handle)sessHdl);
	
	((**theHdl).WindowName)[0] = 0;
	(**theHdl).ftpstate = 0;
	(**sessHdl).ip_address = 0;
	
	if (foundPort) (**sessHdl).port = portRequested;
	
	return(theHdl);
}

ConnInitParams	**ReturnDefaultConnInitParams(void)
{
	ConnInitParams	**theHdl;

	theHdl = (ConnInitParams **)NewHandleClear(sizeof(ConnInitParams));

	(**theHdl).session = GetDefaultSession();
	(**(**theHdl).session).ip_address = 0;
	(**theHdl).terminal = GetDefaultTerminal();
	
	return(theHdl);
}

Boolean			ProcessHostnameString(StringPtr	HostnameString, short *port)
{
	Str255			MungeString;
	short			scratchshort;
	long			portRequested;
	StringPtr		xxxxptr, yyyyptr;
	Boolean			foundPort = FALSE;
	
	//	Copy the whole damn thing over
	BlockMove(HostnameString, MungeString, 255);
	
	//	Remove leading spaces
	scratchshort = 1;
	while((scratchshort <= Length(MungeString)) && (MungeString[scratchshort] == ' '))
		scratchshort++;
	
	if (scratchshort > Length(MungeString)) {
		HostnameString[0] = 0;
		return(FALSE);
		}

	xxxxptr = &MungeString[scratchshort-1];
	
	//	Now look for a port number...
	while((scratchshort <= Length(MungeString)) && (MungeString[scratchshort] != ' '))
		scratchshort++;
	
	yyyyptr = &MungeString[scratchshort];
	
	if (scratchshort < Length(MungeString)) {
		MungeString[scratchshort] = Length(MungeString) - scratchshort;
		StringToNum(&MungeString[scratchshort], &portRequested);
		if ((portRequested > 0) && (portRequested < 65535))	foundPort = TRUE;
		}
	
	xxxxptr[0] = yyyyptr - xxxxptr - 1;

	//	Copy parsed hostname string back
	BlockMove(xxxxptr, HostnameString, Length(xxxxptr)+1);
	
	*port = (short)portRequested;
	
	return(foundPort);
}