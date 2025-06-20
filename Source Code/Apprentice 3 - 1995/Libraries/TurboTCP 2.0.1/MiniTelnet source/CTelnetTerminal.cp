/*
** CTelnetTerminal.cp
**
**	MiniTelnet application
**	Telnet terminal session document
**
**	Copyright � 1993-94, FrostByte Design / Eric Scouten
**
*/


#include "CTelnetTerminal.h"

#include "CBartender.h"
#include "CClipboard.h"
#include "CDecorator.h"
#include "CDesktop.h"
#include "CScrollPane.h"
#include "Commands.h"
#include "TCLUtilities.h"

#include "CTerminalPane.h"


//	�� TCL globals ��

extern CBartender*	gBartender;
extern CClipboard*	gClipboard;
extern CDecorator*	gDecorator;
extern CDesktop*	gDesktop;

TCL_DEFINE_CLASS_M2(CTelnetTerminal, CDocument, CTelnetInterp);


//	�� constructor ��

/*______________________________________________________________________
**
** constructor
**
**	Initialize the Telnet terminal session document.
**
**		theDefaultPort (unsigned short):	default IP port number
**		recBufferSize (unsigned long):		size of the receive buffer we need
**		autoReceiveSize (unsigned long):	number of entries in RDS for auto-receive,
**										0 to disable autoreceiving
**		autoReceiveNum (unsigned short):	number of auto-receive calls to issue at once
**										must be at least 1
**		doUseCName (unsigned Boolean):	TRUE to look up canonical names for hosts
**		printable (Boolean):				TRUE if document is printable
**
*/

CTelnetTerminal::CTelnetTerminal(unsigned short theDefaultPort, unsigned long recBufferSize,
							unsigned short autoReceiveSize, unsigned short autoReceiveNum,
							Boolean doUseCName, Boolean printable)

	: CDocument(printable),
	  CTelnetInterp(theDefaultPort, recBufferSize, autoReceiveSize, autoReceiveNum, doUseCName)

{
	itsTerminal = NULL;
	itsTermMode = 0;
	showFileName = FALSE;
}


//	�� creating new sessions ��

/*______________________________________________________________________
**
** NewSession
**
**	Create a new window for document. Pulls information from the settings record which
**	it receives and opens a session accordingly.
**
**		theSettings (TelnetSettingsRec*):	the settings record to use
**
*/

void CTelnetTerminal::NewSession(TelnetSettingsRec* theSettings)

{
	// copy the settings record
	
	BlockMove(theSettings, &r, sizeof (TelnetSettingsRec));
	BlockMove(r.hostName, hostCName, 256);
	goAwayOnClose = r.closeOnSessionEnd;
	showDebug = r.showDebug;
	

	// build a window immediately
	
	BuildWindow();
	

	// open host by name
	
	#if _TestTerminal
		itsTerminal->DoWriteStr("No TCP session. Just type onto terminal.\r\n");
	#else
		SetOpenTimeout(20);
		OpenUserHost((char*) r.hostName, defaultPort, TRUE);
	#endif

}


/*______________________________________________________________________
**
** BuildWindow
**
**	Build a window for a terminal connection.
**
*/

void CTelnetTerminal::BuildWindow()

{
	CScrollPane*		theScrollPane;
	CTerminalPane*	theMainPane;
	Rect				sizeRect;
	

	// create a window

	itsWindow = new CWindow(WINDTelnet, FALSE, this);


	// set maximum size of window

	SetRect(&sizeRect, 100, 100, sizeX+16, sizeY+16);
	itsWindow->SetSizeRect(&sizeRect);


	// create the scrolling pane

	theScrollPane = new CScrollPane(itsWindow, this, 10, 10, 0, 0,
							sizELASTIC, sizELASTIC, TRUE, TRUE, TRUE);
	theScrollPane->FitToEnclFrame(TRUE, TRUE);


	// create the main view pane

	theMainPane = new CTerminalPane(theScrollPane, this, 0, 0, 0, 0, sizELASTIC, sizELASTIC);
	itsMainPane = theMainPane;
	itsTerminal = theMainPane;
	itsGopher = theMainPane;
	theMainPane->FitToEnclosure(TRUE, TRUE);


	// make sure the scroll pane knows about the panorama

	theScrollPane->InstallPanorama(theMainPane);
	

	// place, size, and title the window

	gDecorator->PlaceNewWindow(itsWindow);
	StateChanged();
	itsWindow->Select();

}


// �� closing windows & sessions ��

/*______________________________________________________________________
**
** Close
**
**	Respond to user close. Ensure that the TCP stream is gracefully closed. (This method is
**	also called if the remote host closes the session or the session is terminated.)
**
**		quitting (Boolean):	TRUE if quitting
**
**		return (Boolean):	FALSE if close/quit aborted by user
**
*/

Boolean CTelnetTerminal::Close(Boolean quitting)

{
	closeAndQuit = quitting;						// do we really want to close?
	if (!ConfirmClose(quitting))
		return FALSE;

	if (itsFile)									// close file first
		itsFile->Close();

	if (!LocalClose(quitting))						// now close stream
		return FALSE;

	return CDirector::Close(quitting);

}


/*______________________________________________________________________
**
** RemoteClose
**
**	Respond to notification that the remote host cancelled the session (either by normal
**	close or terminate).
**
*/

void CTelnetTerminal::RemoteClose()

{
	Boolean sessionWasEstablished = SessionEstablished();

	CTelnetInterp::RemoteClose();
	if ((goAwayOnClose) && (!sessionWasEstablished))
		Close(FALSE);
}


//	�� window titling ��

/*______________________________________________________________________
**
** SetWindowTitle
**
**	Sets title for window. Turns on or off cursor blinking depending on whether session
**	is established.
**
**		newTitle (Str255):	the new title for the window
**
*/

void CTelnetTerminal::SetWindowTitle(Str255 newTitle)

{
	if (itsWindow)
		itsWindow->SetTitle(newTitle);
	itsTerminal->SetBlinking(SessionEstablished());
}


//	�� command/event handling ��

/*______________________________________________________________________
**
** DoCommand
**
**	Handle all commands document can understand.
**
**		theCommand (long):	command number that was issued
**
*/

void CTelnetTerminal::DoCommand(long theCommand)

{
	char addrStr[18];						// used for send IP addr command
	
	switch (theCommand) {
	
		// Edit menu: support pasting TEXT
		
		case cmdPaste:
			DoPaste();
			break;
		
		// Telnet menu: send various strings to server
		
		case cmdSendSynch:
			if (showDebug)
				PrintDebugStr("{Urgent IAC DM}");
			SetNextUrgent();
			*this << "\377\362";
			break;

		case cmdSendBreak:
			if (showDebug)
				PrintDebugStr("{Urgent IAC BRK IAC DM}");
			SetNextUrgent();
			*this << "\377\363\377\362";
			break;
			
		case cmdSendAO:
			if (showDebug)
				PrintDebugStr("{Urgent IAC AO IAC DM}");
			SetNextUrgent();
			*this << "\377\365\377\362";
			break;
			
		case cmdSendIP:
			if (showDebug)
				PrintDebugStr("{Urgent IAC IP IAC DM}");
			SetNextUrgent();
			*this << "\377\364\377\362";
			break;
			
		case cmdSendAYT:
			if (showDebug)
				PrintDebugStr("{Urgent IAC AYT IAC DM}");
			SetNextUrgent();
			*this << "\377\366\377\362";
			break;
			
		case cmdSendGA:
			if (showDebug)
				PrintDebugStr("{IAC GA}");
			*this << "\377\371";
			break;
			
		case cmdSendEC:
			if (showDebug)
				PrintDebugStr("{IAC EC}");
			*this << "\377\367";
			break;
			
		case cmdSendEL:
			if (showDebug)
				PrintDebugStr("{IAC EL}");
			*this << "\377\370";
			break;
			
		case cmdSendIPAddr:
			*this << local_IP;
			break;

		case cmdShowDebug:
			showDebug = !showDebug;
			r.showDebug = showDebug;
			break;
			
		// not ours, send along the chain
		
		default:
			CDocument::DoCommand(theCommand);
	}
}


/*______________________________________________________________________
**
** DoKeyDown
**
**	Process key-down events. Parses CR, LF codes and generates the
**	standard CR/LF sequence when CR is passed and ignores LF.
**
**		theChar (char):				the character that was entered
**		keyCode (byte):			the Mac ADB key number for the key that was pressed
**		macEvent (EventRecord*):	the entire event record
**
*/

void CTelnetTerminal::DoKeyDown(char theChar, Byte keyCode, EventRecord* macEvent)

{

	// make sure this isn�t an errant command key
	
	if (!((*macEvent).modifiers & cmdKey)) {
		if (theChar != charLF) {
			#if _TestTerminal
				HandleNVTChar(theChar);
				if (theChar == charCR)
					HandleNVTChar(charLF);
			#else
				if (!SessionEstablished()) {
					SysBeep(0);
					return;
				}
		
				if (theChar == charBS)
					SendChar(r.backspaceChar);
				else
					SendChar(theChar);
		
				if (theChar == charCR)
					SendChar(charLF);
			#endif
		itsTerminal->ScrollToSelection();
		}
	}

}


/*______________________________________________________________________
**
** DoPaste
**
**	Respond to Paste command by sending all characters from the
**	clipboard to the remote host.
**
*/

void CTelnetTerminal::DoPaste()

{
	Handle	theData;
	char		lineBfr[83];
	char		*p, *pl, *lb;
	long		rem;
	short	i;
	
	// get the clipboard contents
	
	if (!(gClipboard->GetData('TEXT', &theData)))
		return;
	HLock(theData);
	
	// send it in line or 80-char chunks
	
	p = (char*) *theData;
	rem = GetHandleSize(theData);
	
	while (rem > 0) {
	
		// fetch a line (or 81 chars)
		
		i = 0;
		pl = p;
		lb = lineBfr;
		while ((rem-(i) > 0) && (*pl != charCR) && (i < 80))
			i++, *lb++ = *pl++;
		if (*pl == charCR) {
			i++, *pl++;
			*lb++ = charCR;
			*lb++ = charLF;
		}
		*lb = '\0';
	
		// send the line
		
		#if _TestTerminal
			HandleNVTLine(lineBfr);
		#else
			*this << lineBfr;
		#endif
	
		rem -= i;
		p += i;
		
	}
	
	HUnlock(theData);
	DisposHandle(theData);
}


/*______________________________________________________________________
**
** UpdateMenus
**
**	Enable Telnet-specific commands.
**
*/

void CTelnetTerminal::UpdateMenus()

{
	CDocument::UpdateMenus();
	
	// enable �Paste� command only if valid Clipboard
	
	#if _TestTerminal
		if (gClipboard->DataSize('TEXT'))
			gBartender->EnableCmd(cmdPaste);
	#else
		if (SessionEstablished() && (gClipboard->DataSize('TEXT')))
			gBartender->EnableCmd(cmdPaste);
	#endif

	
	// enable �Send�� commands only if session is active

	if (SessionEstablished()) {
		gBartender->EnableCmd(cmdSendSynch);
		gBartender->EnableCmd(cmdSendBreak);
		gBartender->EnableCmd(cmdSendAO);
		gBartender->EnableCmd(cmdSendIP);
		gBartender->EnableCmd(cmdSendAYT);
		gBartender->EnableCmd(cmdSendGA);
		gBartender->EnableCmd(cmdSendEC);
		gBartender->EnableCmd(cmdSendEL);
		gBartender->EnableCmd(cmdSendIPAddr);
		gBartender->EnableCmd(cmdShowDebug);
		gBartender->CheckMarkCmd(cmdShowDebug, showDebug);
	}

}


//	�� Telnet command handling ��

/*______________________________________________________________________
**
** ReceivedDo
**
**	Respond to a Telnet [IAC DO option] sequence.
**
**		theOption (uchar):	the option code
**
*/

void CTelnetTerminal::ReceivedDo(uchar theOption)

{
	char respondStr[4];
	
	// parse the option and respond to it
	
	switch (theOption) {
	
		// accept terminal type option only
		
		case optTERMINAL_TYPE:
			respondStr[0] = charIAC;
			respondStr[1] = escWILL;
			respondStr[2] = theOption;
			respondStr[3] = '\0';
			if (showDebug) {
				PrintDebugStr("{IAC WILL");
				PrintDebugCharNum(theOption, ' ', '}');
			}
			*this << respondStr;
			break;
		
		// reject the remaining options
		
		default:
			CTelnetInterp::ReceivedDo(theOption);
	}

}


/*______________________________________________________________________
**
** ReceivedAYT
**
**	Respond to a Telnet [IAC AYT] sequence.
**
*/

void CTelnetTerminal::ReceivedAYT()

{
	if (showDebug)
		PrintDebugStr("{Yes}");
	*this << "[Yes]";
}


/*______________________________________________________________________
**
** ReceivedSE
**
**	Parse subnegotiation parameters. Called when [IAC SE] is received.
**
*/

void CTelnetTerminal::ReceivedSE()

{
	// dispatch to an option-parsing routine
	
	if (sbBfrIndex < 1)
		return;
	switch (sbBfr[0]) {

		case optTERMINAL_TYPE:
			OptionTerminalType();
			break;
		
		default:
			CTelnetInterp::ReceivedSE();
	}
	
}


//	�� Telnet option handling ��

/*______________________________________________________________________
**
** OptionTerminalType
**
**	Interpret an [IAC SB TERMINAL-TYPE ... IAC SE] sequence. Default method always says
**	this is an unknown terminal.
**
*/

void CTelnetTerminal::OptionTerminalType()

{
	char		respondStr[47];
	short	i = 2;
	
	// respond only to TERMINAL-TYPE SEND queries; ignore others
	
	if (sbBfr[1] == 1) {

		// build a response string
		
		respondStr[0] = charIAC;
		respondStr[1] = escSB;
		respondStr[2] = optTERMINAL_TYPE;
		
		// switch to next terminal emulation
		
		itsTermMode++;
		if (itsTermMode > termMax)
			itsTermMode = 0;
		// SetTerminalMode(itsTermMode);
		
		// get name of terminal emulation
		
		GetTerminalName(itsTermMode, &respondStr[3]);
		
		// display response if debugging mode enabled
				
		if (showDebug) {
			PrintDebugStr("{IAC SB TERM-TYPE IS ");
			PrintDebugStr(&respondStr[3]);
			PrintDebugStr(" IAC SE}");
		}
		
		// add end of SB string & send it
		
		while (respondStr[++i])
			;
		respondStr[i] = charIAC;
		respondStr[i+1] = escSE;
		respondStr[i+2] = '\0';
		*this << respondStr;
	}
	
}


//	�� terminal emulation handling ��

/*______________________________________________________________________
**
** GetTerminalName
**
**	Return the Internet assigned name for the terminal being used.
**
**		termIndex (short):	the terminal emulation number
**		termStr (char*):	buffer to receive the terminal name (max 40 chars)
**
*/

void CTelnetTerminal::GetTerminalName(short termIndex, char* termStr)

{
	char theTerm[41] = "UNKNOWN";		// for now, all we have is �UNKNOWN�
	BlockMove(theTerm, termStr, 41);
}


