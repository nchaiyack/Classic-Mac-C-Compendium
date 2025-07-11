/*
	Terminal 2.2
	"Main.h"
*/

#define	MENUBAR		128
#define	APPLE		128
#define 	ABOUT		1
#define	FILE		129
#define		SCREEN		1		/* Save screen buffer... */
#define 	CAPTURE		2		/* Text capture... */
#define		SEND		3		/* Text send... */
#define 	RECEIVE		5		/* File receive... */
#define 	TRANSMIT	6		/* File send... */
#define		MAKE		8		/* Make MacBinary file... */
#define		EXTRACT		9		/* Extract MacBinary file... */
#define		KISS		10		/* Kiss script file... */
#define		QUIT		12
#define EDIT		130
#define		UNDO		1
#define		CUT			3
#define 	COPY		4
#define 	PASTE		5
#define 	CLEAR		6
#define		RESET		8		/* Clear capture buffer */
#define		SHOWPW		9		/* Show progress window */
#define		DEBLOCK		11		/* Deblock send */
#define		DTRDROP		12		/* Drop DTR */
#define		DTRASSERT	13		/* Assert DTR */
#define		CTSCHECK	14		/* Check CTS */
#define	OPTIONS		131
#define		PORT		1		/* Communication port settings... */
#define		TEXTSEND	2		/* TEXT file send options... */
#define		TERMINAL	3		/* Terminal options... */
#define		OTHER		4		/* Other options... */
#define		TRANSFER	5		/* File transfer options... */
#define		XYOPTIONS	6		/* XY-Modem options... */
#define		ZOPTIONS	7		/* Z-Modem options... */
#define SCRIPT		132
#define		DOSCRIPT	1		/* Execute script... */
#define		SCRIPT1		3		/* First script */
#define MACRO		133
#define		DOMACRO		1		/* Execute macro... */
#define		DOMACRO1	3		/* First macro */

#define ALRT_ERROR		128		/* Error alert */
#define DLOG_ABOUT		129		/* About dialog */
#define DLOG_TERMINAL	130		/* Terminal options dialog */
#define DLOG_PROGRESS	131		/* Progress dialog */
#define DLOG_PORT		132		/* Communication port settings dialog */
#define DLOG_XYMODEM	133		/* XY-Modem options dialog */
#define DLOG_SEND		134		/* TEXT file send options dialog */
#define DLOG_OTHER		135		/* Other options dialog */
#define DLOG_ZMODEM		136		/* Z-Modem options dialog */
#define DLOG_TRANSFER	137		/* File transfer options dialog */

#define RECORD		1024		/* Size of capture file record */
#define SETTINGS	0x0220		/* Version of settings structure */

#define ACTIVE		(outline + bold + extend)

#define FINE		0		/* No error */
#define TIMEOUT		1		/* Timeout error */
#define CANCEL		2		/* Cancel by button or menu */
#define ABORT		3		/* Abort by ctrl-X received */
#define ERROR		4		/* Other error */

#define USECTB		/* Use Communications Toolbox if available */

typedef struct {
	WindowRecord	window;
	Rect			messRect;	/* Message rectangle */
	ControlHandle	messOk;		/* "Ok" button */
	Byte			mess[256];	/* Message */
	Rect			rect;		/* Rx display rectangle */
	Point			cursor0;	/* First cursor position in rx rectangle */
	Point			cursor;		/* Current rx cursor position */
	unsigned short	height;		/* Line height */
	Rect			character;	/* Character enclosing rectangle */
	ControlHandle	vs;			/* Vertical scroll bar */
	unsigned short	linesPage;	/* Number of lines per page */
	TextRecord		buf;		/* Circular capture buffer */
	short			file;		/* Text capture file reference */
	short			volume;		/* Text capture volume reference */
	long			length;		/* Current length in record */
	Byte		record[RECORD];	/* Capture file record */
} DocumentRecord, *DocumentPeek;

typedef struct {
	unsigned short	crc;		/* To check if options are valid */
	unsigned short	version;	/* Version of options structure */
	short		dirty;			/* If options were modified */

	/* Communication options */

	Byte		portName[256];	/* What port to use */
	short		portSetup;		/* Serial port setup */
	short		handshake;		/* Handshake */
	short		dropDTR;		/* Dont' drop DTR when quitting */

	/* TEXT file send options */

	Byte		prompt[12];		/* Prompt string */
	long		linedelay;		/* Line delay (ticks) */
	long		chardelay;		/* Character delay (ticks) */

	/* Binary file transfer options */

	short		Binary;			/* Use and recognize MacBinary */
	Byte		volumeName[28];	/* Volume name (down- & uploads) */
	short		volume;			/* Volume reference (down- & uploads) */
	long		directory;		/* Directory id (down- & uploads) */
	short		protocol;		/* Enable CIS-B file transfer protocol */
	short		ZModem;			/* Use ZModem (else use XYModem) */
	short		ZAutoReceive;	/* Z-Modem: auto receive */

	/* XYModem options */

	long		XModemtimeout;	/* XModem: timeout (ticks) */
	short		XModemCRC;		/* XModem: use and recognize CRC */
	short		XModem1K;		/* Xmodem: use and recognize 1K blocks */
	short		batch;			/* Y-Modem batch mode */

	/* ZModem options */

	short		ZEscapeCtl;		/* Z-Modem: escape all control characters */
	long		ZTimeout;		/* Z-Modem: receive timeout (ticks) */
	long		ZBuffer;		/* Z-Modem: receive buffer size (bytes) */
	long		ZRetries;		/* Z-Modem: receive maximum retries */
	long		ZPacket;		/* Z-Modem: transmit sub-packet length (Bytes) */
	long		ZWindow;		/* Z-Modem: transmit window size (Bytes) */
	long		Zcrcq;			/* Z-Modem: transmit ZCRCQ spacing (Bytes) */

	/* Terminal options */

	short	 	localEcho;		/* Local echo flag */
	short		echo;			/* Echo received characters */
	short		autoLF;			/* Insert LF after each CR sent */
	short		save;			/* Save and display received characters */
	short		scriptVolume;	/* Volume reference (script files) */
	long		scriptDirectory;/* Directory id (script files) */
	Byte		startVName[28];	/* Volume name (autostart script file) */
	short		startVolume;	/* Volume reference (autostart script file) */
	long		startDirectory;	/* Directory id (autostart script file) */
	Byte		startName[32];	/* Autostart script file name */

	/* Other options */

	long		textCreator;	/* TEXT file creator */
	long		binType;		/* Non-MacBinary file type */
	long		binCreator;		/* Non-MacBinary file creator */
	Byte		backspace;		/* Code for backspace key */
	Byte		escape;			/* Code for ` key */
	short		beep;			/* Accept control-G as beep */
	short		ctrl;			/* Control key */

	/* Window positions */
	
	Point		terminalWindow;	/* Terminal window */
	Point		progressWindow;	/* Progress window */
} Options;

typedef struct {
	short		icon;			/* Application 'ICN#' id (128) */
	short		version;		/* Resource id of version data (0) */
	long		signature;		/* Application signature ('ET_T') */
	long		otype;			/* Options file type ('ET_O') */
	long		btype;			/* MacBinary file type ('ET_B') */
	long		stype;			/* Script file type ('TEXT') */
	long		ztype;			/* Z-modem partial file ('ET_Z') */
} Types;

typedef struct {		/* 'CNFG' (configuration) resource */
	short font;			/* Font number (should be monospaced) */
	short size;			/* Font size */
	short lines;		/* Lines in terminal window */
	short columns;		/* Columns in terminal window */
	long buffer;		/* Size of terminal buffer (bytes) */
	long input;			/* Size of serial input buffer (bytes) */
	long output;		/* Size of serial output buffer (bytes) */
	long script;		/* Size of script memory (bytes) */
	Byte fontname[];	/* Font name (should be monospaced) */
} Configuration;

#define HiWrd(a)	(((a) >> 16) & 0xFFFF)
#define LoWrd(a)	((a) & 0xFFFF)
#define Max(a,b)	((a) > (b) ? (a) : (b))
#define Min(a,b)	((a) < (b) ? (a) : (b))

#define Transfer_Rx		1		/* File transfer, reception */
#define Transfer_Tx		2		/* File transfer, transmission */
#define Transfer_B		3		/* File transfer, Quick-B */

extern SysEnvRec		Mac;			/* Machine information */
extern Boolean			WNE;			/* WaitNextEvent() flag */
extern Boolean			Background;		/* In background flag */
extern DocumentPeek		TerminalWindow;	/* Terminal window */
extern Options			Settings;		/* Current options */
extern Byte				EmptyStr[];		/* Often needed empty string */
extern OSType			TEXT;			/* Text file type */
extern Handle			KCHR;			/* 'KCHR' resource */
extern Types			Application;	/* Signature information */
extern Boolean			Abort;			/* Abort flag */
extern Boolean			Busy;			/* Transmit in progress flag */
extern short			SendFileRef;	/* Send file reference number */
extern Boolean			Sending;		/* Sending in progress */
extern short			Transfer;		/* File transfer in progress */
extern Configuration	Config;			/* Configuration parameters */
extern Boolean			Control_X;		/* Control-X abort flag */
extern Handle			MacrosText;		/* Macros text */
extern Boolean			MFmemory;		/* MultiFinder temporary memory */
extern Boolean			DTR;			/* Current state of DTR output */
#ifdef USECTB
extern Boolean			CTB;			/* Communications Tool Box flag */
#endif

void About (short);
void CheckEvents (void);
pascal void Crash (void);
void DoEvent (EventRecord *);
void Error (short, Byte *);
Boolean IsDocument (WindowPtr);
Boolean IsSystem (WindowPtr);
short Loop (long, Byte *, short);
short LoopBuffer(long, Byte *, long);
Boolean Init (void);
