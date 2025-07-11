/*
** CTelnetSettingsDLOG.h
**
**	MiniTelnet application
**	Telnet settings dialog director
**
**	Copyright � 1993-94, FrostByte Design / Eric Scouten
**
*/


#pragma once

#include "CDLOGDirector.h"
#include "MiniTelnet.const.h"


/*______________________________________________________________________
**
** CTelnetSettingsDLOG
**
**	This class provides the dialog box used to collect information on the Telnet connection.
**	It packages these settings into a record used by CTelnetTerminal to open connections.
**
*/

class CTelnetSettingsDLOG : public CDLOGDirector {

public:
	TelnetSettingsRec	r;				// settings record (see TelnetSettingsRec)


	// constructor

					CTelnetSettingsDLOG(CDirectorOwner* aSupervisor);


	// settings record functions

	virtual void		DefaultSettings();
	virtual void		GrabSettings();
	virtual void		PutSettings();


	// dialog handling functions

	virtual void		DoCommand(long theCommand);
	virtual void		UpdateMenus();


	// file interactions

	virtual void		DoSaveFile();

};