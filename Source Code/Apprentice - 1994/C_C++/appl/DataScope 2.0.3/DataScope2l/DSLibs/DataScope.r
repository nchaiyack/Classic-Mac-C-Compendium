resource 'MENU' (128, "Apple", preload) {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About DataScopeÉ", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129, "File", preload) {
	129,
	textMenuProc,
	0x7FFFFF6F,
	enabled,
	"File",
	{	/* array: 9 elements */
		/* [1] */
		"OpenÉ", noIcon, "O", noMark, plain,
		/* [2] */
		"Close", noIcon, "W", noMark, plain,
		/* [3] */
		"SaveÉ", noIcon, "S", noMark, plain,
		/* [4] */
		"Save AsÉ", noIcon, noKey, noMark, plain,
		/* [5] */
		"-", noIcon, noKey, noMark, plain,
		/* [6] */
		"Load PaletteÉ", noIcon, "L", noMark, plain,
		/* [7] */
		"Load TextÉ", noIcon, "T", noMark, plain,
		/* [8] */
		"-", noIcon, noKey, noMark, plain,
		/* [9] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (130, "Edit", preload) {
	130,
	textMenuProc,
	0x7FFFFFFC,
	enabled,
	"Edit",
	{	/* array: 6 elements */
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
		"Clear", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (132, "Numbers", preload) {
	132,
	textMenuProc,
	0x7FFFFFF7,
	enabled,
	"Numbers",
	{	/* array: 6 elements */
		/* [1] */
		"AttributesÉ", noIcon, "A", noMark, plain,
		/* [2] */
		"Synchronize", noIcon, "Y", noMark, plain,
		/* [3] */
		"Extract Selection", noIcon, noKey, noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"See Notebook", noIcon, "N", noMark, plain,
		/* [6] */
		"Calculate From Notes", noIcon, "R", noMark, plain
	}
};

resource 'MENU' (131, "Image", preload) {
	131,
	textMenuProc,
	0x7FFFFFDB,
	enabled,
	"Image",
	{	/* array: 8 elements */
		/* [1] */
		"Generate Image", noIcon, "G", noMark, plain,
		/* [2] */
		"Image SizeÉ", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Interpolated Image", noIcon, "I", noMark, plain,
		/* [5] */
		"Interpolate SizeÉ", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"Polar Image", noIcon, "P", noMark, plain,
		/* [8] */
		"Polar SizeÉ", noIcon, noKey, noMark, plain
	}
};

resource 'DLOG' (129, "Attributes", preload) {
	{32, 20, 370, 366},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	129,
	"New Dialog"
};

resource 'DLOG' (131, "open DLOG", preload) {
	{34, 89, 366, 426},
	plainDBox,
	visible,
	noGoAway,
	0x0,
	131,
	"About DS"
};

resource 'DLOG' (134, "save it?", preload) {
	{54, 30, 208, 352},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1003,
	"About Telpass"
};

resource 'DLOG' (135, "image size") {
	{34, 54, 272, 428},
	dBoxProc,
	visible,
	goAway,
	0x0,
	1135,
	"New Dialog"
};

resource 'DLOG' (133, "polar") {
	{34, 34, 344, 472},
	dBoxProc,
	visible,
	goAway,
	0x0,
	21373,
	"New Dialog"
};

resource 'DLOG' (136, "ask var") {
	{40, 40, 206, 280},
	dBoxProc,
	visible,
	goAway,
	0x0,
	21274,
	"askvar"
};

resource 'DLOG' (137, "search lib") {
	{48, 32, 348, 254},
	dBoxProc,
	visible,
	goAway,
	0x0,
	20577,
	"New Dialog"
};

resource 'DLOG' (138, "host setup") {
	{40, 40, 260, 342},
	plainDBox,
	visible,
	goAway,
	0x0,
	28197,
	"New Dialog"
};

resource 'DLOG' (8000, "HelpDialog", purgeable) {
	{38, 44, 282, 450},
	altDBoxProc,
	visible,
	noGoAway,
	0x0,
	8000,
	"New Dialog"
};

resource 'DLOG' (8001, "TextDialog", purgeable) {
	{38, 44, 282, 450},
	altDBoxProc,
	visible,
	noGoAway,
	0x0,
	8001,
	"New Dialog"
};

resource 'DLOG' (139, "about DLOG") {
	{40, 82, 372, 419},
	plainDBox,
	visible,
	noGoAway,
	0x0,
	139,
	"About DS"
};

resource 'DITL' (131, "open") {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{144, 4, 276, 331},
		Picture {
			enabled,
			151
		},
		/* [2] */
		{146, 99, 200, 243},
		Picture {
			enabled,
			1022
		},
		/* [3] */
		{283, 56, 304, 319},
		StaticText {
			enabled,
			""
		},
		/* [4] */
		{2, 4, 139, 330},
		Picture {
			enabled,
			1028
		}
	}
};

resource 'DITL' (129, "Attr.ditl", preload) {
	{	/* array DITLarray: 18 elements */
		/* [1] */
		{15, 258, 35, 318},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{311, 264, 335, 338},
		StaticText {
			enabled,
			"NEW"
		},
		/* [3] */
		{14, 183, 34, 243},
		Button {
			enabled,
			"Cancel"
		},
		/* [4] */
		{56, 49, 76, 266},
		StaticText {
			enabled,
			"filename"
		},
		/* [5] */
		{79, 186, 97, 336},
		EditText {
			enabled,
			"dep var"
		},
		/* [6] */
		{105, 186, 123, 336},
		EditText {
			enabled,
			"x axis"
		},
		/* [7] */
		{131, 186, 149, 336},
		EditText {
			enabled,
			"y axis"
		},
		/* [8] */
		{178, 186, 196, 257},
		EditText {
			enabled,
			"d fmt"
		},
		/* [9] */
		{204, 186, 222, 336},
		EditText {
			enabled,
			"max"
		},
		/* [10] */
		{231, 186, 249, 336},
		EditText {
			enabled,
			"min"
		},
		/* [11] */
		{286, 186, 304, 257},
		EditText {
			enabled,
			"cmin"
		},
		/* [12] */
		{286, 265, 304, 336},
		EditText {
			enabled,
			"cmax"
		},
		/* [13] */
		{311, 184, 335, 258},
		StaticText {
			enabled,
			"NEW"
		},
		/* [14] */
		{6, 8, 328, 181},
		Picture {
			disabled,
			300
		},
		/* [15] */
		{178, 265, 196, 336},
		EditText {
			enabled,
			"c fmt"
		},
		/* [16] */
		{15, 258, 35, 318},
		UserItem {
			enabled
		},
		/* [17] */
		{258, 192, 278, 332},
		Button {
			enabled,
			"Calculate Max/Min"
		},
		/* [18] */
		{161, 195, 174, 340},
		Picture {
			enabled,
			9994
		}
	}
};

resource 'DITL' (1003, "save it?", purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{90, 20, 110, 100},
		Button {
			enabled,
			"Yes"
		},
		/* [2] */
		{15, 10, 82, 298},
		StaticText {
			disabled,
			"Save changes to ^0 before closing?"
		},
		/* [3] */
		{119, 20, 139, 100},
		Button {
			enabled,
			"No"
		},
		/* [4] */
		{90, 20, 110, 100},
		UserItem {
			enabled
		},
		/* [5] */
		{119, 147, 139, 227},
		Button {
			enabled,
			"Cancel"
		}
	}
};

resource 'DITL' (10360, "polar non", purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{204, 223, 224, 303},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{65, 12, 134, 301},
		StaticText {
			disabled,
			"One of your scales (radius or angle valu"
			"es) is not continuously increasing.  \n\nC"
			"heck your radius and angle values."
		},
		/* [3] */
		{153, 13, 210, 199},
		StaticText {
			enabled,
			"1 2 3 4 5 6          correct\n1 2 2 3 4 4"
			"          correct\n1 2 3 0 0 4          i"
			"ncorrect"
		}
	}
};

resource 'DITL' (1034, "load text failed", purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{144, 202, 164, 282},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{51, 9, 138, 282},
		StaticText {
			disabled,
			"Load Text failed on file ^0.\n\nCheck the "
			"format of the text in the file.\nMake sur"
			"e it matches the format instructions in "
			"the manual."
		}
	}
};

resource 'DITL' (28260) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{127, 181, 147, 261},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{53, 52, 105, 255},
		StaticText {
			enabled,
			"Out of Memory.\n\nCannot complete this act"
			"ion."
		}
	}
};

resource 'DITL' (1035, purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{116, 209, 136, 289},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{51, 9, 105, 290},
		StaticText {
			disabled,
			"Cannot read from file ^0.\n\nIt must be in"
			" HDF format."
		}
	}
};

resource 'DITL' (1135, "img size") {
	{	/* array DITLarray: 15 elements */
		/* [1] */
		{195, 254, 215, 314},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{195, 254, 215, 314},
		UserItem {
			enabled
		},
		/* [3] */
		{11, 20, 32, 167},
		StaticText {
			enabled,
			"Image Size Selection"
		},
		/* [4] */
		{205, 17, 223, 179},
		StaticText {
			enabled,
			"Width Expansion Factor"
		},
		/* [5] */
		{76, 149, 97, 202},
		EditText {
			enabled,
			"height box"
		},
		/* [6] */
		{176, 72, 195, 107},
		EditText {
			enabled,
			"exp factor"
		},
		/* [7] */
		{139, 63, 160, 116},
		EditText {
			enabled,
			"width box"
		},
		/* [8] */
		{175, 37, 197, 59},
		Picture {
			enabled,
			9998
		},
		/* [9] */
		{174, 118, 196, 140},
		Picture {
			enabled,
			9997
		},
		/* [10] */
		{166, 254, 186, 314},
		Button {
			enabled,
			"Cancel"
		},
		/* [11] */
		{43, 36, 133, 141},
		Picture {
			enabled,
			9995
		},
		/* [12] */
		{95, 232, 117, 254},
		Picture {
			enabled,
			9998
		},
		/* [13] */
		{96, 313, 118, 335},
		Picture {
			enabled,
			9997
		},
		/* [14] */
		{98, 267, 117, 302},
		EditText {
			enabled,
			"exp factor"
		},
		/* [15] */
		{126, 204, 144, 366},
		StaticText {
			enabled,
			"Height Expansion Factor"
		}
	}
};

resource 'DITL' (21373, "polar size") {
	{	/* array DITLarray: 20 elements */
		/* [1] */
		{283, 340, 303, 400},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{2, 318, 22, 378},
		RadioButton {
			enabled,
			"North"
		},
		/* [3] */
		{70, 312, 89, 386},
		StaticText {
			enabled,
			"Zero Axis"
		},
		/* [4] */
		{41, 320, 61, 380},
		RadioButton {
			enabled,
			"South"
		},
		/* [5] */
		{21, 363, 41, 423},
		RadioButton {
			enabled,
			"East"
		},
		/* [6] */
		{23, 276, 43, 336},
		RadioButton {
			enabled,
			"West"
		},
		/* [7] */
		{148, 283, 169, 404},
		StaticText {
			enabled,
			"Expansion Factor"
		},
		/* [8] */
		{226, 270, 247, 323},
		StaticText {
			enabled,
			"0"
		},
		/* [9] */
		{121, 320, 140, 355},
		EditText {
			enabled,
			"0"
		},
		/* [10] */
		{194, 270, 215, 323},
		StaticText {
			enabled,
			"0"
		},
		/* [11] */
		{195, 325, 218, 431},
		StaticText {
			enabled,
			"Window Width"
		},
		/* [12] */
		{225, 324, 247, 430},
		StaticText {
			enabled,
			"Window Height"
		},
		/* [13] */
		{120, 286, 142, 308},
		Picture {
			enabled,
			9998
		},
		/* [14] */
		{120, 363, 142, 385},
		Picture {
			enabled,
			9997
		},
		/* [15] */
		{283, 264, 303, 324},
		Button {
			enabled,
			"Cancel"
		},
		/* [16] */
		{283, 340, 303, 400},
		UserItem {
			enabled
		},
		/* [17] */
		{8, 62, 29, 203},
		StaticText {
			enabled,
			"Polar Size Selection"
		},
		/* [18] */
		{145, 189, 165, 249},
		Picture {
			enabled,
			0
		},
		/* [19] */
		{40, 10, 274, 252},
		Picture {
			enabled,
			9999
		},
		/* [20] */
		{40, 10, 273, 250},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (10240, "can't read", purgeable) {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{139, 250, 159, 330},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{70, 9, 134, 330},
		StaticText {
			disabled,
			"The file \"^0\" \ncould not be read.  To be"
			" read in, it must be an HDF file which c"
			"ontains floating point data."
		}
	}
};

resource 'DITL' (21274) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{130, 151, 150, 211},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{130, 34, 150, 94},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{9, 7, 80, 229},
		StaticText {
			enabled,
			"This variable name not found.\n\nEnter a v"
			"alid variable name and\npress OK or press"
			" Cancel to exit."
		},
		/* [4] */
		{90, 36, 110, 208},
		EditText {
			enabled,
			"the var"
		},
		/* [5] */
		{130, 151, 150, 211},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (20577) {
	{	/* array DITLarray: 8 elements */
		/* [1] */
		{180, 35, 200, 185},
		Button {
			enabled,
			"Select Library File"
		},
		/* [2] */
		{101, 35, 121, 185},
		Button {
			enabled,
			"Return to Notebook"
		},
		/* [3] */
		{180, 35, 200, 185},
		UserItem {
			enabled
		},
		/* [4] */
		{13, 8, 48, 235},
		StaticText {
			enabled,
			"The function \"^0()\"\ncannot be found."
		},
		/* [5] */
		{140, 14, 176, 209},
		StaticText {
			enabled,
			"Would you like to search an external fun"
			"ction library?"
		},
		/* [6] */
		{60, 17, 96, 212},
		StaticText {
			enabled,
			"Would you like to return to the notebook"
			" window?"
		},
		/* [7] */
		{216, 5, 255, 222},
		StaticText {
			enabled,
			"Have you set up a network server to exec"
			"ute this function?"
		},
		/* [8] */
		{261, 35, 281, 185},
		Button {
			enabled,
			"Use Network Server"
		}
	}
};

resource 'DITL' (1036, "non-cont", purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{204, 223, 224, 303},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{65, 12, 135, 314},
		StaticText {
			disabled,
			"One of your scales (row or column labels"
			") is not continuously increasing or decr"
			"easing.  The interpolated image may be a"
			"ll black or incorrect in some areas."
		},
		/* [3] */
		{153, 13, 210, 199},
		StaticText {
			enabled,
			"1 2 3 4 5 6          correct\n6 5 4 3 2 1"
			"          correct\n1 2 3 0 0 4          i"
			"ncorrect"
		}
	}
};

resource 'DITL' (28197) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{175, 67, 195, 127},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{175, 180, 195, 240},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{175, 67, 195, 127},
		UserItem {
			enabled
		},
		/* [4] */
		{50, 90, 70, 290},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{85, 90, 105, 290},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{120, 90, 140, 290},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{120, 10, 162, 82},
		StaticText {
			enabled,
			"Password\n(hidden)"
		},
		/* [8] */
		{85, 10, 105, 76},
		StaticText {
			enabled,
			"Username"
		},
		/* [9] */
		{50, 10, 70, 67},
		StaticText {
			enabled,
			"Host"
		},
		/* [10] */
		{8, 26, 42, 287},
		StaticText {
			enabled,
			"Enter parameters for executing remote fu"
			"nction \"^0()\""
		}
	}
};

resource 'DITL' (1008) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{199, 235, 219, 295},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{60, 21, 84, 160},
		StaticText {
			enabled,
			"ERROR in network:"
		},
		/* [3] */
		{93, 20, 193, 311},
		StaticText {
			enabled,
			"^0"
		}
	}
};

resource 'DITL' (500, "ErrorAlert") {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{90, 15, 110, 75},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 107, 109, 295},
		StaticText {
			disabled,
			"^0"
		}
	}
};

resource 'DITL' (8000, "HelpDList", purgeable) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{216, 336, 236, 396},
		Button {
			enabled,
			"Done"
		},
		/* [2] */
		{72, 16, 208, 392},
		UserItem {
			disabled
		},
		/* [3] */
		{16, 16, 72, 392},
		StaticText {
			disabled,
			"^0"
		},
		/* [4] */
		{216, 264, 236, 324},
		Button {
			enabled,
			"Help"
		}
	}
};

resource 'DITL' (8001, "TextDList", purgeable) {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{216, 24, 236, 84},
		Button {
			enabled,
			"Done"
		},
		/* [2] */
		{216, 128, 236, 188},
		Button {
			enabled,
			"Topics"
		},
		/* [3] */
		{216, 328, 236, 388},
		Button {
			enabled,
			"Next"
		},
		/* [4] */
		{216, 232, 236, 292},
		Button {
			enabled,
			"Prior"
		},
		/* [5] */
		{8, 16, 29, 392},
		StaticText {
			disabled,
			"^0"
		},
		/* [6] */
		{24, 16, 205, 392},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (139, "about") {
	{	/* array DITLarray: 7 elements */
		/* [1] */
		{293, 13, 313, 73},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{293, 257, 313, 317},
		Button {
			enabled,
			"Help"
		},
		/* [3] */
		{271, 71, 292, 287},
		StaticText {
			enabled,
			"^1"
		},
		/* [4] */
		{139, 3, 271, 330},
		Picture {
			enabled,
			151
		},
		/* [5] */
		{2, 4, 139, 330},
		Picture {
			enabled,
			1028
		},
		/* [6] */
		{146, 99, 200, 243},
		Picture {
			enabled,
			1022
		},
		/* [7] */
		{307, 106, 327, 227},
		StaticText {
			disabled,
			"^0"
		}
	}
};

resource 'CNTL' (129, "new user", preload) {
	{120, 180, 140, 280},
	0,
	visible,
	0,
	0,
	pushButProc,
	129,
	"New User"
};

resource 'CNTL' (130, "delete", preload) {
	{90, 180, 110, 280},
	0,
	visible,
	0,
	0,
	pushButProc,
	0,
	"Delete"
};

resource 'CNTL' (131, "passwd", preload) {
	{60, 180, 80, 280},
	0,
	visible,
	0,
	0,
	pushButProc,
	0,
	"Set Password"
};

resource 'CNTL' (301, "H-bar") {
	{285, -1, 301, 385},
	0,
	visible,
	1000,
	0,
	scrollBarProc,
	0,
	""
};

resource 'CNTL' (300, "V-bar") {
	{-1, 385, 285, 401},
	0,
	visible,
	1000,
	0,
	scrollBarProc,
	0,
	""
};

resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	notMultiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	notHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	524288,
	130000
};

resource 'SIZE' (0) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	notMultiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	notHighLevelEventAware,
	onlyLocalHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	2560000,
	130000
};

resource 'BNDL' (128) {
	'NCSf',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'ICN#',
		{	/* array IDArray: 3 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129,
			/* [3] */
			2, 130
		},
		/* [2] */
		'FREF',
		{	/* array IDArray: 3 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129,
			/* [3] */
			2, 130
		}
	}
};

resource 'FREF' (128) {
	'APPL',
	0,
	"DataScope"
};

resource 'FREF' (129) {
	'_HDF',
	1,
	""
};

resource 'FREF' (130, "extern") {
	'DSff',
	2,
	"External fns"
};

resource 'PICT' (1022) {
	5136,
	{0, 0, 54, 144},
	$"0011 02FF 0C00 FFFF FFFF 0000 0000 0000"
	$"0000 0090 0000 0036 0000 0000 0000 001E"
	$"0001 000A 0000 0000 0036 0090 0098 8090"
	$"0000 0000 0036 0090 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0008 0001 0008"
	$"0000 0000 0014 BA5C 0000 0000 0000 0B68"
	$"0000 0040 0000 FFFF FFFF FFFF 0001 FC00"
	$"FC00 FC00 0002 F800 F800 F800 0003 F400"
	$"F400 F400 0004 F000 F000 F000 0005 EC00"
	$"EC00 EC00 0006 E800 E800 E800 0007 E400"
	$"E400 E400 0008 E000 E000 E000 0009 DC00"
	$"DC00 DC00 000A D800 D800 D800 000B D400"
	$"D400 D400 000C D000 D000 D000 000D CC00"
	$"CC00 CC00 000E C800 C800 C800 000F C400"
	$"C400 C400 0010 C000 C000 C000 0011 BC00"
	$"BC00 BC00 0012 B800 B800 B800 0013 B400"
	$"B400 B400 0014 B000 B000 B000 0015 AC00"
	$"AC00 AC00 0016 A800 A800 A800 0017 A400"
	$"A400 A400 0018 A000 A000 A000 0019 9C00"
	$"9C00 9C00 001A 9800 9800 9800 001B 9400"
	$"9400 9400 001C 9000 9000 9000 001D 8C00"
	$"8C00 8C00 001E 8800 8800 8800 001F 8400"
	$"8400 8400 0020 8000 8000 8000 0021 7C00"
	$"7C00 7C00 0022 7800 7800 7800 0023 7400"
	$"7400 7400 0024 7000 7000 7000 0025 6C00"
	$"6C00 6C00 0026 6800 6800 6800 0027 6400"
	$"6400 6400 0028 6000 6000 6000 0029 5C00"
	$"5C00 5C00 002A 5800 5800 5800 002B 5400"
	$"5400 5400 002C 5000 5000 5000 002D 4C00"
	$"4C00 4C00 002E 4800 4800 4800 002F 4400"
	$"4400 4400 0030 4000 4000 4000 0031 3C00"
	$"3C00 3C00 0032 3800 3800 3800 0033 3400"
	$"3400 3400 0034 3000 3000 3000 0035 2C00"
	$"2C00 2C00 0036 2800 2800 2800 0037 2400"
	$"2400 2400 0038 2000 2000 2000 0039 1C00"
	$"1C00 1C00 003A 1800 1800 1800 003B 1400"
	$"1400 1400 003C 1000 1000 1000 003D 0C00"
	$"0C00 0C00 003E 0800 0800 0800 003F 0400"
	$"0400 0400 0040 0000 0000 0000 0000 0000"
	$"0036 0090 0000 0000 0036 0090 0000 0481"
	$"00F1 000C FB00 0001 A000 0201 0303 DC00"
	$"2EFC 0009 0204 0403 0000 0101 0303 F000"
	$"0201 0302 F200 0101 01F9 0000 01D7 0000"
	$"01FE 0000 02F6 0000 01FE 0200 01ED 0020"
	$"FB00 0001 FB00 0001 ED00 0001 F500 0201"
	$"0201 F500 0102 01D9 0003 0108 0901 DD00"
	$"4F0D 0000 0100 060B 0B09 0800 0101 0001"
	$"F000 0402 0909 0C02 F800 1401 0200 0002"
	$"0608 0C11 1617 1612 0D09 0802 0000 0201"
	$"F300 0001 FC00 0001 F402 0A03 0404 0B22"
	$"3A3A 1E02 0101 F900 040B 0909 0A06 ED00"
	$"680D 0001 0104 243B 3C3E 3713 0204 0001"
	$"F100 0703 1F3A 3E3A 1403 02FB 0006 0101"
	$"0009 0E1F 37FE 3D01 3E3E FE3D 073E 3A26"
	$"1205 0001 01F6 0012 0100 0001 070A 0F17"
	$"1F26 292B 2D2B 292B 2C2C 2DFE 2F03 3133"
	$"343B FE40 0339 1003 03FC 000A 0200 1338"
	$"3E3B 3C2C 0803 01F0 0052 0400 0201 052D"
	$"FD40 032B 0B04 04F2 0009 0100 0429 4040"
	$"3F14 0402 FC00 0601 0006 1835 3E3F F540"
	$"053D 2D12 0301 02F9 0009 0102 0003 0E1D"
	$"343C 3C3E EC40 043F 1705 0201 FD00 0A01"
	$"062C 4040 3F40 3C11 0503 F000 6A0D 0001"
	$"0104 293F 403F 403E 2306 0501 F200 0701"
	$"0427 3F40 3D11 02FC 000A 0300 0C2A 3C40"
	$"403F 4040 3FFD 3D0B 3C3F 403F 3E40 403A"
	$"2206 0202 FB00 0602 0201 1028 3A3D FC40"
	$"FB3F 033E 3E3F 3FFE 3E03 3D3D 3C3F FE40"
	$"043E 1503 0201 FE00 0B01 000E 3D40 3D3F"
	$"403F 1F05 02F0 0069 FD00 0923 3E40 403D"
	$"4037 0D02 04F0 0004 213E 403C 0EFC 001C"
	$"0301 0A2A 4040 3C3E 403C 3823 0F06 0404"
	$"0A21 373E 403D 3D40 3C1B 0203 01FD 0011"
	$"0102 041A 363F 4040 3E3C 3B32 251C 1917"
	$"1615 FD13 0112 11FE 1208 0E0C 1D38 3F40"
	$"340E 02FA 0008 1C3D 3F40 403F 3F30 08EF"
	$"005C FD00 0324 3E40 3FFE 3E03 1F04 0301"
	$"F100 0422 3F40 3D0F FD00 0B02 0105 213F"
	$"3E3C 403D 2E13 05FA 000A 0617 3640 3E3C"
	$"4037 1101 03FE 000B 0102 061A 3840 3E3B"
	$"3926 1206 F000 0501 071D 2928 13F9 000B"
	$"0532 403F 3D34 3D40 3B0C 0002 F100 5CFE"
	$"000B 0127 403F 3F40 3D40 3811 0104 F100"
	$"0422 3F40 3D10 FE00 0A01 0202 1F3C 3F3C"
	$"4037 1705 F600 140A 273F 3E3D 402A 0602"
	$"0100 0102 051D 3840 392D 1B0B F800 F801"
	$"0603 1A35 3A34 1A02 FA00 0D01 000B 3B40"
	$"3D26 153A 403C 1800 03F1 0065 FD00 0B26"
	$"3F3F 403A 403D 4028 0502 02F2 0004 1F3F"
	$"403D 11FE 0008 0400 1239 403B 4030 10F6"
	$"0021 0100 0007 2A40 3C40 3B0F 0001 0003"
	$"0113 3540 351F 0B00 0005 0B14 212B 333A"
	$"3B3B 3C3B FA3C 013A 3CFE 4002 3309 02F9"
	$"000C 1E3C 403E 1406 3A40 3F30 0501 01F2"
	$"005F FD00 0B24 3E40 3A1E 393F 3F3B 1500"
	$"03F2 0012 1F3F 403D 1000 0002 0007 2D40"
	$"3C40 3410 0003 01F6 0017 0300 0C37 403E"
	$"3E1B 0100 0101 092E 4035 1702 0412 2437"
	$"3B3C F040 063F 3D40 4013 0301 FD00 0F01"
	$"0005 3540 403A 0B00 283D 403B 0E00 03F2"
	$"0067 FD00 0C23 3D40 3907 2340 3D40 310A"
	$"0002 F300 111D 3F40 3D11 0000 0101 1F3D"
	$"3E3F 3B16 0003 01F3 0001 011E FE3E 0D2E"
	$"0501 0500 133A 3A18 0413 2B39 3CFE 40FE"
	$"3EFD 3D02 3C3D 3DFC 3C01 3B3F FE40 0336"
	$"0E04 02FD 0010 0100 0D3C 403E 2A05 000F"
	$"3C40 3E25 0302 01F3 006A FE00 0D01 253E"
	$"403C 0409 323F 3E3E 1C01 03F3 0004 1D3F"
	$"403D 12FE 0008 0A38 403D 4023 0402 01F3"
	$"001A 0100 0A3D 4040 3C09 0003 042A 3E22"
	$"061D 3A3C 3F40 3B38 2817 0E0A 07FE 050E"
	$"0304 0503 0205 0502 0620 363D 361A 01FA"
	$"000F 0123 3D40 3F16 0000 043B 4040 3A0C"
	$"0001 F300 58FD 000D 233F 403C 0A00 183F"
	$"3D40 360A 0104 F400 041E 3F40 3E13 FE00"
	$"071A 3D3E 403C 1100 02F0 0013 0635 3D3C"
	$"370E 0100 043B 3C0E 0F35 403C 3A2D 1608"
	$"F100 0404 080A 0807 F800 0505 373F 403A"
	$"0CFE 0006 2B40 403E 1D00 01F3 005F FD00"
	$"0E20 3F40 3A0A 0006 3540 3D3E 1C01 0301"
	$"F500 0D1B 3E40 3D13 0000 0531 403E 402D"
	$"08ED 0003 0809 0907 FE00 090B 3C27 0926"
	$"3D39 3522 0BFD 00FB 01FE 0208 0304 020C"
	$"2B3B 3723 02F9 0007 0100 0D3B 403E 2504"
	$"FE00 0714 3E40 3F33 0600 01F4 0061 FD00"
	$"0E1F 3F40 3A06 0000 1B3D 3E40 380F 0004"
	$"F500 0E1B 3E40 3D13 0200 073B 4040 3E18"
	$"0101 EC00 0001 FD00 0F1E 3C11 1337 3A26"
	$"1100 0009 111F 2F39 3BFC 3C05 3D3C 3C3D"
	$"3B3B FE40 033C 1306 02FA 0005 0122 3C40"
	$"3C12 FD00 0708 3B40 403C 0C00 02F4 0058"
	$"FD00 0C1F 3F40 3B04 0000 0429 403D 4023"
	$"FE02 F600 0C1D 3F40 3D13 0000 153D 3F40"
	$"3A0B EB00 0102 02FD 000C 323A 0D2C 3F25"
	$"0B03 1022 373C 3FF2 4006 3E40 4017 0603"
	$"01FD 0007 0100 0839 403E 2D06 FD00 0602"
	$"2A40 3F3E 2001 F300 64FD 0004 223F 403C"
	$"05FE 0008 123A 3F3E 3A14 0003 01F7 000C"
	$"1F3F 403E 1500 002A 3F40 402A 03E6 0016"
	$"063B 2E0C 3B36 0D0B 2137 3E40 3D3D 3E3D"
	$"3D3C 3C3D 3D3C 3BFE 3C01 3A3A FE40 0438"
	$"0E02 0101 FD00 0802 0013 3C40 3D19 0001"
	$"FD00 0714 3E40 4038 0800 01F5 005D FD00"
	$"0422 3F40 3D09 FE00 0801 2240 3D40 3410"
	$"0103 F700 0B1F 3F40 3D14 0001 3740 403E"
	$"18E5 0020 083A 2418 3D1C 0C2C 3E40 3A2F"
	$"1B0F 0801 0203 0200 0204 0100 0102 010B"
	$"283B 371D 02F9 0005 0227 3D40 3B0D FB00"
	$"0708 3A40 3F3D 1200 02F5 0057 FD00 1122"
	$"3F40 3C0A 0000 0100 0C38 403C 4028 0502"
	$"02F8 000B 1E3F 403D 1300 013A 4040 3E10"
	$"E500 1307 2C1D 2B36 0D22 4039 2910 0601"
	$"0000 0308 0E0D 05F9 0002 0106 06F7 0005"
	$"0939 403E 2805 FB00 0702 2640 3F3E 2605"
	$"01F5 0051 FD00 0421 3F40 3C0A FC00 071A"
	$"3E3E 3F3B 1502 04F8 000B 1B3E 403D 1100"
	$"043B 4040 3C0A E500 0903 151C 3319 0D3A"
	$"3814 06FE 0008 0823 383A 3B3B 3A2B 0CEE"
	$"0005 163C 403D 1601 FA00 070F 3E40 403A"
	$"0B00 01F6 0057 FD00 041F 3F40 3D0B FC00"
	$"0807 3240 3C40 2B05 0102 F900 0B1B 3E40"
	$"3D10 0008 3B40 403B 07E4 000D 051D 3A09"
	$"203C 1B01 0004 0004 293E FB40 032F 0A02"
	$"01FE 0002 0105 03F8 0005 0229 3D40 3C0D"
	$"F900 0708 3840 3F3D 1100 02F6 0057 FD00"
	$"041D 3F40 3C0A FB00 0718 3D3E 3F3C 1400"
	$"04F9 000B 1C3F 403E 1200 0C3C 4040 3C08"
	$"E300 0D16 370D 362D 0603 0300 0019 3D40"
	$"3EFE 4005 3E40 3E1A 0202 FD00 0102 01FA"
	$"0007 0100 0739 403E 2C05 F800 051B 3E40"
	$"3E25 03F5 0059 FD00 041F 3F40 3C08 FB00"
	$"0804 2B40 3C40 2F08 0103 FA00 0B19 3E40"
	$"3E11 0011 3D40 403B 08E3 001C 0C26 0E36"
	$"1700 0300 0005 3440 3D40 3F3E 3F40 3E3F"
	$"3008 0102 0000 0108 01F8 0005 133C 403E"
	$"1901 F800 0709 3B40 403A 0B00 02F7 0059"
	$"FD00 041E 3F40 3C08 FA00 0711 3840 3D3D"
	$"1D00 02FA 000B 183E 403D 0F00 133D 4040"
	$"3A08 E300 0404 160F 2F09 FD00 0406 3B40"
	$"3F40 FE3E 0B40 3F40 3B09 0002 0000 0618"
	$"07FB 0007 0100 022A 3E40 3B0D F700 0702"
	$"293F 403D 1500 02F7 0058 FD00 041B 3E40"
	$"3C08 FB00 0901 001D 3D3D 4037 0C00 03FB"
	$"000B 193E 403E 1100 153D 4040 3807 E200"
	$"030B 1326 03FD 0003 053A 4040 FD3E 0C3F"
	$"3F40 3B07 0000 0100 0824 1601 FC00 0702"
	$"000C 3A40 3E2A 05F6 0005 123E 403E 2B04"
	$"F600 5AFD 0004 1B3F 403C 09F9 0008 0629"
	$"403D 3F22 0202 01FC 000B 193E 403D 1000"
	$"143D 4040 3604 E200 0301 0B19 02FD 0003"
	$"053B 403F FC3E 0D3F 403A 0900 0002 0314"
	$"2824 0800 01FD 0007 0224 3E40 3E17 0101"
	$"F700 0707 3B40 403B 0B00 01F8 0058 FD00"
	$"041D 3F40 3C09 F800 0710 3B3F 3F39 0F00"
	$"03FC 000B 1A3F 403D 1100 103C 4040 3A03"
	$"E100 0201 0801 FD00 0406 3840 3E3F FE3E"
	$"FE3F 0130 08FE 0005 0829 2425 1003 FE00"
	$"0701 000A 3940 403B 0CF5 0007 0227 3F40"
	$"3D1A 0101 F800 53FD 0004 1D3F 403C 08F8"
	$"0008 0227 403D 3F25 0201 02FD 000B 193E"
	$"403D 1200 0C3C 4040 3C05 E000 0001 FB00"
	$"0323 3F3E 3CFE 3F0D 3C3E 3E1E 0000 0200"
	$"0D37 1C27 1C11 FC00 0517 3D40 3F29 04F4"
	$"0005 113E 403F 2F05 F700 57FD 0004 1F3F"
	$"403D 08F7 0007 0F3A 403E 3A11 0004 FD00"
	$"0B18 3E40 3D10 0009 3C40 403C 08E1 0002"
	$"0303 01FC 001E 0D37 403D 3E3E 3D3D 4037"
	$"0D00 0302 001E 3C16 3122 2405 0001 0003"
	$"2E3E 3F3F 18F3 0007 083C 403F 3B09 0002"
	$"F900 57FD 0004 1E3F 403C 08F7 0008 0122"
	$"3F3D 4029 0500 02FE 000B 193E 403D 1200"
	$"043B 4040 3C0A E200 0402 0000 0201 FC00"
	$"081A 363D 3D3F 3E3E 361A FD00 1014 3B37"
	$"1336 1C32 0F00 0200 0B3A 3F40 3B0A F200"
	$"062A 4040 3B18 0001 F900 5EFD 0004 1B3E"
	$"403C 08F6 0005 0A35 403D 3D1C FE01 0D00"
	$"0019 3E40 3D14 0001 3B40 403D 12E1 0001"
	$"0405 F900 060B 1D2C 3633 200B FE00 1802"
	$"0F35 3F1B 1539 1539 1600 0001 1F3D 3F3F"
	$"310B 0402 0406 0403 FC05 0B04 0203 031B"
	$"3D40 3F2F 0500 01FA 0063 FD00 041B 3E40"
	$"3C09 F500 161B 3D3E 4038 1500 0200 0019"
	$"3E40 3D13 0001 3940 403E 2202 E300 0613"
	$"272A 1403 0001 FB00 2A01 0102 0000 0607"
	$"0E25 3740 290A 2B3E 103A 1B02 0006 3640"
	$"3F40 3F38 3633 383A 3633 3637 3637 3939"
	$"3435 373B FE40 033B 1200 02FA 005F FD00"
	$"041D 3F40 3D09 F500 0606 2D40 3C40 3309"
	$"FE00 0719 3E40 3E12 0000 2CFE 4001 3305"
	$"E600 2501 0011 3840 403B 2B1D 1A19 1513"
	$"1517 1714 1316 212B 3338 3A3F 4034 0E11"
	$"3D37 0F3A 1E03 000E 3CFE 4003 3F40 3F3F"
	$"F640 003F FB40 032A 0500 01FB 0065 FD00"
	$"041C 3F40 3C08 F400 0512 3A40 3D3E 1DFE"
	$"000E 183E 403E 1200 0017 3E40 403B 0B00"
	$"01ED 0002 0102 01FE 0001 031C FC40 093E"
	$"3F3F 3E3E 3D3D 3E3F 40FE 3F10 4040 3E38"
	$"2E14 0A2B 3D1C 133A 1B00 0024 3FFE 4002"
	$"3F3E 3FF5 400A 3F3E 3E3F 403F 403B 1300"
	$"02FB 0060 FD00 0419 3E40 3C08 F400 1601"
	$"2040 3C40 370C 0000 143E 403E 1300 0009"
	$"3C40 3F3D 1D01 EC00 0002 FC00 0402 1A40"
	$"403F F340 153D 3B35 1D11 0904 1D3E 350D"
	$"253D 1600 0439 403E 3F35 2FFD 2D13 2C2B"
	$"2C2D 2E2D 2E2D 2C2C 2D2D 2B2E 3A40 3E40"
	$"2902 FA00 69FD 0004 1A3E 403C 08F5 0019"
	$"0100 0B39 403D 3F1E 0000 163D 403E 1400"
	$"0005 3940 3D40 3103 0001 EF00 3002 0607"
	$"0600 0001 0008 3640 403C 3430 3132 322F"
	$"2D2E 2D2C 2C29 1E10 0705 0000 0218 343F"
	$"270D 393E 0F00 0B3C 403E 2A08 0001 F300"
	$"0A01 0001 143C 3E40 3906 0001 FC00 56FD"
	$"0004 1C3F 403C 08F3 0017 0123 3F3D 4037"
	$"0900 1B3D 403E 1500 0001 2540 3F40 3A0E"
	$"0002 F000 0E03 1F2E 342D 0A00 0001 071F"
	$"3336 1B02 F200 1003 0C19 3140 3814 103E"
	$"3A09 0020 3C3F 3D16 ED00 0704 3940 3F3C"
	$"1100 02FC 005A FD00 041A 3E40 3C08 F200"
	$"0B08 2D40 3D3F 2301 183F 403E 17FE 0008"
	$"0F39 403C 4027 0400 02F3 000F 0300 0B3A"
	$"4040 3E10 0010 2637 2C14 0801 F500 1401"
	$"040A 1528 3639 3F3E 210A 273F 2903 0535"
	$"3F40 390A ED00 0601 2D40 3E3F 2702 FB00"
	$"64FD 0004 193E 403C 08F1 000A 1139 3F3E"
	$"380C 113F 403E 19FE 0009 0225 403D 3F3C"
	$"1700 0403 F500 1401 0201 1C40 3F40 3B0D"
	$"0936 3E40 4039 2F2C 2A27 292B FD2A 1829"
	$"282B 3138 3B3B 3D40 403E 3B26 0816 3C39"
	$"1700 0D3A 403E 2904 EC00 0718 3E40 4039"
	$"0900 01FD 0058 FD00 0418 3E40 3C08 F000"
	$"091D 3D3E 3D23 153C 403D 1AFD 0009 0D36"
	$"403C 4037 1500 0302 F700 0D01 0300 1038"
	$"403E 4027 0209 3E3F 3DFD 4001 3F3F F340"
	$"0F3C 3831 1903 0E34 4029 0903 243B 403E"
	$"17EB 0007 093B 403E 3E1A 0001 FD00 5EFD"
	$"0004 173E 403D 09F0 000A 0930 3F3D 3A2B"
	$"3D40 3E1B 01FD 000A 183B 403C 4035 1100"
	$"0103 01FA 000C 0204 000A 2D40 3C40 390E"
	$"0008 37FD 4001 3C3D F73E 153D 3D3C 3B39"
	$"2D1A 0F07 000F 2D40 3518 0007 383F 403B"
	$"0BEB 0006 0329 403E 4031 06FC 0062 FD00"
	$"0418 3E40 3C06 EF00 0916 3C3E 3D3C 3F40"
	$"3F1B 01FD 0008 0223 403D 3C40 361A 04F9"
	$"0009 0100 000C 2A40 3D3E 3E20 FE00 081F"
	$"383F 3B29 150F 0D0D FD0F 190D 0B0C 0A09"
	$"0805 0303 0100 0001 172E 3C3A 220A 000C"
	$"3A40 3F29 02EA 0005 103C 4040 3C0D FC00"
	$"52FD 0004 173E 403C 06EF 0008 062D 3F3C"
	$"3E3F 403F 1AFB 0009 092F 403C 3C40 3B24"
	$"0E02 FA00 1003 112F 403D 3C40 2F09 0001"
	$"0009 161E 1401 EF00 0E04 0D1C 353D 3E2E"
	$"1002 0120 3C40 3E18 E900 0505 3240 3F3D"
	$"1BFC 005D FD00 0417 3E40 3D08 EE00 0714"
	$"393F 3C3F 403E 1AFA 000A 1135 403E 3C3F"
	$"403A 2612 0AFE 0915 0A11 283B 403C 3E40"
	$"3716 0001 1E37 3928 1103 0001 0100 F601"
	$"1203 070A 172B 373B 403D 3015 0500 0234"
	$"4040 3C0E E800 071A 3E3F 4031 0500 01FF"
	$"005A FD00 0418 3D40 3C08 EE00 0802 253F"
	$"3C3D 403C 1A01 FA00 0710 2F3E 403F 3E40"
	$"3FFB 3D0B 3C3F 403E 403E 2F15 0200 1236"
	$"FE40 053B 3A3C 3B3A 3BF6 3C12 3D3D 3C3E"
	$"4040 3F38 260F 0304 0012 3A3F 3F2E 06E8"
	$"0007 0B3B 403F 3B0A 0003 FF00 47FD 0004"
	$"193F 403D 09ED 0007 0E37 403E 403F 1D02"
	$"F900 0309 2138 3EF6 400B 3F3E 381D 0500"
	$"0002 1C3F 3E3E EB40 093C 3929 1204 0002"
	$"0002 30FE 4000 19E7 0009 0229 4040 3E1E"
	$"0100 0200 52FD 0004 173A 3E39 0FED 0007"
	$"011F 383B 3B39 1A02 F700 0F08 162F 3B3B"
	$"3C3D 3E3E 3D3B 3A3B 331B 09FC 0001 113B"
	$"FE40 033B 3B3C 3CF7 3DFD 3C05 3B38 2411"
	$"0A03 FC00 050A 363B 3D39 0FE6 0008 1439"
	$"3D3B 2D0B 0001 003F FD00 0301 0B0A 0BEA"
	$"0004 0506 0508 01F4 000B 0203 0710 171B"
	$"1B15 0C06 0302 FA00 0704 293B 3A2C 1005"
	$"04F3 0302 0201 02F7 0003 0607 0B0A E400"
	$"030C 0A07 05FD 000D BC00 0303 0706 01E1"
	$"0000 01DB 0012 FC00 0201 0301 A100 0102"
	$"02E2 0001 0202 FC00 0BFB 0000 01C2 0001"
	$"0203 B900 00FF"
};

resource 'PICT' (1028, "opening") {
	20224,
	{51, 88, 188, 414},
	$"0011 02FF 0C00 FFFF FFFF 0058 0000 0033"
	$"0000 019E 0000 00BC 0000 0000 0000 00A0"
	$"0082 00A0 008E 001E 0001 000A 0033 0058"
	$"00BC 019E 0098 814E 0033 0058 00BC 01A5"
	$"0000 0000 0000 0000 0048 0000 0048 0000"
	$"0000 0008 0001 0008 0000 0000 0000 1F10"
	$"0000 0000 0000 043C 8000 00FF 0000 FFFF"
	$"FFFF FFFF 0000 8200 8200 8200 0000 8300"
	$"8300 8300 0000 8400 8400 8400 0000 8500"
	$"8500 8500 0000 C200 C200 C200 0000 8600"
	$"8600 8600 0000 8700 8700 8700 0000 8800"
	$"8800 8800 0000 8A00 8A00 8A00 0000 8B00"
	$"8B00 8B00 0000 C300 C300 C300 0000 8C00"
	$"8C00 8C00 0000 8D00 8D00 8D00 0000 8E00"
	$"8E00 8E00 0000 8F00 8F00 8F00 0000 9000"
	$"9000 9000 0000 C400 C400 C400 0000 9100"
	$"9100 9100 0000 9200 9200 9200 0000 9300"
	$"9300 9300 0000 9400 9400 9400 0000 9500"
	$"9500 9500 0000 C500 C500 C500 0000 9600"
	$"9600 9600 0000 9700 9700 9700 0000 9800"
	$"9800 9800 0000 9900 9900 9900 0000 B100"
	$"B100 B100 0000 C600 C600 C600 0000 C700"
	$"C700 C700 0000 C800 C800 C800 0000 C900"
	$"C900 C900 0000 CA00 CA00 CA00 0000 CB00"
	$"CB00 CB00 0000 CC00 CC00 CC00 0000 9B00"
	$"9B00 9B00 0000 9C00 9C00 9C00 0000 9D00"
	$"9D00 9D00 0000 9E00 9E00 9E00 0000 B200"
	$"B200 B200 0000 CE00 CE00 CE00 0000 9F00"
	$"9F00 9F00 0000 CD00 CD00 CD00 0000 5A00"
	$"5A00 5A00 0000 5B00 5B00 5B00 0000 5C00"
	$"5C00 5C00 0000 5D00 5D00 5D00 0000 A000"
	$"A000 A000 0000 5E00 5E00 5E00 0000 5F00"
	$"5F00 5F00 0000 6000 6000 6000 0000 6100"
	$"6100 6100 0000 6200 6200 6200 0000 A100"
	$"A100 A100 0000 6300 6300 6300 0000 6400"
	$"6400 6400 0000 6500 6500 6500 0000 6700"
	$"6700 6700 0000 CF00 CF00 CF00 0000 B300"
	$"B300 B300 0000 6800 6800 6800 0000 6900"
	$"6900 6900 0000 6A00 6A00 6A00 0000 6B00"
	$"6B00 6B00 0000 D000 D000 D000 0000 D100"
	$"D100 D100 0000 D200 D200 D200 0000 D300"
	$"D300 D300 0000 D400 D400 D400 0000 D500"
	$"D500 D500 0000 D600 D600 D600 0000 A200"
	$"A200 A200 0000 A300 A300 A300 0000 A400"
	$"A400 A400 0000 A500 A500 A500 0000 B400"
	$"B400 B400 0000 D700 D700 D700 0000 A600"
	$"A600 A600 0000 6C00 6C00 6C00 0000 6D00"
	$"6D00 6D00 0000 6E00 6E00 6E00 0000 6F00"
	$"6F00 6F00 0000 D800 D800 D800 0000 A700"
	$"A700 A700 0000 7000 7000 7000 0000 9A00"
	$"9A00 9A00 0000 2D00 2D00 2D00 0000 2E00"
	$"2E00 2E00 0000 2F00 2F00 2F00 0000 A800"
	$"A800 A800 0000 7100 7100 7100 0000 3000"
	$"3000 3000 0000 3100 3100 3100 0000 3200"
	$"3200 3200 0000 3400 3400 3400 0000 B500"
	$"B500 B500 0000 7200 7200 7200 0000 3500"
	$"3500 3500 0000 3600 3600 3600 0000 3700"
	$"3700 3700 0000 3800 3800 3800 0000 D900"
	$"D900 D900 0000 DA00 DA00 DA00 0000 3900"
	$"3900 3900 0000 3A00 3A00 3A00 0000 3B00"
	$"3B00 3B00 0000 3C00 3C00 3C00 0000 A900"
	$"A900 A900 0000 AA00 AA00 AA00 0000 AC00"
	$"AC00 AC00 0000 AD00 AD00 AD00 0000 B600"
	$"B600 B600 0000 DB00 DB00 DB00 0000 AE00"
	$"AE00 AE00 0000 7300 7300 7300 0000 7400"
	$"7400 7400 0000 7500 7500 7500 0000 7600"
	$"7600 7600 0000 DC00 DC00 DC00 0000 AF00"
	$"AF00 AF00 0000 7800 7800 7800 0000 3D00"
	$"3D00 3D00 0000 3E00 3E00 3E00 0000 3F00"
	$"3F00 3F00 0000 4000 4000 4000 0000 B000"
	$"B000 B000 0000 7900 7900 7900 0000 4100"
	$"4100 4100 0000 6600 6600 6600 0000 1400"
	$"1400 1400 0000 1500 1500 1500 0000 B700"
	$"B700 B700 0000 7A00 7A00 7A00 0000 4200"
	$"4200 4200 0000 1600 1600 1600 0000 1700"
	$"1700 1700 0000 1800 1800 1800 0000 DD00"
	$"DD00 DD00 0000 DF00 DF00 DF00 0000 4300"
	$"4300 4300 0000 1900 1900 1900 0000 1A00"
	$"1A00 1A00 0000 1B00 1B00 1B00 0000 B800"
	$"B800 B800 0000 B900 B900 B900 0000 BA00"
	$"BA00 BA00 0000 BB00 BB00 BB00 0000 BD00"
	$"BD00 BD00 0000 E000 E000 E000 0000 BE00"
	$"BE00 BE00 0000 7B00 7B00 7B00 0000 7C00"
	$"7C00 7C00 0000 7D00 7D00 7D00 0000 7E00"
	$"7E00 7E00 0000 E100 E100 E100 0000 BF00"
	$"BF00 BF00 0000 7F00 7F00 7F00 0000 4500"
	$"4500 4500 0000 4600 4600 4600 0000 4700"
	$"4700 4700 0000 4800 4800 4800 0000 C000"
	$"C000 C000 0000 8000 8000 8000 0000 4900"
	$"4900 4900 0000 1C00 1C00 1C00 0000 1D00"
	$"1D00 1D00 0000 1E00 1E00 1E00 0000 C100"
	$"C100 C100 0000 8100 8100 8100 0000 4A00"
	$"4A00 4A00 0000 1F00 1F00 1F00 0000 3300"
	$"3300 3300 0000 0700 0700 0700 0000 E200"
	$"E200 E200 0000 E300 E300 E300 0000 4B00"
	$"4B00 4B00 0000 2000 2000 2000 0000 0800"
	$"0800 0800 0000 0900 0900 0900 0000 E400"
	$"E400 E400 0000 E500 E500 E500 0000 E600"
	$"E600 E600 0000 E700 E700 E700 0000 E800"
	$"E800 E800 0000 E900 E900 E900 0000 EA00"
	$"EA00 EA00 0000 EB00 EB00 EB00 0000 EC00"
	$"EC00 EC00 0000 ED00 ED00 ED00 0000 EE00"
	$"EE00 EE00 0000 0000 CCCC 0000 0000 0000"
	$"9999 FFFF 0000 0000 9999 CCCC 0000 4C00"
	$"4C00 4C00 0000 4D00 4D00 4D00 0000 4E00"
	$"4E00 4E00 0000 4F00 4F00 4F00 0000 0000"
	$"6666 FFFF 0000 0000 6666 CCCC 0000 5000"
	$"5000 5000 0000 2100 2100 2100 0000 2300"
	$"2300 2300 0000 2400 2400 2400 0000 0000"
	$"3333 FFFF 0000 0000 3333 CCCC 0000 5100"
	$"5100 5100 0000 2500 2500 2500 0000 0A00"
	$"0A00 0A00 0000 0B00 0B00 0B00 0000 0000"
	$"0000 FFFF 0000 46E3 0000 A53E 0000 5200"
	$"5200 5200 0000 2600 2600 2600 0000 0C00"
	$"0C00 0C00 0000 FFFF 648A 028C 0000 DDDD"
	$"0000 0000 0000 5700 5700 5700 0000 5300"
	$"5300 5300 0000 2A00 2A00 2A00 0000 2700"
	$"2700 2700 0000 1000 1000 1000 0000 0D00"
	$"0D00 0D00 0000 0400 0400 0400 0000 0100"
	$"0100 0100 0000 0000 EEEE 0000 0000 0000"
	$"DDDD 0000 0000 5800 5800 5800 0000 5400"
	$"5400 5400 0000 2B00 2B00 2B00 0000 2800"
	$"2800 2800 0000 1200 1200 1200 0000 0E00"
	$"0E00 0E00 0000 0500 0500 0500 0000 0200"
	$"0200 0200 0000 0000 0000 EEEE 0000 0000"
	$"0000 DDDD 0000 5900 5900 5900 0000 5600"
	$"5600 5600 0000 2C00 2C00 2C00 0000 2900"
	$"2900 2900 0000 1300 1300 1300 0000 0F00"
	$"0F00 0F00 0000 0600 0600 0600 0000 0300"
	$"0300 0300 0000 EF00 EF00 EF00 0000 DE00"
	$"DE00 DE00 0000 BC00 BC00 BC00 0000 AB00"
	$"AB00 AB00 0000 8900 8900 8900 0000 7700"
	$"7700 7700 0000 5500 5500 5500 0000 4400"
	$"4400 4400 0000 2200 2200 2200 0000 1100"
	$"1100 1100 0000 0000 0000 0000 0033 0058"
	$"00BC 019E 0033 0058 00BC 019E 0000 0006"
	$"8100 8100 B300 0008 8100 B100 8EF2 F600"
	$"0012 0100 0081 F2B6 F2FE 0000 F291 3001"
	$"F2F2 F600 0016 0200 00F2 8130 B930 01F2"
	$"F2FE 0000 F291 3001 F2F2 F600 0016 0200"
	$"00F2 8130 B930 01F2 F2FE 0000 F291 3001"
	$"F2F2 F600 0024 0200 00F2 8130 B930 01F2"
	$"F2FE 0001 F230 FBFF 0030 F6FF 0030 F9FF"
	$"C930 E6FF 0230 F2F2 F600 002C 0200 00F2"
	$"8130 B930 01F2 F2FE 0000 F2F9 3000 FFF8"
	$"3000 FFF1 3005 F6F6 3030 F6F6 F030 01F6"
	$"F6CD 3001 F2F2 F600 0040 0200 00F2 B230"
	$"05F6 F630 30F6 F6F0 3001 F6F6 A130 01F2"
	$"F2FE 0001 F230 FBFF 0130 FFF8 3001 FF30"
	$"F9FF FA30 05F6 F630 30F6 F6F0 3001 F6F6"
	$"E930 E6FF 0230 F2F2 F600 004F 0200 00F2"
	$"B230 05F6 F630 30F6 F6F0 3001 F6F6 A130"
	$"01F2 F2FE 0000 F2F9 3000 FFF8 3000 FFF1"
	$"3005 F6F6 3030 F6F6 FE30 FDF6 FE30 05F6"
	$"F630 F6F6 30FD F6FE 30FD F6FE 3005 F6F6"
	$"3030 F6F6 DE30 01F2 F2F6 0000 7402 0000"
	$"F2B2 3005 F6F6 3030 F6F6 FE30 FDF6 FE30"
	$"05F6 F630 F6F6 30FD F6FE 30FD F6FE 3005"
	$"F6F6 3030 F6F6 B230 01F2 F2FE 0001 F230"
	$"FBFF 0130 FFF8 3001 FF30 F9FF FA30 0FF6"
	$"F630 30F6 F630 30F6 F630 30F6 F630 30FE"
	$"F6FD 3001 F6F6 FE30 0DF6 F630 30F6 F630"
	$"30F6 F630 30F6 F6FA 30E6 FF02 30F2 F2F6"
	$"0000 7202 0000 F2B2 300F F6F6 3030 F6F6"
	$"3030 F6F6 3030 F6F6 3030 FEF6 FD30 01F6"
	$"F6FE 300D F6F6 3030 F6F6 3030 F6F6 3030"
	$"F6F6 B230 01F2 F2FE 0000 F2F9 3000 FFF8"
	$"3000 FFF1 3011 F6F6 3030 F6F6 3030 F6F6"
	$"3030 F6F6 3030 F6F6 FC30 01F6 F6FE 300D"
	$"F6F6 3030 F6F6 3030 F6F6 3030 F6F6 DE30"
	$"01F2 F2F6 0000 7102 0000 F2B2 3011 F6F6"
	$"3030 F6F6 3030 F6F6 3030 F6F6 3030 F6F6"
	$"FC30 01F6 F6FE 300D F6F6 3030 F6F6 3030"
	$"F6F6 3030 F6F6 B230 01F2 F2FE 0001 F230"
	$"FBFF 0130 FFF8 3001 FF30 F9FF FA30 11F6"
	$"F630 30F6 F630 30F6 F630 30F6 F630 30F6"
	$"F6FC 3001 F6F6 FE30 FBF6 FE30 FDF6 F930"
	$"E6FF 0230 F2F2 F600 0066 0200 00F2 B230"
	$"11F6 F630 30F6 F630 30F6 F630 30F6 F630"
	$"30F6 F6FC 3001 F6F6 FE30 FBF6 FE30 FDF6"
	$"B130 01F2 F2FE 0000 F2F9 3000 FFF8 3000"
	$"FFF1 3011 F6F6 3030 F6F6 3030 F6F6 3030"
	$"F6F6 3030 F6F6 FC30 01F6 F6FE 3001 F6F6"
	$"FB30 05F6 F630 30F6 F6DE 3001 F2F2 F600"
	$"0077 0200 00F2 B230 11F6 F630 30F6 F630"
	$"30F6 F630 30F6 F630 30F6 F6FC 3001 F6F6"
	$"FE30 01F6 F6FB 3005 F6F6 3030 F6F6 B230"
	$"01F2 F2FE 0001 F230 FBFF 0130 FFF8 3001"
	$"FF30 F9FF FA30 04F6 F630 30F6 FE30 09F6"
	$"F630 30F6 F630 30F6 F6FC 3001 F6F6 FE30"
	$"01F6 F6FE 3008 F630 30F6 F630 30F6 F6FA"
	$"30E6 FF02 30F2 F2F6 0000 6602 0000 F2B2"
	$"3004 F6F6 3030 F6FE 3009 F6F6 3030 F6F6"
	$"3030 F6F6 FC30 01F6 F6FE 3001 F6F6 FE30"
	$"08F6 3030 F6F6 3030 F6F6 B230 01F2 F2FE"
	$"0000 F2F9 3000 FFF8 3000 FFF1 30FD F6FC"
	$"30FD F6FE 3001 F6F6 FB30 01F6 F6FE 30FD"
	$"F6FE 3005 F6F6 3030 F6F6 DE30 01F2 F2F6"
	$"0000 4102 0000 F2B2 30FD F6FC 30FD F6FE"
	$"3001 F6F6 FB30 01F6 F6FE 30FD F6FE 3005"
	$"F6F6 3030 F6F6 B230 01F2 F2FE 0001 F230"
	$"FBFF 0030 F6FF 0030 F9FF C930 E6FF 0230"
	$"F2F2 F600 0016 0200 00F2 8130 B930 01F2"
	$"F2FE 0000 F291 3001 F2F2 F600 0016 0200"
	$"00F2 8130 B930 01F2 F2FE 0000 F291 3001"
	$"F2F2 F600 0016 0200 00F2 8130 B930 01F2"
	$"F2FE 0000 F291 3001 F2F2 F600 0011 0200"
	$"00F2 8130 B930 01F2 F2FE 008E F2F6 0000"
	$"5F01 0000 81F2 B6F2 FE00 25F2 52ED 8CC4"
	$"2CE3 FBD9 ED2D 2E31 3438 813E 4F51 5250"
	$"4050 5B74 FA76 6161 7474 615B 555B 7475"
	$"73FE 6104 5B55 5252 55F7 5BFD 55FD 5201"
	$"5151 FE50 FE4F FD40 FC3F F23E FE3D FE3A"
	$"0C81 8139 3981 3D38 343D 5250 F2F2 F600"
	$"0069 0200 00F2 D300 00FF 8100 F700 00FF"
	$"F300 01F2 F2FE 0009 F22E AA64 8CC8 DAEE"
	$"2C32 FD33 1E35 393E 5152 5150 4F50 5574"
	$"FA76 7361 7373 615B 555B 7374 615B 6173"
	$"615B 5555 F75B FB55 FE52 FE51 FE50 FD4F"
	$"FD40 FE3F F93E F73D 013A 3AFE 810A 3938"
	$"813A 3834 3D52 50F2 F2F6 0000 7102 0000"
	$"F2D3 0000 FF81 00F7 0000 FFFB 0000 FFFA"
	$"0001 F2F2 FE00 08F2 A469 E565 9FC4 EE31"
	$"FE39 1E35 3234 383E 5555 504F 4F50 5573"
	$"FAFA 7361 7373 615B 5555 7374 615B 7374"
	$"7361 FC5B FC55 015B 5BFA 55FE 52FD 51FE"
	$"50FD 4FFE 40FE 3FFB 3EF6 3D0F 3A3A 8181"
	$"3938 3881 3A38 343D 5250 F2F2 F600 0076"
	$"0200 00F2 D300 00FF 8100 F700 00FF FC00"
	$"02FF 00FF FB00 01F2 F2FE 0026 F231 C36A"
	$"8CB0 C3C8 EE2E 3133 3435 3839 3D4F 5152"
	$"5040 4F55 7376 7573 6174 7461 5B55 5B74"
	$"7461 5BFE 6105 5B5B 6161 7373 F261 FD5B"
	$"0155 55FE 52FD 5101 5050 FE4F FE40 FD3F"
	$"F83E FE3D 0E3A 3A81 8139 3881 3A37 343D"
	$"5250 F2F2 F600 0080 0200 00F2 D300 00FF"
	$"8100 F700 00FF FD00 00FF FE00 00FF FC00"
	$"01F2 F2FE 0027 F276 34C2 C4C8 C3A4 C3D4"
	$"D92E 3339 8181 3A3D 4055 523F 4055 7374"
	$"7461 6174 7461 5B5B 6174 7573 5B5B FE55"
	$"0261 7374 FE75 FE74 FE73 F874 FD73 0161"
	$"61FE 5BFE 5501 5252 FE51 0150 50FE 4FFE"
	$"40FC 3FFC 3E01 3D3D FE3A 0A39 3981 3A37"
	$"333D 5251 F2F2 F600 0090 0200 00F2 D300"
	$"00FF F300 FDFF D500 01FF FFD7 00FC FFE6"
	$"0000 FFFE 0000 FFFC 0000 FFFD 0001 F2F2"
	$"FE00 25F2 5B33 C5E4 EDD9 E4EE 2C2E 3234"
	$"3538 813D 404F 5050 4F50 5573 7675 6161"
	$"7575 735B 5B61 7576 74FE 6105 5B5B 6161"
	$"7374 FA75 0276 76FA FE76 FEFA FD76 1175"
	$"7474 7373 6161 5B5B 5555 5251 5150 504F"
	$"4FFE 40FC 3FFD 3EFE 3DFE 3A0B 8181 393A"
	$"3D38 343D 5250 F2F2 F600 0093 0200 00F2"
	$"D300 00FF F300 00FF D300 00FF D500 00FF"
	$"FE00 00FF E600 03FF 0000 FFFA 0000 FFFE"
	$"0001 F2F2 FE00 08F2 3E2E C8ED 3735 3334"
	$"FE37 1934 3234 393E 5150 3E3F 5152 5573"
	$"79FA 7361 7575 7361 6173 76FA 74FC 7304"
	$"615B 7374 74FD 7503 76FA 7979 FD7F FE85"
	$"147F 7F79 79FA FA76 7575 7473 7361 5B55"
	$"5552 5150 504F FE40 FD3F FC3E 013D 3DFE"
	$"3AFC 8108 3A3E 3935 3D52 50F2 F2F6 0000"
	$"9602 0000 F2D3 0000 FFF3 00FE FFD6 0000"
	$"FFD0 0000 FFE6 0002 FF00 FFF8 0004 FF00"
	$"00F2 F2FE 0011 F238 CE7C AAD9 EEDA FBED"
	$"2D31 3335 3881 3E4F FE50 124F 515B 7476"
	$"7673 7374 7473 6161 7375 7674 6173 FE74"
	$"0B73 7375 76FA 7979 7F7F 8597 98FD 9901"
	$"9A9A FE9D 169A 9A99 9998 9785 7F79 FA76"
	$"7574 7361 5B55 5251 5150 4F4F FE40 FE3F"
	$"FD3E 033D 3D3A 3AFE 810A 3939 3A3D 3935"
	$"3E52 50F2 F2F6 0000 8F02 0000 F2D3 0000"
	$"FFF0 0000 FFD7 00FD FFD4 0000 FFE5 00FC"
	$"FFFC 00FD FF02 00F2 F2FE 0018 F232 9EEF"
	$"68A1 9F8C A0C2 D4E3 3139 3A3A 3D3F 5161"
	$"523F 4F61 75FB 74FD 730C 7575 7361 7374"
	$"7676 7575 FA7F 97FE 9809 999A 9DA3 A9A9"
	$"0101 0202 FC03 1602 0201 A99D 9A99 9785"
	$"79FA 7675 7473 615B 5552 5150 504F FE40"
	$"FD3F 123E 3E3D 3D3A 8181 3938 3881 3A38"
	$"353E 5551 F2F2 F600 009E 0200 00F2 D300"
	$"00FF EF00 00FF D800 00FF FE00 00FF D500"
	$"00FF E500 00FF FE00 00FF FC00 00FF FD00"
	$"01F2 F2FE 0027 F281 C869 A1D9 FBCE E4D9"
	$"ED2E 3235 383A 3F50 5255 514F 5161 75FA"
	$"7673 7375 7574 7373 7476 FA75 7475 FD76"
	$"11FA 7F97 999A 9DA3 A901 0306 0809 0A0D"
	$"0F12 13FC 1519 1413 100E 0C09 0703 019D"
	$"9885 7976 7473 5B55 5252 5150 4F4F 4040"
	$"FE3F 123E 3E3D 3D3A 3A81 8139 3981 3A38"
	$"353E 5251 F2F2 F600 00A1 0200 00F2 D300"
	$"00FF EF00 00FF D800 00FF FE00 00FF D600"
	$"00FF E400 00FF FE00 00FF FC00 00FF FD00"
	$"01F2 F2FE 003F F252 31C2 2D4F 3E38 3881"
	$"3835 332F 343A 4F5B 554F 4F51 5561 7597"
	$"7F61 5B76 FA74 7373 7479 7F79 FA79 79FA"
	$"76FA 7997 999D A901 0306 0709 0C0F 1418"
	$"5626 3048 4A4B FE4E 1C4B 4936 2A25 1A15"
	$"100C 0702 9D98 7F76 7573 615B 5552 5150"
	$"504F 4040 3F3F FE3E 013D 3DFE 3A0A 8139"
	$"813A 3834 3D52 50F2 F2F6 0000 9E02 0000"
	$"F2D3 0000 FFEF 0000 FFD8 0000 FFFE 0000"
	$"FFD6 0000 FFE4 0000 FFFE 0000 FFFC 0000"
	$"FFFD 0001 F2F2 FE00 3CF2 34C5 6BAA EDED"
	$"E32C 2F32 3334 3538 3A40 5252 5151 5052"
	$"7376 7F79 7473 76FA 7574 7475 797F 7979"
	$"8597 9785 9798 A302 0407 090D 1216 562A"
	$"484E 5AF8 6E6F FB72 216F 6F6E F86C 5A4B"
	$"4827 5616 100A 04A9 9879 7573 5B55 5251"
	$"504F 4F40 3F3F 3E3E 3D3A 3AFE 810A 3939"
	$"813A 3835 3D52 50F2 F2F6 0000 9302 0000"
	$"F2D3 0000 FFF0 0000 FFD6 00FE FFD5 0000"
	$"FFE4 0000 FFFE 00FA FFFD 0001 F2F2 FE00"
	$"18F2 C569 CF5C 6B80 9EAA C8EE 2F37 3D3E"
	$"3A3E 4051 6152 3F50 75FA F976 0675 76FA"
	$"7F79 8599 FD9D 1AA3 04F9 0A0E 1318 274B"
	$"6F4C 9093 F7F7 9391 9084 604C 4C3C 4C4C"
	$"6070 FE84 1C70 3C78 6D4A 2515 0C04 9D7F"
	$"7561 5552 5150 4F4F 403F 3F3E 3D3A 3A81"
	$"3939 FE38 0881 3A38 353E 5550 F2F2 F600"
	$"008D 0200 00F2 D300 00FF F300 FEFF 8800"
	$"00FF F300 01F2 F2FE 001E F22F A05D 7BC3"
	$"C5C5 E4ED 2D2E 3237 393A 3F50 525B 5550"
	$"5274 FA79 FA76 75FA FAFE 7601 FA85 FE97"
	$"1899 9DA9 0204 070C 1219 274A 6D72 1C28"
	$"3C1C 786E 5A4B 362A 2524 FE1B 2356 2426"
	$"3048 4B5A F86F 726F 6E6C 4A27 1910 08A9"
	$"9776 6155 5150 4F40 403F 3F3E 3D3A 8181"
	$"39FE 3808 3981 3835 3E52 50F2 F2F6 0000"
	$"8B02 0000 F2D3 0000 FF81 00F7 0000 FFF3"
	$"0001 F2F2 FE00 72F2 7631 9FD9 3F3A 3538"
	$"3A37 2F2D 2C32 3A4F 5B5B 5555 5B61 7376"
	$"9785 7574 7979 7676 FA7F 999D 9A99 999D"
	$"0106 090E 141B 4B7E 91A2 96F7 4C6E 481B"
	$"1309 049D 9879 FA75 7576 FA7F 98A3 0409"
	$"121A 2A4E 6E28 4C60 286E 4A1B 0F04 98FA"
	$"6155 514F 4F40 403F 3F3E 3E3D 3A81 3937"
	$"3738 3937 343D 5250 F2F2 F600 0087 0200"
	$"00F2 D300 00FF 8100 F700 F2FF 01F2 F2FE"
	$"0008 F24F D980 C42F 2DED 2EFD 3304 3438"
	$"3D4F 52FE 5529 525B 7479 9785 7675 7F7F"
	$"79FA 7F97 9DA9 A901 0306 080C 1526 4B6F"
	$"1C4C 3C1C 6C48 1A0E 0499 FA5B 513F 3D81"
	$"FD39 1B81 3A3E 4052 61FA 9902 0A13 5636"
	$"5AF8 6FF8 4E2A 180D 0398 765B 514F 40FE"
	$"3F06 3E3E 3D3A 3A81 39FE 3806 3534 3A50"
	$"40F2 F2F6 0000 7D02 0000 F281 FFC7 FFF3"
	$"0001 F2F2 FE00 1EF2 E39F 5F69 80FC A0C2"
	$"CEED 353D 5150 3F3F 4050 5B52 3F51 FA85"
	$"857F FAFA 8597 FE85 1A98 A302 0307 0D10"
	$"1014 3072 F71D 943C 4A16 0897 5B3D 3731"
	$"2E2C 2CED FE2C F92D 172E 2F33 383E 557F"
	$"0310 244E 6F1C 7E6D 3015 0899 7555 5040"
	$"3FFE 3EFD 3D01 3A81 FE39 0635 3439 3F3E"
	$"F2F2 F600 0084 0200 00F2 D300 00FF 8100"
	$"F700 00FF F300 01F2 F2FE 001D F235 C263"
	$"80C3 C4C5 DAE3 2D32 3581 3D3E 4051 5561"
	$"5550 5576 7F97 8579 7985 FE98 1699 9D03"
	$"0708 0A13 5636 5A72 3C4C 284B 1AF9 975B"
	$"3D37 312E FD2C 042D 2E2F 2F31 FA32 FE31"
	$"1732 3438 3E55 7901 0D19 3654 6D5A 361B"
	$"0E03 9773 524F 3F3E 3EFE 3DFE 3A09 8181"
	$"3938 3781 3E3D F2F2 F600 0088 0200 00F2"
	$"D300 00FF 8100 F700 00FF F300 01F2 F2FE"
	$"0063 F279 2D7B DA3E 8135 383A 33E3 D9D9"
	$"2F81 5073 7461 7374 7461 7699 9979 7997"
	$"9999 9AA3 0208 0A0A 0E18 4884 1194 3C2A"
	$"0D9D 5B3E 3533 3132 3234 3535 3738 3839"
	$"3981 3A3D 3E3F 4040 4F4F 403F 3E81 3834"
	$"3232 3339 4F79 0615 366D 726E 4818 0899"
	$"7552 4F3F 3E3D FC3A FE81 0639 393A 3D3D"
	$"F2F2 F600 0083 0200 00F2 D300 00FF 8100"
	$"F700 00FF F300 01F2 F2FE 0011 F252 FB6A"
	$"C232 2F2C 3135 322D 2E31 373A 4F5B FD73"
	$"1E74 7579 9999 8585 999D 9DA3 0207 0C12"
	$"1825 4E78 8491 F825 F9FA 5037 322D 2CED"
	$"FE2C FC2D 0B2E 2E2F 3132 3335 3739 813A"
	$"3DFE 3E19 3A81 3735 3334 373E 5B98 0918"
	$"3654 5448 1A0D 0179 6150 403E 3D3D FD3A"
	$"FD81 FE3A 01F2 F2F6 0000 B302 0000 F2F5"
	$"00FD FFFE 00FE FFE9 0000 FFF9 00FE FFFB"
	$"00FE FFFD 0000 FFE4 00FE FFFB 00FE FFFD"
	$"0000 FFE4 00FE FFFB 00FE FFFE 00FE FFED"
	$"0000 FFF3 0001 F2F2 FE00 0EF2 D99E AC69"
	$"86FC 9EC2 E42D 353F 6152 FE3F 0651 7473"
	$"5573 9799 FE98 1E99 9DA3 A902 070A 101A"
	$"4978 9394 6E24 F985 5B3D 3834 322F 2CD9"
	$"E4CE C5C2 B0AA FEA4 2BAA B0C2 C3C5 CEDA"
	$"FBE3 2D31 3439 3D4F 5152 514F 3E39 3533"
	$"374F 79F9 1A4B F85A 3013 0497 7352 403F"
	$"3E3D 3D3A 3AFC 8103 3A3A F2F2 F600 00CF"
	$"0200 00F2 F500 00FF FC00 00FF FE00 00FF"
	$"EA00 00FF FA00 00FF FE00 00FF FD00 00FF"
	$"FE00 04FF 0000 FFFF E500 00FF FE00 00FF"
	$"FD00 00FF FE00 04FF 0000 FFFF E500 00FF"
	$"FE00 00FF FD00 00FF FE00 02FF 00FF FE00"
	$"00FF EE00 00FF F300 01F2 F2FE 0069 F235"
	$"C469 86C2 C3C3 CEEE 2D33 8150 503F 4F51"
	$"5B74 7355 7385 999A 9A99 9A01 0304 070A"
	$"1325 4E7E 8484 781A 0276 3E37 312C D9E4"
	$"CEC4 B0A1 9EFC 8686 8086 868C FC9E A0A1"
	$"AAC2 C4C8 CEE4 EEED 2E32 373A 3F40 4F40"
	$"3E39 3534 353D 73A9 1226 4A4B 2615 089A"
	$"7655 5040 3F3E 3D3D FA3A 01F2 F2F6 0000"
	$"C502 0000 F2F5 00FE FFFE 0000 FFFE 0000"
	$"FFEA 0000 FFFA 0000 FFFE 0000 FFF9 0000"
	$"FFFE 0000 FFE5 0000 FFFE 0000 FFF9 0000"
	$"FFFE 0000 FFE5 0000 FFFE 0000 FFF9 0002"
	$"FF00 FFFE 0000 FFEE 0000 FFF3 0001 F2F2"
	$"FE00 12F2 A981 AA2C 403E 3938 3531 E3ED"
	$"2D34 3D55 7979 FE73 5074 7485 0202 9A9D"
	$"0408 090C 1056 6E90 F790 4A12 9D55 3F37"
	$"2EFB C4A1 8C7B 6A68 6565 696A 6B7B 7D86"
	$"FC9E A0A4 B0C2 C3C5 C5C8 C8C5 C8C8 D4DA"
	$"D92D 3339 3F51 5251 3F38 3433 8174 0216"
	$"484E 4A1A 0CA3 765B 5040 FE3F 013E 3EFC"
	$"3D01 F2F2 F600 00C3 0200 00F2 F200 03FF"
	$"0000 FFFE 0000 FFEA 0000 FFFA 0000 FFFE"
	$"0000 FFFA 0000 FFFD 0000 FFE5 0000 FFFE"
	$"0000 FFFA 0000 FFFD 0000 FFE5 0000 FFFE"
	$"0000 FFFA 0003 FF00 00FF FE00 00FF EE00"
	$"00FF F300 01F2 F2FE 0014 F299 35A0 E437"
	$"3735 3738 332E 3134 393D 5175 7675 75FE"
	$"762F 9702 03A3 A907 0C0E 1525 5470 9460"
	$"5A10 9851 372F D9C8 A48C 7A68 625F AC5F"
	$"6264 6869 6B7B 7D86 FC9F A1AA C2C4 C8CE"
	$"D4D4 FCCE 1DDA FBED 2F35 3D40 504F 3D37"
	$"3131 3851 9D0F 2448 271A 0CA9 7F61 514F"
	$"403F 3FFC 3E02 3DF2 F2F6 0000 CC02 0000"
	$"F2F1 0002 FF00 FFFE 0000 FFEA 0002 FF00"
	$"00FD FF01 00FF FE00 00FF FB00 00FF FC00"
	$"00FF EA00 FDFF 0100 FFFE 0000 FFFB 0000"
	$"FFFC 0000 FFEA 00FD FF01 00FF FE00 00FF"
	$"FB00 00FF FE00 00FF FE00 00FF EE00 00FF"
	$"F300 01F2 F2FE 0069 F234 C263 6A80 A0C2"
	$"FB32 373A 5279 733F 3D3D 527F 85FA 7F99"
	$"9DA9 0102 0609 0E14 266E 8494 9048 0D98"
	$"5038 2CC5 9E7D 6A65 625F 5F62 6264 6568"
	$"696A 6B7A 7B7D 86FC 9FA4 C2C5 D4E4 EEE3"
	$"EDED 2CED E3D9 EEFB FBEE ED31 383E 4F4F"
	$"3D37 2E2E 3855 0316 2748 5610 037F 7350"
	$"403F FA3E 01F2 F2F6 0000 C402 0000 F2F1"
	$"0002 FF00 FFFE 0000 FFEA 0000 FFFA 0000"
	$"FFFE 0000 FFFC 0000 FFFB 0000 FFE5 0000"
	$"FFFE 0000 FFFC 0000 FFFB 0000 FFE5 0000"
	$"FFFE 0000 FFFC 0000 FFFD 0000 FFFE 0000"
	$"FFEE 0000 FFF3 0001 F2F2 FE00 31F2 3EEE"
	$"FC9E A1B0 C5EE 2E33 3740 745B 4040 5061"
	$"7F85 7F85 9AA9 0204 06F9 0E15 265A 70F7"
	$"705A 0E79 4F34 E3C5 9E6B 64AC 5D58 59FE"
	$"5C14 5D5E ACAC 5F62 6364 6569 6B7B 809E"
	$"A1C2 C5D4 E4EE E3FD ED1C E3EE FBFB EEED"
	$"2F35 3D3F 3E81 31ED E331 519A 0E1B 5619"
	$"0D02 8561 513F 3EFC 3D02 3EF2 F2F6 0000"
	$"D002 0000 F2F1 0002 FF00 FFFE 0000 FFEA"
	$"0000 FFFA 0000 FFFE 0000 FFFD 0000 FFFE"
	$"0000 FFFE 0000 FFE5 0000 FFFE 0000 FFFD"
	$"0000 FFFE 0000 FFFE 0000 FFE5 0000 FFFE"
	$"0000 FFFD 0000 FFFE 0002 FF00 FFFE 0000"
	$"FFEE 0000 FFF3 0001 F2F2 FE00 2FF2 0D97"
	$"3D3F 503F 8134 2EE3 E4D9 2C37 4F76 9D99"
	$"7676 8597 859D 0809 F90C 1426 F860 9491"
	$"4B10 7F3D 34E3 B080 6B65 5F5C 57FE DB02"
	$"E6D5 D5FE DC15 E6F0 DBE5 5759 5D5E 6368"
	$"7A80 FCA0 AAC2 C4CE DAEE E32C FE2E 1A2D"
	$"2CED ED2E 3238 3A39 34ED FBE3 3361 020F"
	$"1A15 0F04 8573 4F3F 3A3A FD81 01F2 F2F6"
	$"0000 C102 0000 F2F2 0000 FFFE 00FE FFE9"
	$"0000 FFF9 00FE FFFE 0001 FF00 FCFF 0100"
	$"00FE FFE5 00FE FFFE 0001 FF00 FCFF 0100"
	$"00FE FFE5 00FE FFFE 0001 FF00 FCFF 0100"
	$"00FE FFED 0000 FFF3 0001 F2F2 FE00 35F2"
	$"1603 735B 5251 4F3D 3732 2D2F 333A 5076"
	$"9A99 7979 8597 98A9 090D 0C12 255A 70F7"
	$"926F 1398 4F31 D9C4 8064 625E 57F0 DCCF"
	$"CBCA FDB1 B1FE C934 CACF D5DC F0E5 EF57"
	$"5D5F 657A 7D8C 9FA1 AAC2 C5CE E4D9 ED2D"
	$"2E2E 2DED E3E3 2C32 3538 352D FBC8 CE31"
	$"4099 0A0F 12F9 017F 5B4F 3D81 39FE 3801"
	$"F2F2 F600 008F 0200 00F2 F500 FEFF E200"
	$"00FF 8100 F700 00FF F300 01F2 F2FE 002D"
	$"F206 7681 2ED4 2D39 405B 6173 8502 9752"
	$"4F4F 74A3 9D7F 97A9 0609 0D10 1A4E 3C9C"
	$"966F 1BA3 4F37 2CC4 8C68 5D58 DBCB FEAB"
	$"41A7 A6A7 A7AB B1C9 FDCB D5E6 DBE5 5758"
	$"595E 6269 7C8C 9FA1 A4AA B0B0 C2C3 C4C8"
	$"DAEE ED2E 3131 2E2C EDED 2D2F 312E FBCE"
	$"C8E4 355B 9D09 0908 A379 5B3F 3A37 3534"
	$"35F2 F2F6 0000 8B02 0000 F2D3 0000 FF81"
	$"00F7 0000 FFF3 0001 F2F2 FE00 49F2 0E01"
	$"763D 2C34 3F55 7576 769A 079D 6151 5276"
	$"A9A9 9899 0208 0D12 1630 7EF7 A891 2A07"
	$"6135 2EDA 9E68 5EEF DCCA A78E A5A6 A58F"
	$"A6AB B1B1 C9FD CACB D5E6 DBE5 EFEF 585D"
	$"636B 809E A0A4 AAFE B025 C2C2 C3C5 D4EE"
	$"ED2E 312F 2DE3 D9D9 E32D 2DD9 D4B0 A4CE"
	$"2E4F 85A9 0601 9975 513E 3835 3333 F2F2"
	$"F600 008A 0200 00F2 D300 00FF 8100 F700"
	$"00FF F300 01F2 F2FE 003D F272 6C49 18F9"
	$"0198 765B 3E35 8150 5B75 9906 0398 9A02"
	$"0201 0712 1A25 5492 A8F7 6E0E FA3E 2FE4"
	$"AA6A 5CE5 DCC9 A68F 8EA5 A7AB B1FD CAC9"
	$"B1B1 ABAB A7B1 FDCB FECF 0AD5 DC57 5F69"
	$"7C8C A0AA C3C5 FDC8 22C5 C8C8 D4FB E32E"
	$"2F31 2EE3 FBDA DAE4 E4C8 C3B0 C2D9 3755"
	$"989A 9D7F 734F 8135 3232 F2F2 F600 008B"
	$"0200 00F2 D300 00FF 8100 F700 00FF F300"
	$"01F2 F2FE 004C F24B 546D 481B 1309 0399"
	$"7652 5573 7999 030A 089D A306 0703 F915"
	$"2649 78A2 054C 2AA3 4037 EDC3 7DAC F0D5"
	$"CAA6 8D8F A5AB FDCB D5E6 F0D5 CAC9 ABA6"
	$"8EA5 A7B1 FDC9 C9B1 B1D5 575F 697C 8CA0"
	$"B0C3 C8FE CE22 C8C8 C5C8 D4FB ED2E 3131"
	$"2CEE D4C8 D4DA CEC4 A19E AACE 3250 7598"
	$"7FFA 553F 3933 33F2 F2F6 0000 8A02 0000"
	$"F2D3 0000 FF81 00F7 0000 FFF3 0001 F2F2"
	$"FE00 51F2 4076 A903 060D 1456 3048 4A49"
	$"4818 08A3 859D 0909 0607 090F 162A F890"
	$"119C 5A10 7638 31E4 9E64 57CF CAC9 A58E"
	$"A6C9 CBDC E558 5C5E 59EF DCCA A78D 8EA5"
	$"ABFD FDC9 A78F B1E6 585F 687A 80FC A0AA"
	$"C2C4 C8CE D4FD DA1C E4EE 2C2E 2F2E D9D4"
	$"C4C2 C3C4 B0A4 A0A0 C5E3 3955 7579 7455"
	$"3F38 34F2 F2F6 0000 8B02 0000 F2D3 0000"
	$"FF81 00F7 0000 FFF3 0001 F2F2 FE00 53F2"
	$"C32C 3A3F 50FA A90C 1930 5A6E 784B 190A"
	$"A904 0F0E 08F9 0D14 1B4A 4C9C 1792 2602"
	$"5132 2CC8 7B5C F0CA C9A7 8F8F C9DC EF5E"
	$"657C 86FC 807B 645C DCA7 8F8D A6C9 FDCA"
	$"A789 A6CA DB5C 6269 7B80 FCA0 A4C2 C4C8"
	$"D4DA E4FE DA13 E4D9 2C2F 312C FBC4 AAB0"
	$"C2C2 B09F 8C9F AAEE 354F FE74 0451 3E39"
	$"F2F2 F600 008B 0200 00F2 D300 00FF 8100"
	$"F700 00FF F300 01F2 F2FE 0072 F2EE 2D34"
	$"D9C2 A4A0 C2D4 2E3A 850F 266C F86C 260E"
	$"0A0E 0E0A 132A 6FF7 0B05 2814 853F 31EE"
	$"AA65 E5DC FDAB A58F A6D5 5968 FCC8 2D33"
	$"3935 33FB B069 DBB1 888F ABCA DCB1 898F"
	$"C9E6 5962 6A7C 8C9F A1A4 AAC2 C3C4 C8D4"
	$"E4FB EEEE D9E3 2C2D 2DED E4C4 AAA0 A4AA"
	$"A4A1 9EFC AACE 2E3A 5175 735B 4FF2 F2F6"
	$"0000 B402 0000 F2F5 00FD FFFD 0000 FFE8"
	$"0000 FFF9 00FE FFFB 00FE FFFD 0000 FFE4"
	$"00FE FFFB 00FE FFFE 00FE FFE5 00FE FFFB"
	$"00FE FFFE 00FE FFED 0000 FFF3 0001 F2F2"
	$"FE00 55F2 403F 3EED A46B 5C59 5964 80EE"
	$"5106 276E 846E 1913 1613 0D15 4A3C 0B1D"
	$"9CF8 0973 3A2E DA9F 5FDC CFFD A7A5 ABCB"
	$"5E7C C42E 4079 9A02 01A3 753F FB7D EF8D"
	$"8D8F FDF0 C989 8EA7 CFEF 5E68 7B86 9EA1"
	$"A4AA B0B0 C3C4 C8DA FBFD D918 E32C 2E2E"
	$"E3DA B0A0 A0A1 A4AA 9F8C 8C9E C52C 3952"
	$"6174 5BF2 F2F6 0000 D202 0000 F2F5 0000"
	$"FFFB 0001 FFFF E800 00FF FA00 00FF FE00"
	$"00FF FD00 00FF FE00 04FF 0000 FFFF E500"
	$"00FF FE00 00FF FD00 00FF FE00 02FF 00FF"
	$"FE00 00FF E700 00FF FE00 00FF FD00 00FF"
	$"FE00 02FF 00FF FE00 00FF EE00 00FF F300"
	$"01F2 F2FE 0064 F212 0E09 A375 37CE A480"
	$"8C9E A4C2 EE35 9824 6D1C 6D24 1818 254B"
	$"3C1D 1F93 4B04 553A 2EC5 805C CFCA C9A7"
	$"ABF0 62AA 3152 9D0A 161B 2627 301B 1398"
	$"39FC FD8D F1A5 DCB1 8788 8DAB CFEF AC68"
	$"7C8C A0A4 B0C3 C4C4 C5C8 CED4 E4EE E3ED"
	$"2C2D 2E2E EDEE C8B0 A09E A0FE A10A 9EFC"
	$"9FAA E432 3A55 61F2 F2F6 0000 C402 0000"
	$"F2F5 00FE FFFC 0000 FFE8 0000 FFFA 0000"
	$"FFFE 0000 FFF9 0000 FFFE 0000 FFE5 0000"
	$"FFFE 0000 FFF9 0002 FF00 FFFE 0000 FFE7"
	$"0000 FFFE 0000 FFF9 0002 FF00 FFFE 0000"
	$"FFEE 0000 FFF3 0001 F2F2 FE00 26F2 2A56"
	$"1610 097F 3A2E D4C3 AA8C 6A69 6BD9 851B"
	$"704C 4B2A 2748 6C70 2020 9030 0152 3A2F"
	$"C37B 59CF FECB 30E6 64A0 3275 0313 1B27"
	$"3048 546E 6F72 260A 3463 FDF2 8DD5 B188"
	$"8787 8FC9 E659 626A 7D9E A1B0 C3C4 C5C5"
	$"C8C8 CEDA FBE3 2C2D FD2E 13ED E4C5 A4FC"
	$"9E9F A4B0 A19E 8C80 A4CE 2E3A 50F2 F2F6"
	$"0000 C702 0000 F2F2 0000 FFFD 0000 FFE8"
	$"0000 FFFA 0000 FFFE 0000 FFFA 0000 FFFD"
	$"0000 FFE5 0000 FFFE 0000 FFFA 0003 FF00"
	$"00FF FE00 00FF E700 00FF FE00 00FF FA00"
	$"03FF 0000 FFFE 0000 FFEE 0000 FFF3 0001"
	$"F2F2 FE00 5BF2 9885 7974 5B4F 3D37 31D9"
	$"C8B0 A09E 8CA1 CE4F 104B 4C7E 4849 1C96"
	$"2320 702A 0255 3E31 C47C 5EDB EF58 5E68"
	$"B02F 529A 0206 02A3 9DA3 0A18 4B3C 3C28"
	$"0934 69F1 8ECF C98E 8988 8FB1 DC57 5F68"
	$"7B86 9EA1 AAC2 C4C5 C8D4 DADA FBEE E32C"
	$"2EFE 3113 2EE3 E4C4 A49F FC9F A1AA B0A1"
	$"9F9F A0C4 FB33 F2F2 F600 00D2 0200 00F2"
	$"F100 00FF FE00 00FF E800 02FF 0000 FDFF"
	$"0100 FFFE 0000 FFFB 0000 FFFC 0000 FFEA"
	$"00FD FF01 00FF FE00 00FF FB00 00FF FE00"
	$"00FF FE00 00FF EC00 FDFF 0100 FFFE 0000"
	$"FFFB 0000 FFFE 0000 FFFE 0000 FFEE 0000"
	$"FFF3 0001 F2F2 FE00 72F2 5140 3A38 3539"
	$"3D3D 8132 EEDA CEC3 A17D 64B0 4F0F 4C90"
	$"5A6D 7005 2920 702A 0373 4033 C580 645D"
	$"5F65 7CA1 2C40 7598 FA5B 812E 2CED 3D7F"
	$"146F F723 F808 DAA6 A7CF C9A5 8E89 8FAB"
	$"CFEF 5E65 7A80 FC9F A1B0 C3C4 C8D4 DAE4"
	$"E4FB D9ED 2E31 3333 322E E3DA C4A1 FC9E"
	$"9FB0 C4C2 AAA0 8C9E A1E4 F2F2 F600 00C9"
	$"0200 00F2 F100 00FF FE00 00FF E800 00FF"
	$"FA00 00FF FE00 00FF FC00 00FF FB00 00FF"
	$"E500 00FF FE00 00FF FC00 00FF FD00 00FF"
	$"FE00 00FF E700 00FF FE00 00FF FC00 00FF"
	$"FD00 00FF FE00 00FF EE00 00FF F300 01F2"
	$"F2FE 0004 F20C 9D61 50FE 3D6A 3A81 3531"
	$"EDEE C8AA 9E80 9EC2 3D10 5470 9270 9C41"
	$"2B90 36F9 7955 38E4 A07C 6A7B 86A1 C52E"
	$"3939 35E4 9F64 DBDC DC68 C24F 0CF8 2090"
	$"4B50 5FF0 CBAB 8F8D 888E A6CB DB5D 647A"
	$"809E A0A4 AAC2 C3C4 C8CE DAFB EEE3 ED2D"
	$"2F32 3435 332F E3DA C4A4 9FFC A0AA C2C4"
	$"C4C3 A49F C2F2 F2F6 0000 D102 0000 F2F1"
	$"0000 FFFE 0000 FFE8 0000 FFFA 0000 FFFE"
	$"0000 FFFD 0000 FFFE 0000 FFFE 0000 FFE5"
	$"0000 FFFE 0000 FFFD 0000 FFFE 0002 FF00"
	$"FFFE 0000 FFE7 0000 FFFE 0000 FFFD 0000"
	$"FFFE 0002 FF00 FFFE 0000 FFEE 0000 FFF3"
	$"0001 F2F2 FE00 72F2 1503 7351 3E3D 3A3A"
	$"8138 3732 2DFB C5C2 AA86 64B0 5012 70A2"
	$"91A2 433B 934B 0D99 743E EDC3 9F8C A0AA"
	$"C5E4 2D31 FBAA 64CB 82F3 F3AD 8F5D E4FA"
	$"301F 05F7 02A0 5FCB AB8F 8987 8DA5 FDF0"
	$"5C63 6B80 9EA0 A4AA B0C2 C3C4 C8D4 E4EE"
	$"E3ED 2C2E 3134 3737 352F EDDA C4A4 FC9E"
	$"9FB0 C5D4 DAC3 A0A0 F2F2 F600 00C1 0200"
	$"00F2 F200 00FF FE00 FEFF E900 00FF F900"
	$"FEFF FE00 01FF 00FC FF01 0000 FEFF E500"
	$"FEFF FE00 01FF 00FC FF01 0000 FEFF E500"
	$"FEFF FE00 01FF 00FC FF01 0000 FEFF ED00"
	$"00FF F300 01F2 F2FE 0072 F235 322D 3133"
	$"3739 3837 3735 3534 312C FBC5 A48C 9EC3"
	$"3F14 78A8 2344 29A2 6E15 027F 5032 DAC2"
	$"A1B0 C3C5 CEDA D49F 68E6 8DF2 B2F2 8859"
	$"A038 A94E 221E 0B0D DA6B E6C9 A58D 8789"
	$"8FC9 DC58 5F69 7C8C 9FA1 AAB0 C3C4 C5C8"
	$"CEDA E4EE E32C 2D2F 3335 3839 3532 EDE4"
	$"C4AA A09F A0A4 C3CE D4D4 C5F2 F2F6 0000"
	$"8F02 0000 F2F5 00FE FFE2 0000 FF81 00F7"
	$"0000 FFF3 0001 F2F2 FE00 72F2 8CA1 C3E4"
	$"2C32 3737 3535 3738 8139 372F EECE C3FC"
	$"64B0 5B1A 9642 453B 173C 24F9 9A61 39ED"
	$"CEC2 C5CE C8C5 C4A4 6A57 ABF1 DDDD A6E5"
	$"9F33 FA0E 7841 2B21 162F 8CE5 CAA6 8E88"
	$"898F B1D5 EF5E 657A 80FC A0AA B0C3 C4C5"
	$"C5C8 CEDA FBD9 ED2D 2F31 3338 3A81 3832"
	$"2CE4 C5AA 9F9E FCA4 C3DA EDE4 F2F2 F600"
	$"0089 0200 00F2 D300 00FF 8100 F700 00FF"
	$"F300 01F2 F2FE 0006 F234 322F 3132 34FC"
	$"3800 39FE 815A 3834 2EEE C38C 9FCE 5256"
	$"9244 5323 934A 1304 FA3E 31EE D4FB EEDA"
	$"C8C2 A06A 59D5 A7FD E669 C33F 0418 4EA8"
	$"7747 3B1A 328C DBFD A78F 898D A5C9 DC57"
	$"AC68 7A80 FCA0 A4AA B0C3 C4C5 C8CE DAE4"
	$"EEE3 2C2E 2F32 3539 3A3D 3833 EDDA C5B0"
	$"A4FE A004 C2CE D4F2 F2F6 0000 8B02 0000"
	$"F2D3 0000 FF81 00F7 0000 FFF3 0001 F2F2"
	$"FE00 07F2 FA55 3E81 3737 38FE 3967 3839"
	$"393A 3D3E 3F39 32D9 C59E 69C4 7630 2267"
	$"430B 7224 0C99 5238 2EE3 2C2C FBC8 B09E"
	$"6BAC 59EF 6386 E351 0A54 2894 43B6 7741"
	$"1B33 8CF0 FDA7 8F8D 8EA5 C9DC 575F 687B"
	$"80FC 9FA1 A4AA C2C3 C4C8 CED4 E4FB D9ED"
	$"2E2F 3133 373A 3F3E 8132 E3DA C4C2 A49F"
	$"8C9F AAC5 F2F2 F600 0087 0200 00F2 D300"
	$"00FF 8100 F700 00FF F300 01F2 F2FE 0006"
	$"F23E 8135 3534 35FA 385E 3981 3A3E 3D3A"
	$"352D D4A0 AAEE 7549 0543 4193 4E14 0279"
	$"5037 2E2C D9DA C5C2 A18C 7D7D 8CCE 3274"
	$"0949 F71D 3B8A BB53 1112 2E80 DBFD A68F"
	$"8D8E A5C9 DC57 AC65 7A80 FCA0 A1AA B0C2"
	$"C3C3 C4C8 CEDA E4EE E32C 2E31 3234 393D"
	$"3F40 3932 D9C8 C3AA A4FE A102 B0F2 F2F6"
	$"0000 8D02 0000 F2D3 0000 FFD3 00D3 FFD3"
	$"0000 FFF3 0001 F2F2 FE00 00F2 FE2F 6631"
	$"3234 3737 3537 3837 3738 3981 3A3F 5140"
	$"8131 E4AA 7BD4 994E 2247 1728 250A A976"
	$"3E32 2CEE D4C5 C3B0 B0C2 CEED 3E97 0F49"
	$"913B 4D8B B6BD 4384 07EE 7CDB CAA6 8F8D"
	$"8FA6 C9DC EF5E 657A 80FC A0A4 AAB0 C2C2"
	$"C3C3 C4C8 D4DA FBD9 ED2E 3131 3235 8140"
	$"513F 392E DAC5 FEB0 04A4 A0A0 F2F2 F600"
	$"008D 0200 00F2 D300 00FF D300 D3FF D300"
	$"00FF F300 01F2 F2FE 0006 F22C 2D2E 2F31"
	$"32FE 3400 35FE 3704 3839 813A 3EFE 405C"
	$"3A35 2CC8 D42E FA30 920B 9C72 25F9 7F50"
	$"3731 2CE3 D9FB FBD9 2C35 3F98 0D30 4C1D"
	$"538B B6B7 B81D 4AFA AA65 F0CA ABA6 8FA5"
	$"ABCA E657 AC65 7A80 FC9F A1AA B0C2 C2C3"
	$"C3C4 C8CE D4E4 FBD9 2C2E 2F32 3337 3D4F"
	$"4F3F 35ED D4C2 AAA1 A4AA A4F2 F2F6 0000"
	$"8D02 0000 F2D3 0000 FFD3 00D3 FFD3 0000"
	$"FFF3 0001 F2F2 FE00 72F2 D9ED 2D2E 2E2F"
	$"3233 3435 3737 3838 3981 813A 3D40 5252"
	$"513D 34DA 86C5 7925 931E 93F8 149D 613D"
	$"3732 312F 2E2F 353A 5597 0C27 7E05 428B"
	$"B5BA B58B 9012 3A7B 5DE6 CBC9 ABA6 A7B1"
	$"CBF0 585F 657A 7D8C 9FA1 A4AA B0C2 C3C3"
	$"C4C5 C8CE DAE4 EEED 2D2F 3131 3339 3F51"
	$"523A 32EE C4AA A0A4 B0B0 F2F2 F600 00BA"
	$"0200 00F2 F500 FDFF FE00 FEFF E900 00FF"
	$"F900 FEFF FA00 00FF FC00 01FF FFED 00F9"
	$"FFFE 00FA FF00 00FE FFFC 00EE FFF9 00FE"
	$"FFFA 0000 FFFC 0001 FFFF ED00 00FF F300"
	$"01F2 F2FE 0001 F22E FE2F 072E 2F31 3232"
	$"3334 35FE 375C 3839 813A 3E40 5051 503E"
	$"35ED ED33 610E 496F 7249 1807 794F 3732"
	$"2E2E 3138 4075 A912 4828 0529 6671 7741"
	$"0B24 73EE 685C E6CB C9AB A7AB C9CF DB59"
	$"5F68 7B80 FC9F A1A4 AAB0 C2C3 C3C4 C5C8"
	$"CED4 DAFB D9ED 2E2F 2F31 353A 4051 3D35"
	$"2CDA C3FD A401 F2F2 F600 00CF 0200 00F2"
	$"F500 00FF FC00 00FF FE00 00FF EA00 00FF"
	$"FA00 00FF FE00 00FF FC00 01FF FFFD 0000"
	$"FFEB 00FA FF00 00FE FF00 00FC FF01 0000"
	$"FEFF 0000 FEFF 0000 EEFF FA00 00FF FE00"
	$"00FF FC00 01FF FFFD 0000 FFEB 0000 FFF3"
	$"0001 F2F2 FE00 03F2 3332 31FD 2F0F 3131"
	$"3232 3334 3535 3738 3939 813A 3F52 FE55"
	$"5751 35C2 C381 A926 7278 6C1B 08FA 3E35"
	$"2F2F 333A 5597 0718 4E4C A821 4341 2992"
	$"4E9D D99E 6258 F0CF FDB1 A7B1 FDD5 E55C"
	$"6269 7B80 FCA0 A4AA B0B0 C2C2 C3C3 C4C5"
	$"C8CE D4DA FBD9 2C2E 2E2F 3337 3E50 3E39"
	$"31D9 CEAA A19F A1F2 F2F6 0000 C402 0000"
	$"F2F5 00FE FFFA 0000 FFEA 0000 FFFA 0000"
	$"FFFE 0000 FFFB 0000 FFFE 0000 FFEA 00FA"
	$"FF00 00FE FF00 00FB FF00 00FA FF00 00EE"
	$"FFFA 0000 FFFE 0000 FFFB 0000 FFFE 0000"
	$"FFEA 0000 FFF3 0001 F2F2 FE00 00F2 FD2D"
	$"0D2C 2D2D 2E2E 2F31 3232 3334 3537 38FE"
	$"393E 3D4F 5155 5551 3E35 3339 5097 0712"
	$"1916 1208 9D79 7355 5273 79A9 0918 486F"
	$"90F7 9C92 6027 083E B07A 5D58 EFE6 CBFD"
	$"B1C9 CADC EF5D 6369 7B86 FCA0 A1AA B0B0"
	$"C2FE C31A C4C5 C8CE D4DA E4EE E32C 2C2D"
	$"2F33 813F 3A38 2FD9 D4C2 AAA4 A4F2 F2F6"
	$"0000 C502 0000 F2F2 0000 FFFC 0000 FFE9"
	$"0000 FFFA 0000 FFFE 0000 FFFB 0000 FFFE"
	$"00FD FFED 00FA FF00 00FE FF00 00FB FF00"
	$"00FB FF00 00ED FFFA 0000 FFFE 0000 FFFB"
	$"0000 FFFE 00FD FFED 0000 FFF3 0001 F2F2"
	$"FE00 02F2 D9D9 FEE3 00ED FE2C 332D 2F2F"
	$"3132 3334 3435 3838 3739 3D4F 5255 515B"
	$"7661 382E 2D37 7504 121A 1814 0D07 A999"
	$"9AA3 070E 1827 5478 726E 4A24 023F E47C"
	$"63FE 5813 E5D5 CAC9 FDCF F057 5E64 6A7C"
	$"86FC A0A1 AAB0 C2C2 FEC3 1AC4 C5C8 C8CE"
	$"DAE4 FBEE D9E3 ED2D 2F37 3A39 372F D9D4"
	$"C4C3 B0AA F2F2 F600 00D7 0200 00F2 F100"
	$"00FF FE00 00FF E800 02FF 0000 FDFF 0100"
	$"FFFE 0000 FFFB 0000 FFFE 0000 FFFE 0000"
	$"FFEE 0001 FFFF FD00 01FF 00FE FF00 00FB"
	$"FF00 00FB FF00 00ED FF01 0000 FDFF 0100"
	$"FFFE 0000 FFFB 0000 FFFE 0000 FFFE 0000"
	$"FFEE 0000 FFF3 0001 F2F2 FE00 02F2 FBFB"
	$"FDEE 4DD9 D9E3 ED2C 2D2E 2F2F 3133 3334"
	$"3435 3739 3A3E 4F51 5573 5B3F 8139 3D50"
	$"737F 9AA3 0101 A9A9 0102 0309 0F14 1A24"
	$"261A 1308 9A3D DA9E 6463 625E 58F0 CFCA"
	$"C9CA D5DB 595F 656B 7D8C 9EA0 A4AA B0C2"
	$"C2FE C31A C4C5 C5C8 CED4 DAE4 FBEE EED9"
	$"E32D 3237 3432 2DEE CEC3 C2AA AAF2 F2F6"
	$"0000 C502 0000 F2F1 0003 FF00 00FF E700"
	$"00FF FA00 00FF FE00 00FF FB00 00FF FE00"
	$"00FF FE00 00FF EE00 FAFF 0000 FEFF 0000"
	$"FBFF 0000 FCFF 0000 ECFF FA00 00FF FE00"
	$"00FF FB00 00FF FE00 00FF FE00 00FF EE00"
	$"00FF F300 01F2 F2FE 0000 F2FB DA09 E4FB"
	$"EEEE D9E3 2C2C 2D2E FE31 0732 3334 3535"
	$"373A 50FC 5533 5250 8134 3438 405B 7698"
	$"A906 0708 0C0F 1212 0F0A 0179 5034 C46B"
	$"635C 626A 6258 E6CA FDFD CBDC EF5C 6269"
	$"7A80 FC9F A1A4 AAB0 C2C2 FEC3 06C4 C5C5"
	$"C8CE D4DA FCE4 0EEE E32E 3231 2EE3 FBC8"
	$"C2B0 AAB0 F2F2 F600 00CD 0200 00F2 F100"
	$"02FF 00FF FE00 00FF EA00 00FF FA00 00FF"
	$"FE00 00FF FB00 00FF FE00 00FF FE00 00FF"
	$"EE00 FAFF 0000 FEFF 0000 FBFF 0000 FCFF"
	$"0000 ECFF FA00 00FF FE00 00FF FB00 00FF"
	$"FE00 00FF FE00 00FF EE00 00FF F300 01F2"
	$"F2FE 0005 F2DA DAE4 DADA FEE4 FEFB 06D9"
	$"E3ED ED2C 2D2E FE2F 2331 3132 3437 813D"
	$"3E40 5155 5B52 403D 3D3E 4F51 5252 5B73"
	$"7475 76FA 79FA 6150 3D37 2CCE A0FE 7A04"
	$"6A68 5EE5 DCFE FD0E CFE6 575E 636A 7B86"
	$"FCA0 A1AA B0B0 C2FD C31A C4C5 C5C8 CED4"
	$"DAE4 DADA D4D4 E4FB ED2D 2CE3 FBD4 C5C2"
	$"B0AA B0F2 F2F6 0000 BC02 0000 F2F2 0002"
	$"FF00 00FC FFEA 0000 FFF9 00FE FFFE 0002"
	$"FF00 00FE FFFE 00FE FFED 00F9 FFFE 00FE"
	$"FF02 00FF FFFE 00FD FF00 00EC FFF9 00FE"
	$"FFFE 0002 FF00 00FE FFFE 00FE FFED 0000"
	$"FFF3 0001 F2F2 FE00 00F2 F6E4 08FB EED9"
	$"E3E3 ED2C 2E2E FE2C 292E 3233 3334 3581"
	$"505B 615B 5552 5151 504F 3A37 3534 3535"
	$"3332 312F EEC4 C4C3 A086 7D7C FCAA 8064"
	$"5CF0 CFFE FD0E D5DB 58AC 656B 7C86 9EA0"
	$"A4AA B0B0 C2FD C31A C4C5 C8CE D4DA E4E4"
	$"DAD4 CEC8 CEDA EEE3 EEE4 D4C5 C3C2 B0AA"
	$"B0F2 F2F6 0000 8A02 0000 F2F5 00FE FFE2"
	$"0000 FFD3 00D3 FFD3 0000 FFF3 0001 F2F2"
	$"FE00 00F2 FBCE FCD4 08DA FBFB EEEE D9ED"
	$"2CED FEE3 0AED 2C2D 2C2D 2F33 383A 3D3E"
	$"FD3F 2C3E 3E3A 3938 3738 3935 3231 2EE3"
	$"E4E4 DAC4 A1A0 9F8C 7D65 59F0 CAC9 ABC9"
	$"CAE6 EF5D 6369 7B80 FC9F A1A4 AAB0 C2C2"
	$"FEC3 1BC4 C4C5 C8CE D4DA E4E4 DAD4 CEC8"
	$"C8CE DAFB DAD4 C8C4 C2B0 B0AA B0F2 F2F6"
	$"0000 8902 0000 F2D3 0000 FFD3 00D3 FFD3"
	$"0000 FFF3 0001 F2F2 FE00 00F2 FCC4 01C5"
	$"C5FD C805 D4DA E4E4 FBEE FED9 FEEE 54FB"
	$"EEFB FBEE E32C 2D2E 3234 3738 8181 3939"
	$"3A3D 8139 3A3D 3834 2F2C 2C2D 2C2C FBC8"
	$"C4C3 8062 58DC FDA6 A5A5 B1CB F059 AC65"
	$"6B7C 869E A0A4 AAB0 B0C2 C2C3 C3C4 C4C5"
	$"C5C8 CEDA E4FB FBE4 DACE C8C5 C5CE D4CE"
	$"C5C4 C2B0 FEAA 02B0 F2F2 F600 008A 0200"
	$"00F2 D300 00FF D300 D3FF D300 00FF F300"
	$"01F2 F2FE 0000 F2FC C401 C5C5 FDC8 0BCE"
	$"D4DA DAE4 FBFB EEEE FBE4 E4FE DA37 D4D4"
	$"DAE4 E4FB D9E3 ED2C 2D2D 2C2C 2D2E 2D2C"
	$"E3D9 EEFB D4CE D4DA C5B0 A0FC 7C69 5EF0"
	$"CBAB A68F A5AB CADC EF5E 6369 7B80 FC9F"
	$"A1A4 AAB0 C2C2 FEC3 1CC4 C4C5 C5C8 CEDA"
	$"FBFB EEFB E4D4 C8C8 C5C8 CEC8 C5C4 C3C2"
	$"C2B0 B0C2 F2F2 F600 0083 0200 00F2 D300"
	$"00FF D300 D3FF D300 00FF F300 01F2 F2FE"
	$"0000 F2F6 C50F C8CE D4D4 DADA E4FB FBE4"
	$"DAD4 CECE C8C5 FDC4 08C5 C5C8 C5C5 C8C8"
	$"C5C4 FEC5 23C3 A49E A0A1 A0A0 A4AA 8669"
	$"6564 58D5 FDA6 8E88 898D A6C9 D5E5 5C62"
	$"687A 7D8C 9EA0 A4AA B0FE C2FE C307 C4C5"
	$"C5C8 C8CE DAFB FEEE 0CE4 DACE C8C5 C5C8"
	$"C5C5 C4C4 C3C3 FEC2 01F2 F2F6 0000 7D02"
	$"0000 F2D3 0000 FF81 00F7 0000 FFF3 0001"
	$"F2F2 FE00 00F2 FCC3 FBC4 10C5 C8CE CED4"
	$"DADA E4E4 DAD4 CEC8 C8C5 C3C2 FBB0 FCAA"
	$"01A4 A1FE A022 FC86 8080 7D7B 6A65 5F5C"
	$"57E5 CFB1 A68F 8E8E A5AB CADC EF5C 5F65"
	$"6B7C 869E 9FA1 A4AA B0FE C213 C3C3 C4C4"
	$"C5C5 C8C8 CEDA FBEE D9EE FBDA D4C8 C5C5"
	$"FCC8 06C5 C5C4 C3C3 F2F2 F600 0087 0200"
	$"00F2 D300 00FF 8100 F700 00FF F300 01F2"
	$"F2FE 0000 F2FD B0FE C2FD C327 C4C5 C5C8"
	$"CED4 D4DA DAD4 CECE C8C4 C3C2 B0A4 A4A1"
	$"A1A0 9F9E 9EFC FC9E 9E86 7D80 807D 7D7A"
	$"6A65 6257 FED5 18FD ABA6 A58E 8D8F A6C9"
	$"CFF0 585E 6368 6B7C 86FC 9FA1 A4AA B0B0"
	$"FEC2 1FC3 C3C4 C4C5 C8C8 CED4 DAFB EED9"
	$"EEEE E4D4 CEC5 C8C8 CED4 D4DA D4C8 C5C4"
	$"C4F2 F2F6 0000 8602 0000 F2D3 0000 FF81"
	$"00F7 0000 FFF3 0001 F2F2 FE00 00F2 FDAA"
	$"FDB0 FDC2 18C3 C4C5 C8CE CED4 D4CE C8C8"
	$"C5C3 C2AA A4A1 A09F 9F9E FC8C 8C86 FE80"
	$"267D 7B7B 7A7A 6B69 6564 625C EFEF E5E6"
	$"CFCB CAFD FDCB DCF0 575C AC63 686B 7C80"
	$"8C9E A0A1 A4AA B0B0 FEC2 0BC3 C3C4 C4C5"
	$"C8C8 CED4 DAFB FBFE EE10 E4DA CEC8 C8CE"
	$"DAE4 E4FB DAD4 C8C5 C4F2 F2F6 0000 AE02"
	$"0000 F2F5 00FD FFFE 00FE FFE9 0000 FFF9"
	$"00FE FFFA 0000 FFFE 00FC FFE6 00FE FFFA"
	$"0000 FFFE 00FC FFE6 00FE FFFA 0000 FFFE"
	$"00FC FFEE 0000 FFF3 0001 F2F2 FE00 00F2"
	$"FEA4 FCAA FDB0 1AC2 C3C4 C5C8 C8CE C8C8"
	$"C5C4 C3C2 AAA4 A1A0 9EFC FC8C 8C86 807D"
	$"7C7B FE7A 236A 6968 6564 6463 6263 645F"
	$"5D59 58EF E5EF 5759 5DAC 6263 6569 6B7C"
	$"808C 9E9F A1A4 AAAA B0FC C20A C3C4 C4C5"
	$"C8CE CED4 DAE4 FBFE EE10 E4DA CEC8 CEDA"
	$"FBEE EED9 FBE4 D4C8 C5F2 F2F6 0000 CB02"
	$"0000 F2F5 0000 FFFC 0000 FFFE 0000 FFEA"
	$"0000 FFFA 0000 FFFE 0000 FFFC 0001 FFFF"
	$"FE00 00FF FE00 00FF E700 00FF FE00 00FF"
	$"FC00 01FF FFFE 0000 FFFE 0000 FFE7 0000"
	$"FFFE 0000 FFFC 0001 FFFF FE00 00FF FE00"
	$"00FF EE00 00FF F300 01F2 F2FE 0000 F2F9"
	$"A4FD AA04 B0C2 C3C4 C5FE C822 C5C4 C4C2"
	$"B0AA A1A0 9EFC 8C8C 8680 807D 7C7B 7A6B"
	$"6B6A 6968 6564 6362 5F5E ACAC 5D59 59FE"
	$"5811 595C 5EAC 6264 6569 6B7B 7D86 FC9F"
	$"A0A1 A4AA FEB0 FDC2 0AC3 C4C4 C5C8 CECE"
	$"D4DA DAE4 FEFB 06E4 D4C8 C8D4 E4EE FEE3"
	$"06EE FBDA CEC5 F2F2 F600 00B7 0200 00F2"
	$"F500 FEFF FA00 00FF EA00 00FF FA00 00FF"
	$"FE00 00FF FB00 00FF FA00 00FF E700 00FF"
	$"FE00 00FF FB00 00FF FA00 00FF E700 00FF"
	$"FE00 00FF FB00 00FF FA00 00FF EE00 00FF"
	$"F300 01F2 F2FE 0000 F2FA A1FD A404 AAAA"
	$"B0C2 C4FC C51D C4C3 C2AA A4A0 9FFC 8C8C"
	$"8680 7D7C 7C7B 7A6B 6A69 6968 6564 6362"
	$"AC5D 5958 FDEF 1457 585C 5D5D AC5F 6364"
	$"686A 7A7D 808C 9E9F A0A1 A4AA FDB0 07C2"
	$"C2C3 C3C4 C5C5 C8FE CE0C D4DA E4E4 FBFB"
	$"DACE C8C8 D4FB D9FE ED06 E3EE E4CE C8F2"
	$"F2F6 0000 BB02 0000 F2F2 0000 FFFD 0001"
	$"FFFF E900 00FF FA00 00FF FE00 00FF FB00"
	$"00FF FB00 00FF E600 00FF FE00 00FF FB00"
	$"00FF FB00 00FF E600 00FF FE00 00FF FB00"
	$"00FF FB00 00FF ED00 00FF F300 01F2 F2FE"
	$"0000 F2FB A0FC A105 A4AA B0C2 C3C4 FEC5"
	$"1EC4 C3C2 B0AA A1A0 9EFC 8C86 8080 7D7C"
	$"7B7B 7A6B 6A69 6868 6564 6362 AC5E 5C59"
	$"FE58 1559 595C 5D5E AC62 6365 686A 7A7C"
	$"808C FC9F A0A1 A4A4 AAFD B011 C2C2 C3C3"
	$"C4C5 C5C8 C8CE CED4 DADA E4E4 DAD4 FEC8"
	$"02DA EEE3 FE2C 06E3 EEE4 D4C8 F2F2 F600"
	$"00BF 0200 00F2 F100 00FF FC00 00FF EA00"
	$"02FF 0000 FDFF 0100 FFFE 0000 FFFB 0000"
	$"FFFB 0000 FFEB 00FD FF01 00FF FE00 00FF"
	$"FB00 00FF FB00 00FF EB00 FDFF 0100 FFFE"
	$"0000 FFFB 0000 FFFB 0000 FFED 0000 FFF3"
	$"0001 F2F2 FE00 00F2 FC9F FBA0 26A1 A4AA"
	$"B0C2 C4C4 C5C4 C4C3 C2B0 A4A1 9F9E 8C86"
	$"8680 807D 7C7B 7A6B 6B6A 6968 6565 6463"
	$"625F ACAC FA5E 12AC 5F62 6465 696B 7B7C"
	$"8086 FC9E 9FA0 A1A4 AAAA FDB0 02C2 C2C3"
	$"FEC4 00C5 FEC8 02CE CED4 FEDA 07D4 CEC8"
	$"C8CE E4D9 EDFE 2D06 EDD9 E4D4 C8F2 F2F6"
	$"0000 B402 0000 F2F1 0000 FFFC 0000 FFEA"
	$"0000 FFFA 0000 FFFE 0000 FFFB 0000 FFFC"
	$"0000 FFE5 0000 FFFE 0000 FFFB 0000 FFFC"
	$"0000 FFE5 0000 FFFE 0000 FFFB 0000 FFFC"
	$"0000 FFEC 0000 FFF3 0001 F2F2 FE00 02F2"
	$"9E9E FA9F 07A0 A0A1 A1AA B0C2 C3FD C41A"
	$"C3C2 AAA4 A09F 9E8C 8686 807D 7D7C 7B7A"
	$"7A6B 6A69 6968 6565 6463 63F9 6212 6363"
	$"6568 696A 7A7B 7D80 8CFC 9E9F A0A1 A4AA"
	$"AAFD B0FE C204 C3C3 C4C4 C5FD C816 CED4"
	$"DADA D4CE C8C8 CED4 EEED 2C2E 2E2D EDD9"
	$"E4D4 C8F2 F2F6 0000 B302 0000 F2F1 0000"
	$"FFFC 0000 FFEA 0000 FFFA 0000 FFFE 0000"
	$"FFFB 0000 FFFC 0000 FFE5 0000 FFFE 0000"
	$"FFFB 0000 FFFC 0000 FFE5 0000 FFFE 0000"
	$"FFFB 0000 FFFC 0000 FFEC 0000 FFF3 0001"
	$"F2F2 FE00 00F2 FC9E FC9F 06A0 A0A1 A4B0"
	$"C2C3 FDC4 19C3 C2AA A4A0 9F9E 8C86 8680"
	$"7D7D 7C7B 7A7A 6B6A 6A69 6968 6865 65FA"
	$"6414 6568 696A 6B7A 7B7C 7D80 868C 9E9F"
	$"A0A1 A1A4 AAB0 B0FA C205 C3C3 C4C4 C5C5"
	$"FEC8 00CE FED4 12CE CEC8 C8D4 E4D9 2C2E"
	$"2F2E 2EED D9E4 CEC8 F2F2 F600 00B3 0200"
	$"00F2 F200 00FF FC00 00FF E900 00FF F900"
	$"FEFF FE00 02FF 0000 FEFF FD00 00FF E400"
	$"FEFF FE00 02FF 0000 FEFF FD00 00FF E400"
	$"FEFF FE00 02FF 0000 FEFF FD00 00FF EC00"
	$"00FF F300 01F2 F2FE 0000 F2FE FCFE 9EFC"
	$"9F05 A0A1 A4AA C2C3 FDC4 15C3 B0AA A4A0"
	$"9FFC 8C86 8680 7D7D 7C7B 7B7A 7A6B 6B6A"
	$"6AFE 69FB 6816 6969 6A6B 7A7B 7B7C 7D80"
	$"868C FC9E 9FA0 A1A4 A4AA AAB0 B0F9 C204"
	$"C3C4 C4C5 C5FE C800 CEFE D412 CEC8 CECE"
	$"DAEE ED2E 2F31 2F2E EDD9 E4CE C8F2 F2F6"
	$"0000 8302 0000 F2F5 00FE FFFD 0001 FFFF"
	$"E800 00FF 8100 F700 00FF F300 01F2 F2FE"
	$"0000 F2FD FCFD 9EFE 9F05 A0A0 A4AA C2C3"
	$"FDC4 10C3 B0AA A1A0 9EFC 8C86 8680 7D7D"
	$"7C7C 7B7B FD7A FE6B FC6A FE6B 127A 7B7B"
	$"7C7D 8086 868C FC9E 9FA0 A0A1 A4A4 AAAA"
	$"F8B0 20C2 C2C3 C3C4 C4C5 C8C8 CECE D4CE"
	$"CEC8 C8CE DAFB E32D 3132 3231 2EED D9E4"
	$"CEC8 F2F2 F600 0073 0200 00F2 D300 00FF"
	$"8100 F700 00FF F300 01F2 F2FE 0000 F2FD"
	$"FCFD 9EFE 9F17 A0A1 A4B0 C2C3 C4C5 C5C4"
	$"C3C2 AAA4 A09F FC8C 8686 8080 7D7D FE7C"
	$"FC7B FB7A FE7B 107C 7C7D 8080 868C 8CFC"
	$"9E9F A0A0 A1A1 A4A4 FEAA F7B0 07C2 C3C3"
	$"C4C5 C5C8 C8FA CE10 DAFB D92C 2F32 3333"
	$"312E EDEE DAC8 C5F2 F2F6 0000 7502 0000"
	$"F2D3 0000 FF81 00F7 0000 FFF3 0001 F2F2"
	$"FE00 00F2 FDFC FE9E FD9F 05A0 A1A4 B0C2"
	$"C4FE C509 C4C3 C2B0 A4A1 9F9E 8C8C FE86"
	$"0180 80FD 7DF9 7CFD 7D0B 8080 8686 8C8C"
	$"FCFC 9E9E 9FA0 FEA1 01A4 A4FD AAFD B0FD"
	$"AA07 B0B0 C2C3 C3C4 C5C8 FACE 12D4 DAFB"
	$"D9ED 2E32 3433 3331 2DE3 FBDA C8C5 F2F2"
	$"F600 0072 0200 00F2 D300 00FF 8100 F700"
	$"00FF F300 01F2 F2FE 0000 F2FD FCFE 9EFD"
	$"9F05 A0A1 AAB0 C3C4 FDC5 09C4 C2B0 AAA1"
	$"A09E FC8C 8CFE 86F2 80FE 860B 8C8C FCFC"
	$"9E9E 9F9F A0A0 A1A1 FEA4 FBAA 01B0 B0FC"
	$"AA0A B0B0 C2C3 C4C4 C5C8 CEC8 C8FE CE13"
	$"D4DA E4EE ED2D 3133 3434 322F 2CD9 E4D4"
	$"C5C5 F2F2 F600 0069 0200 00F2 D300 00FF"
	$"8100 F700 F2FF 01F2 F2FE 0000 F2FE FCFD"
	$"9EFD 9F05 A0A1 AAB0 C3C4 FDC5 08C4 C3B0"
	$"AAA1 A09F FCFC FE8C F486 FD8C FEFC FE9E"
	$"039F 9FA0 A0FE A1FE A4F4 AA09 B0B0 C2C2"
	$"C3C4 C5C8 CECE FEC8 15CE D4DA FBEE ED2D"
	$"2F33 3435 3332 2EED EEDA C8C4 C4F2 F2F6"
	$"0000 6902 0000 F2D3 0000 FF81 00F7 0000"
	$"FFF3 0001 F2F2 FE00 01F2 FCFC 9EFD 9F12"
	$"A0A1 A4AA B0C3 C4C5 C8C8 C5C4 C3C2 AAA4"
	$"A19F 9EFE FCF5 8CFB FCFE 9EFE 9F01 A0A0"
	$"FDA1 FEA4 F3AA 22B0 B0C2 C3C3 C4C5 C8C8"
	$"CEC8 C8CE D4DA FBEE E32D 2F32 3435 3533"
	$"2F2C D9E4 CEC5 C4C4 F2F2 F600 0069 0200"
	$"00F2 D300 00FF 8100 F700 00FF F300 01F2"
	$"F2FE 0000 F2FD 9EFC 9F15 A0A0 A1A4 AAC2"
	$"C3C4 C5C8 C8C5 C4C3 C2B0 AAA1 A09F 9E9E"
	$"FEFC FD8C FDFC F99E FE9F FEA0 FDA1 FEA4"
	$"F1AA 06B0 B0C2 C3C4 C5C5 FDC8 09CE D4E4"
	$"FBEE E32C 2F32 34FE 350A 322E EDFB DAC8"
	$"C5C3 C4F2 F2F6 0000 9902 0000 F2F5 00FD"
	$"FFFC 0000 FFE9 0000 FFF9 00FE FFFA 0000"
	$"FFFB 0000 FFE5 00FE FFFA 0000 FFFC 0001"
	$"FFFF E500 FEFF FA00 00FF FE00 FCFF EE00"
	$"00FF FE00 FAFF FD00 01F2 F2FE 0000 F2FE"
	$"9EFD 9FFE A006 A1A4 A4B0 C2C3 C5FD C809"
	$"C5C4 C3B0 AAA4 A1A0 A09F FE9E FBFC FD9E"
	$"FB9F FCA0 FEA1 FCA4 FDAA F6A4 05AA AAB0"
	$"C2C3 C4FD C519 C8CE D4E4 FBD9 E32C 2F32"
	$"3435 3735 3431 2CD9 E4CE C5C4 C3C4 F2F2"
	$"F600 00AA 0200 00F2 F500 00FF FA00 00FF"
	$"E800 00FF FA00 00FF FE00 00FF FC00 01FF"
	$"FFFC 0000 FFE5 0000 FFFE 0000 FFFC 0001"
	$"FFFF FD00 00FF E400 00FF FE00 00FF FC00"
	$"01FF FFFE 0000 FFFE 0000 FFEE 0000 FFFE"
	$"0000 FFFC 0000 FFFD 0001 F2F2 FE00 00F2"
	$"FD9F FCA0 07A1 A1A4 AAB0 C2C4 C5FD C80C"
	$"C5C4 C3C2 B0AA A4A1 A1A0 A09F 9FF7 9EFD"
	$"9FFD A0FC A1EC A404 AAAA B0C2 C3FE C41B"
	$"C5C8 CED4 E4FB D9E3 2C2E 3234 3537 3735"
	$"332E EDFB D4C8 C4C4 C3C5 F2F2 F600 00AE"
	$"0200 00F2 F500 FEFF FD00 02FF 00FF E900"
	$"00FF FA00 00FF FE00 00FF FB00 00FF FD00"
	$"02FF 00FF E600 00FF FE00 00FF FB00 00FF"
	$"FE00 00FF E300 00FF FE00 00FF FB00 00FF"
	$"FA00 00FF EE00 00FF FE00 00FF FC00 00FF"
	$"FD00 01F2 F2FE 0001 F29F FBA0 FEA1 07A4"
	$"A4AA C2C3 C4C5 C8FE CE0B C8C5 C4C2 B0AA"
	$"AAA4 A1A1 A0A0 FC9F 019E 9EFB 9FFC A0FB"
	$"A1F7 A4FA A1FD A404 AAB0 C2C3 C3FE C40B"
	$"C5CE D4E4 EED9 ED2C 2E32 3435 FE37 0C34"
	$"312C EEE4 CEC5 C4C3 C3C5 F2F2 F600 00AC"
	$"0200 00F2 F200 06FF 0000 FF00 00FF E900"
	$"00FF FA00 00FF FE00 00FF FB00 00FF FE00"
	$"03FF 0000 FFE6 0000 FFFE 0000 FFFB 0000"
	$"FFFE 00FD FFE6 0000 FFFE 0000 FFFB 0000"
	$"FFFB 0000 FFED 0000 FFFE 0000 FFFC 0000"
	$"FFFD 0001 F2F2 FE00 00F2 FDA0 FCA1 08A4"
	$"A4AA B0C2 C3C4 C5C8 FECE 0AC8 C5C4 C3C2"
	$"B0AA A4A4 A1A1 FCA0 FE9F F5A0 F6A1 00A4"
	$"F6A1 26A4 A4AA AAB0 C2C2 C3C4 C4C8 CEDA"
	$"FBEE D9ED 2D2F 3134 3537 3837 3532 2EE3"
	$"E4D4 C5C4 C3C3 C4C8 F2F2 F600 00B5 0200"
	$"00F2 F100 01FF 00FC FFEA 0002 FF00 00FD"
	$"FF01 00FF FE00 00FF FB00 00FF FE00 FCFF"
	$"EC00 FDFF 0100 FFFE 0000 FFFB 0000 FFFE"
	$"0000 FFFE 0000 FFEC 00FD FF01 00FF FE00"
	$"00FF FB00 00FF FB00 00FF ED00 FCFF FC00"
	$"FDFF 0200 F2F2 FE00 00F2 F9A1 09A4 A4AA"
	$"B0B0 C3C4 C5C8 C8FE CE0A C8C8 C5C3 C3C2"
	$"B0AA AAA4 A4F0 A103 A0A0 A1A1 FEA0 FDA1"
	$"01A0 A0FD A1FB A0FE A114 A4A4 AAB0 C2C2"
	$"C3C4 C4C8 CEDA FBEE E3ED 2D2F 3133 35FE"
	$"380E 3734 2FED EED4 C8C4 C3C3 C4C5 CEF2"
	$"F2F6 0000 AA02 0000 F2F1 0000 FFFD 0000"
	$"FFE9 0000 FFFA 0000 FFFE 0000 FFFB 0000"
	$"FFFB 0000 FFE6 0000 FFFE 0000 FFFB 0000"
	$"FFFE 0000 FFFE 0000 FFE7 0000 FFFE 0000"
	$"FFFB 0000 FFFC 0000 FFEC 0002 FF00 FFF8"
	$"0004 FF00 00F2 F2FE 0000 F2FA A4FE AA07"
	$"B0C2 C2C4 C5C8 C8CE FED4 08CE C8C8 C5C4"
	$"C3C2 B0B0 FEAA F2A4 FCA1 F9A0 009F FDA0"
	$"FD9F FEA0 FEA1 25A4 AAB0 C2C3 C4C5 CED4"
	$"E4FB D9E3 ED2D 2E31 3335 3839 3938 3532"
	$"2DD9 DAC8 C5C4 C3C3 C5CE DAF2 F2F6 0000"
	$"AC02 0000 F2F1 0000 FFFD 0000 FFE9 0000"
	$"FFFA 0000 FFFE 0000 FFFB 0000 FFFB 0000"
	$"FFE6 0000 FFFE 0000 FFFB 0000 FFFE 0000"
	$"FFFE 0000 FFE7 0000 FFFE 0000 FFFB 0000"
	$"FFFC 0000 FFEC 0003 FF00 00FF FA00 00FF"
	$"FE00 01F2 F2FE 0002 F2B0 B0FC AAFE B006"
	$"C2C3 C3C4 C5C8 CEFC D409 CEC8 C5C4 C4C3"
	$"C2C2 B0B0 F2AA FDA4 03A1 A1A0 A0F1 9F2A"
	$"A0A0 A1A1 A4AA B0C2 C3C4 C5CE DAE4 EED9"
	$"E3ED 2C2E 3133 3537 3981 3937 332F E3E4"
	$"CEC4 C4C3 C3C4 C8DA FBF2 F2F6 0000 AF02"
	$"0000 F2F2 0000 FFFD 00FE FFEA 0000 FFF9"
	$"00FE FFFE 0002 FF00 00FE FFFD 00FE FFE6"
	$"00FE FFFE 0002 FF00 00FE FFFE 00FE FFE5"
	$"00FE FFFE 0002 FF00 00FE FFFD 0000 FFEC"
	$"0000 FFFE 0000 FFFC 0000 FFFD 0001 F2F2"
	$"FE00 01F2 C2FB B0FE C208 C3C3 C4C5 C8CE"
	$"D4D4 DAFE D40A CEC8 C8C5 C4C3 C3C2 C2B0"
	$"B0F2 AAFE A403 A1A1 A0A0 FB9F FA9E FE9F"
	$"2AA0 A0A1 A4AA B0C2 C4C5 CED4 E4FB EED9"
	$"E3ED 2C2D 2F32 3437 3981 3A38 3431 2CFB"
	$"CEC5 C4C3 C3C4 C5D4 FBD9 F2F2 F600 0081"
	$"0200 00F2 F500 FEFF E200 00FF 8100 F700"
	$"00FF FD00 00FF FE00 00FF FC00 01F2 F2FE"
	$"0000 F2F9 C2FE C314 C4C5 C8C8 CED4 D4DA"
	$"DAD4 D4CE CEC8 C5C4 C4C3 C3C2 C2F7 B0FE"
	$"AA00 B0FD AA03 A4A4 A1A1 FEA0 FE9F FA9E"
	$"FE9F 2BA0 A0A1 A4AA B0C3 C4C8 D4DA FBEE"
	$"D9E3 E3ED 2C2D 2F32 3437 393A 3A81 3732"
	$"2DD9 DAC5 C4C3 C3C4 C5C8 E4D9 E3F2 F2F6"
	$"0000 7802 0000 F2D3 0000 FF81 00F7 0000"
	$"FFFC 0002 FF00 FFFB 0001 F2F2 FE00 01F2"
	$"C3FC C2FE C307 C4C4 C5C5 C8CE D4D4 FEDA"
	$"07D4 D4CE C8C8 C5C4 C4FE C3FD C2F4 B0FE"
	$"AA02 A4A1 A1FD A0F8 9F2E A0A0 A1A1 A4AA"
	$"B0C3 C4C8 CEDA E4FB EED9 E3E3 ED2C 2D2F"
	$"3234 3739 3A3D 3A39 332E E3E4 C8C4 C3C3"
	$"C4C4 C8D4 EEED 2CF2 F2F6 0000 7302 0000"
	$"F2D3 0000 FF81 00F7 0000 FFFB 0000 FFFA"
	$"0001 F2F2 FE00 00F2 F9C3 FEC4 04C5 C8CE"
	$"CED4 FDDA 06D4 D4CE CEC8 C5C5 FEC4 FEC3"
	$"FDC2 F5B0 05AA AAA4 A4A1 A1F6 A00E A1A1"
	$"A4AA AAB0 C3C4 C8CE DAE4 FBEE D9FE E31D"
	$"ED2C 2D2F 3234 3739 3A3D 3D39 352F E3E4"
	$"CEC4 C2C3 C4C4 C5D4 FBE3 2D2D F2F2 F600"
	$"0071 0200 00F2 D300 00FF 8100 F700 00FF"
	$"F300 01F2 F2FE 0000 F2F9 C4FE C510 C8C8"
	$"CED4 DADA E4E4 DADA D4D4 CEC8 C8C5 C5FC"
	$"C4FE C3F7 C2FE B003 AAAA A4A4 F6A1 0FA4"
	$"A4AA AAB0 C2C3 C4C8 CED4 E4FB EEEE D9FE"
	$"E31E ED2C 2D2E 3133 3739 3A3D 3D3A 3732"
	$"2CFB D4C5 C3C2 C4C5 C8D4 FBE3 2D2F 2FF2"
	$"F2F6 0000 6802 0000 F281 FFB9 FF01 F2F2"
	$"FE00 00F2 F8C5 06C8 C8CE CED4 DADA FDE4"
	$"06DA DAD4 CECE C8C8 FDC5 FEC4 FBC3 FAC2"
	$"FEB0 01AA AAF7 A40F AAAA B0B0 C2C3 C4C5"
	$"C8CE D4E4 FBEE EED9 FDE3 1FED 2C2D 2E31"
	$"3335 393A 3D3E 3D39 332D EED4 C5C3 C3C2"
	$"C4C8 D4E4 E32D 2F31 31F2 F2F6 0000 7902"
	$"0000 F2FB 0000 FFFA 0000 FF81 00E6 0000"
	$"FFFA 0000 FFFB 0000 FFF3 0001 F2F2 FE00"
	$"02F2 CECE FAC8 FECE 02D4 D4DA FBE4 05DA"
	$"D4D4 CEC8 C8FD C5FD C4F8 C3FD C2FD B0FB"
	$"AAFD B00E C2C2 C3C4 C4C5 C8CE D4DA E4FB"
	$"EED9 D9FC E307 ED2C 2E31 3335 383A FE3E"
	$"0681 352E D9DA C5C4 FEC3 0AC8 D4E4 D92C"
	$"3131 3232 F2F2 F600 0077 0200 00F2 FC00"
	$"01FF FFFA 0000 FF81 00E6 0000 FFFA 0001"
	$"FFFF FC00 00FF F300 01F2 F2FE 0001 F2D4"
	$"F7CE 0FD4 D4DA DAE4 E4FB FBE4 E4DA D4D4"
	$"CEC8 C8FD C5FC C4F7 C3F3 C20C C3C3 C4C4"
	$"C5C8 CECE D4DA E4FB EEFE D9FC E31A ED2C"
	$"2D2F 3234 3881 3E3F 3E3A 3731 E3E4 C8C4"
	$"C3C2 C3C4 CEFB D9ED 2FFE 3302 34F2 F2F6"
	$"0000 7C02 0000 F2FD 0002 FF00 FFFA 0000"
	$"FF81 00E6 0000 FFFA 0002 FF00 FFFD 0000"
	$"FFF3 0001 F2F2 FE00 00F2 FDD4 FDCE FCD4"
	$"01DA DAFC E406 DADA D4CE CEC8 C8FE C5FB"
	$"C4FD C3FD C4F4 C3FD C40B C5C5 C8CE D4D4"
	$"DAE4 E4FB EEEE FED9 FDE3 11ED 2C2D 2F32"
	$"3437 813D 3F3F 3A38 32ED E4C8 C4FE C30C"
	$"C5CE E4D9 2C2E 3235 3434 35F2 F2F6 0000"
	$"7702 0000 F2FE 0002 FF00 00FC FFFE 0000"
	$"FF81 00E6 0000 FFFE 00FC FF02 0000 FFFE"
	$"0000 FFF3 0001 F2F2 FE00 00F2 FEDA F7D4"
	$"01DA DAFC E405 DAD4 D4CE C8C8 FDC5 E7C4"
	$"FDC5 0BC8 C8CE D4D4 DADA E4FB FBEE EEFB"
	$"D923 E3E3 ED2D 2E31 3337 393D 3F3F 3E39"
	$"332C FBCE C4C3 C2C3 C4CE DAD9 2C2F 3234"
	$"3737 3538 F2F2 F600 007B 0500 00F2 0000"
	$"FFFA 0000 FFFE 0000 FF81 00E6 0000 FFFE"
	$"0000 FFFA 0003 FF00 00FF F300 01F2 F2FE"
	$"0000 F2FE DAF7 D4FE DAFE E406 DADA D4CE"
	$"CEC8 C8FE C5F4 C4FC C5FE C4FB C5FE C807"
	$"CECE D4D4 DADA E4E4 FEFB FEEE FCD9 11E3"
	$"ED2C 2E31 3335 393A 3E3F 3E81 342E EED4"
	$"C5FE C308 C5C8 E4D9 2C2F 3234 37FE 3802"
	$"81F2 F2F6 0000 7704 0000 F200 FFF9 0000"
	$"FFFE 0000 FF81 00E6 0000 FFFE 0000 FFF9"
	$"0002 FF00 FFF3 0001 F2F2 FE00 00F2 FEDA"
	$"F6D4 FEDA 08E4 E4DA DAD4 CECE C8C8 FEC5"
	$"F8C4 F3C5 FCC8 05CE CED4 D4DA DAFE E4FE"
	$"FBFC EE21 D9EE D9D9 ED2C 2E2F 3234 383A"
	$"3E3F 3F3A 372F D9D4 C5C3 C2C3 C4C8 DAD9"
	$"2C2F 3335 3738 FE39 023D F2F2 F600 0073"
	$"0300 00F2 FFF8 0000 FFFE 0000 FF81 00E6"
	$"0000 FFFE 0000 FFF8 0001 FFFF F300 01F2"
	$"F2FE 0000 F2FD DAFE D4FB 2BF9 DA02 D4D4"
	$"CEFE C8FC C501 C4C4 F3C5 F9C8 01CE CEFE"
	$"D401 DADA FEE4 FDFB FBEE 1FD9 E32C 2D2F"
	$"3234 3781 3D3F 3F3D 3831 E3DA C8C3 C2C2"
	$"C4C8 DAEE 2C2F 3335 3739 39FE 8102 3DF2"
	$"F2F6 0000 7404 0000 F200 FFF9 0000 FFFE"
	$"0000 FF81 00E6 0000 FFFE 0000 FFF9 0002"
	$"FF00 FFF3 0001 F2F2 FE00 00F2 FCDA 02D4"
	$"D42B FDD4 002B F9DA 03D4 D4CE CEFE C8F3"
	$"C5F7 C8FC CE01 D4D4 FEDA FEE4 FBFB FCEE"
	$"1FE3 ED2D 2E31 3337 393D 3F3F 3E39 322C"
	$"E4C8 C3C2 C2C3 C5D4 EEED 2F33 3538 3981"
	$"81FE 3A02 3DF2 F2F6 0000 7305 0000 F200"
	$"00FF FA00 00FF FE00 00FF 8100 E600 00FF"
	$"FE00 00FF FA00 03FF 0000 FFF3 0001 F2F2"
	$"FE00 00F2 FCDA 02D4 D42B FDD4 002B F9DA"
	$"01D4 D4FE CEFE C8F7 C5F6 C8FB CEFE D401"
	$"DADA FDE4 F8FB 1EEE D9ED 2C2E 3133 3539"
	$"3A3E 3F3E 3933 2CFB CEC4 C3C2 C4C5 D4FB"
	$"ED2E 3235 3739 FB81 0239 F2F2 F600 007E"
	$"0200 00F2 FE00 02FF 0000 FCFF FE00 00FF"
	$"8100 E600 00FF FE00 FCFF 0200 00FF FE00"
	$"00FF F300 01F2 F2FE 0002 F2E4 E4FC DA05"
	$"2BD4 D4DA DA25 FEDA 00E4 FCDA 01D4 D4FE"
	$"CEFE C8F9 C5FA C8F7 CEFE D401 DADA FEE4"
	$"F7FB 28EE D9E3 2C2E 2F32 3438 813E 3F3E"
	$"8134 2DFB CEC4 C2C2 C3C5 D4FB ED2E 3235"
	$"3839 8181 3A81 8139 3937 F2F2 F600 0077"
	$"0200 00F2 FD00 02FF 00FF FA00 00FF 8100"
	$"E600 00FF FA00 02FF 00FF FD00 00FF F300"
	$"01F2 F2FE 0000 F2FC E402 DADA 25FD DA01"
	$"25DA FDE4 FDDA 03D4 D4CE CEFD C8FE C5F9"
	$"C8F9 CEFA D4FE DAFE E4F7 FB1E EED9 E32C"
	$"2D2F 3234 3781 3D3E 3E81 352E D9D4 C4C3"
	$"C2C3 C5D4 FBED 2E32 3437 39FD 8106 3938"
	$"3735 33F2 F2F6 0000 6C02 0000 F2FC 0001"
	$"FFFF FA00 00FF 8100 E600 00FF FA00 01FF"
	$"FFFC 0000 FFF3 0001 F2F2 FE00 00F2 FDFB"
	$"FEE4 FB1B FBE4 03DA DAD4 D4FE CEF5 C8FD"
	$"CEF7 D4FC DAFE E4F5 FB1D EEE3 ED2D 2E31"
	$"3337 393D 3E3E 3A37 31E3 DAC5 C2C2 C3C5"
	$"D4FB E32E 3234 3739 FD81 0739 3837 3432"
	$"31F2 F2F6 0000 6702 0000 F2FB 0000 FFFA"
	$"0000 FF81 00E6 0000 FFFA 0000 FFFB 0000"
	$"FFF3 0001 F2F2 FE00 00F2 EFE4 FEDA 01D4"
	$"D4FD CEFC C8FD CEFD D4FD DAF4 E4FE FBF5"
	$"EE1D E3ED 2C2E 3133 3538 813D 3E3A 3832"
	$"EDE4 C8C3 C2C3 C5D4 FBE3 2D31 3335 3839"
	$"FE81 0839 3837 3432 312F F2F2 F600 0060"
	$"0200 00F2 F300 00FF 8100 E600 00FF F300"
	$"00FF F300 01F2 F2FE 0000 F2F0 E4FD DA01"
	$"D4D4 F8CE FED4 FDDA FEE4 FEFB 00EE F7FB"
	$"F2EE 2AD9 ED2C 2E2F 3234 3781 3D3E 3D39"
	$"332C FBC8 C3C2 C3C5 CEFB E32D 3133 3537"
	$"3939 8181 3938 3734 3331 2E2D F2F2 F600"
	$"0051 0100 0081 F2B6 F2FE 0000 F2F4 E4F8"
	$"DAFB D4FC DAFD E401 FBFB FDEE F9D9 FCEE"
	$"F6D9 20EE D9D9 E3ED 2C2D 2F31 3337 393A"
	$"3D3A 3934 2EEE D4C5 C3C4 C5D4 E4D9 2C2F"
	$"3234 3738 FD39 0938 3735 3332 2F2E 2DF2"
	$"F2F6 0000 5801 0000 81F2 B6F2 FE00 02F2"
	$"E4E4 FAFB FDE4 F4DA FDE4 FDFB FDEE 01D9"
	$"D9FD E3FD EDFC E3FE D9F7 E3FE D92C E3ED"
	$"2C2D 2E31 3335 3881 3D3A 8135 2FE3 DAC8"
	$"C4C4 C5CE E4D9 2C2E 3133 3537 3838 3939"
	$"3837 3534 3331 2F2E 2DF2 F2F6 0000 5081"
	$"00B1 0000 F2FD E4F9 FBF5 E4FC FBFC EEFD"
	$"D9FE E3FD ED00 2CFD EDF7 E3FD EDFC E31E"
	$"ED2C 2D2E 2F32 3437 3981 3A39 3531 EDFB"
	$"D4C5 C5C8 D4E4 D9ED 2D2F 3234 3537 37FE"
	$"380A 3735 3433 3231 2F2D 2DF2 F2F6 0000"
	$"4E81 00B1 0002 F2DA DAFE E4F0 FBFB EEFC"
	$"D9FC E3FE EDFA 2CFA ED00 E3F7 ED20 E3E3"
	$"ED2C 2D2D 2F31 3337 3981 3A81 3732 2DEE"
	$"D4C8 C5C8 D4E4 D9ED 2D2F 3233 3435 35FD"
	$"370A 3534 3332 312F 2E2D 2CF2 F2F6 0000"
	$"4F81 00B1 0000 F2FE E4FD FBF6 EEF5 D9FA"
	$"E3FD EDFA 2CFA ED00 E3FC ED00 2CFB ED2D"
	$"2C2C 2D2E 3133 3437 3981 3938 342F EDFB"
	$"DACE CED4 E4EE E32C 2E2F 3132 3334 3535"
	$"3735 3534 3433 3231 2F2E 2D2C F2F2 F600"
	$"0008 8100 B100 8EF2 F600 0008 8100 B100"
	$"8EF2 F600 0006 8100 8100 B300 0006 8100"
	$"8100 B300 0006 8100 8100 B300 00A0 008F"
	$"00A0 0083 00FF"
};

resource 'PICT' (9997) {
	141,
	{72, 27, 94, 49},
	$"1101 0100 0A00 0000 0001 5602 0090 0004"
	$"0048 0018 005E 0038 0048 001B 005E 0031"
	$"0048 001B 005E 0031 0000 0000 0000 0000"
	$"0000 0002 0000 0005 0000 0008 8000 0010"
	$"4000 0020 2000 0040 1000 0080 0800 0100"
	$"0400 01E0 3C00 01E0 3C00 0020 2000 0020"
	$"2000 0020 2000 0020 2000 0020 2000 003F"
	$"E000 003F E000 0000 0000 0000 0000 0000"
	$"0000 FF"
};

resource 'PICT' (9998) {
	145,
	{72, 48, 95, 69},
	$"1101 0100 0A00 0000 0001 5602 0090 0004"
	$"0048 0030 005F 0048 0048 0030 005F 0045"
	$"0048 0030 005F 0045 0000 0000 0000 0000"
	$"0000 0000 0000 03FE 0000 0202 0000 0202"
	$"0000 0202 0000 0202 0000 0202 0000 0202"
	$"0000 1E03 C000 1000 4000 1800 C000 0C01"
	$"8000 0603 0000 0306 0000 018C 0000 00D8"
	$"0000 0070 0000 0020 0000 0000 0000 0000"
	$"0000 0000 0000 FF"
};

resource 'PICT' (9999) {
	4092,
	{82, 34, 316, 276},
	$"1101 0100 0A00 0000 0001 5602 0098 0020"
	$"0052 0020 013C 0118 0052 0022 013C 0114"
	$"0052 0022 013C 0114 0000 02E1 0002 E100"
	$"0800 07E5 FF00 FCFF 0008 0006 E500 000C"
	$"FF00 0800 05E5 0000 16FF 0009 0104 80E6"
	$"0000 26FF 0009 0104 40E6 0000 46FF 0009"
	$"0104 3FE6 FF00 86FF 0009 0104 20E6 0000"
	$"86FF 000A 0104 2FE7 FF01 FE86 FF00 0A01"
	$"0428 E700 0102 86FF 000A 0104 28E7 0001"
	$"0286 FF00 0A01 0428 E700 0102 86FF 000A"
	$"0104 28E7 0001 0286 FF00 0A01 0428 E700"
	$"0102 86FF 000A 0104 28E7 0001 0286 FF00"
	$"0A01 0428 E700 0102 86FF 0010 0104 28F5"
	$"0002 7FFF F0F6 0001 0286 FF00 1201 0428"
	$"F600 047F 8020 0FF0 F700 0102 86FF 0014"
	$"0104 28F7 0006 0F80 0020 000F 80F8 0001"
	$"0286 FF00 1401 0428 F700 06F0 0000 2000"
	$"0078 F800 0102 86FF 0015 0104 28F8 0000"
	$"07FE 0003 2000 0007 F800 0102 86FF 0016"
	$"0104 28F8 0000 38FE 0000 20FE 0000 E0F9"
	$"0001 0286 FF00 1701 0428 F900 0101 C0FE"
	$"0000 20FE 0000 1CF9 0001 0286 FF00 1601"
	$"0428 F900 0006 FD00 0020 FE00 0003 F900"
	$"0102 86FF 0016 0104 28F9 0000 38FD 0000"
	$"20FD 0000 E0FA 0001 0286 FF00 1201 0428"
	$"F900 00C0 F800 0018 FA00 0102 86FF 0012"
	$"0104 28FA 0000 03F7 0000 06FA 0001 0286"
	$"FF00 1301 0428 FA00 000C F700 0101 80FB"
	$"0001 0286 FF00 1201 0428 FA00 0030 F600"
	$"0060 FB00 0102 86FF 0012 0104 28FA 0000"
	$"C0F6 0000 18FB 0001 0286 FF00 1301 0428"
	$"FB00 0101 80F6 0000 0CFB 0001 0286 FF00"
	$"1201 0428 FB00 0006 F500 0003 FB00 0102"
	$"86FF 0012 0104 28FB 0000 08F4 0000 80FC"
	$"0001 0286 FF00 1201 0428 FB00 0030 F400"
	$"0060 FC00 0102 86FF 0016 0104 28FB 0000"
	$"40FB 0000 20FB 0000 10FC 0001 0286 FF00"
	$"1701 0428 FC00 0101 80FB 0000 20FB 0000"
	$"0CFC 0001 0286 FF00 1601 0428 FC00 0003"
	$"FA00 0020 FB00 0006 FC00 0102 86FF 0016"
	$"0104 28FC 0000 04FA 0000 20FB 0000 01FC"
	$"0001 0286 FF00 1601 0428 FC00 0018 FA00"
	$"0020 FA00 00C0 FD00 0102 86FF 0016 0104"
	$"28FC 0000 30FA 0000 20FA 0000 60FD 0001"
	$"0286 FF00 1601 0428 FC00 0060 FA00 0020"
	$"FA00 0030 FD00 0102 86FF 0016 0104 28FC"
	$"0000 80FA 0000 20FA 0000 08FD 0001 0286"
	$"FF00 1601 0428 FD00 0001 F900 0020 FA00"
	$"0004 FD00 0102 86FF 0012 0104 28FD 0000"
	$"02F1 0000 02FD 0001 0286 FF00 1201 0428"
	$"FD00 0004 F100 0001 FD00 0102 86FF 0012"
	$"0104 28FD 0000 08F0 0000 80FE 0001 0286"
	$"FF00 1201 0428 FD00 0010 F000 0040 FE00"
	$"0102 86FF 0012 0104 28FD 0000 20F0 0000"
	$"20FE 0001 0286 FF00 1201 0428 FD00 0040"
	$"F000 0010 FE00 0102 86FF 0012 0104 28FD"
	$"0000 80F0 0000 08FE 0001 0286 FF00 1201"
	$"0428 FE00 0001 EF00 0004 FE00 0102 86FF"
	$"0012 0104 28FE 0000 03EF 0000 06FE 0001"
	$"0286 FF00 1601 0428 FE00 0006 F800 0020"
	$"F900 0003 FE00 0102 86FF 0016 0104 28FE"
	$"0000 04F8 0000 20F9 0000 01FE 0001 0286"
	$"FF00 1501 0428 FE00 0008 F800 0020 F800"
	$"0480 0000 0286 FF00 1501 0428 FE00 0010"
	$"F800 0020 F800 0440 0000 0286 FF00 1501"
	$"0428 FE00 0030 F800 0020 F800 0460 0000"
	$"0286 FF00 1501 0428 FE00 0020 F800 0020"
	$"F800 0420 0000 0286 FF00 1501 0428 FE00"
	$"0040 F800 0020 F800 0410 0000 0286 FF00"
	$"1501 0428 FE00 0080 F800 0020 F800 0408"
	$"0000 0286 FF00 1501 0428 FE00 0080 F800"
	$"0020 F800 0408 0000 0286 FF00 1004 0428"
	$"0000 01ED 0004 0400 0002 86FF 0010 0404"
	$"2800 0002 ED00 0402 0000 0286 FF00 1004"
	$"0428 0000 02ED 0004 0200 0002 86FF 0010"
	$"0404 2800 0004 ED00 0401 0000 0286 FF00"
	$"1004 0428 0000 0CED 0004 0180 0002 86FF"
	$"000F 0404 2800 0008 EC00 0380 0002 86FF"
	$"000F 0404 2800 0010 EC00 0340 0002 86FF"
	$"000F 0404 2800 0010 EC00 0340 0002 86FF"
	$"000F 0404 2800 0020 EC00 0320 0002 86FF"
	$"0013 0404 2800 0020 F700 0020 F700 0320"
	$"0002 86FF 0013 0404 2800 0040 F700 0020"
	$"F700 0310 0002 86FF 0013 0404 2800 0040"
	$"F700 0020 F700 0310 0002 86FF 0013 0404"
	$"2800 0080 F700 0020 F700 0308 0002 86FF"
	$"0013 0404 2800 0080 F700 0020 F700 0308"
	$"0002 86FF 0012 0304 2800 01F6 0000 20F7"
	$"0003 0400 0286 FF00 1203 0428 0001 F600"
	$"0020 F700 0304 0002 86FF 0012 0304 2800"
	$"01F6 0000 20F7 0003 0400 0286 FF00 1203"
	$"0428 0002 F600 0020 F700 0302 0002 86FF"
	$"000E 0304 2800 02EB 0003 0200 0286 FF00"
	$"0E03 0428 0004 EB00 0301 0002 86FF 000E"
	$"0304 2800 04EB 0003 0100 0286 FF00 0E03"
	$"0428 0004 EB00 0301 0002 86FF 000D 0304"
	$"2800 08EA 0002 8002 86FF 000D 0304 2800"
	$"08EA 0002 8002 86FF 000D 0304 2800 08EA"
	$"0002 8002 86FF 000D 0304 2800 10EA 0002"
	$"4002 86FF 000D 0304 2800 10EA 0002 4002"
	$"86FF 0011 0304 2800 10F6 0000 20F6 0002"
	$"4002 86FF 0011 0304 2800 20F6 0000 20F6"
	$"0002 2002 86FF 0011 0304 2800 20F6 0000"
	$"20F6 0002 2002 86FF 0011 0304 2800 20F6"
	$"0000 20F6 0002 2002 86FF 0011 0304 2800"
	$"20F6 0000 20F6 0002 2002 86FF 0011 0304"
	$"2800 40F6 0000 20F6 0002 1002 86FF 0011"
	$"0304 2800 40F6 0000 20F6 0002 1002 86FF"
	$"0011 0304 2800 40F6 0000 20F6 0002 1002"
	$"86FF 0011 0304 2800 40F6 0000 20F6 0002"
	$"1002 86FF 000D 0304 2800 40EA 0002 1002"
	$"86FF 000D 0304 2800 80EA 0002 0802 86FF"
	$"000D 0304 2800 80EA 0002 0802 86FF 000D"
	$"0304 2800 80EA 0002 0802 86FF 000D 0304"
	$"2800 80EA 0002 0802 86FF 000D 0304 2800"
	$"80EA 0002 0802 86FF 000D 0304 2800 80EA"
	$"0002 0802 86FF 000D 0304 2800 80EA 0002"
	$"0802 86FF 000D 0304 2800 80EA 0002 0802"
	$"86FF 0010 0204 2801 F500 0020 F600 0204"
	$"0286 FF00 1002 0428 01F5 0000 20F6 0002"
	$"0402 86FF 0010 0204 2801 F500 0020 F600"
	$"0204 0286 FF00 1002 0428 01F5 0000 20F6"
	$"0002 0402 86FF 0010 0204 2801 F500 0020"
	$"F600 0204 0286 FF00 1002 0428 01F5 0000"
	$"20F6 0002 0402 86FF 0010 0204 2801 F500"
	$"0020 F600 0204 0286 FF00 1002 0428 01F5"
	$"0000 20F6 0002 0402 86FF 0010 0204 2801"
	$"F500 0020 F600 0204 0286 FF00 211D 0428"
	$"01FF 007F C01F F007 FC01 FF00 7FFF E00F"
	$"F803 FE00 FF80 3FE0 0FFC 0286 FF00 1002"
	$"0428 01F5 0000 20F6 0002 0402 86FF 0010"
	$"0204 2801 F500 0020 F600 0204 0286 FF00"
	$"1002 0428 01F5 0000 20F6 0002 0402 86FF"
	$"0010 0204 2801 F500 0020 F600 0204 0286"
	$"FF00 1002 0428 01F5 0000 20F6 0002 0402"
	$"86FF 0010 0204 2801 F500 0020 F600 0204"
	$"0286 FF00 1002 0428 01F5 0000 20F6 0002"
	$"0402 86FF 0010 0204 2801 F500 0020 F600"
	$"0204 0286 FF00 0C02 0428 01E9 0002 0402"
	$"86FF 000D 0304 2800 80EA 0002 0802 86FF"
	$"000D 0304 2800 80EA 0002 0802 86FF 000D"
	$"0304 2800 80EA 0002 0802 86FF 000D 0304"
	$"2800 80EA 0002 0802 86FF 000D 0304 2800"
	$"80EA 0002 0802 86FF 000D 0304 2800 80EA"
	$"0002 0802 86FF 000D 0304 2800 80EA 0002"
	$"0802 86FF 000D 0304 2800 80EA 0002 0802"
	$"86FF 0011 0304 2800 40F6 0000 20F6 0002"
	$"1002 86FF 0011 0304 2800 40F6 0000 20F6"
	$"0002 1002 86FF 0011 0304 2800 40F6 0000"
	$"20F6 0002 1002 86FF 0011 0304 2800 40F6"
	$"0000 20F6 0002 1002 86FF 0011 0304 2800"
	$"40F6 0000 20F6 0002 1002 86FF 0011 0304"
	$"2800 20F6 0000 20F6 0002 2002 86FF 0011"
	$"0304 2800 20F6 0000 20F6 0002 2002 86FF"
	$"0011 0304 2800 20F6 0000 20F6 0002 2002"
	$"86FF 0011 0304 2800 20F6 0000 20F6 0002"
	$"2002 86FF 000D 0304 2800 10EA 0002 4002"
	$"86FF 000D 0304 2800 10EA 0002 4002 86FF"
	$"000D 0304 2800 10EA 0002 4002 86FF 000D"
	$"0304 2800 08EA 0002 8002 86FF 000D 0304"
	$"2800 08EA 0002 8002 86FF 000D 0304 2800"
	$"08EA 0002 8002 86FF 000E 0304 2800 04EB"
	$"0003 0100 0286 FF00 0E03 0428 0004 EB00"
	$"0301 0002 86FF 000E 0304 2800 04EB 0003"
	$"0100 0286 FF00 1203 0428 0002 F600 0020"
	$"F700 0302 0002 86FF 0012 0304 2800 02F6"
	$"0000 20F7 0003 0200 0286 FF00 1203 0428"
	$"0001 F600 0020 F700 0304 0002 86FF 0012"
	$"0304 2800 01F6 0000 20F7 0003 0400 0286"
	$"FF00 1203 0428 0001 F600 0020 F700 0304"
	$"0002 86FF 0013 0404 2800 0080 F700 0020"
	$"F700 0308 0002 86FF 0013 0404 2800 0080"
	$"F700 0020 F700 0308 0002 86FF 0013 0404"
	$"2800 0040 F700 0020 F700 0310 0002 86FF"
	$"0013 0404 2800 0040 F700 0020 F700 0310"
	$"0002 86FF 000F 0404 2800 0020 EC00 0320"
	$"0002 86FF 000F 0404 2800 0020 EC00 0320"
	$"0002 86FF 000F 0404 2800 0010 EC00 0340"
	$"0002 86FF 000F 0404 2800 0010 EC00 0340"
	$"0002 86FF 000F 0404 2800 0008 EC00 0380"
	$"0002 86FF 0010 0404 2800 000C ED00 0401"
	$"8000 0286 FF00 1004 0428 0000 04ED 0004"
	$"0100 0002 86FF 0010 0404 2800 0002 ED00"
	$"0402 0000 0286 FF00 1004 0428 0000 02ED"
	$"0004 0200 0002 86FF 0014 0404 2800 0001"
	$"F700 0020 F800 0404 0000 0286 FF00 1501"
	$"0428 FE00 0080 F800 0020 F800 0408 0000"
	$"0286 FF00 1501 0428 FE00 0080 F800 0020"
	$"F800 0408 0000 0286 FF00 1501 0428 FE00"
	$"0040 F800 0020 F800 0410 0000 0286 FF00"
	$"1501 0428 FE00 0020 F800 0020 F800 0420"
	$"0000 0286 FF00 1501 0428 FE00 0030 F800"
	$"0020 F800 0460 0000 0286 FF00 1501 0428"
	$"FE00 0010 F800 0020 F800 0440 0000 0286"
	$"FF00 1501 0428 FE00 0008 F800 0020 F800"
	$"0480 0000 0286 FF00 1601 0428 FE00 0004"
	$"F800 0020 F900 0001 FE00 0102 86FF 0012"
	$"0104 28FE 0000 06EF 0000 03FE 0001 0286"
	$"FF00 1201 0428 FE00 0003 EF00 0006 FE00"
	$"0102 86FF 0012 0104 28FE 0000 01EF 0000"
	$"04FE 0001 0286 FF00 1201 0428 FD00 0080"
	$"F000 0008 FE00 0102 86FF 0012 0104 28FD"
	$"0000 40F0 0000 10FE 0001 0286 FF00 1201"
	$"0428 FD00 0020 F000 0020 FE00 0102 86FF"
	$"0012 0104 28FD 0000 10F0 0000 40FE 0001"
	$"0286 FF00 1201 0428 FD00 0008 F000 0080"
	$"FE00 0102 86FF 0012 0104 28FD 0000 04F1"
	$"0000 01FD 0001 0286 FF00 1601 0428 FD00"
	$"0002 F900 0020 FA00 0002 FD00 0102 86FF"
	$"0016 0104 28FD 0000 01F9 0000 20FA 0000"
	$"04FD 0001 0286 FF00 1601 0428 FC00 0080"
	$"FA00 0020 FA00 0008 FD00 0102 86FF 0016"
	$"0104 28FC 0000 60FA 0000 20FA 0000 30FD"
	$"0001 0286 FF00 1601 0428 FC00 0030 FA00"
	$"0020 FA00 0060 FD00 0102 86FF 0016 0104"
	$"28FC 0000 18FA 0000 20FA 0000 C0FD 0001"
	$"0286 FF00 1601 0428 FC00 0004 FA00 0020"
	$"FB00 0001 FC00 0102 86FF 0016 0104 28FC"
	$"0000 03FA 0000 20FB 0000 06FC 0001 0286"
	$"FF00 1701 0428 FC00 0101 80FB 0000 20FB"
	$"0000 0CFC 0001 0286 FF00 1201 0428 FB00"
	$"0040 F400 0010 FC00 0102 86FF 0012 0104"
	$"28FB 0000 30F4 0000 60FC 0001 0286 FF00"
	$"1201 0428 FB00 0008 F400 0080 FC00 0102"
	$"86FF 0012 0104 28FB 0000 06F5 0000 03FB"
	$"0001 0286 FF00 1301 0428 FB00 0101 80F6"
	$"0000 0CFB 0001 0286 FF00 1201 0428 FA00"
	$"00C0 F600 0018 FB00 0102 86FF 0012 0104"
	$"28FA 0000 30F6 0000 60FB 0001 0286 FF00"
	$"1301 0428 FA00 000C F700 0101 80FB 0001"
	$"0286 FF00 1201 0428 FA00 0003 F700 0006"
	$"FA00 0102 86FF 0016 0104 28F9 0000 C0FD"
	$"0000 20FD 0000 18FA 0001 0286 FF00 1601"
	$"0428 F900 0038 FD00 0020 FD00 00E0 FA00"
	$"0102 86FF 0016 0104 28F9 0000 06FD 0000"
	$"20FE 0000 03F9 0001 0286 FF00 1701 0428"
	$"F900 0101 C0FE 0000 20FE 0000 1CF9 0001"
	$"0286 FF00 1601 0428 F800 0038 FE00 0020"
	$"FE00 00E0 F900 0102 86FF 0015 0104 28F8"
	$"0000 07FE 0003 2000 0007 F800 0102 86FF"
	$"0014 0104 28F7 0006 F000 0020 0000 78F8"
	$"0001 0286 FF00 1401 0428 F700 060F 8000"
	$"2000 0F80 F800 0102 86FF 0012 0104 28F6"
	$"0004 7F80 200F F0F7 0001 0286 FF00 1001"
	$"0428 F500 027F FFF0 F600 0102 86FF 000A"
	$"0104 28E7 0001 0286 FF00 0A01 0428 E700"
	$"0102 86FF 000A 0104 28E7 0001 0286 FF00"
	$"0A01 0428 E700 0102 86FF 000A 0104 28E7"
	$"0001 0286 FF00 0A01 0428 E700 0102 86FF"
	$"000A 0104 2FE7 FF01 FE86 FF00 0901 0420"
	$"E600 0086 FF00 0901 043F E6FF 0086 FF00"
	$"0901 0440 E600 0046 FF00 0901 0480 E600"
	$"0026 FF00 0800 05E5 0000 16FF 0008 0006"
	$"E500 000E FF00 0800 07E5 FF00 FEFF 0008"
	$"0001 E5FF 00FE FF00 02E1 0002 E100 02E1"
	$"00FF"
};

resource 'PICT' (9994) {
	262,
	{246, 300, 259, 445},
	$"1101 0100 0A00 0000 0001 5602 0098 0014"
	$"00F6 0128 0103 01C0 00F6 012C 0103 01BD"
	$"00F6 012C 0103 01BD 0000 02ED 0015 0500"
	$"4000 01E0 20FE 0008 7800 1381 0000 4020"
	$"10FF 0015 0500 4000 0110 20FE 0008 4400"
	$"1441 0000 4020 10FF 0015 0500 A556 5116"
	$"76FE 0008 4495 2409 5798 4328 93FF 0015"
	$"0500 A661 5111 21FE 0008 7955 2415 5554"
	$"40B5 54FF 0015 0500 A443 5113 23FE 0008"
	$"4555 2415 5554 41A5 D2FF 0015 0501 F445"
	$"5115 25FE 0008 4555 2415 5554 42A5 11FF"
	$"0015 0501 1445 3115 25FE 0008 454A 4455"
	$"5554 42B5 55FF 0015 0501 1443 11E3 33FE"
	$"0008 448A 4389 3554 79A8 92FF 0006 FE00"
	$"0010 F100 1005 03FF FF67 FFFF FE00 007F"
	$"F9FF 0180 0002 ED00 02ED 00FF"
};

resource 'PICT' (300) {
	1552,
	{0, 0, 322, 173},
	$"1101 A014 DEA1 14DE 0028 0002 8104 0002"
	$"8104 0000 0000 0142 00AD 0001 0000 0000"
	$"0006 0002 80B4 696C 000C 0000 0000 0000"
	$"0000 A114 DE02 E902 E9FF FFFF FF01 4100"
	$"AC11 01A0 0082 A100 9600 0C02 0000 0002"
	$"0000 0000 0000 00A1 009A 0008 FFFC 0000"
	$"002A 0000 0100 0A00 1A00 2300 2D00 7B03"
	$"0003 0410 0D00 0C2B 2427 0A41 7474 7269"
	$"6275 7465 73A0 0097 A100 9600 0C01 0000"
	$"0002 0000 0000 0000 00A1 009A 0008 FFFC"
	$"0000 004B 0000 0100 0A00 6200 1200 7300"
	$"AB04 0028 006E 0013 1749 6E64 6570 656E"
	$"6465 6E74 2076 6172 2C20 782D 6178 6973"
	$"A000 97A1 0096 000C 0100 0000 0200 0000"
	$"0000 0000 A100 9A00 08FF FC00 0000 4B00"
	$"0001 000A 007D 0012 008E 00AB 2A1B 1749"
	$"6E64 6570 656E 6465 6E74 2076 6172 2C20"
	$"792D 6178 6973 A000 97A1 0096 000C 0100"
	$"0000 0200 0000 0000 0000 A100 9A00 08FF"
	$"FC00 0000 2D00 0001 000A 0000 0000 000D"
	$"005D 0401 0D00 0A28 000A 0001 0E4E 4353"
	$"4120 4461 7461 5363 6F70 65A0 0097 A100"
	$"9600 0C01 0000 0002 0000 0000 0000 00A1"
	$"009A 0008 FFFC 0000 002D 0000 0100 0A00"
	$"AB00 1200 B900 6F03 0015 0400 0D00 0C2B"
	$"12AB 0F44 6973 706C 6179 2046 6F72 6D61"
	$"7473 A000 97A1 0096 000C 0100 0000 0200"
	$"0000 0000 0000 A100 9A00 08FF FC00 0000"
	$"3D00 0001 000A 0047 0012 0058 008F 0300"
	$"0328 0053 0013 1244 6570 656E 6465 6E74"
	$"2076 6172 6961 626C 65A0 0097 A100 9600"
	$"0C01 0000 0002 0000 0000 0000 00A1 009A"
	$"0008 FFFD 0000 0047 0000 0100 0A00 E100"
	$"1200 F000 A303 0015 2A98 194D 696E 696D"
	$"756D 2076 616C 7565 206F 6620 696E 7465"
	$"7265 7374 A000 97A1 0096 000C 0100 0000"
	$"0200 0000 0000 0000 A100 9A00 08FF FD00"
	$"0000 4900 0001 000A 00C6 0012 00D5 00A7"
	$"2800 D000 1319 4D61 7869 6D75 6D20 7661"
	$"6C75 6520 6F66 2069 6E74 6572 6573 74A0"
	$"0097 A100 9600 0C01 0000 0002 0000 0000"
	$"0000 00A1 009A 0008 FFFC 0000 0040 0000"
	$"0100 0A01 1700 1201 2500 952A 5116 436F"
	$"6C6F 7220 4D69 6E2C 204D 6178 2028 312D"
	$"3235 3429 A000 97A1 0096 000C 0100 0000"
	$"0200 0000 0000 0000 A100 9A00 08FF FD00"
	$"0000 3B00 0001 000A 0132 0012 0140 008B"
	$"2A1B 1444 696D 656E 7369 6F6E 7320 2872"
	$"6F77 2C63 6F6C 29A0 0097 0100 0AFF FFFF"
	$"FF01 4100 AC22 009B 0018 7E00 A000 83FF"
	$"A000 82A1 0096 000C 0200 0000 0200 0000"
	$"0000 0000 A100 9A00 08FF FC00 0000 2A00"
	$"0001 000A 001B 0024 002E 007C 0300 0304"
	$"100D 000C 1001 4200 AD01 4200 AD2B 2528"
	$"0A41 7474 7269 6275 7465 73A0 0097 A100"
	$"9600 0C01 0000 0002 0000 0000 0000 00A1"
	$"009A 0008 FFFC 0000 004B 0000 0100 0A00"
	$"6300 1300 7400 AC04 0028 006F 0014 1749"
	$"6E64 6570 656E 6465 6E74 2076 6172 2C20"
	$"782D 6178 6973 A000 97A1 0096 000C 0100"
	$"0000 0200 0000 0000 0000 A100 9A00 08FF"
	$"FC00 0000 4B00 0001 000A 007E 0013 008F"
	$"00AC 2A1B 1749 6E64 6570 656E 6465 6E74"
	$"2076 6172 2C20 792D 6178 6973 A000 97A1"
	$"0096 000C 0100 0000 0200 0000 0000 0000"
	$"A100 9A00 08FF FC00 0000 2D00 0001 000A"
	$"0001 0001 000E 005E 0401 0D00 0A28 000B"
	$"0002 0E4E 4353 4120 4461 7461 5363 6F70"
	$"65A0 0097 A100 9600 0C01 0000 0002 0000"
	$"0000 0000 00A1 009A 0008 FFFC 0000 002D"
	$"0000 0100 0A00 AC00 1300 BA00 7003 0015"
	$"0400 0D00 0C2B 12AB 0F44 6973 706C 6179"
	$"2046 6F72 6D61 7473 A000 97A1 0096 000C"
	$"0100 0000 0200 0000 0000 0000 A100 9A00"
	$"08FF FC00 0000 3D00 0001 000A 0048 0013"
	$"0059 0090 0300 0328 0054 0014 1244 6570"
	$"656E 6465 6E74 2076 6172 6961 626C 65A0"
	$"0097 A100 9600 0C01 0000 0002 0000 0000"
	$"0000 00A1 009A 0008 FFFD 0000 0047 0000"
	$"0100 0A00 E200 1300 F100 A403 0015 2A98"
	$"194D 696E 696D 756D 2076 616C 7565 206F"
	$"6620 696E 7465 7265 7374 A000 97A1 0096"
	$"000C 0100 0000 0200 0000 0000 0000 A100"
	$"9A00 08FF FD00 0000 4900 0001 000A 00C7"
	$"0013 00D6 00A8 2800 D100 1419 4D61 7869"
	$"6D75 6D20 7661 6C75 6520 6F66 2069 6E74"
	$"6572 6573 74A0 0097 A100 9600 0C01 0000"
	$"0002 0000 0000 0000 00A1 009A 0008 FFFC"
	$"0000 0040 0000 0100 0A01 1800 1301 2600"
	$"962A 5116 436F 6C6F 7220 4D69 6E2C 204D"
	$"6178 2028 312D 3235 3429 A000 97A1 0096"
	$"000C 0100 0000 0200 0000 0000 0000 A100"
	$"9A00 08FF FD00 0000 3B00 0001 000A 0133"
	$"0013 0141 008C 2A1B 1444 696D 656E 7369"
	$"6F6E 7320 2872 6F77 2C63 6F6C 29A0 0097"
	$"0100 0A00 0000 0001 4200 AD22 009C 0019"
	$"7E00 A000 83FF"
};

resource 'PICT' (151) {
	7336,
	{105, 97, 237, 424},
	$"0011 02FF 0C00 FFFF FFFF 0061 0000 0069"
	$"0000 01A8 0000 00ED 0000 0000 0000 001E"
	$"0001 000A 0000 0038 0135 01DC 0099 8148"
	$"0069 0061 00ED 01A8 0000 0000 0000 0000"
	$"0048 0000 0048 0000 0000 0008 0001 0008"
	$"0000 0000 0000 1F10 0000 0000 011B 229C"
	$"8000 00FF 8800 FFFF FFFF FFFF 8800 0000"
	$"6900 FF00 8800 0000 6300 FF00 8800 0000"
	$"5D00 FF00 8800 0000 5600 FF00 8800 0000"
	$"5000 FF00 8800 0000 4A00 FF00 8800 0000"
	$"4300 FF00 8800 0000 3D00 FF00 8800 0000"
	$"3600 FF00 8800 0000 3000 FF00 8800 0000"
	$"2A00 FF00 8800 0000 2300 FF00 8800 0000"
	$"1D00 FF00 8800 0000 1700 FF00 8800 0000"
	$"1000 FF00 8800 0000 0A00 FF00 8800 0000"
	$"0300 FF00 8800 0600 0000 FF00 8800 0C00"
	$"0000 FF00 8800 1300 0000 FF00 8800 1900"
	$"0000 FF00 8800 1F00 0000 FF00 8800 2600"
	$"0000 FF00 8800 2C00 0000 FF00 8800 3300"
	$"0000 FF00 8800 3900 0000 FF00 8800 3F00"
	$"0000 FF00 8800 4600 0000 FF00 8800 4C00"
	$"0000 FF00 8800 5200 0000 FF00 8800 5900"
	$"0000 FF00 8800 5F00 0000 FF00 8800 6600"
	$"0000 FF00 8800 6C00 0000 FF00 8800 7200"
	$"0000 FF00 8800 7900 0000 FF00 8800 7F00"
	$"0000 FF00 8800 8500 0000 FF00 8800 8C00"
	$"0000 FF00 8800 9200 0000 FF00 8800 9900"
	$"0000 FF00 8800 9F00 0000 FF00 8800 A500"
	$"0000 FF00 8800 AC00 0000 FF00 8800 B200"
	$"0000 FF00 8800 B800 0000 FF00 8800 BF00"
	$"0000 FF00 8800 C500 0000 FF00 8800 CC00"
	$"0000 FF00 8800 D200 0000 FF00 8800 D800"
	$"0000 FF00 8800 DF00 0000 FF00 8800 E500"
	$"0000 FF00 8800 EB00 0000 FF00 8800 F200"
	$"0000 FF00 8800 F800 0000 FF00 8800 FF00"
	$"0000 FF00 8800 FF00 0000 F800 8800 FF00"
	$"0000 F200 8800 FF00 0000 EB00 8800 FF00"
	$"0000 E500 8800 FF00 0000 DF00 8800 FF00"
	$"0000 D800 8800 FF00 0000 D200 8800 FF00"
	$"0000 CC00 8800 FF00 0000 C500 8800 FF00"
	$"0000 BF00 8800 FF00 0000 B800 8800 FF00"
	$"0000 B200 8800 FF00 0000 AC00 8800 FF00"
	$"0000 A500 8800 FF00 0000 9F00 8800 FF00"
	$"0000 9900 8800 FF00 0000 9200 8800 FF00"
	$"0000 8C00 8800 FF00 0000 8500 8800 FF00"
	$"0000 7F00 8800 FF00 0000 7900 8800 FF00"
	$"0000 7200 8800 FF00 0000 6C00 8800 FF00"
	$"0000 6600 8800 FF00 0000 5F00 8800 FF00"
	$"0000 5900 8800 FF00 0000 5200 8800 FF00"
	$"0000 4C00 8800 FF00 0000 4600 8800 FF00"
	$"0000 3F00 8800 FF00 0000 3900 8800 FF00"
	$"0000 3300 8800 FF00 0000 2C00 8800 FF00"
	$"0000 2600 8800 FF00 0000 1F00 8800 FF00"
	$"0000 1900 8800 FF00 0000 1300 8800 FF00"
	$"0000 0C00 8800 FF00 0000 0600 8800 FF00"
	$"0000 0000 8800 FF00 0600 0000 8800 FF00"
	$"0C00 0000 8800 FF00 1300 0000 8800 FF00"
	$"1900 0000 8800 FF00 1F00 0000 8800 FF00"
	$"2600 0000 8800 FF00 2C00 0000 8800 FF00"
	$"3300 0000 8800 FF00 3900 0000 8800 FF00"
	$"3F00 0000 8800 FF00 4600 0000 8800 FF00"
	$"4C00 0000 8800 FF00 5200 0000 8800 FF00"
	$"5900 0000 8800 FF00 5F00 0000 8800 FF00"
	$"6600 0000 8800 FF00 6C00 0000 8800 FF00"
	$"7200 0000 8800 FF00 7900 0000 8800 FF00"
	$"7F00 0000 8800 FF00 8500 0000 8800 FF00"
	$"8C00 0000 8800 FF00 9200 0000 8800 FF00"
	$"9900 0000 8800 FF00 9F00 0000 8800 FF00"
	$"A500 0000 8800 FF00 AC00 0000 8800 FF00"
	$"B200 0000 8800 FF00 B800 0000 8800 FF00"
	$"BF00 0000 8800 FF00 C500 0000 8800 FF00"
	$"CC00 0000 8800 FF00 D200 0000 8800 FF00"
	$"D800 0000 8800 FF00 DF00 0000 8800 FF00"
	$"E500 0000 8800 FF00 EB00 0000 8800 FF00"
	$"F200 0000 8800 FF00 F800 0000 8800 FF00"
	$"FF00 0000 8800 F800 FF00 0000 8800 F200"
	$"FF00 0000 8800 EB00 FF00 0000 8800 E500"
	$"FF00 0000 8800 DF00 FF00 0000 8800 D800"
	$"FF00 0000 8800 D200 FF00 0000 8800 CC00"
	$"FF00 0000 8800 C500 FF00 0000 8800 BF00"
	$"FF00 0000 8800 B800 FF00 0000 8800 B200"
	$"FF00 0000 8800 AC00 FF00 0000 8800 A500"
	$"FF00 0000 8800 9F00 FF00 0000 8800 9900"
	$"FF00 0000 8800 9200 FF00 0000 8800 8C00"
	$"FF00 0000 8800 8500 FF00 0000 8800 7F00"
	$"FF00 0000 8800 7900 FF00 0000 8800 7200"
	$"FF00 0000 8800 6C00 FF00 0000 8800 6600"
	$"FF00 0000 8800 5F00 FF00 0000 8800 5900"
	$"FF00 0000 8800 5200 FF00 0000 8800 4C00"
	$"FF00 0000 8800 4600 FF00 0000 8800 3F00"
	$"FF00 0000 8800 3900 FF00 0000 8800 3300"
	$"FF00 0000 8800 2C00 FF00 0000 8800 2600"
	$"FF00 0000 8800 1F00 FF00 0000 8800 1900"
	$"FF00 0000 8800 1300 FF00 0000 8800 0C00"
	$"FF00 0000 8800 0600 FF00 0000 8800 0000"
	$"FF00 0000 8800 0000 FF00 0600 8800 0000"
	$"FF00 0C00 8800 0000 FF00 1300 8800 0000"
	$"FF00 1900 8800 0000 FF00 1F00 8800 0000"
	$"FF00 2600 8800 0000 FF00 2C00 8800 0000"
	$"FF00 3300 8800 0000 FF00 3900 8800 0000"
	$"FF00 3F00 8800 0000 FF00 4600 8800 0000"
	$"FF00 4C00 8800 0000 FF00 5200 8800 0000"
	$"FF00 5900 8800 0000 FF00 5F00 8800 0000"
	$"FF00 6600 8800 0000 FF00 6C00 8800 0000"
	$"FF00 7200 8800 0000 FF00 7900 8800 0000"
	$"FF00 7F00 8800 0000 FF00 8500 8800 0000"
	$"FF00 8C00 8800 0000 FF00 9200 8800 0000"
	$"FF00 9900 8800 0000 FF00 9F00 8800 0000"
	$"FF00 A500 8800 0000 FF00 AC00 8800 0000"
	$"FF00 B200 8800 0000 FF00 B800 8800 0000"
	$"FF00 BF00 8800 0000 FF00 C500 8800 0000"
	$"FF00 CC00 8800 0000 FF00 D200 8800 0000"
	$"FF00 D800 8800 0000 FF00 DF00 8800 0000"
	$"FF00 E500 8800 0000 FF00 EB00 8800 0000"
	$"FF00 F200 8800 0000 FF00 F800 8800 0000"
	$"FF00 FF00 8800 0000 F800 FF00 8800 0000"
	$"F200 FF00 8800 0000 EB00 FF00 8800 0000"
	$"E500 FF00 8800 0000 DF00 FF00 8800 0000"
	$"D800 FF00 8800 0000 D200 FF00 8800 0000"
	$"CC00 FF00 8800 0000 C500 FF00 8800 0000"
	$"BF00 FF00 8800 0000 B800 FF00 8800 0000"
	$"B200 FF00 8800 0000 AC00 FF00 8800 0000"
	$"A500 FF00 8800 0000 9F00 FF00 8800 0000"
	$"9900 FF00 8800 0000 9200 FF00 8800 0000"
	$"8C00 FF00 8800 0000 8500 FF00 8800 0000"
	$"7F00 FF00 8800 0000 7900 FF00 8800 0000"
	$"7200 FF00 8800 F000 F000 F000 8800 E000"
	$"E000 E000 8800 D000 D000 D000 8800 C000"
	$"C000 C000 8800 B000 B000 B000 8800 A000"
	$"A000 A000 8800 9000 9000 9000 8800 8000"
	$"8000 8000 8800 7000 7000 7000 8800 6000"
	$"6000 6000 8800 5000 5000 5000 8800 4000"
	$"4000 4000 8800 3000 3000 3000 8800 2000"
	$"2000 2000 8800 1000 1000 1000 8800 0000"
	$"0000 0000 0069 0061 00ED 01A8 0069 0061"
	$"00ED 01A8 0000 000A 0069 0061 00ED 01A8"
	$"0008 8100 8100 BA00 00BC 000B 0000 81FF"
	$"81FF BCFF 0100 AD00 0D01 00FF 8100 8100"
	$"BE00 00FF FF00 000D 0100 FF81 0081 00BE"
	$"0000 FFFF 0000 0D01 00FF 8100 8100 BE00"
	$"00FF FF00 000D 0100 FF81 0081 00BE 0000"
	$"FFFF 0000 0D01 00FF 8100 8100 BE00 00FF"
	$"FF00 000D 0100 FF81 0081 00BE 0000 FFFF"
	$"0000 0D01 00FF 8100 8100 BE00 00FF FF00"
	$"000D 0100 FF81 0081 00BE 0000 FFFF 0000"
	$"0D01 00FF 8100 8100 BE00 00FF FF00 000D"
	$"0100 FF81 0081 00BE 0000 FFFF 0000 0D01"
	$"00FF 8100 8100 BE00 00FF FF00 000D 0100"
	$"FF81 0081 00BE 0000 FFFF 0000 0D01 00FF"
	$"8100 8100 BE00 00FF FF00 000D 0100 FF81"
	$"0081 00BE 0000 FFFF 0000 0D01 00FF 8100"
	$"8100 BE00 00FF FF00 000D 0100 FF81 0081"
	$"00BE 0002 FF00 FF00 0D01 00FF 8100 8100"
	$"BE00 00FF FF00 000D 0100 FF81 0081 00BE"
	$"0000 FFFF 0000 0D01 00FF 8100 8100 BE00"
	$"00FF FF00 000D 0100 FF81 0081 00BE 0000"
	$"FFFF 0000 0D01 00FF 8100 8100 BE00 00FF"
	$"FF00 000D 0100 FF81 0081 00BE 0000 FFFF"
	$"0000 0D01 00FF 8100 8100 BE00 00FF FF00"
	$"000D 0100 FF81 0081 00BE 0000 FFFF 0000"
	$"0D01 00FF 8100 8100 BE00 00FF FF00 000D"
	$"0100 FF81 0081 00BE 0002 FF00 FF00 0D01"
	$"00FF 8100 8100 BE00 00FF FF00 000D 0100"
	$"FF81 0081 00BE 0000 FFFF 0000 0D01 00FF"
	$"8100 8100 BE00 02FF 00FF 000D 0100 FF81"
	$"0081 00BE 0000 FFFF 0000 0D01 00FF 8100"
	$"8100 BE00 00FF FF00 000D 0100 FF81 0081"
	$"00BE 0000 FFFF 0000 0D01 00FF 8100 8100"
	$"BE00 00FF FF00 000D 0100 FF81 0081 00BE"
	$"0000 FFFF 0000 0D01 00FF 8100 8100 BE00"
	$"00FF FF00 000D 0100 FF81 0081 00BE 0000"
	$"FFFF 0000 0D01 00FF 8100 8100 BE00 00FF"
	$"FF00 000D 0100 FF81 0081 00BE 0000 FFFF"
	$"0000 0D01 00FF 8100 8100 BE00 00FF FF00"
	$"000D 0100 FF81 0081 00BE 0000 FFFF 0000"
	$"0D01 00FF 8100 8100 BE00 00FF FF00 000D"
	$"0100 FF81 0081 00BE 0000 FFFF 0000 0D01"
	$"00FF 8100 8100 BE00 00FF FF00 000D 0100"
	$"FF81 0081 00BE 0000 FFFF 0000 0D01 00FF"
	$"8100 8100 BE00 00FF FF00 000D 0100 FF81"
	$"0081 00BE 0000 FFFF 0000 0D01 00FF 8100"
	$"8100 BE00 00FF FF00 000D 0100 FF81 0081"
	$"00BE 0000 FFFF 0000 0D01 00FF 8100 8100"
	$"BE00 00FF FF00 000D 0100 FF81 0081 00BE"
	$"0002 FF00 FF00 0D01 00FF 8100 8100 BE00"
	$"02FF 00FF 000D 0100 FF81 0081 00BE 0000"
	$"FFFF 0000 0D01 00FF 8100 8100 BE00 00FF"
	$"FF00 000D 0100 FF81 0081 00BE 0000 FFFF"
	$"0000 0D01 00FF 8100 8100 BE00 00FF FF00"
	$"000D 0100 FF81 0081 00BE 0000 FFFF 0000"
	$"0D01 00FF 8100 8100 BE00 00FF FF00 000D"
	$"0100 FF81 0081 00BE 0000 FFFF 0000 0D01"
	$"00FF 8100 8100 BE00 00FF FF00 000D 0100"
	$"FF81 0081 00BE 0000 FFFF 0000 0D01 00FF"
	$"8100 8100 BE00 00FF FF00 000D 0100 FF81"
	$"0081 00BE 0000 FFFF 0000 0D01 00FF 8100"
	$"8100 BE00 00FF FF00 000D 0100 FF81 0081"
	$"00BE 0000 FFFF 0000 0D01 00FF 8100 8100"
	$"BE00 00FF FF00 000D 0100 FF81 0081 00BE"
	$"0000 FFFF 0000 0D01 00FF 8100 8100 BE00"
	$"00FF FF00 000D 0100 FF81 0081 00BE 0000"
	$"FFFF 0000 0D01 00FF 8100 8100 BE00 00FF"
	$"FF00 000D 0100 FF81 0081 00BE 0000 FFFF"
	$"0000 0D01 00FF 8100 8100 BE00 00FF FF00"
	$"000D 0100 FF81 0081 00BE 0000 FFFF 0000"
	$"0D01 00FF 8100 8100 BE00 00FF FF00 000D"
	$"0100 FF81 0081 00BE 0000 FFFF 0000 0D01"
	$"00FF 8100 8100 BE00 00FF FF00 000D 0100"
	$"FF81 0081 00BE 0000 FFFF 0000 0D01 00FF"
	$"8100 8100 BE00 00FF FF00 0052 0100 FFD5"
	$"00FC FF01 00FF F500 04FF FF00 00FF FA00"
	$"00FF FE00 00FF F100 01FF FFFC 00FE FFF4"
	$"0000 FFF0 0001 FFFF F300 FEFF CE00 00FF"
	$"FE00 00FF F000 00FF F400 04FF FF00 00FF"
	$"F500 00FF FE00 00FF C700 00FF FF00 0049"
	$"0100 FFD3 0000 FFFE 0000 FFF5 0004 FFFF"
	$"0000 FFFA 0000 FFEC 0000 FFFD 0000 FFFE"
	$"0000 FFF5 0000 FFF1 0000 FFF2 0000 FFFE"
	$"0000 FFCF 0000 FFEC 0000 FFF3 0000 FFF2"
	$"0000 FFC3 0000 FFFF 0000 DD01 00FF D300"
	$"00FF FE00 FEFF FE00 01FF FFFC 000A FF00"
	$"FF00 FF00 00FF FF00 00FE FF08 00FF FF00"
	$"00FF FF00 00FE FFFE 0001 FFFF FE00 00FF"
	$"FD00 00FF FB00 03FF FF00 00FE FF01 0000"
	$"FEFF 0900 00FF FF00 00FF 00FF FFFD 00FE"
	$"FF09 0000 FFFF 0000 FF00 FFFF FD00 00FF"
	$"FC00 04FF 0000 FF00 FEFF FE00 0BFF FF00"
	$"00FF 00FF FF00 00FF FFFE 0003 FFFF 0000"
	$"FEFF 0400 FFFF 0000 FEFF 0600 00FF 0000"
	$"FF00 FEFF 0300 FFFF 00FE FFFE 00FE FFFB"
	$"0004 FF00 FF00 00FE FF01 0000 FEFF FE00"
	$"07FF 00FF FF00 00FF FFFE 0003 FFFF 0000"
	$"FEFF 0800 FFFF 0000 FFFF 0000 FEFF FE00"
	$"FEFF D600 00FF FF00 00E0 0100 FFD3 0000"
	$"FFFE 0008 FF00 00FF 00FF 0000 FFFD 0004"
	$"FF00 FF00 FFFD 0003 FF00 00FF FE00 0AFF"
	$"00FF 0000 FF00 FF00 00FF FD00 03FF 0000"
	$"FFFD 0000 FFFC 0014 FF00 00FF 00FF 0000"
	$"FF00 00FF 0000 FF00 00FF 00FF FFFA 0009"
	$"FF00 00FF 0000 FF00 FFFF FA00 FEFF 1200"
	$"00FF 0000 FF00 FF00 00FF 00FF 0000 FF00"
	$"FFFF FE00 1DFF 0000 FF00 FF00 00FF 00FF"
	$"0000 FF00 00FF 00FF 0000 FF00 FF00 00FF"
	$"0000 FFFE 000A FF00 FF00 00FF 00FF 0000"
	$"FFFB 0018 FF00 FF00 00FF 0000 FF00 FF00"
	$"00FF 0000 FF00 00FF 00FF 0000 FFFD 0003"
	$"FF00 00FF FE00 0CFF 00FF 0000 FF00 FF00"
	$"00FF 00FF D300 00FF FF00 00D4 0100 FFD3"
	$"0000 FFFE 0004 FF00 00FF 00FD FFFD 0006"
	$"FF00 00FF FF00 00FE FF02 0000 FFFE 000C"
	$"FF00 FF00 00FF 00FF 0000 FF00 00FE FF02"
	$"0000 FFFD 0000 FFFC 00FD FF09 00FF 0000"
	$"FF00 00FF 0000 FDFF 0100 FFF9 0008 FF00"
	$"00FF 0000 FF00 FFF6 000B FF00 FF00 00FF"
	$"00FF 0000 FF00 FDFF 0100 FFFD 0000 FFFD"
	$"0018 FF00 00FF 00FF 0000 FF00 00FF 00FF"
	$"0000 FF00 FF00 00FF 0000 FFFE 000A FF00"
	$"FF00 00FF 00FF 0000 FFFC 00FC FF11 00FF"
	$"0000 FF00 FF00 00FF 0000 FF00 00FF 00FF"
	$"FC00 FEFF 0200 00FF FE00 0EFF 00FF 0000"
	$"FF00 FF00 00FF 0000 FFFF D500 00FF FF00"
	$"00E1 0100 FFD3 0000 FFFE 0005 FF00 00FF"
	$"00FF FA00 0CFF 0000 FFFF 00FF 0000 FF00"
	$"00FF FE00 12FF 00FF 0000 FF00 FF00 00FF"
	$"00FF 0000 FF00 00FF FD00 00FF FE00 02FF"
	$"00FF FD00 09FF 0000 FF00 00FF 0000 FFFD"
	$"0000 FFF9 0008 FF00 00FF 0000 FF00 FFFA"
	$"0000 FFFE 000C FF00 FF00 00FF 00FF 0000"
	$"FF00 FFFD 0000 FFFD 001D FF00 00FF 00FF"
	$"0000 FF00 FF00 00FF 0000 FF00 FF00 00FF"
	$"00FF 0000 FF00 00FF FE00 0AFF 00FF 0000"
	$"FF00 FF00 00FF FC00 00FF FE00 1DFF 00FF"
	$"0000 FF00 FF00 00FF 0000 FF00 00FF 00FF"
	$"0000 FF00 FF00 00FF 0000 FFFE 000A FF00"
	$"FF00 00FF 00FF 0000 FFFD 0000 FFD6 0000"
	$"FFFF 0000 D401 00FF D300 00FF FE00 07FF"
	$"0000 FF00 00FF FFFC 0000 FFFE 0002 FF00"
	$"00FE FFFE 000F FF00 00FF 0000 FFFF 0000"
	$"FF00 00FF 0000 FEFF 0200 00FF FC00 FEFF"
	$"FE00 07FF FF00 00FF 0000 FFFE 0007 FF00"
	$"00FF FF00 00FF F900 00FF FE00 04FF FF00"
	$"00FF F900 FEFF FE00 FEFF 0000 FEFF FE00"
	$"04FF FF00 00FF FC00 01FF FFFE 000B FFFF"
	$"0000 FF00 00FF 0000 FF00 FEFF FE00 FEFF"
	$"FE00 0AFF 0000 FF00 FF00 00FF 0000 FEFF"
	$"FC00 00FF FE00 01FF 00FE FF01 0000 FEFF"
	$"FE00 07FF 0000 FF00 00FF FFFE 00FE FFFE"
	$"000E FF00 00FF 0000 FFFF 0000 FF00 00FF"
	$"00FE FFD5 0000 FFFF 0000 1F01 00FF 8100"
	$"DE00 00FF E100 00FF E900 00FF F600 00FF"
	$"FD00 00FF AD00 00FF FF00 0020 0100 FF81"
	$"00DE 0000 FFE1 0000 FFEB 0001 FFFF F500"
	$"00FF FD00 00FF AD00 00FF FF00 000D 0100"
	$"FF81 0081 00BE 0000 FFFF 0000 0D01 00FF"
	$"8100 8100 BE00 00FF FF00 000D 0100 FF81"
	$"0081 00BE 0000 FFFF 0000 6101 00FF DE00"
	$"00FF FB00 03FF 0000 FFF5 0000 FFFE 0000"
	$"FFFA 0000 FFEA 0003 FF00 00FF EF00 01FF"
	$"FFFD 0009 FF00 FFFF 00FF FF00 00FF F500"
	$"00FF F200 00FF FC00 00FF FE00 00FF FB00"
	$"00FF E800 FEFF 0200 00FF E500 00FF F300"
	$"FDFF E500 00FF DB00 00FF FF00 0054 0100"
	$"FFDE 0000 FFFB 0003 FF00 00FF F500 00FF"
	$"FE00 00FF DF00 00FF F000 00FF FB00 06FF"
	$"0000 FF00 00FF E200 00FF FC00 00FF FE00"
	$"00FF FB00 00FF E900 00FF FE00 02FF 00FF"
	$"D600 00FF FE00 00FF E600 00FF DB00 00FF"
	$"FF00 00EF 0100 FFE3 0003 FFFF 0000 FEFF"
	$"FD00 FEFF 0000 FEFF FE00 01FF FFFC 0000"
	$"FFFE 0001 FF00 FEFF 0500 00FF FF00 FFFE"
	$"000C FF00 00FF FF00 00FF 00FF FF00 00FE"
	$"FF03 00FF FF00 FEFF 0400 FF00 00FF FB00"
	$"03FF FF00 00FE FFFC 000A FF00 00FF 0000"
	$"FF00 FFFF 00FE FFFE 0007 FFFF 0000 FFFF"
	$"0000 FEFF FC00 03FF FF00 00FE FFFD 0000"
	$"FFFE 0006 FF00 FF00 FFFF 00FE FFFE 0003"
	$"FFFF 0000 FEFF FE00 01FF FFFA 0000 FFFC"
	$"00FE FFFE 0003 FFFF 0000 FEFF 0400 FFFF"
	$"0000 FEFF FE00 07FF FF00 00FF FF00 00FE"
	$"FF00 00FE FFFC 0000 FFFE 0009 FF00 FF00"
	$"FFFF 0000 FFFF FE00 FEFF 0500 00FF FF00"
	$"00FE FF01 0000 FEFF 0100 00FE FFE1 0000"
	$"FFFF 0000 ED01 00FF E100 03FF 0000 FFFB"
	$"000B FF00 00FF 0000 FF00 FF00 00FF FD00"
	$"00FF FE00 16FF 00FF 0000 FF00 00FF 0000"
	$"FF00 FF00 00FF 0000 FF00 FFFF FE00 00FF"
	$"FC00 09FF 0000 FF00 00FF 0000 FFFC 0006"
	$"FF00 00FF 0000 FFFB 0018 FF00 00FF 0000"
	$"FF00 00FF 00FF 0000 FF00 FF00 00FF 0000"
	$"FF00 FFF7 0003 FF00 00FF FC00 00FF FE00"
	$"03FF 00FF FFFE 0003 FF00 00FF FD00 05FF"
	$"00FF 0000 FFFD 0001 FF00 FDFF 0100 FFFC"
	$"0003 FF00 00FF FD00 0DFF 00FF 0000 FF00"
	$"00FF 00FF 0000 FFFD 000D FF00 00FF 00FF"
	$"0000 FF00 FF00 00FF FD00 FDFF 0300 00FF"
	$"FFFE 0005 FF00 00FF 00FF FD00 0EFF 0000"
	$"FF00 FF00 00FF 0000 FF00 00FF DE00 00FF"
	$"FF00 00ED 0100 FFE3 00FE FF02 0000 FFFB"
	$"0007 FF00 00FF 0000 FF00 FDFF FD00 00FF"
	$"FE00 0FFF 00FF 0000 FF00 00FF 0000 FF00"
	$"FF00 00FD FF01 00FF FC00 01FF FFFE 0009"
	$"FF00 00FF 0000 FF00 00FF FC00 06FF 0000"
	$"FF00 00FF FB00 1AFF 0000 FF00 00FF 0000"
	$"FF00 FF00 00FF 00FF 0000 FF00 00FF 0000"
	$"FFFF FB00 FEFF 0200 00FF FC00 00FF FE00"
	$"02FF 00FF FD00 05FF 0000 FF00 00FE FF06"
	$"00FF 0000 FF00 00FE FFFB 0000 FFFC 0005"
	$"FF00 00FF 0000 FEFF 0E00 FF00 00FF 0000"
	$"FF00 FF00 00FF 0000 FEFF 0C00 00FF 00FF"
	$"0000 FF00 FF00 00FF FD00 00FF FC00 00FF"
	$"FD00 FDFF 0500 00FF FF00 00FD FF07 00FF"
	$"0000 FF00 00FF FE00 01FF FFE0 0000 FFFF"
	$"0000 EE01 00FF E400 06FF 0000 FF00 00FF"
	$"FB00 08FF 0000 FF00 00FF 00FF FA00 00FF"
	$"FE00 08FF 00FF 0000 FF00 00FF FE00 00FF"
	$"FE00 00FF FD00 00FF FA00 0CFF 0000 FF00"
	$"00FF 0000 FF00 00FF FC00 06FF 0000 FF00"
	$"00FF FB00 16FF 0000 FF00 00FF 0000 FF00"
	$"FF00 00FF 00FF 0000 FF00 00FF FD00 00FF"
	$"FD00 06FF 0000 FF00 00FF FC00 00FF FE00"
	$"02FF 00FF FD00 12FF 0000 FF00 FF00 00FF"
	$"00FF 0000 FF00 FF00 00FF FB00 00FF FE00"
	$"29FF 00FF 0000 FF00 FF00 00FF 00FF 0000"
	$"FF00 00FF 00FF 0000 FF00 FF00 00FF 0000"
	$"FF00 FF00 00FF 00FF 0000 FFFD 0000 FFFC"
	$"0000 FFFD 0000 FFFA 0002 FF00 FFFD 0006"
	$"FF00 00FF 0000 FFFC 0000 FFE1 0000 FFFF"
	$"0000 E001 00FF E300 FEFF FE00 00FF FB00"
	$"09FF 00FF 0000 FF00 00FF FFFB 00FE FF08"
	$"0000 FF00 00FF 0000 FFFE 0000 FFFD 0004"
	$"FFFF 0000 FFFD 00FE FFFE 0000 FFFE 0002"
	$"FF00 00FE FFFB 0001 FFFF FE00 00FF FB00"
	$"12FF 0000 FF00 00FF 0000 FF00 FF00 00FF"
	$"0000 FFFF FE00 01FF 00FE FFFB 00FE FFFE"
	$"0000 FFFC 00FE FF02 0000 FFFD 00FE FFFE"
	$"00FE FF06 00FF 0000 FF00 00FE FFFA 00FE"
	$"FF07 0000 FF00 00FF 0000 FEFF 0800 FF00"
	$"00FF 0000 FF00 FEFF FE00 FEFF 0400 00FF"
	$"0000 FEFF 0400 FF00 00FF FD00 00FF FC00"
	$"00FF FC00 03FF FF00 00FE FFFE 0007 FFFF"
	$"0000 FF00 00FF FE00 01FF 00FE FFE0 0000"
	$"FFFF 0000 1501 00FF 9A00 00FF 8700 00FF"
	$"F200 00FF B100 00FF FF00 0017 0100 FF9C"
	$"0001 FFFF 8600 00FF F400 01FF FFB0 0000"
	$"FFFF 0000 0D01 00FF 8100 8100 BE00 00FF"
	$"FF00 000D 0100 FF81 0081 00BE 0000 FFFF"
	$"0000 0D01 00FF 8100 8100 BE00 00FF FF00"
	$"000D 0100 FF81 0081 00BE 0000 FFFF 0000"
	$"0D01 00FF 8100 8100 BE00 00FF FF00 000D"
	$"0100 FF81 0081 00BE 0000 FFFF 0000 0D01"
	$"00FF 8100 8100 BE00 00FF FF00 000D 0100"
	$"FF81 0081 00BE 0000 FFFF 0000 0D01 00FF"
	$"8100 8100 BE00 00FF FF00 000D 0100 FF81"
	$"0081 00BE 0000 FFFF 0000 0D01 00FF 8100"
	$"8100 BE00 00FF FF00 000D 0100 FF81 0081"
	$"00BE 0000 FFFF 0000 0D01 00FF 8100 8100"
	$"BE00 00FF FF00 0028 0100 FF9E 00FD FFFE"
	$"00FD FFFD 00FB FFFD 00FC FFFC 0001 FFFF"
	$"FA00 F8FF E700 FCFF 8100 F500 00FF FF00"
	$"003B 0100 FF9D 00FE FFFD 0001 FFFF FE00"
	$"FEFF FE00 FEFF 0000 FEFF 0100 00FE FFFD"
	$"00FE FFFA 0001 FFFF FD00 FEFF EA00 FEFF"
	$"0100 00FE FF81 00F6 0000 FFFF 0000 4201"
	$"00FF 9D00 FDFF FE00 01FF FFFE 0001 FFFF"
	$"FC00 04FF FF00 FFFF FD00 01FF FFFE 00FD"
	$"FFFA 0001 FFFF FC00 FEFF F800 01FF FFF6"
	$"0001 FFFF FD00 01FF FF81 00F6 0002 FF00"
	$"FF00 3501 00FF 9D00 FDFF FE00 05FF FF00"
	$"00FF FFF8 0001 FFFF F800 FDFF FA00 01FF"
	$"FFFB 0001 FFFF F800 01FF FFF6 0001 FFFF"
	$"8100 F000 00FF FF00 004E 0100 FF9D 000C"
	$"FFFF 00FF FF00 00FF FF00 00FF FFF8 00FE"
	$"FFF9 00FC FFFB 0001 FFFF FB00 03FF FF00"
	$"00FD FF01 0000 FCFF 0100 00FD FFFE 00FE"
	$"FFFA 00FD FF01 0000 FCFF 0000 FAFF FE00"
	$"FDFF 9200 00FF FF00 0060 0100 FF9D 000C"
	$"FFFF 00FF FF00 00FF FF00 00FF FFF7 00FB"
	$"FFFE 0005 FFFF 0000 FFFF FB00 01FF FFFB"
	$"0005 FFFF 00FF FF00 FEFF 0300 00FF FFFE"
	$"0002 FFFF 00FE FFFE 00FB FF00 00FE FF00"
	$"00FC FF01 0000 FEFF 0000 FEFF 0000 FBFF"
	$"0000 FEFF 9300 00FF FF00 0058 0100 FF9D"
	$"000C FFFF 0000 FFFF 00FF FF00 00FF FFF3"
	$"00FE FF07 0000 FFFF 0000 FFFF FB00 01FF"
	$"FFFB 0001 FFFF FC00 05FF FF00 00FF FFFA"
	$"0001 FFFF FA00 FCFF FD00 01FF FFFD 0004"
	$"FFFF 00FF FFFE 00FD FFFE 0001 FFFF 9300"
	$"00FF FF00 004D 0100 FF9D 000C FFFF 0000"
	$"FFFF 00FF FF00 00FF FFF2 0003 FFFF 0000"
	$"FAFF FC00 01FF FFFB 0002 FFFF 00FB FF03"
	$"0000 FFFF FE00 FBFF F900 FDFF FD00 01FF"
	$"FFFD 0004 FFFF 00FF FFFE 00F8 FF93 0000"
	$"FFFF 0000 6101 00FF 9D00 01FF FFFE 00FD"
	$"FF01 0000 FEFF F900 01FF FFFD 0004 FFFF"
	$"00FF FFFD 0001 FFFF FC00 01FF FFFC 0004"
	$"FFFF 00FF FFFE 0009 FFFF 0000 FFFF 0000"
	$"FFFF FE00 05FF FF00 00FF FFFD 00FD FFFD"
	$"0001 FFFF FD00 04FF FF00 FFFF FE00 FDFF"
	$"8E00 02FF 00F6 006D 0100 FF9D 0001 FFFF"
	$"FE00 FDFF FE00 FEFF FD00 02FF FF00 FEFF"
	$"0100 00FE FF02 00FF FFFD 0001 FFFF FC00"
	$"01FF FFFD 00FE FF00 00FE FF00 00FE FF05"
	$"0000 FFFF 0000 FEFF 0000 FEFF 0100 00FE"
	$"FF01 0000 FBFF FE00 FEFF 0100 00FE FF07"
	$"00FF FF00 00FF FF00 FEFF 0300 00FF FF93"
	$"0002 FF00 F600 4901 00FF 9E00 FDFF FE00"
	$"FEFF FD00 FAFF FE00 FCFF 0100 00FD FF01"
	$"0000 FDFF FE00 F9FF FD00 FAFF 0100 00FE"
	$"FF00 00FA FF01 0000 FCFF FE00 FCFF 0000"
	$"FCFF FE00 FCFF FE00 FCFF 9200 02FF 00F2"
	$"0010 0100 FF81 00BA 0001 FFFF 8700 02FF"
	$"0037 0010 0100 FF81 00BA 0001 FFFF 8700"
	$"02FF 0042 0010 0100 FF81 00BA 0001 FFFF"
	$"8700 02FF 0065 000F 0100 FF81 00BB 00FD"
	$"FF88 0002 FF00 6500 0D01 00FF 8100 8100"
	$"BE00 02FF 0067 000D 0100 FF81 0081 00BE"
	$"0002 FF00 2000 0B00 0081 FF81 FFBC FF01"
	$"0063 0008 8100 8100 BA00 0064 00FF"
};

data 'NCSf' (0) {
	$"7874 4E43 5341 2044 6174 6153 636F 7065"            /* xtNCSA DataScope */
	$"2056 6572 7369 6F6E 2032 2E30 2E32 2028"            /*  Version 2.0.2 ( */
	$"5365 7074 2039 3129 2E0D 4E61 7469 6F6E"            /* Sept 91).ÂNation */
	$"616C 2043 656E 7465 7220 666F 720D 5375"            /* al Center forÂSu */
	$"7065 7263 6F6D 7075 7469 6E67 2041 7070"            /* percomputing App */
	$"6C69 6361 7469 6F6E 7353 656E 6420 6275"            /* licationsSend bu */
	$"6773 2074 6F20 6275 6773 406E 6373 612E"            /* gs to bugs@ncsa. */
	$"7569 7563 2E65 6475 0D"                             /* uiuc.eduÂ */
};

resource 'vers' (1) {
	0x2,
	0x2,
	release,
	0x0,
	verUS,
	"2.0.2",
	"NCSA DataScope 2.0.2"
};

resource 'vers' (2) {
	0x2,
	0x2,
	release,
	0x0,
	verUS,
	"2.0.2",
	"NCSA DataScope 2.0.2"
};

resource 'WIND' (129, "image window") {
	{60, 60, 360, 460},
	documentProc,
	invisible,
	goAway,
	0x0,
	"Image"
};

resource 'WIND' (128, "float window") {
	{60, 60, 360, 460},
	documentProc,
	invisible,
	goAway,
	0x0,
	"DataScope"
};

resource 'ICN#' (128, "appl icon") {
	{	/* array: 2 elements */
		/* [1] */
		$"07FF FFFF 0400 0001 043F FCFD 0400 0101"
		$"008F 323D 00C9 0441 3FE9 088D 20CF 3111"
		$"2080 0225 2400 0449 244F F891 2490 0125"
		$"2527 FE49 2448 0091 2493 FF21 2420 0001"
		$"27FF FFFF 23FF FFFF 2000 0000 2000 0000"
		$"2000 0040 2000 0060 FFFF FC70 8000 0478"
		$"B8A9 747C 88A9 447E 98BD 747F 8889 5478"
		$"BA89 746C 8000 044C FFFF FC06 7FFF FC06",
		/* [2] */
		$"07FF FFFF 07FF FFFF 07FF FFFF 07FF FFFF"
		$"03FF FFFF 7FFF FFFF 7FFF FFFF 7FFF FFFF"
		$"73FF FFFF 77FF FFFF 77FF FFFF 77FF FFFF"
		$"77FF FFFF 77FF FFFF 77FF FFFF 77FF FFFF"
		$"77FF FFFF 73FF FFFF 7000 0000 7000 0000"
		$"7000 0040 7000 0060 FFFF FC70 FFFF FC78"
		$"FFFF FC7C FFFF FC7E FFFF FC7F FFFF FC78"
		$"FFFF FC6C FFFF FC4C FFFF FC06 7FFF FC06"
	}
};

resource 'ICN#' (130, "_HDF icon") {
	{	/* array: 2 elements */
		/* [1] */
		$"01F0 0FF8 03FF FFF8 001F 8000 03FF FFF8"
		$"01F0 0FF8 0000 0000 0000 0000 1FFF FFF8"
		$"1000 0008 17FF F3E8 1000 0408 10E0 C8E8"
		$"1000 1108 11D8 2228 1203 C448 1400 0888"
		$"1000 1128 113F E248 1240 0488 149F F928"
		$"1120 0248 124F FC88 1080 0008 1FFF FFF8"
		$"0FFF FFF8 0000 0000 0000 0000 3FF6 0000"
		$"201E 0000 3FF7 FFF8 201E 0000 3FF6",
		/* [2] */
		$"0000 0000 0000 0000 0000 0000 0000 0000"
		$"0000 0000 0000 0000 0000 0000 1FFF FFF8"
		$"1FFF FFF8 1FFF FFF8 1FFF FFF8 1FFF FFF8"
		$"1FFF FFF8 1FFF FFF8 1FFF FFF8 1FFF FFF8"
		$"1FFF FFF8 1FFF FFF8 1FFF FFF8 1FFF FFF8"
		$"1FFF FFF8 1FFF FFF8 1FFF FFF8 1FFF FFF8"
		$"0FFF FFF8 0000 0000 0000 0000 3FF6 0000"
		$"3FFE 0000 3FFF FFF8 3FFE 0000 3FF6"
	}
};

resource 'ICN#' (129, "_HDF icon") {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 000E 0000 001F 07FF FFFF 1FFF FFFF"
		$"3C00 001F 7000 000E 6000 0000 DFFF FFF8"
		$"D000 0008 D0FF F3E8 9000 0408 903C C8E8"
		$"9024 1108 1124 2228 123C C448 1400 0888"
		$"1000 1128 113F E248 1240 0488 149F F928"
		$"1120 0249 124F FC89 1080 0009 1FFF FFFB"
		$"0FFF FFFB 0000 0006 7000 000E F800 003C"
		$"FFFF FFF8 FFFF FFE0 F800 0000 70",
		/* [2] */
		$"0000 000E 0000 001B 07FF FFF1 1FFF FFF1"
		$"3C00 001B 7000 000E 6000 0000 DFFF FFF8"
		$"DFFF FFF8 DFFF FFF8 9FFF FFF8 9FFF FFF8"
		$"9FFF FFF8 1FFF FFF8 1FFF FFF8 1FFF FFF8"
		$"1FFF FFF8 1FFF FFF8 1FFF FFF8 1FFF FFF8"
		$"1FFF FFF9 1FFF FFF9 1FFF FFF9 1FFF FFFB"
		$"0FFF FFFB 0000 0006 7000 000E D800 003C"
		$"8FFF FFF8 8FFF FFE0 D800 0000 70"
	}
};

resource 'CURS' (257, locked, preload) {
	$"3F00 3F00 3F00 3F00 4080 8040 8140 8260"
	$"9C60 8040 8040 4080 3F00 3F00 3F00 3F",
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F",
	{8, 8}
};

resource 'CURS' (258, locked, preload) {
	$"3F00 3F00 3F00 3F00 4080 8040 8040 8060"
	$"9F60 8040 8040 4080 3F00 3F00 3F00 3F",
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F",
	{8, 8}
};

resource 'CURS' (259, locked, preload) {
	$"3F00 3F00 3F00 3F00 4080 8040 8040 8060"
	$"9C60 8240 8040 4080 3F00 3F00 3F00 3F",
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F",
	{8, 8}
};

resource 'CURS' (260, locked, preload) {
	$"3F00 3F00 3F00 3F00 4080 8040 8040 8060"
	$"9C60 8440 8440 4080 3F00 3F00 3F00 3F",
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F",
	{8, 8}
};

resource 'CURS' (261, locked, preload) {
	$"3F00 3F00 3F00 3F00 4080 8040 8040 8060"
	$"9C60 8840 9040 4080 3F00 3F00 3F00 3F",
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F",
	{8, 8}
};

resource 'CURS' (262, locked, preload) {
	$"3F00 3F00 3F00 3F00 4080 8040 8040 8060"
	$"BC60 8040 8040 4080 3F00 3F00 3F00 3F",
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F",
	{8, 8}
};

resource 'CURS' (263, locked, preload) {
	$"3F00 3F00 3F00 3F00 4080 8040 9040 8860"
	$"9C60 8040 8040 4080 3F00 3F00 3F00 3F",
	$"3F00 3F00 3F00 3F00 7F80 FFC0 FFC0 FFC0"
	$"FFC0 FFC0 FFC0 7F80 3F00 3F00 3F00 3F",
	{8, 8}
};

resource 'ALRT' (1003, purgeable) {
	{38, 48, 194, 350},
	1003,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1001, "NotHDFfp", purgeable) {
	{34, 56, 210, 410},
	10240,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1008, "netfail", purgeable) {
	{38, 48, 268, 376},
	1008,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1002, "out of mem", purgeable) {
	{38, 48, 194, 350},
	28260,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1004, purgeable) {
	{38, 48, 210, 350},
	1034,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1005, purgeable) {
	{38, 48, 190, 354},
	1035,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1006, "non-cont", purgeable) {
	{38, 48, 268, 376},
	1036,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (1007, "polar non", purgeable) {
	{38, 48, 268, 376},
	10360,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, sound1
	}
};

resource 'ALRT' (500, "General") {
	{132, 178, 258, 498},
	500,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, silent,
		/* [2] */
		OK, visible, silent,
		/* [3] */
		OK, visible, silent,
		/* [4] */
		OK, visible, silent
	}
};

data 'MDPL' (10416) {
	$"0001 0000 0000 04B2 0000 0011 0000 000A"            /* .......²........ */
	$"0000 0000 0000 0000 0140 0000 00AA FFFE"            /* .........@...ªÿş */
	$"0000 0001 0000 0100 0000 0018 0000 0001"            /* ................ */
	$"0000 0118 0000 000C 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0004 0000 0124 0000 0048 0000 0001"            /* .......$...H.... */
	$"0000 016C 0000 000C 0000 0000 0000 0000"            /* ...l............ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0002 0000 0178 0000 0010 0000 0002"            /* .......x........ */
	$"0000 0188 0000 0012 0000 0000 0000 0000"            /* ...ˆ............ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 000B"            /* ................ */
	$"0000 019A 0000 0160 0000 000A 0000 02FA"            /* ...š...`.......ú */
	$"0000 01B8 0000 000A 0000 04B2 0000 00C3"            /* ...¸.......²...Ã */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"0000 0000 0000 0000 0000 0000 0000 0000"            /* ................ */
	$"FFFF FFFE 4022 0000 0000 0000 3FBC 71C7"            /* ÿÿÿş@"......?¼qÇ */
	$"1C71 C71C 0008 2000 FFFF FFFE 0001 0000"            /* .qÇ... .ÿÿÿş.... */
	$"0001 0001 FFFF 0013 000D 0003 1000 000C"            /* ....ÿÿ...Â...... */
	$"0000 0000 0000 FFFD 0010 000C 0003 0000"            /* ......ÿı........ */
	$"000C 0000 0000 0000 FFFF 000D 000A 0003"            /* ........ÿÿ.Â.... */
	$"0100 000A 0000 0000 0000 FFFA 000E 000A"            /* ..........ÿú.... */
	$"0015 0000 000C 0000 0000 0000 FFFF FFFC"            /* ............ÿÿÿü */
	$"FFFF FFFF FFFF FFFF FFFF 0003 0000 0188"            /* ÿÿÿÿÿÿÿÿÿÿ.....ˆ */
	$"FFFF 0015 0000 018F 0647 656E 6576 6109"            /* ÿÿ......GenevaÆ */
	$"4865 6C76 6574 6963 6100 001A 0000 0023"            /* Helvetica......# */
	$"7FFF 002D 0000 007B 7FFF 4801 0000 0000"            /* .ÿ.-...{.ÿH..... */
	$"0000 0000 02FA 0000 0000 0062 0000 0012"            /* .....ú.....b.... */
	$"0000 0072 FFFE 00AA FFFE 4801 0000 0000"            /* ...rÿş.ªÿşH..... */
	$"0000 0000 0326 0000 0000 007D 0000 0012"            /* .....&.....}.... */
	$"0000 008D FFFE 00AA FFFE 4801 0000 0000"            /* ...ÿş.ªÿşH..... */
	$"0000 0000 0352 0000 0000 0000 0000 0000"            /* .....R.......... */
	$"0000 000D 0000 005C FFFE 4801 0000 0000"            /* ...Â...\ÿşH..... */
	$"0000 0000 037E 0000 0000 00AB 0000 0012"            /* .....~.....«.... */
	$"0000 00B9 0000 006E FFFE 4801 0000 0000"            /* ...¹...nÿşH..... */
	$"0000 0000 03AA 0000 0000 0047 0000 0012"            /* .....ª.....G.... */
	$"0000 0057 FFFE 008E FFFE 4801 0000 0000"            /* ...Wÿş.ÿşH..... */
	$"0000 0000 03D6 0000 0000 00E1 0000 0012"            /* .....Ö.....á.... */
	$"0000 00EF FFFE 00A2 FFFE 4801 0000 0000"            /* ...ïÿş.¢ÿşH..... */
	$"0000 0000 0402 0000 0000 00C6 0000 0012"            /* ...........Æ.... */
	$"0000 00D4 FFFE 00A6 FFFE 4801 0000 0000"            /* ...Ôÿş.¦ÿşH..... */
	$"0000 0000 042E 0000 0000 0117 0000 0012"            /* ................ */
	$"0000 0125 0000 0094 FFFE 4801 0000 0000"            /* ...%...”ÿşH..... */
	$"0000 0000 045A 0000 0000 0132 0000 0012"            /* .....Z.....2.... */
	$"0000 0140 0000 008A FFFE 4C01 0000 0000"            /* ...@...ŠÿşL..... */
	$"0000 0000 0486 0000 0000 009A 8000 0017"            /* .....†.....š€... */
	$"8000 009B 8000 0096 8000 0802 0101 0001"            /* €..›€..–€....... */
	$"0000 0000 0000 0000 0000 0000 0001 0000"            /* ................ */
	$"002C 0000 04B2 0000 0001 000A 0001 0001"            /* .,...².......... */
	$"0000 000A 0000 0000 000B FFFF 0013 000D"            /* ..........ÿÿ...Â */
	$"0013 000D 0000 0000 0001 0000 002C 0000"            /* ...Â.........,.. */
	$"04BC 0000 0000 0017 0001 0001 0000 0017"            /* .¼.............. */
	$"0000 0001 0018 FFFF 0010 000C 0010 000C"            /* ......ÿÿ........ */
	$"0000 0000 0001 0000 002C 0000 04D3 0000"            /* .........,...Ó.. */
	$"0000 0017 0001 0001 0000 0017 0000 0001"            /* ................ */
	$"0018 FFFF 0010 000C 0010 000C 0000 0000"            /* ..ÿÿ............ */
	$"0001 0000 002C 0000 04EA 0000 0000 000E"            /* .....,...ê...... */
	$"0001 0001 0000 000E 0000 0002 000F FFFF"            /* ..............ÿÿ */
	$"000D 000A 000D 000A 0000 0000 0001 0000"            /* .Â...Â.......... */
	$"002C 0000 04F8 0000 0000 000F 0001 0001"            /* .,...ø.......... */
	$"0000 000F 0000 0003 0010 FFFF 000E 000A"            /* ..........ÿÿ.... */
	$"000E 000A 0000 0000 0001 0000 002C 0000"            /* .............,.. */
	$"0507 0000 0000 0012 0001 0001 0000 0012"            /* ................ */
	$"0000 0001 0013 FFFF 0010 000C 0010 000C"            /* ......ÿÿ........ */
	$"0000 0000 0001 0000 002C 0000 0519 0000"            /* .........,...... */
	$"0000 0019 0001 0001 0000 0019 0000 0003"            /* ................ */
	$"001A FFFF 000E 000A 000E 000A 0000 0000"            /* ..ÿÿ............ */
	$"0001 0000 002C 0000 0532 0000 0000 0019"            /* .....,...2...... */
	$"0001 0001 0000 0019 0000 0003 001A FFFF"            /* ..............ÿÿ */
	$"000E 000A 000E 000A 0000 0000 0001 0000"            /* ................ */
	$"002C 0000 054B 0000 0000 0016 0001 0001"            /* .,...K.......... */
	$"0000 0016 0000 0003 0017 FFFF 000E 000A"            /* ..........ÿÿ.... */
	$"000E 000A 0000 0000 0001 0000 002C 0000"            /* .............,.. */
	$"0561 0000 0000 0014 0001 0001 0000 0014"            /* .a.............. */
	$"0000 0003 0015 FFFF 000E 000A 000E 000A"            /* ......ÿÿ........ */
	$"000E 4174 7472 6962 7574 6573 496E 6465"            /* ..AttributesInde */
	$"7065 6E64 656E 7420 7661 722C 2078 2D61"            /* pendent var, x-a */
	$"7869 7349 6E64 6570 656E 6465 6E74 2076"            /* xisIndependent v */
	$"6172 2C20 792D 6178 6973 4E43 5341 2044"            /* ar, y-axisNCSA D */
	$"6174 6153 636F 7065 4469 7370 6C61 7920"            /* ataScopeDisplay  */
	$"466F 726D 6174 7344 6570 656E 6465 6E74"            /* FormatsDependent */
	$"2076 6172 6961 626C 654D 696E 696D 756D"            /*  variableMinimum */
	$"2076 616C 7565 206F 6620 696E 7465 7265"            /*  value of intere */
	$"7374 4D61 7869 6D75 6D20 7661 6C75 6520"            /* stMaximum value  */
	$"6F66 2069 6E74 6572 6573 7443 6F6C 6F72"            /* of interestColor */
	$"204D 696E 2C20 4D61 7820 2831 2D32 3534"            /*  Min, Max (1-254 */
	$"2944 696D 656E 7369 6F6E 7320 2872 6F77"            /* )Dimensions (row */
	$"2C63 6F6C 29"                                       /* ,col) */
};

data 'HCOD' (2000, "Help Code") {
	$"600E 0000 434F 4445 07D0 0000 0000 0000"            /* `...CODE.Ğ...... */
	$"41FA FFEE 21C8 09CE 6000 01BC 4EFA 001E"            /* Aúÿî!ÈÆÎ`..¼Nú.. */
	$"4EFA 002E 4EFA 003E 4EFA 0056 4EFA 0088"            /* Nú..Nú.>Nú.VNú.ˆ */
	$"4EFA 00A4 4EFA 00C2 4EFA 00DE 205F 3218"            /* Nú.¤Nú.ÂNú.Ş _2. */
	$"3418 B058 57C9 FFFA 4A42 67FE 4EF0 20FC"            /* 4.°XWÉÿúJBgşNğ ü */
	$"205F 3218 3418 B098 57C9 FFFA 4A42 67FE"            /*  _2.4.°˜WÉÿúJBgş */
	$"4EF0 20FA 205F 3218 3418 B042 6E0A 9041"            /* Nğ ú _2.4.°Bn.A */
	$"6D06 D040 41F0 0002 3010 67FE 4EF0 0000"            /* m.Ğ@Ağ..0.gşNğ.. */
	$"202F 0004 2F41 0004 222F 0008 2F5F 0004"            /*  /../A.."/../_.. */
	$"48E7 3C00 2400 2601 4842 C4C3 2800 2A01"            /* Hç<.$.&.HBÄÃ(.*. */
	$"4845 C8C5 D444 4842 4242 C0C1 D082 4CDF"            /* HEÈÅÔDHBBBÀÁĞ‚Lß */
	$"003C 221F 4E75 202F 0004 2F41 0004 222F"            /* .<".Nu /../A.."/ */
	$"0008 2F5F 0004 48E7 3100 4EBA 009C 4CDF"            /* ../_..Hç1.Nº.œLß */
	$"008C 221F 4E75 202F 0004 2F41 0004 222F"            /* .Œ".Nu /../A.."/ */
	$"0008 2F5F 0004 48E7 3100 4EBA 007C 2001"            /* ../_..Hç1.Nº.| . */
	$"4CDF 008C 221F 4E75 202F 0004 2F41 0004"            /* Lß.Œ".Nu /../A.. */
	$"222F 0008 2F5F 0004 48E7 3100 4EBA 002C"            /* "/../_..Hç1.Nº., */
	$"4CDF 008C 221F 4E75 202F 0004 2F41 0004"            /* Lß.Œ".Nu /../A.. */
	$"222F 0008 2F5F 0004 48E7 3100 4EBA 000C"            /* "/../_..Hç1.Nº.. */
	$"2001 4CDF 008C 221F 4E75 4A80 6A1C 4A81"            /*  .Lß.Œ".NuJ€j.J */
	$"6A0C 4480 4481 4EBA 0020 4481 4E75 4480"            /* j.D€DNº. DNuD€ */
	$"4EBA 0016 4480 4481 4E75 4A81 6A0A 4481"            /* Nº..D€DNuJj.D */
	$"4EBA 0006 4480 4E75 2E3C 0000 FFFF B280"            /* Nº..D€Nu.<..ÿÿ²€ */
	$"6306 2200 7000 4E75 B087 620C 80C1 4840"            /* c.".p.Nu°‡b.€ÁH@ */
	$"3200 4240 4840 4E75 B287 621A 2E00 4240"            /* 2.B@H@Nu²‡b...B@ */
	$"4840 80C1 4840 4847 3E00 4847 8EC1 3007"            /* H@€ÁH@HG>.HGÁ0. */
	$"4847 3207 4E75 2400 2601 E288 E289 B287"            /* HG2.Nu$.&.âˆâ‰²‡ */
	$"62F8 80C1 C087 3203 C2C0 2E03 4847 CEC0"            /* bø€ÁÀ‡2.ÂÀ..HGÎÀ */
	$"4847 D287 6508 9282 6204 4481 4E75 5340"            /* HGÒ‡e.’‚b.DNuS@ */
	$"60E4 0000 0000 4E56 FDBE 48E7 0030 426E"            /* `ä....NVı¾Hç.0Bn */
	$"FEC6 426E FEC4 2F0C 2848 A850 486E FFFC"            /* şÆBnşÄ/.(H¨PHnÿü */
	$"A874 598F 3F3C 1F40 42A7 4878 FFFF A97C"            /* ¨tY?<.@B§Hxÿÿ©| */
	$"205F 2648 B6FC 0000 6606 285F 6000 0324"            /*  _&H¶ü..f.(_`..$ */
	$"2F0B A873 2F0B 3F3C 0002 486E FFFA 486E"            /* /.¨s/.?<..HnÿúHn */
	$"FFF6 486E FFEE A98D 2F0B 3F3C 0002 3F2E"            /* ÿöHnÿî©/.?<..?. */
	$"FFFA 487A 030E 486E FFEE A98E 41EE FFDE"            /* ÿúHz..Hnÿî©AîÿŞ */
	$"43EE FFEE 20D9 20D9 486E FFEE 3F3C 0004"            /* Cîÿî Ù ÙHnÿî?<.. */
	$"3F3C 0004 A8A9 066E FFF1 FFF4 486E FFEA"            /* ?<..¨©.nÿñÿôHnÿê */
	$"4267 4267 A880 486E FFD2 4267 4267 3F3C"            /* BgBg¨€HnÿÒBgBg?< */
	$"0001 4267 A8A7 598F 486E FFEE 486E FFD2"            /* ..Bg¨§YHnÿîHnÿÒ */
	$"2F2E FFEA 4267 2F0B 4267 4267 4267 1F3C"            /* /.ÿêBg/.BgBgBg.< */
	$"0001 4EBA 0F6E 205F 2D48 FFDA 206E FFDA"            /* ..Nº.n _-HÿÚ nÿÚ */
	$"2050 0028 0080 0024 426E FFEC 426E FFEA"            /*  P.(.€.$BnÿìBnÿê */
	$"7000 6048 2F2E FFE6 486E FEC2 486E FEBE"            /* p.`H/.ÿæHnşÂHnş¾ */
	$"486E FDBE A9A8 558F 3F3C 0001 3F2E FFEA"            /* Hnı¾©¨U?<..?.ÿê */
	$"2F2E FFDA 4EBA 0F0E 301F 7000 102E FDBE"            /* /.ÿÚNº..0.p...ı¾ */
	$"486E FDBF 3F00 2F2E FFEA 2F2E FFDA 4EBA"            /* Hnı¿?./.ÿê/.ÿÚNº */
	$"0F1C 2F2E FFE6 A9A3 526E FFEA 598F 4879"            /* ../.ÿæ©£RnÿêYHy */
	$"4854 5854 3F2E FFEA A9A0 205F 2D48 FFE6"            /* HTXT?.ÿê©  _-Hÿæ */
	$"2008 671A 558F 2F2E FFE6 A9A4 301F 558F"            /*  .g.U/.ÿæ©¤0.U */
	$"3D40 FEC2 A994 301F B06E FEC2 6786 2F0B"            /* =@şÂ©”0.°nşÂg†/. */
	$"486E FFDA A918 598F 4879 4846 4E54 4267"            /* HnÿÚ©.YHyHFNTBg */
	$"A9A0 205F 2D48 FEC8 B0FC 0000 6748 2F2E"            /* ©  _-HşÈ°ü..gH/. */
	$"FEC8 4EBA 0F02 206E FEC8 2D50 FECC 206E"            /* şÈNº.. nşÈ-PşÌ n */
	$"FECC 4868 0002 486E FEC6 A900 558F 3F2E"            /* şÌHh..HnşÆ©.U?. */
	$"FEC6 206E FECC 3F10 A902 101F 4A00 6708"            /* şÆ nşÌ?.©...J.g. */
	$"206E FECC 3D50 FEC4 2F2E FEC8 4EBA 0ED2"            /*  nşÌ=PşÄ/.şÈNº.Ò */
	$"2F2E FEC8 A9A3 486E FFEA 4267 4267 A880"            /* /.şÈ©£HnÿêBgBg¨€ */
	$"2F0B A873 2F0B A915 2F0B A91F 4267 4879"            /* /.¨s/.©./.©.BgHy */
	$"4849 4E46 4EBA 030E 5C8F 3F3C 0004 2F0B"            /* HINFNº..\?<../. */
	$"4EBA 04E6 5C8F 1F3C 0001 2F2E FFEA 2F2E"            /* Nº.æ\.<../.ÿê/. */
	$"FFDA 4EBA 0E3A 487A 01E2 486E FED0 A991"            /* ÿÚNº.:Hz.âHnşĞ©‘ */
	$"0C6E 0004 FED0 6652 486E FFEA 4267 4267"            /* .n..şĞfRHnÿêBgBg */
	$"A880 558F 1F3C 0001 486E FFEA 2F2E FFDA"            /* ¨€U.<..Hnÿê/.ÿÚ */
	$"4EBA 0DFC 101F 4A00 6606 422E FED2 6020"            /* NºÂü..J.f.B.şÒ`  */
	$"3D7C 00FF FEC2 486E FED3 486E FEC2 2F2E"            /* =|.ÿşÂHnşÓHnşÂ/. */
	$"FFEA 2F2E FFDA 4EBA 0DD4 1D6E FEC3 FED2"            /* ÿê/.ÿÚNºÂÔ.nşÃşÒ */
	$"7000 102E FED2 4A40 679C 2F0B A916 4267"            /* p...şÒJ@gœ/.©.Bg */
	$"2F2E FFEA 2F2E FFDA 4EBA 0DC4 0C6E 0004"            /* /.ÿê/.ÿÚNºÂÄ.n.. */
	$"FED0 6600 00CA 598F 3F3C 1F41 42A7 4878"            /* şĞf..ÊY?<.AB§Hx */
	$"FFFF A97C 205F 2448 B4FC 0000 6700 00B0"            /* ÿÿ©| _$H´ü..g..° */
	$"2F0A A873 3F2E FEC4 3F2E FEC6 486E FED2"            /* /.¨s?.şÄ?.şÆHnşÒ */
	$"2F0A 4EBA 02A8 4FEF 000C 4EBA FB98 0001"            /* /.Nº.¨Oï..Nºû˜.. */
	$"0004 000A 0008 000E 0014 0034 2F0A A983"            /* ...........4/.©ƒ */
	$"6000 007C 2F0A A983 6000 FEF6 526E FFEA"            /* `..|/.©ƒ`.şöRnÿê */
	$"206E FFDA 558F 2F2E FFEA 2050 4868 0048"            /*  nÿÚU/.ÿê PHh.H */
	$"A8AD 101F 4A00 6632 426E FFEA 602C 536E"            /* ¨­..J.f2Bnÿê`,Sn */
	$"FFEA 206E FFDA 558F 2F2E FFEA 2050 4868"            /* ÿê nÿÚU/.ÿê PHh */
	$"0048 A8AD 101F 4A00 6610 206E FFDA 2050"            /* .H¨­..J.f. nÿÚ P */
	$"3028 004C 5340 3D40 FFEA 3D7C 00FF FEC2"            /* 0(.LS@=@ÿê=|.ÿşÂ */
	$"486E FED3 486E FEC2 2F2E FFEA 2F2E FFDA"            /* HnşÓHnşÂ/.ÿê/.ÿÚ */
	$"4EBA 0CEA 1D6E FEC3 FED2 6000 FF58 2F2E"            /* Nº.ê.nşÃşÒ`.ÿX/. */
	$"FFDA 4EBA 0CD0 2F0B A983 2F2E FFFC A873"            /* ÿÚNº.Ğ/.©ƒ/.ÿü¨s */
	$"285F 4CDF 0C00 4E5E 4E75 4D41 494E 2020"            /* (_Lß..N^NuMAIN   */
	$"2020 4E56 FFF8 598F 2F2E 000A A917 201F"            /*   NVÿøY/...©. . */
	$"2D40 FFF8 206E FFF8 41E8 000C 2F20 2F20"            /* -@ÿø nÿøAè../ /  */
	$"4EBA 0882 508F 1F3C 0001 206E FFF8 2F10"            /* Nº.‚P.<.. nÿø/. */
	$"4EBA 0C84 486E FFFC 4267 4267 A880 2F2E"            /* Nº.„HnÿüBgBg¨€/. */
	$"FFFC 206E FFF8 2F10 4EBA 0C6E 558F 4267"            /* ÿü nÿø/.Nº.nUBg */
	$"1F3C 0001 486E FFFC 206E FFF8 2F10 4EBA"            /* .<..Hnÿü nÿø/.Nº */
	$"0C64 101F 4A00 66D6 4E5E 205F 4FEF 0006"            /* .d..J.fÖN^ _Oï.. */
	$"4ED0 4452 4157 4C49 5354 4E56 FFF6 598F"            /* NĞDRAWLISTNVÿöY */
	$"2F2E 0010 A917 201F 2D40 FFF8 206E 000C"            /* /...©. .-@ÿø n.. */
	$"3010 4EBA FA48 0004 0012 0008 0046 0006"            /* 0.NºúH.......F.. */
	$"006A 0001 00A0 0003 00C6 206E 000C 0828"            /* .j... ...Æ n...( */
	$"0000 000F 671C 206E 000C 2F28 0002 A873"            /* ....g. n../(..¨s */
	$"1F3C 0001 206E FFF8 2F10 4EBA 0BD4 6000"            /* .<.. nÿø/.Nº.Ô`. */
	$"009E 4267 206E FFF8 2F10 4EBA 0BC4 6000"            /* .Bg nÿø/.Nº.Ä`. */
	$"008E 206E 000C 2068 0002 0C68 0002 006C"            /* . n.. h...h...l */
	$"6600 007C 206E 0010 2F28 0018 206E FFF8"            /* f..| n../(.. nÿø */
	$"2F10 4EBA 0BCE 6000 0066 206E 000C 2D68"            /* /.Nº.Î`..f n..-h */
	$"000A FFFC 486E FFFC A871 558F 2F2E FFFC"            /* ..ÿüHnÿü¨qU/.ÿü */
	$"206E 000C 3F28 000E 206E FFF8 2F10 4EBA"            /*  n..?(.. nÿø/.Nº */
	$"0B7C 101F 4A00 6736 206E 0008 7004 3080"            /* .|..J.g6 n..p.0€ */
	$"7001 602C 206E 000C 707F C0A8 0002 3D40"            /* p.`, n..p.À¨..=@ */
	$"FFF6 0C6E 000D FFF6 6708 0C6E 0003 FFF6"            /* ÿö.n.Âÿög..n..ÿö */
	$"660C 206E 0008 7004 3080 7001 6002 7000"            /* f. n..p.0€p.`.p. */
	$"4E5E 205F 4FEF 000C 1E80 4ED0 4D59 4649"            /* N^ _Oï...€NĞMYFI */
	$"4C54 4552 4E56 FEF8 598F 2F2E 0008 3F2E"            /* LTERNVşøY/...?. */
	$"000C A9A0 205F 2D48 FEFC B0FC 0000 6740"            /* ..©  _-Hşü°ü..g@ */
	$"598F 2F2E FEFC A9A5 201F 2D40 FEF8 0C80"            /* Y/.şü©¥ .-@şø.€ */
	$"0000 00FF 6E24 206E FEFC 2F10 486E FF01"            /* ...ÿn$ nşü/.Hnÿ. */
	$"2F2E FEF8 4EBA 0B64 1D6E FEFB FF00 486E"            /* /.şøNº.d.nşûÿ.Hn */
	$"FF00 42A7 42A7 42A7 A98B 2F2E FEFC A9A3"            /* ÿ.B§B§B§©‹/.şü©£ */
	$"4E5E 4E75 5253 5243 544F 5041 4E56 FFE8"            /* N^NuRSRCTOPANVÿè */
	$"2F2E 000C 42A7 42A7 42A7 A98B 598F 4879"            /* /...B§B§B§©‹YHy */
	$"4854 5854 2F2E 000C A9A1 205F 2D48 FFFC"            /* HTXT/...©¡ _-Hÿü */
	$"4AAE FFFC 6606 70FF 6000 009C 2F2E FFFC"            /* J®ÿüf.pÿ`..œ/.ÿü */
	$"A992 2F2E 0008 3F3C 0006 486E FFFA 486E"            /* ©’/...?<..HnÿúHn */
	$"FFF6 486E FFEC A98D 2F2E 0008 3F3C 0006"            /* ÿöHnÿì©/...?<.. */
	$"3F2E FFFA 487A 007C 486E FFEC A98E 3F2E"            /* ?.ÿúHz.|Hnÿì©?. */
	$"0012 3F2E 0010 2F2E FFFC 486E FFEC 2F2E"            /* ..?.../.ÿüHnÿì/. */
	$"0008 4EBA 015E 4FEF 0010 2D40 FFE8 4AAE"            /* ..Nº.^Oï..-@ÿèJ® */
	$"FFE8 6736 2F2E 0008 2F2E FFE8 A918 3F3C"            /* ÿèg6/.../.ÿè©.?< */
	$"0001 2F2E 0008 4EBA 00E0 5C8F 206E 0008"            /* ../...Nº.à\ n.. */
	$"4868 0010 A928 487A 0052 486E FFF4 A991"            /* Hh..©(Hz.RHnÿô©‘ */
	$"2F2E FFE8 4EBA 0362 588F 2F2E FFFC 4EBA"            /* /.ÿèNº.bX/.ÿüNº */
	$"0A48 302E FFF4 4E5E 4E75 4954 454D 4845"            /* .H0.ÿôN^NuITEMHE */
	$"4C50 4E56 0000 598F 2F2E 000A A917 201F"            /* LPNV..Y/...©. . */
	$"2F00 4EBA 03A6 588F 4E5E 205F 4FEF 0006"            /* /.Nº.¦XN^ _Oï.. */
	$"4ED0 4D59 4452 4157 5445 4E56 FFFE 206E"            /* NĞMYDRAWTENVÿş n */
	$"000C 3010 4EBA F7E6 0004 0058 0008 0054"            /* ..0.Nº÷æ...X...T */
	$"0006 000A 0001 0022 0003 0048 598F 2F2E"            /* ......."...HY/. */
	$"0010 A917 201F 206E 000C 2F28 000A 2F00"            /* ..©. . n../(../. */
	$"4EBA 0464 508F 602A 206E 000C 707F C0A8"            /* Nº.dP`* n..p.À¨ */
	$"0002 3D40 FFFE 0C6E 000D FFFE 6708 0C6E"            /* ..=@ÿş.n.Âÿşg..n */
	$"0003 FFFE 660C 206E 0008 7001 3080 7001"            /* ..ÿşf. n..p.0€p. */
	$"6002 7000 4E5E 205F 4FEF 000C 1E80 4ED0"            /* `.p.N^ _Oï...€NĞ */
	$"4845 4C50 4649 4C54 4E56 FFE0 2F2E 0008"            /* HELPFILTNVÿà/... */
	$"3F2E 000C 486E FFFA 486E FFFC 486E FFF2"            /* ?...HnÿúHnÿüHnÿò */
	$"A98D 486E FFE0 A898 A89E 3F3C 0003 3F3C"            /* ©Hnÿà¨˜¨?<..?< */
	$"0003 A89B 486E FFF2 3F3C FFFC 3F3C FFFC"            /* ..¨›Hnÿò?<ÿü?<ÿü */
	$"A8A9 486E FFF2 3F3C 0010 3F3C 0010 A8B0"            /* ¨©Hnÿò?<..?<..¨° */
	$"486E FFE0 A899 4E5E 4E75 454D 424F 4C44"            /* Hnÿà¨™N^NuEMBOLD */
	$"454E 4E56 FFCC 486E FFCE A874 2F2E 0008"            /* ENNVÿÌHnÿÎ¨t/... */
	$"A873 598F 4878 0354 4EBA 0908 205F 2D48"            /* ¨sYHx.TNºÆ. _-H */
	$"FFE2 4AAE FFE2 6606 7000 6000 0210 206E"            /* ÿâJ®ÿâf.p.`... n */
	$"000C 43EE FFF8 22D8 22D8 486E FFF8 3F3C"            /* ..Cîÿø"Ø"ØHnÿø?< */
	$"0004 3F3C 0004 A8A9 3F2E 0014 A887 3F2E"            /* ..?<..¨©?...¨‡?. */
	$"0016 A88A 486E FFD2 A88B 4267 A887 4267"            /* ..¨ŠHnÿÒ¨‹Bg¨‡Bg */
	$"A88A 302E FFD2 D06E FFD4 5640 3D40 FFDE"            /* ¨Š0.ÿÒĞnÿÔV@=@ÿŞ */
	$"302E FFFC 906E FFF8 48C0 81EE FFDE 3D40"            /* 0.ÿünÿøHÀîÿŞ=@ */
	$"FFE0 41EE FFF0 43EE FFF8 20D9 20D9 302E"            /* ÿàAîÿğCîÿø Ù Ù0. */
	$"FFF6 0640 FFF0 3D40 FFF2 302E FFE0 C1EE"            /* ÿö.@ÿğ=@ÿò0.ÿàÁî */
	$"FFDE D06E FFF8 3D40 FFFC 066E FFF0 FFFE"            /* ÿŞĞnÿø=@ÿü.nÿğÿş */
	$"206E 000C 226E FFE2 22D8 22D8 206E FFE2"            /*  n.."nÿâ"Ø"Ø nÿâ */
	$"316E FFE0 0022 206E FFE2 5088 43EE FFF8"            /* 1nÿà." nÿâPˆCîÿø */
	$"20D9 20D9 206E FFE2 216E 0010 0010 206E"            /*  Ù Ù nÿâ!n.... n */
	$"FFE2 316E FFDE 001C 206E FFE2 316E FFD2"            /* ÿâ1nÿŞ.. nÿâ1nÿÒ */
	$"001E 206E FFE2 316E FFD4 0020 206E FFE2"            /* .. nÿâ1nÿÔ.  nÿâ */
	$"7004 3140 0014 206E FFE2 4268 0016 598F"            /* p.1@.. nÿâBh..Y */
	$"A8D8 205F 226E FFE2 2348 0024 598F A8D8"            /* ¨Ø _"nÿâ#H.$Y¨Ø */
	$"205F 226E FFE2 2348 0028 598F A8D8 205F"            /*  _"nÿâ#H.(Y¨Ø _ */
	$"226E FFE2 2348 002C 206E FFE2 316E 0014"            /* "nÿâ#H., nÿâ1n.. */
	$"0030 206E FFE2 316E 0016 0032 206E FFE2"            /* .0 nÿâ1n...2 nÿâ */
	$"2F28 0028 486E FFF8 A8DF 2F2E 0010 4EBA"            /* /(.(Hnÿø¨ß/...Nº */
	$"07F6 426E FFEE 206E 0010 2D50 FFEA 598F"            /* .öBnÿî n..-PÿêY */
	$"2F2E 0010 4EBA 07CC 201F D0AE FFEA 2D40"            /* /...Nº.Ì .Ğ®ÿê-@ */
	$"FFE6 6000 005C 302E FFEE EA40 48C0 E580"            /* ÿæ`..\0.ÿîê@HÀå€ */
	$"D0AE FFE2 206E 0010 222E FFEA 9290 2040"            /* Ğ®ÿâ n..".ÿê’ @ */
	$"2141 0034 426E FFCC 601C 202E FFEA B0AE"            /* !A.4BnÿÌ`. .ÿê°® */
	$"FFE6 640E 206E FFEA 52AE FFEA 0C10 000D"            /* ÿæd. nÿêR®ÿê...Â */
	$"66E8 526E FFCC 202E FFEA B0AE FFE6 6408"            /* fèRnÿÌ .ÿê°®ÿæd. */
	$"0C6E 0020 FFCC 6DD2 302E FFCC D16E FFEE"            /* .n. ÿÌmÒ0.ÿÌÑnÿî */
	$"202E FFEA B0AE FFE6 659C 2F2E 0010 4EBA"            /*  .ÿê°®ÿæeœ/...Nº */
	$"0770 302E FFE0 916E FFEE 4A6E FFEE 6C04"            /* .p0.ÿà‘nÿîJnÿîl. */
	$"426E FFEE 598F 2F2E 0008 486E FFF0 487A"            /* BnÿîY/...HnÿğHz */
	$"076A 1F3C 0001 4267 4267 3F2E FFEE 3F3C"            /* .j.<..BgBg?.ÿî?< */
	$"0010 2F2E FFE2 A954 205F 226E FFE2 2348"            /* ../.ÿâ©T _"nÿâ#H */
	$"0018 2F2E FFCE A873 202E FFE2 4E5E 4E75"            /* ../.ÿÎ¨s .ÿâN^Nu */
	$"5445 5854 5649 4557 4E56 0000 206E 0008"            /* TEXTVIEWNV.. n.. */
	$"2F28 0018 A955 206E 0008 2F28 0024 A8D9"            /* /(..©U n../(.$¨Ù */
	$"206E 0008 2F28 0028 A8D9 206E 0008 2F28"            /*  n../(.(¨Ù n../( */
	$"002C A8D9 2F2E 0008 4EBA 06B4 4E5E 4E75"            /* .,¨Ù/...Nº.´N^Nu */
	$"5445 5854 5649 4557 4E56 0000 2F0B 266E"            /* TEXTVIEWNV../.&n */
	$"0008 4A6E 000C 670A 2F2B 0018 4267 A95D"            /* ..Jn..g./+..Bg©] */
	$"600A 2F2B 0018 3F3C 00FF A95D 265F 4E5E"            /* `./+..?<.ÿ©]&_N^ */
	$"4E75 5445 5854 5649 4557 4E56 0000 2F0B"            /* NuTEXTVIEWNV../. */
	$"266E 0008 41EB 0008 2F20 2F20 4EBA 0216"            /* &n..Aë../ / Nº.. */
	$"508F 2F2B 0028 2F0B 4EBA 037C 508F 265F"            /* P/+.(/.Nº.|P&_ */
	$"4E5E 4E75 5445 5854 5649 4557 4E56 FFE8"            /* N^NuTEXTVIEWNVÿè */
	$"48E7 0030 266E 0008 4A6E 000C 6700 00B8"            /* Hç.0&n..Jn..g..¸ */
	$"558F 2F2B 0018 A960 301F 3D40 FFEA 4A6E"            /* U/+..©`0.=@ÿêJn */
	$"000C 6C36 558F 2F2B 0018 A961 301F 3D40"            /* ..l6U/+..©a0.=@ */
	$"FFE8 302E FFE8 B06E FFEA 6700 008A 302E"            /* ÿè0.ÿè°nÿêg..Š0. */
	$"FFEA D06E 000C B06E FFE8 6C40 302E FFE8"            /* ÿêĞn..°nÿèl@0.ÿè */
	$"906E FFEA 3D40 000C 6032 558F 2F2B 0018"            /* nÿê=@..`2U/+.. */
	$"A962 301F 3D40 FFE8 302E FFE8 B06E FFEA"            /* ©b0.=@ÿè0.ÿè°nÿê */
	$"6754 302E FFEA D06E 000C B06E FFE8 6F0C"            /* gT0.ÿêĞn..°nÿèo. */
	$"302E FFE8 906E FFEA 3D40 000C 302E 000C"            /* 0.ÿènÿê=@..0... */
	$"4440 C1EB 001C 486B 0008 4267 3F00 2F2B"            /* D@Áë..Hk..Bg?./+ */
	$"002C A8EF 558F 2F2B 0018 A960 301F D06E"            /* .,¨ïU/+..©`0.Ğn */
	$"000C 2F2B 0018 3F00 A963 2F2B 002C 2F0B"            /* ../+..?.©c/+.,/. */
	$"4EBA 02A4 508F 4CDF 0C00 4E5E 4E75 5445"            /* Nº.¤PLß..N^NuTE */
	$"5854 5649 4557 4E56 FFFC 48E7 0110 266E"            /* XTVIEWNVÿüHç..&n */
	$"0008 486E 000C A871 558F 2F2B 0018 2F2E"            /* ..Hn..¨qU/+../. */
	$"000C A966 301F 3E00 3007 4EBA F330 0006"            /* ..©f0.>.0.Nºó0.. */
	$"001A 0000 001C 0014 005A 0015 0096 0016"            /* .........Z...–.. */
	$"00A6 0017 00B6 0081 00CA 7000 6000 00C6"            /* .¦...¶..Êp.`..Æ */
	$"2F2B 0018 3F07 A95D 3F3C FFFF 2F0B 4EBA"            /* /+..?.©]?<ÿÿ/.Nº */
	$"FECC 5C8F 486E 000C A972 558F A974 101F"            /* şÌ\Hn..©rU©t.. */
	$"4A00 6712 558F 2F2B 0018 2F2E 000C A966"            /* J.g.U/+../...©f */
	$"301F B047 67D2 2F2B 0018 4267 A95D 6000"            /* 0.°GgÒ/+..Bg©]`. */
	$"0082 2F2B 0018 3F07 A95D 3F3C 0001 2F0B"            /* .‚/+..?.©]?<../. */
	$"4EBA FE8A 5C8F 486E 000C A972 558F A974"            /* NºşŠ\Hn..©rU©t */
	$"101F 4A00 6712 558F 2F2B 0018 2F2E 000C"            /* ..J.g.U/+../... */
	$"A966 301F B047 67D2 2F2B 0018 4267 A95D"            /* ©f0.°GgÒ/+..Bg©] */
	$"6040 558F 2F2B 0018 2F2E 000C 487A 00CC"            /* `@U/+../...Hz.Ì */
	$"A968 301F 602C 558F 2F2B 0018 2F2E 000C"            /* ©h0.`,U/+../... */
	$"487A 012E A968 301F 6018 558F 2F2B 0018"            /* Hz..©h0.`.U/+.. */
	$"2F2E 000C 42A7 A968 301F 2F0B 4EBA FDEC"            /* /...B§©h0./.Nºıì */
	$"588F 7001 4CDF 0880 4E5E 4E75 5445 5854"            /* Xp.Lß.€N^NuTEXT */
	$"5649 4557 4E56 0000 486E 0008 A8A1 486E"            /* VIEWNV..Hn..¨¡Hn */
	$"0008 3F3C 0001 3F3C 0001 A8A9 486E 0008"            /* ..?<..?<..¨©Hn.. */
	$"A8A1 486E 0008 3F3C 0002 3F3C 0002 A8A9"            /* ¨¡Hn..?<..?<..¨© */
	$"486E 0008 A8A1 4E5E 4E75 5445 5854 5649"            /* Hn..¨¡N^NuTEXTVI */
	$"4557 4E56 0000 206E 0008 316E 000C 0014"            /* EWNV.. n..1n.... */
	$"2F2E 0008 4EBA FD84 588F 4E5E 4E75 5445"            /* /...Nºı„XN^NuTE */
	$"5854 5649 4557 4E56 0000 206E 0008 316E"            /* XTVIEWNV.. n..1n */
	$"000C 0016 2F2E 0008 4EBA FD60 588F 4E5E"            /* ..../...Nºı`XN^ */
	$"4E75 5445 5854 5649 4557 4E56 FFFA 598F"            /* NuTEXTVIEWNVÿúY */
	$"2F2E 000A A95A 201F 2D40 FFFC 0C6E 0016"            /* /...©Z .-@ÿü.n.. */
	$"0008 664A 558F 2F2E 000A A960 301F 558F"            /* ..fJU/...©`0.U */
	$"2F2E 000A 3D40 FFFA A961 301F B06E FFFA"            /* /...=@ÿú©a0.°nÿú */
	$"672C 558F 2F2E 000A A960 301F 206E FFFC"            /* g,U/...©`0. nÿü */
	$"9068 0022 2F2E 000A 3F00 A963 206E FFFC"            /* h."/...?.©c nÿü */
	$"2F28 0028 2F2E FFFC 4EBA 008C 508F 4E5E"            /* /(.(/.ÿüNº.ŒPN^ */
	$"205F 4FEF 0006 4ED0 5041 4745 5550 2020"            /*  _Oï..NĞPAGEUP   */
	$"4E56 FFFA 598F 2F2E 000A A95A 201F 2D40"            /* NVÿúY/...©Z .-@ */
	$"FFFC 0C6E 0017 0008 664A 558F 2F2E 000A"            /* ÿü.n....fJU/... */
	$"A960 301F 558F 2F2E 000A 3D40 FFFA A962"            /* ©`0.U/...=@ÿú©b */
	$"301F B06E FFFA 672C 558F 2F2E 000A A960"            /* 0.°nÿúg,U/...©` */
	$"301F 206E FFFC D068 0022 2F2E 000A 3F00"            /* 0. nÿüĞh."/...?. */
	$"A963 206E FFFC 2F28 0028 2F2E FFFC 4EBA"            /* ©c nÿü/(.(/.ÿüNº */
	$"0016 508F 4E5E 205F 4FEF 0006 4ED0 5041"            /* ..PN^ _Oï..NĞPA */
	$"4745 444F 574E 4E56 FFF8 48E7 0330 266E"            /* GEDOWNNVÿøHç.0&n */
	$"0008 3F2B 0030 A887 3F2B 0032 A88A 558F"            /* ..?+.0¨‡?+.2¨ŠU */
	$"2F2B 0018 A960 301F 3C00 70E0 C046 3E00"            /* /+..©`0.<.pàÀF>. */
	$"2F2B 0010 4EBA 02D0 3006 EA40 48C0 E580"            /* /+..Nº.Ğ0.ê@HÀå€ */
	$"D08B 2040 226B 0010 2011 D0A8 0034 2440"            /* Ğ‹ @"k.. .Ğ¨.4$@ */
	$"598F 2F2B 0010 4EBA 029A 201F 206B 0010"            /* Y/+..Nº.š . k.. */
	$"D090 2D40 FFF8 BE46 6D12 6016 41D2 528A"            /* Ğ-@ÿø¾Fm.`.AÒRŠ */
	$"0C10 000D 6606 5247 BE46 6706 B5EE FFF8"            /* ...Âf.RG¾Fg.µîÿø */
	$"65EA 2D4A FFFC 7E00 2F2B 0024 A87A 2F2E"            /* eê-Jÿü~./+.$¨z/. */
	$"000C A879 486B 0008 A8A3 6052 206E FFFC"            /* ..¨yHk..¨£`R nÿü */
	$"52AE FFFC 0C10 000D 6644 3007 5247 B06B"            /* R®ÿü...ÂfD0.RG°k */
	$"0022 6C44 302B 0008 D06B 001E 3207 5341"            /* ."lD0+..Ğk..2.SA */
	$"C3EB 001C D041 322B 000A 5841 3F01 5840"            /* Ãë..ĞA2+..XA?.X@ */
	$"3F00 A893 202E FFFC 908A 5340 3F00 2F0A"            /* ?.¨“ .ÿüŠS@?./. */
	$"2F0B 4EBA 0072 4FEF 000A 246E FFFC 202E"            /* /.Nº.rOï..$nÿü . */
	$"FFFC B0AE FFF8 65A4 B5EE FFFC 6732 302B"            /* ÿü°®ÿøe¤µîÿüg20+ */
	$"0008 D06B 001E 322B 001C C3C7 D041 322B"            /* ..Ğk..2+..ÃÇĞA2+ */
	$"000A 5841 3F01 5840 3F00 A893 202E FFFC"            /* ..XA?.X@?.¨“ .ÿü */
	$"908A 3F00 2F0A 2F0B 4EBA 002C 4FEF 000A"            /* Š?././.Nº.,Oï.. */
	$"2F2B 0024 A879 2F2B 0010 4EBA 01D4 4267"            /* /+.$¨y/+..Nº.ÔBg */
	$"A88A 4267 A887 4CDF 0CC0 4E5E 4E75 4452"            /* ¨ŠBg¨‡Lß.ÀN^NuDR */
	$"4157 2020 2020 4E56 FDF2 48E7 0130 266E"            /* AW    NVıòHç.0&n */
	$"0008 246E 000C 3E2E 0010 3F2B 0016 4267"            /* ..$n..>...?+..Bg */
	$"A894 486E FDF8 A89A 41EE FE00 2D48 FDFC"            /* ¨”Hnıø¨šAîş.-Hıü */
	$"6000 00C2 7000 1012 0C40 0020 650E 206E"            /* `..Âp....@. e. n */
	$"FDFC 52AE FDFC 1092 6000 00A8 41EE FE00"            /* ıüR®ıü.’`..¨Aîş. */
	$"B1EE FDFC 641E 202E FDFC 908E 0680 0000"            /* ±îıüd. .ıü.€.. */
	$"0200 486E FE00 4267 3F00 A885 41EE FE00"            /* ..Hnş.Bg?.¨…Aîş. */
	$"2D48 FDFC 7000 1012 4EBA EF0A 0001 0009"            /* -Hıüp...Nºï....Æ */
	$"0072 004A 0064 0054 0066 0068 0066 0064"            /* .r.J.d.T.f.h.f.d */
	$"0062 0002 486E FDF4 A89A 302E FDF6 906E"            /* .b..Hnıô¨š0.ıön */
	$"FDFA 3D40 FDF2 306E FDF2 2008 81FC 0024"            /* ıú=@ıò0nıò .ü.$ */
	$"5240 C1FC 0024 3D40 FDF2 302E FDFA D06E"            /* R@Áü.$=@ıò0.ıúĞn */
	$"FDF2 3F00 3F2E FDF8 A893 6026 528A 7000"            /* ıò?.?.ıø¨“`&RŠp. */
	$"1012 3F00 A888 5347 6018 528A 7000 1012"            /* ..?.¨ˆSG`.RŠp... */
	$"3F00 A887 5347 600A 4267 A888 6004 4267"            /* ?.¨‡SG`.Bg¨ˆ`.Bg */
	$"A887 528A 3007 5347 4A40 6E00 FF38 41EE"            /* ¨‡RŠ0.SGJ@n.ÿ8Aî */
	$"FE00 B1EE FDFC 6416 202E FDFC 908E 0680"            /* ş.±îıüd. .ıü.€ */
	$"0000 0200 486E FE00 4267 3F00 A885 4CDF"            /* ....Hnş.Bg?.¨…Lß */
	$"0C80 4E5E 4E75 4452 4157 5445 5854 201F"            /* .€N^NuDRAWTEXT . */
	$"41FA 0010 9048 E348 225F 3F00 2F09 ADE7"            /* Aú..HãH"_?./Æ­ç */
	$"61EC 61EA 61E8 61E6 61E4 61E2 61E0 61DE"            /* aìaêaèaæaäaâaàaŞ */
	$"61DC 61DA 61D8 61D6 61D4 61D2 61D0 61CE"            /* aÜaÚaØaÖaÔaÒaĞaÎ */
	$"61CC 61CA 61C8 61C6 61C4 61C2 61C0 61BE"            /* aÌaÊaÈaÆaÄaÂaÀa¾ */
	$"61BC 61BA 7000 2F09 31C0 0220 4E75 7000"            /* a¼aºp./Æ1À. Nup. */
	$"60F6 225F 201F A11E 2E88 4EFA FFEA 225F"            /* `ö"_ .¡..ˆNúÿê"_ */
	$"205F A01F 4EFA FFE0 225F 205F A023 4EFA"            /*  _ .Núÿà"_ _ #Nú */
	$"FFD6 225F 205F A025 2E80 6A06 4297 4EFA"            /* ÿÖ"_ _ %.€j.B—Nú */
	$"FFC6 4EFA FFC0 225F 205F A029 4EFA FFB8"            /* ÿÆNúÿÀ"_ _ )Núÿ¸ */
	$"225F 205F A02A 4EFA FFAE 221F 201F 225F"            /* "_ _ *Núÿ®". ."_ */
	$"205F A02E 2241 4EFA FF9E 0000"                      /*  _ ."ANúÿ.. */
};

resource 'STR ' (1001, "noSpace") {
	"Error 1001:  Short on memory; unable to "
	"allocate space."
};

resource 'STR ' (1002, "noMacExtFn") {
	"Error 1002:  Unable to find the named fu"
	"nction in the external Mac subroutine li"
	"brary."
};

resource 'STR ' (1003, "ExtFn_Kind") {
	"Error 1003:  Error in the \"kind\" of outp"
	"ut (ie, not a valid array nor a valid co"
	"nstant) returned from an external functi"
	"on."
};

resource 'STR ' (1004, "NetFn_Gen") {
	"Error 1004:  Unspecified networking-rela"
	"ted error in an attempt to run an extern"
	"al function."
};

resource 'STR ' (1005, "NetFn_EDat") {
	"Error 1005:  Error in the data received "
	"from the network external function to th"
	"e program."
};

resource 'STR ' (1006, "NetFn_EMsg") {
	"Error 1006:  Message received from the n"
	"etwork prior to completion; assumed to b"
	"e an error message."
};

resource 'STR ' (1007, "NetFn_UAb") {
	"Error 1007:  User abort during external "
	"function call over the network."
};

resource 'STR ' (1008, "NetFn_UNAb") {
	"Error 1008:  User abort during external "
	"function call over the network or reques"
	"t rejected by the network."
};

resource 'STR ' (1009, "ArrayDims") {
	"Error 1009:  The two input arrays do not"
	" have matching dimensions."
};

resource 'STR ' (1010, "KernFmt") {
	"Error 1010:  Kernels must be 3x3, 5x5, o"
	"r 7x7 and they must be in the DataScope "
	"format for kernels."
};

resource 'STR ' (1011, "KernDim") {
	"Error 1011:  To run a kernel on an input"
	" array, that input array must have dimen"
	"sion equal to or greater than the kernel"
	"."
};

resource 'STR ' (1012, "KernConst") {
	"Error 1012:  Constants are not appropria"
	"te to kernel operations, whether as an i"
	"nput or as the kernel itself."
};

resource 'STR ' (1013, "BltinDim") {
	"Error 1013:  To run a builtin function o"
	"n an input array, that input array must "
	"have dimension equal to or greater than "
	"the func."
};

resource 'STR ' (1014, "UnkOpn") {
	"Error 1014:  Unrecognized operation.  Re"
	"cognized operations are +, -, *, and /."
};

resource 'STR ' (1015, "UnkTok") {
	"Error 1015:  Unrecognized token."
};

resource 'STR ' (1016, "NoImageGen") {
	"Message 1016:  An image has not been cre"
	"ated ... most likely, the Attributes' ma"
	"x and min are out of range."
};

resource 'STR ' (1017, "window size") {
	"The image size has changed. You must clo"
	"se the current image window so that one "
	"of a different size may be opened."
};

data 'HFNT' (0, purgeable) {
	$"0000 010D 046A 0000 5020 0000 0072 F01F"            /* ...Â.j..P ...rğ. */
	$"0000 00FF 0000 0004 0D80 0000 0004 0072"            /* ...ÿ....Â€.....r */
	$"F120"                                               /* ñ  */
};

data 'HINF' (0, "Help Dialog Heading", purgeable) {
	$"4E43 5341 2044 6174 6153 636F 7065 2048"            /* NCSA DataScope H */
	$"656C 7020 6661 6369 6C69 7479 2E0D 5365"            /* elp facility.ÂSe */
	$"6C65 6374 2061 2073 7562 6A65 6374 202E"            /* lect a subject . */
	$"2E2E 0D"                                            /* ..Â */
};

data 'HTXT' (0, "----- How to use this Help Facility -----\n", purgeable) {
	$"020D 0254 6865 2066 6972 7374 2066 6577"            /* .Â.The first few */
	$"2073 6563 7469 6F6E 7320 636F 6E74 6169"            /*  sections contai */
	$"6E20 6765 6E65 7261 6C0D 0269 6E66 6F72"            /* n generalÂ.infor */
	$"6D61 7469 6F6E 2061 626F 7574 2044 6174"            /* mation about Dat */
	$"6153 636F 7065 2020 5468 6520 7265 6D61"            /* aScope  The rema */
	$"696E 696E 670D 0273 6563 7469 6F6E 7320"            /* iningÂ.sections  */
	$"6C69 7374 2074 6865 206D 656E 7520 6974"            /* list the menu it */
	$"656D 7320 616E 6420 7468 6569 7220 6675"            /* ems and their fu */
	$"6E63 7469 6F6E 732E 0D02 0D"                        /* nctions.Â.Â */
};

data 'HTXT' (1, "About NCSA DataScope\n", purgeable) {
	$"020D 024E 4353 4120 4461 7461 5363 6F70"            /* .Â.NCSA DataScop */
	$"6520 6469 7370 6C61 7973 2034 2D62 7974"            /* e displays 4-byt */
	$"6520 666C 6F61 7469 6E67 2070 6F69 6E74"            /* e floating point */
	$"0D02 6461 7461 2028 3244 2063 6172 7465"            /* Â.data (2D carte */
	$"7369 616E 206F 7220 706F 6C61 7229 2069"            /* sian or polar) i */
	$"6E20 7370 7265 6164 7368 6565 7420 666F"            /* n spreadsheet fo */
	$"726D 200D 0261 6E64 2061 7320 6120 3235"            /* rm Â.and as a 25 */
	$"362D 636F 6C6F 7220 696D 6167 652E 2020"            /* 6-color image.   */
	$"4461 7461 2063 616E 2062 6520 7573 6564"            /* Data can be used */
	$"2069 6E0D 0261 6E61 6C79 7469 6320 6571"            /*  inÂ.analytic eq */
	$"7561 7469 6F6E 7320 7669 6120 6120 6E6F"            /* uations via a no */
	$"7465 626F 6F6B 2066 6561 7475 7265 2C20"            /* tebook feature,  */
	$"616E 640D 0275 7365 722D 7772 6974 7465"            /* andÂ.user-writte */
	$"6E20 7375 6272 6F75 7469 6E65 7320 6361"            /* n subroutines ca */
	$"6E20 6265 2061 7070 6C69 6564 2074 6F20"            /* n be applied to  */
	$"7468 650D 0264 6174 6120 7669 6120 7468"            /* theÂ.data via th */
	$"6520 6E6F 7465 626F 6F6B 2E0D 020D 0254"            /* e notebook.Â.Â.T */
	$"6869 7320 7665 7273 696F 6E20 6F66 2044"            /* his version of D */
	$"6174 6153 636F 7065 2061 6C6C 6F77 7320"            /* ataScope allows  */
	$"6F75 7470 7574 2066 726F 6D0D 0270 726F"            /* output fromÂ.pro */
	$"6772 616D 7320 6F6E 2061 2072 656D 6F74"            /* grams on a remot */
	$"6520 686F 7374 2074 6F20 6265 2064 6973"            /* e host to be dis */
	$"706C 6179 6564 206F 6E0D 0244 6174 6153"            /* played onÂ.DataS */
	$"636F 7065 206F 6E20 7468 6520 4D61 6320"            /* cope on the Mac  */
	$"2865 672C 2072 756E 2061 2070 726F 6772"            /* (eg, run a progr */
	$"616D 206F 6E20 610D 0243 7261 7920 7573"            /* am on aÂ.Cray us */
	$"696E 6720 4E43 5341 2054 656C 6E65 7420"            /* ing NCSA Telnet  */
	$"616E 6420 726F 7574 6520 7468 6520 6F75"            /* and route the ou */
	$"7470 7574 0D02 746F 2044 6174 6153 636F"            /* tputÂ.to DataSco */
	$"7065 206F 6E20 7468 6520 4D61 6329 2E20"            /* pe on the Mac).  */
	$"2041 6C73 6F2C 2075 7365 722D 7772 6974"            /*  Also, user-writ */
	$"7465 6E0D 0273 7562 726F 7574 696E 6573"            /* tenÂ.subroutines */
	$"2074 6861 7420 7265 7369 6465 206F 6E20"            /*  that reside on  */
	$"6120 7265 6D6F 7465 2068 6F73 740D 0228"            /* a remote hostÂ.( */
	$"6567 2C20 6120 4372 6179 2920 6361 6E20"            /* eg, a Cray) can  */
	$"6265 2061 7070 6C69 6564 2074 6F20 3244"            /* be applied to 2D */
	$"2064 6174 6120 6F6E 2074 6865 0D02 4D61"            /*  data on theÂ.Ma */
	$"6369 6E74 6F73 6820 7573 696E 6720 7468"            /* cintosh using th */
	$"6520 6E6F 7465 626F 6F6B 2E0D 020D"                 /* e notebook.Â.Â */
};

data 'HTXT' (2, ".....DataScope Capabilities\n", purgeable) {
	$"020D 0244 6174 6153 636F 7065 2072 6561"            /* .Â.DataScope rea */
	$"6473 2061 6E64 2077 7269 7465 7320 3244"            /* ds and writes 2D */
	$"2034 2D62 7974 6520 666C 6F61 7469 6E67"            /*  4-byte floating */
	$"0D02 706F 696E 7420 6461 7461 2069 6E20"            /* Â.point data in  */
	$"4844 463B 2061 6C6C 6F77 7320 7465 7874"            /* HDF; allows text */
	$"2066 696C 6573 2074 6F20 6265 2072 6561"            /*  files to be rea */
	$"6420 616E 640D 0263 6F6E 7665 7274 6564"            /* d andÂ.converted */
	$"2074 6F20 6461 7461 3B20 7265 6164 7320"            /*  to data; reads  */
	$"4844 4620 7061 6C65 7474 6573 3B20 6765"            /* HDF palettes; ge */
	$"6E65 7261 7465 730D 0269 6D61 6765 732C"            /* neratesÂ.images, */
	$"2069 6E74 6572 706F 6C61 7465 6420 696D"            /*  interpolated im */
	$"6167 6573 2C20 616E 6420 706F 6C61 7220"            /* ages, and polar  */
	$"696D 6167 6573 3B0D 026D 6167 6E69 6669"            /* images;Â.magnifi */
	$"6573 2069 6D61 6765 733B 2061 6C6C 6F77"            /* es images; allow */
	$"7320 7468 6520 7573 6572 2074 6F20 7370"            /* s the user to sp */
	$"6563 6966 7920 6461 7461 0D02 6174 7472"            /* ecify dataÂ.attr */
	$"6962 7574 6573 2073 7563 6820 6173 2064"            /* ibutes such as d */
	$"6174 6120 6469 7370 6C61 7920 666F 726D"            /* ata display form */
	$"6174 2C0D 0265 6C69 6D69 6E61 7469 6F6E"            /* at,Â.elimination */
	$"206F 6620 6F75 746C 6965 7273 2069 6E20"            /*  of outliers in  */
	$"6469 7370 6C61 7965 6420 696D 6167 6573"            /* displayed images */
	$"2C20 6461 7461 0D02 636F 6C75 6D6E 2061"            /* , dataÂ.column a */
	$"6E64 2072 6F77 206C 6162 656C 732C 2061"            /* nd row labels, a */
	$"6E64 2064 6174 6173 6574 206C 6162 656C"            /* nd dataset label */
	$"733B 200D 0270 6572 666F 726D 7320 7374"            /* s; Â.performs st */
	$"616E 6461 7264 204D 6163 696E 746F 7368"            /* andard Macintosh */
	$"2063 6C69 7062 6F61 7264 2063 6F70 7920"            /*  clipboard copy  */
	$"616E 640D 0270 6173 7465 3B20 616E 6420"            /* andÂ.paste; and  */
	$"7065 7266 6F72 6D73 206D 6174 6865 6D61"            /* performs mathema */
	$"7469 6361 6C20 6F70 6572 6174 696F 6E73"            /* tical operations */
	$"206F 6E0D 0274 6865 2064 6174 6120 7669"            /*  onÂ.the data vi */
	$"6120 6120 6E6F 7465 626F 6F6B 2074 6861"            /* a a notebook tha */
	$"7420 6361 6E20 6265 2073 6176 6564 2077"            /* t can be saved w */
	$"6974 680D 0274 6865 2064 6174 612E 2020"            /* ithÂ.the data.   */
	$"5468 6520 7669 6577 696E 6720 6F66 2073"            /* The viewing of s */
	$"6576 6572 616C 2064 6174 6120 7365 7473"            /* everal data sets */
	$"2069 730D 0266 6163 696C 6974 6174 6564"            /*  isÂ.facilitated */
	$"2062 7920 7468 6520 7379 6E63 6872 6F6E"            /*  by the synchron */
	$"697A 6174 696F 6E20 6665 6174 7572 652C"            /* ization feature, */
	$"2061 6E64 0D02 7375 6272 6567 696F 6E73"            /*  andÂ.subregions */
	$"206F 6620 7468 6520 3244 2064 6174 6120"            /*  of the 2D data  */
	$"6361 6E20 6265 2065 7874 7261 6374 6564"            /* can be extracted */
	$"2066 726F 6D0D 0274 6865 206F 7269 6769"            /*  fromÂ.the origi */
	$"6E61 6C20 6461 7461 7365 742E 2020 5573"            /* nal dataset.  Us */
	$"6572 2D77 7269 7474 656E 2073 7562 726F"            /* er-written subro */
	$"7574 696E 6573 0D02 6361 6E20 6265 2075"            /* utinesÂ.can be u */
	$"7365 6420 7669 6120 7468 6520 6E6F 7465"            /* sed via the note */
	$"626F 6F6B 2074 6F20 7065 7266 6F72 6D0D"            /* book to performÂ */
	$"026F 7065 7261 7469 6F6E 7320 6F6E 2074"            /* .operations on t */
	$"6865 2064 6174 612E 0D02 0D02 4E65 7720"            /* he data.Â.Â.New  */
	$"6665 6174 7572 6573 206F 6620 7468 6973"            /* features of this */
	$"2076 6572 7369 6F6E 206F 6620 4461 7461"            /*  version of Data */
	$"5363 6F70 650D 0269 6E63 6C75 6465 2074"            /* ScopeÂ.include t */
	$"6865 2061 6269 6C69 7479 2074 6F20 6C69"            /* he ability to li */
	$"6E6B 2074 6F20 6120 7265 6D6F 7465 2068"            /* nk to a remote h */
	$"6F73 7420 2865 672C 2061 0D02 4372 6179"            /* ost (eg, aÂ.Cray */
	$"292C 2072 756E 2070 726F 6772 616D 7320"            /* ), run programs  */
	$"7468 6174 2067 656E 6572 6174 6520 3244"            /* that generate 2D */
	$"2064 6174 612C 2061 6E64 0D02 726F 7574"            /*  data, andÂ.rout */
	$"6520 7468 6520 6F75 7470 7574 2062 6163"            /* e the output bac */
	$"6B20 746F 2044 6174 6153 636F 7065 206F"            /* k to DataScope o */
	$"6E20 7468 6520 4D61 630D 0266 6F72 2076"            /* n the MacÂ.for v */
	$"6965 7769 6E67 2061 6E64 2061 6464 6974"            /* iewing and addit */
	$"696F 6E61 6C20 7072 6F63 6573 7369 6E67"            /* ional processing */
	$"2E20 2049 6E0D 0261 6464 6974 696F 6E2C"            /* .  InÂ.addition, */
	$"2075 7365 722D 7772 6974 7465 6E20 7375"            /*  user-written su */
	$"6272 6F75 7469 6E65 7320 7265 7369 6469"            /* broutines residi */
	$"6E67 206F 6E20 7468 650D 0272 656D 6F74"            /* ng on theÂ.remot */
	$"6520 686F 7374 2063 616E 2062 6520 6170"            /* e host can be ap */
	$"706C 6965 6420 746F 2032 4420 6461 7461"            /* plied to 2D data */
	$"7365 7473 2069 6E0D 0244 6174 6153 636F"            /* sets inÂ.DataSco */
	$"7065 2076 6961 2074 6865 206E 6F74 6562"            /* pe via the noteb */
	$"6F6F 6B2E 0D02 0D02 5553 4552 2046 4545"            /* ook.Â.Â.USER FEE */
	$"4442 4143 4B20 4F4E 2054 4845 2044 4553"            /* DBACK ON THE DES */
	$"4952 4142 494C 4954 5920 4F46 2047 4956"            /* IRABILITY OF GIV */
	$"454E 0D02 4645 4154 5552 4553 2057 494C"            /* ENÂ.FEATURES WIL */
	$"4C20 504C 4159 2041 204C 4152 4745 2050"            /* L PLAY A LARGE P */
	$"4152 5420 494E 0D02 5052 494F 5249 5449"            /* ART INÂ.PRIORITI */
	$"5A49 4E47 2054 4845 2049 4E43 4C55 5349"            /* ZING THE INCLUSI */
	$"4F4E 204F 4620 5448 4F53 4520 4645 4154"            /* ON OF THOSE FEAT */
	$"5552 4553 0D02 494E 2046 5554 5552 4520"            /* URESÂ.IN FUTURE  */
	$"5645 5253 494F 4E53 204F 4620 4E43 5341"            /* VERSIONS OF NCSA */
	$"2044 4154 4153 434F 5045 2E0D 020D"                 /*  DATASCOPE.Â.Â */
};

data 'HTXT' (3, ".....User Data\n", purgeable) {
	$"020D 0244 6174 6153 636F 7065 2072 6561"            /* .Â.DataScope rea */
	$"6473 2061 6E64 2077 7269 7465 7320 3244"            /* ds and writes 2D */
	$"2034 2D62 7974 6520 666C 6F61 7469 6E67"            /*  4-byte floating */
	$"0D02 706F 696E 7420 6461 7461 2073 746F"            /* Â.point data sto */
	$"7265 6420 696E 2048 4446 2066 696C 6573"            /* red in HDF files */
	$"206F 6E20 7468 6520 4D61 6369 6E74 6F73"            /*  on the Macintos */
	$"682E 0D02 486F 7269 7A6F 6E74 616C 2061"            /* h.Â.Horizontal a */
	$"6E64 2076 6572 7469 6361 6C20 7363 616C"            /* nd vertical scal */
	$"6520 696E 666F 726D 6174 696F 6E20 6973"            /* e information is */
	$"0D02 7265 6164 2C20 616E 6420 7072 6F63"            /* Â.read, and proc */
	$"6573 7369 6E67 2068 6973 746F 7279 2061"            /* essing history a */
	$"6E64 2061 6E6E 6F74 6174 696F 6E73 2061"            /* nd annotations a */
	$"7265 0D02 7361 7665 6420 7669 6120 7468"            /* reÂ.saved via th */
	$"6520 6E6F 7465 626F 6F6B 2E20 2050 616C"            /* e notebook.  Pal */
	$"6574 7465 7320 7374 6F72 6564 2069 6E20"            /* ettes stored in  */
	$"7468 650D 0248 4446 2066 696C 6573 2061"            /* theÂ.HDF files a */
	$"7265 2075 7365 6420 2869 6620 6E6F 2070"            /* re used (if no p */
	$"616C 6574 7465 2065 7869 7374 732C 2061"            /* alette exists, a */
	$"2064 6566 6175 6C74 0D02 7769 6C6C 2062"            /*  defaultÂ.will b */
	$"6520 7573 6564 2062 7920 4461 7461 5363"            /* e used by DataSc */
	$"6F70 6529 2E0D 020D 0257 6865 6E20 616E"            /* ope).Â.Â.When an */
	$"2048 4446 2066 696C 6520 636F 6E74 6169"            /*  HDF file contai */
	$"6E69 6E67 206D 756C 7469 706C 6520 696D"            /* ning multiple im */
	$"6167 6573 206F 720D 026D 756C 7469 706C"            /* ages orÂ.multipl */
	$"6520 7061 6C65 7474 6573 2069 7320 7265"            /* e palettes is re */
	$"6164 2C20 6F6E 6C79 2074 6865 2066 6972"            /* ad, only the fir */
	$"7374 2069 6D61 6765 2061 6E64 0D02 7061"            /* st image andÂ.pa */
	$"6C65 7474 6520 6973 206C 6F61 6465 642C"            /* lette is loaded, */
	$"2074 6865 2072 656D 6169 6E64 6572 2062"            /*  the remainder b */
	$"6569 6E67 2069 676E 6F72 6564 2E0D 020D"            /* eing ignored.Â.Â */
	$"0257 6865 6E20 6461 7461 2069 7320 7265"            /* .When data is re */
	$"6164 2C20 6974 2069 7320 6173 7375 6D65"            /* ad, it is assume */
	$"6420 746F 2062 6520 696E 0D02 2272 6F77"            /* d to be inÂ."row */
	$"2D6D 616A 6F72 2220 6F72 6465 722E 2020"            /* -major" order.   */
	$"466F 7220 706F 6C61 7220 6461 7461 2C20"            /* For polar data,  */
	$"726F 7720 6865 6164 696E 6773 0D02 7370"            /* row headingsÂ.sp */
	$"6563 6966 7920 7261 6469 692C 2077 6869"            /* ecify radii, whi */
	$"6C65 2063 6F6C 756D 6E20 6865 6164 696E"            /* le column headin */
	$"6773 2073 7065 6369 6679 0D02 616E 676C"            /* gs specifyÂ.angl */
	$"6573 2E20 2041 6E67 6C65 7320 7368 6F75"            /* es.  Angles shou */
	$"6C64 2062 6520 6265 7477 6565 6E20 3020"            /* ld be between 0  */
	$"616E 6420 32B9 2E0D 020D"                           /* and 2¹.Â.Â */
};

data 'HTXT' (4, ".....Operations on Data\n", purgeable) {
	$"020D 0244 6174 6120 6973 206E 6576 6572"            /* .Â.Data is never */
	$"2063 6861 6E67 6564 2062 7920 6120 4461"            /*  changed by a Da */
	$"7461 5363 6F70 6520 6F70 6572 6174 696F"            /* taScope operatio */
	$"6E3A 0D02 7768 656E 6576 6572 2061 2064"            /* n:Â.whenever a d */
	$"6174 6120 6F70 6572 6174 696F 6E20 6973"            /* ata operation is */
	$"2070 6572 666F 726D 6564 2C20 6974 2069"            /*  performed, it i */
	$"730D 0261 6C77 6179 7320 7065 7266 6F72"            /* sÂ.always perfor */
	$"6D65 6420 6F6E 2074 6865 2066 6C6F 6174"            /* med on the float */
	$"696E 6720 706F 696E 7420 6E75 6D62 6572"            /* ing point number */
	$"730D 0228 6E65 7665 7220 6F6E 2061 6E20"            /* sÂ.(never on an  */
	$"696D 6167 652C 2077 6869 6368 2065 7869"            /* image, which exi */
	$"7374 7320 736F 6C65 6C79 2066 6F72 2064"            /* sts solely for d */
	$"6973 706C 6179 0D02 7075 7270 6F73 6573"            /* isplayÂ.purposes */
	$"2061 6E64 2066 6F72 2063 6F70 7969 6E67"            /*  and for copying */
	$"2074 6F20 7468 6520 636C 6970 626F 6172"            /*  to the clipboar */
	$"6429 2C20 7769 7468 0D02 6F75 7470 7574"            /* d), withÂ.output */
	$"2077 7269 7474 656E 2074 6F20 6120 6E65"            /*  written to a ne */
	$"7720 4461 7461 5363 6F70 6520 6461 7461"            /* w DataScope data */
	$"7365 742E 0D02 0D"                                  /* set.Â.Â */
};

data 'HTXT' (5, ".....Working with Multiple Datasets\n", purgeable) {
	$"020D 0254 6865 206E 756D 6265 7220 6F66"            /* .Â.The number of */
	$"2064 6174 6173 6574 7320 7468 6174 2063"            /*  datasets that c */
	$"616E 2062 6520 6C6F 6164 6564 2069 6E74"            /* an be loaded int */
	$"6F0D 0244 6174 6153 636F 7065 2069 7320"            /* oÂ.DataScope is  */
	$"6C69 6D69 7465 6420 6F6E 6C79 2062 7920"            /* limited only by  */
	$"7468 6520 616D 6F75 6E74 206F 660D 026D"            /* the amount ofÂ.m */
	$"656D 6F72 7920 6176 6169 6C61 626C 6520"            /* emory available  */
	$"2870 726F 6772 616D 2073 697A 6520 7368"            /* (program size sh */
	$"6F75 6C64 2062 6520 7365 7420 6173 0D02"            /* ould be set asÂ. */
	$"6170 7072 6F70 7269 6174 6520 746F 2074"            /* appropriate to t */
	$"6865 206D 6163 6869 6E65 2061 6E64 2074"            /* he machine and t */
	$"6F20 7468 6520 7061 7274 6963 756C 6172"            /* o the particular */
	$"0D02 6170 706C 6963 6174 696F 6E20 6174"            /* Â.application at */
	$"2068 616E 6429 2E0D 020D 0245 6163 6820"            /*  hand).Â.Â.Each  */
	$"6461 7461 7365 7420 6973 2070 7265 7365"            /* dataset is prese */
	$"6E74 6564 2069 6E20 6120 7365 7061 7261"            /* nted in a separa */
	$"7465 2077 696E 646F 772C 0D02 616E 6420"            /* te window,Â.and  */
	$"616E 7920 636F 6D6D 616E 6473 2069 7373"            /* any commands iss */
	$"7565 6420 6F72 206F 7065 7261 7469 6F6E"            /* ued or operation */
	$"7320 7065 7266 6F72 6D65 640D 0261 7070"            /* s performedÂ.app */
	$"6C79 206F 6E6C 7920 746F 2074 6865 2061"            /* ly only to the a */
	$"6374 6976 6520 696D 6167 6520 7769 6E64"            /* ctive image wind */
	$"6F77 2E0D 020D 0246 6F72 2076 6965 7769"            /* ow.Â.Â.For viewi */
	$"6E67 2070 7572 706F 7365 732C 2064 6966"            /* ng purposes, dif */
	$"6665 7265 6E74 2064 6174 6173 6574 7320"            /* ferent datasets  */
	$"6361 6E20 6265 0D02 7379 6E63 6872 6F6E"            /* can beÂ.synchron */
	$"697A 6564 2073 6F20 7468 6174 2068 696C"            /* ized so that hil */
	$"6974 6564 2073 7562 7265 6769 6F6E 7320"            /* ited subregions  */
	$"6F66 206F 6E65 0D02 6461 7461 7365 7420"            /* of oneÂ.dataset  */
	$"6F72 2069 6D61 6765 2077 696C 6C20 6361"            /* or image will ca */
	$"7573 6520 616C 6C20 6F74 6865 7220 6461"            /* use all other da */
	$"7461 7365 7473 0D02 616E 6420 696D 6167"            /* tasetsÂ.and imag */
	$"6573 2074 6F20 6265 2073 696D 696C 6172"            /* es to be similar */
	$"6C79 2068 696C 6974 6564 2E0D 020D"                 /* ly hilited.Â.Â */
};

data 'HTXT' (6, ".....Corresponding with NCSA\n", purgeable) {
	$"020D 024D 6169 6C20 2866 6565 6462 6163"            /* .Â.Mail (feedbac */
	$"6B2C 2073 7567 6765 7374 696F 6E73 2C20"            /* k, suggestions,  */
	$"6275 6773 2920 6D61 7920 6265 2073 656E"            /* bugs) may be sen */
	$"7420 746F 0D02 0D02 2020 2020 2020 2020"            /* t toÂ.Â.         */
	$"2020 4E43 5341 2053 6F66 7477 6172 6520"            /*   NCSA Software  */
	$"546F 6F6C 730D 0220 2020 2020 2020 2020"            /* ToolsÂ.          */
	$"2044 6174 6153 636F 7065 0D02 2020 2020"            /*  DataScopeÂ.     */
	$"2020 2020 2020 3135 3220 436F 6D70 7574"            /*       152 Comput */
	$"696E 6720 4170 706C 6963 6174 696F 6E73"            /* ing Applications */
	$"2042 6C64 672E 0D02 2020 2020 2020 2020"            /*  Bldg.Â.         */
	$"2020 3630 3520 452E 2053 7072 696E 6766"            /*   605 E. Springf */
	$"6965 6C64 2041 7665 2E0D 0220 2020 2020"            /* ield Ave.Â.      */
	$"2020 2020 2043 6861 6D70 6169 676E 2C20"            /*      Champaign,  */
	$"494C 2020 2020 3631 3832 300D 020D 0245"            /* IL    61820Â.Â.E */
	$"2D6D 6169 6C20 6275 6720 7265 706F 7274"            /* -mail bug report */
	$"7320 6D61 7920 6265 2073 656E 7420 746F"            /* s may be sent to */
	$"0D02 0D02 2020 2020 2020 2020 2020 6275"            /* Â.Â.          bu */
	$"6773 406E 6373 612E 7569 7563 2E65 6475"            /* gs@ncsa.uiuc.edu */
	$"0D02 2020 2020 2020 2020 2020 6275 6773"            /* Â.          bugs */
	$"406E 6373 6176 6D73 2E62 6974 6E65 740D"            /* @ncsavms.bitnetÂ */
	$"020D 0241 6C6C 206F 7468 6572 2065 2D6D"            /* .Â.All other e-m */
	$"6169 6C20 6D61 7920 6265 2073 656E 7420"            /* ail may be sent  */
	$"746F 0D02 0D02 2020 2020 2020 2020 2020"            /* toÂ.Â.           */
	$"736F 6674 6465 7640 6E63 7361 2E75 6975"            /* softdev@ncsa.uiu */
	$"632E 6564 750D 0220 2020 2020 2020 2020"            /* c.eduÂ.          */
	$"2073 6F66 7464 6576 406E 6373 6176 6D73"            /*  softdev@ncsavms */
	$"2E62 6974 6E65 740D 020D"                           /* .bitnetÂ.Â */
};

data 'HTXT' (7, ".....DataScope Error Messages\n", purgeable) {
	$"020D 0244 6174 6153 636F 7065 2773 2063"            /* .Â.DataScope's c */
	$"6F64 6520 6973 206C 6962 6572 616C 6C79"            /* ode is liberally */
	$"2073 7072 696E 6B6C 6564 2077 6974 6820"            /*  sprinkled with  */
	$"6572 726F 720D 026D 6573 7361 6765 7320"            /* errorÂ.messages  */
	$"746F 2063 6174 6368 2074 6865 206D 6F73"            /* to catch the mos */
	$"7420 636F 6D6D 6F6E 6C79 206D 6164 6520"            /* t commonly made  */
	$"7573 6572 0D02 6572 726F 7273 2E20 2054"            /* userÂ.errors.  T */
	$"6865 7365 206D 6573 7361 6765 7320 7370"            /* hese messages sp */
	$"6563 6966 7920 7468 6520 6E61 7475 7265"            /* ecify the nature */
	$"206F 6620 7468 650D 0270 726F 626C 656D"            /*  of theÂ.problem */
	$"2061 6E64 2061 7474 656D 7074 2074 6F20"            /*  and attempt to  */
	$"6865 6C70 2074 6865 2075 7365 7220 696E"            /* help the user in */
	$"0D02 636F 7272 6563 7469 6E67 2074 6865"            /* Â.correcting the */
	$"2070 726F 626C 656D 202E 0D02 0D02 5768"            /*  problem .Â.Â.Wh */
	$"656E 2073 656E 6469 6E67 2063 6F72 7265"            /* en sending corre */
	$"7370 6F6E 6465 6E63 6520 7265 6C61 7469"            /* spondence relati */
	$"6E67 2074 6F20 6572 726F 720D 026D 6573"            /* ng to errorÂ.mes */
	$"7361 6765 7320 746F 204E 4353 412C 2070"            /* sages to NCSA, p */
	$"6C65 6173 6520 7375 7070 6C79 2074 6865"            /* lease supply the */
	$"206E 756D 6265 720D 0274 6861 7420 6170"            /*  numberÂ.that ap */
	$"7065 6172 7320 6F6E 2074 6865 2070 6572"            /* pears on the per */
	$"7469 6E65 6E74 2065 7272 6F72 206D 6573"            /* tinent error mes */
	$"7361 6765 2873 293A 0D02 7468 6579 2061"            /* sage(s):Â.they a */
	$"7265 2061 2062 6967 2068 656C 7020 696E"            /* re a big help in */
	$"2064 6574 6572 6D69 6E69 6E67 2074 6865"            /*  determining the */
	$"2070 6572 7469 6E65 6E74 0D02 6C6F 6361"            /*  pertinentÂ.loca */
	$"7469 6F6E 2069 6E20 7468 6520 736F 7572"            /* tion in the sour */
	$"6365 2063 6F64 6520 7468 6174 2069 7320"            /* ce code that is  */
	$"7265 6C61 7465 6420 746F 200D 0274 6865"            /* related to Â.the */
	$"2063 6F72 7265 7370 6F6E 6465 6E63 652E"            /*  correspondence. */
	$"0D02 0D"                                            /* Â.Â */
};

data 'HTXT' (8, "Network Capabilities\n", purgeable) {
	$"020D 0255 7369 6E67 204E 4353 4120 5465"            /* .Â.Using NCSA Te */
	$"6C6E 6574 2077 6974 6820 4D61 6354 4350"            /* lnet with MacTCP */
	$"2C20 7573 6572 7320 6361 6E20 7275 6E0D"            /* , users can runÂ */
	$"0270 726F 6772 616D 7320 7468 6174 2067"            /* .programs that g */
	$"656E 6572 6174 6520 3244 206F 7574 7075"            /* enerate 2D outpu */
	$"7420 2865 6974 6865 7220 7369 6E67 6C65"            /* t (either single */
	$"0D02 6669 6C65 7320 6F72 2061 2073 6572"            /* Â.files or a ser */
	$"6965 7320 6F66 2061 6E69 6D61 7469 6F6E"            /* ies of animation */
	$"2069 6D61 6765 7329 206F 6E20 6120 7265"            /*  images) on a re */
	$"6D6F 7465 0D02 686F 7374 2028 6567 2C20"            /* moteÂ.host (eg,  */
	$"6120 4372 6179 2920 616E 6420 726F 7574"            /* a Cray) and rout */
	$"6520 7468 6520 6F75 7470 7574 2062 6163"            /* e the output bac */
	$"6B20 746F 200D 0244 6174 6153 636F 7065"            /* k to Â.DataScope */
	$"206F 6E20 7468 6520 4D61 6369 6E74 6F73"            /*  on the Macintos */
	$"6820 666F 7220 7669 6577 696E 6720 616E"            /* h for viewing an */
	$"640D 0261 6464 6974 696F 6E61 6C20 6F70"            /* dÂ.additional op */
	$"6572 6174 696F 6E73 2E20 2049 6E20 6164"            /* erations.  In ad */
	$"6469 7469 6F6E 2C20 7375 6272 6F75 7469"            /* dition, subrouti */
	$"6E65 730D 0272 6573 6964 696E 6720 6F6E"            /* nesÂ.residing on */
	$"2074 6865 2072 656D 6F74 6520 686F 7374"            /*  the remote host */
	$"2063 616E 2062 6520 7573 6564 2074 6F20"            /*  can be used to  */
	$"7072 6F63 6573 730D 0244 6174 6153 636F"            /* processÂ.DataSco */
	$"7065 2064 6174 6173 6574 7320 6F6E 2074"            /* pe datasets on t */
	$"6865 204D 6163 696E 746F 7368 2076 6961"            /* he Macintosh via */
	$"2074 6865 0D02 4461 7461 5363 6F70 6520"            /*  theÂ.DataScope  */
	$"6E6F 7465 626F 6F6B 2E20 2052 6566 6572"            /* notebook.  Refer */
	$"2074 6F20 7468 6520 6D61 6E75 616C 2066"            /*  to the manual f */
	$"6F72 0D02 6465 7461 696C 7320 6F66 2074"            /* orÂ.details of t */
	$"6865 206D 6563 6861 6E69 6373 206F 6620"            /* he mechanics of  */
	$"7468 6973 206F 7065 7261 7469 6F6E 2E0D"            /* this operation.Â */
	$"020D 0244 6174 6153 636F 7065 2773 2077"            /* .Â.DataScope's w */
	$"656C 636F 6D69 6E67 2064 6961 6C6F 6720"            /* elcoming dialog  */
	$"696E 666F 726D 7320 7468 6520 7573 6572"            /* informs the user */
	$"0D02 6173 2074 6F20 7768 6574 6865 7220"            /* Â.as to whether  */
	$"7468 6520 6E65 7477 6F72 6B69 6E67 2066"            /* the networking f */
	$"756E 6374 696F 6E61 6C69 7479 0D02 6361"            /* unctionalityÂ.ca */
	$"6E20 6265 2075 7365 643A 2020 6966 2073"            /* n be used:  if s */
	$"6F2C 2074 6865 206D 6573 7361 6765 2072"            /* o, the message r */
	$"6561 6473 2022 4E65 7477 6F72 6B0D 0245"            /* eads "NetworkÂ.E */
	$"6E61 626C 6564 223B 2069 662C 2066 6F72"            /* nabled"; if, for */
	$"2061 6E79 2072 6561 736F 6E2C 2074 6865"            /*  any reason, the */
	$"206E 6574 776F 726B 696E 670D 0263 6170"            /*  networkingÂ.cap */
	$"6162 696C 6974 7920 7769 6C6C 206E 6F74"            /* ability will not */
	$"2077 6F72 6B2C 2074 6865 206D 6573 7361"            /*  work, the messa */
	$"6765 2072 6561 6473 0D02 224E 6574 776F"            /* ge readsÂ."Netwo */
	$"726B 2044 6973 6162 6C65 6422 2E0D 020D"            /* rk Disabled".Â.Â */
	$"0249 6620 7468 6520 6E65 7477 6F72 6B20"            /* .If the network  */
	$"6973 2064 6973 6162 6C65 642C 2044 6174"            /* is disabled, Dat */
	$"6153 636F 7065 2773 206F 7468 6572 0D02"            /* aScope's otherÂ. */
	$"6665 6174 7572 6573 2077 6F72 6B20 6173"            /* features work as */
	$"2074 6865 7920 7368 6F75 6C64 3A20 2074"            /*  they should:  t */
	$"6865 2075 7365 7220 7369 6D70 6C79 0D02"            /* he user simplyÂ. */
	$"6361 6E6E 6F74 2075 7365 2074 6865 206E"            /* cannot use the n */
	$"6574 776F 726B 696E 6720 6361 7061 6269"            /* etworking capabi */
	$"6C69 7469 6573 206F 6620 7468 650D 0270"            /* lities of theÂ.p */
	$"726F 6772 616D 2E0D 020D"                           /* rogram.Â.Â */
};

data 'HTXT' (9, "DataScope's Notebook\n", purgeable) {
	$"020D 0255 7365 2074 6865 206E 6F74 6562"            /* .Â.Use the noteb */
	$"6F6F 6B20 746F 2073 6176 6520 616E 6E6F"            /* ook to save anno */
	$"7461 7469 6F6E 7320 616E 6420 746F 2063"            /* tations and to c */
	$"7265 6174 650D 026E 6577 2064 6174 6173"            /* reateÂ.new datas */
	$"6574 7320 6279 2065 7865 6375 7469 6E67"            /* ets by executing */
	$"2061 6E61 6C79 7469 6320 6571 7561 7469"            /*  analytic equati */
	$"6F6E 7320 7468 6174 0D02 7573 6520 6578"            /* ons thatÂ.use ex */
	$"6973 7469 6E67 2064 6174 6173 6574 732E"            /* isting datasets. */
	$"0D02 0D02 546F 2061 6E6E 6F74 6174 6520"            /* Â.Â.To annotate  */
	$"6120 6461 7461 7365 742C 2073 696D 706C"            /* a dataset, simpl */
	$"7920 7772 6974 6520 7468 6520 7465 7874"            /* y write the text */
	$"2069 6E20 7468 650D 026E 6F74 6562 6F6F"            /*  in theÂ.noteboo */
	$"6B2E 2020 5768 656E 2074 6865 2064 6174"            /* k.  When the dat */
	$"6173 6574 2069 7320 7361 7665 642C 2074"            /* aset is saved, t */
	$"6865 0D02 616E 6E6F 7461 7465 6420 7465"            /* heÂ.annotated te */
	$"7874 2077 696C 6C20 6265 2073 6176 6564"            /* xt will be saved */
	$"2061 6C6F 6E67 2077 6974 6820 6974 2E0D"            /*  along with it.Â */
	$"020D"                                               /* .Â */
};

data 'HTXT' (10, ".....Notebook Calculation Format\n", purgeable) {
	$"020D 0254 6F20 6578 6563 7574 6520 616E"            /* .Â.To execute an */
	$"616C 7974 6963 2065 7175 6174 696F 6E73"            /* alytic equations */
	$"2065 6E74 6572 2061 2046 4F52 5452 414E"            /*  enter a FORTRAN */
	$"2D6C 696B 650D 0261 7373 6967 6E6D 656E"            /* -likeÂ.assignmen */
	$"7420 7374 6174 656D 656E 7420 636F 6E73"            /* t statement cons */
	$"6973 7469 6E67 206F 6620 4461 7461 5363"            /* isting of DataSc */
	$"6F70 6520 0D02 6461 7461 7365 7473 2028"            /* ope Â.datasets ( */
	$"616C 7761 7973 2072 6566 6572 7265 6420"            /* always referred  */
	$"746F 2062 7920 7468 6569 7220 7769 6E64"            /* to by their wind */
	$"6F77 0D02 7469 746C 6573 292C 206D 6174"            /* owÂ.titles), mat */
	$"6865 6D61 7469 6361 6C20 6F70 6572 6174"            /* hematical operat */
	$"6F72 7320 282B 2C2D 2C2A 2C20 616E 6420"            /* ors (+,-,*, and  */
	$"2F29 2C0D 0270 6172 656E 7468 6573 6573"            /* /),Â.parentheses */
	$"2074 6F20 6772 6F75 7020 7468 6520 7661"            /*  to group the va */
	$"7269 6162 6C65 732C 2063 6F6E 7374 616E"            /* riables, constan */
	$"7473 0D02 2869 6E20 6120 666F 726D 2074"            /* tsÂ.(in a form t */
	$"6861 7420 776F 756C 6420 6163 6365 7074"            /* hat would accept */
	$"6162 6C65 2074 6F20 464F 5254 5241 4E2C"            /* able to FORTRAN, */
	$"0D02 6C69 6B65 2065 2D66 6F72 6D61 7420"            /* Â.like e-format  */
	$"6F72 2069 6E74 6567 6572 292C 2061 6E64"            /* or integer), and */
	$"2044 6174 6153 636F 7065 0D02 6675 6E63"            /*  DataScopeÂ.func */
	$"7469 6F6E 732E 0D02 0D02 466F 7220 6578"            /* tions.Â.Â.For ex */
	$"616D 706C 652C 2073 7570 706F 7365 204D"            /* ample, suppose M */
	$"5944 4154 4120 6973 2061 2064 6174 6173"            /* YDATA is a datas */
	$"6574 2E20 2054 6865 6E0D 0265 7865 6375"            /* et.  ThenÂ.execu */
	$"7469 6E67 2074 6865 2065 7175 6174 696F"            /* ting the equatio */
	$"6E20 0D02 0D02 2020 2020 204E 4557 203D"            /* n Â.Â.     NEW = */
	$"2033 2E32 202A 2028 7369 6E28 4D59 4441"            /*  3.2 * (sin(MYDA */
	$"5441 2920 2B20 312E 652D 3329 202B 2031"            /* TA) + 1.e-3) + 1 */
	$"350D 020D 0277 696C 6C20 6372 6561 7465"            /* 5Â.Â.will create */
	$"2061 206E 6577 2064 6174 6173 6574 2C20"            /*  a new dataset,  */
	$"7768 6572 6520 7468 6520 656C 656D 656E"            /* where the elemen */
	$"7473 0D02 6861 7665 2074 6865 2076 616C"            /* tsÂ.have the val */
	$"7565 730D 020D 0220 2020 2020 6E65 7728"            /* uesÂ.Â.     new( */
	$"692C 6A29 203D 2033 2E32 202A 2028 7369"            /* i,j) = 3.2 * (si */
	$"6E28 6D79 6461 7461 2869 2C6A 2929 202B"            /* n(mydata(i,j)) + */
	$"2031 2E65 2D33 2920 2B20 3135 0D02 0D02"            /*  1.e-3) + 15Â.Â. */
	$"666F 7220 6576 6572 7920 7061 6972 2028"            /* for every pair ( */
	$"692C 6A29 2069 6E20 7468 6520 6461 7461"            /* i,j) in the data */
	$"2E20 2049 7420 6973 2069 6D70 6F72 7461"            /* .  It is importa */
	$"6E74 2074 6F0D 0272 656D 656D 6265 7220"            /* nt toÂ.remember  */
	$"7468 6174 206F 7065 7261 7469 6F6E 7320"            /* that operations  */
	$"6F6E 2032 4420 6172 7261 7973 2061 7265"            /* on 2D arrays are */
	$"206F 6E20 616E 0D02 656C 656D 656E 742D"            /*  on anÂ.element- */
	$"6279 2D65 6C65 6D65 6E74 2062 6173 6973"            /* by-element basis */
	$"2C20 616E 6420 6172 6520 6E6F 7420 6D61"            /* , and are not ma */
	$"7472 6978 0D02 6F70 6572 6174 696F 6E73"            /* trixÂ.operations */
	$"2028 7468 6F75 6768 2075 7365 7273 2061"            /*  (though users a */
	$"7265 2066 7265 6520 746F 2077 7269 7465"            /* re free to write */
	$"2073 7563 680D 026D 6174 7269 7820 6F70"            /*  suchÂ.matrix op */
	$"6572 6174 696F 6E73 2069 6E74 6F20 7375"            /* erations into su */
	$"6272 6F75 7469 6E65 7320 7468 6174 2063"            /* broutines that c */
	$"616E 2062 6520 0D02 6361 6C6C 6564 2076"            /* an be Â.called v */
	$"6961 2044 6174 6153 636F 7065 2773 2065"            /* ia DataScope's e */
	$"7874 6572 6E61 6C20 6C69 6272 6172 7920"            /* xternal library  */
	$"6665 6174 7572 6529 2E0D 020D"                      /* feature).Â.Â */
};

data 'HTXT' (11, ".....Builtin Notebook Functions\n", purgeable) {
	$"020D 024C 6574 2050 2061 6E64 2051 2062"            /* .Â.Let P and Q b */
	$"6520 6461 7461 7365 7473 206C 6F61 6465"            /* e datasets loade */
	$"6420 696E 746F 2044 6174 6153 636F 7065"            /* d into DataScope */
	$"2E0D 0254 6865 6E20 7468 6520 666F 6C6C"            /* .Â.Then the foll */
	$"6F77 696E 6720 6275 696C 7469 6E20 6675"            /* owing builtin fu */
	$"6E63 7469 6F6E 7320 6361 6E20 6265 0D02"            /* nctions can beÂ. */
	$"6170 706C 6965 643A 0D02 0D02 312E 2020"            /* applied:Â.Â.1.   */
	$"5374 616E 6461 7264 206D 6174 6820 6675"            /* Standard math fu */
	$"6E63 7469 6F6E 7320 2861 6E67 6C65 7320"            /* nctions (angles  */
	$"696E 2072 6164 6961 6E73 293A 0D02 7369"            /* in radians):Â.si */
	$"6E28 5129 2020 6173 696E 2851 2920 2073"            /* n(Q)  asin(Q)  s */
	$"696E 6828 5129 2020 2020 6C6F 6728 5129"            /* inh(Q)    log(Q) */
	$"2020 2020 2061 6273 2851 2920 2020 2070"            /*      abs(Q)    p */
	$"6F77 2851 2C50 290D 0263 6F73 2851 2920"            /* ow(Q,P)Â.cos(Q)  */
	$"2061 636F 7328 5129 2020 636F 7368 2851"            /*  acos(Q)  cosh(Q */
	$"2920 206C 6F67 3130 2851 2920 2073 7172"            /* )  log10(Q)  sqr */
	$"7428 5129 0D02 7461 6E28 5129 2020 6174"            /* t(Q)Â.tan(Q)  at */
	$"616E 2851 2920 2074 616E 6828 5129 2020"            /* an(Q)  tanh(Q)   */
	$"2065 7870 2851 2920 2020 2020 6174 616E"            /*  exp(Q)     atan */
	$"3228 512C 5029 0D02 0D02 322E 2020 5374"            /* 2(Q,P)Â.Â.2.  St */
	$"616E 6461 7264 2074 7269 6720 6675 6E63"            /* andard trig func */
	$"7469 6F6E 7320 2861 6E67 6C65 7320 696E"            /* tions (angles in */
	$"2064 6567 7265 6573 293A 0D02 6473 696E"            /*  degrees):Â.dsin */
	$"2851 2920 2064 6173 696E 2851 2920 2064"            /* (Q)  dasin(Q)  d */
	$"7369 6E68 2851 2920 2064 6174 616E 3228"            /* sinh(Q)  datan2( */
	$"512C 5029 0D02 6463 6F73 2851 2920 2064"            /* Q,P)Â.dcos(Q)  d */
	$"6173 696E 2851 2920 2064 636F 7368 2851"            /* asin(Q)  dcosh(Q */
	$"290D 0264 7461 6E28 5129 2020 6461 7461"            /* )Â.dtan(Q)  data */
	$"6E28 5129 2020 6474 616E 6828 5129 0D02"            /* n(Q)  dtanh(Q)Â. */
	$"0D02 332E 2020 4D69 7363 2E20 6675 6E63"            /* Â.3.  Misc. func */
	$"7469 6F6E 733A 0D02 7472 616E 7370 6F73"            /* tions:Â.transpos */
	$"6528 5129 2020 2074 7261 6E73 706F 7365"            /* e(Q)   transpose */
	$"2074 6865 2064 6174 6173 6574 0D02 6474"            /*  the datasetÂ.dt */
	$"6F72 2851 2920 2020 2020 2020 2020 2020"            /* or(Q)            */
	$"2063 6F6E 7665 7274 2064 6567 7265 6573"            /*  convert degrees */
	$"2074 6F20 7261 6469 616E 730D 0272 746F"            /*  to radiansÂ.rto */
	$"6428 5129 2020 2020 2020 2020 2020 2020"            /* d(Q)             */
	$"636F 6E76 6572 7420 7261 6469 616E 7320"            /* convert radians  */
	$"746F 2064 6567 7265 6573 0D02 7368 6C28"            /* to degreesÂ.shl( */
	$"5129 2020 2020 2020 2020 2020 2020 2020"            /* Q)               */
	$"7368 6966 7420 6461 7461 2074 6F20 7468"            /* shift data to th */
	$"6520 6C65 6674 0D02 7368 7228 5129 2020"            /* e leftÂ.shr(Q)   */
	$"2020 2020 2020 2020 2020 2020 7368 6966"            /*             shif */
	$"7420 6461 7461 2074 6F20 7468 6520 7269"            /* t data to the ri */
	$"6768 740D 0273 6875 2851 2920 2020 2020"            /* ghtÂ.shu(Q)      */
	$"2020 2020 2020 2020 2073 6869 6674 2064"            /*          shift d */
	$"6174 6120 7570 0D02 7368 6428 5129 2020"            /* ata upÂ.shd(Q)   */
	$"2020 2020 2020 2020 2020 2020 7368 6966"            /*             shif */
	$"7420 6461 7461 2064 6F77 6E0D 026D 6561"            /* t data downÂ.mea */
	$"6E28 5129 2020 2020 2020 2020 2020 206D"            /* n(Q)           m */
	$"6561 6E20 6F66 2074 6865 2064 6174 610D"            /* ean of the dataÂ */
	$"0273 6465 7628 5129 2020 2020 2020 2020"            /* .sdev(Q)         */
	$"2020 2020 7374 616E 6461 7264 2064 6576"            /*     standard dev */
	$"6961 7469 6F6E 206F 6620 7468 6520 6461"            /* iation of the da */
	$"7461 0D02 6D69 6E28 5129 2020 2020 2020"            /* taÂ.min(Q)       */
	$"2020 2020 2020 206D 696E 206F 6620 7468"            /*        min of th */
	$"6520 6461 7461 0D02 6D61 7828 5129 2020"            /* e dataÂ.max(Q)   */
	$"2020 2020 2020 2020 2020 206D 6178 206F"            /*            max o */
	$"6620 7468 6520 6461 7461 0D02 7074 7328"            /* f the dataÂ.pts( */
	$"5129 2020 2020 2020 2020 2020 2020 2020"            /* Q)               */
	$"6E75 6D62 6572 206F 6620 6461 7461 2070"            /* number of data p */
	$"6F69 6E74 730D 0263 6F6C 7328 5129 2020"            /* ointsÂ.cols(Q)   */
	$"2020 2020 2020 2020 2020 6E75 6D62 6572"            /*           number */
	$"206F 6620 636F 6C75 6D6E 730D 0272 6F77"            /*  of columnsÂ.row */
	$"7328 5129 2020 2020 2020 2020 2020 206E"            /* s(Q)           n */
	$"756D 6265 7220 6F66 2072 6F77 730D 0272"            /* umber of rowsÂ.r */
	$"6F77 7261 6E67 6528 5129 2020 2020 2072"            /* owrange(Q)     r */
	$"616E 6765 206F 6620 726F 7720 7363 616C"            /* ange of row scal */
	$"6520 7661 6C75 6573 0D02 636F 6C72 616E"            /* e valuesÂ.colran */
	$"6765 2851 2920 2020 2020 2072 616E 6765"            /* ge(Q)      range */
	$"206F 6620 636F 6C75 6D6E 2073 6361 6C65"            /*  of column scale */
	$"2076 616C 7565 730D 0272 6F77 6D65 616E"            /*  valuesÂ.rowmean */
	$"2851 2920 2020 2020 6D65 616E 206F 6620"            /* (Q)     mean of  */
	$"6469 7374 616E 6365 2062 6574 7765 656E"            /* distance between */
	$"2072 6F77 730D 0263 6F6C 6D65 616E 2851"            /*  rowsÂ.colmean(Q */
	$"2920 2020 2020 206D 6561 6E20 6F66 2064"            /* )      mean of d */
	$"6973 7461 6E63 6520 6265 7477 6565 6E20"            /* istance between  */
	$"636F 6C75 6D6E 730D 0272 6F77 7364 6576"            /* columnsÂ.rowsdev */
	$"2851 2920 2020 2020 2073 7464 2E20 6465"            /* (Q)      std. de */
	$"762E 206F 6620 6469 7374 616E 6365 2062"            /* v. of distance b */
	$"6574 7765 656E 2072 6F77 730D 0263 6F6C"            /* etween rowsÂ.col */
	$"7364 6576 2851 2920 2020 2020 2020 7374"            /* sdev(Q)       st */
	$"642E 2064 6576 2E20 6F66 2064 6973 742E"            /* d. dev. of dist. */
	$"2062 6574 7765 656E 2063 6F6C 756D 6E73"            /*  between columns */
	$"0D02 0D02 342E 2020 4469 7363 7265 7465"            /* Â.Â.4.  Discrete */
	$"2064 6966 6665 7265 6E63 6573 2061 6E64"            /*  differences and */
	$"2063 6F6E 766F 6C75 7469 6F6E 733A 0D02"            /*  convolutions:Â. */
	$"6464 7828 5129 2020 2020 2020 2020 2020"            /* ddx(Q)           */
	$"2020 6469 6666 6572 656E 6365 2066 726F"            /*   difference fro */
	$"6D20 6C65 6674 2074 6F20 7269 6768 740D"            /* m left to rightÂ */
	$"0264 6479 2851 2920 2020 2020 2020 2020"            /* .ddy(Q)          */
	$"2020 2064 6966 6665 7265 6E63 6520 6672"            /*    difference fr */
	$"6F6D 2074 6F70 2074 6F20 626F 7474 6F6D"            /* om top to bottom */
	$"0D02 6432 6478 2851 2920 2020 2020 2020"            /* Â.d2dx(Q)        */
	$"2020 2032 6E64 2064 6572 6976 2E20 6672"            /*    2nd deriv. fr */
	$"6F6D 206C 6566 7420 746F 2072 6967 6874"            /* om left to right */
	$"0D02 6432 6479 2851 2920 2020 2020 2020"            /* Â.d2dy(Q)        */
	$"2020 2032 6E64 2064 6572 6976 2E20 6672"            /*    2nd deriv. fr */
	$"6F6D 2074 6F70 2074 6F20 626F 7474 6F6D"            /* om top to bottom */
	$"0D02 6C61 7028 5129 2020 2020 2020 2020"            /* Â.lap(Q)         */
	$"2020 2020 352D 706F 696E 7420 4C61 706C"            /*     5-point Lapl */
	$"6163 6961 6E0D 026C 6170 3528 5129 2020"            /* acianÂ.lap5(Q)   */
	$"2020 2020 2020 2020 7361 6D65 2061 7320"            /*         same as  */
	$"6C61 7028 5129 0D02 6C61 7039 2851 2920"            /* lap(Q)Â.lap9(Q)  */
	$"2020 2020 2020 2020 2039 2D70 6F69 6E74"            /*          9-point */
	$"204C 6170 6C61 6369 616E 0D02 6B65 726E"            /*  LaplacianÂ.kern */
	$"656C 2851 2C50 2920 2020 2063 6F6E 766F"            /* el(Q,P)    convo */
	$"6C75 7469 6F6E 206B 6572 6E65 6C20 6F70"            /* lution kernel op */
	$"6572 6174 696F 6E2C 2077 6865 7265 0D02"            /* eration, whereÂ. */
	$"2020 2020 2020 2020 2020 2020 2020 2020"            /*                  */
	$"2020 2020 2050 2069 7320 6120 636F 6E76"            /*      P is a conv */
	$"6F6C 7574 696F 6E20 6B65 726E 656C 2061"            /* olution kernel a */
	$"7320 6465 6669 6E65 6420 696E 0D02 2020"            /* s defined inÂ.   */
	$"2020 2020 2020 2020 2020 2020 2020 2020"            /*                  */
	$"2020 2074 6865 206D 616E 7561 6C2E 2020"            /*    the manual.   */
	$"5020 6D61 7920 6265 2033 7833 2C20 3578"            /* P may be 3x3, 5x */
	$"352C 206F 7220 3778 372E 0D02 0D"                   /* 5, or 7x7.Â.Â */
};

data 'HTXT' (12, ".....External (user-written) Functions\n", purgeable) {
	$"020D 0253 7570 706F 7365 2074 6861 7420"            /* .Â.Suppose that  */
	$"5120 6973 2061 2044 6174 6153 636F 7065"            /* Q is a DataScope */
	$"2064 6174 6173 6574 2061 6E64 2074 6865"            /*  dataset and the */
	$"200D 0266 756E 6374 696F 6E20 6D79 6675"            /*  Â.function myfu */
	$"6E63 7469 6F6E 2069 7320 746F 2062 6520"            /* nction is to be  */
	$"6170 706C 6965 6420 746F 2069 7420 2877"            /* applied to it (w */
	$"6865 7265 0D02 6D79 6675 6E63 7469 6F6E"            /* hereÂ.myfunction */
	$"2069 7320 6578 7465 726E 616C 2074 6F20"            /*  is external to  */
	$"4461 7461 5363 6F70 652C 2065 6974 6865"            /* DataScope, eithe */
	$"7220 6F6E 2074 6865 0D02 4D61 6369 6E74"            /* r on theÂ.Macint */
	$"6F73 6820 6F72 206F 6E20 6120 7265 6D6F"            /* osh or on a remo */
	$"7465 2068 6F73 743B 2072 6566 6572 2074"            /* te host; refer t */
	$"6F20 7468 6520 6D61 6E75 616C 0D02 746F"            /* o the manualÂ.to */
	$"2066 696E 6420 6F75 7420 686F 7720 746F"            /*  find out how to */
	$"2063 6F6E 7374 7275 6374 2061 6E20 6578"            /*  construct an ex */
	$"7465 726E 616C 2066 756E 6374 696F 6E0D"            /* ternal functionÂ */
	$"026C 6962 7261 7279 292E 2020 5468 656E"            /* .library).  Then */
	$"2069 6E20 6F72 6465 7220 746F 2063 7265"            /*  in order to cre */
	$"6174 6520 7468 6520 6E65 7720 6461 7461"            /* ate the new data */
	$"7365 740D 024E 4557 2C20 656E 7465 7220"            /* setÂ.NEW, enter  */
	$"696E 2074 6865 206E 6F74 6562 6F6F 6B0D"            /* in the notebookÂ */
	$"0220 2020 2020 4E45 5720 3D20 6D79 6675"            /* .     NEW = myfu */
	$"6E63 7469 6F6E 2851 290D 0261 6E64 2065"            /* nction(Q)Â.and e */
	$"7865 6375 7465 2074 6865 2065 7175 6174"            /* xecute the equat */
	$"696F 6E2E 2020 4461 7461 5363 6F70 6520"            /* ion.  DataScope  */
	$"7769 6C6C 2070 726F 6D70 740D 0279 6F75"            /* will promptÂ.you */
	$"2074 6F20 7370 6563 6966 7920 7468 6520"            /*  to specify the  */
	$"6C6F 6361 7469 6F6E 206F 6620 7468 6520"            /* location of the  */
	$"6578 7465 726E 616C 2066 756E 6374 696F"            /* external functio */
	$"6E0D 026C 6962 7261 7279 2E20 2054 6865"            /* nÂ.library.  The */
	$"2066 756E 6374 696F 6E20 6D79 6675 6E63"            /*  function myfunc */
	$"7469 6F6E 2063 616E 2074 6865 6E20 6265"            /* tion can then be */
	$"2075 7365 640D 0277 6974 6869 6E20 4461"            /*  usedÂ.within Da */
	$"7461 5363 6F70 6520 6A75 7374 2061 7320"            /* taScope just as  */
	$"7468 6F75 6768 2069 7420 7765 7265 2061"            /* though it were a */
	$"2062 7569 6C74 696E 0D02 6675 6E63 7469"            /*  builtinÂ.functi */
	$"6F6E 2E0D 020D"                                     /* on.Â.Â */
};

data 'HTXT' (13, "File Menu\n", purgeable) {
	$"020D 0254 6865 2066 696C 6520 6D65 6E75"            /* .Â.The file menu */
	$"2068 616E 646C 6573 2074 6865 2066 756E"            /*  handles the fun */
	$"6374 696F 6E73 206F 7065 6E2C 2063 6C6F"            /* ctions open, clo */
	$"7365 2C0D 0273 6176 652C 206C 6F61 6420"            /* se,Â.save, load  */
	$"7061 6C65 7474 652C 206C 6F61 6420 7465"            /* palette, load te */
	$"7874 2C20 7072 696E 742C 2061 6E64 2071"            /* xt, print, and q */
	$"7569 742E 0D02 0D"                                  /* uit.Â.Â */
};

data 'HTXT' (14, ".....Open\n", purgeable) {
	$"020D 0254 6869 7320 636F 6D6D 616E 6420"            /* .Â.This command  */
	$"7072 6F6D 7074 7320 7468 6520 7573 6572"            /* prompts the user */
	$"2074 6F20 7370 6563 6966 7920 616E 2048"            /*  to specify an H */
	$"4446 0D02 6669 6C65 2063 6F6E 7461 696E"            /* DFÂ.file contain */
	$"696E 6720 3244 2034 2D62 7974 6520 666C"            /* ing 2D 4-byte fl */
	$"6F61 7469 6E67 2070 6F69 6E74 2064 6174"            /* oating point dat */
	$"612E 2020 4966 0D02 7468 6520 6669 6C65"            /* a.  IfÂ.the file */
	$"2063 6F6E 7461 696E 7320 6120 7061 6C65"            /*  contains a pale */
	$"7474 652C 2069 7420 7769 6C6C 2062 6520"            /* tte, it will be  */
	$"7573 6564 2028 6966 206E 6F74 2C0D 0261"            /* used (if not,Â.a */
	$"2064 6566 6175 6C74 2070 616C 6574 7465"            /*  default palette */
	$"2077 696C 6C20 6265 2075 7365 6429 2E20"            /*  will be used).  */
	$"2054 6865 2066 696C 6520 6D61 7920 616C"            /*  The file may al */
	$"736F 0D02 636F 6E74 6169 6E20 616E 2069"            /* soÂ.contain an i */
	$"6D61 6765 2028 666F 7220 6469 7370 6C61"            /* mage (for displa */
	$"7920 7075 7270 6F73 6573 206F 6E6C 793B"            /* y purposes only; */
	$"200D 0244 6174 6153 636F 7065 206F 7065"            /*  Â.DataScope ope */
	$"7261 7465 7320 6F6E 206F 6E6C 7920 7468"            /* rates on only th */
	$"6520 666C 6F61 7469 6E67 2070 6F69 6E74"            /* e floating point */
	$"2064 6174 6129 0D02 616E 6420 616E 6E6F"            /*  data)Â.and anno */
	$"7461 7469 6F6E 7320 2866 6F72 2065 7861"            /* tations (for exa */
	$"6D70 6C65 2C20 6120 4461 7461 5363 6F70"            /* mple, a DataScop */
	$"650D 026E 6F74 6562 6F6F 6B20 7361 7665"            /* eÂ.notebook save */
	$"6420 6672 6F6D 2061 2070 7265 7669 6F75"            /* d from a previou */
	$"7320 7365 7373 696F 6E29 2E0D 020D"                 /* s session).Â.Â */
};

data 'HTXT' (15, "..... Close\n", purgeable) {
	$"020D 0254 6869 7320 636F 6D6D 616E 6420"            /* .Â.This command  */
	$"7072 6F6D 7074 7320 7468 6520 7573 6572"            /* prompts the user */
	$"2074 6F20 7361 7665 2074 6865 2064 6174"            /*  to save the dat */
	$"610D 0270 6572 7461 696E 696E 6720 7468"            /* aÂ.pertaining th */
	$"6520 6672 6F6E 746D 6F73 7420 7769 6E64"            /* e frontmost wind */
	$"6F77 2061 6E64 2074 6865 6E20 7265 6D6F"            /* ow and then remo */
	$"7665 730D 0261 6C6C 2061 7373 6F63 6961"            /* vesÂ.all associa */
	$"7465 6420 7769 6E64 6F77 7320 2865 672C"            /* ted windows (eg, */
	$"2069 6D61 6765 7320 616E 6420 7468 650D"            /*  images and theÂ */
	$"026E 6F74 6562 6F6F 6B29 2066 726F 6D20"            /* .notebook) from  */
	$"4461 7461 5363 6F70 652E 0D02 0D"                   /* DataScope.Â.Â */
};

data 'HTXT' (16, ".....Save\n", purgeable) {
	$"020D 0254 6869 7320 636F 6D6D 616E 6420"            /* .Â.This command  */
	$"7361 7665 7320 7468 6520 6461 7461 2061"            /* saves the data a */
	$"7373 6F63 6961 7465 6420 7769 7468 2074"            /* ssociated with t */
	$"6865 0D02 6672 6F6E 746D 6F73 7420 7769"            /* heÂ.frontmost wi */
	$"6E64 6F77 2069 6E74 6F20 616E 2048 4446"            /* ndow into an HDF */
	$"2066 696C 6520 7370 6563 6966 6965 6420"            /*  file specified  */
	$"6279 2074 6865 0D02 7573 6572 2E20 2054"            /* by theÂ.user.  T */
	$"6865 2066 726F 6E74 6D6F 7374 2077 696E"            /* he frontmost win */
	$"646F 7720 6973 206E 6F74 2063 6C6F 7365"            /* dow is not close */
	$"6420 6F72 0D02 6D6F 6469 6669 6564 2062"            /* d orÂ.modified b */
	$"7920 7468 6973 2061 6374 696F 6E2E 0D02"            /* y this action.Â. */
	$"0D"                                                 /* Â */
};

data 'HTXT' (17, ".....Save As\n", purgeable) {
	$"020D 0253 616D 6520 6173 2074 6865 2053"            /* .Â.Same as the S */
	$"6176 6520 636F 6D6D 616E 642C 2062 7574"            /* ave command, but */
	$"2069 7420 6973 2061 7373 756D 6564 2074"            /*  it is assumed t */
	$"6861 740D 0274 6865 2075 7365 7220 7370"            /* hatÂ.the user sp */
	$"6563 6966 6963 616C 6C79 2077 6973 6865"            /* ecifically wishe */
	$"7320 746F 206E 616D 6520 6120 6E65 7720"            /* s to name a new  */
	$"6669 6C65 2069 6E0D 0277 6869 6368 2074"            /* file inÂ.which t */
	$"6F20 7361 7665 2074 6865 2064 6174 612E"            /* o save the data. */
	$"2020 5468 6520 6672 6F6E 746D 6F73 7420"            /*   The frontmost  */
	$"7769 6E64 6F77 2069 7320 0D02 6E6F 7420"            /* window is Â.not  */
	$"636C 6F73 6564 206F 7220 6D6F 6469 6669"            /* closed or modifi */
	$"6564 2062 7920 7468 6973 2061 6374 696F"            /* ed by this actio */
	$"6E2E 0D02 0D"                                       /* n.Â.Â */
};

data 'HTXT' (18, ".....Load Palette\n", purgeable) {
	$"020D 024C 6F61 6420 6120 7061 6C65 7474"            /* .Â.Load a palett */
	$"6520 7374 6F72 6564 2069 6E20 616E 2048"            /* e stored in an H */
	$"4446 2066 696C 6520 2869 6620 7468 6572"            /* DF file (if ther */
	$"6520 6172 650D 026D 756C 7469 706C 6520"            /* e areÂ.multiple  */
	$"7061 6C65 7474 6573 2069 6E20 7468 6520"            /* palettes in the  */
	$"6669 6C65 2C20 6F6E 6C79 2074 6865 2066"            /* file, only the f */
	$"6972 7374 206F 6E65 200D 0277 696C 6C20"            /* irst one Â.will  */
	$"6265 206C 6F61 6465 6429 2E20 2054 6865"            /* be loaded).  The */
	$"2070 616C 6574 7465 2077 696C 6C20 6265"            /*  palette will be */
	$"2061 7070 6C69 6564 206F 6E6C 790D 0274"            /*  applied onlyÂ.t */
	$"6F20 7468 6520 6461 7461 2061 7373 6F63"            /* o the data assoc */
	$"6961 7465 6420 7769 7468 2074 6865 2066"            /* iated with the f */
	$"726F 6E74 6D6F 7374 2077 696E 646F 772E"            /* rontmost window. */
	$"0D02 0D"                                            /* Â.Â */
};

data 'HTXT' (19, ".....Load Text\n", purgeable) {
	$"020D 0241 2074 6578 7420 6669 6C65 2069"            /* .Â.A text file i */
	$"7320 6C6F 6164 6564 2069 6E74 6F20 4461"            /* s loaded into Da */
	$"7461 5363 6F70 6520 6173 2061 2064 6174"            /* taScope as a dat */
	$"6120 6669 6C65 2E0D 0254 6865 2066 6F72"            /* a file.Â.The for */
	$"6D61 7420 6973 200D 020D 026E 726F 7773"            /* mat is Â.Â.nrows */
	$"2020 2020 6E63 6F6C 730D 026D 6178 5F76"            /*     ncolsÂ.max_v */
	$"616C 7565 5F6F 665F 696E 7465 7265 7374"            /* alue_of_interest */
	$"2020 2020 6D69 6E5F 7661 6C75 655F 6F66"            /*     min_value_of */
	$"5F69 6E74 6572 6573 740D 0272 6F77 2031"            /* _interestÂ.row 1 */
	$"0D02 726F 7720 320D 0228 6574 6329 0D02"            /* Â.row 2Â.(etc)Â. */
	$"726F 7720 6E72 6F77 730D 020D 0245 6163"            /* row nrowsÂ.Â.Eac */
	$"6820 726F 7720 6D75 7374 2068 6176 6520"            /* h row must have  */
	$"6E63 6F6C 2065 6E74 7269 6573 2C20 616E"            /* ncol entries, an */
	$"6420 6561 6368 2065 6E74 7279 0D02 6D75"            /* d each entryÂ.mu */
	$"7374 2062 6520 7365 7061 7261 7465 6420"            /* st be separated  */
	$"6279 2062 6C61 6E6B 7320 6F72 2063 6172"            /* by blanks or car */
	$"7269 6167 6520 7265 7475 726E 732E 0D02"            /* riage returns.Â. */
	$"5361 6D70 6C65 3A0D 020D 0234 2020 2020"            /* Sample:Â.Â.4     */
	$"330D 0231 2E20 2020 2020 2020 302E 3030"            /* 3Â.1.       0.00 */
	$"6530 0D02 3120 2020 3020 2020 2020 2020"            /* e0Â.1   0        */
	$"2020 300D 022D 3220 2020 2D31 2E30 3030"            /*   0Â.-2   -1.000 */
	$"2020 2020 2030 2E0D 022D 312E 6530 2020"            /*      0.Â.-1.e0   */
	$"2030 2020 2020 2031 0D02 3020 2020 2032"            /*  0     1Â.0    2 */
	$"2020 2020 310D 020D 024F 6E63 6520 7468"            /*     1Â.Â.Once th */
	$"6973 2074 6578 7420 6669 6C65 2069 7320"            /* is text file is  */
	$"6C6F 6164 6564 2069 6E74 6F20 4461 7461"            /* loaded into Data */
	$"5363 6F70 652C 2069 7420 6973 0D02 7472"            /* Scope, it isÂ.tr */
	$"6561 7465 6420 6C69 6B65 2061 6E79 206F"            /* eated like any o */
	$"7468 6572 2044 6174 6153 636F 7065 2064"            /* ther DataScope d */
	$"6174 6173 6574 2E0D 020D"                           /* ataset.Â.Â */
};

data 'HTXT' (20, ".....Quit\n", purgeable) {
	$"020D 0245 7869 7420 4461 7461 5363 6F70"            /* .Â.Exit DataScop */
	$"652E 2020 5468 6520 7573 6572 2069 7320"            /* e.  The user is  */
	$"7072 6F6D 7074 6564 2061 7320 7768 6574"            /* prompted as whet */
	$"6865 720D 0264 6174 6173 6574 7320 6375"            /* herÂ.datasets cu */
	$"7272 656E 746C 7920 6C6F 6164 6564 2069"            /* rrently loaded i */
	$"6E74 6F20 4461 7461 5363 6F70 6520 6172"            /* nto DataScope ar */
	$"6520 746F 0D02 6265 2073 6176 6564 206F"            /* e toÂ.be saved o */
	$"7220 6E6F 742E 0D02 0D"                             /* r not.Â.Â */
};

data 'HTXT' (21, "Edit Menu\n", purgeable) {
	$"020D 0254 6869 7320 6973 2074 6865 2073"            /* .Â.This is the s */
	$"7461 6E64 6172 6420 4D61 6369 6E74 6F73"            /* tandard Macintos */
	$"6820 6564 6974 206D 656E 7520 666F 720D"            /* h edit menu forÂ */
	$"0263 6C69 7062 6F61 7264 206F 7065 7261"            /* .clipboard opera */
	$"7469 6F6E 732E 0D02 0D"                             /* tions.Â.Â */
};

data 'HTXT' (22, ".....Undo\n", purgeable) {
	$"020D 0254 6865 2055 6E64 6F20 636F 6D6D"            /* .Â.The Undo comm */
	$"616E 6420 6973 2069 6E61 6374 6976 6520"            /* and is inactive  */
	$"696E 2044 6174 6153 636F 7065 2E20 2053"            /* in DataScope.  S */
	$"696E 6365 0D02 4461 7461 5363 6F70 6520"            /* inceÂ.DataScope  */
	$"616C 7761 7973 2063 7265 6174 6573 2061"            /* always creates a */
	$"206E 6577 2064 6174 6173 6574 2028 6E65"            /*  new dataset (ne */
	$"7665 720D 026D 6F64 6966 7969 6E67 2061"            /* verÂ.modifying a */
	$"6E20 6578 6973 7469 6E67 206F 6E65 292C"            /* n existing one), */
	$"2074 6869 7320 6665 6174 7572 6520 6973"            /*  this feature is */
	$"0D02 756E 6E65 6365 7373 6172 792E 0D02"            /* Â.unnecessary.Â. */
	$"0D"                                                 /* Â */
};

data 'HTXT' (23, ".....Cut\n", purgeable) {
	$"020D 0252 656D 6F76 6520 7365 6C65 6374"            /* .Â.Remove select */
	$"6564 2074 6578 7420 6672 6F6D 2061 206E"            /* ed text from a n */
	$"6F74 6562 6F6F 6B20 7769 6E64 6F77 0D02"            /* otebook windowÂ. */
	$"616E 6420 706C 6163 6520 6974 206F 6E20"            /* and place it on  */
	$"7468 6520 636C 6970 626F 6172 642E 0D02"            /* the clipboard.Â. */
	$"0D"                                                 /* Â */
};

data 'HTXT' (24, ".....Copy\n", purgeable) {
	$"020D 0255 7365 2074 6865 2043 6F70 7920"            /* .Â.Use the Copy  */
	$"636F 6D6D 616E 6420 746F 2063 6F70 7920"            /* command to copy  */
	$"616E 7920 6461 7461 2069 6D61 6765 0D02"            /* any data imageÂ. */
	$"2877 6865 7468 6572 2067 656E 6572 6174"            /* (whether generat */
	$"6564 2C20 696E 7465 7270 6F6C 6174 6564"            /* ed, interpolated */
	$"2C20 6F72 2070 6F6C 6172 2920 746F 2074"            /* , or polar) to t */
	$"6865 0D02 436C 6970 626F 6172 642E 2020"            /* heÂ.Clipboard.   */
	$"5468 6520 636F 7069 6564 2069 6D61 6765"            /* The copied image */
	$"2063 616E 2074 6865 6E20 6265 2070 6173"            /*  can then be pas */
	$"7465 640D 0262 6163 6B20 696E 746F 2044"            /* tedÂ.back into D */
	$"6174 6153 636F 7065 2028 6372 6561 7469"            /* ataScope (creati */
	$"6E67 2061 206E 6577 2066 6C6F 6174 696E"            /* ng a new floatin */
	$"6720 706F 696E 740D 0264 6174 6173 6574"            /* g pointÂ.dataset */
	$"292C 206F 7220 7061 7374 6564 2069 6E74"            /* ), or pasted int */
	$"6F20 616E 7920 6170 706C 6963 6174 696F"            /* o any applicatio */
	$"6E20 7072 6F67 7261 6D0D 0274 6861 7420"            /* n programÂ.that  */
	$"7375 7070 6F72 7473 2074 6865 2070 6173"            /* supports the pas */
	$"7465 2063 6F6D 6D61 6E64 2E20 2049 6E20"            /* te command.  In  */
	$"6164 6469 7469 6F6E 2C0D 0274 6865 2066"            /* addition,Â.the f */
	$"6C6F 6174 696E 6720 706F 696E 7420 6E75"            /* loating point nu */
	$"6D62 6572 7320 6361 6E20 6265 2063 6F70"            /* mbers can be cop */
	$"6965 6420 746F 2074 6865 0D02 636C 6970"            /* ied to theÂ.clip */
	$"626F 6172 6420 616E 6420 7061 7374 6564"            /* board and pasted */
	$"2069 6E74 6F20 616E 7920 7465 7874 2D70"            /*  into any text-p */
	$"726F 6365 7373 696E 670D 0261 7070 6C69"            /* rocessingÂ.appli */
	$"6361 7469 6F6E 2074 6861 7420 7375 7070"            /* cation that supp */
	$"6F72 7473 2074 6865 2070 6173 7465 206F"            /* orts the paste o */
	$"7074 696F 6E20 2874 6865 0D02 6461 7461"            /* ption (theÂ.data */
	$"2077 696C 6C20 6265 2063 6F70 6965 6420"            /*  will be copied  */
	$"696E 2074 6865 2066 6F72 6D61 7420 696E"            /* in the format in */
	$"2077 6869 6368 2074 6865 790D 0261 7265"            /*  which theyÂ.are */
	$"2064 6973 706C 6179 6564 2C20 616E 6420"            /*  displayed, and  */
	$"7468 6973 2069 7320 7573 6572 2D73 6574"            /* this is user-set */
	$"7461 626C 6529 2E0D 020D"                           /* table).Â.Â */
};

data 'HTXT' (25, ".....Paste\n", purgeable) {
	$"020D 0255 7365 2074 6865 2050 6173 7465"            /* .Â.Use the Paste */
	$"2063 6F6D 6D61 6E64 2074 6F20 7061 7374"            /*  command to past */
	$"6520 616E 7920 6461 7461 2069 6D61 6765"            /* e any data image */
	$"0D02 2877 6865 7468 6572 2063 6F70 6965"            /* Â.(whether copie */
	$"6420 6672 6F6D 2044 6174 6153 636F 7065"            /* d from DataScope */
	$"206F 7220 6672 6F6D 2061 6E79 206F 7468"            /*  or from any oth */
	$"6572 0D02 6170 706C 6963 6174 696F 6E20"            /* erÂ.application  */
	$"7468 6174 2073 7570 706F 7274 7320 7468"            /* that supports th */
	$"6520 436F 7079 2063 6F6D 6D61 6E64 2920"            /* e Copy command)  */
	$"696E 746F 2061 0D02 6E65 7720 666C 6F61"            /* into aÂ.new floa */
	$"7469 6E67 2070 6F69 6E74 2064 6174 6173"            /* ting point datas */
	$"6574 2E20 2049 6E20 6164 6469 7469 6F6E"            /* et.  In addition */
	$"2C20 7465 7874 2061 7272 6179 730D 0263"            /* , text arraysÂ.c */
	$"616E 2062 6520 7061 7374 6564 2066 726F"            /* an be pasted fro */
	$"6D20 7468 6520 636C 6970 626F 6172 642E"            /* m the clipboard. */
	$"2020 5468 6520 666F 726D 6174 206D 7573"            /*   The format mus */
	$"7420 0D02 6265 0D02 0D02 726F 7720 310D"            /* t Â.beÂ.Â.row 1Â */
	$"0272 6F77 2032 0D02 2865 7463 290D 0272"            /* .row 2Â.(etc)Â.r */
	$"6F77 206E 726F 7773 0D02 0D02 4561 6368"            /* ow nrowsÂ.Â.Each */
	$"2072 6F77 206D 7573 7420 6861 7665 206E"            /*  row must have n */
	$"636F 6C20 636F 6C75 6D6E 732C 2061 6E64"            /* col columns, and */
	$"2065 6E74 7269 6573 206D 7573 740D 0262"            /*  entries mustÂ.b */
	$"6520 7365 7061 7261 7465 6420 6279 2062"            /* e separated by b */
	$"6C61 6E6B 732E 2020 5361 6D70 6C65 3A0D"            /* lanks.  Sample:Â */
	$"020D 0231 2020 2030 2020 2020 2020 2020"            /* .Â.1   0         */
	$"2030 0D02 2D32 2020 202D 312E 3030 3020"            /*  0Â.-2   -1.000  */
	$"2020 2020 302E 0D02 2D31 2E65 3020 2020"            /*     0.Â.-1.e0    */
	$"3020 2020 2020 310D 0230 2020 2020 3220"            /* 0     1Â.0    2  */
	$"2020 2031 0D02 0D"                                  /*    1Â.Â */
};

data 'HTXT' (26, ".....Clear\n", purgeable) {
	$"020D 0252 656D 6F76 6520 7365 6C65 6374"            /* .Â.Remove select */
	$"6564 2074 6578 7420 6672 6F6D 2061 206E"            /* ed text from a n */
	$"6F74 6562 6F6F 6B20 7769 6E64 6F77 2E0D"            /* otebook window.Â */
	$"020D"                                               /* .Â */
};

data 'HTXT' (27, "Image Menu\n", purgeable) {
	$"020D 0254 6865 2049 6D61 6765 206D 656E"            /* .Â.The Image men */
	$"7520 6765 6E65 7261 7465 7320 696D 6167"            /* u generates imag */
	$"6573 2028 696E 7465 7270 6F6C 6174 6564"            /* es (interpolated */
	$"206F 720D 0275 6E69 6E74 6572 706F 6C61"            /*  orÂ.uninterpola */
	$"7465 6420 666F 7220 6361 7274 6573 6961"            /* ted for cartesia */
	$"6E20 6461 7461 2C20 6173 2077 656C 6C20"            /* n data, as well  */
	$"6173 0D02 6765 6E65 7261 7469 6E67 2070"            /* asÂ.generating p */
	$"6F6C 6172 2069 6D61 6765 7329 2061 6E64"            /* olar images) and */
	$"2061 6C6C 6F77 7320 7468 6520 7365 7474"            /*  allows the sett */
	$"696E 6720 6F66 0D02 696D 6167 6520 6D61"            /* ing ofÂ.image ma */
	$"676E 6966 6963 6174 696F 6E2E 0D02 0D"              /* gnification.Â.Â */
};

data 'HTXT' (28, ".....Generate Image\n", purgeable) {
	$"020D 0247 656E 6572 6174 6520 6120 7363"            /* .Â.Generate a sc */
	$"616C 6564 2069 6D61 6765 2066 726F 6D20"            /* aled image from  */
	$"7468 6520 666C 6F61 7469 6E67 2070 6F69"            /* the floating poi */
	$"6E74 0D02 6461 7461 2C20 6261 7365 6420"            /* ntÂ.data, based  */
	$"6F6E 2074 6865 206D 6178 696D 756D 2061"            /* on the maximum a */
	$"6E64 206D 696E 696D 756D 2076 616C 7565"            /* nd minimum value */
	$"730D 026F 6620 696E 7465 7265 7374 2061"            /* sÂ.of interest a */
	$"6E64 206F 6E20 7468 6520 7261 6E67 6520"            /* nd on the range  */
	$"6F66 2070 616C 6574 7465 2063 6F6C 6F72"            /* of palette color */
	$"732E 2020 4966 0D02 7468 6520 696D 6167"            /* s.  IfÂ.the imag */
	$"6520 7369 7A65 2069 7320 6E6F 7420 312C"            /* e size is not 1, */
	$"2069 6D61 6765 7320 6172 6520 6D61 676E"            /*  images are magn */
	$"6966 6965 6420 6275 740D 026E 6F74 2069"            /* ified butÂ.not i */
	$"6E74 6572 706F 6C61 7465 642E 2020 5468"            /* nterpolated.  Th */
	$"6520 6461 7461 2069 7320 6173 7375 6D65"            /* e data is assume */
	$"6420 746F 2062 650D 0263 6172 7465 7369"            /* d to beÂ.cartesi */
	$"616E 2E0D 020D"                                     /* an.Â.Â */
};

data 'HTXT' (29, ".....Image Size\n", purgeable) {
	$"020D 0253 6574 2074 6865 2068 6F72 697A"            /* .Â.Set the horiz */
	$"6F6E 7461 6C20 616E 6420 7665 7274 6963"            /* ontal and vertic */
	$"616C 206D 6167 6E69 6669 6361 7469 6F6E"            /* al magnification */
	$"2075 7365 640D 0269 6E20 7468 6520 4765"            /*  usedÂ.in the Ge */
	$"6E65 7261 7465 2049 6D61 6765 2063 6F6D"            /* nerate Image com */
	$"6D61 6E64 2E0D 020D"                                /* mand.Â.Â */
};

data 'HTXT' (30, ".....Interpolated Image\n", purgeable) {
	$"020D 0247 656E 6572 6174 6520 6120 7363"            /* .Â.Generate a sc */
	$"616C 6564 2069 6D61 6765 2066 726F 6D20"            /* aled image from  */
	$"7468 6520 666C 6F61 7469 6E67 2070 6F69"            /* the floating poi */
	$"6E74 0D02 6461 7461 2C20 6261 7365 6420"            /* ntÂ.data, based  */
	$"6F6E 2074 6865 206D 6178 696D 756D 2061"            /* on the maximum a */
	$"6E64 206D 696E 696D 756D 2076 616C 7565"            /* nd minimum value */
	$"730D 026F 6620 696E 7465 7265 7374 2061"            /* sÂ.of interest a */
	$"6E64 206F 6E20 7468 6520 7261 6E67 6520"            /* nd on the range  */
	$"6F66 2070 616C 6574 7465 2063 6F6C 6F72"            /* of palette color */
	$"732E 2020 4966 0D02 7468 6520 696D 6167"            /* s.  IfÂ.the imag */
	$"6520 7369 7A65 2069 7320 6E6F 7420 312C"            /* e size is not 1, */
	$"2062 696C 696E 6561 7220 696E 7465 7270"            /*  bilinear interp */
	$"6F6C 6174 696F 6E20 6973 0D02 7573 6564"            /* olation isÂ.used */
	$"2074 6F20 6D61 676E 6966 7920 7468 6520"            /*  to magnify the  */
	$"696D 6167 652E 2020 5468 6520 6461 7461"            /* image.  The data */
	$"2069 7320 6173 7375 6D65 640D 0274 6F20"            /*  is assumedÂ.to  */
	$"6265 2063 6172 7465 7369 616E 2E0D 020D"            /* be cartesian.Â.Â */
};

data 'HTXT' (31, ".....Interpolate Size\n", purgeable) {
	$"020D 0253 6574 2074 6865 2068 6F72 697A"            /* .Â.Set the horiz */
	$"6F6E 7461 6C20 616E 6420 7665 7274 6963"            /* ontal and vertic */
	$"616C 206D 6167 6E69 6669 6361 7469 6F6E"            /* al magnification */
	$"2075 7365 640D 0269 6E20 7468 6520 496E"            /*  usedÂ.in the In */
	$"7465 7270 6F6C 6174 6564 2049 6D61 6765"            /* terpolated Image */
	$"2063 6F6D 6D61 6E64 2E0D 020D"                      /*  command.Â.Â */
};

data 'HTXT' (32, ".....Polar Image\n", purgeable) {
	$"020D 0247 656E 6572 6174 6520 6120 7363"            /* .Â.Generate a sc */
	$"616C 6564 2069 6D61 6765 2066 726F 6D20"            /* aled image from  */
	$"7468 6520 666C 6F61 7469 6E67 2070 6F69"            /* the floating poi */
	$"6E74 0D02 6461 7461 2C20 6261 7365 6420"            /* ntÂ.data, based  */
	$"6F6E 2074 6865 206D 6178 696D 756D 2061"            /* on the maximum a */
	$"6E64 206D 696E 696D 756D 2076 616C 7565"            /* nd minimum value */
	$"730D 026F 6620 696E 7465 7265 7374 2061"            /* sÂ.of interest a */
	$"6E64 206F 6E20 7468 6520 7261 6E67 6520"            /* nd on the range  */
	$"6F66 2070 616C 6574 7465 2063 6F6C 6F72"            /* of palette color */
	$"732E 0D02 5468 6520 6461 7461 2069 7320"            /* s.Â.The data is  */
	$"6173 7375 6D65 6420 746F 2062 6520 706F"            /* assumed to be po */
	$"6C61 722C 2077 6974 6820 7261 6469 6920"            /* lar, with radii  */
	$"7374 6F72 6564 0D02 696E 2074 6865 2072"            /* storedÂ.in the r */
	$"6F77 2068 6561 6469 6E67 7320 616E 6420"            /* ow headings and  */
	$"616E 676C 6573 2073 746F 7265 6420 696E"            /* angles stored in */
	$"2074 6865 0D02 636F 6C75 6D6E 2068 6561"            /*  theÂ.column hea */
	$"6469 6E67 732E 2020 416E 676C 6573 2073"            /* dings.  Angles s */
	$"686F 756C 6420 6265 2062 6574 7765 656E"            /* hould be between */
	$"0D02 3020 616E 6420 32B9 2E0D 020D"                 /* Â.0 and 2¹.Â.Â */
};

data 'HTXT' (33, ".....Polar Size\n", purgeable) {
	$"020D 0253 6574 2074 6865 2072 6164 6961"            /* .Â.Set the radia */
	$"6C20 616E 6420 616E 6775 6C61 7220 6D61"            /* l and angular ma */
	$"676E 6966 6963 6174 696F 6E20 616E 6420"            /* gnification and  */
	$"7468 6520 0D02 6F72 6965 6E74 6174 696F"            /* the Â.orientatio */
	$"6E20 6F66 2074 6865 2064 6174 6120 2865"            /* n of the data (e */
	$"672C 2022 6561 7374 2220 6D61 7920 6265"            /* g, "east" may be */
	$"2020 7468 650D 0264 6972 6563 7469 6F6E"            /*   theÂ.direction */
	$"206F 6620 7A65 726F 2061 6E67 6C65 2C20"            /*  of zero angle,  */
	$"6F72 2074 6865 2075 7365 7220 6D61 7920"            /* or the user may  */
	$"7370 6563 6966 790D 0273 6F6D 6520 6F74"            /* specifyÂ.some ot */
	$"6865 7220 6469 7265 6374 696F 6E20 746F"            /* her direction to */
	$"2062 6520 7468 6520 7A65 726F 2061 6E67"            /*  be the zero ang */
	$"6C65 292E 0D02 0D"                                  /* le).Â.Â */
};

data 'HTXT' (34, "Numbers Menu\n", purgeable) {
	$"020D 0254 6865 204E 756D 6265 7273 206D"            /* .Â.The Numbers m */
	$"656E 7520 616C 6C6F 7773 2074 6865 2075"            /* enu allows the u */
	$"7365 7220 746F 2076 6965 7720 616E 6420"            /* ser to view and  */
	$"7365 740D 0264 6174 6173 6574 2061 7474"            /* setÂ.dataset att */
	$"7269 6275 7465 732C 2073 796E 6368 726F"            /* ributes, synchro */
	$"6E69 7A65 2074 6865 2076 6965 7769 6E67"            /* nize the viewing */
	$"206F 6620 0D02 6469 6666 6572 656E 7420"            /*  of Â.different  */
	$"6461 7461 7365 7473 2C20 6578 7472 6163"            /* datasets, extrac */
	$"7420 7265 6374 616E 6775 6C61 7220 7375"            /* t rectangular su */
	$"6272 6567 696F 6E73 0D02 6F66 2064 6174"            /* bregionsÂ.of dat */
	$"6173 6574 7320 616E 6420 6C6F 6164 2074"            /* asets and load t */
	$"686F 7365 2073 7562 7265 6769 6F6E 7320"            /* hose subregions  */
	$"6173 206E 6577 0D02 4461 7461 5363 6F70"            /* as newÂ.DataScop */
	$"6520 6461 7461 7365 7473 2C20 616E 6420"            /* e datasets, and  */
	$"7065 7266 6F72 6D20 6F70 6572 6174 696F"            /* perform operatio */
	$"6E73 206F 6E0D 0264 6174 6120 7573 696E"            /* ns onÂ.data usin */
	$"6720 6120 6E6F 7465 626F 6F6B 2E0D 020D"            /* g a notebook.Â.Â */
};

data 'HTXT' (35, ".....Attributes\n", purgeable) {
	$"020D 0256 6965 7720 616E 6420 7370 6563"            /* .Â.View and spec */
	$"6966 7920 7468 6520 666F 6C6C 6F77 696E"            /* ify the followin */
	$"673A 2020 6461 7461 206C 6162 656C 732C"            /* g:  data labels, */
	$"2061 7869 730D 026C 6162 656C 732C 2064"            /*  axisÂ.labels, d */
	$"6973 706C 6179 2066 6F72 6D61 7473 2C20"            /* isplay formats,  */
	$"6D69 6E20 616E 6420 6D61 7820 7661 6C75"            /* min and max valu */
	$"6573 206F 6620 0D02 696E 7465 7265 7374"            /* es of Â.interest */
	$"2074 6F20 7468 6520 7573 6572 2077 6974"            /*  to the user wit */
	$"6869 6E20 7468 6520 6461 7461 2C20 616E"            /* hin the data, an */
	$"6420 7468 6520 7261 6E67 650D 026F 6620"            /* d the rangeÂ.of  */
	$"636F 6C6F 7273 2028 312D 3235 342C 206F"            /* colors (1-254, o */
	$"7220 616E 7920 7375 6273 6574 2920 7573"            /* r any subset) us */
	$"6564 2069 6E0D 0267 656E 6572 6174 696E"            /* ed inÂ.generatin */
	$"6720 696D 6167 6573 2E20 2054 6865 2064"            /* g images.  The d */
	$"6174 6173 6574 7320 6469 6D65 6E73 696F"            /* atasets dimensio */
	$"6E73 2061 7265 0D02 6469 7370 6C61 7965"            /* ns areÂ.displaye */
	$"642E 0D02 0D"                                       /* d.Â.Â */
};

data 'HTXT' (36, ".....Synchronize\n", purgeable) {
	$"020D 0253 796E 6368 726F 6E69 7A65 2074"            /* .Â.Synchronize t */
	$"6865 2076 6965 7769 6E67 206F 6620 6469"            /* he viewing of di */
	$"6666 6572 656E 7420 6461 7461 7365 7473"            /* fferent datasets */
	$"2069 6E0D 0244 6174 6153 636F 7065 2E20"            /*  inÂ.DataScope.  */
	$"2046 6F72 2065 7861 6D70 6C65 2C20 7768"            /*  For example, wh */
	$"656E 2061 2073 7562 7265 6769 6F6E 206F"            /* en a subregion o */
	$"6620 6F6E 650D 0264 6174 6173 6574 2069"            /* f oneÂ.dataset i */
	$"7320 6869 6C69 7465 6420 7769 7468 2074"            /* s hilited with t */
	$"6865 2073 796E 6368 726F 6E69 7A65 206F"            /* he synchronize o */
	$"7074 696F 6E0D 0273 656C 6563 7465 642C"            /* ptionÂ.selected, */
	$"2061 6C6C 206F 7468 6572 2064 6174 6173"            /*  all other datas */
	$"6574 7320 7468 6174 2068 6176 6520 7468"            /* ets that have th */
	$"6520 7361 6D65 0D02 756C 7469 6D61 7465"            /* e sameÂ.ultimate */
	$"206F 7269 6769 6E73 2028 6567 2C20 7468"            /*  origins (eg, th */
	$"6F73 6520 6461 7461 7365 7473 2074 6861"            /* ose datasets tha */
	$"7420 6172 650D 0272 656C 6174 6564 2062"            /* t areÂ.related b */
	$"6563 6175 7365 2074 6865 7920 7765 7265"            /* ecause they were */
	$"2067 656E 6572 6174 6564 2062 7920 6170"            /*  generated by ap */
	$"706C 7969 6E67 0D02 6469 6666 6572 656E"            /* plyingÂ.differen */
	$"7420 6E6F 7465 626F 6F6B 2066 756E 6374"            /* t notebook funct */
	$"696F 6E73 2074 6F20 7468 6520 7361 6D65"            /* ions to the same */
	$"206F 7269 6769 6E61 6C0D 0264 6174 6173"            /*  originalÂ.datas */
	$"6574 2920 6861 7665 2074 6865 2063 6F72"            /* et) have the cor */
	$"7265 7370 6F6E 6469 6E67 2073 7562 7265"            /* responding subre */
	$"6769 6F6E 0D02 6869 6C69 7465 642E 0D02"            /* gionÂ.hilited.Â. */
	$"0D"                                                 /* Â */
};

data 'HTXT' (37, ".....Extract Selection\n", purgeable) {
	$"020D 0245 7874 7261 6374 2061 2068 696C"            /* .Â.Extract a hil */
	$"6974 6564 2073 7562 7265 6769 6F6E 206F"            /* ited subregion o */
	$"6620 6120 6461 7461 7365 7420 616E 6420"            /* f a dataset and  */
	$"6372 6561 7465 0D02 6120 6E65 7720 4461"            /* createÂ.a new Da */
	$"7461 5363 6F70 6520 6461 7461 7365 7420"            /* taScope dataset  */
	$"6672 6F6D 2074 6861 7420 7375 6272 6567"            /* from that subreg */
	$"696F 6E2E 0D02 0D"                                  /* ion.Â.Â */
};

data 'HTXT' (38, ".....See Notebook\n", purgeable) {
	$"020D 0244 6973 706C 6179 2074 6865 2044"            /* .Â.Display the D */
	$"6174 6153 636F 7065 206E 6F74 6562 6F6F"            /* ataScope noteboo */
	$"6B20 6F6E 2074 6865 2073 6372 6565 6E2E"            /* k on the screen. */
	$"0D02 5768 656E 2074 6865 2075 7365 7220"            /* Â.When the user  */
	$"7265 6D6F 7665 7320 7468 6520 6E6F 7465"            /* removes the note */
	$"626F 6F6B 2066 726F 6D20 7468 650D 0273"            /* book from theÂ.s */
	$"6372 6565 6E2C 2074 6865 2069 6E66 6F72"            /* creen, the infor */
	$"6D61 7469 6F6E 2069 6E20 7468 6174 206E"            /* mation in that n */
	$"6F74 6562 6F6F 6B20 6973 206E 6F74 0D02"            /* otebook is notÂ. */
	$"6C6F 7374 2E0D 020D"                                /* lost.Â.Â */
};

data 'HTXT' (39, ".....Calculate From Notes\n", purgeable) {
	$"020D 0250 6572 666F 726D 2074 6865 206F"            /* .Â.Perform the o */
	$"7065 7261 7469 6F6E 7320 746F 2063 7265"            /* perations to cre */
	$"6174 6520 6120 6E65 7720 6461 7461 7365"            /* ate a new datase */
	$"740D 0266 726F 6D20 7468 6520 6571 7561"            /* tÂ.from the equa */
	$"7469 6F6E 2065 7865 6375 7465 642E 2020"            /* tion executed.   */
	$"546F 2063 616C 6375 6C61 7465 2C20 7772"            /* To calculate, wr */
	$"6974 650D 0274 6865 2065 7175 6174 696F"            /* iteÂ.the equatio */
	$"6E2C 2068 696C 6974 6520 6974 2C20 616E"            /* n, hilite it, an */
	$"6420 7365 6C65 6374 2074 6869 7320 6F70"            /* d select this op */
	$"7469 6F6E 2066 726F 6D0D 0274 6865 206D"            /* tion fromÂ.the m */
	$"656E 752E 2020 4966 2074 6865 206F 7074"            /* enu.  If the opt */
	$"696F 6E20 6973 2073 656C 6563 7465 6420"            /* ion is selected  */
	$"7768 656E 206E 6F20 7465 7874 0D02 6973"            /* when no textÂ.is */
	$"2068 696C 6974 6564 2C20 4461 7461 5363"            /*  hilited, DataSc */
	$"6F70 6520 7769 6C6C 2074 7279 2074 6F20"            /* ope will try to  */
	$"6578 6563 7574 6520 7468 6520 6C69 6E65"            /* execute the line */
	$"0D02 7768 6572 6520 7468 6520 6375 7273"            /* Â.where the curs */
	$"6F72 2069 7320 6375 7272 656E 746C 7920"            /* or is currently  */
	$"6C6F 6361 7465 642E 0D02 0D02"                      /* located.Â.Â. */
};

data 'ckid' (128, "Projector") {
	$"E1D8 0256 3FE5 3230 0004 0000 0000 0000"            /* áØ.V?å20........ */
	$"0000 A75D D281 A75D D281 A28B D8A7 0000"            /* ..§]Ò§]Ò¢‹Ø§.. */
	$"8F01 0001 0001 0007 1844 6174 6153 636F"            /* ........DataSco */
	$"7065 326C BA44 5352 6573 6F75 7263 6573"            /* pe2lºDSResources */
	$"BA00 0774 7265 646D 616E 0001 3500 0E44"            /* º..tredman..5..D */
	$"6174 6153 636F 7065 2E72 7372 6300 0000"            /* ataScope.rsrc... */
	$"0000 00"                                            /* ... */
};

