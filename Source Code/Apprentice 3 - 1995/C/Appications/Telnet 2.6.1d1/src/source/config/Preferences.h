#define	PREFERENCES_FILE_NAME_STR_ID	1991	// STR resource containing "NCSA Telnet
												//	Preferences" or translated name

// Name of each instance is the resource name.  Resource type TeRm
typedef	struct {
	short
		version;				// Version of this structure
		
	RGBColor
		nfcolor,				// Normal foreground RGB
		nbcolor,				// Normal background RGB
		bfcolor,				// Blinking foreground RGB
		bbcolor,				// Blinking background RGB
		reserved1,				// Reserved by JMB - NCSA
		reserved2;				// Reserved by JMB - NCSA
		
	short
		vtemulation,			// 0 = VT100, 1 = VT220 (supposedly)
		vtwidth,				// Width of the terminal screen in characters
		vtheight,				// Height of the terminal screen in characters
		fontsize,				// Size of DisplayFont to use to display text
		numbkscroll;			// Number of lines to save in scroll buffer
		
	Boolean
		ANSIgraphics,			// Recognize ANSI color sequences
		Xtermsequences,			// Recognize Xterm sequences
		vtwrap,					// Use VT wrap mode
		emacsmetakey,			// Option key is EMACS meta key
		emacsarrows,			// Arrow keys and mouse position are EMACS flavor
		MATmappings,			// Map PageUp, PageDown, Home, End. (MAT == Mark Tamsky)
		eightbit,				// Don't strip the high bit
		clearsave;				// Save cleared lines
		
	Str63
		DisplayFont;			// Font to use to display text
	
	Str32
		AnswerBackMessage;		// Response to send when asked what terminal we are
	
	short
		padding[100];			// So I'm generous....
}	TerminalPrefs;
#define	TERMINALPREFS_RESTYPE	'TeRm'
#define	TERMINALPREFS_APPID		1991		// The "<Default>" terminal seed copy

// Name of each instance is the resource name.  Resource type SeSn
typedef	struct {
	short
		version;				// Version of this structure
		
	short
		port,					// Port to connect to
		tektype,				// -1 = inhibit, 0 = 4014, 1 = 4105
		pastemethod,			// 0 = quick, 1 = block
		pasteblocksize;			// Size of blocks to paste if using block mode

	unsigned long
		ip_address;				// IP address of the host

	Boolean
		forcesave,				//
		crmap,					// Strange Bezerkley 4.3 CR mode needed
		linemode,				// Allow linemode to be used
		showlowlevelerrors,		// Show low level errors (??????????????????????)
		tekclear,				// Clear TEK window vs. create new one
		halfdup;				// Half duplex required
		
	char
		bksp,					// 0 DELETE = BKSP, 1 DELETE = DELETE
		ckey,
		skey,
		qkey;

	Str32
		TerminalEmulation,		// Name of terminal emulator to use
		TranslationTable;		// Name of translation table to use by default

	Str63
		hostname;				// The DNS name of the host
		
	Boolean
		authenticate,			// Kerberos authentication
		encrypt,				// Encrypted session
		localecho,				// Force local echo on?
		reserved1;				// Reserved by JMB - NCSA
		
	short
		padding[98];			// Disk space is cheap....
}	SessionPrefs;
#define	SESSIONPREFS_RESTYPE	'SeSn'
#define SESSIONPREFS_APPID		1991		// The "<Default>" session seed copy

// Only one instance of this resource type.  Resource type PrEf
typedef	struct	{
	short
		version;				// Version of this resource
		
	short
		CursorType,				// 0 = block, 1 = underscore, 2 = vertical line
		CopyTableThresh;		// Copy table threshold
		
	long
		TimeSlice;				// Timeslice passed to WaitNextEvent
		
	ResType
		CaptureFileCreator;		// Creator signature for session capture files
		
	Boolean
		WindowsDontGoAway,		// Connection's windows remain open after connection close
		StaggerWindows,			// Stagger connection windows
		CommandKeys,			// Use command key shortcuts for menus
		RemapTilde,				// Remap �~� to escape
		BlinkCursor;			// Blink the cursor
		
	short
		padding[100];			// Memory is getting cheap too....
}	ApplicationPrefs;
#define	APPLICATIONPREFS_RESTYPE	'PrEf'
#define	APPLICATIONPREFS_ID			1991
#define	APPLICATIONPREFS_APPID		1990

extern	ApplicationPrefs	*gApplicationPrefs;

// Only one instance of this structure.  Resource FTPs
typedef	struct {
	short
		version;				// Version of this resource

	short
		ServerState;			// 0 = off, 1 = no protection, 2 = password protection
		
	OSType
		BinaryCreator,			// Default creator for files transferred in binary mode
		BinaryType,				// Default type for files transferred in binary mode
		TextCreator;			// Default creator for file transferred in ASCII mode
		
	Boolean
		ShowFTPlog,				// FTP log defaults to visible
		DNSlookupconnections,	// Translate IP of connecting hosts to DNS names for log
		UseMacBinaryII,			// Allow MacBinary II
		unused1,				// Unused
		unused2,				// Unused
		ResetMacBinary,			// Reset MacBinary after each transfer
		DoISOtranslation;		// Use ISO translation

	short
		padding[100];
}	FTPServerPrefs;
#define	FTPSERVERPREFS_RESTYPE	'FTPs'
#define	FTPSERVERPREFS_ID 		1991
#define	FTPSERVERPREFS_APPID 	1990

extern	FTPServerPrefs	*gFTPServerPrefs;

// Username is the resource name.  Resource type FTPu
typedef	struct {
	short
		version;				// Version of this resource
	Str32
		EncryptedPassword,		// Encrypted form of user's password
		DefaultDirVolName;		// Name of the volume containing user's default directory
		
	long
		DefaultDirDirID;		// DirID of the user's default directory

	Boolean
		UserCanCWD;				// User is allowed to change directories
		
	short
		padding[20];			// Some padding for the future
}	FTPUser;
#define	FTPUSER					'FTPu'

// >>>> FTP Client Prefs? <<<	

typedef	struct {
	TerminalPrefs
		**terminal;
	SessionPrefs
		**session;
	Str255
		WindowName;
	short
		ftpstate;
	Rect
		WindowLocation;	
}	ConnInitParams;
