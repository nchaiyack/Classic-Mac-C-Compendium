/******************************************************************************
**
**  Project Name:	DropShell
**     File Name:	DropShell.r
**
**   Description:	Resource definitions for DropShell
**
**					Since this is a shell, there are a LOT of places where the
**					specifics of the dropbox (and it's name) need to be placed.   
**					Currently a set of question marks (???) are there.  Use Search 
**					& Replace to replace them with your specific functionality.
**
*******************************************************************************
**                       A U T H O R   I D E N T I T Y
*******************************************************************************
**
**	Initials	Name
**	--------	-----------------------------------------------
**	LDR			Leonard Rosenthol
**
*******************************************************************************
**                      R E V I S I O N   H I S T O R Y
*******************************************************************************
**
**	  Date		Time	Author	Description
**	--------	-----	------	---------------------------------------------
**	12/10/91			LDR		Added an 'aete' for scripting support
**									since it needs special #include, conditionaled it
**	12/09/91			LDR		Added new Select File� menu item in File menu
**									and associated help resources & comments
**	11/24/91			LDR		Added some new comments about Balloon resources
**								Added new string for AEVT errors
**	10/28/91			LDR		Officially renamed DropShell (from QuickShell)
**								And added some comments
**	04/09/91	00:00	LDR		Original Version
**
******************************************************************************/
/* We must do this define in order to get some extra flags! */
#define SystemSevenOrLater 1
#include "Types.r"
#include "SysTypes.r"
#include "BalloonTypes.r"

/* comment the next line to NOT build the AETE */
#define BuildAETE 1
#ifdef BuildAETE
	#include "AEUserTermTypes.r"
#endif

/* First thing we need is the Finder Help Balloon */
/* Change the ??? to whatever your dropbox does (or completely change it!) */
resource 'hfdr' (-5696) {
	HelpMgrVersion, 0, 0, 0,
	{
		HMStringItem {
			"Drag any file onto this icon, and I will magically ???",
		},
	}
};

/* Splash Screen Goes Here */
/* 
	This is displayed all the time the program is running, so you do NOT want
	to give it a close box.  Apple's Blue HIG folks recommend having this so 
	that users can easily see what is running.  It is recommended that you put
	some about info, as well as simply instructions about using the product.
*/
resource 'WIND' (128) { {79,171,104,470}, movableDBoxProc, 
	visible, nogoAway, 0x0, "DropShell Splash Screen", centerMainScreen
};


resource 'PICT' (128, "Splash") {
	972,
	{0,0,25,299},
	$"1101 A030 39A0 0082 0100 0A00 0000 0000"
	$"1901 3099 0026 0000 0000 0019 0130 0000"
	$"0000 0019 012B 0000 0000 0019 012B 0000"
	$"000A 0000 0000 0019 012B 1602 FFFF C0FB"
	$"0001 3FFC F800 0201 FFFE FB00 01FF E0FA"
	$"001D 02FF FFC0 FB00 017F FEFD 0001 01FC"
	$"FE00 0401 FFFF 01F8 FD00 01FF F8FA 0026"
	$"FEFF 228F EFC3 FF80 00FF FEFF E007 8007"
	$"FF1F FF00 01FF FF07 FE03 FC00 00FF FC3F"
	$"FE7F 03F7 FFE0 2725 5FFF FF8F EFC7 FFC0"
	$"00FF FEFF F00F C00F FF9F FF80 01FF FF1F"
	$"FF87 FF00 00FF FE3F FF7F 03F7 FFE0 2725"
	$"5FFD DF87 EFC7 FFC0 01FF FEFF F80F C01F"
	$"FFDF FF80 00FE 7F9F FFC7 FF80 00FF FE3F"
	$"FF7F 83E6 FFE0 2725 5FF0 9F03 EFCF FFC0"
	$"01FC 6E7F FC1F C03F FFCF 9F80 00FC 63BF"
	$"EFC7 FF80 00FF FF1F 3F3F 83E2 FF60 2725"
	$"03F0 9F03 E7CF CDC0 00FC 4E7F FE1F E03E"
	$"2BCF 0980 00F8 033F EFE7 FFC0 007E FF9E"
	$"133F C3C2 FE00 2725 03F0 1F03 E78F C4C0"
	$"00FC 467C BE3F E07C 2BEF 0180 00F8 007C"
	$"6CE3 CFC0 007C CF9E 033F E3C0 9E00 2725"
	$"03F0 1F03 E787 E4C0 007E 067C 1E3C F078"
	$"02EF 8000 007C 0078 00F3 CDE0 007C 0F9F"
	$"003F E3C0 1E00 2725 03E0 0F03 E787 F0C0"
	$"003F 003C 1E7C 7078 0067 FE00 007F FC78"
	$"00F3 C0E0 007C 0F8F FC3F E3C0 1C00 2725"
	$"01E0 0E01 E783 F800 001F 803C 1E78 7870"
	$"0027 FE00 007F FC70 0073 E0C0 007C 0F0F"
	$"FC3F F3C0 1C00 2725 01E0 0F01 E7C1 FC00"
	$"000F E01C 3EF0 3870 0003 C400 003E 1870"
	$"0071 E0C0 003C 0F07 883C F3C0 1C00 2725"
	$"01E0 0FFF E3C0 FE00 0007 F03C 7AF0 1C70"
	$"0007 8400 003C 1870 0033 E180 007C 1E0F"
	$"081E 7380 1C00 2725 01E0 0FFF C380 7F00"
	$"0007 F83F F0FF FC30 0003 8400 007C 1030"
	$"0033 E280 007C 7A07 081E 3B80 3C00 2725"
	$"01E0 0F6B 8380 7F00 0003 781F D0FF FC38"
	$"0003 8000 003C 0030 0021 FC40 003F F207"
	$"001E 3F80 3C00 2725 01E0 0701 8384 2F80"
	$"00C2 3C1E 50FF 7C18 0003 8000 003C 0018"
	$"0061 FE00 003F F207 001E 1F80 3C00 2725"
	$"01E0 0703 8306 0780 00E0 3C1C 00E0 1C1C"
	$"00C1 8600 003C 001C 0061 8F00 0038 F003"
	$"0C1E 0F80 3C00 2725 01E0 0601 8307 0780"
	$"0070 3C18 00E0 1C1F 0381 FC00 001C 000F"
	$"01A1 8F00 0038 7803 F816 0380 0C00 2725"
	$"01E0 0401 8303 8F00 0078 7818 00E0 0C0F"
	$"FF01 FC00 001E 000F FF21 8380 0018 7803"
	$"F816 0300 0C00 2725 00E0 0601 8101 FE00"
	$"003F F018 00E0 0809 F901 E400 001E 0009"
	$"FF01 C080 0018 7C03 C806 0300 1800 2719"
	$"00E0 0401 8100 F800 001F E010 00C0 0800"
	$"F000 8000 000E 0000 7C01 FE00 081C 0C01"
	$"0006 0000 1000 2410 00C0 0000 8000 5000"
	$"000F C010 0040 0800 60FD 0004 0E00 0018"
	$"01FE 0004 1C06 0000 02FD 0016 0100 C0FD"
	$"0004 4000 0005 80F7 0000 0CFA 0001 1804"
	$"FA00 1401 0080 FD00 0340 0000 04F6 0000"
	$"04FA 0000 10F9 0006 F800 0004 E500 A000"
	$"83FF"
};




/* Menus & Menu Help Resources Go Here */
/* You will probably want to put the application name here! */
resource 'MENU' (128, "Apple") { 128, textMenuProc, 0x7FFFFFFD, enabled, apple, 
	{	/* array: 2 elements */
		/* [1] */	"About ???�", noIcon, noKey, noMark, plain,
		/* [2] */	"-", noIcon, noKey, noMark, plain
	}
};

/* You'll probably want to replace Select File with something particular about your app */
resource 'MENU' (129, "File") { 129, textMenuProc, allEnabled, enabled, "File", 
	{	/* array: 2 elements */
		/* [1] */	"Select File�",	noIcon, "O", noMark, plain,
		/* [2] */	"Quit", 		noIcon, "Q", noMark, plain
	}
};

/* 
	These are the help resources for the menus.  You should not need to change
	these unless you add or delete menu items.  If you wish to change the text
	that is displayed, see the STR# with the same ID
*/
resource 'hmnu' (128, purgeable) {	/* Apple */
	HelpMgrVersion, 0, 0, 0,
	HMSkipItem {	/* no missing items */	},
	{	/* array HMenuArray: 2 elements */
		HMSkipItem		{	/* skip the menu title */ },
		HMStringResItem {	/* About Item */
			128, 1,
			128, 1,
			0, 0,
			0, 0
		}
	}
};

resource 'hmnu' (129, purgeable) {	/* File */
	HelpMgrVersion, 0, 0, 0,
	HMSkipItem {	/* no missing items */	},
	{	/* array HMenuArray: 4 elements */
		HMStringResItem {	/* Menu Title */
			129, 1,
			129, 2,
			129, 3,
			129, 4
		},
		HMStringResItem {	/* Open File� */
			129, 5,
			129, 6,
			0, 0,
			0, 0
		},
		HMStringResItem {	/* Quit */
			129, 7,
			129, 8,
			0, 0,
			0, 0
		},
	}
};

/* How about the About box? */
/* Of course, you will modify this for your application */
resource 'ALRT' (128, purgeable) { {75, 79, 198, 354}, 128,
	{	/* array: 4 elements */
		/* [1] */	OK, visible, silent,
		/* [2] */	OK, visible, silent,
		/* [3] */	OK, visible, silent,
		/* [4] */	OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (128, purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */ {97, 198, 117, 270}, Button { enabled, "OK" }, 
		/* [2] */ {8, 8, 87, 271}, StaticText { disabled,
			"DropShell - the simplest program possible with the DS Technology!" },
		/* [3] */ {0, 0, 0, 0}, HelpItem { disabled, HMScanhdlg { 1 } } 
	}
};

/* 
	This is the help resource for the about box.  Assuming you change
	the contents of the about box, you will NEED to change this resource.
	I suggest that you consult the Help Manager chapter of IM VI for
	an explanation of how these resources work.
*/
resource 'hdlg' (128, purgeable) {
	HelpMgrVersion, 0, hmSaveBitsNoWindow, 0, 3,	/* need bits saved in Alert, since the OK ring & icons are not updated */
	HMSkipItem	{ /* no missing items */ },
	{
		HMStringResItem {	/* OK button */
			{0,0},		/* default tip */
			{0,0,0,0},	/* alternate rect */
			128, 2,		/* res ID's as normal */
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem {	/* StatText */
			{0,0},		/* default tip */
			{0,0,0,0},	/* alternate rect */
			0, 0,		
			128, 3,		/* stat text is disabled! */
			0, 0,
			0, 0
		}
	}
};

/* Here are the STR#'s that the Help text is stored in! */
resource 'STR#' (128,purgeable) {	/* help items for Apple Menu */
	{ /* array StringArray: 3 elements */
		/* [1] */ 	"Everything you always wanted to know about ???, but were afraid to ask",
		/* [2] */ 	"Click here to dismiss this informative dialog.",
		/* [3] */ 	"This text describes the product, its author, and why!"
	}
};

resource 'STR#' (129,purgeable) {	/* help items for File Menu */
	{ /* array StringArray: ? elements */
		/* [1] */	"Use this menu to select a file and to exit the program.",
		/* [2] */ 	"Use this menu to select a file and to exit the program. "
				  	"This menu is unavailable now.",
		/* [3] */ 	"Use this menu to select a file and to exit the program. "
					"This menu is unavailable until you respond to the alert box or dialog box.",
		/* [4] */ 	"This command is unavailable until you respond to the alert box or dialog box.",
		/* [5] */ 	"Use this command to select a file.",
		/* [6] */ 	"Use this command to select a file. "
					"Not available for some strange reason.",
		/* [7] */ 	"Use this command to exit the program.",
		/* [8] */ 	"Use this command to exit the program. "
					"Not available for some strange reason."
	}
};

/* Error Alert Stuff */
resource 'ALRT' (200, purgeable) {
	{100, 120, 224, 452}, 200,
	{	/* array: 4 elements */
		/* [1] */	OK, visible, silent,
		/* [2] */	OK, visible, silent,
		/* [3] */	OK, visible, silent,
		/* [4] */	OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (200, purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */	{92, 246, 112, 318}, Button { enabled, "OK" }, 
		/* [2] */ 	{11, 73, 75, 318}, StaticText { disabled, "^0" }, 
		/* [3] */ 	{11, 11, 43, 43}, Icon { disabled, 0 }, 
		/* [4] */ 	{96, 11, 112, 100}, StaticText { disabled, "Error #^1" } 
	}
};

/* 
	This is the help resource for the ErrorAlert above.  
	You should not need to change this.
*/
resource 'hdlg' (200, purgeable) {
	HelpMgrVersion, 0, hmSaveBitsNoWindow, 0, 3,	/* need bits saved in Alert, since the OK ring & icons are not updated */
	HMSkipItem	{ /* no missing items */ },
	{
		HMStringResItem {	/* OK button */
			{0,0},		/* default tip */
			{0,0,0,0},	/* alternate rect */
			200, 1,		/* res ID's as normal */
			0, 0,
			0, 0,
			0, 0
		},
		HMStringResItem {	/* StatText - error message*/
			{0,0},		/* default tip */
			{0,0,0,0},	/* alternate rect */
			0, 0,		
			200, 2,		/* stat text is disabled! */
			0, 0,
			0, 0
		},
		HMSkipItem		{	/* icon */ },
		HMStringResItem {	/* StatText - error ID*/
			{0,0},		/* default tip */
			{0,0,0,0},	/* alternate rect */
			0, 0,		
			200, 3,		/* stat text is disabled! */
			0, 0,
			0, 0
		},
	}
};

resource 'STR#' (100) {	/* error alert messages! */
	{	/* array StringArray: 1 elements */
		/* [1] */	"This application requires Apple events.  Please upgrade to System 7.0 or later to use.",
		/* [2] */	"An error occured during Apple event processing."
	}
};

resource 'STR#' (200, purgeable) {	/* help info for the Error Dialog */
	{	/* array StringArray: 3 elements */
		/* [1] */	"Click here to dismiss this informative dialog.",
		/* [2] */	"This text describes the error that occured.",
		/* [3] */	"This error ID gives more specific details of the error."
	}
};

resource 'vers' (1) {
	0x01,	/* major revision */
	0x00,	/* minor revision */
	release,
	0x00,	/* non-final release version */
	verUS,	/* country code */
	"1.0",	/* short vers string */
	"Version 1.0"
};

resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	102400,
	102400
};

resource 'SIZE' (0) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	102400,
	102400
};

resource 'FREF' (128) { 'APPL', 0, "" }; 
resource 'FREF' (129) { '****', 1, "" }; 

/* 	if you plan to support folders & disks then uncomment the next two lines 
	and add the appropriate lines to the BNDL as well!

	resource 'FREF' (130) { 'fold', 2, "" }; 
	resource 'FREF' (131) { 'disk', 3, "" }; 
*/

/* Don't forget to give yourself a REAL creator type! */
/* Then modify the BNDL & the next resource for the old vers string */
resource 'BNDL' (128, "Bundle") {
	'????',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'FREF',
		{	/* array IDArray: 2 elements */
			/* [1] */ 0, 128,
			/* [2] */ 1, 129
		},
		/* [2] */
		'ICN#',
		{	/* array IDArray: 2 elements */
			/* [1] */ 0, 128,
			/* [2] */ 1, 0
		}
	}
};

data '????' (0) {
	""
};


#ifdef BuildAETE

/*
	This is the KEY resource in supporting AEVTs - it is used by scripting/macro
	systems such as Control Tower & QuicKeys in determing what events an application
	supports.
	
	In this one, we simply tell it we support the required events, since that is all
	we do.  If you add additional events, you MUST consult the AETE documentation
	distributed by Apple.  Make sure you find the RELEASE NOTES, since this resource
	has undergone a NUMBER of chanes.
*/
resource 'aete' (0, "Apple Events Terminology") {

	0x00,			/* major version number in BCD */
	0x90,			/* minor version number in BCD. From 7/9/91 version of AEUT docs */
	english,		/* descriptions are in english */
	roman,			/* use roman language script system */
	
	{	/* array Suites: 1 elements */
	
		/*******************************************************
		[1] -- The part that show we do the required AppleEvents
		*******************************************************/

		"Required Suite",
		"Events that every application should support",
		'reqd',
		1,			/* suite level */
		1,			/* suite version */
		
		{
			/* array Events: 0 elements */
		},
		
		{
			/* array Classes: 0 elements */
		},
		
		{
			/* array ComparisonOps: 0 elements */
		},
		
		{
			/* array Enumerations: 0 elements */
		},
	}
};

#endif
