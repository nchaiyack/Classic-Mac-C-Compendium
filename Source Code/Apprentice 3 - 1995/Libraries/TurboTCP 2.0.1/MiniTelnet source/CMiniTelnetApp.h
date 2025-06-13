/*
** CMiniTelnetApp.h
**
**	MiniTelnet application
**	Application subclass
**
**	Copyright © 1993-94, FrostByte Design / Eric Scouten
**
*/


#pragma once

#include "CTCPApplication.h"
#include "MiniTelnet.const.h"


// MiniTelnet file parameters

#define kAppSignature		'dTCP'
#define kSettingsFileType		'dTSR'


// memory size information

#define kExtraMasters		4
#define kRainyDayFund		20480
#define kCriticalBalance		20480
#define kToolboxBalance		20480


/*______________________________________________________________________
**
** CMiniTelnetApp
**
**	This application class handles originating Telnet sessions and application-wide menus.
**
*/

class CMiniTelnetApp : public CTCPApplication {

	TCL_DECLARE_CLASS;


	// constructor

public:
				CMiniTelnetApp();
	virtual void	ForceClassReferences();


	// creation of Telnet sessions (documents)

	virtual void	CreateDocument();
	virtual void	OpenDocument(SFReply* macSFReply);
	virtual void	OpenSettings(SFReply* macSFReply);
	virtual void	OpenSettingsFile(SFReply* macSFReply, TelnetSettingsRec* theSettings);
	virtual void	NewSession(TelnetSettingsRec* newSettings);
	virtual void	SetUpFileParameters();


	// menu/command handling

	virtual void	DoCommand(long theCommand);
	virtual void	SetUpMenus(); 
	virtual void	UpdateMenus();
	virtual void	Run();

};
