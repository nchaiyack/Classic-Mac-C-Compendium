/*
** CTelnetTerminal.h
**
**	MiniTelnet application
**	Telnet session document
**
**	Copyright � 1993-94, FrostByte Design / Eric Scouten
**
*/


#pragma once

#include "CDocument.h"
#include "CFile.h"
#include "CWindow.h"

#include "CTelnetInterp.h"
#include "CTerminalPane.h"
#include "MiniTelnet.const.h"


// important resource numbers

#define MENUTelnet			21
#define WINDTelnet			1026
#define SCPNTelnet			1026
#define DLOGSettings		2800
#define STR_SettingsPrompt	2800


// command numbers

#define cmdOpenSettings		2000L				// File menu

#define cmdSendSynch		2100L				// Telnet menu
#define cmdSendBreak		2101L
#define cmdSendAO			2102L
#define cmdSendIP			2103L
#define cmdSendAYT			2104L
#define cmdSendGA			2105L
#define cmdSendEC			2110L
#define cmdSendEL			2111L
#define cmdSendIPAddr		2120L
#define cmdShowDebug		2199L

#define cmdSaveSettings		2800L				// Settings dialog


/*______________________________________________________________________
**
** CTelnetTerminal
**
**	This class implements the behavior of a terminal connected to Telnet. It uses the
**	CTerminalPane class to draw the terminal screen. It handles all user events (such as
**	key-downs) and interacts with CTCPStream to process them.
**
**	Some debugging methods are included so that the �behind-the-scenes� negotiation of
**	Telnet can be viewed on-screen.
**
**	TurboTCP 2.0 changes: This class is a descendent of both CDocument and CTelnetInterp.
**	It no longer bases off CTCPSessionDoc.
**
*/

class CTelnetTerminal : public CDocument, public CTelnetInterp {

	TCL_DECLARE_CLASS;

protected:
	CTerminalPane*	itsTerminal;				// terminal who displays our I/O
	TelnetSettingsRec	r;						// settings record
	short			itsTermMode;				// which terminal emulation?


	// constructor

public:
					CTelnetTerminal(unsigned short theDefaultPort,
						unsigned long recBufferSize = recReceiveSize,
						unsigned short autoReceiveSize = recAutoRecSize,
						unsigned short autoReceiveNum = recAutoRecNum,
						Boolean doUseCName = TRUE,
						Boolean printable = TRUE);

	// creating new sessions

	virtual void		NewSession(TelnetSettingsRec* theSettings);
	virtual void		BuildWindow();


	// closing windows & sessions

	virtual Boolean		Close(Boolean quitting);
	virtual void		RemoteClose();


	// window titling

	virtual void		SetWindowTitle(Str255 newTitle);
	virtual void		GetFileName(Str255 theName)
						{ if (itsFile) itsFile->GetName(theName); else theName[0] = '\0'; }
	
	// command/event handling

	virtual void		DoCommand(long theCommand);
	virtual void		DoKeyDown(char theChar, Byte keyCode, EventRecord* macEvent);
	virtual void		DoAutoKey(char theChar, Byte keyCode, EventRecord* macEvent)
						{ DoKeyDown(theChar, keyCode, macEvent); }
	virtual void		DoPaste();
	virtual void		UpdateMenus();


	// data handling methods

	virtual void		HandleNVTChar(uchar theChar)
						{ itsTerminal->DoWriteChar(theChar); }
	virtual void		HandleNVTLine(char* theLine)
						{ itsTerminal->DoWriteStr(theLine); }


	// Telnet command handling

	virtual void		ReceivedDo(uchar theOption);
	virtual void		ReceivedAYT();
	virtual void		ReceivedEC()
						{ itsTerminal->DoEraseChar(); }
	virtual void		ReceivedEL()
						{ itsTerminal->DoEraseLine(); }
	virtual void		ReceivedSE();


	// Telnet option handling

	virtual void		OptionTerminalType();


	// terminal emulation handling

	virtual void		GetTerminalName(short termIndex, char* termStr);


	// debugging methods
		// These methods were useful to me in debugging the implementation of various Telnet
		// options. They are enabled by the �Show debugging codes� option of the settings dialog.

	virtual void		PrintDebugStr(char* theDebugStr)
						{ itsTerminal->DoWriteStr(theDebugStr); }
	virtual void		PrintDebugCharNum(char theChar, char leftBracket, char rightBracket)
						{ itsTerminal->DoWriteCharNum(theChar, leftBracket, rightBracket); }


	// CDocument methods which are not implemented here
	
	virtual void		NewFile() {};
	virtual void		OpenFile(SFReply* macSFReply) {};

};
