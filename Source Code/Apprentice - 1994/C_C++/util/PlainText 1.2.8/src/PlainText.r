/*-----------------------------------------------------------------------
PlainText.r -- Resource file for PlainText

				Copyright (c) 1993-1994 by Melburn R. Park

This code is free. It may be freely distributed and incorporated into other
programs. This code may not be subsequently copyrighted, sold, or assigned 
in any way that restricts its use by any others, including its authors.
------------------------------------------------------------------------*/

/*------------------- Cursors -------------------*/

resource 'acur' (160) {
	{	/* array CursIdArray: 4 elements */
		/* [1] */
		160,
		/* [2] */
		161,
		/* [3] */
		162,
		/* [4] */
		163
	}
};

resource 'CURS' (163, preload) {
	$"07C0 1F30 3F08 7F04 7F04 FF02 FF02 FFFE"
	$"81FE 81FE 41FC 41FC 21F8 19F0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (160, preload) {
	$"07C0 1FF0 3FF8 5FF4 4FE4 87C2 8382 8102"
	$"8382 87C2 4FE4 5FF4 3FF8 1FF0 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (161, preload) {
	$"07C0 19F0 21F8 41FC 41FC 81FE 81FE FFFE"
	$"FF02 FF02 7F04 7F04 3F08 1F30 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

resource 'CURS' (162, preload) {
	$"07C0 1830 2008 701C 783C FC7E FEFE FFFE"
	$"FEFE FC7E 783C 701C 2008 1830 07C0",
	$"07C0 1FF0 3FF8 7FFC 7FFC FFFE FFFE FFFE"
	$"FFFE FFFE 7FFC 7FFC 3FF8 1FF0 07C0",
	{7, 7}
};

/*------------------- Strings -------------------*/

resource 'STR#' (128,purgeable) {
	{	/* array StringArray: */
		"Mark the selection with what name?",
		"\"%s\" is an existing marker. Do you want to replace it?",
		"Save this template as:",
		"Replace existing \"%s\"?",
		"Delete which markers?",
		"Do you really want to discard the changes to \"%s\"?"
	}
};

resource 'STR#' (129,purgeable) {
	{	/* array StringArray: */
		"No Worksheet",
		"Use Worksheet",
		"Show Clipboard",
		"Hide Clipboard",
		"Clipboard"
	}
};

resource 'STR#' (130,purgeable) {
	{	/* array StringArray: */
		"Out of Memory",
		"That file is already open",
		"That file is open in another application",
		"Can't open any more documents!",	
/*5*/	"Can't open any more windows!",	
		"Not enough memory for this operation to be undone.",
		"Got Apple Event parameters I don't know what to do with.",
		"MissedAnyParameters: after AEGetAttributeDesc.",
		"GetParamDesc error in Open/Print",
/*10*/	"AECountItems error in Open/Print",
		"AEGetNthPtr in Open/Print",
		"AEDisposeDesc in Open/Print",
		"Unknown error",
		"Can't get memory for color table!",
/*15*/	"Sorry! PlainText requires System 6.04 or higher.",
		"This file cannot be opened. Try opening it with the Open menu.",
		"Resources in this file are corrupted but the text is undamaged.",
		"This file is locked.",
		"File system error."
	}
};

resource 'STR#' (131,purgeable) {
	{	/* array StringArray: */
		"PlainText Worksheet",
		"Open document",
		"Untitled",
		"Save document as",
		"Save a Copy as",
		"PlainText Preferences"
	}
};

/*------------------- Menus -------------------*/

resource 'MENU' (128, "AppleMenu") {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About PlainText", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129, "FileMenu") {
	129,
	textMenuProc,
	0x7FFFF403,
	enabled,
	"File",
	{	/* array: 13 elements */
		/* [1] */
		"New…", noIcon, "N", noMark, plain,
		/* [2] */
		"Open…", noIcon, "O", noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Close", noIcon, "W", noMark, plain,
		/* [5] */
		"Save", noIcon, "S", noMark, plain,
		/* [6] */
		"Save As…", noIcon, noKey, noMark, plain,
		/* [7] */
		"Save a Copy…", noIcon, noKey, noMark, plain,
		/* [8] */
		"Revert to Saved", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Page Setup…", noIcon, noKey, noMark, plain,
		/* [11] */
		"Print…", noIcon, "P", noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (130, "EditMenu") {
	130,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"Edit",
	{	/* array: 11 elements */
		/* [1] */
		"Undo", noIcon, "Z", noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Cut", noIcon, "X", noMark, plain,
		/* [4] */
		"Copy", noIcon, "C", noMark, plain,
		/* [5] */
		"Paste", noIcon, "V", noMark, plain,
		/* [6] */
		"Clear", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Select All", noIcon, "A", noMark, plain,
		/* [9] */
		"Show Clipboard", noIcon, noKey, noMark, plain,
		/* [10] */
		"-", noIcon, noKey, noMark, plain,
		/* [11] */
		"Format…", noIcon, "Y", noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Align", noIcon, noKey, noMark, plain,
		/* [14] */
		"Shift Right", noIcon, "]", noMark, plain,
		/* [15] */
		"Shift Left", noIcon, "[", noMark, plain,
		/* [16] */
		"-", noIcon, noKey, noMark, plain,
		/* [17] */
		"No Worksheet", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (131, "FindMenu") {
	131,
	textMenuProc,
	0x7FFFFF20,
	enabled,
	"Find",
	{	/* array: 9 elements */
		/* [1] */
		"Find…", noIcon, "F", noMark, plain,
		/* [2] */
		"Find Same", noIcon, "G", noMark, plain,
		/* [3] */
		"Find Selection", noIcon, "H", noMark, plain,
		/* [4] */
		"Display Selection", noIcon, noKey, noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Replace…", noIcon, "R", noMark, plain,
		/* [7] */
		"Replace Same", noIcon, "T", noMark, plain
	}
};

resource 'MENU' (132, "Mark") {
	132,
	textMenuProc,
	0x7FFFFFF8,
	enabled,
	"Mark",
	{	/* array: 4 elements */
		/* [1] */
		"Mark…", noIcon, "M", noMark, plain,
		/* [2] */
		"Unmark…", noIcon, noKey, noMark, plain,
		/* [3] */
		"Alphabetical", noIcon, noKey, noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (133, "Window") {
	133,
	textMenuProc,
	0x7FFFFFFB,
	disabled,
	"Window",
	{	/* array: 3 elements */
		/* [1] */
		"Tile Windows", noIcon, noKey, noMark, plain,
		/* [2] */
		"Stack Windows", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (134, "ConvertMenu") {
	134,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"Convert",
	{	/* array: 6 elements */
		/* [1] */
		"Straight to Curly Quotes", noIcon, noKey, noMark, plain,
		/* [2] */
		"Curly to Straight Quotes", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Add Linefeeds", noIcon, noKey, noMark, plain,
		/* [5] */
		"Strip Linefeeds", noIcon, noKey, noMark, plain,
		/* [6] */
		"Strip Controls", noIcon, noKey, noMark, plain,
		/* [7] */
		"Returns to Linefeeds", noIcon, noKey, noMark, plain,
		/* [8] */
		"Linefeeds to Returns", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Form Paragraphs", noIcon, "J", noMark, plain,
		/* [11] */
		"Hard Wrap to Window", noIcon, "K", noMark, plain,
		/* [12] */
		"Hard Wrap to Length", noIcon, "L", noMark, plain,
		/* [13] */
		"-", noIcon, noKey, noMark, plain,
		/* [14] */
		"Tabs to Spaces", noIcon, noKey, noMark, plain,
		/* [15] */
		"Spaces to Tabs", noIcon, noKey, noMark, plain

	}
};


/*------------------- Dialogs -------------------*/

resource 'DLOG' (133, "Find & R", purgeable) {
	{66, 75, 276, 437},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	133,
	"Find & Replace"
};

resource 'DLOG' (135, "Request", purgeable) {
	{72, 64, 164, 428},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	135,
	"Request"
};

resource 'DLOG' (136, "Unmark", purgeable) {
	{68, 106, 214, 374},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	136,
	"Unmark"
};

resource 'DLOG' (138, "Text Find", purgeable) {
	{78, 76, 240, 436},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	138,
	"Find"
};

resource 'DLOG' (141, "Confirm", purgeable) {
	{64, 64, 154, 452},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	141,
	"Confirm"
};

resource 'DLOG' (144, "Format", purgeable) {
	{68, 52, 286, 444},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	144,
	"Format"
};

resource 'DLOG' (258, "DoMessage") {
	{100, 100, 200, 400},
	dBoxProc,
	-1,
	noGoAway,
	0x0,
	258,
	"Program Messages"
};

resource 'DLOG' (1001, "AboutDialog", purgeable) {
	{40, 56, 288, 430},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1001,
	"About PlainText"
};

resource 'DLOG' (1002, "CreditsDialog", purgeable) {
	{40, 56, 288, 430},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1002,
	"About PlainText"
};

resource 'ALRT' (500, "phSaveChanges",purgeable) {
	{100, 110, 220, 402},
	500,
	{	
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	}
};


/*------------------- Dialog Items -------------------*/


resource 'DITL' (133, purgeable) {
	{	/* array DITLarray: 17 elements */
		/* [1] */
		{176, 12, 196, 84},
		Button {
			enabled,
			"Replace"
		},
		/* [2] */
		{176, 280, 196, 352},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 8, 24, 352},
		StaticText {
			disabled,
			"Find what?"
		},
		/* [4] */
		{32, 8, 48, 352},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{56, 8, 72, 352},
		StaticText {
			disabled,
			"Replace with what string?"
		},
		/* [6] */
		{80, 8, 96, 352},
		EditText {
			disabled,
			""
		},
		/* [7] */
		{176, 95, 196, 175},
		Button {
			enabled,
			"Replace All"
		},
		/* [8] */
		{176, 216, 196, 272},
		Button {
			enabled,
			"Find"
		},
		/* [9] */
		{112, 8, 128, 152},
		RadioButton {
			enabled,
			"Literal"
		},
		/* [10] */
		{128, 8, 144, 128},
		RadioButton {
			enabled,
			"Entire Word"
		},
		/* [11] */
		{144, 8, 160, 168},
		RadioButton {
			enabled,
			"Selection Expression"
		},
		/* [12] */
		{112, 192, 128, 331},
		CheckBox {
			enabled,
			"Case Sensitive"
		},
		/* [13] */
		{128, 192, 144, 350},
		CheckBox {
			enabled,
			"Search Backwards"
		},
		/* [14] */
		{144, 192, 160, 352},
		CheckBox {
			enabled,
			"Wrap–around Search"
		},
		/* [15] */
		{107, 8, 108, 352},
		UserItem {
			disabled
		},
		/* [16] */
		{165, 8, 166, 352},
		UserItem {
			disabled
		},
		/* [17] */
		{176, 12, 196, 84},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (135, purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{64, 48, 84, 121},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{64, 231, 84, 304},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 8, 24, 352},
		StaticText {
			disabled,
			""
		},
		/* [4] */
		{32, 8, 48, 352},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{64, 48, 84, 121},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (136, purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{118, 16, 138, 89},
		Button {
			enabled,
			"Delete"
		},
		/* [2] */
		{118, 176, 138, 249},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{7, 14, 24, 176},
		StaticText {
			disabled,
			""
		},
		/* [4] */
		{29, 14, 109, 254},
		UserItem {
			disabled
		},
		/* [5] */
		{118, 16, 138, 89},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (138, purgeable) {
	{	/* array DITLarray: 13 elements */
		/* [1] */
		{128, 14, 148, 86},
		Button {
			enabled,
			"Find"
		},
		/* [2] */
		{128, 278, 148, 350},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 8, 24, 352},
		StaticText {
			disabled,
			"Find what?"
		},
		/* [4] */
		{32, 8, 48, 352},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{64, 8, 80, 80},
		RadioButton {
			enabled,
			"Literal"
		},
		/* [6] */
		{80, 8, 96, 112},
		RadioButton {
			enabled,
			"Entire Word"
		},
		/* [7] */
		{96, 8, 112, 168},
		RadioButton {
			enabled,
			"Selection Expression"
		},
		/* [8] */
		{64, 192, 80, 322},
		CheckBox {
			enabled,
			"Case Sensitive"
		},
		/* [9] */
		{80, 192, 96, 347},
		CheckBox {
			enabled,
			"Search Backwards"
		},
		/* [10] */
		{96, 192, 112, 352},
		CheckBox {
			enabled,
			"Wrap-Around Search"
		},
		/* [11] */
		{59, 8, 60, 352},
		UserItem {
			disabled
		},
		/* [12] */
		{115, 8, 116, 352},
		UserItem {
			disabled
		},
		/* [13] */
		{128, 14, 148, 86},
		UserItem {
			disabled
		}
	}
};


resource 'DITL' (141, purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{64, 8, 84, 81},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{64, 296, 84, 369},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{7, 10, 55, 370},
		StaticText {
			disabled,
			""
		},
		/* [4] */
		{64, 8, 84, 81},
		UserItem {
			disabled
		},
		/* [5] */
		{64, 296, 84, 369},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (144, purgeable) {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{186, 304, 206, 367},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{186, 225, 206, 288},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{144, 14, 161, 52},
		StaticText {
			disabled,
			"Tabs: "
		},
		/* [4] */
		{144, 105, 160, 135},
		EditText {
			disabled,
			""
		},
		/* [5] */
		{169, 16, 189, 143},
		CheckBox {
			enabled,
			"Show Invisibles"
		},
		/* [6] */
		{35, 13, 53, 136},
		CheckBox {
			enabled,
			"Word wrap to:"
		},
		/* [7] */
		{7, 166, 25, 204},
		StaticText {
			disabled,
			"Font"
		},
		/* [8] */
		{7, 335, 25, 368},
		StaticText {
			disabled,
			"Size"
		},
		/* [9] */
		{31, 334, 47, 368},
		EditText {
			disabled,
			""
		},
		/* [10] */
		{28, 162, 156, 321},
		UserItem {
			enabled
		},
		/* [11] */
		{58, 332, 154, 370},
		UserItem {
			enabled
		},
		/* [12] */
		{56, 20, 76, 145},
		RadioButton {
			enabled,
			"Window width"
		},
		/* [13] */
		{75, 20, 93, 126},
		RadioButton {
			enabled,
			"Line length"
		},
		/* [14] */
		{103, 14, 120, 95},
		StaticText {
			disabled,
			"Line length:"
		},
		/* [15] */
		{103, 104, 119, 134},
		EditText {
			enabled,
			""
		},
		/* [16] */
		{188, 16, 206, 122},
		CheckBox {
			enabled,
			"Auto Indent"
		},
		/* [17] */
		{7, 15, 25, 80},
		StaticText {
			disabled,
			"Format"
		},
		/* [18] */
		{28, 7, 133, 151},
		UserItem {
			disabled
		},
		/* [19] */
		{186, 150, 206, 213},
		Button {
			enabled,
			"All Files"
		},
		/* [20] */
		{186, 304, 206, 367},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (258, purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{72, 230, 92, 285},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{15, 60, 85, 222},
		StaticText {
			disabled,
			"^0\n^1\n^2\n^3"
		},
		/* [3] */
		{10, 10, 42, 42},
		Icon {
			disabled,
			1
		},
		/* [4] */
		{72, 230, 92, 285},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (500, "phSaveChanges", purgeable) {
	{
		{58,25,76,99},
		Button {
			enabled,
			"Yes"
		},
		{86,195,104,269},
		Button {
			enabled,
			"Cancel"
		},
		{86,25,104,99},
		Button {
			enabled,
			"No"
		},
		{12,20,53,279},
		StaticText {
			disabled,
			"Save changes to “^0” before closing?"
		}
	}
};

resource 'DITL' (1001, purgeable) {
	{	/* array DITLarray: 1 elements */
		/* [1] */
		{3, 5, 241, 367},
		Picture {
			enabled,
			129
		}
	}
};

resource 'DITL' (1002, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{27, 13, 203, 362},
		Picture {
			enabled,
			128
		},
		/* [2] */
		{1, 2, 247, 373},
		UserItem {
			enabled
		}
	}
};

/*------------------- Version -------------------*/

resource 'vers' (1) {
	0x1,
	0x20,
	beta,
	0x8,
	verUS,
	"1.2.8",
	"1.2.8"
};

resource 'vers' (2) {
	0x1,
	0x20,
	beta,
	0x8,
	verUS,
	"1.2.8",
	"Version 1.2.8"
};

data 'MPSR' (1005,"Defaults") {
	$"0009 4D6F 6E61 636F 00E1 2168 0000 0000"            /* .∆Monaco..!h.... */
	$"0000 00A0 0000 51A8 00E0 AB60 8078 0000"            /* ...†..Q®..´`Äx.. */
	$"0000 0006 0004 002C 0004 0123 0180 002C"            /* .......,...#.Ä., */
	$"0004 0123 0180 A976 E29F 0000 0420 0000"            /* ...#.Ä©v.ü... .. */
	$"0420 0000 0000 0100"                                /* . ...... */
};

/*------------------- Finder -------------------*/

data 'MRPH' (0) {
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'FREF' (129) {
	'TEXT',
	1,
	""
};

resource 'BNDL' (128) {
	'MRPH',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		},
		/* [2] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129
		}
	}
};

/*------------------- Size -------------------*/

resource 'SIZE' (-1, "JugglerSize") {
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
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	800000,
	400000
};

/*------------------- About Pict -------------------*/

resource 'PICT' (128) {
	703,
	{0, 0, 170, 350},
	$"1101 A000 82A1 0096 000C 0100 0000 0200"
	$"0000 0000 0000 A100 9A00 0800 0700 0000"
	$"A700 0001 000A 0000 000D 0021 015F 2C00"
	$"0900 0406 4D6F 6E61 636F 0300 040D 0009"
	$"2E00 0400 0001 002B 0E09 3850 6C61 696E"
	$"5465 7874 2069 7320 6120 7075 7265 2043"
	$"2069 6D70 6C65 6D65 6E74 6174 696F 6E20"
	$"6F66 2069 6465 6173 2063 6F6E 7461 696E"
	$"6564 202A 0B36 696E 2074 6865 206F 626A"
	$"6563 7420 636C 6173 7320 6C69 6272 6172"
	$"7920 7075 626C 6973 6865 6420 696E 20D2"
	$"456C 656D 656E 7473 206F 6620 2A0B 2943"
	$"2B2B 204D 6163 696E 746F 7368 2050 726F"
	$"6772 616D 6D69 6E67 D320 6279 2044 616E"
	$"2057 6573 746F 6E2E A000 97A1 0096 000C"
	$"0100 0000 0200 0000 0000 0000 A100 9A00"
	$"0800 0700 0000 A400 0001 000A 002D 000D"
	$"004E 0159 2A17 3754 6865 2074 6578 7420"
	$"656E 6769 6E65 2069 7320 6D6F 6469 6669"
	$"6564 2066 726F 6D20 5445 3332 4B20 6279"
	$"2052 6F79 2057 6F6F 6420 616E 6420 2A0B"
	$"3577 6974 6820 636F 6E74 7269 6275 7469"
	$"6F6E 7320 6279 2050 6174 7269 636B 2043"
	$"2E20 4265 6172 642C 204C 6565 2041 2E20"
	$"4679 6F63 2C20 2A0B 1F44 6176 6520 506C"
	$"6174 742C 2061 6E64 2054 6564 6479 2053"
	$"6C6F 7474 6F77 2E20 A000 97A1 0096 000C"
	$"0100 0000 0200 0000 0000 0000 A100 9A00"
	$"0800 0700 0000 9B00 0001 000A 0066 0016"
	$"0087 014E 2B09 2322 5468 6573 6520 4F53"
	$"2063 6F6D 6D61 6E64 7320 6172 6520 696D"
	$"706C 656D 656E 7465 643A 2A16 3363 642C"
	$"0963 6863 7265 2C20 6368 7479 702C 2066"
	$"696C 652C 2066 696E 642C 206C 732C 206C"
	$"696E 652C 206F 7065 6E2C 2073 7461 7473"
	$"A000 9701 000A FFFF FFFF 00AA 0160 2000"
	$"5D00 0100 5D01 5720 0093 0001 0093 0157"
	$"A100 9600 0C01 0000 0002 0000 0000 0000"
	$"00A1 009A 0008 FFFC 0000 0093 0000 0100"
	$"0A00 9E00 1A00 A901 422B 0422 314D 656C"
	$"2050 6172 6B2C 2050 682E 442E 2C20 556E"
	$"6976 6572 7369 7479 206F 6620 5465 6E6E"
	$"6573 7365 652C 204D 656D 7068 6973 A000"
	$"97A0 0083 FF"
};

resource 'PICT' (129,purgeable) {
	1954,
	{-1, -1, 238, 362},
	$"1101 A000 8201 000A FFFF FFFF 00EE 016A"
	$"3100 0000 0100 1801 6931 0019 0001 0020"
	$"0169 3100 2100 0100 2801 6931 0029 0001"
	$"002B 0169 3100 2D00 0100 2F01 6931 0030"
	$"0001 0031 0169 0900 0000 0000 0000 0051"
	$"0014 00D8 0042 010C 09FF FFFF FFFF FFFF"
	$"FF22 0048 0001 38F8 2340 1023 40F0 2348"
	$"1823 20F0 2318 1023 30E8 0700 0000 0022"
	$"0048 0059 0000 A000 A0A1 00A4 0002 0F01"
	$"7400 4200 4000 3100 6800 6900 4000 3900"
	$"4800 5900 5000 5100 5800 5100 5800 6100"
	$"6000 6900 6800 5900 6800 4900 6000 4100"
	$"6000 3900 5800 3100 5000 3900 4800 3100"
	$"4000 3901 000A 0000 0000 0000 0000 2200"
	$"4000 3920 0823 0000 23F8 0823 0000 2300"
	$"0823 0000 2310 0023 0000 2308 0823 0000"
	$"23F0 0823 0000 23F0 0023 0000 23F8 F823"
	$"0000 23F8 0023 0000 23F8 F823 0000 2308"
	$"F823 0000 23F8 F823 0000 2308 F823 0000"
	$"8400 0A00 0000 0000 0000 00A0 00A3 0100"
	$"0AFF FFFF FF00 EE01 6A84 000A 0000 0000"
	$"0000 0000 2320 0823 F808 2300 0823 1000"
	$"2308 0823 F008 23F0 0023 F8F8 23F8 0023"
	$"F8F8 2308 F823 F8F8 2308 F8A0 00A1 2200"
	$"4A00 9100 00A0 00A0 A100 A400 020F 0174"
	$"0026 004A 0079 0060 0099 0050 0079 004A"
	$"0091 0058 0099 0060 0091 0060 0081 0058"
	$"0081 0050 0079 0100 0A00 0000 0000 0000"
	$"0022 0050 0079 18FA 2300 0023 080E 2300"
	$"0023 F808 2300 0023 F000 2300 0023 00F8"
	$"2300 0023 F8F8 2300 0084 000A 0000 0000"
	$"0000 0000 A000 A301 000A FFFF FFFF 00EE"
	$"016A 8400 0A00 0000 0000 0000 0023 18FA"
	$"2308 0E23 F808 23F0 0023 00F8 23F8 F8A0"
	$"00A1 2200 7000 8100 00A0 00A0 A100 A400"
	$"020F 0174 0022 0070 0059 0080 0089 0070"
	$"0069 0070 0081 0078 0089 0080 0069 0078"
	$"0059 0070 0069 0100 0A00 0000 0000 0000"
	$"0022 0070 0069 1800 2300 0023 0808 2300"
	$"0023 E008 2300 0023 F0F8 2300 0023 10F8"
	$"2300 0084 000A 0000 0000 0000 0000 A000"
	$"A301 000A FFFF FFFF 00EE 016A 8400 0A00"
	$"0000 0000 0000 0023 1800 2308 0823 E008"
	$"23F0 F823 10F8 A000 A122 0050 00E9 0000"
	$"A000 A0A1 00A4 0002 0F01 7400 6600 4000"
	$"9900 A000 F100 4000 B900 5000 E900 6000"
	$"D900 6800 D900 6800 E900 7000 F100 7800"
	$"E900 7800 E100 8800 E100 7800 C900 8800"
	$"B900 8800 A900 9000 A900 9000 B100 A000"
	$"B900 A000 A900 8000 9900 8800 A100 7800"
	$"9900 7000 A100 6000 A900 5800 A100 4000"
	$"B901 000A 0000 0000 0000 0000 2200 4000"
	$"B930 1023 0000 23F0 1023 0000 2300 0823"
	$"0000 2310 0023 0000 2308 0823 0000 23F8"
	$"0823 0000 23F8 0023 0000 2300 1023 0000"
	$"23E8 F023 0000 23F0 1023 0000 23F0 0023"
	$"0000 2300 0823 0000 2308 0023 0000 2308"
	$"1023 0000 23F0 0023 0000 23F0 E023 0000"
	$"2308 0823 0000 23F8 F023 0000 2308 F823"
	$"0000 2308 F023 0000 23F8 F823 0000 2318"
	$"E823 0000 8400 0A00 0000 0000 0000 00A0"
	$"00A3 0100 0AFF FFFF FF00 EE01 6A84 000A"
	$"0000 0000 0000 0000 2330 1023 F010 2300"
	$"0823 1000 2308 0823 F808 23F8 0023 0010"
	$"23E8 F023 F010 23F0 0023 0008 2308 0023"
	$"0810 23F0 0023 F0E0 2308 0823 F8F0 2308"
	$"F823 08F0 23F8 F823 18E8 A000 A122 0050"
	$"0123 0000 A000 A0A1 00A4 0002 0F01 7400"
	$"2A00 4C01 0100 7401 2D00 4C01 1900 5001"
	$"2300 5801 1900 5A01 2D00 7001 2700 7401"
	$"1500 5801 0100 4C01 1901 000A 0000 0000"
	$"0000 0000 2200 4C01 190A 0423 0000 23F6"
	$"0823 0000 2314 0223 0000 23FA 1623 0000"
	$"23EE 0423 0000 23EC E423 0000 2318 F423"
	$"0000 8400 0A00 0000 0000 0000 00A0 00A3"
	$"0100 0AFF FFFF FF00 EE01 6A84 000A 0000"
	$"0000 0000 0000 230A 0423 F608 2314 0223"
	$"FA16 23EE 0423 ECE4 2318 F4A0 00A1 2200"
	$"4001 6900 00A0 00A0 A100 A400 020F 0174"
	$"0026 0040 0151 0080 0169 004C 0151 0040"
	$"0169 0080 0169 0080 015B 0070 0153 0060"
	$"015B 004C 0151 0100 0A00 0000 0000 0000"
	$"0022 004C 0151 18F4 2300 0023 0040 2300"
	$"0023 F200 2300 0023 F8F0 2300 0023 08F0"
	$"2300 0023 F6EC 2300 0084 000A 0000 0000"
	$"0000 0000 A000 A301 000A FFFF FFFF 00EE"
	$"016A 8400 0A00 0000 0000 0000 0023 18F4"
	$"2300 4023 F200 23F8 F023 08F0 23F6 ECA0"
	$"00A1 2200 8000 1900 00A0 00A0 A100 A400"
	$"020F 0174 005E 0078 0001 00C8 0169 0078"
	$"0001 0080 0019 0090 0029 0090 0031 00A0"
	$"0041 0098 0051 00B0 0069 00A8 0079 00B0"
	$"0089 00A0 0099 00A8 00A9 00AA 00D9 00A0"
	$"00E9 00A8 0101 00A0 0119 00A8 0141 00B0"
	$"0151 00A0 0169 00C8 0169 00C8 0001 0078"
	$"0001 0100 0A00 0000 0000 0000 0022 0078"
	$"0001 1808 2300 0023 1010 2300 0023 0800"
	$"2300 0023 1010 2300 0023 10F8 2300 0023"
	$"1818 2300 0023 10F8 2300 0023 1008 2300"
	$"0023 10F0 2300 0023 1008 2300 0023 3002"
	$"2300 0023 10F6 2300 0023 1808 2300 0023"
	$"18F8 2300 0023 2808 2300 0023 1008 2300"
	$"0023 18F0 2300 0023 0028 2300 0021 00C8"
	$"0001 2300 0023 00B0 2300 0084 000A 0000"
	$"0000 0000 0000 A000 A301 000A FFFF FFFF"
	$"00EE 016A 8400 0A00 0000 0000 0000 0023"
	$"1808 2310 1023 0800 2310 1023 10F8 2318"
	$"1823 10F8 2310 0823 10F0 2310 0823 3002"
	$"2310 F623 1808 2318 F823 2808 2310 0823"
	$"18F0 2300 2821 00C8 0001 2300 B0A0 00A1"
	$"A100 9600 0C01 0000 0002 0000 0000 0000"
	$"00A1 009A 0008 FFFB 0000 0023 0000 0100"
	$"0A00 D500 1C00 E500 650D 000C 2E00 0400"
	$"0001 002B 1DE1 0B48 6967 6820 506C 6169"
	$"6E73 A000 97A1 0096 000C 0200 0000 0200"
	$"0000 0000 0000 A100 9A00 0800 0100 0000"
	$"3C00 0001 000A 00D5 00DD 00ED 0162 2C00"
	$"0900 0306 4765 6E65 7661 0300 030D 0009"
	$"2800 DF00 E31C 6272 6F75 6768 7420 746F"
	$"2079 6F75 2069 6E20 506C 6169 6E54 6578"
	$"7420 2B22 0C0B 6279 204D 656C 2050 6172"
	$"6BA0 0097 A000 83FF"
};

/*------------------- Icons -------------------*/

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 03F8 0F00 0406 10F0 0601 1008"
		$"01FE 0FF0 0000 0000 0000 1E00 0008 6180"
		$"0008 8080 0008 7F00 0008 0000 00CE 3000"
		$"0029 4800 00E9 43FE 7D29 4802 40EE 3002"
		$"4000 0002 4100 0002 4510 0002 4510 0042"
		$"4290 0082 42A0 0282 41A4 0282 41A8 4A02"
		$"40D0 2802 40E0 2002 40E0 0002 4000 0002"
		$"4000 0002 7FFF FFFE",
		/* [2] */
		$"7FFF FFFE 7FFF FFFE 7FFF FFFE 7FFF FFFE"
		$"7FFF FFFE 7FFF FFFE 7FFF FFFE 7FFF FFFE"
		$"7FFF FFFE 7FFF FFFE 7FFF FFFE 7FFF FFFE"
		$"7FFF FFFE 7FFF FFFE 7FFF FFFE 7FFF FFFE"
		$"7FFF FFFE 7FFF FFFE 7FFF FFFE 7FFF FFFE"
		$"7FFF FFFE 7FFF FFFE 7FFF FFFE 7FFF FFFE"
		$"7FFF FFFE 7FFF FFFE 7FFF FFFE 7FFF FFFE"
		$"7FFF FFFE 7FFF FFFE"
	}
};

resource 'ICN#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"1FFF FC00 1000 0600 1000 0500 1000 0480"
		$"1000 0440 11FF E420 1000 07F0 11FF F010"
		$"1000 0010 11FF FF10 1000 0010 11FF FF10"
		$"1000 0010 11FF FF10 1000 0010 11FF FF10"
		$"1000 0010 11FF FF10 1000 0010 11FF 0010"
		$"1000 0010 1000 0010 1002 0010 1002 0010"
		$"1002 0010 1033 8C10 100A 5210 103A 5010"
		$"104A 5210 103B 8C10 1000 0010 1FFF FFF0",
		/* [2] */
		$"1FFF FC00 1FFF FE00 1FFF FF00 1FFF FF80"
		$"1FFF FFC0 1FFF FFE0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
		$"1FFF FFF0 1FFF FFF0 1FFF FFF0 1FFF FFF0"
	}
};

resource 'icl8' (128) {
	$"004E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E"
	$"4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E F5F5 F5F5 F5F5 F54E 4E4E"
	$"4E4E 4E4E F5F5 F5F5 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4EF5 F5F5 F5F5 F5F5 F5F5 F54E"
	$"4E4E 4EF5 F5F5 F5F5 F5F5 F5F5 4E4E 4E00"
	$"004E 4E4E 4EF5 F5F5 F5F5 F5F5 F5F5 F5F8"
	$"4E4E 4EF5 F5F5 F5F5 F5F5 F5F5 F84E 4E00"
	$"004E 4E4E 4E4E 4EF8 F8F8 F8F8 F8F8 F84E"
	$"4E4E 4E4E F8F8 F8F8 F8F8 F8F8 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E"
	$"4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E"
	$"4E4E F5F5 F5F5 F54E 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E"
	$"4EF5 F5F5 F5F5 F5F5 F84E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E 4E4E FC4E 4E4E"
	$"F5F5 F5F5 F5F5 F5F5 F84E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E 4E4E FC4E 4E4E"
	$"4EF8 F8F8 F8F8 F8F8 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E 4E4E FC4E 4E4E"
	$"4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E FCFC 4E4E FCFC FC4E"
	$"4E4E FCFC 4E4E 4E4E 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E FC4E FC4E 4EFC"
	$"4EFC 4E4E FC4E 4E4E 4E4E 4E4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E FCFC FC4E FC0A 0AFC"
	$"0AFC 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A00"
	$"004E 4E4E 4E4E 4EFC 0A0A FC0A FC0A 0AFC"
	$"0AFC 0A0A FC0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A FCFC FC0A FCFC FC0A"
	$"0A0A FCFC 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A17 0A0A 0A0A 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A17 0A17 0A0A 0A17 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A17 0A17 0A0A 0A17 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A 0AE4 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 170A 170A 0A17 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A E40A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 170A 170A 170A 0A0A 0A0A"
	$"0A0A 0A0A 0A0A E40A E40A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A17 170A 170A 0A17 0A0A"
	$"0A0A 0A0A 0A0A E40A E40A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A17 170A 170A 170A 0A0A"
	$"0AE4 0A0A E40A E40A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A 1717 0A17 0A0A 0A0A"
	$"0A0A E40A E40A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A 1717 170A 0A0A 0A0A"
	$"0A0A E40A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A 1717 170A 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A"
	$"0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 1111 1111 1111 1111 1111 1111 1111"
	$"1111 1111 1111 1111 1111 1111 1111 11"
};

resource 'icl8' (129) {
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF FF00 0000 0000 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF 2BFF 0000 0000 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF 2B2B FF00 0000 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF 2B2B 2BFF 0000 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"FFFF FFF5 F5FF 2B2B 2B2B FF00 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF FFFF FFFF FFFF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 00F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFFF FFFF FFFF"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 FFF5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 FFF5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 FFF5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 FFFF F5F5 FFFF"
	$"FFF5 F5F5 FFFF F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 FFF5 FFF5"
	$"F5FF F5FF F5F5 FFF5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 FFFF FFF5 FFF5"
	$"F5FF F5FF F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5FF F5F5 FFF5 FFF5"
	$"F5FF F5FF F5F5 FFF5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 FFFF FFF5 FFFF"
	$"FFF5 F5F5 FFFF F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'icl4' (128) {
	$"07C7 C7C7 C7C7 C7C7 C7C7 C7C7 C7C7 C7C0"
	$"0C7C 7CCC CCCC CC7C 7C7C CCCC 7C7C 7C70"
	$"07C7 CCCC CCCC CCC7 C7CC CCCC CCCC C7C0"
	$"0C7C 7CCC CCCC CCCD 7C7C CCCC CCCC DC70"
	$"07C7 C7CD DDDD DDD7 C7C7 DDDD DDDD C7C0"
	$"0C7C 7C7C 7C7C 7C7C 7C7C 7C7C 7C7C 7C70"
	$"07C7 C7C7 C7C7 C7C7 C7CC CCC7 C7C7 C7C0"
	$"0C7C 7C7C 7C7C EC7C 7CCC CCCC CC7C 7C70"
	$"07C7 C7C7 C7C7 F7C7 CCCC CCCC D7C7 C7C0"
	$"0C7C 7C7C 7C7C FC7C 7DDD DDDD 7C7C 7C70"
	$"07C7 C7C7 C7C7 F7C7 C7C7 C7C7 C7C7 C7C0"
	$"0C7C 7C7C EE7C EEEC 7CEE 7C7C 7C7C 7C70"
	$"07C7 C7C7 C7E7 E7CE CE7C E7C7 C7C7 C7C0"
	$"0C7C 7C7C EEEC E11E 1E11 1111 1111 1110"
	$"07C7 C7CE 11E1 E11E 1E11 E111 1111 1120"
	$"0111 1111 EEE1 EEE1 11EE 1111 1111 1120"
	$"0111 1111 1111 1111 1111 1111 1111 1120"
	$"0111 1112 1111 1111 1111 1111 1111 1120"
	$"0111 1212 1112 1111 1111 1111 1111 1120"
	$"0111 1212 1112 1111 1111 1111 1811 1120"
	$"0111 1121 2112 1111 1111 1111 8111 1120"
	$"0111 1121 2121 1111 1111 1181 8111 1120"
	$"0111 1112 2121 1211 1111 1181 8111 1120"
	$"0111 1112 2121 2111 1811 8181 1111 1120"
	$"0111 1111 2212 1111 1181 8111 1111 1120"
	$"0111 1111 2221 1111 1181 1111 1111 1120"
	$"0111 1111 2221 1111 1111 1111 1111 1120"
	$"0111 1111 1111 1111 1111 1111 1111 1120"
	$"0111 1111 1111 1111 1111 1111 1111 1120"
	$"0122 2222 2222 2222 2222 2222 2222 2220"
};

resource 'icl4' (129) {
	$"000F FFFF FFFF FFFF FFFF FF00 0000 0000"
	$"000F C0C0 C0C0 C0C0 C0C0 CFF0 0000 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0FCF 0000 0000"
	$"000F C0C0 C0C0 C0C0 C0C0 CFCC F000 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0FCC CF00 0000"
	$"000F C0CF FFFF FFFF FFF0 CFCC CCF0 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0FFF FFFF 0000"
	$"000F C0CF FFFF FFFF FFFF C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFFF FFFF FFFF FFFF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFFF FFFF FFFF FFFF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFFF FFFF FFFF FFFF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFFF FFFF FFFF FFFF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFFF FFFF FFFF FFFF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFFF FFFF C0C0 C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0C0 C0C0 C0F0 C0C0 C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0CFC 0C0C 0C0C 0C0F 0000"
	$"000F C0C0 C0C0 C0F0 C0C0 C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0CFC 0C0C 0C0C 0C0F 0000"
	$"000F C0C0 C0FF C0FF F0C0 FFC0 C0CF 0000"
	$"000F 0C0C 0C0C FCFC 0F0F 0CFC 0C0F 0000"
	$"000F C0C0 C0FF F0F0 CFCF C0C0 C0CF 0000"
	$"000F 0C0C 0F0C FCFC 0F0F 0CFC 0C0F 0000"
	$"000F C0C0 C0FF F0FF F0C0 FFC0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'ics8' (128) {
	$"004E 4E4E 4E4E 4E4E 4E4E FC4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E FC4E 4E4E 4E00"
	$"004E 4E4E 4E4E 4E4E 4E4E FC4E 4E4E 4E00"
	$"004E 4E4E 4E4E FCFC 4E4E FCFC FC4E 4E00"
	$"004E 4E4E 4E4E 4E4E FC4E FC4E 4EFC 4E00"
	$"004E 4E17 4E4E FCFC FC4E FC4E 4EFC 4E00"
	$"0017 4E17 4EFC 174E FC4E FC0A 0AFC 1100"
	$"004E 174E 174E FCFC FC0A FCFC FC0A 1100"
	$"004E 170A 0A17 0A0A 0A0A 0A0A 0A0A 1100"
	$"000A 0A17 0A17 0A0A 170A 0A0A E40A 1100"
	$"000A 0A0A 1717 0A17 0AE4 0AE4 0A0A 1100"
	$"000A 0A0A 0A17 170A 0AE4 0AE4 0A0A 1100"
	$"000A 0A0A 0A17 0A0A 0AE4 0A0A 0A0A 1100"
	$"000A 0A0A 0A0A 0A0A 0A0A 0A0A 0A0A 1100"
	$"0011 1111 1111 1111 1111 1111 1111 11"
};

resource 'ics8' (129) {
	$"00FF FFFF FFFF FFFF FFFF FFFF 0000 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 FFFF FF00 0000"
	$"00FF F5F5 FFFF FFFF FFF5 FF2B FFFF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 FFFF FFFF 0000"
	$"00FF F5F5 FFFF FFFF FFFF F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF F5F5 FFFF FFF5 FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 FFFF F5F5 FFFF FFF5 F5FF 0000"
	$"00FF F5F5 F5F5 FFF5 FFF5 F5FF F5FF 0000"
	$"00FF F5F5 FFFF FFF5 FFF5 F5FF F5FF 0000"
	$"00FF F5FF F5F5 FFF5 FFF5 F5FF F5FF 0000"
	$"00FF F5F5 FFFF FFF5 FFFF FFF5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'ics#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"0020 0020 0020 0338 00A4 03A4 54A4 2BB8"
		$"2802 190A 5E52 4C52 4442 4002 7FFE",
		/* [2] */
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
		$"FFFF FFFF FFFF FFFF FFFF FFFF FFFF FFFF"
	}
};

resource 'ics#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"7FF0 4038 4FAC 403C 4FC4 4004 4E84 4084"
		$"4084 4CE4 4294 4E94 5294 4EE4 4004 7FFC",
		/* [2] */
		$"7FF0 7FF8 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
		$"7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
	}
};

resource 'ics4' (128) {
	$"07C7 C7C7 C7E7 C7C0 0C7C 7C7C 7CEC 7C70"
	$"07C7 C7C7 C7E7 C7C0 0C7C 7CEE 7CEE 7C70"
	$"07C7 C7C7 E7E7 CEC0 0C72 7CEE ECEC 7E70"
	$"02C2 CE2C E7E1 1E20 0C2C 2CEE E1EE E120"
	$"0721 1211 1111 1120 0112 1212 1111 8120"
	$"0112 2221 1818 1120 0111 2211 1818 1120"
	$"0111 1211 1811 1120 0111 1111 1111 1120"
	$"0222 2222 2222 2220"
};

resource 'ics4' (129) {
	$"0FFF FFFF FFFF 0000 0FC0 C0C0 C0FF F000"
	$"0F0C FFFF FCFD FF00 0FC0 C0C0 C0FF FF00"
	$"0F0C FFFF FF0C 0F00 0FC0 C0C0 C0C0 CF00"
	$"0F0C FFFC FC0C 0F00 0FC0 C0C0 F0C0 CF00"
	$"0F0C 0C0C FC0C 0F00 0FC0 FFC0 FFF0 CF00"
	$"0F0C 0CFC FC0F 0F00 0FC0 FFF0 F0CF CF00"
	$"0F0F 0CFC FC0F 0F00 0FC0 FFF0 FFF0 CF00"
	$"0F0C 0C0C 0C0C 0F00 0FFF FFFF FFFF FF"
};


/*------------------- Palette -------------------*/

resource 'pltt' (130, "Limits") {
	{	/* array ColorInfo: 256 elements */
		/* [1] */
		65535, 65535, 65535, pmTolerant, 0,
		/* [2] */
		0,0,0, pmTolerant, 0,
		/* [3] */
		64768, 64768, 64768, pmTolerant, 0,
		/* [4] */
		64768, 64768, 64768, pmTolerant, 0,
		/* [5] */
		64512, 64512, 64512, pmTolerant, 0,
		/* [6] */
		64256, 64256, 64256, pmTolerant, 0,
		/* [7] */
		64000, 64000, 64000, pmTolerant, 0,
		/* [8] */
		63744, 63744, 63744, pmTolerant, 0,
		/* [9] */
		63488, 63488, 63488, pmTolerant, 0,
		/* [10] */
		63232, 63232, 63232, pmTolerant, 0,
		/* [11] */
		62976, 62976, 62976, pmTolerant, 0,
		/* [12] */
		62720, 62720, 62720, pmTolerant, 0,
		/* [13] */
		62464, 62464, 62464, pmTolerant, 0,
		/* [14] */
		62208, 62208, 62208, pmTolerant, 0,
		/* [15] */
		61952, 61952, 61952, pmTolerant, 0,
		/* [16] */
		61696, 61696, 61696, pmTolerant, 0,
		/* [17] */
		61440, 61440, 61440, pmTolerant, 0,
		/* [18] */
		61184, 61184, 61184, pmTolerant, 0,
		/* [19] */
		60928, 60928, 60928, pmTolerant, 0,
		/* [20] */
		60672, 60672, 60672, pmTolerant, 0,
		/* [21] */
		60416, 60416, 60416, pmTolerant, 0,
		/* [22] */
		60160, 60160, 60160, pmTolerant, 0,
		/* [23] */
		59904, 59904, 59904, pmTolerant, 0,
		/* [24] */
		59648, 59648, 59648, pmTolerant, 0,
		/* [25] */
		59392, 59392, 59392, pmTolerant, 0,
		/* [26] */
		59136, 59136, 59136, pmTolerant, 0,
		/* [27] */
		58880, 58880, 58880, pmTolerant, 0,
		/* [28] */
		58624, 58624, 58624, pmTolerant, 0,
		/* [29] */
		58368, 58368, 58368, pmTolerant, 0,
		/* [30] */
		58112, 58112, 58112, pmTolerant, 0,
		/* [31] */
		57856, 57856, 57856, pmTolerant, 0,
		/* [32] */
		57600, 57600, 57600, pmTolerant, 0,
		/* [33] */
		57344, 57344, 57344, pmTolerant, 0,
		/* [34] */
		57088, 57088, 57088, pmTolerant, 0,
		/* [35] */
		56832, 56832, 56832, pmTolerant, 0,
		/* [36] */
		56576, 56576, 56576, pmTolerant, 0,
		/* [37] */
		56320, 56320, 56320, pmTolerant, 0,
		/* [38] */
		56064, 56064, 56064, pmTolerant, 0,
		/* [39] */
		55808, 55808, 55808, pmTolerant, 0,
		/* [40] */
		55552, 55552, 55552, pmTolerant, 0,
		/* [41] */
		55296, 55296, 55296, pmTolerant, 0,
		/* [42] */
		55040, 55040, 55040, pmTolerant, 0,
		/* [43] */
		54784, 54784, 54784, pmTolerant, 0,
		/* [44] */
		54528, 54528, 54528, pmTolerant, 0,
		/* [45] */
		54272, 54272, 54272, pmTolerant, 0,
		/* [46] */
		54016, 54016, 54016, pmTolerant, 0,
		/* [47] */
		53760, 53760, 53760, pmTolerant, 0,
		/* [48] */
		53504, 53504, 53504, pmTolerant, 0,
		/* [49] */
		53248, 53248, 53248, pmTolerant, 0,
		/* [50] */
		52992, 52992, 52992, pmTolerant, 0,
		/* [51] */
		52736, 52736, 52736, pmTolerant, 0,
		/* [52] */
		52480, 52480, 52480, pmTolerant, 0,
		/* [53] */
		52224, 52224, 52224, pmTolerant, 0,
		/* [54] */
		51968, 51968, 51968, pmTolerant, 0,
		/* [55] */
		51712, 51712, 51712, pmTolerant, 0,
		/* [56] */
		51456, 51456, 51456, pmTolerant, 0,
		/* [57] */
		51200, 51200, 51200, pmTolerant, 0,
		/* [58] */
		50944, 50944, 50944, pmTolerant, 0,
		/* [59] */
		50688, 50688, 50688, pmTolerant, 0,
		/* [60] */
		50432, 50432, 50432, pmTolerant, 0,
		/* [61] */
		50176, 50176, 50176, pmTolerant, 0,
		/* [62] */
		49920, 49920, 49920, pmTolerant, 0,
		/* [63] */
		49664, 49664, 49664, pmTolerant, 0,
		/* [64] */
		49408, 49408, 49408, pmTolerant, 0,
		/* [65] */
		49152, 49152, 49152, pmTolerant, 0,
		/* [66] */
		48896, 48896, 48896, pmTolerant, 0,
		/* [67] */
		48640, 48640, 48640, pmTolerant, 0,
		/* [68] */
		48384, 48384, 48384, pmTolerant, 0,
		/* [69] */
		48128, 48128, 48128, pmTolerant, 0,
		/* [70] */
		47872, 47872, 47872, pmTolerant, 0,
		/* [71] */
		47616, 47616, 47616, pmTolerant, 0,
		/* [72] */
		47360, 47360, 47360, pmTolerant, 0,
		/* [73] */
		47104, 47104, 47104, pmTolerant, 0,
		/* [74] */
		46848, 46848, 46848, pmTolerant, 0,
		/* [75] */
		46592, 46592, 46592, pmTolerant, 0,
		/* [76] */
		46336, 46336, 46336, pmTolerant, 0,
		/* [77] */
		46080, 46080, 46080, pmTolerant, 0,
		/* [78] */
		45824, 45824, 45824, pmTolerant, 0,
		/* [79] */
		45568, 45568, 45568, pmTolerant, 0,
		/* [80] */
		45312, 45312, 45312, pmTolerant, 0,
		/* [81] */
		45056, 45056, 45056, pmTolerant, 0,
		/* [82] */
		44800, 44800, 44800, pmTolerant, 0,
		/* [83] */
		44544, 44544, 44544, pmTolerant, 0,
		/* [84] */
		44288, 44288, 44288, pmTolerant, 0,
		/* [85] */
		44032, 44032, 44032, pmTolerant, 0,
		/* [86] */
		43776, 43776, 43776, pmTolerant, 0,
		/* [87] */
		43520, 43520, 43520, pmTolerant, 0,
		/* [88] */
		43264, 43264, 43264, pmTolerant, 0,
		/* [89] */
		43008, 43008, 43008, pmTolerant, 0,
		/* [90] */
		42752, 42752, 42752, pmTolerant, 0,
		/* [91] */
		42496, 42496, 42496, pmTolerant, 0,
		/* [92] */
		42240, 42240, 42240, pmTolerant, 0,
		/* [93] */
		41984, 41984, 41984, pmTolerant, 0,
		/* [94] */
		41728, 41728, 41728, pmTolerant, 0,
		/* [95] */
		41472, 41472, 41472, pmTolerant, 0,
		/* [96] */
		41216, 41216, 41216, pmTolerant, 0,
		/* [97] */
		40960, 40960, 40960, pmTolerant, 0,
		/* [98] */
		40704, 40704, 40704, pmTolerant, 0,
		/* [99] */
		40448, 40448, 40448, pmTolerant, 0,
		/* [100] */
		40192, 40192, 40192, pmTolerant, 0,
		/* [101] */
		39936, 39936, 39936, pmTolerant, 0,
		/* [102] */
		39680, 39680, 39680, pmTolerant, 0,
		/* [103] */
		39424, 39424, 39424, pmTolerant, 0,
		/* [104] */
		39168, 39168, 39168, pmTolerant, 0,
		/* [105] */
		38912, 38912, 38912, pmTolerant, 0,
		/* [106] */
		38656, 38656, 38656, pmTolerant, 0,
		/* [107] */
		38400, 38400, 38400, pmTolerant, 0,
		/* [108] */
		38144, 38144, 38144, pmTolerant, 0,
		/* [109] */
		37888, 37888, 37888, pmTolerant, 0,
		/* [110] */
		37632, 37632, 37632, pmTolerant, 0,
		/* [111] */
		37376, 37376, 37376, pmTolerant, 0,
		/* [112] */
		37120, 37120, 37120, pmTolerant, 0,
		/* [113] */
		36864, 36864, 36864, pmTolerant, 0,
		/* [114] */
		36608, 36608, 36608, pmTolerant, 0,
		/* [115] */
		36352, 36352, 36352, pmTolerant, 0,
		/* [116] */
		36096, 36096, 36096, pmTolerant, 0,
		/* [117] */
		35840, 35840, 35840, pmTolerant, 0,
		/* [118] */
		35584, 35584, 35584, pmTolerant, 0,
		/* [119] */
		35328, 35328, 35328, pmTolerant, 0,
		/* [120] */
		35072, 35072, 35072, pmTolerant, 0,
		/* [121] */
		34816, 34816, 34816, pmTolerant, 0,
		/* [122] */
		34560, 34560, 34560, pmTolerant, 0,
		/* [123] */
		34304, 34304, 34304, pmTolerant, 0,
		/* [124] */
		34048, 34048, 34048, pmTolerant, 0,
		/* [125] */
		33792, 33792, 33792, pmTolerant, 0,
		/* [126] */
		33536, 33536, 33536, pmTolerant, 0,
		/* [127] */
		33280, 33280, 33280, pmTolerant, 0,
		/* [128] */
		33024, 33024, 33024, pmTolerant, 0,
		/* [129] */
		32768, 32768, 32768, pmTolerant, 0,
		/* [130] */
		32512, 32512, 32512, pmTolerant, 0,
		/* [131] */
		32256, 32256, 32256, pmTolerant, 0,
		/* [132] */
		32000, 32000, 32000, pmTolerant, 0,
		/* [133] */
		31744, 31744, 31744, pmTolerant, 0,
		/* [134] */
		31488, 31488, 31488, pmTolerant, 0,
		/* [135] */
		31232, 31232, 31232, pmTolerant, 0,
		/* [136] */
		30976, 30976, 30976, pmTolerant, 0,
		/* [137] */
		30720, 30720, 30720, pmTolerant, 0,
		/* [138] */
		30464, 30464, 30464, pmTolerant, 0,
		/* [139] */
		30208, 30208, 30208, pmTolerant, 0,
		/* [140] */
		29952, 29952, 29952, pmTolerant, 0,
		/* [141] */
		29696, 29696, 29696, pmTolerant, 0,
		/* [142] */
		29440, 29440, 29440, pmTolerant, 0,
		/* [143] */
		29184, 29184, 29184, pmTolerant, 0,
		/* [144] */
		28928, 28928, 28928, pmTolerant, 0,
		/* [145] */
		28672, 28672, 28672, pmTolerant, 0,
		/* [146] */
		28416, 28416, 28416, pmTolerant, 0,
		/* [147] */
		28160, 28160, 28160, pmTolerant, 0,
		/* [148] */
		27904, 27904, 27904, pmTolerant, 0,
		/* [149] */
		27648, 27648, 27648, pmTolerant, 0,
		/* [150] */
		27392, 27392, 27392, pmTolerant, 0,
		/* [151] */
		27136, 27136, 27136, pmTolerant, 0,
		/* [152] */
		26880, 26880, 26880, pmTolerant, 0,
		/* [153] */
		26624, 26624, 26624, pmTolerant, 0,
		/* [154] */
		26368, 26368, 26368, pmTolerant, 0,
		/* [155] */
		26112, 26112, 26112, pmTolerant, 0,
		/* [156] */
		25856, 25856, 25856, pmTolerant, 0,
		/* [157] */
		25600, 25600, 25600, pmTolerant, 0,
		/* [158] */
		25344, 25344, 25344, pmTolerant, 0,
		/* [159] */
		25088, 25088, 25088, pmTolerant, 0,
		/* [160] */
		24832, 24832, 24832, pmTolerant, 0,
		/* [161] */
		24576, 24576, 24576, pmTolerant, 0,
		/* [162] */
		24320, 24320, 24320, pmTolerant, 0,
		/* [163] */
		24064, 24064, 24064, pmTolerant, 0,
		/* [164] */
		23808, 23808, 23808, pmTolerant, 0,
		/* [165] */
		23552, 23552, 23552, pmTolerant, 0,
		/* [166] */
		23296, 23296, 23296, pmTolerant, 0,
		/* [167] */
		23040, 23040, 23040, pmTolerant, 0,
		/* [168] */
		22784, 22784, 22784, pmTolerant, 0,
		/* [169] */
		22528, 22528, 22528, pmTolerant, 0,
		/* [170] */
		22272, 22272, 22272, pmTolerant, 0,
		/* [171] */
		22016, 22016, 22016, pmTolerant, 0,
		/* [172] */
		21760, 21760, 21760, pmTolerant, 0,
		/* [173] */
		21504, 21504, 21504, pmTolerant, 0,
		/* [174] */
		21248, 21248, 21248, pmTolerant, 0,
		/* [175] */
		20992, 20992, 20992, pmTolerant, 0,
		/* [176] */
		20736, 20736, 20736, pmTolerant, 0,
		/* [177] */
		20480, 20480, 20480, pmTolerant, 0,
		/* [178] */
		20224, 20224, 20224, pmTolerant, 0,
		/* [179] */
		19968, 19968, 19968, pmTolerant, 0,
		/* [180] */
		19712, 19712, 19712, pmTolerant, 0,
		/* [181] */
		19456, 19456, 19456, pmTolerant, 0,
		/* [182] */
		19200, 19200, 19200, pmTolerant, 0,
		/* [183] */
		18944, 18944, 18944, pmTolerant, 0,
		/* [184] */
		18688, 18688, 18688, pmTolerant, 0,
		/* [185] */
		18432, 18432, 18432, pmTolerant, 0,
		/* [186] */
		18176, 18176, 18176, pmTolerant, 0,
		/* [187] */
		17920, 17920, 17920, pmTolerant, 0,
		/* [188] */
		17664, 17664, 17664, pmTolerant, 0,
		/* [189] */
		17408, 17408, 17408, pmTolerant, 0,
		/* [190] */
		17152, 17152, 17152, pmTolerant, 0,
		/* [191] */
		16896, 16896, 16896, pmTolerant, 0,
		/* [192] */
		16640, 16640, 16640, pmTolerant, 0,
		/* [193] */
		16384, 16384, 16384, pmTolerant, 0,
		/* [194] */
		16128, 16128, 16128, pmTolerant, 0,
		/* [195] */
		15872, 15872, 15872, pmTolerant, 0,
		/* [196] */
		15616, 15616, 15616, pmTolerant, 0,
		/* [197] */
		15360, 15360, 15360, pmTolerant, 0,
		/* [198] */
		15104, 15104, 15104, pmTolerant, 0,
		/* [199] */
		14848, 14848, 14848, pmTolerant, 0,
		/* [200] */
		14592, 14592, 14592, pmTolerant, 0,
		/* [201] */
		14336, 14336, 14336, pmTolerant, 0,
		/* [202] */
		14080, 14080, 14080, pmTolerant, 0,
		/* [203] */
		13824, 13824, 13824, pmTolerant, 0,
		/* [204] */
		13568, 13568, 13568, pmTolerant, 0,
		/* [205] */
		13312, 13312, 13312, pmTolerant, 0,
		/* [206] */
		13056, 13056, 13056, pmTolerant, 0,
		/* [207] */
		12800, 12800, 12800, pmTolerant, 0,
		/* [208] */
		12544, 12544, 12544, pmTolerant, 0,
		/* [209] */
		12288, 12288, 12288, pmTolerant, 0,
		/* [210] */
		12032, 12032, 12032, pmTolerant, 0,
		/* [211] */
		11776, 11776, 11776, pmTolerant, 0,
		/* [212] */
		11520, 11520, 11520, pmTolerant, 0,
		/* [213] */
		11264, 11264, 11264, pmTolerant, 0,
		/* [214] */
		11008, 11008, 11008, pmTolerant, 0,
		/* [215] */
		10752, 10752, 10752, pmTolerant, 0,
		/* [216] */
		10496, 10496, 10496, pmTolerant, 0,
		/* [217] */
		10240, 10240, 10240, pmTolerant, 0,
		/* [218] */
		9984, 9984, 9984, pmTolerant, 0,
		/* [219] */
		9728, 9728, 9728, pmTolerant, 0,
		/* [220] */
		9472, 9472, 9472, pmTolerant, 0,
		/* [221] */
		9216, 9216, 9216, pmTolerant, 0,
		/* [222] */
		8960, 8960, 8960, pmTolerant, 0,
		/* [223] */
		8704, 8704, 8704, pmTolerant, 0,
		/* [224] */
		8448, 8448, 8448, pmTolerant, 0,
		/* [225] */
		8192, 8192, 8192, pmTolerant, 0,
		/* [226] */
		7936, 7936, 7936, pmTolerant, 0,
		/* [227] */
		7680, 7680, 7680, pmTolerant, 0,
		/* [228] */
		7424, 7424, 7424, pmTolerant, 0,
		/* [229] */
		7168, 7168, 7168, pmTolerant, 0,
		/* [230] */
		6912, 6912, 6912, pmTolerant, 0,
		/* [231] */
		6656, 6656, 6656, pmTolerant, 0,
		/* [232] */
		6400, 6400, 6400, pmTolerant, 0,
		/* [233] */
		6144, 6144, 6144, pmTolerant, 0,
		/* [234] */
		5888, 5888, 5888, pmTolerant, 0,
		/* [235] */
		5632, 5632, 5632, pmTolerant, 0,
		/* [236] */
		5376, 5376, 5376, pmTolerant, 0,
		/* [237] */
		5120, 5120, 5120, pmTolerant, 0,
		/* [238] */
		4864, 4864, 4864, pmTolerant, 0,
		/* [239] */
		4608, 4608, 4608, pmTolerant, 0,
		/* [240] */
		4352, 4352, 4352, pmTolerant, 0,
		/* [241] */
		4096, 4096, 4096, pmTolerant, 0,
		/* [242] */
		3840, 3840, 3840, pmTolerant, 0,
		/* [243] */
		3584, 3584, 3584, pmTolerant, 0,
		/* [244] */
		3328, 3328, 3328, pmTolerant, 0,
		/* [245] */
		3072, 3072, 3072, pmTolerant, 0,
		/* [246] */
		2816, 2816, 2816, pmTolerant, 0,
		/* [247] */
		2560, 2560, 2560, pmTolerant, 0,
		/* [248] */
		2304, 2304, 2304, pmTolerant, 0,
		/* [249] */
		2048, 2048, 2048, pmTolerant, 0,
		/* [250] */
		1792, 1792, 1792, pmTolerant, 0,
		/* [251] */
		1536, 1536, 1536, pmTolerant, 0,
		/* [252] */
		1280, 1280, 1280, pmTolerant, 0,
		/* [253] */
		1024, 1024, 1024, pmTolerant, 0,
		/* [254] */
		768, 768, 768, pmTolerant, 0,
		/* [255] */
		768, 768, 768, pmTolerant, 0,
		/* [256] */
		512, 512, 512, pmTolerant, 0
	}
};

resource 'CNTL' (128) {
	{0, 0, 0, 0},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0,
	""
};

resource 'CNTL' (129) {
	{0, 0, 0, 0},
	0,
	visible,
	0,
	0,
	scrollBarProc,
	0,
	""
};
