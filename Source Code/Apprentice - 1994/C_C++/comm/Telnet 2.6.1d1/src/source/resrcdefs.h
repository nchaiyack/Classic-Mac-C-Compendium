/*	Resource Defines */

static ResType	creator 		= 'NCSA';	/* Creator signature for "saved set" files */
static ResType	filetype 		= 'CONF';	/* Filetype for "saved set" files */

/* ----------------- Defines for translation routines -------------------------------*/
#define	MY_TRSL				'TRSL'	/* resource type for translation */
#define	TRSL_DEFAULT_TABLE	256		/* The default table */
#define	TRSL_FTP_TABLE		257		/* The MAC->ISO, ISO->MAC table for FTP */

#define	USER_TRSL	'taBL'	/* User translation tables resource type */
							/* 256 bytes of in table followed by 256 bytes of out table */
							
#define GetStrRes(x)	(Str255 **)GetResource('STR ',(x));		/* Get string resource */

/*================================================================================*/
/* Defines for the Menu related resources */

#define	NMENUS	11			/*	We have NMENUS menus right now */

#define	appleMenu	1		/*	Apple Menu ID */
#define	NfileMenu	512		/*	File Menu Resource ID */
#define	NeditMenu	513		/*	Edit Menu Resource ID */
#define NtermMenu	514		/*	Emulation Menu Resource ID */
#define	NconnMenu	515		/*	Connection Menu Resource ID */
#define NnetMenu	516		/*	Network Menu Resource ID */

#define	fileMenu	256		/*	File Menu ID */
#define	editMenu	257		/*	Edit Menu ID */
#define termMenu	258		/*	Emulation Menu ID */
#define	connMenu	259		/*	Connection Menu ID */
#define netMenu		260		/*	Network Menu Resource ID */
#define fontMenu	128		/*  Font Menu Resource ID */
#define sizeMenu	129		/*	Size Menu Resource ID */
#define opspecMenu	130		/*	Open Special Sub-Menu Resource ID */
#define prefsMenu	131		/*	Preferences Sub-Menu Resource ID */
#define	transMenu	132		/*	Translation Sub-Menu Resource ID */

#define	Fil			1		/*	File Menu's Position */
#define Edit		2		/*  Edit Menu's Position */
#define	Emul		3		/*	Emulation Menu's position */
#define Net			4		/*	Network Menu's position */
#define Conn		5		/*	Connection Menu's position */
#define Font		6		/*	Font Menu's position */
#define Sizem		7		/*  Size Menu's position */
#define OpSpec		8		/*	Open Special Sub-Menu's position */
#define PrefsSub	9		/*	Preferences Sub-Menu position */
#define	National	10		/*	Translation Sub-Menu's postition */

/* ----------------- The File Menu ---------------------- */
#define FLopen		1		/* File Menu: Open Connection */
#define FLopenspec	2		/* File Menu: Open Special */
#define	FLclose		3		/* File Menu: Close Connection */
#define	FLload		5		/* File Menu: Load Set */
#define	FLsave		6		/* File Menu: Save Set */
#define FLbin		8		/* File Menu: MacBinary on */
#define FLlog		10		/* File Menu: ftp log on */
#define FLprint		12		/* File Menu: Print Selection */
#define FLpset		13		/* File Menu: Page Setup */
#define FLquit		15		/* File Menu: Quit */

/* ----------------- The Edit Menu ---------------------- */
#define	EDundo		1
#define EDcut		3
#define EDcopy		4		/* Edit Menu: Copy */
#define EDpaste		5		/* Edit Menu: Paste */
#define EDclear		6		/* Edit Menu: Clear */
#define EDcopyt		7		/* Edit Menu: Copy Table */
#define EDmacros	9		/* Edit Menu: Macros */
#define EDprefs		10		/* Edit Menu: Preferences */

/* ----------------- The Session Menu ---------------------- */
#define EMbs		1		/* Emulation Menu: backspace */
#define EMdel		2		/* Emulation Menu: delete */
#define EMecho		4		/* Emulation Menu: echo */
#define EMwrap      5       /* Emulation Menu: Wrap mode on/off */
#define EMarrowmap	6		/* JMB - Emulation Menu: Turn EMACS mapping off/on */
#define EMpgupdwn	7		/* JMB - Emulation Menu: Turn local PgUp/PgDwn/etc. on/off */
#define EMnational	8		/* LU */
#define EMscroll	9		/* Emulation Menu: Clear Screen Saves Lines */
#define EMreset 	10		/* Emulation Menu: Reset Terminal */
#define EMjump		11		/* Emulation Menu: Jump Scroll */
#define EMpage		12		/* Emulation Menu: TEK Page command */
#define EMclear		13		/* BYU 2.4.8 - Emulation Menu: TEK Page clears screen */
#define EMscreensize 15		/* NCSA: SB - new screen dimensions dialog */
#define EMsetup     17      /* BYU 2.4.8 - Emulation Menu: Setup keys */
#define EMfont		18		/* BYU 2.4.8 - Emulation Menu: Font */
#define EMsize		19		/* BYU 2.4.8 - Emulation Menu: Size */
#define EMcolor		20		/* BYU 2.4.8 - Emulation Menu: Color */
#define EMcapture	22		/* BYU 2.4.18 - Emulation Menu: Capture to file */

/* ----------------- The Network Menu ---------------------- */
/* #define NEcommand	19	Emulation Menu: Command Keys */
#define	NEftp		1		/* Emulation Menu: Send ftp command */
#define	NEip		2		/* Emulation Menu: Send IP # */
#define NEayt		4		/* Emulation Menu: Send "Are You There" */
#define NEao		5		/* Emulation Menu: Send "Abort Output" */
#define NEinter		6		/* Emulation Menu: Send "Interrupt Process" */
#define NEec		7		/* Emulation Menu: Send "Erase Character" */
#define NEel		8		/* Emulation Menu: Send "Erase Line" */
#define NEscroll	10		/* Network Menu: Scroll Lock */
#define NEnet		12		/* Network Menu: Show Net #'s */

/* ----------------- The Connection Menu ---------------------- */
#define COnext			1	/* Connection Menu: Next Session... */
#define	COtitle			2	/* Change window title */
#define	FIRST_CNXN_ITEM	4	/* The first slot for connections in the menu */
/* ----------------- The Preferences Menu --------------------- */
#define	prfGlobal	1
#define	prfTerm		2
#define	prfSess		3
#define	prfFTP		4
#define prfFTPUser	5
/*================================================================================*/
/*	Cursor resource related defines */

#define	rPOSCURS		256
#define rBeamCursor		1
#define rCrossCursor 	2
#define rPlusCursor  	3
#define rWatchCursor  	4
#define	rGINCURS		257
#define	rDbugCURS		268
#define	rXferCURS		290
#define leftcrsr		130		/* cursors to display encryption state */
#define rightcrsr		129		/* cursors to display encryption state */
#define lockcrsr		131		/* cursors to display encryption state */
/*================================================================================*/
/*	WIND resources for remembering the position of our windows */
#define	FTPlogWIND	257

/*================================================================================*/
/* String resource related #defines */
#define SAVE_SET_STRINGS_ID		23239		/* NCSA Telnet-save-set keywords */
#define SAVE_SET_STRINGS_COUNT 	40
#define AFF_WORDS_ID			23240		/* NCSA save-set file, affirmatives */
#define AFF_WORDS_COUNT			13
#define MSG_RESOURCE_ID 		23227		/* ftp strings of interest */

#define NETERROR_RESOURCE_ID 	23250		/* network errors */
#define NETERROR_RESOURCE_COUNT 17			/* number of network errors */
#define MEMERROR_RESOURCE_ID 	23251		/* memory errors */
#define MEMERROR_RESOURCE_COUNT 7			
#define RESERROR_RESOURCE_ID 	23253		/* resource errors */
#define RESERROR_RESOURCE_COUNT 7	
		
#define	GENERAL_MESSAGES_ID		2000		// Misc. messages
#define	PREFERENCES_PROBLEM		1			// Problem w/Prefs, should I attempt fix?
#define RESOURCE_PROBLEM		2			// Vital resource missing from Telnet
#define	NUKED_PREFS				3			// Unable to repair the prefs file
#define	PREFS_ARE_NEWER_ID		4			// Prefs on disk are newer than those in memory
#define	CANT_OPEN_MACTCP		5			// Error occured after OpenDriver("\p.IPP");
#define REALLY_QUIT_QUESTION	6			// Really quit w/open connections?
#define	CANT_CREATE_PREFS		7			// Error creating prefs file
#define	NEED_HFS_ERR			8			// Telnet requires HFS
#define	SYS_ENVIRON_ERR			9			// Wrong SysEnvirons version
#define	SYSTEM_VERS_ERR			10			// Telnet requires at least System 6.0
#define	ROM_VERS_ERR			11			// Telnet requires at least 128k ROMS
#define	AE_PROBLEM_ERR			12			// Problem installing AppleEvent handlers

#define	OPFAILED_MESSAGES_ID	2001		// General messages when an operation fails
#define	CANT_CREATE_FILE		1			// Can't create a file for some reason
#define	CANT_OPEN_FILE			2			// Can't open a file
#define	OUT_OF_MEMORY			3			// Not enough memory
#define	BAD_SET_ERR				4			// Invalid keyword in set file

#define	MISC_STRINGS			2002		// Misc. internal strings
#define	CAPTFILENAME			1			// Name of default capture file
#define	MISC_NEWSESSION			2			// Default name for new session
#define	MISC_NEWTERM			3			// Default name for new terminal

#define	DNR_MESSAGES_ID			2004		// DNR error messages

/*================================================================================*/
/*	#defines for ALRT and DITL resources */

#define	MemoryLowAlert	200	// Preloaded, unpurgable alert to tell the user memory is tight

/*	#defines for the DLOG and DITL resources */

#define	DLOGOk		1
#define DLOGCancel	2

#define RESOLVERERROR	128

#define	kItemDLOG	150
#define	kRemove		2
#define	kChange		3
#define	kNew		4
#define	kItemList	5

#define	FATALCANCEL_ID	160		// Fatal Cancel Alert
#define	FATAL_ID		170		// Fatal Alert
#define OPFAILED_ID		175		// Operation Failed Alert
#define	ASKUSER_ID		180		// Ask user an OK/CANCEL question ALERT

#define	WinTitlDLOG		190		// Change window title DLOG
#define	kWinNameTE		3		// Window Name TextEdit

#define	DNRErrorDLOG	195		// DNR error occurred.

#define	MyIPDLOG		259

#define	StatusDLOG		270		// "Machine" is currently being "opened/looked up"

#define SetupDLOG		273		/*  Setup keys interface */
#define killbox			4
#define stopbox			5
#define startbox		6

#define	NewCnxnDLOG		280
#define	NCconnect		1
#define	NCcancel		2
#define	NChostname		4
#define	NCwindowname	6
#define	NCftpcheckbox	7
#define	NCsesspopup		8
#define NCauthenticate	9
#define NCencrypt		10

#define	ConnFalDLOG		303
#define MacroDLOG		500		/* The Macros dialog */
#define	AboutDLOG		501
#define OopsDLOG		640
#define	FirstDLOG		1023

#define	ColorDLOG		1001	/* Connection color selection dialog */
#define	ColorNF			3
#define	ColorNB			4
#define	ColorBF			5
#define	ColorBB			6

#define	CloseDLOG		2001	/* The Close Confirmation Dialog */

#define WDSET_DLOG		4001

#define	SizeDLOG		5500	/* The number of lines config dialog ----------- */
#define	ColumnsNumber	4		/* NCSA: SB - User given # of columns */
#define	LinesNumber		6		/* User given # of lines */

#define	PrefDLOG	7001			/* The Preferences Dialog		---------------- */
#define PrefDClose				3	/* Don't close the window on "Close" */
#define PrefStag    			4	/* Staggered Windows? */
#define PrefCMDkey				5	/* Command keys */
#define PrefTMap				6	/* Shall we map the tilde? */
#define PrefBlink   			7	/* Blink the cursor? */
#define PrefBlockCursor   		8	/* Display a block cursor? */
#define PrefUnderscoreCursor	9	/* Display an underscore cursor? */
#define PrefVerticalCursor		10	/* Display a vertical line cursor? */
#define PrefCTt					13	/* Copy table threshold */
#define PrefTimeSlice			14	/* Multifinder Time Slice TxtEdit */
#define PrefCaptCreat			15	/* Capture File Creator Button */
#define	PrefCaptTE				17	/* Capture File Creator TE */

#define FTPDLOG		7002			/* The FTP config Dialog		---------------- */
#define	FTPServerOff			3	/* FTP server off button */
#define FTPServerUnsecure		4	/* FTP server ON Unsecure button */
#define FTPServerPswdPlease		5	/* FTP server Passwords Required */
#define FTPShowFTPlog			6	/* FTP show FTP log on startup */
#define FTPrevDNS				7	/* FTP reverse DNS IP of connection hosts */
#define FTPUseMacBinaryII		8	/* FTP Use Macbinary II */
#define FTPResetMacBinary		9	/* FTP Reset Macbinary after each ftp */
#define FTPISO					10	/* FTP ISO checkbox */
#define FTPbintypeTE			11	/* FTP binary filetype TextEdit */
#define FTPbincreatTE			12	/* FTP binary creator type TextEdit */
#define FTPbinexamplebutton		13	/* FTP binary example file selection button */
#define FTPtextcreatTE			14	/* FTP text creator type TextEdit */ 
#define FTPtextcreatorbutton	15	/* FTP TEXT file creator selection button */

#define TermDLOG	7003			/* The Terminal Config Dialog	---------------- */
#define TermANSI				3	/* ANSI escape sequences checkbox */
#define TermXterm				4	/* Xterm escape sequences checkbox */
#define Termvtwrap				5	/* Use vtwrap mode checkbox*/
#define Termmeta				6	/* Use emacs meta key checkbox */
#define Termarrow				7	/* Use emacs arrow keys checkbox */
#define TermMAT					8	/* Map pgup, etc. checkbox */
#define Termeightbit			9	/* Eight bit connection */
#define Termclearsave			10	/* Clear screen saves lines */
#define TermVT100				11	/* VT-100 emulation radio button */
#define TermVT220				12	/* VT-220 emulation radio button */
#define TermName				13	/* Terminal configuration name TE */
#define TermWidth				14	/* Terminal width TE */
#define TermHeight				15	/* Terminal height TE */
#define TermFontSize			16	/* Font size TE */
#define TermScrollback			17	/* Scrollback TE */
#define TermAnswerback			18	/* Answerback TE */
#define TermNFcolor				19	/* NF color user item */
#define TermNBcolor				20	/* NB color user item */
#define TermBFcolor				21	/* BF color user item */
#define TermBBcolor				22	/* BB color user item */
//#define TermUFcolor				23	/* UF color user item */
//#define TermUBcolor				24	/* UB color user item */
#define	TermFontPopup			23	/* Font menu popup item */
#define	TermNameStatText		24
#define	TermSafeItem			24

#define SessionConfigDLOG	7004	/* The Session Config Dialog	---------------- */
#define	SessTEKinhib			3	/* Inhibit TEK radio button */
#define SessTEK4014				4	/* TEK 4014 radio button */
#define	SessTEK4105				5	/* TEK 4105 radio button */
#define	SessPasteQuick			6
#define	SessPasteBlock			7
#define	SessDeleteDel			8
#define	SessDeleteBS			9
#define	SessForceSave			10
#define	SessBezerkeley			11
#define	SessLinemode			12
#define	SessTEKclear			13
#define	SessHalfDuplex			14
#define	SessLowLevelErrs		15
#define SessAuthenticate		16
#define SessEncrypt				17
#define	SessLocalEcho			18
#define SessAlias				19
#define	SessHostName			20
#define SessPort				21
#define	SessBlockSize			22
#define	SessInterrupt			23
#define	SessSuspend				24
#define	SessResume				25
#define	SessTermPopup			26
#define	SessTransTablePopup		27
#define	SessAliasStatText		29
#define	SessSafeItem			28

#define FTPUserDLOG			7005	/* The FTP User Config Dialog   ---------------- */
#define	FTPUcanchangeCWD		3	/* User can change WD checkbox */
#define FTPUusername			4	/* Username TE */
#define FTPUpassword			5	/* User's password */
#define	FTPUDfltDirDsply		6	/* User's default dir path display */
#define	FTPUDfltDirButton		7	/* Button to change user's default dir */
