/*
	Terminal 2.2
	"Main.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main
#endif

#include "Text.h"
#include "Main.h"
#include "Strings.h"
#include "Utilities.h"
#include "Document.h"
#include "File.h"
#include "Options.h"
#include "Scroll.h"
#include "Port.h"
#include "FormatStr.h"
#include "Macros.h"
#include "CancelDialog.h"
#include "Crc.h"

#define CAN		0x18	/* Cntrl-X */

/* ----- Globals ------------------------------------------------------- */

SysEnvRec		Mac;				/* Machine information */
Boolean			WNE;				/* WaitNextEvent() flag */
Boolean			Background;			/* In background flag */
DocumentPeek	TerminalWindow;		/* Terminal window */
Options			Settings;			/* Current options */
Byte			EmptyStr[] = "";	/* Often needed empty string */
OSType			TEXT = 'TEXT';		/* Text file type */
Handle			KCHR;				/* 'KCHR' resource */
Types			Application;		/* Application information */
Boolean			Abort = FALSE;		/* Abort flag */
Boolean			Busy = FALSE;		/* Transmit in progress flag */
short			SendFileRef = 0;	/* Send file reference number */
Boolean			Sending = FALSE;	/* Sending in progress */
short			Transfer = 0;		/* File transfer in progress */
Configuration	Config;				/* Configuration parameters */
Boolean			Control_X = FALSE;	/* Control-X abort flag */
Handle			MacrosText = 0;		/* Macros text */
Boolean			MFmemory;			/* MultiFinder temporary memory */
Boolean			DTR = FALSE;		/* Current state of DTR output */
#ifdef USECTB
Boolean			CTB;				/* Communications Tool Box flag */
#endif

/* ----- Display error alert ------------------------------------------- */

void Error(
	register short code,
	register Byte *text)
{
	Byte message[256];

	FormatStr(message, (Byte *)"\p%s%i %s", MyString(STR_M, M_ERROR),
		code, text);
	if (TerminalWindow) {
		SysBeep(1);
		MakeMessage(TerminalWindow, message);
	} else {
		ParamText(message, EmptyStr, EmptyStr, EmptyStr);
		CenterDialog('ALRT', ALRT_ERROR);
		Alert(ALRT_ERROR, 0);
	}
}

/* ----- See if window is document window ------------------------------ */

Boolean IsDocument(register WindowPtr window)
{
	return window && ((WindowPeek)window)->windowKind == userKind;
}

/* ----- See if window is system window -------------------------------- */

Boolean IsSystem(register WindowPtr window)
{
	return window && ((WindowPeek)window)->windowKind < 0;
}

/* ----- Quit ---------------------------------------------------------- */

static void Terminate(void)
{
	register short err;
	register Byte *name;
	short r;
	long count;
	Point position;

	SerialDropDTR(Settings.dropDTR);
	SerialClose();
	if (SendFileRef)			/* Close send file if necessary */
		FSClose(SendFileRef);
	if (TerminalWindow->file)	/* Close capture file if necessary */
		TextCapture(0);
	DisposPtr((Ptr)TerminalWindow->buf.text);
	position = topLeft(((GrafPtr)TerminalWindow)->portRect);
	SetPort((GrafPtr)TerminalWindow);
	LocalToGlobal(&position);
	if (position.h != Settings.terminalWindow.h ||
			position.v != Settings.terminalWindow.v ) {
		Settings.terminalWindow = position;
		Settings.dirty = TRUE;
	}
	DisposeWindow((WindowPtr)TerminalWindow);
	if (MacrosText)
		DisposHandle(MacrosText);

	/* Save options if necessary */

	if (Settings.dirty) {
		if ((err = OpenFile(Settings.scriptVolume,
				Settings.scriptDirectory,
				name = MyString(STR_G, G_SETTINGS), &r)) == fnfErr) {
			if (!(err = CreateFile(Settings.scriptVolume,
					Settings.scriptDirectory, name,
					Application.signature, Application.otype)))
				err = OpenFile(Settings.scriptVolume,
						Settings.scriptDirectory,
					name, &r);
		}
		if (!err) {
			Settings.version = SETTINGS;
			Settings.crc = CalcCRC((Byte *)&Settings + sizeof(short),
				sizeof(Settings) - sizeof(short), 0);
			count = sizeof(Settings);
			err = FSWrite(r, &count, (Ptr)&Settings);
			FSClose(r);
			FlushVol(0, Mac.sysVRefNum);
		}
		if (err)
			Error(err, EmptyStr);
	}
	ExitToShell();
}

/* ----- Adjust menus -------------------------------------------------- */

static void AdjustMenus(void)
{
	register MenuHandle menu;
	register Boolean empty =
		TerminalWindow->buf.newChar == TerminalWindow->buf.firstChar;

	menu = GetMHandle(FILE);
	if (Transfer) {
		DisableItem(menu, SCREEN);
		DisableItem(menu, CAPTURE);
		DisableItem(menu, SEND);
		switch (Transfer) {
			case Transfer_Rx:
				EnableItem(menu, RECEIVE);	/* To cancel receive */
				DisableItem(menu, TRANSMIT);
				break;
			case Transfer_Tx:
				DisableItem(menu, RECEIVE);
				EnableItem(menu, TRANSMIT);	/* To cancel transmit */
				break;
			case Transfer_B:
				DisableItem(menu, RECEIVE);
				DisableItem(menu, TRANSMIT);
		}
		DisableItem(menu, MAKE);
		DisableItem(menu, EXTRACT);
		DisableItem(menu, KISS);
		DisableItem(menu, QUIT);
	} else {
		if (empty)
			DisableItem(menu, SCREEN);
		else
			EnableItem(menu, SCREEN);
		EnableItem(menu, CAPTURE);
		EnableItem(menu, SEND);
		EnableItem(menu, RECEIVE);
		EnableItem(menu, TRANSMIT);
		EnableItem(menu, MAKE);
		EnableItem(menu, EXTRACT);
		EnableItem(menu, KISS);
		EnableItem(menu, QUIT);
	}

	menu = GetMHandle(EDIT);
	if (Transfer) {
		DisableItem(menu, RESET);
		EnableItem(menu, SHOWPW);
		DisableItem(menu, DEBLOCK);
	} else {
		if (empty)
			DisableItem(menu, RESET);
		else
			EnableItem(menu, RESET);
		DisableItem(menu, SHOWPW);
		EnableItem(menu, DEBLOCK);
	}
	if (DTR) {
		EnableItem(menu, DTRDROP);
		DisableItem(menu, DTRASSERT);
	} else {
		DisableItem(menu, DTRDROP);
		EnableItem(menu, DTRASSERT);
	}
	if (IsSystem(FrontWindow())) {
		EnableItem(menu, UNDO);
		EnableItem(menu, CUT);
		EnableItem(menu, COPY);
		EnableItem(menu, PASTE);
		EnableItem(menu, CLEAR);
	} else {
		long offset;
		DisableItem(menu, UNDO);
		DisableItem(menu, CUT);
		DisableItem(menu, COPY);
		if (Transfer || GetScrap(0, TEXT, &offset) <= 0)
			DisableItem(menu, PASTE);
		else
			EnableItem(menu, PASTE);
		DisableItem(menu, CLEAR);
	}

	menu = GetMHandle(OPTIONS);
	if (Transfer)
		DisableItem(menu, 0);
	else
		EnableItem(menu, 0);

	menu = GetMHandle(SCRIPT);
	if (Transfer)
		DisableItem(menu, 0);
	else
		EnableItem(menu, 0);

	menu = GetMHandle(MACRO);
	if (Transfer)
		DisableItem(menu, 0);
	else
		EnableItem(menu, 0);
}

/* ----- Handle menu commands ------------------------------------------ */

static void DoMenuCommand(
	register long menuResult,
	short options)
{
	register short menuItem;
	Str255 daName;

	menuItem = LoWrd(menuResult);
	switch(HiWrd(menuResult)) {
		case APPLE:
			switch(menuItem) {
				case ABOUT:
					About(options);
					UnloadSeg(About);
					break;
				default:
					GetItem(GetMHandle(APPLE), menuItem, (Byte *)&daName);
					OpenDeskAcc((Byte *)&daName);
					break;
			}
			break;
		case FILE:
			switch(menuItem) {
				case SCREEN:		/* Save screen buffer */
					SaveBuffer(options);
					break;
				case CAPTURE:		/* Capture to text file */
					TextCapture(options);
					break;
				case SEND:			/* Send text file */
					HiliteMenu(0);
					SendText();
					return;
				case RECEIVE:		/* Receive file */
					HiliteMenu(0);
					FileReceive();
					return;
				case TRANSMIT:		/* Transmit file */
					HiliteMenu(0);
					FileTransmit();
					return;
				case MAKE:			/* Make MacBinary file */
					Make();
					break;
				case EXTRACT:		/* Extract from MacBinary file */
					Extract();
					break;
				case KISS:			/* Kiss script file file */
					Kiss(options);
					break;
				case QUIT:
					HiliteMenu(0);
					Terminate();
					return;
			}
			break;
		case EDIT:
			if (!SystemEdit(menuItem-1)) {
				switch(menuItem) {
					case PASTE:		/* Send TEXT from scrap */
						HiliteMenu(0);
						SendScrap();
						return;
					case RESET:		/* Clear capture buffer */
						ClearBuffer();
						break;
					case SHOWPW:	/* Show progress window */
						SelectCancelDialog();
						break;
					case DEBLOCK:	/* Deblock send after XOFF/CTS */
						Sending = FALSE;
						SerialAbort();
						break;
					case DTRDROP:	/* Drop DTR */
						SerialDTR(FALSE);
						break;
					case DTRASSERT:	/* Assert DTR */
						SerialDTR(TRUE);
						break;
					case CTSCHECK:	/* Check CTS */
						{
							Byte s[80];
							MakeMessage(TerminalWindow,
								FormatStr(s, (Byte *)"\pCTS=%i", SerialCTS()));
						}
						break;
				}
			}
			break;
		case OPTIONS:
			switch(menuItem) {
				case PORT:
					PortOptions();
					UnloadSeg(PortOptions);
					break;
				case TEXTSEND:
					SendOptions();
					UnloadSeg(SendOptions);
					break;
				case TERMINAL:
					TerminalOptions();
					UnloadSeg(TerminalOptions);
					break;
				case OTHER:
					OtherOptions();
					UnloadSeg(OtherOptions);
					break;
				case TRANSFER:
					ProtocolOptions();
					UnloadSeg(ProtocolOptions);
					break;
				case XYOPTIONS:
					XYOptions();
					UnloadSeg(XYOptions);
					break;
				case ZOPTIONS:
					ZOptions();
					UnloadSeg(ZOptions);
					break;
			}
			break;
		case SCRIPT:
			HiliteMenu(0);
			if (menuItem == DOSCRIPT) {
				if (RunScript(0, 0, 0, menuItem))
					Terminate();
				return;
			}
			if (DoMenuScript(menuItem))
				Terminate();
			return;
		case MACRO:
			if (menuItem == DOMACRO) {
				short err;
				if (err = LoadMacros(0, 0, 0))
					Error(err, EmptyStr);
			} else {
				HiliteMenu(0);
				DoMacro(menuItem, (options & (optionKey | shiftKey)) != 0);
				return;
			}
	}
	HiliteMenu(0);
}

/* ----- Handle new event ---------------------------------------------- */

#define SuspendResume	1
#define ResumeMask		1

void DoEvent(register EventRecord *event)
{
	register short part;
	register Byte key;			/* ASCII key code */
	register short code;		/* Virtual key code */
	long state;					/* Used by KeyTrans() */
	WindowPtr window;

	switch(event->what) {
		case mouseDown:
			switch(part = FindWindow(event->where, &window)) {
				case inMenuBar:
					AdjustMenus();
					DoMenuCommand(MenuSelect(event->where),
						event->modifiers);
					break;
				case inSysWindow:
					SystemClick(event, window);
					break;
				case inContent:
					if (window != FrontWindow())
						SelectWindow(window);
					else {
						if (IsDocument(window)) {
							SetPort(window);
							GlobalToLocal(&event->where);
							DocumentClick((DocumentPeek)window,
								event->where, event->modifiers);
						}
					}
					break;
				case inDrag:
					DragWindow(window, event->where, &QD(screenBits.bounds));
					break;
				case inGoAway:
					/* Quit if close box clicked in document window */
					if (TrackGoAway(window, event->where)) {
						if (IsDocument(window)) {
							if (Transfer)
								SysBeep(1);
							else
								Terminate();
						}
						if (IsSystem(window))
							CloseDeskAcc(((WindowPeek)window)->windowKind);
					}
					break;
				}
				break;
			case keyDown:
			case autoKey:
				if (IsSystem(FrontWindow()))
					break;
				key = event->message & charCodeMask;
				/* Menu commands are not available if the command key
				is used as control key */
				if (Settings.ctrl != 2 && (event->modifiers & cmdKey)) {
					if (event->what == keyDown) {
						AdjustMenus();
						DoMenuCommand(MenuKey(key), event->modifiers);
					}
					break;
				}
				/* Ignore key strokes if file transfer is in progress,
				or if terminal window is not frontmost */
				if (Transfer || !IsDocument(FrontWindow()))
					break;
				/* Use option key as control key, if this option is set */
				if (Settings.ctrl == 1 && (event->modifiers & optionKey) && KCHR) {
					code = (event->message >> 8) & 0x7F;
					if (event->modifiers & shiftKey)
						code |= 0x0200;
					state = 0;
					LoadResource(KCHR);
					if (*KCHR)
						key = KeyTrans(*KCHR, code, &state) & 0x1F;
				} else
					/* Use option key as control key, if this option is set */
					if (Settings.ctrl == 2 && (event->modifiers & cmdKey))
						key &= 0x1F;
				NewKey(key);
				break;
			case activateEvt:
				if (IsDocument((WindowPtr)event->message))
					ActivateDocument((DocumentPeek)event->message,
						event->modifiers & activeFlag);
				break;
			case updateEvt:
				if (IsDocument((WindowPtr)event->message)) {
					BeginUpdate((WindowPtr)event->message);
					DrawDocument((DocumentPeek)event->message, 0);
					DrawControls((WindowPtr)event->message);
					EndUpdate((WindowPtr)event->message);
				}
				break;
			case diskEvt:
				if (HiWord(event->message) != noErr) {
					static Point where = { 80, 80 };
					DIBadMount(where, event->message);
				}
				break;
			case app4Evt:
				if (((unsigned long)event->message >> 24)==SuspendResume) {
					window = FrontWindow();
					if (event->message & ResumeMask) {
						Background = FALSE;
						if (IsDocument(window)) {
							HiliteWindow(window, TRUE);
							ActivateDocument((DocumentPeek)window, TRUE);
						}
					} else {
						Background = TRUE;
						if (IsDocument(window)) {
							HiliteWindow(window, FALSE);
							ActivateDocument((DocumentPeek)window, FALSE);
						}
					}
				}
				break;
		}
}

/* ----- Check for ctrl-X (abort sending) ------------------------------ */

static void AbortCheck(
	register Byte *buffer,
	register long count)
{
	static short can = 0;

	while (count--) {
		*buffer &= 0x7F;			/* Strip bit 7 */
		if (*buffer++ == CAN) {		/* Check for control-X */
			if (can == 4) {			/* 5th consecutive control-X */
				SerialAbort();
				Sending = FALSE;
				Control_X = TRUE;
			} else
				++can;
		} else
			can = 0;
	}
}

/* ----- Strip BS characters from string ------------------------------- */

static void StripBS(register Byte *s)
{
	register Byte t[256];
	register Byte *p, *q;
	register Byte c;
	register short n;

	n = *s;
	p = s + 1;
	q = t + 1;
	while (n--) {
		if ((c = *p++) == Settings.backspace) {
			if (q == t + 1)
				break;
			--q;
		} else
			*q++ = c;
	}
	*t = q - (t + 1);
	memcpy(s, t, *t + 1);
}

/* ----- Quick event check --------------------------------------------- */

void CheckEvents(void)
{
	register long count;
	register Byte buffer[256];
	EventRecord event;

	/* Make sure events and receive buffer are checked at least once */

	do {

		/* While transmitting check Mac events */

		do {
			if (WNE)
				WaitNextEvent(everyEvent, &event, 0, 0);
			else {
				SystemTask();
				GetNextEvent(everyEvent, &event);
			}
			DoEvent(&event);
		} while (Busy);

		/* If transmission finished see if something received */

		if (count = SerialRead(buffer, sizeof(buffer))) {
			AbortCheck(buffer, count);
			if (Settings.echo)
				SerialSend(buffer, count, &Busy);
			if (Settings.save)
				NewCharacters(buffer, count, FALSE);
		}

	} while (Busy);
	/* Only return if no more transmission is going on */
}

/* ----- Universal main event loop ------------------------------------- */

short Loop(
	register long timeout,	/* Timeout in ticks (0 for no timeout) */
	Byte *string,			/* Prompt string (1) or line (2) */
	register short mode)	/* 1: wait for prompt, 2: return next line */
{
	register long count;		/* Number of characters received */
	register short code = -1;	/* Return code */
	Byte buffer[256];			/* Receive buffer */
	EventRecord event;
	register Byte *s, *p, *max;
	register Byte c;

	if (timeout)
		timeout += Ticks;
	Abort = FALSE;
	if (string) {
		switch (mode) {
			case 1:	/* Prompt mode */
				if (*string)
					max = string + *string;
				else
					mode = 0;
				break;
			case 2:	/* Line mode */
				max = string + 255;
				break;
		}
		s = string + 1;
	} else
		mode = 0;

	/* Make sure events and receive buffer are checked at least once */

	do {

		/* Check for and handle Mac events */

		do {
			if (WNE)
				WaitNextEvent(everyEvent, &event, 0, 0);
			else {
				SystemTask();
				GetNextEvent(everyEvent, &event);
			}
			DoEvent(&event);
		} while (Busy);

		/* Check receive buffer */

		if (code < 0 && (count = SerialRead(buffer, sizeof(buffer)))) {
			AbortCheck(buffer, count);
			if (Settings.echo)
				SerialSend(buffer, count, &Busy);
			if (Settings.save)
				NewCharacters(buffer, count, TRUE);
			switch (mode) {
				case 1:	/* Prompt mode */
					p = buffer;
					while (count--)
						if ((*p++ & 0x7F) == *s) {
							if (s >= max) {
								code = FINE;
								break;	/* Prompt received */
							}
							s++;
						} else
							s = string + 1;
					break;
				case 2:	/* Line mode */
					p = buffer;
					while (count--) {
						c = *p++ & 0x7F;
						if (c == '\015') {
							*string = s - string - 1;
							StripBS(string);
							code = FINE;
							break;	/* Command line received */
						}
						if (s <= max)
							*s++ = c;
					}
					break;
			} /* switch (mode) */
		} /* if (count) */

		/* Check abort or timeout */

		if (Abort)
			code = CANCEL;
		else if (timeout && Ticks > timeout)
			code = TIMEOUT;

	} while (code < 0 || Busy);
	/* Only return if no transmission is going on */
	Abort = TRUE;
	return code;
}

/* ----- Universal main event loop ------------------------------------- */

short LoopBuffer(
	register long timeout,	/* Timeout in ticks (0 for no timeout) */
	register Byte *buffer,	/* Buffer supplied by caller */
	register long limit)	/* Size of buffer */
{
	register long count;		/* Number of characters received */
	register short code = -1;	/* Return code */
	EventRecord event;

	if (timeout)
		timeout += Ticks;
	Abort = FALSE;

	/* Make sure events and receive buffer are checked at least once */

	do {

		/* Check for and handle Mac events */

		do {
			if (WNE)
				WaitNextEvent(everyEvent, &event, 0, 0);
			else {
				SystemTask();
				GetNextEvent(everyEvent, &event);
			}
			DoEvent(&event);
		} while (Busy);

		/* Check receive buffer */

		if ((count = SerialCheck()) >= limit) {
			SerialFastRead(buffer, limit);
			AbortCheck(buffer, limit);
			if (Settings.echo)
				SerialSend(buffer, limit, &Busy);
			if (Settings.save)
				NewCharacters(buffer, limit, TRUE);
			code = FINE;	/* Buffer received */
		}

		/* Check abort or timeout */

		if (Abort)
			code = CANCEL;
		else if (timeout && Ticks > timeout)
			code = TIMEOUT;

	} while (code < 0 || Busy);
	/* Only return if no transmission is going on */
	Abort = TRUE;
	return code;
}

/* ----- Crash --------------------------------------------------------- */

pascal void Crash(void)
{
	ExitToShell();
}

/* ----- Main ---------------------------------------------------------- */

#define STACKSPACE	0x4000	/* We need a least 16K stack (default is 8K on
								a Mac Plus and 24K on a Mac IIcx) */

void main(void)
{
#ifdef applec
	void _DataInit(void);
	UnloadSeg((Ptr)_DataInit);
#endif
	SetApplLimit(CurStackBase - STACKSPACE);
	MaxApplZone();
	if (Init())
		return;
	UnloadSeg(Init);
	LoadMacros (Settings.scriptVolume, Settings.scriptDirectory,
		MyString(STR_G, G_MACROS));

	/* Check if script selected from desktop */

	{
		short message, count, i;
		AppFile file;

		CountAppFiles(&message, &count);
		if (message == appOpen)
			for (i = 1; i <= count; ++i) {
				GetAppFiles (i, &file);
				if (file.fType == TEXT &&
						CheckSuffix((Byte *)file.fName,
							MyString(STR_G, G_SUFFIX))) {
					if (RunScript(file.vRefNum, 0,
							(Byte *)file.fName, DOSCRIPT))
						Terminate();
					break;
				}
			}
	}

	/* Check for startup script */

	if (Settings.startName[0] && RunScript(Settings.startVolume,
			Settings.startDirectory, Settings.startName, DOSCRIPT))
		Terminate();
	for (;;) {
		SetWTitle((WindowPtr)TerminalWindow, MyString(STR_G, G_TERMINAL));
		Loop(0, 0, 0);
	}
}
