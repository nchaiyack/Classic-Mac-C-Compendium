/*______________________________________________________________________

	rez.r - Sample Resource File Definitions.
	
	Copyright � 1988, 1989, 1990, Northwestern University.
	
	These definitions must exactly match the constant definitions in
	the C header file rez.h.
_____________________________________________________________________*/


#include "Types.r"
#include "SysTypes.r"


/* Include pictures and sounds. */

include	"picts";
include	"sounds";



/* Menus. */
   
#define AllItems	 	0b1111111111111111111111111111111
#define MenuItem1	 	0b0000000000000000000000000000001
#define MenuItem2	 	0b0000000000000000000000000000010
#define MenuItem3	 	0b0000000000000000000000000000100
#define MenuItem4	 	0b0000000000000000000000000001000
#define MenuItem5 	0b0000000000000000000000000010000
#define MenuItem6 	0b0000000000000000000000000100000
#define MenuItem7	 	0b0000000000000000000000001000000
#define MenuItem8		0b0000000000000000000000010000000
#define MenuItem9	 	0b0000000000000000000000100000000
#define MenuItem10	0b0000000000000000000001000000000
#define MenuItem11	0b0000000000000000000010000000000
#define MenuItem12	0b0000000000000000000100000000000
#define MenuItem13	0b0000000000000000001000000000000
#define MenuItem14	0b0000000000000000010000000000000
#define MenuItem15	0b0000000000000000100000000000000

resource 'MBAR' (128, preload) {
	{128, 129, 130, 131}
};

resource 'MENU' (128, "Apple", preload) {
	128, 
	textMenuProc,
	AllItems & ~MenuItem3,
	enabled, 
	apple,
	{
		"About Sample",
			noicon, nokey, nomark, plain;
		"Sample Help",
			noicon, "H", nomark, plain;
		"-",
			noicon, nokey, nomark, plain
	}
};

resource 'MENU' (129, "File", preload) {
	129, 
	textMenuProc,
	AllItems & ~MenuItem1 & ~MenuItem3 & ~MenuItem7 & ~MenuItem9,
	enabled, 
	"File",
	{
		"Close",
			noicon, "W", nomark, plain;
		"Save As�",
			noicon, "S", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Page Setup�",
			noicon, nokey, nomark, plain;
		"Print�",
			noicon, "P", nomark, plain;
		"Print One",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Preferences",
			noicon, nokey, nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Quit",
			noicon, "Q", nomark, plain
	}
};

resource 'MENU' (130, "Edit", preload) {
	130, 
	textMenuProc,
	AllItems & ~MenuItem1 & ~MenuItem2 & ~MenuItem3 & ~MenuItem4 & ~MenuItem5,
	enabled, 
	"Edit",
	 {
		"Undo",
			noicon, "Z", nomark, plain;
		"-",
			noicon, nokey, nomark, plain;
		"Cut",
			noicon, "X", nomark, plain;
		"Copy",
			noicon, "C", nomark, plain;
		"Paste",
			noicon, "V", nomark, plain;
		"Clear",
			noicon, nokey, nomark, plain
	}
};

resource 'MENU' (131, "Scan", preload) {
	131, 
	textMenuProc,
	AllItems,
	enabled, 
	"Scan",
	 {
		"File�",
			noicon, nokey, nomark, plain;
		"Folder�",
			noicon, nokey, nomark, plain;
		"Floppies",
			noicon, nokey, nomark, plain;
		"All Disks",
			noicon, nokey, nomark, plain;
		"Some Disks�",
			noicon, nokey, nomark, plain;
		"System File",
			noicon, nokey, nomark, plain;
		"System Folder",
			noicon, nokey, nomark, plain;
		"Desktop Files",
			noicon, nokey, nomark, plain;
	}
};


/*	Main window. */
	
#define	windRight	492
#define	windBot		295

#define	drive			128
#define	eject			129
#define 	scan			130
#define	cancel		131
#define	quit			132
#define	reset			133

#define	butTop		182
#define	butLeft		312	
#define	butWidth		77
#define	butHeight	20
#define	butHSep		90
#define	butVSep		30

#define	butTop2		butTop+butVSep
#define	butTop3		butTop2+butVSep
#define	butLeft2		butLeft+butHSep

#define	resetTop		134
#define	resetLeft	438
#define	resetWidth	33
#define	resetHeight	14

resource 'WIND' (128, purgeable, preload) {
	{0, 0, windBot, windRight},
	zoomDocProc,
	invisible,
	noGoAway,
	0x0,
	"Sample"
};

resource 'CNTL' (drive, purgeable, preload) {
	{butTop, butLeft, butTop+butHeight, butLeft+butWidth},
	0,
	visible,
	0,
	0,
	pushButProc,
	drive,
	"Drive"
};

resource 'CNTL' (eject, purgeable, preload) {
	{butTop, butLeft2, butTop+butHeight, butLeft2+butWidth},
	0,
	visible,
	0,
	0,
	pushButProc,
	eject,
	"Eject"
};

resource 'CNTL' (scan, purgeable, preload) {
	{butTop2, butLeft, butTop2+butHeight, butLeft+butWidth},
	0,
	visible,
	0,
	0,
	pushButProc,
	scan,
	"Scan"
};

resource 'CNTL' (cancel, purgeable, preload) {
	{butTop3, butLeft, butTop3+butHeight, butLeft+butWidth},
	0,
	visible,
	0,
	0,
	pushButProc,
	cancel,
	"Cancel"
};

resource 'CNTL' (quit, purgeable, preload) {
	{butTop3, butLeft2, butTop3+butHeight, butLeft2+butWidth},
	0,
	visible,
	0,
	0,
	pushButProc,
	quit,
	"Quit"
};

resource 'CNTL' (reset, purgeable, preload) {
	{resetTop, resetLeft, resetTop+resetHeight, resetLeft+resetWidth},
	0,
	visible,
	0,
	0,
	pushButProcUseWFont,
	reset,
	"Reset"
};



/* Help window. */

#define	helpRight		windRight
#define	helpBot			windBot

resource 'WIND' (129) {
	{0, 0, helpBot, helpRight},
	zoomDocProc,
	invisible,
	goAway,
	0x0,
	"Help"
};


/* Peferences window. */

resource 'DLOG' (400) {
	{0, 0, windBot, windRight},
	noGrowDocProc,
	invisible,
	goAway,
	0x0,
	400,
	"Preferences"
};

resource 'DITL' (400) {
	{	/* array DITLarray: 33 elements */
		/* [1] */
		{11, 42, 30, 101},
		StaticText {
			disabled,
			"Beeping"
		},
		/* [2] */
		{27, 47, 45, 95},
		StaticText {
			disabled,
			"option"
		},
		/* [3] */
		{75, 10, 93, 125},
		StaticText {
			disabled,
			"Scanning station"
		},
		/* [4] */
		{91, 43, 110, 97},
		StaticText {
			disabled,
			"options"
		},
		/* [5] */
		{146, 31, 162, 107},
		StaticText {
			disabled,
			"Saved text"
		},
		/* [6] */
		{162, 29, 179, 109},
		StaticText {
			disabled,
			"file options"
		},
		/* [7] */
		{226, 27, 244, 108},
		StaticText {
			disabled,
			"Background"
		},
		/* [8] */
		{242, 27, 258, 111},
		StaticText {
			disabled,
			"notification"
		},
		/* [9] */
		{258, 42, 276, 95},
		StaticText {
			disabled,
			"options"
		},
		/* [10] */
		{55, 10, 56, 482},
		UserItem {
			disabled
		},
		/* [11] */
		{122, 10, 123, 482},
		UserItem {
			disabled
		},
		/* [12] */
		{205, 10, 206, 482},
		UserItem {
			disabled
		},
		/* [13] */
		{10, 128, 45, 129},
		UserItem {
			disabled
		},
		/* [14] */
		{65, 128, 112, 129},
		UserItem {
			disabled
		},
		/* [15] */
		{132, 128, 195, 129},
		UserItem {
			disabled
		},
		/* [16] */
		{215, 128, 285, 129},
		UserItem {
			disabled
		},
		/* [17] */
		{22, 150, 41, 187},
		StaticText {
			disabled,
			"Beep"
		},
		/* [18] */
		{21, 194, 37, 217},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{22, 226, 41, 457},
		StaticText {
			disabled,
			"times when infection discovered"
		},
		/* [20] */
		{68, 151, 89, 469},
		CheckBox {
			enabled,
			"Scanning station with no mouse or keyboa"
			"rd"
		},
		/* [21] */
		{93, 218, 113, 278},
		RadioButton {
			enabled,
			"Scan"
		},
		/* [22] */
		{93, 300, 113, 383},
		RadioButton {
			enabled,
			"Disinfect"
		},
		/* [23] */
		{142, 159, 158, 282},
		StaticText {
			disabled,
			"Reports belong to"
		},
		/* [24] */
		{168, 155, 184, 282},
		StaticText {
			disabled,
			"Manual belongs to"
		},
		/* [25] */
		{142, 283, 158, 410},
		UserItem {
			enabled
		},
		/* [26] */
		{168, 283, 184, 410},
		UserItem {
			enabled
		},
		/* [27] */
		{142, 423, 158, 470},
		EditText {
			enabled,
			""
		},
		/* [28] */
		{168, 423, 184, 470},
		EditText {
			enabled,
			""
		},
		/* [29] */
		{224, 162, 278, 234},
		Picture {
			disabled,
			130
		},
		/* [30] */
		{222, 238, 238, 421},
		RadioButton {
			enabled,
			"Only display \0x13 in \0x14 menu"
		},
		/* [31] */
		{241, 238, 257, 453},
		RadioButton {
			enabled,
			"Also display icon in menu bar"
		},
		/* [32] */
		{260, 238, 276, 376},
		RadioButton {
			enabled,
			"Also display alert"
		},
		/* [33] */
		{0, 0, 0, 0},
		UserItem {
			disabled
		},
	}
};

/* Prefs window popup menus.  The menu items must be in one-to-one
	correspondence with the creator types in the CREA resources.
	
	Resource id 200 is for saved reports.
	Resource id 201 is for saved manual. */

resource 'MENU' (200, "") {
	200, 
	textMenuProc,
	AllItems,
	enabled, 
	"",
	{
		"Edit",
			noicon, nokey, nomark, plain;
		"FullWrite",
			noicon, nokey, nomark, plain;
		"MacWrite",
			noicon, nokey, nomark, plain;
		"MacWrite II",
			noicon, nokey, nomark, plain;
		"McSink/Vantage",
			noicon, nokey, nomark, plain;
		"microEmacs",
			noicon, nokey, nomark, plain;
		"MindWrite",
			noicon, nokey, nomark, plain;
		"MPW",
			noicon, nokey, nomark, plain;
		"MS Word",
			noicon, nokey, nomark, plain;
		"Nisus",
			noicon, nokey, nomark, plain;
		"QUED/M",
			noicon, nokey, nomark, plain;
		"TeachText",
			noicon, nokey, nomark, plain;
		"WordPerfect",
			noicon, nokey, nomark, plain;
		"WriteNow",
			noicon, nokey, nomark, plain;
		"Other",
			noicon, nokey, nomark, plain;
	}
};

resource 'MENU' (201, "") {
	201, 
	textMenuProc,
	AllItems,
	enabled, 
	"",
	{
		"Edit",
			noicon, nokey, nomark, plain;
		"FullWrite",
			noicon, nokey, nomark, plain;
		"MacWrite II",
			noicon, nokey, nomark, plain;
		"McSink/Vantage",
			noicon, nokey, nomark, plain;
		"microEmacs",
			noicon, nokey, nomark, plain;
		"MPW",
			noicon, nokey, nomark, plain;
		"MS Word",
			noicon, nokey, nomark, plain;
		"Nisus",
			noicon, nokey, nomark, plain;
		"QUED/M",
			noicon, nokey, nomark, plain;
		"WordPerfect",
			noicon, nokey, nomark, plain;
		"WriteNow",
			noicon, nokey, nomark, plain;
		"Other",
			noicon, nokey, nomark, plain;
	}
};

/* Prefs window creator lists.  The creator types must be in one-to-one
	correspondence with the menu items in the popup menus. */

type 'CREA' {
	array {
		literal longint;
	};
};

resource 'CREA' (200) {
	{
		'EDIT',
		'FWRT',
		'MACA',
		'MWII',
		'McSk',
		'Earl',
		'PWRI',
		'MPS ',
		'MSWD',
		'NISI',
		'QED1',
		'ttxt',
		'SSIW',
		'nX^n',
		'????',
	},
};

resource 'CREA' (201) {
	{
		'EDIT',
		'FWRT',
		'MWII',
		'McSk',
		'Earl',
		'MPS ',
		'MSWD',
		'NISI',
		'QED1',
		'SSIW',
		'nX^n',
		'????',
	},
};


/* About window. */

#define	abtRight		windRight
#define	abtBot		windBot

resource 'WIND' (130, purgeable) {
	{0, 0, abtBot, abtRight},
	noGrowDocProc,
	invisible,
	goAway,
	0x0,
	"About"
};


/*	Rectangles.  A single resource is defined containing an array of 
	all the rectangles used by the program.
*/
	
type 'RCT#' {
	array {
		rect;
	};
};

#define	volNameTop		18
#define	nameSep			23
#define	foldNameTop		volNameTop+nameSep
#define	fileNameTop		foldNameTop+nameSep
#define	nameLeft			butLeft+20
#define	nameRight		butLeft2+butWidth
#define	nameHeight		16
#define	sicnLeft			butLeft
#define	volIconTop		volNameTop
#define	foldIconTop		foldNameTop-1
#define	fileIconTop		fileNameTop-1
#define	thermTop			91
#define	thermHeight		9
#define	thermLeft		butLeft
#define	thermRight		butLeft2+butWidth
#define	counterTop		121
#define	counterRight	377
#define	reportTop		8
#define	reportBot		286
#define	reportLeft		12
#define	reportWidth		285
#define	reportRight		reportLeft+reportWidth

resource 'RCT#' (128, preload) {
	{
		{volIconTop, sicnLeft, volIconTop+16, sicnLeft+16},
											/* volume small icon */
		{volNameTop, nameLeft, volNameTop+nameHeight, nameRight},
											/* volume name */
		{foldIconTop, sicnLeft, foldIconTop+16, sicnLeft+16},
											/* folder small icon */
		{foldNameTop, nameLeft, foldNameTop+nameHeight, nameRight},
											/* folder name */
		{fileIconTop, sicnLeft, fileIconTop+16, sicnLeft+16},
											/* file small icon */
		{fileNameTop, nameLeft, fileNameTop+nameHeight, nameRight},
											/* file name */
		{thermTop, thermLeft, thermTop+thermHeight, thermRight},
											/* thermometer */
		{counterTop, 0, 0, counterRight},
											/* counters */
		{reportTop, reportLeft, reportBot, reportRight},
											/* report */
		{10, 12, 288, 374},   		/* help window scrolling field */
		{25, 386, 267, 483},    	/* help window table of contents */
		{15, 62, 147, 181},   		/* about box picture of me */
		{160, 22, 1000, 234}, 		/* about box credits rect */
		{0, 243, 164, 480},			/* about box foot */
	},
};


/* Report full alert. */

resource 'ALRT' (128) {
	{0, 0, 98, 370},
	128,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (128) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{68, 300, 88, 360},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{68, 227, 88, 287},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{68, 154, 88, 214},
		Button {
			enabled,
			"Clear"
		},
		/* [4] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"The report is too big. It must be saved or cleared before"
			" the scan can continue.  Save the report?"
		}
	}
};


/*  System too early alert. */

resource 'ALRT' (129) {
	{0, 0, 66, 360},
	129,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (129) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{36, 290, 56, 350},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 23, 350},
		StaticText {
			disabled,
			"Sample requires System 3.2 or later."
		}
	}
};


/* Save report before quitting alert. */

resource 'ALRT' (130) {
	{0, 0, 66, 300},
	130,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (130) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{36, 230, 56, 290},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{36, 157, 56, 217},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{36, 84, 56, 144},
		Button {
			enabled,
			"Quit"
		},
		/* [4] */
		{7, 74, 23, 290},
		StaticText {
			disabled,
			"Save report before quitting?"
		}
	}
};


/* Unlock and resinsert disk alert. */

resource 'ALRT' (131) {
	{0, 0, 98, 370},
	131,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (131) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{68, 400, 88, 401},
		Button {
			disabled,
			""
		},
		/* [2] */
		{68, 300, 88, 360},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"The disk cannot be repaired because it i"
			"s locked.  Please unlock and reinsert th"
			"e disk."
		}
	}
};


/* Disk full alert. */

resource 'ALRT' (132) {
	{0, 0, 82, 370},
	132,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (132) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"The file could not be saved because the "
			"disk is full."
		}
	}
};


/* Restart alert. */

resource 'ALRT' (133) {
	{0, 0, 114, 480},
	133,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (133) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{84, 410, 104, 470},
		Button {
			enabled,
			"Restart"
		},
		/* [2] */
		{84, 337, 104, 397},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{84, 264, 104, 324},
		Button {
			enabled,
			"Quit"
		},
		/* [4] */
		{7, 74, 71, 470},
		StaticText {
			disabled,
			"A virus may still be active in memory."
			"  Some of your files may have or could "
			"become reinfected.  "
			"You should immediately restart your Maci"
			"ntosh using a locked virus tools floppy "
			"and run Sample again."
		},
	}
};


/* Old restart alert. */

resource 'ALRT' (134) {
	{0, 0, 114, 480},
	134,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (134) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{84, 410, 104, 470},
		Button {
			enabled,
			"Cancel"
		},
		/* [2] */
		{84, 337, 104, 397},
		Button {
			enabled,
			"Quit"
		},
		/* [3] */
		{7, 74, 71, 470},
		StaticText {
			disabled,
			"A virus may still be active in memory."
			" Some of your files may have or could "
			"become reinfected. "
			"You should immediately restart your Maci"
			"ntosh using a locked virus tools floppy "
			"and run Sample again."
		},
	}
};

/* Out of memory alert. */

resource 'ALRT' (135, preload) {
	{0, 0, 66, 190},
	135,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (135, preload) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{36, 120, 56, 180},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 23, 180},
		StaticText {
			disabled,
			"Out of memory."
		}
	}
};


/* Need HFS alert. */

resource 'ALRT' (136) {
	{0, 0, 82, 370},
	136,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (136) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"Sample requires the hierarchical f"
			"ile system (HFS)."
		}
	}
};

/* Unable to repair alert. */

resource 'ALRT' (137) {
	{0, 0, 274, 460},
	137,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (137) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{244, 390, 264, 450},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 72, 23, 450},
		StaticText {
			disabled,
			"Sample is unable to repair files o"
			"n this system."
		},
		/* [3] */
		{39, 72, 71, 450},
		StaticText {
			disabled,
			"One possible reason is that you are usin"
			"g GateKeeper and you forgot to grant Dis"
			"infectant privileges."
		},
		/* [4] */
		{87, 72, 167, 450},
		StaticText {
			disabled,
			"Another possible reason is that you are "
			"using the special University of Michigan"
			" version of Vaccine (Vaccine.UofM).  You "
			"must remove this version of Vaccine from"
			" your system folder before using Disinfe"
			"ctant to repair files."
		},
		/* [5] */
		{183, 72, 231, 450},
		StaticText {
			disabled,
			"You may use Sample on this system "
			"to check for viruses, but you will not b"
			"e able to use the Disinfect button to re"
			"pair infected files."
		}
	}
};


/* Save report before clearing alert. */

resource 'ALRT' (138) {
	{0, 0, 66, 300},
	138,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (138) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{36, 230, 56, 290},
		Button {
			enabled,
			"Save"
		},
		/* [2] */
		{36, 157, 56, 217},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{36, 84, 56, 144},
		Button {
			enabled,
			"Clear"
		},
		/* [4] */
		{7, 74, 23, 290},
		StaticText {
			disabled,
			"Save report before clearing?"
		}
	}
};


/* Printing error alert. */

resource 'ALRT' (139) {
	{0, 0, 66, 320},
	139,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (139) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{36, 250, 56, 310},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 23, 310},
		StaticText {
			disabled,
			"Printing error (error code = ^0)."
		}
	}
};


/* No printer driver alert. */

resource 'ALRT' (140) {
	{0, 0, 82, 370},
	140,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (140) {
	{	/* array DITLarray: 2 elements */
		/* [1]*/
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"Printing error�could not locate printer driver in"
			" system folder."
		}
	}
};

/* Illegal font size alert. */

resource 'ALRT' (141) {
	{0, 0, 98, 370},
	141,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (141) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{68, 300, 88, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"The font size must be in the range ^0 "
			"through ^1.  Please correct it or click "
			"on the Cancel button."
		}
	}
};

/* No pages to print alert. */

resource 'ALRT' (142) {
	{0, 0, 82, 370},
	142,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (142) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"You selected the page range ^0 through ^1.  "
			"There are no pages in this range."
		}
	}
};

/* Margins too big alert. */

resource 'ALRT' (143) {
	{0, 0, 98, 370},
	143,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (143) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{68, 300, 88, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"The margins you specified are too large.  "
			"Please make them smaller or click on the "
			"Cancel button."
		}
	}
};

/* Unlock and reinsert, SS alert */

resource 'ALRT' (144) {
	{0, 0, 65, 370},
	144,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (144) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{100, 100, 101, 101},
		Button {
			disabled,
			""
		},
		/* [2] */
		{100, 100, 101, 101},
		Button {
			disabled,
			""
		},
		/* [3] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"The disk cannot be repaired because it i"
			"s locked.  Please unlock and reinsert th"
			"e disk or insert the next disk to be re"
			"paired."
		}
	}
};

/*	Restart after installing INIT alert. */

resource 'ALRT' (145) {
	{0, 0, 178, 470},
	145,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (145) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{148, 327, 168, 387},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{148, 400, 168, 460},
		Button {
			enabled,
			"Restart"
		},
		/* [3] */
		{7, 74, 39, 460},
		StaticText {
			disabled,
			"The Sample protection INIT has bee"
			"n installed.  You must restart your Macin"
			"tosh to activate the INIT."
		},
		/* [4] */
		{55, 74, 87, 460},
		StaticText {
			disabled,
			"WARNING: If you restart now, you will lose all changes "
			"to any documents which may be open in other applications!"
		},
		/* [5] */
		{103, 74, 135, 460},
		StaticText {
			disabled,
			"To restart now, select the Restart button.  To return to "
			"Sample without restarting, select the OK button."
		},
	}
};

/* Restart after installing INIT alert for old systems. */

resource 'ALRT' (146) {
	{0, 0, 98, 370},
	146,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (146) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{68, 300, 88, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"The Sample protection INIT has bee"
			"n installed.  You must restart your Macin"
			"tosh to activate the INIT."
		}
	}
};

/* Replace existing INIT file alert. */

resource 'ALRT' (147) {
	{0, 0, 66, 420},
	147,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (147) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{36, 277, 56, 337},
		Button {
			enabled,
			"Cancel"
		},
		/* [2] */
		{36, 350, 56, 410},
		Button {
			enabled,
			"Replace"
		},
		/* [3] */
		{7, 74, 23, 410},
		StaticText {
			disabled,
			"Replace existing �^0�?"
		}
	}
};


/* Disk locked alert. */

resource 'ALRT' (148) {
	{0, 0, 82, 370},
	148,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (148) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"The protection INIT could not be installed b"
			"ecause the startup disk is locked."
		}
	}
};


/* Unexpected error on file save alert. */

resource 'ALRT' (149) {
	{0, 0, 82, 370},
	149,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (149) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"An unexpected error (^0) occurred while trying to sa"
			"ve a file."
		}
	}
};


/* No printer selected alert. */

resource 'ALRT' (150) {
	{0, 0, 82, 370},
	150,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (150) {
	{	/* array DITLarray: 2 elements */
		/* [1]*/
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"Printing error�you must use the Chooser to select "
			"a printer."
		}
	}
};


/* Printing disk full alert. */

resource 'ALRT' (151) {
	{0, 0, 66, 370},
	151,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (151) {
	{	/* array DITLarray: 2 elements */
		/* [1]*/
		{36, 300, 56, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 23, 370},
		StaticText {
			disabled,
			"Printing error�the startup disk is full."
		}
	}
};

/* Printing disk locked alert. */

resource 'ALRT' (152) {
	{0, 0, 66, 370},
	152,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (152) {
	{	/* array DITLarray: 2 elements */
		/* [1]*/
		{36, 300, 56, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 23, 360},
		StaticText {
			disabled,
			"Printing error�the startup disk is locked."
		}
	}
};

/* Install new INIT alert. */

resource 'ALRT' (153) {
	{0, 0, 98, 370},
	153,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (153) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{68, 300, 88, 360},
		Button {
			enabled,
			"Install"
		},
		/* [2] */
		{68, 227, 88, 287},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"An old version of the Sample INIT "
			"is installed on this system.  Do you want"
			" to install the new version?"
		}
	}
};


/* File locked alert. */

resource 'ALRT' (154) {
	{0, 0, 82, 370},
	154,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (154) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{52, 300, 72, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 360},
		StaticText {
			disabled,
			"The file could not be saved because the old "
			"version of the file is locked."
		}
	}
};

/* Page box too narrow alert */

resource 'ALRT' (155) {
	{0, 0, 162, 460},
	155,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (155) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{132, 390, 152, 450},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 450},
		StaticText {
			disabled,
			"The document cannot be printed because s"
			"ome pages would be truncated on the right."
		},
		/* [3] */
		{55, 74, 119, 450},
		StaticText {
			disabled,
			"To correct this problem, use the Page Se"
			"tup command.  Make the left and right margins smaller or"
			" make the font size smaller.  You might a"
			"lso try printing with landscape orientat"
			"ion instead of portrait orientation."
		}
	}
};

/* Page box too short alert */

resource 'ALRT' (156) {
	{0, 0, 146, 460},
	156,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (156) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{116, 390, 136, 450},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 39, 450},
		StaticText {
			disabled,
			"The document cannot be printed because s"
			"ome pages would be truncated on the bottom."
		},
		/* [3] */
		{55, 74, 103, 450},
		StaticText {
			disabled,
			"To correct this problem, use the Page Se"
			"tup command. Make the margins smaller or"
			" make the font size smaller."
		}
	}
};

/* Prefs file insane alert. */

resource 'ALRT' (157) {
	{0, 0, 98, 370},
	157,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'DITL' (157) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{68, 300, 88, 360},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{7, 74, 55, 360},
		StaticText {
			disabled,
			"The Disinfectant Prefs file was damaged. "
			"The default preferences will be used and "
			"a new Prefs file will be created."
		}
	}
};


/* Open file or folder dialog. */

resource 'DLOG' (200) {
	{0, 0, 284, 368},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	200,
	"  "
};


resource 'DITL' (200) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{183, 258, 201, 338},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{0, 699, 80, 717},
		Button {
			enabled,
			"Hidden"
		},
		/* [3] */
		{241, 258, 259, 338},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{77, 245, 97, 350},
		UserItem {
			disabled
		},
		/* [5] */
		{111, 258, 129, 338},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{140, 258, 158, 338},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{77, 12, 271, 227},
		UserItem {
			enabled
		},
		/* [8] */
		{77, 226, 271, 243},
		UserItem {
			enabled
		},
		/* [9] */
		{170, 254, 171, 342},
		UserItem {
			disabled
		},
		/* [10] */
		{6, 16, 22, 360},
		StaticText {
			disabled,
			""
		},
		/* [11] */
		{29, 30, 47, 215},
		Button {
			enabled,
			"Select Current Folder:"
		},
		/* [12] */
		{212, 258, 230, 338},
		Button {
			enabled,
			"Folder"
		}
	}
};

/* Printing dialog. */

resource 'DLOG' (201) {
	{0, 0, 75, 385},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	201,
	""
};

resource 'DITL' (201) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{15, 15, 35, 370},
		StaticText {
			disabled,
			"Printing �^0�."
		},
		/* [2] */
		{40, 15, 60, 370},
		StaticText {
			disabled,
			"To cancel, hold down the \$11 key and type a period (.)."
		}
	}
};

/* Eject wait dialog. */

resource 'DLOG' (202) {
	{0, 0, 50, 285},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	202,
	""
};

resource 'DITL' (202) {
	{	/* array DITLarray: 1 element */
		/* [1] */
		{15, 15, 35, 270},
		StaticText {
			disabled,
			"Please wait�"
		},
	}
};

/* Volume list dialog. */

resource 'DLOG' (203) {
	{0, 0, 255, 380},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	203,
	""
};

resource 'DITL' (203) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{107, 293, 127, 353},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{175, 293, 195, 353},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] - volume list */
		{61, 24, 239, 267},
		UserItem {
			enabled
		},
		/* [4] - outline OK button */
		{0, 0, 0, 0},
		UserItem {
			enabled
		},
		/* [5] */
		{11, 17, 49, 370},
		StaticText {
			disabled,
			"Select the disks to be ^0.  Hold "
			"down the shift key while clicking to "
			"select multiple disks."
		}
	}
};

/* Get File Dialog. */

resource 'DLOG' (204) {
	{0, 0, 220, 368},
	dBoxProc,
	invisible,
	noGoAway,
	0x0,
	204,
	""
};

resource 'DITL' (204) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{158, 258, 176, 338},
		Button {
			enabled,
			"Open"
		},
		/* [2] */
		{0, 1211, 80, 1229},
		Button {
			enabled,
			""
		},
		/* [3] */
		{183, 258, 201, 338},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{59, 245, 79, 350},
		UserItem {
			disabled
		},
		/* [5] */
		{88, 258, 106, 338},
		Button {
			enabled,
			"Eject"
		},
		/* [6] */
		{113, 258, 131, 338},
		Button {
			enabled,
			"Drive"
		},
		/* [7] */
		{59, 12, 205, 227},
		UserItem {
			enabled
		},
		/* [8] */
		{59, 226, 205, 243},
		UserItem {
			enabled
		},
		/* [9] */
		{144, 254, 145, 342},
		UserItem {
			disabled
		},
		/* [10] */
		{8, 16, 24, 360},
		StaticText {
			disabled,
			""
		}
	}
};


/* Dialog item list to be appended to Page Setup dialog. */

resource 'DITL' (300) {
	{	/* array DITLarray: 16 elements */
		/* [1] */
		{10, 5, 11, 465},
		UserItem {
			disabled
		},
		/* [2] */
		{38, 14, 152, 176},
		UserItem {
			enabled
		},
		/* [3] */
		{41, 188, 57, 231},
		EditText {
			disabled,
			""
		},
		/* [4] */
		{70, 185, 152, 234},
		UserItem {
			enabled
		},
		/* [5] */
		{19, 253, 42, 462},
		CheckBox {
			enabled,
			"Print Pages in Reverse Order"
		},
		/* [6] */
		{84, 285, 100, 335},
		EditText {
			disabled,
			""
		},
		/* [7] */
		{121, 285, 137, 335},
		EditText {
			disabled,
			""
		},
		/* [8] */
		{84, 405, 100, 457},
		EditText {
			disabled,
			""
		},
		/* [9] */
		{121, 405, 137, 457},
		EditText {
			disabled,
			""
		},
		/* [10] */
		{21, 19, 36, 58},
		StaticText {
			disabled,
			"Font"
		},
		/* [11] */
		{21, 193, 36, 233},
		StaticText {
			disabled,
			"Size"
		},
		/* [12] */
		{84, 250, 105, 280},
		StaticText {
			disabled,
			"Left"
		},
		/* [13] */
		{121, 244, 142, 280},
		StaticText {
			disabled,
			"Right"
		},
		/* [14] */
		{84, 374, 107, 401},
		StaticText {
			disabled,
			"Top"
		},
		/* [15] */
		{121, 348, 142, 401},
		StaticText {
			disabled,
			"Bottom"
		},
		/* [16] */
		{56, 302, 75, 429},
		StaticText {
			disabled,
			"Margins (inches)"
		}
	}
};




/* Strings. */

resource 'STR#' (128, preload) {
	{
/*1*/			"Sample 2.0",  /* Prefs file version num, NOT cur vers num! */
/*2*/			"Save report as:",
/*3*/			"Save manual as:",
/*4*/			"Sample 2.4 Manual",
/*5*/			"Continuing scan of �^0�.",
/*6*/			"### File infected by ^0.",
/*7*/			"### File partially infected by ^0,",
/*8*/			"Last modification ^0, ^1.",
/*9*/			"### An I/O error occurred while trying to repair.",
/*10*/		"### this file.",
/*11*/		"Disk disinfection run started.",
/*12*/		"Disk scanning run started.",
/*13*/		"^0, ^1.",
/*14*/		"Disk disinfection run completed.",
/*15*/		"Disk scanning run completed.",
/*16*/		"Summary:",
/*17*/		"^0 total files.",
/*18*/		"^0 total file.",
/*19*/		"^0 errors.",
/*20*/		"^0 error.",
/*21*/		"^0 total infected files.",
/*22*/		"^0 total infected file.",
/*23*/		"^0 files infected by ^1.",
/*24*/		"^0 file infected by ^1.",
/*25*/		"Earliest infected file: ^0",
/*26*/		"No infected files were found on this disk.",
/*27*/		"File deleted.",
/*28*/		"File repaired.",
/*29*/		"### This file is busy and cannot be checked.",
/*30*/		"### This file is busy and cannot be repaired.",
/*31*/		"### System files cannot be scanned over TOPS.",        								
/*32*/		"### Scan canceled.",
/*33*/		"### Unexpected error (^0).",
/*34*/		"### but not contagious.",
/*35*/		"No infected files were found in this folder.",
/*36*/		"No infections were found in this file.",
/*37*/		"Folder disinfection run started.",
/*38*/		"Folder scanning run started.",
/*39*/		"Folder disinfection run completed.",
/*40*/		"Folder scanning run completed.",
/*41*/		"File disinfection run started.",
/*42*/		"File scanning run started.",
/*43*/		"File disinfection run completed.",
/*44*/		"File scanning run completed.",
/*45*/		"Please insert next disk",
/*46*/		"Please insert first disk",
/*47*/		"At least one file in your currently active system folder"
					" was infected.",
/*48*/		"At least one file on your currently active system disk"
					" was infected.",
/*49*/		"### WARNING: This file is still infected!",
/*50*/		"### Unexpected error (^0) occurred while trying",
/*51*/		"### to open this file for repair.",
/*52*/		"### WARNING: This file may still be infected!",
/*53*/		"### There is not enough memory to check this file.",
/*54*/		"### There is not enough memory to repair this file.",
/*55*/		"### You do not have See Folders privilege",
/*56*/		"### to this folder. Folders within this folder", 
/*57*/		"### cannot be checked.",
/*58*/		"### You do not have See Files privilege",
/*59*/		"### to this folder. Files within this folder", 
/*60*/		"### cannot be checked.",
/*61*/		"### You have neither See Files nor See Folders",
/*62*/		"### privileges to this folder. This folder",
/*63*/		"### cannot be checked.",
/*64*/		"### You do not have Make Changes privilege",
/*65*/		"### to the folder containing this file.",  
/*66*/		"### It cannot be repaired.",
/*67*/		"### WARNING: You do not have the proper privileges",
/*68*/		"### to access files in some of the folders. Some",
/*69*/		"### files in those folders may be infected!",
/*70*/		"### Unexpected error (^0). If you are using",
/*71*/		"### GateKeeper, check to make certain you have",
/*72*/		"### granted privileges to Sample.",
/*73*/		"### The disk is too full to repair this file.",
/*74*/		"### NOTE: Some errors were reported. For a detailed",
/*75*/		"### explanation of an error message, press Command-?",
/*76*/		"### and click on the error message text.",
/*77*/		"### An error or inconsistency was detected while",
/*78*/		"### trying to repair this file.",
/*79*/		"### WARNING: This file is probably still infected!",
/*80*/		"### An I/O error occurred while trying to check",
/*81*/		"### this file.",
/*82*/		"### The resource fork of this file is damaged or",
/*83*/		"### in an unknown format. It cannot be checked.",
/*84*/		"### This file was damaged by the virus, and it cannot",
/*85*/		"### be repaired properly. You should delete the file",
/*86*/		"### and replace it with a known good copy.",
/*87*/		"### File partially infected by nVIR A or B,",
/*88*/		"Sample Report",
/*89*/		"^0  ^1  page ^2",
/*90*/		"Sample Manual",
/*91*/		"page ^2",
/*92*/		"Files scanned:",
/*93*/		"Infected files:",
/*94*/		"Errors:",
/*95*/		"Table of Contents",
/*96*/		"Select the file or folder to be ^0:",
/*97*/		"Select the folder to be ^0:",
/*98*/		"Select the file to be ^0:",
/*99*/		"scanned",
/*100*/		"disinfected",
/*101*/		"Sample Prefs",
/*102*/		"Sample has found an infected file.",
/*103*/		"Sample requires attention.",
/*104*/		"Save Protection INIT as:",
/*105*/		"\$D7 Sample INIT",
/*106*/		"### Try using Finder instead of MultiFinder.",
/*107*/		"### The inserted disk is uninitialized, damaged,",
/*108*/		"### or not a Mac disk. It cannot be scanned.",
	}
};


/* Credits for about box. */

resource 'STR#' (129) {
	{
		"Sample was written in MPW C by John Norstad, ",
		"with help from his friends Mark Anbinder, Wade Blomgren, ",
		"Chris Borton, Scott Boyd, Shawn Cokus, ",
		"Zbigniew Fiedorowicz, Bob Hablutzel, ",
		"Tim Krauskopf, Joel Levin, ",
		"Robert Lentz, Bill Lipa, Albert Lunde, James Macak, ",
		"Lance Nakata, Dave Platt, Leonard Rosenthol, Art Schumer, ",
		"Dan Schwendener, Stephan Somogyi, David Spector, ",
		"Werner Uhrig, and Ephraim Vishniac.",	
	}
};


/* Virus names list for about box. */

resource 'STR#' (130) {
	{
		"Scores",
		"nVIR",
		"INIT 29",
		"ANTI",
		"MacMag",
		"WDEF",
		"ZUC",
		"MDEF",
		"Frankie",
		"CDEF",
	}
};



/* Program icon. */

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"00 0F E0 00 00 7F FC 00 01 F0 1F 00 03 80 03 80"
		$"07 00 01 C0 0E 00 00 60 1F 38 04 30 3B B8 04 38"
		$"31 FC 1E 18 61 FA 3E 0C 61 FA 46 0C 61 BA 46 0C"
		$"C0 3E 06 06 C0 3E 3E 06 C0 3F 46 06 C0 3B C6 06"
		$"C0 3B C6 06 C0 3A FE 06 C0 3A 7E 06 60 3A 7E 0C"
		$"60 3B 9C 0C 60 3A 1E 0C 30 3A 1F 18 38 7E 23 B8"
		$"18 1F C1 F0 0C 07 80 E0 07 07 81 C0 03 82 03 80"
		$"01 F0 1F 00 00 7F FC 00 00 0F E0",
		/* [2] */
		$"00 0F E0 00 00 7F FC 00 01 FF FF 00 03 FF FF 80"
		$"07 FF FF C0 0F FF FF E0 1F FF FF F0 3F FF FF F8"
		$"3F FF FF F8 7F FF FF FC 7F FF FF FC 7F FF FF FC"
		$"FF FF FF FE FF FF FF FE FF FF FF FE FF FF FF FE"
		$"FF FF FF FE FF FF FF FE FF FF FF FE 7F FF FF FC"
		$"7F FF FF FC 7F FF FF FC 3F FF FF F8 3F FF FF F8"
		$"1F FF FF F0 0F FF FF E0 07 FF FF C0 03 FF FF 80"
		$"01 FF FF 00 00 7F FC 00 00 0F E0"
	}
};

/* Prefs file icon. */

resource 'ICN#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"3F FF FF 80 20 00 00 C0 20 00 00 A0 20 1F C0 90"
		$"20 38 F0 88 20 60 38 FC 20 FC 4C 04 21 BE EC 04"
		$"21 BF 64 04 21 0E E6 04 21 0F 66 04 21 0F E6 04"
		$"21 8F E6 04 21 8F E4 04 20 DE FC 04 20 CF 38 04"
		$"20 77 30 04 20 3F E0 04 20 07 80 04 20 00 00 04"
		$"20 00 00 04 20 00 03 04 20 00 04 04 27 2C CE 74"
		$"24 B1 24 84 24 A1 E4 64 24 A1 04 14 27 20 C4 E4"
		$"24 00 00 04 24 00 00 04 20 00 00 04 3F FF FF FC",
		/* [2] */
		$"3F FF FF 80 3F FF FF C0 3F FF FF E0 3F FF FF F0"
		$"3F FF FF F8 3F FF FF FC 3F FF FF FC 3F FF FF FC"
		$"3F FF FF FC 3F FF FF FC 3F FF FF FC 3F FF FF FC"
		$"3F FF FF FC 3F FF FF FC 3F FF FF FC 3F FF FF FC"
		$"3F FF FF FC 3F FF FF FC 3F FF FF FC 3F FF FF FC"
		$"3F FF FF FC 3F FF FF FC 3F FF FF FC 3F FF FF FC"
		$"3F FF FF FC 3F FF FF FC 3F FF FF FC 3F FF FF FC"
		$"3F FF FF FC 3F FF FF FC 3F FF FF FC 3F FF FF FC"
	}
};


/* Small icons.

	128 = floppy.
	129 = hard drive.
	130 = folder.
	131 = file.
	132 = Sample. */

data 'SICN' (128, preload) {
	$"0000 0000 3FF0 48A8 48A4 4824 47C4 4004"        /* ....?.H�H�H$G�@. */
	$"4004 47C4 4824 4824 4824 3FF8 0000 0000"        /* @.G�H$H$H$?..... */
};

data 'SICN' (129, preload) {
	$"0000 0000 0000 0000 0000 7FFE 8001 8001"        /* ............�.�. */
	$"A001 8001 7FFE 0000 0000 0000 0000 0000"        /* �.�............. */
};

data 'SICN' (130, preload) {
	$"0000 0000 0000 0000 1E00 21F0 2008 2008"        /* ..........!. . . */
	$"2008 2008 2008 2008 3FF8 0000 0000 0000"        /*  . . . .?....... */
};

data 'SICN' (131, preload) {
	$"0000 0000 0000 0FC0 0860 0870 0810 0810"        /* .......�.`.p.... */
	$"0810 0810 0810 0810 0FF0 0000 0000 0000"        /* ................ */
};

resource 'SICN' (132) {
	{	/* array: 1 elements */
		/* [1] */
		$"0FE0 1C78 301C 7E26 DF76 DFB2 8773 87B3"
		$"87F3 C7F3 C7F2 6F7E 679C 3B98 1FF0 03C0"
	}
};


/* Question mark cursor */

resource 'CURS' (201, preload, locked) {
	$"00 00 00 00 0F C0 1C E0 38 70 38 70 00 70 01 E0"
	$"03 80 03 80 03 80 00 00 03 80 03 80",
	$"00 00 00 00 0F C0 1C E0 38 70 38 70 00 70 01 E0"
	$"03 80 03 80 03 80 00 00 03 80 03 80",
	{8, 7}
};


/* Bundle and signature. */

resource 'BNDL' (128) {
	'SAMP',
	0,
	{
		'ICN#',
		{
			0, 128,
			1, 129,
			2, 130,
		},
		'FREF',
		{
			0, 128,
			1, 129,
			2, 130,
		}
	}
};
resource 'ICN#' (128, preload) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 0000 0000 0007 F000 003F FE00"
		$"00F8 0F80 01E0 03C0 0380 00E0 0700 0070"
		$"0E00 0038 0C03 E418 1C07 F81C 1808 700C"
		$"180C 200C 300E 4006 3017 FC06 3033 FE06"
		$"3039 0706 301F E306 300F F306 3004 3306"
		$"181F B20C 183F E40C 1C20 F81C 0C21 3018"
		$"0E1E 0038 0700 0070 0380 00E0 01E0 03C0"
		$"00F8 0F80 003F FE00 0007 F0",
		/* [2] */
		$"0000 0000 0000 0000 0007 F000 003F FE00"
		$"00FF FF80 01FF FFC0 03FF FFE0 07FF FFF0"
		$"0FFF FFF8 0FFF FFF8 1FFF FFFC 1FFF FFFC"
		$"1FFF FFFC 3FFF FFFE 3FFF FFFE 3FFF FFFE"
		$"3FFF FFFE 3FFF FFFE 3FFF FFFE 3FFF FFFE"
		$"1FFF FFFC 1FFF FFFC 1FFF FFFC 0FFF FFF8"
		$"0FFF FFF8 07FF FFF0 03FF FFE0 01FF FFC0"
		$"00FF FF80 003F FE00 0007 F0"
	}
};

resource 'ICN#' (129, preload) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 0000 0000 0007 F000 003F FE00"
		$"00F8 0F80 01E0 03C0 0380 00E0 0700 0070"
		$"0E00 0038 0C03 E418 1C07 F81C 1808 700C"
		$"180C 200C 300E 4006 3017 FC06 3033 FE06"
		$"3039 0706 301F E306 300F F306 3004 3306"
		$"181F B20C 183F E40C 1C20 F81C 0C21 3018"
		$"0E1E 0038 0700 0070 0380 00E0 01E0 03C0"
		$"00F8 0F80 003F FE00 0007 F0",
		/* [2] */
		$"0000 0000 0000 0000 0007 F000 003F FE00"
		$"00FF FF80 01FF FFC0 03FF FFE0 07FF FFF0"
		$"0FFF FFF8 0FFF FFF8 1FFF FFFC 1FFF FFFC"
		$"1FFF FFFC 3FFF FFFE 3FFF FFFE 3FFF FFFE"
		$"3FFF FFFE 3FFF FFFE 3FFF FFFE 3FFF FFFE"
		$"1FFF FFFC 1FFF FFFC 1FFF FFFC 0FFF FFF8"
		$"0FFF FFF8 07FF FFF0 03FF FFE0 01FF FFC0"
		$"00FF FF80 003F FE00 0007 F0"
	}
};


resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'FREF' (129) {
	'D2C1',
	1,
	""
};

resource 'FREF' (130) {
	'D2C2',
	2,
	""
};

type 'SAMP' as 'STR ';

resource 'SAMP' (0) {
	"Sample 2.4.  Copyright Northwestern University 1988-1990"
};

resource 'vers' (1) {
	0x02, 0x40, release, 0x00, verUS,      /* 2.4 */
	"2.4",
	"2.4, Copyright Northwestern University 1988-1990"
};

resource 'vers' (2) {
	0x02, 0x40, release, 0x00, verUS,      /* 2.4 */
	"2.4",
	"Sample 2.4"
};


/* MF SIZE resource. */

resource 'SIZE' (-1) {
	dontSaveScreen,
	acceptSuspendResumeEvents,
	disableOptionSwitch,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	reserved,
	716800,
	409600,
};

/* Sousa's Liberty Bell for About window. */

type 'TUNE' {
	integer = $$Countof(noteArray);
	array noteArray {
		boolean	note=0, rest=1;
		bitstring[7]	x=0, e=1, q=2, qq=3, h=4, hh=5;
		byte	c4=60, cs4=61, d4=62, ds4=63, eb4=63, e4=64, f4=65, 
			fs4=66, g4=67, gs4=68, a4=69, as4=70, bb4=70, b4=71, c5=72, 
			cs5=73, d5=74, ds5=75, eb5=75, e5=76, f5=77, fs5=78, gb5=78, g5=79, 
			gs5=80, a5=81, as5=82, bb5=82, b5=83,
			c6=84, cs6=85, d6=86, eb6=87, e6=88, f6=89;
	};
};

resource 'TUNE' (128, purgeable) {
	{
		note, e, c5, 
		note, e, a4, 
		rest, e, 0, 
		note, e, a4, 
		note, e, a4, 
		note, e, gs4, 
		note, e, a4, 
		note, e, f5, 
		rest, e, 0, 
		note, e, c5, 
		note, e, c5, 
		rest, e, 0, 
		note, e, a4, 
		note, q, bb4,
		note, e, bb4, 
		note, q, bb4, 
		note, e, c5, 
		note, qq, d5, 
		note, q, d5, 
		note, e, bb4, 
		note, e, g4, 
		rest, e, 0, 
		note, e, g4, 
		note, e, g4, 
		note, e, fs4,
		note, e, g4, 
		note, e, e5, 
		rest, e, 0, 
		note, e, d5, 
		note, e, d5, 
		rest, e, 0, 
		note, e, bb4, 
		note, e, a4, 
		rest, e, 0, 
		note, e, a4, 
		note, e, a4,
		rest, e, 0, 
		note, e, bb4, 
		note, qq, c5, 
		note, q, c5, 
		note, e, d5, 
		note, e, a4, 
		rest, e, 0, 
		note, e, a4, 
		note, e, a4, 
		note, e, gs4, 
		note, e, a4, 
		note, e, a5,
		rest, e, 0, 
		note, e, f5, 
		note, e, f5, 
		rest, e, 0, 
		note, e, c5, 
		note, e, b4, 
		rest, e, 0, 
		note, e, g5,
		note, e, g5, 
		rest, e, 0, 
		note, e, g5, 
		note, qq, g5, 
		note, q, g5, 
		note, e, f5, 
		note, e, e5, 
		rest, e, 0, 
		note, e, g5, 
		note, e, g5, 
		note, e, fs5, 
		note, e, g5,
		note, e, d5, 
		rest, e, 0, 
		note, e, g5, 
		note, e, g5, 
		note, e, fs5, 
		note, e, g5,
		note, e, c5, 
		rest, e, 0, 
		note, e, b4, 
		note, e, c5, 
		rest, e, 0, 
		note, e, b4, 
		note, qq, c5,
		note, qq, c5,
		note, e, a4,
		rest, e, 0,
		note, e, a4,
		note, e, a4,
		note, e, gs4,
		note, e, a4,
		note, e, f5,
		rest, e, 0,
		note, e, c5,
		note, e, c5,
		rest, e, 0,
		note, e, a4,
		note, q, bb4,
		note, e, bb4,
		note, q, bb4,
		note, e, c5,
		note, qq, d5,
		note, q, d5,
		note, e, bb4,
		note, e, g4,
		rest, e, 0,
		note, e, g4,
		note, e, g4,
		note, e, fs4,
		note, e, g4,
		note, e, e5,
		rest, e, 0,
		note, e, d5,
		note, e, d5,
		rest, e, 0,
		note, e, bb4,
		note, e, a4,
		rest, e, 0,
		note, e, a4,
		note, e, a4,
		rest, e, 0,
		note, e, bb4,
		note, qq, c5,
		note, q, c5,
		note, e, d5,
		note, e, a4,
		rest, e, 0,
		note, e, a4,
		note, e, a4,
		note, e, gs4,
		note, e, a4,
		note, e, a5,
		rest, e, 0,
		note, e, f5,
		note, e, f5,
		rest, e, 0,
		note, e, c5,
		note, e, b4,
		rest, e, 0,
		note, e, g5,
		note, e, g5,
		rest, e, 0,
		note, e, g5,
		note, qq, g5,
		note, q, g5,
		note, e, f5,
		note, e, e5,
		rest, e, 0,
		note, e, g5,
		note, e, g5,
		note, e, fs5,
		note, e, g5,
		note, e, d5,
		rest, e, 0,
		note, e, g5,
		note, e, g5,
		note, e, fs5,
		note, e, g5,
		note, e, c5,
		rest, e, 0,
		note, e, b4,
		note, e, c5,
		rest, e, 0,
		note, e, b4,
		note, qq, c5,
		note, qq, bb4,
		note, e, a4,
		note, e, gs4,
		note, e, a4,
		note, q, d5,
		note, e, c5,
		note, qq, a4,
		note, qq, f4,
		note, qq, d4,
		note, qq, g4,
		note, qq, f4,
		note, q, f4,
		note, e, f4,
		note, e, g4,
		note, e, a4,
		note, e, bb4,
		note, q, e5,
		note, e, d5,
		note, qq, c5,
		note, qq, f5,
		note, qq, e5,
		note, qq, d5,
		note, qq, c5,
		note, q, c5,
		note, e, c5,
		note, e, d5,
		rest, e, 0,
		note, e, d5,
		note, x, e5,
		note, e, d5,
		note, e, cs5,
		note, e, d5,
		note, qq, e5,
		note, qq, e5,
		note, q, f5,
		note, e, f5,
		note, x, a5,
		note, e, g5,
		note, e, f5,
		note, e, g5,
		note, qq, a5,
		note, q, a5,
		note, e, a5,
		note, x, a5,
		note, e, g5,
		rest, e, 0,
		note, e, f5,
		note, e, d5,
		rest, e, 0,
		note, e, bb4,
		note, qq, a4,
		note, qq, f4,
		note, qq, g4,
		note, qq, e4,
		note, qq, f4,
		note, q, f4,
		note, e, c5,
		note, e, a4,
		note, e, gs4,
		note, e, a4,
		note, q, d5,
		note, e, c5,
		note, qq, a4,
		note, qq, f4,
		note, qq, d4,
		note, qq, g4,
		note, qq, f4,
		note, q, f4,
		note, e, f4,
		note, e, g4,
		note, e, a4,
		note, e, bb4,
		note, q, e5,
		note, e, d5,
		note, qq, c5,
		note, qq, f5,
		note, qq, e5,
		note, qq, d5,
		note, qq, c5,
		note, q, c5,
		note, e, c5,
		note, e, d5,
		rest, e, 0,
		note, e, d5,
		note, x, e5,
		note, e, d5,
		note, e, cs5,
		note, e, d5,
		note, qq, e5,
		note, qq, e5,
		note, q, f5,
		note, e, f5,
		note, x, a5,
		note, e, g5,
		note, e, f5,
		note, e, g5,
		note, qq, a5,
		note, q, a5,
		note, e, a5,
		note, x, a5,
		note, e, g5,
		rest, e, 0,
		note, e, f5,
		note, e, d5,
		rest, e, 0,
		note, e, bb4,
		note, qq, a4,
		note, qq, f4,
		note, qq, g4,
		note, qq, e4,
		note, qq, f4,
		note, q, f4,
		note, e, f4,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, d5,
		rest, e, 0,
		note, e, d5,
		note, e, d5,
		rest, e, 0,
		note, e, cs5,
		note, qq, d5,
		note, q, d5,
		note, e, f4,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, eb5,
		rest, e, 0,
		note, e, eb5,
		note, e, eb5,
		rest, e, 0,
		note, e, d5,
		note, qq, eb5,
		note, q, eb5,
		note, e, d5,
		note, qq, c5,
		note, qq, b4,
		note, q, c5,
		note, e, f4,
		note, q, e4,
		note, e, f4,
		note, qq, d5,
		note, qq, cs5,
		note, q, d5,
		note, e, f4,
		note, q, e4,
		note, e, f4,
		note, qq, a4,
		note, qq, c5,
		note, q, bb4,
		note, e, c5,
		note, q, g4,
		note, e, bb4,
		note, q, a4,
		note, e, bb4,
		note, q, g4,
		note, e, a4,
		note, q, f4,
		note, e, g4,
		note, q, eb4,
		note, e, f4,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, d5,
		rest, e, 0,
		note, e, d5,
		note, e, d5,
		rest, e, 0,
		note, e, cs5,
		note, qq, d5,
		note, q, d5,
		note, e, f4,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, eb5,
		rest, e, 0,
		note, e, eb5,
		note, e, eb5,
		rest, e, 0,
		note, e, d5,
		note, qq, eb5,
		note, q, eb5,
		note, e, d5,
		note, qq, c5,
		note, qq, b4,
		note, q, c5,
		note, e, f4,
		note, q, e4,
		note, e, f4,
		note, qq, d5,
		note, qq, cs5,
		note, q, d5,
		note, e, f4,
		note, q, e4,
		note, e, f4,
		note, qq, g4,
		note, qq, eb5,
		note, q, d5,
		note, e, bb4,
		note, q, c5,
		note, e, a4,
		note, hh, bb4,
		note, q, bb4,
		rest, qq, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, eb5,
		note, e, eb5,
		note, e, d5,
		note, e, c5,
		note, qq, bb4,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, eb5,
		note, e, eb5,
		note, e, d5,
		note, e, c5,
		note, q, bb4,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, eb5,
		note, e, eb5,
		note, e, d5,
		note, e, d5,
		note, q, bb4,
		note, e, bb4,
		note, e, g5,
		note, e, g5,
		note, e, f5,
		note, e, eb5,
		note, q, d5,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, g5,
		note, e, g5,
		note, e, f5,
		note, e, eb5,
		note, q, d5,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, g5,
		note, e, g5,
		note, e, f5,
		note, e, eb5,
		note, q, d5,
		note, e, d5,
		note, e, e5,
		note, e, d5,
		note, e, e5,
		note, q, f5,
		rest, e, 0,
		note, q, f5,
		rest, e, 0,
		note, q, f6,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, e5,
		note, q, f5,
		note, e, c6,
		note, qq, f6,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, e5,
		note, q, f6,
		note, e, c6,
		note, qq, f6,
		note, h, e5,
		note, hh, f5,
		note, hh, gb5,
		note, qq, f5,
		note, qq, eb5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, d6,
		rest, e, 0,
		note, e, d6,
		note, e, d6,
		rest, e, 0,
		note, e, cs6,
		note, qq, d6,
		note, q, d6,
		note, e, f5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, eb6,
		rest, e, 0,
		note, e, eb6,
		note, e, eb6,
		rest, e, 0,
		note, e, d6,
		note, qq, eb6,
		note, q, eb6,
		note, e, d6,
		note, qq, c6,
		note, qq, b5,
		note, q, c6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, d6,
		note, qq, cs6,
		note, q, d6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, a5,
		note, qq, c6,
		note, q, bb5,
		note, e, c6,
		note, q, g5,
		note, e, bb5,
		note, q, a5,
		note, e, bb5,
		note, q, g5,
		note, e, a5,
		note, q, f5,
		note, e, g5,
		note, q, eb5,
		note, e, f5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, d6,
		rest, e, 0,
		note, e, d6,
		note, e, d6,
		rest, e, 0,
		note, e, cs6,
		note, qq, d6,
		note, q, d6,
		note, e, f5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, eb6,
		rest, e, 0,
		note, e, eb6,
		note, e, eb6,
		rest, e, 0,
		note, e, d6,
		note, qq, eb6,
		note, q, eb6,
		note, e, d6,
		note, qq, c6,
		note, qq, b5,
		note, q, c6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, d6,
		note, qq, cs6,
		note, q, d6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, g5,
		note, qq, eb6,
		note, q, d6,
		note, e, bb5,
		note, q, c6,
		note, e, a5,
		note, hh, bb5,
		note, q, bb5,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, eb5,
		note, e, eb5,
		note, e, d5,
		note, e, c5,
		note, qq, bb4,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, eb5,
		note, e, eb5,
		note, e, d5,
		note, e, c5,
		note, q, bb4,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, eb5,
		note, e, eb5,
		note, e, d5,
		note, e, d5,
		note, q, bb4,
		note, e, bb4,
		note, e, g5,
		note, e, g5,
		note, e, f5,
		note, e, eb5,
		note, q, d5,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, g5,
		note, e, g5,
		note, e, f5,
		note, e, eb5,
		note, q, d5,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, g5,
		note, e, g5,
		note, e, f5,
		note, e, eb5,
		note, q, d5,
		note, e, d5,
		note, e, e5,
		note, e, d5,
		note, e, e5,
		note, q, f5,
		rest, e, 0,
		note, q, f5,
		rest, e, 0,
		note, q, f6,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, e5,
		note, q, f5,
		note, e, c6,
		note, qq, f6,
		rest, q, 0,
		rest, e, 0,
		rest, q, 0,
		note, e, e5,
		note, q, f6,
		note, e, c6,
		note, qq, f6,
		note, h, e5,
		note, hh, f5,
		note, hh, gb5,
		note, qq, f5,
		note, qq, eb5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, d6,
		rest, e, 0,
		note, e, d6,
		note, e, d6,
		rest, e, 0,
		note, e, cs6,
		note, qq, d6,
		note, q, d6,
		note, e, f5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, eb6,
		rest, e, 0,
		note, e, eb6,
		note, e, eb6,
		rest, e, 0,
		note, e, d6,
		note, qq, eb6,
		note, q, eb6,
		note, e, d6,
		note, qq, c6,
		note, qq, b5,
		note, q, c6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, d6,
		note, qq, cs6,
		note, q, d6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, a5,
		note, qq, c6,
		note, q, bb5,
		note, e, c6,
		note, q, g5,
		note, e, bb5,
		note, q, a5,
		note, e, bb5,
		note, q, g5,
		note, e, a5,
		note, q, f5,
		note, e, g5,
		note, q, eb5,
		note, e, f5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, d6,
		rest, e, 0,
		note, e, d6,
		note, e, d6,
		rest, e, 0,
		note, e, cs6,
		note, qq, d6,
		note, q, d6,
		note, e, f5,
		note, qq, d5,
		note, qq, eb5,
		note, qq, e5,
		note, qq, f5,
		note, e, eb6,
		rest, e, 0,
		note, e, eb6,
		note, e, eb6,
		rest, e, 0,
		note, e, d6,
		note, qq, eb6,
		note, q, eb6,
		note, e, d6,
		note, qq, c6,
		note, qq, b5,
		note, q, c6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, d6,
		note, qq, cs6,
		note, q, d6,
		note, e, f5,
		note, q, e5,
		note, e, f5,
		note, qq, g5,
		note, qq, eb6,
		note, q, d6,
		note, e, bb5,
		note, q, c6,
		note, e, a5,
		note, hh, bb5,
		note, q, bb5,
	},
};