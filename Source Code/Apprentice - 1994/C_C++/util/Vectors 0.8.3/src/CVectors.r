/* CVectors.r -- Resource file for CVectors */

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

/*------------------- Menus -------------------*/

resource 'MENU' (128, "AppleMenu") {
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About Vectors", noIcon, noKey, noMark, plain,
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
		"New�", noIcon, "N", noMark, plain,
		/* [2] */
		"Open�", noIcon, "O", noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Close", noIcon, "W", noMark, plain,
		/* [5] */
		"Save", noIcon, "S", noMark, plain,
		/* [6] */
		"Save As�", noIcon, noKey, noMark, plain,
		/* [7] */
		"Save a Copy�", noIcon, noKey, noMark, plain,
		/* [8] */
		"Revert to Saved", noIcon, noKey, noMark, plain,
		/* [9] */
		"-", noIcon, noKey, noMark, plain,
		/* [10] */
		"Page Setup�", noIcon, noKey, noMark, plain,
		/* [11] */
		"Print�", noIcon, noKey, noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (130, "EditMenu") {
	130,
	textMenuProc,
	0x7FFFFD00,
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
		"Format�", noIcon, "Y", noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Align", noIcon, noKey, noMark, plain,
		/* [14] */
		"Shift Right", noIcon, "]", noMark, plain,
		/* [15] */
		"Shift Left", noIcon, "[", noMark, plain
		/* [16] */
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
		"Select Survey�", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, plain,
		/* [3] */
		"Find�", noIcon, "F", noMark, plain,
		/* [4] */
		"Find Same", noIcon, "G", noMark, plain,
		/* [5] */
		"Find Selection", noIcon, "H", noMark, plain,
		/* [6] */
		"Display Selection", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Replace�", noIcon, "R", noMark, plain,
		/* [9] */
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
		"Mark", noIcon, "M", noMark, plain,
		/* [2] */
		"Unmark", noIcon, noKey, noMark, plain,
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

resource 'MENU' (134, "ViewMenu") {
	134,
	textMenuProc,
	0x7FFFFFFB,
	enabled,
	"View",
	{	/* array: 6 elements */
		/* [1] */
		"Rotate", noIcon, hierarchicalMenu, "�", plain,
		/* [2] */
		"Scale", noIcon, hierarchicalMenu, "�", plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Enlarge", noIcon, "I", noMark, plain,
		/* [5] */
		"Reduce", noIcon, "J", noMark, plain,
		/* [6] */
		"Fit To Window", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (200, "Rotation") {
	200,
	textMenuProc,
	0x7FFFFFFB,
	enabled,
	"Rotate",
	{	/* array: 4 elements */
		/* [1] */
		"Plan", noIcon, noKey, noMark, plain,
		/* [2] */
		"Profile", noIcon, noKey, noMark, plain,
		/* [3] */
		"-", noIcon, noKey, noMark, plain,
		/* [4] */
		"Other�", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (201, "Scale") {
	201,
	textMenuProc,
	0x7FFFFFDF,
	enabled,
	"Scale",
	{	/* array: 7 elements */
		/* [1] */
		"1\"=36'", noIcon, noKey, noMark, plain,
		/* [2] */
		"1\"=72'", noIcon, noKey, noMark, plain,
		/* [3] */
		"1\"=144'", noIcon, noKey, noMark, plain,
		/* [4] */
		"1\"=288'", noIcon, noKey, noMark, plain,
		/* [5] */
		"1\"=576'", noIcon, noKey, noMark, plain,
		/* [6] */
		"-", noIcon, noKey, noMark, plain,
		/* [7] */
		"Other�", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (135, "ReportsMenu") {
	135,
	textMenuProc,
	0x7FFFFFF0,
	disabled,
	"Reports",
	{	/* array: 4 elements */
		/* [1] */
		"Schematic", noIcon, "K", noMark, plain,
		/* [2] */
		"Log", noIcon, "L", noMark, plain,
		/* [3] */
		"Rose Diagram�", noIcon, noKey, noMark, plain,
		/* [4] */
		"Position", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (136, "Data") {
	136,
	textMenuProc,
	0x7FFFF837,
	enabled,
	"Data",
	{	/* array: 14 elements */
		/* [1] */
		"Process", noIcon, "P", noMark, plain,
		/* [2] */
		"Process Files�", noIcon, noKey, noMark, plain,
		/* [3] */
		"Options�", noIcon, "U", noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Title Page", noIcon, "B", noMark, plain,
		/* [6] */
		"Data Page", noIcon, "D", noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Next Page", noIcon, "E", noMark, plain,
		/* [9] */
		"Previous Page", noIcon, noKey, noMark, plain,
		/* [10] */
		"First Page", noIcon, noKey, noMark, plain,
		/* [11] */
		"Last Page", noIcon, noKey, noMark, plain,
		/* [12] */
		"-", noIcon, noKey, noMark, plain,
		/* [13] */
		"Select Template", noIcon, hierarchicalMenu, "�", plain,
		/* [14] */
		"Edit/View Template�", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (138) {
	138,
	textMenuProc,
	allEnabled,
	enabled,
	"Data Field",
	{	/* array: 11 elements */
		/* [1] */
		"Blank", noIcon, noKey, noMark, plain,
		/* [2] */
		"From-Station", noIcon, noKey, noMark, plain,
		/* [3] */
		"To-Station", noIcon, noKey, noMark, plain,
		/* [4] */
		"Distance", noIcon, noKey, noMark, plain,
		/* [5] */
		"Fore-Azimuth", noIcon, noKey, noMark, plain,
		/* [6] */
		"Back-Azimuth", noIcon, noKey, noMark, plain,
		/* [7] */
		"Fore-Clino", noIcon, noKey, noMark, plain,
		/* [8] */
		"Back-Clino", noIcon, noKey, noMark, plain,
		/* [9] */
		"Instrument Height", noIcon, noKey, noMark, plain,
		/* [10] */
		"Target Height", noIcon, noKey, noMark, plain,
		/* [11] */
		"From Dimensions", noIcon, noKey, noMark, plain,
		/* [12] */
		"To Dimensions", noIcon, noKey, noMark, plain,
		/* [13] */
		"Remarks", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (205, "Select Page") {
	205,
	textMenuProc,
	allEnabled,
	enabled,
	"Select Page",
	{	/* array: 0 elements */
		/* [1] */
		/* "CRF", noIcon, noKey, noMark, plain */
	}
};

resource 'MRPd' (128, "CRF", purgeable) {
	139,
	lrfc,
	nonotes,
	noremarks,
	staggered,
	10,
	{	/* array ColArray: 6 elements */
		/* [1] */
		nosplit, wide, tabdown, station,
		/* [2] */
		nosplit, wide, tabright, distance,
		/* [3] */
		split, wide, tabright, azimuth,
		/* [4] */
		split, wide, tabright, clinometer,
		/* [5] */
		nosplit, narrow, tabdown, blank,
		/* [6] */
		nosplit, narrow, tabright, dimensions
	}
};

resource 'MRPd' (129, "CRF-West", purgeable) {
	18708,
	lrfc,
	nonotes,
	noremarks,
	straight,
	10,
	{	/* array ColArray: 6 elements */
		/* [1] */
		nosplit, wide, tabright, fromstation,
		/* [2] */
		nosplit, wide, tabright, tostation,
		/* [3] */
		nosplit, wide, tabright, distance,
		/* [4] */
		nosplit, wide, tabright, foreazimuth,
		/* [5] */
		nosplit, wide, tabright, backazimuth,
		/* [6] */
		nosplit, wide, tabright, foreclinometer,
		/* [7] */
		nosplit, wide, tabright, backclinometer,
		/* [7] */
		nosplit, narrow, tabright, todimensions,
		/* [9] */
		nosplit, wide, tabright, remarks
	}
};

/*------------------- Dialogs -------------------*/

resource 'DLOG' (18708, "CRF-West") {
	{40, 20, 502, 560},
	documentProc,
	visible,
	goAway,
	0x0,
	20426,
	"CRF-West"
};

resource 'DLOG' (128, "SelectSurvey",purgeable) {
	{69, 108, 286, 425},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	128,
	"Browser"
};

resource 'DLOG' (129, "CorrectDialog",purgeable) {
	{74, 41, 210, 393},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	129,
	""
};

resource 'DLOG' (130, "Correct2Dialog",purgeable) {
	{74, 41, 210, 393},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	130,
	""
};

resource 'DLOG' (131, "Options", purgeable) {
	{34, 40, 319, 478},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	131,
	""
};

resource 'DLOG' (132, "Process Files", purgeable) {
	{78, 60, 188, 422},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	132,
	""
};

resource 'DLOG' (133, "Find & R", purgeable) {
	{66, 75, 276, 437},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	133,
	"Find & Replace"
};

resource 'DLOG' (134, "More Options", purgeable) {
	{66, 44, 272, 462},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	134,
	""
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

resource 'DLOG' (137, "Map Find", purgeable) {
	{78, 76, 240, 436},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	137,
	"Find"
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

resource 'DLOG' (139, "DataPage") {
	{40, 20, 510, 435},
	documentProc,
	visible,
	goAway,
	0x0,
	139,
	"Data Page"
};

resource 'DLOG' (140, "TitlePage") {
	{40, 20, 510, 435},
	documentProc,
	visible,
	goAway,
	0x0,
	140,
	"Title Page"
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

resource 'DLOG' (142, "Rose Parameters") {
	{40, 40, 240, 280},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	142,
	"Rose Parameters"
};

resource 'DLOG' (143, "Configure Data Page") {
	{40, 34, 308, 472},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	143,
	""
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

resource 'DLOG' (1001, "AboutDialog",purgeable) {
	{42, 94, 316, 412},
	dBoxProc,
	visible,
	noGoAway,
	0x0,
	1001,
	"About CMLView"
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
resource 'ALRT' (501, "No Dims Alert") {
	{52, 60, 212, 423},
	501,
	{	
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	}
};


/*------------------- Dialog Items -------------------*/


resource 'DITL' (20426, "CRF-West") {
	{	/* array DITLarray: 123 elements */
		/* [1] */
		{81, 20, 113, 60},
		EditText {
			enabled,
			""
		},
		/* [2] */
		{81, 65, 113, 105},
		EditText {
			enabled,
			""
		},
		/* [3] */
		{81, 110, 113, 150},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{81, 155, 113, 195},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{81, 200, 113, 240},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{81, 245, 113, 285},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{81, 290, 113, 330},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{81, 335, 113, 365},
		EditText {
			enabled,
			""
		},
		/* [9] */
		{81, 370, 113, 400},
		EditText {
			enabled,
			""
		},
		/* [10] */
		{81, 405, 113, 435},
		EditText {
			enabled,
			""
		},
		/* [11] */
		{81, 440, 113, 470},
		EditText {
			enabled,
			""
		},
		/* [12] */
		{81, 475, 113, 515},
		EditText {
			enabled,
			""
		},
		/* [13] */
		{118, 20, 150, 60},
		EditText {
			enabled,
			""
		},
		/* [14] */
		{118, 65, 150, 105},
		EditText {
			enabled,
			""
		},
		/* [15] */
		{118, 110, 150, 150},
		EditText {
			enabled,
			""
		},
		/* [16] */
		{118, 155, 150, 195},
		EditText {
			enabled,
			""
		},
		/* [17] */
		{118, 200, 150, 240},
		EditText {
			enabled,
			""
		},
		/* [18] */
		{118, 245, 150, 285},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{118, 290, 150, 330},
		EditText {
			enabled,
			""
		},
		/* [20] */
		{118, 335, 150, 365},
		EditText {
			enabled,
			""
		},
		/* [21] */
		{118, 370, 150, 400},
		EditText {
			enabled,
			""
		},
		/* [22] */
		{118, 405, 150, 435},
		EditText {
			enabled,
			""
		},
		/* [23] */
		{118, 440, 150, 470},
		EditText {
			enabled,
			""
		},
		/* [24] */
		{118, 475, 150, 515},
		EditText {
			enabled,
			""
		},
		/* [25] */
		{155, 20, 187, 60},
		EditText {
			enabled,
			""
		},
		/* [26] */
		{155, 65, 187, 105},
		EditText {
			enabled,
			""
		},
		/* [27] */
		{155, 110, 187, 150},
		EditText {
			enabled,
			""
		},
		/* [28] */
		{155, 155, 187, 195},
		EditText {
			enabled,
			""
		},
		/* [29] */
		{155, 200, 187, 240},
		EditText {
			enabled,
			""
		},
		/* [30] */
		{155, 245, 187, 285},
		EditText {
			enabled,
			""
		},
		/* [31] */
		{155, 290, 187, 330},
		EditText {
			enabled,
			""
		},
		/* [32] */
		{155, 335, 187, 365},
		EditText {
			enabled,
			""
		},
		/* [33] */
		{155, 370, 187, 400},
		EditText {
			enabled,
			""
		},
		/* [34] */
		{155, 405, 187, 435},
		EditText {
			enabled,
			""
		},
		/* [35] */
		{155, 440, 187, 470},
		EditText {
			enabled,
			""
		},
		/* [36] */
		{155, 475, 187, 515},
		EditText {
			enabled,
			""
		},
		/* [37] */
		{192, 20, 224, 60},
		EditText {
			enabled,
			""
		},
		/* [38] */
		{192, 65, 224, 105},
		EditText {
			enabled,
			""
		},
		/* [39] */
		{192, 110, 224, 150},
		EditText {
			enabled,
			""
		},
		/* [40] */
		{192, 155, 224, 195},
		EditText {
			enabled,
			""
		},
		/* [41] */
		{192, 200, 224, 240},
		EditText {
			enabled,
			""
		},
		/* [42] */
		{192, 245, 224, 285},
		EditText {
			enabled,
			""
		},
		/* [43] */
		{192, 290, 224, 330},
		EditText {
			enabled,
			""
		},
		/* [44] */
		{192, 335, 224, 365},
		EditText {
			enabled,
			""
		},
		/* [45] */
		{192, 370, 224, 400},
		EditText {
			enabled,
			""
		},
		/* [46] */
		{192, 405, 224, 435},
		EditText {
			enabled,
			""
		},
		/* [47] */
		{192, 440, 224, 470},
		EditText {
			enabled,
			""
		},
		/* [48] */
		{192, 475, 224, 515},
		EditText {
			enabled,
			""
		},
		/* [49] */
		{229, 20, 261, 60},
		EditText {
			enabled,
			""
		},
		/* [50] */
		{229, 65, 261, 105},
		EditText {
			enabled,
			""
		},
		/* [51] */
		{229, 110, 261, 150},
		EditText {
			enabled,
			""
		},
		/* [52] */
		{229, 155, 261, 195},
		EditText {
			enabled,
			""
		},
		/* [53] */
		{229, 200, 261, 240},
		EditText {
			enabled,
			""
		},
		/* [54] */
		{229, 245, 261, 285},
		EditText {
			enabled,
			""
		},
		/* [55] */
		{229, 290, 261, 330},
		EditText {
			enabled,
			""
		},
		/* [56] */
		{229, 335, 261, 365},
		EditText {
			enabled,
			""
		},
		/* [57] */
		{229, 370, 261, 400},
		EditText {
			enabled,
			""
		},
		/* [58] */
		{229, 405, 261, 435},
		EditText {
			enabled,
			""
		},
		/* [59] */
		{229, 440, 261, 470},
		EditText {
			enabled,
			""
		},
		/* [60] */
		{229, 475, 261, 515},
		EditText {
			enabled,
			""
		},
		/* [61] */
		{266, 20, 298, 60},
		EditText {
			enabled,
			""
		},
		/* [62] */
		{266, 65, 298, 105},
		EditText {
			enabled,
			""
		},
		/* [63] */
		{266, 110, 298, 150},
		EditText {
			enabled,
			""
		},
		/* [64] */
		{266, 155, 298, 195},
		EditText {
			enabled,
			""
		},
		/* [65] */
		{266, 200, 298, 240},
		EditText {
			enabled,
			""
		},
		/* [66] */
		{266, 245, 298, 285},
		EditText {
			enabled,
			""
		},
		/* [67] */
		{266, 290, 298, 330},
		EditText {
			enabled,
			""
		},
		/* [68] */
		{266, 335, 298, 365},
		EditText {
			enabled,
			""
		},
		/* [69] */
		{266, 370, 298, 400},
		EditText {
			enabled,
			""
		},
		/* [70] */
		{266, 405, 298, 435},
		EditText {
			enabled,
			""
		},
		/* [71] */
		{266, 440, 298, 470},
		EditText {
			enabled,
			""
		},
		/* [72] */
		{266, 475, 298, 515},
		EditText {
			enabled,
			""
		},
		/* [73] */
		{303, 20, 335, 60},
		EditText {
			enabled,
			""
		},
		/* [74] */
		{303, 65, 335, 105},
		EditText {
			enabled,
			""
		},
		/* [75] */
		{303, 110, 335, 150},
		EditText {
			enabled,
			""
		},
		/* [76] */
		{303, 155, 335, 195},
		EditText {
			enabled,
			""
		},
		/* [77] */
		{303, 200, 335, 240},
		EditText {
			enabled,
			""
		},
		/* [78] */
		{303, 245, 335, 285},
		EditText {
			enabled,
			""
		},
		/* [79] */
		{303, 290, 335, 330},
		EditText {
			enabled,
			""
		},
		/* [80] */
		{303, 335, 335, 365},
		EditText {
			enabled,
			""
		},
		/* [81] */
		{303, 370, 335, 400},
		EditText {
			enabled,
			""
		},
		/* [82] */
		{303, 405, 335, 435},
		EditText {
			enabled,
			""
		},
		/* [83] */
		{303, 440, 335, 470},
		EditText {
			enabled,
			""
		},
		/* [84] */
		{303, 475, 335, 515},
		EditText {
			enabled,
			""
		},
		/* [85] */
		{340, 20, 372, 60},
		EditText {
			enabled,
			""
		},
		/* [86] */
		{340, 65, 372, 105},
		EditText {
			enabled,
			""
		},
		/* [87] */
		{340, 110, 372, 150},
		EditText {
			enabled,
			""
		},
		/* [88] */
		{340, 155, 372, 195},
		EditText {
			enabled,
			""
		},
		/* [89] */
		{340, 200, 372, 240},
		EditText {
			enabled,
			""
		},
		/* [90] */
		{340, 245, 372, 285},
		EditText {
			enabled,
			""
		},
		/* [91] */
		{340, 290, 372, 330},
		EditText {
			enabled,
			""
		},
		/* [92] */
		{340, 335, 372, 365},
		EditText {
			enabled,
			""
		},
		/* [93] */
		{340, 370, 372, 400},
		EditText {
			enabled,
			""
		},
		/* [94] */
		{340, 405, 372, 435},
		EditText {
			enabled,
			""
		},
		/* [95] */
		{340, 440, 372, 470},
		EditText {
			enabled,
			""
		},
		/* [96] */
		{340, 475, 372, 515},
		EditText {
			enabled,
			""
		},
		/* [97] */
		{377, 20, 409, 60},
		EditText {
			enabled,
			""
		},
		/* [98] */
		{377, 65, 409, 105},
		EditText {
			enabled,
			""
		},
		/* [99] */
		{377, 110, 409, 150},
		EditText {
			enabled,
			""
		},
		/* [100] */
		{377, 155, 409, 195},
		EditText {
			enabled,
			""
		},
		/* [101] */
		{377, 200, 409, 240},
		EditText {
			enabled,
			""
		},
		/* [102] */
		{377, 245, 409, 285},
		EditText {
			enabled,
			""
		},
		/* [103] */
		{377, 290, 409, 330},
		EditText {
			enabled,
			""
		},
		/* [104] */
		{377, 335, 409, 365},
		EditText {
			enabled,
			""
		},
		/* [105] */
		{377, 370, 409, 400},
		EditText {
			enabled,
			""
		},
		/* [106] */
		{377, 405, 409, 435},
		EditText {
			enabled,
			""
		},
		/* [107] */
		{377, 440, 409, 470},
		EditText {
			enabled,
			""
		},
		/* [108] */
		{377, 475, 409, 515},
		EditText {
			enabled,
			""
		},
		/* [109] */
		{414, 20, 446, 60},
		EditText {
			enabled,
			""
		},
		/* [110] */
		{414, 65, 446, 105},
		EditText {
			enabled,
			""
		},
		/* [111] */
		{414, 110, 446, 150},
		EditText {
			enabled,
			""
		},
		/* [112] */
		{414, 155, 446, 195},
		EditText {
			enabled,
			""
		},
		/* [113] */
		{414, 200, 446, 240},
		EditText {
			enabled,
			""
		},
		/* [114] */
		{414, 245, 446, 285},
		EditText {
			enabled,
			""
		},
		/* [115] */
		{414, 290, 446, 330},
		EditText {
			enabled,
			""
		},
		/* [116] */
		{414, 335, 446, 365},
		EditText {
			enabled,
			""
		},
		/* [117] */
		{414, 370, 446, 400},
		EditText {
			enabled,
			""
		},
		/* [118] */
		{414, 405, 446, 435},
		EditText {
			enabled,
			""
		},
		/* [119] */
		{414, 440, 446, 470},
		EditText {
			enabled,
			""
		},
		/* [120] */
		{414, 475, 446, 515},
		EditText {
			enabled,
			""
		},
		/* [121] */
		{13, 442, 29, 480},
		StaticText {
			enabled,
			"Page"
		},
		/* [122] */
		{13, 484, 29, 517},
		StaticText {
			enabled,
			"1"
		},
		/* [123] */
		{40, 17, 80, 520},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (128,purgeable) {
	{	/* array DITLarray: 10 elements */
		/* [1] */
		{120, 194, 140, 252},
		Button {
			enabled,
			"Select"
		},
		/* [2] */
		{157, 194, 177, 252},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{32, 10, 184, 150},
		UserItem {
			disabled
		},
		/* [4] */
		{191, 12, 209, 99},
		CheckBox {
			enabled,
			"Center"
		},
		/* [5] */
		{191, 111, 209, 217},
		CheckBox {
			enabled,
			"Scale To Fit"
		},
		/* [6] */
		{120, 194, 140, 252},
		UserItem {
			disabled
		},
		/* [7] */
		{38, 186, 56, 292},
		RadioButton {
			enabled,
			"By Book"
		},
		/* [8] */
		{64, 186, 84, 308},
		RadioButton {
			enabled,
			"By Designation"
		},
		/* [9] */
		{7, 16, 23, 130},
		StaticText {
			disabled,
			"Select Survey:"
		},
		/* [10] */
		{97, 186, 98, 308},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (129,purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{107, 255, 127, 323},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{18, 37, 65, 310},
		StaticText {
			enabled,
			"^0"
		},
		/* [3] */
		{73, 38, 89, 311},
		EditText {
			enabled,
			"Edit Text"
		}
	}
};

resource 'DITL' (130,purgeable) {
	{	/* array DITLarray: 6 elements */
		/* [1] */
		{107, 255, 127, 323},
		Button {
			enabled,
			"Continue"
		},
		/* [2] */
		{18, 37, 58, 312},
		StaticText {
			enabled,
			"^0"
		},
		/* [3] */
		{82, 41, 99, 156},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [4] */
		{82, 196, 98, 311},
		EditText {
			enabled,
			"Edit Text"
		},
		/* [5] */
		{60, 37, 76, 159},
		StaticText {
			disabled,
			"^1"
		},
		/* [6] */
		{60, 193, 77, 313},
		StaticText {
			disabled,
			"^2"
		}
	}
};

resource 'DITL' (131, purgeable) {
	{	/* array DITLarray: 30 elements */
		/* [1] */
		{254, 343, 274, 401},
		Button {
			enabled,
			"Done"
		},
		/* [2] */
		{253, 262, 273, 320},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{255, 35, 275, 150},
		Button {
			enabled,
			"More Options�"
		},
		/* [4] */
		{26, 26, 44, 78},
		RadioButton {
			enabled,
			"All or"
		},
		/* [5] */
		{26, 82, 44, 207},
		RadioButton {
			enabled,
			"Selected Books:"
		},
		/* [6] */
		{27, 216, 43, 411},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{64, 24, 82, 145},
		RadioButton {
			enabled,
			"No coordinates"
		},
		/* [8] */
		{84, 24, 102, 191},
		RadioButton {
			enabled,
			"Calculate coordinates"
		},
		/* [9] */
		{103, 24, 121, 212},
		RadioButton {
			enabled,
			"Calculate and close loops"
		},
		/* [10] */
		{124, 24, 142, 175},
		RadioButton {
			enabled,
			"Exchange format"
		},
		/* [11] */
		{64, 220, 82, 355},
		CheckBox {
			enabled,
			"Report warnings"
		},
		/* [12] */
		{84, 220, 102, 315},
		CheckBox {
			enabled,
			"Abort after"
		},
		/* [13] */
		{85, 319, 101, 352},
		EditText {
			enabled,
			"10"
		},
		/* [14] */
		{105, 220, 123, 394},
		CheckBox {
			enabled,
			"Ignore names"
		},
		/* [15] */
		{124, 176, 142, 220},
		RadioButton {
			enabled,
			"SEF"
		},
		/* [16] */
		{124, 222, 142, 322},
		RadioButton {
			enabled,
			"SMAPS Ascii"
		},
		/* [17] */
		{220, 60, 238, 166},
		CheckBox {
			enabled,
			"Pretty print"
		},
		/* [18] */
		{145, 89, 162, 250},
		CheckBox {
			enabled,
			"Write reports to disk"
		},
		/* [19] */
		{166, 60, 184, 166},
		CheckBox {
			enabled,
			"Schematics"
		},
		/* [20] */
		{184, 60, 202, 166},
		CheckBox {
			enabled,
			"Coordinates"
		},
		/* [21] */
		{202, 60, 220, 200},
		CheckBox {
			enabled,
			"Closure statistics"
		},
		/* [22] */
		{166, 253, 184, 359},
		CheckBox {
			enabled,
			"Attributes"
		},
		/* [23] */
		{184, 253, 202, 359},
		CheckBox {
			enabled,
			"Postscript�"
		},
		/* [24] */
		{202, 253, 220, 359},
		CheckBox {
			enabled,
			"Super 3D�"
		},
		/* [25] */
		{220, 253, 238, 359},
		CheckBox {
			enabled,
			"DXF"
		},
		/* [26] */
		{6, 25, 22, 162},
		StaticText {
			disabled,
			"Processing Options"
		},
		/* [27] */
		{84, 361, 100, 412},
		StaticText {
			disabled,
			"errors"
		},
		/* [28] */
		{54, 21, 55, 420},
		UserItem {
			disabled
		},
		/* [29] */
		{154, 43, 244, 375},
		UserItem {
			disabled
		},
		/* [30] */
		{254, 343, 274, 401},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (132, purgeable) {
	{	/* array DITLarray: 5 elements */
		/* [1] */
		{80, 279, 100, 337},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{80, 201, 100, 259},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{27, 35, 69, 335},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{4, 34, 20, 189},
		StaticText {
			enabled,
			"Process files:"
		},
		/* [5] */
		{80, 279, 100, 337},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (134, purgeable) {
	{	/* array DITLarray: 18 elements */
		/* [1] */
		{171, 334, 191, 392},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{171, 256, 191, 314},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{59, 121, 77, 205},
		RadioButton {
			enabled,
			"Use tabs"
		},
		/* [4] */
		{59, 27, 77, 123},
		RadioButton {
			enabled,
			"Use spaces"
		},
		/* [5] */
		{40, 28, 56, 206},
		CheckBox {
			enabled,
			"Convert dimensions to:"
		},
		/* [6] */
		{39, 211, 57, 280},
		RadioButton {
			enabled,
			"Meters"
		},
		/* [7] */
		{40, 288, 58, 343},
		RadioButton {
			enabled,
			"Feet"
		},
		/* [8] */
		{136, 246, 153, 387},
		CheckBox {
			enabled,
			"Reports in meters"
		},
		/* [9] */
		{103, 137, 119, 169},
		EditText {
			enabled,
			"0.0"
		},
		/* [10] */
		{137, 122, 155, 159},
		EditText {
			enabled,
			"0.0"
		},
		/* [11] */
		{164, 123, 180, 160},
		EditText {
			enabled,
			"0.0"
		},
		/* [12] */
		{58, 223, 76, 401},
		CheckBox {
			enabled,
			"Print 5th stations only"
		},
		/* [13] */
		{19, 26, 35, 167},
		StaticText {
			disabled,
			"Pretty Print options"
		},
		/* [14] */
		{103, 34, 119, 133},
		StaticText {
			disabled,
			"Convergeance"
		},
		/* [15] */
		{103, 177, 119, 252},
		StaticText {
			disabled,
			"degrees"
		},
		/* [16] */
		{136, 32, 152, 107},
		StaticText {
			disabled,
			"Compass"
		},
		/* [17] */
		{161, 31, 177, 106},
		StaticText {
			disabled,
			"Clinometer"
		},
		/* [18] */
		{171, 334, 191, 392},
		UserItem {
			disabled
		}
	}
};

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
			"Wrap�around Search"
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

resource 'DITL' (137, purgeable) {
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
			"Survey"
		},
		/* [6] */
		{80, 8, 96, 112},
		RadioButton {
			enabled,
			"Station"
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
			disabled,
			"Select All"
		},
		/* [9] */
		{80, 192, 96, 347},
		CheckBox {
			enabled,
			"Center Selection"
		},
		/* [10] */
		{96, 192, 112, 352},
		CheckBox {
			enabled,
			"Scale To Fit"
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

resource 'DITL' (139) {
	{	/* array DITLarray:  elements */

		/* 10 stations [1-10] */
		{62, 20, 94, 60}, EditText { enabled, "" },
		{99, 20, 131, 60}, EditText { enabled, "" },
		{136, 20, 168, 60}, EditText { enabled, "" },
		{173, 20, 205, 60}, EditText { enabled, "" },
		{210, 20, 242, 60}, EditText { enabled, "" },
		{247, 20, 279, 60}, EditText { enabled, "" },
		{284, 20, 316, 60}, EditText { enabled, "" },
		{321, 20, 353, 60}, EditText { enabled, "" },
		{358, 20, 390, 60}, EditText { enabled, "" },
		{395, 20, 427, 60}, EditText { enabled, "" },
		
		/* 9 distances [11-55] */
		/* 18 azis (fs,bs) */
		/* 18 vang (fs,bs) */
		{81, 65, 113, 105}, EditText { enabled, "" },
		{81, 110, 95, 150}, EditText { enabled, "" }, {100, 110, 113, 150}, EditText { enabled, "" },
		{81, 155, 95, 195}, EditText { enabled, "" }, {100, 155, 113, 195}, EditText { enabled, "" },

		{118, 65, 150, 105}, EditText { enabled, "" },
		{118, 110, 132, 150}, EditText { enabled, "" }, {137, 110, 150, 150}, EditText { enabled, "" },
		{118, 155, 132, 195}, EditText { enabled, "" }, {137, 155, 150, 195}, EditText { enabled, "" },

		{155, 65, 187, 105}, EditText { enabled, "" },
		{155, 110, 169, 150}, EditText { enabled, "" }, {174, 110, 187, 150}, EditText { enabled, "" },
		{155, 155, 169, 195}, EditText { enabled, "" }, {174, 155, 187, 195}, EditText { enabled, "" },

		{192, 65, 224, 105}, EditText { enabled, "" },
		{192, 110, 206, 150}, EditText { enabled, "" }, {211, 110, 224, 150}, EditText { enabled, "" },
		{192, 155, 206, 195}, EditText { enabled, "" }, {211, 155, 224, 195}, EditText { enabled, "" },

		{229, 65, 261, 105}, EditText { enabled, "" },
		{229, 110, 243, 150}, EditText { enabled, "" }, {248, 110, 261, 150}, EditText { enabled, "" },
		{229, 155, 243, 195}, EditText { enabled, "" }, {248, 155, 261, 195}, EditText { enabled, "" },

		{266, 65, 298, 105}, EditText { enabled, "" },
		{266, 110, 280, 150}, EditText { enabled, "" }, {285, 110, 298, 150}, EditText { enabled, "" },
		{266, 155, 280, 195}, EditText { enabled, "" }, {285, 155, 298, 195}, EditText { enabled, "" },

		{303, 65, 335, 105}, EditText { enabled, "" },
		{303, 110, 317, 150}, EditText { enabled, "" }, {322, 110, 335, 150}, EditText { enabled, "" },
		{303, 155, 317, 195}, EditText { enabled, "" }, {322, 155, 335, 195}, EditText { enabled, "" },

		{340, 65, 372, 105}, EditText { enabled, "" },
		{340, 110, 354, 150}, EditText { enabled, "" }, {359, 110, 372, 150}, EditText { enabled, "" },
		{340, 155, 354, 195}, EditText { enabled, "" }, {359, 155, 372, 195}, EditText { enabled, "" },

		{377, 65, 409, 105}, EditText { enabled, "" },
		{377, 110, 391, 150}, EditText { enabled, "" }, {396, 110, 409, 150}, EditText { enabled, "" },
		{377, 155, 391, 195}, EditText { enabled, "" }, {396, 155, 409, 195}, EditText { enabled, "" },

		/* 10 dimensions [56-95] */
		{62, 235, 94, 265}, EditText { enabled, "" },{62, 270, 94, 300}, EditText { enabled, "" },{62, 305, 94, 335}, EditText { enabled, "" },{62, 340, 94, 370}, EditText { enabled, "" },
		{99, 235, 131, 265}, EditText { enabled, "" },{99, 270, 131, 300}, EditText { enabled, "" },{99, 305, 131, 335}, EditText { enabled, "" },{99, 340, 131, 370}, EditText { enabled, "" },
		{136, 235, 168, 265}, EditText { enabled, "" },{136, 270, 168, 300}, EditText { enabled, "" },{136, 305, 168, 335}, EditText { enabled, "" },{136, 340, 168, 370}, EditText { enabled, "" },
		{173, 235, 205, 265}, EditText { enabled, "" },{173, 270, 205, 300}, EditText { enabled, "" },{173, 305, 205, 335}, EditText { enabled, "" },{173, 340, 205, 370}, EditText { enabled, "" },
		{210, 235, 242, 265}, EditText { enabled, "" },{210, 270, 242, 300}, EditText { enabled, "" },{210, 305, 242, 335}, EditText { enabled, "" },{210, 340, 242, 370}, EditText { enabled, "" },
		{247, 235, 279, 265}, EditText { enabled, "" },{247, 270, 279, 300}, EditText { enabled, "" },{247, 305, 279, 335}, EditText { enabled, "" },{247, 340, 279, 370}, EditText { enabled, "" },
		{284, 235, 316, 265}, EditText { enabled, "" },{284, 270, 316, 300}, EditText { enabled, "" },{284, 305, 316, 335}, EditText { enabled, "" },{284, 340, 316, 370}, EditText { enabled, "" },
		{321, 235, 353, 265}, EditText { enabled, "" },{321, 270, 353, 300}, EditText { enabled, "" },{321, 305, 353, 335}, EditText { enabled, "" },{321, 340, 353, 370}, EditText { enabled, "" },
		{358, 235, 390, 265}, EditText { enabled, "" },{358, 270, 390, 300}, EditText { enabled, "" },{358, 305, 390, 335}, EditText { enabled, "" },{358, 340, 390, 370}, EditText { enabled, "" },
		{395, 235, 427, 265}, EditText { enabled, "" },{395, 270, 427, 300}, EditText { enabled, "" },{395, 305, 427, 335}, EditText { enabled, "" },{395, 340, 427, 370}, EditText { enabled, "" },

		/* 9 other [96-105] */
		{81, 200, 113, 230}, EditText { enabled, "" },
		{118, 200, 150, 230}, EditText { enabled, "" },
		{155, 200, 187, 230}, EditText { enabled, "" },
		{192, 200, 224, 230}, EditText { enabled, "" },
		{229, 200, 261, 230}, EditText { enabled, "" },
		{266, 200, 298, 230}, EditText { enabled, "" },
		{303, 200, 335, 230}, EditText { enabled, "" },
		{340, 200, 372, 230}, EditText { enabled, "" },
		{377, 200, 409, 230}, EditText { enabled, "" },
		
		/* [105] */
		{13, 296, 29, 334},
		StaticText {
			disabled,
			"Page"
		},
		/* [106] */
		{13, 337, 29, 370},
		StaticText {
			disabled,
			"1"
		},
		/* [107] */
		{40, 17, 80, 373},
		UserItem {
			disabled
		}
	}
};

resource 'DITL' (140, "Title Page") {
	{	/* array DITLarray: 22 elements */
		/* [1] */
		{36, 20, 50, 180},
		EditText {
			enabled,
			""
		},
		/* [2] */
		{69, 20, 83, 180},
		EditText {
			enabled,
			""
		},
		/* [3] */
		{108, 20, 203, 180},
		EditText {
			enabled,
			""
		},
		/* [4] */
		{245, 20, 261, 187},
		EditText {
			enabled,
			""
		},
		/* [5] */
		{266, 20, 283, 187},
		EditText {
			enabled,
			""
		},
		/* [6] */
		{288, 20, 306, 187},
		EditText {
			enabled,
			""
		},
		/* [7] */
		{311, 20, 330, 187},
		EditText {
			enabled,
			""
		},
		/* [8] */
		{335, 20, 352, 187},
		EditText {
			enabled,
			""
		},
		/* [9] */
		{357, 20, 373, 187},
		EditText {
			enabled,
			""
		},
		/* [10] */
		{378, 20, 410, 97},
		EditText {
			enabled,
			""
		},
		/* [11] */
		{378, 102, 410, 187},
		EditText {
			enabled,
			""
		},
		/* [12] */
		{34, 275, 69, 351},
		EditText {
			enabled,
			""
		},
		/* [13] */
		{88, 215, 103, 351},
		EditText {
			enabled,
			""
		},
		/* [14] */
		{127, 215, 214, 351},
		EditText {
			enabled,
			""
		},
		/* [15] */
		{249, 242, 263, 351},
		EditText {
			enabled,
			""
		},
		/* [16] */
		{268, 242, 315, 351},
		EditText {
			enabled,
			""
		},
		/* [17] */
		{346, 242, 362, 351},
		EditText {
			enabled,
			""
		},
		/* [18] */
		{367, 242, 412, 351},
		EditText {
			enabled,
			""
		},
		/* [19] */
		{23, 17, 64, 94},
		UserItem {
			disabled
		},
		/* [20] */
		{226, 17, 242, 116},
		UserItem {
			disabled
		},
		/* [21] */
		{32, 212, 85, 274},
		UserItem {
			disabled
		},
		/* [22] */
		{231, 212, 344, 345},
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

resource 'DITL' (142) {
	{	/* array DITLarray: 12 elements */
		/* [1] */
		{166, 164, 186, 222},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{166, 93, 186, 151},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{8, 28, 23, 206},
		StaticText {
			disabled,
			"Rose Diagram Parameters"
		},
		/* [4] */
		{40, 19, 57, 216},
		CheckBox {
			enabled,
			"Just the selected survey?"
		},
		/* [5] */
		{111, 19, 129, 125},
		RadioButton {
			enabled,
			"Polar"
		},
		/* [6] */
		{128, 19, 146, 125},
		RadioButton {
			enabled,
			"Histogram"
		},
		/* [7] */
		{145, 19, 163, 80},
		RadioButton {
			enabled,
			"Text"
		},
		/* [8] */
		{28, 20, 29, 226},
		UserItem {
			enabled
		},
		/* [9] */
		{68, 20, 102, 141},
		StaticText {
			disabled,
			"How many bins?\n^0"
		},
		/* [10] */
		{77, 150, 93, 201},
		EditText {
			enabled,
			"36"
		},
		/* [11] */
		{105, 20, 106, 226},
		UserItem {
			enabled
		},
		/* [12] */
		{166, 164, 186, 222},
		UserItem {
			enabled
		}
	}
};

resource 'DITL' (143) {
	{	/* array DITLarray: 26 elements */
		/* [1] */
		{231, 355, 251, 413},
		Button {
			enabled,
			"Cancel"
		},
		/* [2] */
		{231, 283, 251, 341},
		Button {
			enabled,
			"Make"
		},
		/* [3] */
		{11, 75, 27, 210},
		EditText {
			enabled,
			"New Template"
		},
		/* [4] */
		{12, 251, 28, 373},
		StaticText {
			enabled,
			"Stations per page"
		},
		/* [5] */
		{10, 386, 26, 408},
		EditText {
			enabled,
			"10"
		},
		/* [6] */
		{32, 250, 48, 381},
		StaticText {
			enabled,
			"Number of Columns"
		},
		/* [7] */
		{33, 386, 49, 408},
		EditText {
			enabled,
			"9"
		},
		/* [8] */
		{124, 30, 213, 410},
		UserItem {
			disabled
		},
		/* [9] */
		{83, 35, 101, 132},
		CheckBox {
			enabled,
			"Half-width"
		},
		/* [10] */
		{100, 35, 118, 93},
		CheckBox {
			enabled,
			"Split"
		},
		/* [11] */
		{82, 247, 98, 293},
		StaticText {
			disabled,
			"Type:"
		},
		/* [12] */
		{82, 296, 98, 394},
		UserItem {
			disabled
		},
		/* [13] */
		{84, 142, 102, 173},
		StaticText {
			disabled,
			"Tab:"
		},
		/* [14] */
		{102, 129, 120, 185},
		RadioButton {
			enabled,
			"Right"
		},
		/* [15] */
		{102, 186, 120, 244},
		RadioButton {
			enabled,
			"Down"
		},
		/* [16] */
		{102, 244, 120, 297},
		RadioButton {
			enabled,
			"Back"
		},
		/* [17] */
		{74, 30, 121, 409},
		UserItem {
			disabled
		},
		/* [18] */
		{65, 46, 81, 153},
		StaticText {
			disabled,
			"Field Properties"
		},
		/* [19] */
		{58, 23, 219, 416},
		UserItem {
			disabled
		},
		/* [20] */
		{50, 175, 66, 259},
		StaticText {
			disabled,
			"Page Layout"
		},
		/* [21] */
		{226, 32, 244, 203},
		CheckBox {
			enabled,
			"Remarks field on page"
		},
		/* [22] */
		{243, 32, 261, 182},
		CheckBox {
			enabled,
			"Remarks are notes"
		},
		/* [23] */
		{31, 31, 49, 120},
		CheckBox {
			enabled,
			"Staggered"
		},
		/* [24] */
		{11, 23, 27, 61},
		StaticText {
			disabled,
			"Title:"
		},
		/* [25] */
		{231, 211, 251, 269},
		Button {
			enabled,
			"Delete"
		},
		/* [26] */
		{231, 355, 251, 413},
		UserItem {
			enabled
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

resource 'DITL' (258,purgeable) {
	{	/* array DITLarray: 3 elements */
		/* [1] */
		{67, 230, 87, 285},
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
		}
	}
};

resource 'DITL' (500, "phSaveChanges", purgeable) {
	{
		{58, 25, 78, 99},
		Button {
			enabled,
			"Yes"
		},
		{83,195, 103, 269},
		Button {
			enabled,
			"Cancel"
		},
		{83, 25, 103, 99},
		Button {
			enabled,
			"No"
		},
		{12, 20, 53, 279},
		StaticText {
			disabled,
			"Save changes to �^0� before closing?"
		}
	}
};

resource 'DITL' (501) {
	{	/* array DITLarray: 4 elements */
		/* [1] */
		{125, 280, 145, 338},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{93, 279, 113, 337},
		Button {
			enabled,
			"Cancel"
		},
		/* [3] */
		{125, 8, 145, 268},
		Button {
			enabled,
			"OK, do not show this message again."
		},
		/* [4] */
		{10, 60, 85, 343},
		StaticText {
			disabled,
			"Wall dimensions were ignored when \"^0\" w"
			"as made. The rose diagram will not be ex"
			"act but will still have 1� accuracy."
		}
	}
};

resource 'DITL' (1001,purgeable) {
	{	/* array DITLarray: 1 elements */
		/* [1] */
		{4, 7, 268, 312},
		Picture {
			enabled,
			1001
		}
	}
};

/*------------------- Strings -------------------*/

resource 'STR#' (128,purgeable) {
	{	/* array StringArray: */
		"Mark the selection with what name?",
		"\"%s\" is an existing marker. Do you want to replace it?",
		"Save this template as:",
		"Replace existing \"%s\"?",
		"Delete which markers?",
		"Do you really want to discard the changes to \"%s\"?",
		"Do you really want to delete the \"%s\" template?"
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
/*15*/	"Sorry! Vectors requires System 6.04 or higher.",
		"This file cannot be opened. Try opening it with the Open menu.",
		"Resources in this file are corrupted but the text is undamaged.",
		"This file is locked.",
		"The scrap is too large to paste here.",
/*20*/	"Couldn't open offscreen bitmap",
		"There cannot be more that 21 stations in a data page",
		"There cannot be more that 20 columns in a data page"
	}
};

resource 'STR#' (131,purgeable) {
	{	/* array StringArray: */
		"Vectors Worksheet",
		"Open document",
		"Untitled",
		"Save document as",
		"Save a Copy as",
		"Vectors Preferences"
	}
};

resource 'STR#' (139,"Data Page",purgeable) {
	{	/* array StringArray: 15 elements */
		"From",
		"To",
		"Dist.",
		"Fore",				/* 4 */
		"Back",				/* 5 */
		"Fore",				/* 6 */
		"Back",				/* 7 */
		"IH",				/* 8 */
		"TH",				/* 9 */
		"From-Dims",		/* 10 */
		"To-Dims",
		"Remarks",			/* 12 */
		"","","","",
		"Station",			/* 17 */
		"","",
		"Azi.",				/* 20 */
		"",
		"Clin.",			/* 22 */
		"",
		"Hghts",			/* 24 */
		"",
		"Dimensions",		/* 26 */
		"left",				/* 27 */
		"right",
		"ceil.",
		"floor"
	}
};

resource 'STR#' (140,"Title Page",purgeable) {
	{	/* array StringArray: 12 elements */
		"Cave",
		"Area",
		"Objectives",
		"Personnel",
		"Compass",			/* [5] */
		"ID",
		"Test",
		"Clinometer",
		"ID",
		"Test",				/* [10] */
		"Date",
		"FSB"
	}
};

resource 'STR#' (143,"Menu Labels",purgeable) {
	{	/* array StringArray: 15 elements */
		"From-Station",
		"To-Station",
		"Distance",
		"Fore-Azimuth",			/* 4 */
		"Back-Azimuth",			/* 5 */
		"Fore-Clino",			/* 6 */
		"Back-Clino",			/* 7 */
		"Instrument Height",	/* 8 */
		"Target Height",		/* 9 */
		"From-Dimensions",		/* 10 */
		"To-Dimensions",
		"Remarks",				/* 12 */
		"","","","",
		"Station",				/* 17 */
		"","",
		"Azimuth",				/* 20 */
		"",
		"Clinometer",			/* 22 */
		"",
		"Heights",				/* 24 */
		"",
		"Dimensions"			/* 26 */
	}
};

resource 'STR#' (144,purgeable) {
	{	/* array StringArray: 16 elements */
		"Fr",
		"To",
		"Di",
		"FA",
		"BA",
		"FC",
		"BC",
		"IH",
		"TH",
		"FD",
		"TD",
		"Re",					/* 12 */
		"","","","",
		"St",
		"","",
		"Az",
		"",
		"Cl",
		"",
		"H",
		"",
		"WD",
		"LRUD"					/* 27 */
		
	}
};

/*------------------- Version -------------------*/

resource 'vers' (1) {
	0x0,
	0x08,
	beta,
	0x1,
	verUS,
	"0.8.1",
	"0.8.1"
};

resource 'vers' (2) {
	0x0,
	0x08,
	beta,
	0x1,
	verUS,
	"0.8.1",
	"Version 0.8.1"
};

data 'MPSR' (1005,"Defaults") {
	$"0009 4D6F 6E61 636F 00E1 2168 0000 0000"            /* .�Monaco..!h.... */
	$"0000 00A0 0000 51A8 00E0 AB60 8078 0000"            /* ...�..Q�..�`�x.. */
	$"0000 0006 0004 002C 0004 0123 0180 002C"            /* .......,...#.�., */
	$"0004 0123 0180 A976 E29F 0000 0420 0000"            /* ...#.��v.�... .. */
	$"0420 0000 0000 0100"                                /* . ...... */
};

/*------------------- Finder -------------------*/

data 'MRPG' (0) {
};

resource 'FREF' (128) {
	'APPL',
	0,
	""
};

resource 'FREF' (129) {
	'CMLB',
	1,
	""
};

resource 'FREF' (130) {
	'TEXT',
	2,
	""
};

resource 'BNDL' (128) {
	'MRPG',
	0,
	{	/* array TypeArray: 2 elements */
		/* [1] */
		'ICN#',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129,
			/* [3] */
			2, 130
		},
		/* [2] */
		'FREF',
		{	/* array IDArray: 1 elements */
			/* [1] */
			0, 128,
			/* [2] */
			1, 129,
			/* [3] */
			2, 130
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
	localAndRemoteHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	1256000,
	1256000 
};


/*------------------- About Pict -------------------*/

#if 0
resource 'PICT' (1001,purgeable) {
	8028,
	{-1, -1, 269, 303},
	$"1101 A000 82A0 008C A000 8C01 000A FFFF"
	$"FFFF 010D 012F 2200 5000 D902 0123 FE02"
	$"2300 0223 0001 2200 5900 DCFD FD22 0059"
	$"00DC 01FF 2302 0023 0100 2302 0123 0100"
	$"23FF 0223 0201 23FF 0123 0202 23FF 0123"
	$"0103 2301 0423 0202 23FD 0223 0002 2300"
	$"0223 0300 2303 0023 0202 23FF 0123 0003"
	$"2300 0123 0100 2200 5500 D900 0122 006B"
	$"00E5 FFFF 2200 7600 ED01 0223 0302 2300"
	$"0123 0102 2302 0123 0202 2304 0122 0082"
	$"00FD FDFF 2200 8200 FD05 0323 0200 2301"
	$"0023 00FE 2303 0123 0501 2200 8401 08FE"
	$"0222 0088 0118 0001 23FF FE23 FBFF 23FF"
	$"0123 FF01 23FF FE23 FF00 2300 FF23 FF00"
	$"2300 0022 0051 00AD 0101 2302 0123 0202"
	$"2302 0023 00FF 2300 FF23 02FF 2302 0023"
	$"02FF 2302 0123 0300 2302 0023 0102 2300"
	$"0123 0200 23FF 0123 0102 23FF 0123 0102"
	$"2301 0123 0002 2300 0123 0201 2301 0023"
	$"0200 2301 0023 0000 2302 0023 FF02 2300"
	$"0223 FF00 2300 0123 0003 23FF 0223 FE02"
	$"2300 0423 FF03 2300 0323 0002 23FE 0223"
	$"0000 2301 0123 0003 2302 0223 0000 2300"
	$"0123 0101 2300 0223 0001 2302 0223 FD02"
	$"2300 0123 0000 23FE 0223 0001 23FE 0223"
	$"FE02 2303 0323 0303 2303 0023 0202 2300"
	$"0423 0302 2303 FE23 0103 2303 FF23 0400"
	$"2301 FF23 0302 2200 A100 E001 FD23 FEFD"
	$"23FF FE23 FEFF 2300 0023 FF01 23FE FF23"
	$"0000 23FE 0023 0001 23FD FF23 FDFE 23FE"
	$"FF23 FFFF 2300 0023 FEFE 23FF 0223 FE00"
	$"23FE 0023 FD01 23FE 0022 0096 00BF 03FF"
	$"2200 A000 DF01 0122 009F 00E1 00FF 2200"
	$"9600 BF00 0122 00A2 0126 0000 2300 FF23"
	$"00FF 23FF FF23 0000 23FF 0023 FF00 23FF"
	$"FE23 FFFF 23FF 0023 FEFF 2300 0123 FF00"
	$"23FF FE23 FF00 23FF FD23 0000 2301 FE23"
	$"00FF 2301 FF23 01FD 2302 0123 01FF 2300"
	$"FE23 FF00 23FF FE23 FFFF 23FF FE23 FD00"
	$"23FD FE22 00A3 0126 00FF 2200 8901 19FF"
	$"FF23 0000 2200 A301 2602 FF23 0102 2300"
	$"0123 0101 23FF 0023 0200 2302 0322 0090"
	$"010E 01FE 2301 0023 01FE 2300 FF23 02FE"
	$"2300 FE23 FDFF 23FE FF23 FDFF 23FE 0023"
	$"FF00 23FE 0023 FF00 23FE FF23 FF01 23FD"
	$"FD23 FE00 23FF 0023 FC00 23FF 0123 00FF"
	$"23FD FE23 FDFF 23FF FF23 00FF 23FF FE23"
	$"FFFE 23FD FE23 FA00 23FE FF23 00FF 23FF"
	$"FE23 0000 23FF 0023 FEFF 2300 FF23 FF00"
	$"23FE 0023 00FF 23FF 0023 FE00 23FF FE23"
	$"0000 2301 FF23 00FF 23FF 0023 0000 2301"
	$"0023 0000 2300 FF23 00FD 2300 0023 FF00"
	$"23FF 0023 01FF 23FF 0023 FEFF 2300 FF23"
	$"FF00 2300 0023 0000 2300 0023 FFFF 2300"
	$"0023 FFFF 23FF FF23 FF01 2200 8701 1301"
	$"FF22 0067 00D9 FF01 2200 6200 D000 FF22"
	$"0068 00D8 FFFF 2300 0023 0000 23FF 0023"
	$"FFFF 2300 0023 00FE 23FF FF23 FFFF 23FF"
	$"0023 FFFE 2300 0023 0002 2200 6100 D100"
	$"0022 0086 0114 0000 2300 0122 0087 0113"
	$"0100 2200 8701 13FF 0023 FE00 2300 0023"
	$"0001 2200 4800 7DFD FD23 FDFE 23FF FF23"
	$"FDFE 23FD FF23 FD00 23FE FD23 FEFF 23FD"
	$"FE23 FEFC 23FE FE23 01FD 2301 FC23 00FD"
	$"23FD FE23 FCFE 23FD FD23 FCFE 23FD FE23"
	$"FD02 23FD FE23 FDFC 23FD FE23 FC00 23FC"
	$"0122 0042 0076 03FD 2200 2700 3404 FD23"
	$"01FD 2302 FE23 03FB 2301 FF22 003F 0079"
	$"0303 2303 0123 03FE 2303 0123 03FC 2302"
	$"FE23 FEFF 23FE FF22 003C 008A 0002 2302"
	$"0023 0100 2303 0223 02FF 2302 0223 0301"
	$"2302 0022 003F 0092 0002 2300 0123 0000"
	$"2300 0123 0000 2300 0123 0000 23FA FE22"
	$"0044 0092 0400 2200 4500 9702 0122 0044"
	$"0096 FF02 23FF 0023 0101 2301 0023 0100"
	$"2300 0023 0000 23FE 0123 FE00 2300 FF23"
	$"FF00 23FE FF22 0044 0096 0203 2200 4700"
	$"97FF FF22 0046 0090 FF01 2200 4600 9600"
	$"0023 FEFF 2301 FE23 0000 2300 0022 0047"
	$"008F FEFF 2301 0023 01FF 2301 0023 01FF"
	$"2302 0123 0000 2301 0023 01FF 2301 0023"
	$"0201 2301 FF23 0100 2301 FF23 0100 2300"
	$"0023 FF00 2300 0023 FF00 23FF 0023 0000"
	$"2200 4500 98FF FF22 0043 0099 00FF 23FC"
	$"FE23 FE00 23FF FF23 FEFF 23FF 0223 FEFF"
	$"23FE FD23 01FF 23FD FF23 00FB 2301 FF23"
	$"01FC 23FF FE23 01FF 2301 0023 00FC 2301"
	$"FF23 00FE 2300 FF23 0200 2300 FF23 0200"
	$"2302 FF23 00FF 2300 FE23 FFFE 23FD FC23"
	$"FDFD 23FF FE23 FDFE 23FF 0023 FE02 23FC"
	$"0023 FCFE 23FC FE23 FEFE 23FE FD23 FEFD"
	$"23FE 0123 FFFF 23FF 0123 FDFF 23FF FF23"
	$"0000 2300 FF23 02FC 2300 FF23 FCFF 23FE"
	$"0023 0000 23FF 0023 FD01 2300 FF23 FE00"
	$"2300 0223 0001 2300 0223 0101 2300 0123"
	$"0101 2200 2300 9300 0122 0044 0097 0000"
	$"23FE 0023 FEFF 23FE 0023 FF00 2300 0023"
	$"FE00 23FF FF23 00FF 2300 FE23 0200 2300"
	$"0022 0044 0097 04FF 2200 3F00 8E01 0122"
	$"0024 0093 0101 2302 0023 01FF 2301 FE23"
	$"FEFC 23FD FC23 FDFD 23FE FE23 00FE 23FC"
	$"FD23 FD00 23FD 0023 FEFF 23FF 0023 FFFF"
	$"23FB FF23 FCFF 23FF FF23 FDFD 23FD FD22"
	$"000B 0076 0001 2200 0400 70FD FE22 000C"
	$"0076 0201 2301 0123 0302 2303 0223 0203"
	$"2200 4300 9B02 0123 0101 2302 0123 0202"
	$"2301 0023 0001 2302 0023 0000 2301 0023"
	$"0001 23FF 0023 0302 2301 0123 02FF 2300"
	$"0023 0200 2300 0023 01FF 2301 0123 0001"
	$"2300 0123 0100 2300 0023 0100 2200 4E00"
	$"B2FF 0022 004E 00B2 0100 2300 0023 FFFE"
	$"2200 4C00 AD00 0022 0047 0098 0201 2302"
	$"0223 0001 2304 0023 02FF 2302 0022 00AE"
	$"00BF 0001 2303 0123 0001 23FE 0223 0106"
	$"23FB 0423 0305 2300 0623 0504 2304 0423"
	$"0102 2305 0223 0202 2304 FE23 03FE 2301"
	$"FF23 0101 2303 0023 04FF 2302 0123 0302"
	$"2303 0223 06FF 2301 FD23 02FF 2303 0023"
	$"03FE 2300 FE23 FEFF 23FF 0023 00FE 23FF"
	$"FF23 01FC 23FD FB23 00FC 23FD FC23 FFFD"
	$"2301 FC23 FDFE 23FE FD23 01FE 2304 0023"
	$"FFFC 23FC FD23 FFFE 23FC FB23 FFFE 23FF"
	$"FD23 FFFD 23FD FD23 FDFB 23FF FE23 FEFF"
	$"23FF FF23 FEFF 23FD FD22 00B1 00C2 00FF"
	$"2200 B900 C1FF FC22 00D1 00E0 FC00 2200"
	$"B500 ED01 FC22 00B5 00ED 0102 2302 0123"
	$"01FF 2301 0323 0102 2302 0423 0201 2301"
	$"FF23 0101 2301 FE23 01FF 2302 0123 0000"
	$"2301 0123 04FF 2302 FF23 0100 2303 FD23"
	$"0100 2301 FF23 00FF 2301 FF23 00FD 2303"
	$"0022 00B7 00F1 FEFE 2200 BB01 09FF 0422"
	$"00B5 010E 00FD 2303 FF23 00FF 2300 FE23"
	$"0300 2301 FF22 00AF 0118 01FE 23FF FE23"
	$"FF00 2300 0123 FDFF 2300 0123 FF01 23FE"
	$"0122 00BF 0108 0101 2200 C301 0BFE FD22"
	$"00C3 010B 00FE 23FF 0122 00C2 0108 0200"
	$"2200 C201 0800 FE23 FF00 23FF 0023 FFFF"
	$"23FF 0123 0000 23FE FE22 00C0 0104 FFFF"
	$"2200 BE01 02FE FE22 00BE 0102 0001 2200"
	$"BF01 03FE FF23 0000 2300 FE23 01FF 2300"
	$"FE23 FF00 23FE 0123 FF01 2301 0123 FF00"
	$"23FF 0123 0000 23FF 0123 0000 23FE FE23"
	$"0000 23FF 0123 FFFF 2300 0023 FFFF 2300"
	$"FF23 00FF 2300 0023 0000 23FF FF23 FF00"
	$"23FF 0023 FEFE 23FF FF22 00BC 0100 FEFF"
	$"23FD FD23 01FE 23FE FE23 FCFE 23FF FF22"
	$"00B4 00FA 0201 2301 0023 00FF 2300 0023"
	$"02FE 2301 FF23 0200 2300 0223 0201 2301"
	$"0123 0301 2300 0122 00CE 0111 FFFF 2300"
	$"FE23 01FE 2301 FE23 00FD 23FF FF23 FE00"
	$"23FE 0023 FE00 2300 FF23 FFFE 2200 BF01"
	$"0800 0022 00CD 0110 0002 2200 BF01 0A00"
	$"FF22 00BF 0108 0101 2200 CF01 1002 0123"
	$"FFFF 2300 FF23 FFFE 23FE FF23 FFFD 2300"
	$"FE23 FF00 2301 FE23 FEFF 2200 C001 0B00"
	$"0322 00C0 010B 00FF 2200 BE01 0CFF 0122"
	$"00BE 010A 0200 2200 C801 0D01 0222 00BE"
	$"010A FE01 2201 0B00 5004 FE23 F3F5 2309"
	$"F623 0300 2301 FF23 00FC 2304 FD23 00FA"
	$"2301 FE23 0500 2303 0023 0500 2302 FF23"
	$"04FF 2301 FF23 0403 2303 FF23 02FE 23FF"
	$"FD23 FFFE 23FC FD23 FFFD 2301 FE23 FFFE"
	$"2300 FC23 FBFC 23FF FB23 FDFC 2304 F723"
	$"00FF 2301 FF23 FFFE 2302 FD23 FEFD 2300"
	$"FA23 04FD 23FF FB23 00FB 2303 FF23 0700"
	$"2303 0123 0300 2300 0423 0200 2302 FF23"
	$"0302 2304 0023 04FF 2303 0023 0200 23FF"
	$"0123 0302 2302 0023 0403 2305 0223 01FF"
	$"2200 B700 6AFD 0022 00A8 006A F901 2200"
	$"A000 8403 FF22 00A6 00A2 02FF 2200 A600"
	$"9F03 0123 03FF 2302 0023 02FF 2302 0123"
	$"0004 2303 0023 0301 2304 0223 0402 2303"
	$"FD23 00FF 2302 0223 0101 2301 0023 0202"
	$"2300 0123 0103 2300 0123 0000 2302 0123"
	$"0001 2300 0123 0100 2300 0023 FF00 2300"
	$"0223 FF02 23FF FF23 00FF 23FF 0022 00B9"
	$"00C1 0101 2200 A600 A1FE 0022 00AB 00BC"
	$"0000 2303 FF22 00AE 00BF FFFF 2200 B400"
	$"C300 0022 00BC 00C4 00FF 2200 B900 C100"
	$"0022 00A5 00A4 0104 2304 0223 0402 2302"
	$"0123 0402 2302 0623 0202 2302 0023 0300"
	$"2200 B800 BEFE 0022 00B8 00BE 0100 2302"
	$"0123 0103 2302 0123 0305 2303 0023 0001"
	$"2301 0023 0200 2301 0123 0000 23FF 0123"
	$"FF02 23FF 0023 0003 23FD FF23 FFFE 23FF"
	$"FF23 FDFE 23FD FF23 FEFD 2200 BD00 C400"
	$"0122 00C5 00CD 02FF 2200 CA00 CB00 0122"
	$"00C0 00BE FEFE 2200 B700 6700 0123 0000"
	$"23FB 0023 FDFD 2300 FF23 FEFE 2300 FF23"
	$"FFFF 23FE FD23 F9FE 23FC FF23 FBFB 23FD"
	$"FF23 FDF6 23FB FF23 FFFE 23FF 0023 FDFE"
	$"23FF FF23 01FE 23FD FC23 FBFC 23FD FD23"
	$"FBF8 23FA FE23 FCFE 23FF FF23 FCFE 23FB"
	$"FA23 FCFD 23FC 0123 FE01 23FD FF23 FD02"
	$"23FB FC22 00A9 0063 F804 23FF 0022 0046"
	$"0099 0201 2301 FF23 0900 2305 0023 0802"
	$"2305 0323 0201 2303 0123 00FC 2303 FF23"
	$"0202 2303 0023 0201 2304 FF23 0301 2307"
	$"0123 0300 23FF 0223 0101 2302 0022 0046"
	$"009C 0101 2200 5000 D900 FF22 0047 009D"
	$"FC07 23F9 0123 F800 23F9 FB23 FAFC 23FC"
	$"0723 FF04 2304 0823 0703 23FF 0523 FA02"
	$"23FD FF23 FE09 23FD 0423 FC02 2300 0923"
	$"FF05 2306 0323 0009 2300 0323 0703 2304"
	$"0023 0606 2301 0323 0305 2304 0423 0801"
	$"2304 0023 0301 2300 0223 0500 2200 BB00"
	$"C400 0023 0201 2300 0223 0002 2303 0123"
	$"0202 2301 0023 0000 2301 0022 006A 00E4"
	$"FF00 23FF 0023 FE01 23FE FF23 FEFD 23FE"
	$"0123 FDFF 2300 0023 FE00 23FF FF23 FE00"
	$"23FF FF23 FEFF 2300 0023 FFFF 2300 0122"
	$"00A9 00BF 0201 2301 FD23 03FF 2303 FD23"
	$"04FE 2301 FD23 FFFB 23FE FC22 0092 00C7"
	$"0303 2200 9200 C7FE FE23 FFFC 2301 FF22"
	$"008A 00C8 FD01 2200 8A00 C8FD 0223 00FD"
	$"2302 FE23 01FE 2302 0023 02FE 2200 8200"
	$"CB01 0122 0082 00CB 0000 23FE FF23 0100"
	$"2303 FD22 007E 00CE FF00 2200 7E00 CEFE"
	$"FE22 0078 00CB 0104 2200 7800 CB02 FE23"
	$"FEFC 2300 FD23 FFFF 23FF FC23 00FD 2303"
	$"FF23 02FF 23FE FC23 01FF 2301 FF23 0000"
	$"2200 8A00 C802 FF22 0089 00C5 0000 2200"
	$"7E00 CE00 0122 0065 00CE FFFF 2200 5F00"
	$"CE00 0022 009F 0087 0000 2300 0023 FEFC"
	$"23FC FF23 FD00 23FE FD23 FF00 23FF FE23"
	$"FE01 23FD FE23 FFFF 23FE FE23 FEFD 23FF"
	$"FF23 FFFE 2300 FE23 00FF 2301 FF23 01FE"
	$"2302 0023 02FE 2200 9F00 8703 0322 0083"
	$"0074 0201 2303 FE23 02FF 2301 0023 00FE"
	$"2300 FD23 01FE 2302 FF23 00FE 23FE FF23"
	$"FFFD 2300 FF23 00FE 2300 0023 00FF 23FE"
	$"0022 00A2 008A FF04 23FE 0123 0003 2300"
	$"0223 0101 2300 0123 0100 2200 B800 C500"
	$"0123 0002 2300 0123 FD00 2300 0023 0000"
	$"23FE 0023 0000 23FE 0123 FF00 2200 BB00"
	$"BCFD FD22 00BC 00C0 0101 2200 BD00 BDFF"
	$"FE22 00BA 00BC 0001 2200 B800 B900 0022"
	$"00BA 00BC 0001 2301 0123 0001 2301 0023"
	$"0101 2300 0023 0001 2300 0123 0000 2300"
	$"0023 0002 2300 0023 0001 2302 0123 0000"
	$"2301 0122 00B4 00C3 02FF 2301 0223 0200"
	$"2300 0023 0501 2303 FF23 02FC 2302 0223"
	$"0300 2302 0023 01FF 2302 0123 0402 2301"
	$"0123 0102 2300 0123 0002 23FF 0122 00AE"
	$"008B 02FF 2304 0223 0401 2305 0023 0201"
	$"2305 0123 01FF 2303 0023 03FE 2303 FF23"
	$"0401 2301 0022 00CD 00DC 00FF 23FE 0023"
	$"FDFF 23FD FE22 00C8 00D2 0201 2200 C900"
	$"D101 FF22 00C7 00D1 0002 2200 C800 CF02"
	$"FF22 00CD 00DC 0201 2200 CA00 CB04 FE22"
	$"00CE 00DF FF00 2200 CE00 DF01 0023 0100"
	$"2301 0023 0100 2301 FF23 0100 2303 0123"
	$"0203 2301 0123 0302 23FE 0123 0201 2300"
	$"0022 00CE 00DF 0100 2200 D000 C701 FE23"
	$"0001 23FF FE23 01FF 2301 0123 0100 2301"
	$"0023 0001 2302 0123 01FF 2300 0123 01FF"
	$"2302 0023 00FE 2301 FF23 0201 2300 0023"
	$"0201 2301 0023 00FF 2302 0123 02FF 2301"
	$"0122 00D0 00C7 0001 2200 CD00 DC00 0022"
	$"00D1 00C7 0101 2300 0023 0201 2301 0023"
	$"0002 2303 FF22 00D5 00CE 00FF 2200 D500"
	$"CEFF 0123 FE00 2300 0023 FE00 23FE 0023"
	$"FFFD 2304 FD23 FB01 2301 FF22 00CE 00C6"
	$"0002 2200 CE00 C6FC FF22 00CE 00C6 FF00"
	$"2200 CB00 C101 0222 00CB 00C1 FB00 23FD"
	$"FE22 00C9 00B6 0300 2200 C900 B6F9 FA23"
	$"FCFD 23FE FF23 FCFF 2300 FE23 FFFF 23FE"
	$"FE23 FD00 23FF FF23 FBFF 23FB FF23 FB00"
	$"23FD 0223 FBFD 23FB 0223 FFFE 23FB FC23"
	$"F802 23FF FF23 FAFD 23FC 0223 FAFC 23FC"
	$"0023 FCFE 2200 CB00 C104 0122 00B8 009E"
	$"00FD 2200 CD00 C503 FE23 06FE 2303 0022"
	$"00C4 00CF 0201 2300 FD23 00FE 23FD 0023"
	$"FF00 23FF 0023 FEFF 2200 BE00 BC00 FF23"
	$"0001 23FD FF23 FF02 23FE FF22 00BD 00BC"
	$"0101 2301 FF23 0301 2301 0223 0303 2301"
	$"FF23 0305 2302 0123 0101 2302 0022 00B4"
	$"009C 0000 2200 B400 9DFF 0022 00B4 009D"
	$"0100 2301 FE23 0300 2301 FF23 0400 2302"
	$"0023 0301 2302 0023 0200 2300 0223 0201"
	$"2301 0122 00B6 00B4 FF00 2200 B600 B401"
	$"0123 0401 2301 0423 0203 2301 0123 0100"
	$"2200 AB00 BCFE FF23 FF01 2300 0023 FDFF"
	$"2300 0023 0003 23FF FF23 FF01 23FE 0122"
	$"00A9 00B7 FF01 2300 0122 00B1 00B2 00FD"
	$"2200 AB00 B600 0023 0202 2300 0323 0001"
	$"23FF 0023 0000 2301 0023 0302 2301 0123"
	$"0401 2303 0023 0201 2300 0123 0001 2302"
	$"0423 0102 2302 0222 00BF 00C7 0000 2200"
	$"B800 C500 0022 00BE 00C8 FF01 2302 0222"
	$"00BD 00C1 0000 23FE 0223 0002 2301 0223"
	$"0201 2300 0123 0000 2301 0023 0102 23FF"
	$"0123 0100 2300 0223 0101 2303 FF23 00FF"
	$"2301 FF23 0000 2300 FF23 0101 23FE 0123"
	$"0000 2300 0022 00BE 00C4 FF00 23FF 0023"
	$"00FF 2300 0022 00BC 00C2 FE00 2200 AF00"
	$"B200 0123 0100 2200 CB00 CB00 0023 0101"
	$"2302 0123 0001 2302 0123 0101 2302 FF23"
	$"0001 2301 0223 0001 2300 0223 01FF 2302"
	$"0223 0000 2301 0123 0101 2300 FF23 0200"
	$"2300 0123 0101 2300 0023 0201 2200 DE00"
	$"DEFD FF23 FF00 23FE 0023 FF01 23FF 0123"
	$"FF02 2302 0023 0001 2300 0023 0102 2300"
	$"0022 00DA 00DE 0004 2200 E400 D800 0022"
	$"00CE 00C5 0300 2300 0023 0300 2301 FF23"
	$"0200 2302 0123 0100 2303 0123 0102 2304"
	$"0123 0000 2300 0223 0002 2300 0023 0002"
	$"2200 D200 D903 0122 00C5 00C2 0000 2300"
	$"FF23 0102 2301 0023 0000 2300 0123 FF00"
	$"2300 0023 0101 2300 0123 FF01 2300 0023"
	$"0101 2301 0023 0000 2200 5500 D900 FF23"
	$"00FF 23FE FE23 FF00 23FF 0023 FF01 23FF"
	$"0122 0058 00D4 FFFB 2200 5800 D400 0123"
	$"0102 23FF 0123 0000 23FD 0023 FF00 23FF"
	$"0023 0001 23FF 0023 FEFF 23FF 0123 0003"
	$"2300 0123 0002 2300 0023 0100 2200 5100"
	$"D601 0022 00A7 00B7 FFFE 23FE FE23 FFFF"
	$"2301 FE23 02FE 2303 FD23 02FD 2304 FF23"
	$"03FD 2200 9000 C200 0422 008C 00C4 FE04"
	$"2200 8900 CA02 FF23 0303 2301 0123 0100"
	$"2301 0223 0102 2301 FF23 0101 2302 0023"
	$"0103 2301 0023 0204 2301 0122 009A 00DE"
	$"FEFE 2200 9A00 DE00 0123 0301 2304 0123"
	$"0102 2301 0223 FE01 2300 0122 0097 00DB"
	$"0100 2200 9E00 E101 0023 0001 2301 0123"
	$"0000 2303 FF23 FFF9 23FD FE23 FFFD 23FB"
	$"FD23 FFFF 23FC FB23 F7FF 23FD FE23 FD01"
	$"2300 0223 FD02 2300 0022 008A 00C8 0000"
	$"2200 8C00 C5FF 0022 0097 00DC 0500 2303"
	$"0023 FFFF 23FF FE23 0000 23FF FF23 FEFE"
	$"2300 0023 FF01 23FF FF23 FF00 2300 0023"
	$"00FF 2200 8A00 C500 FF22 008A 00C5 0000"
	$"23FE FF23 00FC 2301 FE23 0300 2300 0023"
	$"01FF 2301 FF22 0089 00C3 0201 2200 8100"
	$"C801 0022 0064 00CD FE00 23FF 0023 FEFF"
	$"23FE 0023 FFFF 23FE FF23 FEFF 2300 FC23"
	$"FEFD 23FD FE23 00FE 23FF 0023 00FE 23FE"
	$"FF23 00FF 2300 0023 FFFE 2300 0023 FEFF"
	$"2300 0023 FF00 2300 FF23 00FF 23FF FD23"
	$"FF00 23FF FF22 0049 00CE FEFF 2200 4B00"
	$"D0FE FE22 004B 00D0 0102 2300 0023 0001"
	$"2301 0123 0001 2300 0123 0100 2301 FF23"
	$"0201 2200 8A00 C502 0023 0102 2303 0023"
	$"0102 2200 9000 CEFE FE22 0090 00CE 0200"
	$"2301 FD23 0301 2303 0022 0090 00D6 01FE"
	$"2200 9000 D601 0223 0101 2301 0123 0000"
	$"2302 0323 0202 2301 0023 01FF 2300 FF23"
	$"0100 2300 0022 0090 00D6 0000 2200 9A00"
	$"DD01 0022 0097 00E0 0100 2200 9000 D6FF"
	$"0022 007F 00CE 0404 2301 0022 00B0 00C2"
	$"0000 23FF 0023 FFFF 2200 AF00 BF01 0022"
	$"00AF 00BF FDFF 2300 0023 FDFF 2301 FE23"
	$"00FF 23FF FF23 FF01 23FE FD23 FF00 23FD"
	$"FF23 FF00 23FF 0023 FFFF 23FF 0123 FFFF"
	$"23FF FF23 FF00 23FF FF23 FCFF 23FF 0023"
	$"FE00 23FF 0023 FDFF 2300 FF23 FD01 23FE"
	$"0023 FF00 23FD 0123 FD00 23FB FD23 FCFE"
	$"2300 FF23 FFFF 23FF 0023 FDFE 23FF 0023"
	$"FFFE 23FD FF23 FCFE 2300 FC23 FEFF 2300"
	$"FF23 00FB 23FF FE23 FCFF 23FF 0123 FC00"
	$"23FD FE23 FD02 23FE FE23 FCFF 23FF FE23"
	$"FBFF 23FE 0123 FFFF 23FD 0223 FBFE 23FF"
	$"0023 FC00 23FF 0023 FDFC 2300 0023 01FD"
	$"2301 0023 FFFD 2302 FE23 FEFE 2301 FF23"
	$"00FE 2302 FF23 02FD 23FF FF23 FEFC 23FF"
	$"0023 01FD 2301 0023 02FE 2301 FE23 0200"
	$"2301 FF23 00FF 2301 FE22 00E5 00E6 02FF"
	$"2301 FF23 01FF 2301 FF23 00FD 2301 FF23"
	$"00FE 23FF FE23 0000 2300 FF23 FF00 23FE"
	$"FE23 FFFF 23FF FF23 FEFF 23FE 0023 00FF"
	$"23FF 0023 FE00 2300 0023 FD00 2200 B100"
	$"EE01 0023 0000 2302 FD23 00FE 23FD FE22"
	$"00B5 00EF 0100 23FF FF23 01FF 2200 B100"
	$"EF05 0022 00AA 00EE 0000 2200 AE00 F1FF"
	$"0522 00C7 00D1 00FE 2300 0023 01FE 23FF"
	$"FD23 FD01 2200 C300 CD01 FE22 00C3 00CD"
	$"FE00 2200 C100 CB00 0222 00C1 00CB FDFD"
	$"23FF 0023 FFFF 23FF 0023 00FF 2300 0023"
	$"FDFF 23FF FE23 0000 2200 C100 CE00 0023"
	$"03FF 2200 BA00 C001 FF22 00C1 00CE 0000"
	$"23FE 0023 0001 2200 C300 CB01 FF22 00C3"
	$"00CB FE00 2301 FF22 005C 0055 01FF 2300"
	$"FF23 FFFF 2300 FE23 FEFE 2300 FF23 FFFF"
	$"2300 FE23 FDFD 23FE FF23 FFFE 23FE 0223"
	$"FDFF 23FD FE23 FFFE 23FE 0023 00FE 23FE"
	$"FE23 01FD 2300 FE23 FF00 2301 FE23 03FE"
	$"2300 FF23 FDFE 23FF 0023 FFFC 2300 0023"
	$"FDFF 2300 FE23 01FF 23FF 0023 00FF 2300"
	$"FE23 FF00 2200 D000 DCFC FF23 FC00 23FD"
	$"0323 0000 23FD 0023 FDFD 23FD 0122 00B5"
	$"00C0 02FD 2303 FF22 00B4 00C6 FFFD 2200"
	$"B400 C601 0222 00B1 00C5 00FE 2302 0223"
	$"03FE 2301 0023 0101 2301 0123 0100 2301"
	$"0123 01FF 2301 0023 0200 2301 0023 0100"
	$"2302 0023 0101 2301 0123 0200 2301 0123"
	$"00FF 2301 0123 0101 2300 0023 01FF 2300"
	$"0123 0000 2302 0023 0002 2301 0123 0000"
	$"2300 0023 0001 2300 FF23 0101 2300 0023"
	$"FF02 2300 0123 0300 2200 B100 C500 0022"
	$"00B8 00E2 0200 2301 0023 0000 2302 0123"
	$"0100 2301 0123 0100 2301 0123 0001 2301"
	$"0123 0201 2301 0123 0101 2301 0023 0103"
	$"2301 0023 0100 2300 0123 0102 2301 0123"
	$"0100 2300 0023 0100 2302 0123 0301 2303"
	$"0223 01FE 2300 FF23 0000 2303 0223 0101"
	$"2301 0023 0100 2301 FF23 0201 2301 0023"
	$"0200 2301 0123 0302 2302 0023 0100 2301"
	$"0123 0301 2302 0023 0202 23FF 0123 0001"
	$"23FF 0023 FE02 2300 0323 0001 2300 0123"
	$"FF01 2200 8A00 C800 0023 00FF 2303 0023"
	$"0201 2301 0123 0101 2301 0123 0103 2301"
	$"0023 0100 2302 0123 0100 2301 0023 0300"
	$"2301 0123 01FF 2301 FF23 FF01 2200 9000"
	$"D3FF 0022 0091 00DC 00FF 2200 F501 0300"
	$"0023 00FF 2300 0023 FFFF 2300 0023 FFFF"
	$"23FF FF23 00FF 23FF 0023 FEFF 2300 0023"
	$"0000 23FF 0023 FF00 23FF 0023 00FF 2300"
	$"0023 00FF 2300 0023 FF00 23FF FF23 02FF"
	$"2300 FF23 01FE 2300 0023 0000 2300 FE23"
	$"0000 2200 F501 0302 0122 00EA 00FA 0000"
	$"2200 DC01 1700 0123 FD01 2300 FF23 FE02"
	$"23FF 0123 FE02 23FF 0023 0000 23FF 0023"
	$"FF01 23FC 0023 0000 23FF FF23 FF00 23FF"
	$"FF23 FE00 23FF 0023 FF00 23FF 0023 FFFF"
	$"23FE 0123 0000 2300 0123 0000 23FF 0123"
	$"0001 2300 0023 FF01 2300 0122 00F6 0106"
	$"02FF 2300 FE23 0100 2200 F201 0AFF 0122"
	$"00F2 010A 01FE 2302 FF22 00EF 010E FF00"
	$"2200 EF01 0E01 0023 0100 2200 EF01 11FF"
	$"0022 00EF 0111 01FF 2301 FE22 00EF 0111"
	$"0000 2200 EC01 1300 0022 00CA 010E FD01"
	$"2300 0023 0002 23FF 0023 0101 2304 0023"
	$"0200 23FF 0122 00EC 0113 FE01 2300 FF23"
	$"FF00 23FE 0123 FF00 23FF 0023 FEFE 23FE"
	$"0123 0000 23FE 0023 0000 23FD 0023 FF00"
	$"2300 FF23 FF00 2300 FE23 FF00 2300 FF23"
	$"00FF 23FD FE23 FFFF 23FF FF23 FF00 23FF"
	$"FF23 FCFF 23FF 0023 00FF 23FE FE23 FF00"
	$"2300 FF22 00DE 00EC 05FF 2200 DE00 ECFF"
	$"0123 FFFE 2300 0023 FF01 23FF 0123 FF00"
	$"23FF 0023 FF00 2300 FF23 FE00 23FF 0123"
	$"0002 23FE 0023 0002 23FF 0223 0001 2303"
	$"0222 00ED 0111 03FF 2200 C300 D100 FD22"
	$"00C3 00D1 0101 23FF 0123 0100 23FF 0123"
	$"0001 2302 0023 0100 2300 0123 0101 2300"
	$"0122 00C9 00D7 0202 2300 FE23 0302 2301"
	$"0023 0203 2301 0023 0102 2301 0023 0101"
	$"2302 0123 02FF 2300 FD23 0101 2300 FF23"
	$"0101 2301 0023 0000 2301 0122 00C9 00D5"
	$"0200 2200 B801 0702 FF22 00B6 010A FF01"
	$"2200 B601 0A01 0123 0000 2303 0123 01FE"
	$"2200 B801 0D02 FE22 00B8 010C 0100 2200"
	$"B801 0CFF 0423 0101 2300 0123 FF01 2300"
	$"0123 0000 2200 B501 10FF 0123 FFFF 2200"
	$"B801 0C00 FE22 00C0 010B 0000 2200 B501"
	$"1002 FE22 00B5 010E 0000 23FF 0122 00AF"
	$"00B2 FF00 2200 AF00 B001 0022 00AF 00B0"
	$"FF00 2200 E700 DA04 0222 00E7 00DA FFFE"
	$"2300 0022 00E4 00D8 0100 2300 FF23 0000"
	$"2302 0123 0402 2300 0023 0102 2300 0022"
	$"00E5 00D9 FFFF 2200 E800 E0FF FE22 00A6"
	$"009F FD00 23FE 0023 01FE 23FE FF23 FFFF"
	$"23FF 0123 FF01 2300 FF23 FE01 23FF 0123"
	$"FD01 23FF FF23 FFFE 23FF FE23 00FF 23FD"
	$"FE23 FFFE 2301 FF23 FEFF 23FF FE23 0000"
	$"2300 0123 0001 2200 EC01 1400 0023 0100"
	$"2301 FF23 01FF 2302 0023 01FF 2300 0023"
	$"01FF 2301 0122 00E8 011D FF01 2200 E801"
	$"1DFF 0123 0001 2300 0023 0001 2300 0023"
	$"0000 2301 0023 0000 2301 0023 00FF 2302"
	$"0123 0000 2301 0023 0000 23FF 0123 FF01"
	$"23FF FF23 FF01 23FE 0022 00EA 011E 0201"
	$"2200 CE00 E001 0023 0100 2302 0023 01FF"
	$"2300 0123 0300 2303 0423 0002 2301 0223"
	$"0100 2301 0023 0000 2300 0023 0100 2200"
	$"EB01 2000 0023 00FE 2300 FF23 0100 2302"
	$"FF23 00FF 2301 FF23 01FF 2301 0023 0101"
	$"2302 0023 0000 2200 EF01 1101 0023 0000"
	$"2301 0023 0100 2300 0022 00EA 00FA 0001"
	$"2301 0023 0201 2300 0023 0000 2300 0023"
	$"0101 2301 FF23 0000 2301 0123 0000 2301"
	$"0123 0101 2300 0023 0200 2301 0023 00FF"
	$"2300 FE23 FE00 2200 CE01 02FE 0022 00CE"
	$"0102 0000 2301 0023 0101 2301 0123 0302"
	$"2301 0222 00CF 0105 02FF 2302 FF23 0200"
	$"2300 0123 0101 2301 0123 0102 2301 0023"
	$"02FE 2300 FE22 00CF 0105 0001 2200 CF01"
	$"1100 FF09 0000 0000 0000 0000 3100 1800"
	$"B600 2201 0A39 A000 8DA1 0096 000C 0100"
	$"0000 0200 0000 0000 0000 A100 9A00 08FF"
	$"FB00 0000 2F00 0001 000A 0013 00B6 0023"
	$"0117 0D00 0C2E 0004 0000 0100 2BB7 1F0D"
	$"4265 6471 7569 6C74 2043 6176 65A0 0097"
	$"A100 9600 0C01 0000 0002 0000 0000 0000"
	$"00A1 009A 0008 0009 0000 003D 0000 0100"
	$"0A00 DE00 7C01 0500 F92C 0009 0003 0647"
	$"656E 6576 6103 0003 0D00 0A28 00E8 007D"
	$"0E43 6176 65C9 7468 6520 576F 726B 732A"
	$"0D0B 6279 204D 656C 2050 6172 6B2A 0D18"
	$"4361 7665 2052 6573 6561 7263 6820 466F"
	$"756E 6461 7469 6F6E A000 97A0 008D A000"
	$"83FF"
};

#endif

resource 'PICT' (1001,purgeable) {
	7997,
	{-1, -1, 268, 302},
	$"1101 A000 82A0 008C A000 8C01 000A FFFF"
	$"FFFF 010C 012E 2200 5000 D902 0123 FE02"
	$"2300 0223 0001 2200 5900 DCFD FD22 0059"
	$"00DC 01FF 2302 0023 0100 2302 0123 0100"
	$"23FF 0223 0201 23FF 0123 0202 23FF 0123"
	$"0103 2301 0423 0202 23FD 0223 0002 2300"
	$"0223 0300 2303 0023 0202 23FF 0123 0003"
	$"2300 0123 0100 2200 5500 D900 0122 006B"
	$"00E5 FFFF 2200 7600 ED01 0223 0302 2300"
	$"0123 0102 2302 0123 0202 2304 0122 0082"
	$"00FD FDFF 2200 8200 FD05 0323 0200 2301"
	$"0023 00FE 2303 0123 0501 2200 8401 08FE"
	$"0222 0088 0118 0001 23FF FE23 FBFF 23FF"
	$"0123 FF01 23FF FE23 FF00 2300 FF23 FF00"
	$"2300 0022 0051 00AD 0101 2302 0123 0202"
	$"2302 0023 00FF 2300 FF23 02FF 2302 0023"
	$"02FF 2302 0123 0300 2302 0023 0102 2300"
	$"0123 0200 23FF 0123 0102 23FF 0123 0102"
	$"2301 0123 0002 2300 0123 0201 2301 0023"
	$"0200 2301 0023 0000 2302 0023 FF02 2300"
	$"0223 FF00 2300 0123 0003 23FF 0223 FE02"
	$"2300 0423 FF03 2300 0323 0002 23FE 0223"
	$"0000 2301 0123 0003 2302 0223 0000 2300"
	$"0123 0101 2300 0223 0001 2302 0223 FD02"
	$"2300 0123 0000 23FE 0223 0001 23FE 0223"
	$"FE02 2303 0323 0303 2303 0023 0202 2300"
	$"0423 0302 2303 FE23 0103 2303 FF23 0400"
	$"2301 FF23 0302 2200 A100 E001 FD23 FEFD"
	$"23FF FE23 FEFF 2300 0023 FF01 23FE FF23"
	$"0000 23FE 0023 0001 23FD FF23 FDFE 23FE"
	$"FF23 FFFF 2300 0023 FEFE 23FF 0223 FE00"
	$"23FE 0023 FD01 23FE 0022 0096 00BF 03FF"
	$"2200 A000 DF01 0122 009F 00E1 00FF 2200"
	$"9600 BF00 0122 00A2 0126 0000 2300 FF23"
	$"00FF 23FF FF23 0000 23FF 0023 FF00 23FF"
	$"FE23 FFFF 23FF 0023 FEFF 2300 0123 FF00"
	$"23FF FE23 FF00 23FF FD23 0000 2301 FE23"
	$"00FF 2301 FF23 01FD 2302 0123 01FF 2300"
	$"FE23 FF00 23FF FE23 FFFF 23FF FE23 FD00"
	$"23FD FE22 00A3 0126 00FF 2200 8901 19FF"
	$"FF23 0000 2200 A301 2602 FF23 0102 2300"
	$"0123 0101 23FF 0023 0200 2302 0322 0090"
	$"010E 01FE 2301 0023 01FE 2300 FF23 02FE"
	$"2300 FE23 FDFF 23FE FF23 FDFF 23FE 0023"
	$"FF00 23FE 0023 FF00 23FE FF23 FF01 23FD"
	$"FD23 FE00 23FF 0023 FC00 23FF 0123 00FF"
	$"23FD FE23 FDFF 23FF FF23 00FF 23FF FE23"
	$"FFFE 23FD FE23 FA00 23FE FF23 00FF 23FF"
	$"FE23 0000 23FF 0023 FEFF 2300 FF23 FF00"
	$"23FE 0023 00FF 23FF 0023 FE00 23FF FE23"
	$"0000 2301 FF23 00FF 23FF 0023 0000 2301"
	$"0023 0000 2300 FF23 00FD 2300 0023 FF00"
	$"23FF 0023 01FF 23FF 0023 FEFF 2300 FF23"
	$"FF00 2300 0023 0000 2300 0023 FFFF 2300"
	$"0023 FFFF 23FF FF23 FF01 2200 8701 1301"
	$"FF22 0067 00D9 FF01 2200 6200 D000 FF22"
	$"0068 00D8 FFFF 2300 0023 0000 23FF 0023"
	$"FFFF 2300 0023 00FE 23FF FF23 FFFF 23FF"
	$"0023 FFFE 2300 0023 0002 2200 6100 D100"
	$"0022 0086 0114 0000 2300 0122 0087 0113"
	$"0100 2200 8701 13FF 0023 FE00 2300 0023"
	$"0001 2200 4800 7DFD FD23 FDFE 23FF FF23"
	$"FDFE 23FD FF23 FD00 23FE FD23 FEFF 23FD"
	$"FE23 FEFC 23FE FE23 01FD 2301 FC23 00FD"
	$"23FD FE23 FCFE 23FD FD23 FCFE 23FD FE23"
	$"FD02 23FD FE23 FDFC 23FD FE23 FC00 23FC"
	$"0122 0042 0076 03FD 2200 2700 3404 FD23"
	$"01FD 2302 FE23 03FB 2301 FF22 003F 0079"
	$"0303 2303 0123 03FE 2303 0123 03FC 2302"
	$"FE23 FEFF 23FE FF22 003C 008A 0002 2302"
	$"0023 0100 2303 0223 02FF 2302 0223 0301"
	$"2302 0022 003F 0092 0002 2300 0123 0000"
	$"2300 0123 0000 2300 0123 0000 23FA FE22"
	$"0044 0092 0400 2200 4500 9702 0122 0044"
	$"0096 FF02 23FF 0023 0101 2301 0023 0100"
	$"2300 0023 0000 23FE 0123 FE00 2300 FF23"
	$"FF00 23FE FF22 0044 0096 0203 2200 4700"
	$"97FF FF22 0046 0090 FF01 2200 4600 9600"
	$"0023 FEFF 2301 FE23 0000 2300 0022 0047"
	$"008F FEFF 2301 0023 01FF 2301 0023 01FF"
	$"2302 0123 0000 2301 0023 01FF 2301 0023"
	$"0201 2301 FF23 0100 2301 FF23 0100 2300"
	$"0023 FF00 2300 0023 FF00 23FF 0023 0000"
	$"2200 4500 98FF FF22 0043 0099 00FF 23FC"
	$"FE23 FE00 23FF FF23 FEFF 23FF 0223 FEFF"
	$"23FE FD23 01FF 23FD FF23 00FB 2301 FF23"
	$"01FC 23FF FE23 01FF 2301 0023 00FC 2301"
	$"FF23 00FE 2300 FF23 0200 2300 FF23 0200"
	$"2302 FF23 00FF 2300 FE23 FFFE 23FD FC23"
	$"FDFD 23FF FE23 FDFE 23FF 0023 FE02 23FC"
	$"0023 FCFE 23FC FE23 FEFE 23FE FD23 FEFD"
	$"23FE 0123 FFFF 23FF 0123 FDFF 23FF FF23"
	$"0000 2300 FF23 02FC 2300 FF23 FCFF 23FE"
	$"0023 0000 23FF 0023 FD01 2300 FF23 FE00"
	$"2300 0223 0001 2300 0223 0101 2300 0123"
	$"0101 2200 2300 9300 0122 0044 0097 0000"
	$"23FE 0023 FEFF 23FE 0023 FF00 2300 0023"
	$"FE00 23FF FF23 00FF 2300 FE23 0200 2300"
	$"0022 0044 0097 04FF 2200 3F00 8E01 0122"
	$"0024 0093 0101 2302 0023 01FF 2301 FE23"
	$"FEFC 23FD FC23 FDFD 23FE FE23 00FE 23FC"
	$"FD23 FD00 23FD 0023 FEFF 23FF 0023 FFFF"
	$"23FB FF23 FCFF 23FF FF23 FDFD 23FD FD22"
	$"000B 0076 0001 2200 0400 70FD FE22 000C"
	$"0076 0201 2301 0123 0302 2303 0223 0203"
	$"2200 4300 9B02 0123 0101 2302 0123 0202"
	$"2301 0023 0001 2302 0023 0000 2301 0023"
	$"0001 23FF 0023 0302 2301 0123 02FF 2300"
	$"0023 0200 2300 0023 01FF 2301 0123 0001"
	$"2300 0123 0100 2300 0023 0100 2200 4E00"
	$"B2FF 0022 004E 00B2 0100 2300 0023 FFFE"
	$"2200 4C00 AD00 0022 0047 0098 0201 2302"
	$"0223 0001 2304 0023 02FF 2302 0022 00AE"
	$"00BF 0001 2303 0123 0001 23FE 0223 0106"
	$"23FB 0423 0305 2300 0623 0504 2304 0423"
	$"0102 2305 0223 0202 2304 FE23 03FE 2301"
	$"FF23 0101 2303 0023 04FF 2302 0123 0302"
	$"2303 0223 06FF 2301 FD23 02FF 2303 0023"
	$"03FE 2300 FE23 FEFF 23FF 0023 00FE 23FF"
	$"FF23 01FC 23FD FB23 00FC 23FD FC23 FFFD"
	$"2301 FC23 FDFE 23FE FD23 01FE 2304 0023"
	$"FFFC 23FC FD23 FFFE 23FC FB23 FFFE 23FF"
	$"FD23 FFFD 23FD FD23 FDFB 23FF FE23 FEFF"
	$"23FF FF23 FEFF 23FD FD22 00B1 00C2 00FF"
	$"2200 B900 C1FF FC22 00D1 00E0 FC00 2200"
	$"B500 ED01 FC22 00B5 00ED 0102 2302 0123"
	$"01FF 2301 0323 0102 2302 0423 0201 2301"
	$"FF23 0101 2301 FE23 01FF 2302 0123 0000"
	$"2301 0123 04FF 2302 FF23 0100 2303 FD23"
	$"0100 2301 FF23 00FF 2301 FF23 00FD 2303"
	$"0022 00B7 00F1 FEFE 2200 BB01 09FF 0422"
	$"00B5 010E 00FD 2303 FF23 00FF 2300 FE23"
	$"0300 2301 FF22 00AF 0118 01FE 23FF FE23"
	$"FF00 2300 0123 FDFF 2300 0123 FF01 23FE"
	$"0122 00BF 0108 0101 2200 C301 0BFE FD22"
	$"00C3 010B 00FE 23FF 0122 00C2 0108 0200"
	$"2200 C201 0800 FE23 FF00 23FF 0023 FFFF"
	$"23FF 0123 0000 23FE FE22 00C0 0104 FFFF"
	$"2200 BE01 02FE FE22 00BE 0102 0001 2200"
	$"BF01 03FE FF23 0000 2300 FE23 01FF 2300"
	$"FE23 FF00 23FE 0123 FF01 2301 0123 FF00"
	$"23FF 0123 0000 23FF 0123 0000 23FE FE23"
	$"0000 23FF 0123 FFFF 2300 0023 FFFF 2300"
	$"FF23 00FF 2300 0023 0000 23FF FF23 FF00"
	$"23FF 0023 FEFE 23FF FF22 00BC 0100 FEFF"
	$"23FD FD23 01FE 23FE FE23 FCFE 23FF FF22"
	$"00B4 00FA 0201 2301 0023 00FF 2300 0023"
	$"02FE 2301 FF23 0200 2300 0223 0201 2301"
	$"0123 0301 2300 0122 00CE 0111 FFFF 2300"
	$"FE23 01FE 2301 FE23 00FD 23FF FF23 FE00"
	$"23FE 0023 FE00 2300 FF23 FFFE 2200 BF01"
	$"0800 0022 00CD 0110 0002 2200 BF01 0A00"
	$"FF22 00BF 0108 0101 2200 CF01 1002 0123"
	$"FFFF 2300 FF23 FFFE 23FE FF23 FFFD 2300"
	$"FE23 FF00 2301 FE23 FEFF 2200 C001 0B00"
	$"0322 00C0 010B 00FF 2200 BE01 0CFF 0122"
	$"00BE 010A 0200 2200 C801 0D01 0222 00BE"
	$"010A FE01 2201 0B00 5004 FE23 F3F5 2309"
	$"F623 0300 2301 FF23 00FC 2304 FD23 00FA"
	$"2301 FE23 0500 2303 0023 0500 2302 FF23"
	$"04FF 2301 FF23 0403 2303 FF23 02FE 23FF"
	$"FD23 FFFE 23FC FD23 FFFD 2301 FE23 FFFE"
	$"2300 FC23 FBFC 23FF FB23 FDFC 2304 F723"
	$"00FF 2301 FF23 FFFE 2302 FD23 FEFD 2300"
	$"FA23 04FD 23FF FB23 00FB 2303 FF23 0700"
	$"2303 0123 0300 2300 0423 0200 2302 FF23"
	$"0302 2304 0023 04FF 2303 0023 0200 23FF"
	$"0123 0302 2302 0023 0403 2305 0223 01FF"
	$"2200 B700 6AFD 0022 00A8 006A F901 2200"
	$"A000 8403 FF22 00A6 00A2 02FF 2200 A600"
	$"9F03 0123 03FF 2302 0023 02FF 2302 0123"
	$"0004 2303 0023 0301 2304 0223 0402 2303"
	$"FD23 00FF 2302 0223 0101 2301 0023 0202"
	$"2300 0123 0103 2300 0123 0000 2302 0123"
	$"0001 2300 0123 0100 2300 0023 FF00 2300"
	$"0223 FF02 23FF FF23 00FF 23FF 0022 00B9"
	$"00C1 0101 2200 A600 A1FE 0022 00AB 00BC"
	$"0000 2303 FF22 00AE 00BF FFFF 2200 B400"
	$"C300 0022 00BC 00C4 00FF 2200 B900 C100"
	$"0022 00A5 00A4 0104 2304 0223 0402 2302"
	$"0123 0402 2302 0623 0202 2302 0023 0300"
	$"2200 B800 BEFE 0022 00B8 00BE 0100 2302"
	$"0123 0103 2302 0123 0305 2303 0023 0001"
	$"2301 0023 0200 2301 0123 0000 23FF 0123"
	$"FF02 23FF 0023 0003 23FD FF23 FFFE 23FF"
	$"FF23 FDFE 23FD FF23 FEFD 2200 BD00 C400"
	$"0122 00C5 00CD 02FF 2200 CA00 CB00 0122"
	$"00C0 00BE FEFE 2200 B700 6700 0123 0000"
	$"23FB 0023 FDFD 2300 FF23 FEFE 2300 FF23"
	$"FFFF 23FE FD23 F9FE 23FC FF23 FBFB 23FD"
	$"FF23 FDF6 23FB FF23 FFFE 23FF 0023 FDFE"
	$"23FF FF23 01FE 23FD FC23 FBFC 23FD FD23"
	$"FBF8 23FA FE23 FCFE 23FF FF23 FCFE 23FB"
	$"FA23 FCFD 23FC 0123 FE01 23FD FF23 FD02"
	$"23FB FC22 00A9 0063 F804 23FF 0022 0046"
	$"0099 0201 2301 FF23 0900 2305 0023 0802"
	$"2305 0323 0201 2303 0123 00FC 2303 FF23"
	$"0202 2303 0023 0201 2304 FF23 0301 2307"
	$"0123 0300 23FF 0223 0101 2302 0022 0046"
	$"009C 0101 2200 5000 D900 FF22 0047 009D"
	$"FC07 23F9 0123 F800 23F9 FB23 FAFC 23FC"
	$"0723 FF04 2304 0823 0703 23FF 0523 FA02"
	$"23FD FF23 FE09 23FD 0423 FC02 2300 0923"
	$"FF05 2306 0323 0009 2300 0323 0703 2304"
	$"0023 0606 2301 0323 0305 2304 0423 0801"
	$"2304 0023 0301 2300 0223 0500 2200 BB00"
	$"C400 0023 0201 2300 0223 0002 2303 0123"
	$"0202 2301 0023 0000 2301 0022 006A 00E4"
	$"FF00 23FF 0023 FE01 23FE FF23 FEFD 23FE"
	$"0123 FDFF 2300 0023 FE00 23FF FF23 FE00"
	$"23FF FF23 FEFF 2300 0023 FFFF 2300 0122"
	$"00A9 00BF 0201 2301 FD23 03FF 2303 FD23"
	$"04FE 2301 FD23 FFFB 23FE FC22 0092 00C7"
	$"0303 2200 9200 C7FE FE23 FFFC 2301 FF22"
	$"008A 00C8 FD01 2200 8A00 C8FD 0223 00FD"
	$"2302 FE23 01FE 2302 0023 02FE 2200 8200"
	$"CB01 0122 0082 00CB 0000 23FE FF23 0100"
	$"2303 FD22 007E 00CE FF00 2200 7E00 CEFE"
	$"FE22 0078 00CB 0104 2200 7800 CB02 FE23"
	$"FEFC 2300 FD23 FFFF 23FF FC23 00FD 2303"
	$"FF23 02FF 23FE FC23 01FF 2301 FF23 0000"
	$"2200 8A00 C802 FF22 0089 00C5 0000 2200"
	$"7E00 CE00 0122 0065 00CE FFFF 2200 5F00"
	$"CE00 0022 009F 0087 0000 2300 0023 FEFC"
	$"23FC FF23 FD00 23FE FD23 FF00 23FF FE23"
	$"FE01 23FD FE23 FFFF 23FE FE23 FEFD 23FF"
	$"FF23 FFFE 2300 FE23 00FF 2301 FF23 01FE"
	$"2302 0023 02FE 2200 9F00 8703 0322 0083"
	$"0074 0201 2303 FE23 02FF 2301 0023 00FE"
	$"2300 FD23 01FE 2302 FF23 00FE 23FE FF23"
	$"FFFD 2300 FF23 00FE 2300 0023 00FF 23FE"
	$"0022 00A2 008A FF04 23FE 0123 0003 2300"
	$"0223 0101 2300 0123 0100 2200 B800 C500"
	$"0123 0002 2300 0123 FD00 2300 0023 0000"
	$"23FE 0023 0000 23FE 0123 FF00 2200 BB00"
	$"BCFD FD22 00BC 00C0 0101 2200 BD00 BDFF"
	$"FE22 00BA 00BC 0001 2200 B800 B900 0022"
	$"00BA 00BC 0001 2301 0123 0001 2301 0023"
	$"0101 2300 0023 0001 2300 0123 0000 2300"
	$"0023 0002 2300 0023 0001 2302 0123 0000"
	$"2301 0122 00B4 00C3 02FF 2301 0223 0200"
	$"2300 0023 0501 2303 FF23 02FC 2302 0223"
	$"0300 2302 0023 01FF 2302 0123 0402 2301"
	$"0123 0102 2300 0123 0002 23FF 0122 00AE"
	$"008B 02FF 2304 0223 0401 2305 0023 0201"
	$"2305 0123 01FF 2303 0023 03FE 2303 FF23"
	$"0401 2301 0022 00CD 00DC 00FF 23FE 0023"
	$"FDFF 23FD FE22 00C8 00D2 0201 2200 C900"
	$"D101 FF22 00C7 00D1 0002 2200 C800 CF02"
	$"FF22 00CD 00DC 0201 2200 CA00 CB04 FE22"
	$"00CE 00DF FF00 2200 CE00 DF01 0023 0100"
	$"2301 0023 0100 2301 FF23 0100 2303 0123"
	$"0203 2301 0123 0302 23FE 0123 0201 2300"
	$"0022 00CE 00DF 0100 2200 D000 C701 FE23"
	$"0001 23FF FE23 01FF 2301 0123 0100 2301"
	$"0023 0001 2302 0123 01FF 2300 0123 01FF"
	$"2302 0023 00FE 2301 FF23 0201 2300 0023"
	$"0201 2301 0023 00FF 2302 0123 02FF 2301"
	$"0122 00D0 00C7 0001 2200 CD00 DC00 0022"
	$"00D1 00C7 0101 2300 0023 0201 2301 0023"
	$"0002 2303 FF22 00D5 00CE 00FF 2200 D500"
	$"CEFF 0123 FE00 2300 0023 FE00 23FE 0023"
	$"FFFD 2304 FD23 FB01 2301 FF22 00CE 00C6"
	$"0002 2200 CE00 C6FC FF22 00CE 00C6 FF00"
	$"2200 CB00 C101 0222 00CB 00C1 FB00 23FD"
	$"FE22 00C9 00B6 0300 2200 C900 B6F9 FA23"
	$"FCFD 23FE FF23 FCFF 2300 FE23 FFFF 23FE"
	$"FE23 FD00 23FF FF23 FBFF 23FB FF23 FB00"
	$"23FD 0223 FBFD 23FB 0223 FFFE 23FB FC23"
	$"F802 23FF FF23 FAFD 23FC 0223 FAFC 23FC"
	$"0023 FCFE 2200 CB00 C104 0122 00B8 009E"
	$"00FD 2200 CD00 C503 FE23 06FE 2303 0022"
	$"00C4 00CF 0201 2300 FD23 00FE 23FD 0023"
	$"FF00 23FF 0023 FEFF 2200 BE00 BC00 FF23"
	$"0001 23FD FF23 FF02 23FE FF22 00BD 00BC"
	$"0101 2301 FF23 0301 2301 0223 0303 2301"
	$"FF23 0305 2302 0123 0101 2302 0022 00B4"
	$"009C 0000 2200 B400 9DFF 0022 00B4 009D"
	$"0100 2301 FE23 0300 2301 FF23 0400 2302"
	$"0023 0301 2302 0023 0200 2300 0223 0201"
	$"2301 0122 00B6 00B4 FF00 2200 B600 B401"
	$"0123 0401 2301 0423 0203 2301 0123 0100"
	$"2200 AB00 BCFE FF23 FF01 2300 0023 FDFF"
	$"2300 0023 0003 23FF FF23 FF01 23FE 0122"
	$"00A9 00B7 FF01 2300 0122 00B1 00B2 00FD"
	$"2200 AB00 B600 0023 0202 2300 0323 0001"
	$"23FF 0023 0000 2301 0023 0302 2301 0123"
	$"0401 2303 0023 0201 2300 0123 0001 2302"
	$"0423 0102 2302 0222 00BF 00C7 0000 2200"
	$"B800 C500 0022 00BE 00C8 FF01 2302 0222"
	$"00BD 00C1 0000 23FE 0223 0002 2301 0223"
	$"0201 2300 0123 0000 2301 0023 0102 23FF"
	$"0123 0100 2300 0223 0101 2303 FF23 00FF"
	$"2301 FF23 0000 2300 FF23 0101 23FE 0123"
	$"0000 2300 0022 00BE 00C4 FF00 23FF 0023"
	$"00FF 2300 0022 00BC 00C2 FE00 2200 AF00"
	$"B200 0123 0100 2200 CB00 CB00 0023 0101"
	$"2302 0123 0001 2302 0123 0101 2302 FF23"
	$"0001 2301 0223 0001 2300 0223 01FF 2302"
	$"0223 0000 2301 0123 0101 2300 FF23 0200"
	$"2300 0123 0101 2300 0023 0201 2200 DE00"
	$"DEFD FF23 FF00 23FE 0023 FF01 23FF 0123"
	$"FF02 2302 0023 0001 2300 0023 0102 2300"
	$"0022 00DA 00DE 0004 2200 E400 D800 0022"
	$"00CE 00C5 0300 2300 0023 0300 2301 FF23"
	$"0200 2302 0123 0100 2303 0123 0102 2304"
	$"0123 0000 2300 0223 0002 2300 0023 0002"
	$"2200 D200 D903 0122 00C5 00C2 0000 2300"
	$"FF23 0102 2301 0023 0000 2300 0123 FF00"
	$"2300 0023 0101 2300 0123 FF01 2300 0023"
	$"0101 2301 0023 0000 2200 5500 D900 FF23"
	$"00FF 23FE FE23 FF00 23FF 0023 FF01 23FF"
	$"0122 0058 00D4 FFFB 2200 5800 D400 0123"
	$"0102 23FF 0123 0000 23FD 0023 FF00 23FF"
	$"0023 0001 23FF 0023 FEFF 23FF 0123 0003"
	$"2300 0123 0002 2300 0023 0100 2200 5100"
	$"D601 0022 00A7 00B7 FFFE 23FE FE23 FFFF"
	$"2301 FE23 02FE 2303 FD23 02FD 2304 FF23"
	$"03FD 2200 9000 C200 0422 008C 00C4 FE04"
	$"2200 8900 CA02 FF23 0303 2301 0123 0100"
	$"2301 0223 0102 2301 FF23 0101 2302 0023"
	$"0103 2301 0023 0204 2301 0122 009A 00DE"
	$"FEFE 2200 9A00 DE00 0123 0301 2304 0123"
	$"0102 2301 0223 FE01 2300 0122 0097 00DB"
	$"0100 2200 9E00 E101 0023 0001 2301 0123"
	$"0000 2303 FF23 FFF9 23FD FE23 FFFD 23FB"
	$"FD23 FFFF 23FC FB23 F7FF 23FD FE23 FD01"
	$"2300 0223 FD02 2300 0022 008A 00C8 0000"
	$"2200 8C00 C5FF 0022 0097 00DC 0500 2303"
	$"0023 FFFF 23FF FE23 0000 23FF FF23 FEFE"
	$"2300 0023 FF01 23FF FF23 FF00 2300 0023"
	$"00FF 2200 8A00 C500 FF22 008A 00C5 0000"
	$"23FE FF23 00FC 2301 FE23 0300 2300 0023"
	$"01FF 2301 FF22 0089 00C3 0201 2200 8100"
	$"C801 0022 0064 00CD FE00 23FF 0023 FEFF"
	$"23FE 0023 FFFF 23FE FF23 FEFF 2300 FC23"
	$"FEFD 23FD FE23 00FE 23FF 0023 00FE 23FE"
	$"FF23 00FF 2300 0023 FFFE 2300 0023 FEFF"
	$"2300 0023 FF00 2300 FF23 00FF 23FF FD23"
	$"FF00 23FF FF22 0049 00CE FEFF 2200 4B00"
	$"D0FE FE22 004B 00D0 0102 2300 0023 0001"
	$"2301 0123 0001 2300 0123 0100 2301 FF23"
	$"0201 2200 8A00 C502 0023 0102 2303 0023"
	$"0102 2200 9000 CEFE FE22 0090 00CE 0200"
	$"2301 FD23 0301 2303 0022 0090 00D6 01FE"
	$"2200 9000 D601 0223 0101 2301 0123 0000"
	$"2302 0323 0202 2301 0023 01FF 2300 FF23"
	$"0100 2300 0022 0090 00D6 0000 2200 9A00"
	$"DD01 0022 0097 00E0 0100 2200 9000 D6FF"
	$"0022 007F 00CE 0404 2301 0022 00B0 00C2"
	$"0000 23FF 0023 FFFF 2200 AF00 BF01 0022"
	$"00AF 00BF FDFF 2300 0023 FDFF 2301 FE23"
	$"00FF 23FF FF23 FF01 23FE FD23 FF00 23FD"
	$"FF23 FF00 23FF 0023 FFFF 23FF 0123 FFFF"
	$"23FF FF23 FF00 23FF FF23 FCFF 23FF 0023"
	$"FE00 23FF 0023 FDFF 2300 FF23 FD01 23FE"
	$"0023 FF00 23FD 0123 FD00 23FB FD23 FCFE"
	$"2300 FF23 FFFF 23FF 0023 FDFE 23FF 0023"
	$"FFFE 23FD FF23 FCFE 2300 FC23 FEFF 2300"
	$"FF23 00FB 23FF FE23 FCFF 23FF 0123 FC00"
	$"23FD FE23 FD02 23FE FE23 FCFF 23FF FE23"
	$"FBFF 23FE 0123 FFFF 23FD 0223 FBFE 23FF"
	$"0023 FC00 23FF 0023 FDFC 2300 0023 01FD"
	$"2301 0023 FFFD 2302 FE23 FEFE 2301 FF23"
	$"00FE 2302 FF23 02FD 23FF FF23 FEFC 23FF"
	$"0023 01FD 2301 0023 02FE 2301 FE23 0200"
	$"2301 FF23 00FF 2301 FE22 00E5 00E6 02FF"
	$"2301 FF23 01FF 2301 FF23 00FD 2301 FF23"
	$"00FE 23FF FE23 0000 2300 FF23 FF00 23FE"
	$"FE23 FFFF 23FF FF23 FEFF 23FE 0023 00FF"
	$"23FF 0023 FE00 2300 0023 FD00 2200 B100"
	$"EE01 0023 0000 2302 FD23 00FE 23FD FE22"
	$"00B5 00EF 0100 23FF FF23 01FF 2200 B100"
	$"EF05 0022 00AA 00EE 0000 2200 AE00 F1FF"
	$"0522 00C7 00D1 00FE 2300 0023 01FE 23FF"
	$"FD23 FD01 2200 C300 CD01 FE22 00C3 00CD"
	$"FE00 2200 C100 CB00 0222 00C1 00CB FDFD"
	$"23FF 0023 FFFF 23FF 0023 00FF 2300 0023"
	$"FDFF 23FF FE23 0000 2200 C100 CE00 0023"
	$"03FF 2200 BA00 C001 FF22 00C1 00CE 0000"
	$"23FE 0023 0001 2200 C300 CB01 FF22 00C3"
	$"00CB FE00 2301 FF22 005C 0055 01FF 2300"
	$"FF23 FFFF 2300 FE23 FEFE 2300 FF23 FFFF"
	$"2300 FE23 FDFD 23FE FF23 FFFE 23FE 0223"
	$"FDFF 23FD FE23 FFFE 23FE 0023 00FE 23FE"
	$"FE23 01FD 2300 FE23 FF00 2301 FE23 03FE"
	$"2300 FF23 FDFE 23FF 0023 FFFC 2300 0023"
	$"FDFF 2300 FE23 01FF 23FF 0023 00FF 2300"
	$"FE23 FF00 2200 D000 DCFC FF23 FC00 23FD"
	$"0323 0000 23FD 0023 FDFD 23FD 0122 00B5"
	$"00C0 02FD 2303 FF22 00B4 00C6 FFFD 2200"
	$"B400 C601 0222 00B1 00C5 00FE 2302 0223"
	$"03FE 2301 0023 0101 2301 0123 0100 2301"
	$"0123 01FF 2301 0023 0200 2301 0023 0100"
	$"2302 0023 0101 2301 0123 0200 2301 0123"
	$"00FF 2301 0123 0101 2300 0023 01FF 2300"
	$"0123 0000 2302 0023 0002 2301 0123 0000"
	$"2300 0023 0001 2300 FF23 0101 2300 0023"
	$"FF02 2300 0123 0300 2200 B100 C500 0022"
	$"00B8 00E2 0200 2301 0023 0000 2302 0123"
	$"0100 2301 0123 0100 2301 0123 0001 2301"
	$"0123 0201 2301 0123 0101 2301 0023 0103"
	$"2301 0023 0100 2300 0123 0102 2301 0123"
	$"0100 2300 0023 0100 2302 0123 0301 2303"
	$"0223 01FE 2300 FF23 0000 2303 0223 0101"
	$"2301 0023 0100 2301 FF23 0201 2301 0023"
	$"0200 2301 0123 0302 2302 0023 0100 2301"
	$"0123 0301 2302 0023 0202 23FF 0123 0001"
	$"23FF 0023 FE02 2300 0323 0001 2300 0123"
	$"FF01 2200 8A00 C800 0023 00FF 2303 0023"
	$"0201 2301 0123 0101 2301 0123 0103 2301"
	$"0023 0100 2302 0123 0100 2301 0023 0300"
	$"2301 0123 01FF 2301 FF23 FF01 2200 9000"
	$"D3FF 0022 0091 00DC 00FF 2200 F501 0300"
	$"0023 00FF 2300 0023 FFFF 2300 0023 FFFF"
	$"23FF FF23 00FF 23FF 0023 FEFF 2300 0023"
	$"0000 23FF 0023 FF00 23FF 0023 00FF 2300"
	$"0023 00FF 2300 0023 FF00 23FF FF23 02FF"
	$"2300 FF23 01FE 2300 0023 0000 2300 FE23"
	$"0000 2200 F501 0302 0122 00EA 00FA 0000"
	$"2200 DC01 1700 0123 FD01 2300 FF23 FE02"
	$"23FF 0123 FE02 23FF 0023 0000 23FF 0023"
	$"FF01 23FC 0023 0000 23FF FF23 FF00 23FF"
	$"FF23 FE00 23FF 0023 FF00 23FF 0023 FFFF"
	$"23FE 0123 0000 2300 0123 0000 23FF 0123"
	$"0001 2300 0023 FF01 2300 0122 00F6 0106"
	$"02FF 2300 FE23 0100 2200 F201 0AFF 0122"
	$"00F2 010A 01FE 2302 FF22 00EF 010E FF00"
	$"2200 EF01 0E01 0023 0100 2200 EF01 11FF"
	$"0022 00EF 0111 01FF 2301 FE22 00EF 0111"
	$"0000 2200 EC01 1300 0022 00CA 010E FD01"
	$"2300 0023 0002 23FF 0023 0101 2304 0023"
	$"0200 23FF 0122 00EC 0113 FE01 2300 FF23"
	$"FF00 23FE 0123 FF00 23FF 0023 FEFE 23FE"
	$"0123 0000 23FE 0023 0000 23FD 0023 FF00"
	$"2300 FF23 FF00 2300 FE23 FF00 2300 FF23"
	$"00FF 23FD FE23 FFFF 23FF FF23 FF00 23FF"
	$"FF23 FCFF 23FF 0023 00FF 23FE FE23 FF00"
	$"2300 FF22 00DE 00EC 05FF 2200 DE00 ECFF"
	$"0123 FFFE 2300 0023 FF01 23FF 0123 FF00"
	$"23FF 0023 FF00 2300 FF23 FE00 23FF 0123"
	$"0002 23FE 0023 0002 23FF 0223 0001 2303"
	$"0222 00ED 0111 03FF 2200 C300 D100 FD22"
	$"00C3 00D1 0101 23FF 0123 0100 23FF 0123"
	$"0001 2302 0023 0100 2300 0123 0101 2300"
	$"0122 00C9 00D7 0202 2300 FE23 0302 2301"
	$"0023 0203 2301 0023 0102 2301 0023 0101"
	$"2302 0123 02FF 2300 FD23 0101 2300 FF23"
	$"0101 2301 0023 0000 2301 0122 00C9 00D5"
	$"0200 2200 B801 0702 FF22 00B6 010A FF01"
	$"2200 B601 0A01 0123 0000 2303 0123 01FE"
	$"2200 B801 0D02 FE22 00B8 010C 0100 2200"
	$"B801 0CFF 0423 0101 2300 0123 FF01 2300"
	$"0123 0000 2200 B501 10FF 0123 FFFF 2200"
	$"B801 0C00 FE22 00C0 010B 0000 2200 B501"
	$"1002 FE22 00B5 010E 0000 23FF 0122 00AF"
	$"00B2 FF00 2200 AF00 B001 0022 00AF 00B0"
	$"FF00 2200 E700 DA04 0222 00E7 00DA FFFE"
	$"2300 0022 00E4 00D8 0100 2300 FF23 0000"
	$"2302 0123 0402 2300 0023 0102 2300 0022"
	$"00E5 00D9 FFFF 2200 E800 E0FF FE22 00A6"
	$"009F FD00 23FE 0023 01FE 23FE FF23 FFFF"
	$"23FF 0123 FF01 2300 FF23 FE01 23FF 0123"
	$"FD01 23FF FF23 FFFE 23FF FE23 00FF 23FD"
	$"FE23 FFFE 2301 FF23 FEFF 23FF FE23 0000"
	$"2300 0123 0001 2200 EC01 1400 0023 0100"
	$"2301 FF23 01FF 2302 0023 01FF 2300 0023"
	$"01FF 2301 0122 00E8 011D FF01 2200 E801"
	$"1DFF 0123 0001 2300 0023 0001 2300 0023"
	$"0000 2301 0023 0000 2301 0023 00FF 2302"
	$"0123 0000 2301 0023 0000 23FF 0123 FF01"
	$"23FF FF23 FF01 23FE 0022 00EA 011E 0201"
	$"2200 CE00 E001 0023 0100 2302 0023 01FF"
	$"2300 0123 0300 2303 0423 0002 2301 0223"
	$"0100 2301 0023 0000 2300 0023 0100 2200"
	$"EB01 2000 0023 00FE 2300 FF23 0100 2302"
	$"FF23 00FF 2301 FF23 01FF 2301 0023 0101"
	$"2302 0023 0000 2200 EF01 1101 0023 0000"
	$"2301 0023 0100 2300 0022 00EA 00FA 0001"
	$"2301 0023 0201 2300 0023 0000 2300 0023"
	$"0101 2301 FF23 0000 2301 0123 0000 2301"
	$"0123 0101 2300 0023 0200 2301 0023 00FF"
	$"2300 FE23 FE00 2200 CE01 02FE 0022 00CE"
	$"0102 0000 2301 0023 0101 2301 0123 0302"
	$"2301 0222 00CF 0105 02FF 2302 FF23 0200"
	$"2300 0123 0101 2301 0123 0102 2301 0023"
	$"02FE 2300 FE22 00CF 0105 0001 2200 CF01"
	$"1100 FF09 0000 0000 0000 0000 3100 1800"
	$"B600 2201 0A39 A000 8DA1 0096 000C 0100"
	$"0000 0200 0000 0000 0000 A100 9A00 08FF"
	$"FB00 0000 2E00 0001 000A 0013 00B7 0023"
	$"0117 0D00 0C2E 0004 0000 0100 2BB8 1F08"
	$"4265 6471 7569 6C74 A000 97A1 0096 000C"
	$"0100 0000 0200 0000 0000 0000 A100 9A00"
	$"0800 0200 0000 3E00 0001 000A 00E7 0074"
	$"0101 00F2 2C00 0900 0306 4765 6E65 7661"
	$"0300 030D 000A 2800 F100 750F 5365 6520"
	$"7468 6520 5665 6374 6F72 732A 0D0B 6279"
	$"204D 656C 2050 6172 6BA0 0097 A000 8DA0"
	$"0083 FF"
};

/*------------------- Icons -------------------*/

resource 'ICN#' (128) {
	{	/* array: 2 elements */
		/* [1] */
		$"007F FFFE 0080 0002 00FF FFFA 0080 000A"
		$"0080 000A 0080 000A 00FF FE0A 01FF FF0A"
		$"0388 058A 066C 02CA 0C1B 014A 0C28 816A"
		$"0C44 816A 0C47 816A 0C40 C16A 0C41 226A"
		$"0C21 146A 0C26 086A 0C18 086A 0610 046A"
		$"0710 04CA 07A0 05CA 0FFF FF8A 1FFF FF0A"
		$"3E80 000A 7C83 8E0A 7884 510A 7084 210A"
		$"0084 510A 0083 8E0A 0080 000C 00FF FFF8",
		/* [2] */
		$"01FF FFFF 01FF FFFF 01FF FFFF 01FF FFFF"
		$"01FF FFFF 01FF FFFF 01FF FFFF 03FF FFFF"
		$"07FF FFFF 0FFF FFFF 1FFF FFFF 1FFF FFFF"
		$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"
		$"1FFF FFFF 1FFF FFFF 1FFF FFFF 1FFF FFFF"
		$"0FFF FFFF 0FFF FFFF 1FFF FFFF 3FFF FFFF"
		$"7FFF FFFF FFFF FFFF FFFF FFFF F9FF FFFF"
		$"71FF FFFF 01FF FFFF 01FF FFFE 00FF FFFC"
	}
};

resource 'ICN#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"0FFF FC00 0800 0600 0800 0500 0800 0480"
		$"0800 0440 0871 C420 088A 2410 0884 27F8"
		$"088A 2008 0871 C008 0800 0008 0800 0008"
		$"081F FC08 0801 C008 0801 C008 0802 A008"
		$"0802 A008 0804 9008 0804 9008 0808 8808"
		$"0808 8808 0810 8408 0810 8408 0820 8208"
		$"0820 8208 0840 8108 0840 8108 0880 8088"
		$"0880 8088 0900 8048 0800 0008 0FFF FFF8",
		/* [2] */
		$"0FFF FE00 0FFF FF00 0FFF FF80 0FFF FFC0"
		$"0FFF FFE0 0FFF FFF0 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
		$"0FFF FFF8 0FFF FFF8 0FFF FFF8 0FFF FFF8"
	}
};

resource 'ICN#' (130) {
	{	/* array: 2 elements */
		/* [1] */
		$"1FFF FC00 1000 0600 1000 0500 1000 0480"
		$"1000 0440 11E7 E420 1000 07F0 11E7 7010"
		$"1000 0010 11CE CB10 1000 0010 11CE CB10"
		$"1000 0010 11CE CD10 1000 0010 11CE CD10"
		$"1000 0010 11CE CD10 1000 0010 11E5 0010"
		$"1000 0010 1000 0010 1000 0010 1038 E010"
		$"1045 1010 1042 1010 1045 1010 1038 E010"
		$"1000 0010 1000 0010 1000 0010 1FFF FFF0",
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

resource 'icl4' (128, purgeable, preload) {
	$"0000 0000 0DDD DDDD DDDD DDDD DDDD DDD0"
	$"0000 0000 D101 0101 0101 0101 0101 01D0"
	$"0000 0000 DDDD DDDD DDDD DDDD DDDD D0D0"
	$"0000 0000 D101 0101 0101 0101 0101 D1D0"
	$"0000 0000 D010 1010 1010 1010 1010 D0D0"
	$"0000 0000 D101 0101 0101 0101 0101 D1D0"
	$"0000 0000 EEEE EEEE EEEE EEE0 1010 D0D0"
	$"0000 000E EEEE EEEE EEEE EEEE 0101 D1D0"
	$"0000 00EE E000 A000 0000 0B0E E010 D0D0"
	$"0000 0EE0 0BB0 AA00 0000 00B0 EE01 D1D0"
	$"0000 EE00 000A A0AA 0000 000B 0E10 D0D0"
	$"0000 EE00 00A0 B000 A000 000B 0EE1 D1D0"
	$"0000 EE00 0A00 0B00 A000 000B 0EE0 D0D0"
	$"0000 EE00 0A00 0BBB A000 000B 0EE1 D1D0"
	$"0000 EE00 0A00 0000 AB00 000B 0EE0 D0D0"
	$"0000 EE00 0A00 000A 00B0 00B0 0EE1 D1D0"
	$"0000 EE00 00A0 000A 000B 0B00 0EE0 D0D0"
	$"0000 EE00 00A0 0AA0 0000 B000 0EE1 D1D0"
	$"0000 EE00 000A A000 0000 B000 0EE0 D0D0"
	$"0000 0EE0 000A 0000 0000 0B00 0EE1 D1D0"
	$"0000 0EEE 000A 0000 0000 0B00 EE10 D0D0"
	$"0000 0EEE E0A0 0000 0000 0B0E EE01 D1D0"
	$"0000 EEEE EEEE EEEE EEEE EEEE E010 D0D0"
	$"000E EEEE EEEE EEEE EEEE EEEE 0101 D1D0"
	$"00EE EEE0 D010 1010 1010 1010 1010 D0D0"
	$"0EEE EE00 D101 01FF F101 FFF1 0101 D1D0"
	$"0EEE E000 D010 1F10 1F1F 101F 1010 D0D0"
	$"0EEE 0000 D101 0F01 01F1 010F 0101 D1D0"
	$"0000 0000 D010 1F10 1F1F 101F 1010 D0D0"
	$"0000 0000 D101 01FF F101 FFF1 0101 D1D0"
	$"0000 0000 D010 1010 1010 1010 1010 DD00"
	$"0000 0000 DDDD DDDD DDDD DDDD DDDD D0"
};

resource 'icl4' (129) {
	$"0000 FFFF FFFF FFFF FFFF FF00 0000 0000"
	$"0000 FC0C 0C0C 0C0C 0C0C 0FF0 0000 0000"
	$"0000 F0C0 C0C0 C0C0 C0C0 CFCF 0000 0000"
	$"0000 FC0C 0C0C 0C0C 0C0C 0FCC F000 0000"
	$"0000 F0C0 C0C0 C0C0 C0C0 CFCC CF00 0000"
	$"0000 FC0C 0FFF 0C0F FF0C 0FCC CCF0 0000"
	$"0000 F0C0 F0C0 F0F0 C0F0 CFCC CCCF 0000"
	$"0000 FC0C FC0C 0F0C 0CFC 0FFF FFFF F000"
	$"0000 F0C0 F0C0 F0F0 C0F0 C0C0 C0C0 F000"
	$"0000 FC0C 0FFF 0C0F FF0C 0C0C 0C0C F000"
	$"0000 F0C0 C0C0 C0C0 C0C0 C0C0 C0C0 F000"
	$"0000 FC0C 0C0C 0C0C 0C0C 0C0C 0C0C F000"
	$"0000 F0C0 C0CF FFFF FFFF FFC0 C0C0 F000"
	$"0000 FC0C 0C0C 0C0F FF0C 0C0C 0C0C F000"
	$"0000 F0C0 C0C0 C0CF FFC0 C0C0 C0C0 F000"
	$"0000 FC0C 0C0C 0CFC FCFC 0C0C 0C0C F000"
	$"0000 F0C0 C0C0 C0F0 F0F0 C0C0 C0C0 F000"
	$"0000 FC0C 0C0C 0F0C FC0F 0C0C 0C0C F000"
	$"0000 F0C0 C0C0 CFC0 F0CF C0C0 C0C0 F000"
	$"0000 FC0C 0C0C FC0C FC0C FC0C 0C0C F000"
	$"0000 F0C0 C0C0 F0C0 F0C0 F0C0 C0C0 F000"
	$"0000 FC0C 0C0F 0C0C FC0C 0F0C 0C0C F000"
	$"0000 F0C0 C0CF C0C0 F0C0 CFC0 C0C0 F000"
	$"0000 FC0C 0CFC 0C0C FC0C 0CFC 0C0C F000"
	$"0000 F0C0 C0F0 C0C0 F0C0 C0F0 C0C0 F000"
	$"0000 FC0C 0F0C 0C0C FC0C 0C0F 0C0C F000"
	$"0000 F0C0 CFC0 C0C0 F0C0 C0CF C0C0 F000"
	$"0000 FC0C FC0C 0C0C FC0C 0C0C FC0C F000"
	$"0000 F0C0 F0C0 C0C0 F0C0 C0C0 F0C0 F000"
	$"0000 FC0F 0C0C 0C0C FC0C 0C0C 0F0C F000"
	$"0000 F0C0 C0C0 C0C0 C0C0 C0C0 C0C0 F000"
	$"0000 FFFF FFFF FFFF FFFF FFFF FFFF F0"
};

resource 'icl4' (130) {
	$"000F FFFF FFFF FFFF FFFF FF00 0000 0000"
	$"000F C0C0 C0C0 C0C0 C0C0 CFF0 0000 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0FCF 0000 0000"
	$"000F C0C0 C0C0 C0C0 C0C0 CFCC F000 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0FCC CF00 0000"
	$"000F C0CF FFF0 CFFF FFF0 CFCC CCF0 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0FFF FFFF 0000"
	$"000F C0CF FFF0 CFFF CFFF C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFC0 FFF0 FFC0 F0FF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFC0 FFF0 FFC0 F0FF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFC0 FFF0 FFC0 FFCF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFC0 FFF0 FFC0 FFCF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFC0 FFF0 FFC0 FFCF C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0CF FFF0 CFCF C0C0 C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0C0 C0C0 C0C0 C0C0 C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0C0 C0FF F0C0 FFF0 C0C0 C0CF 0000"
	$"000F 0C0C 0F0C 0F0F 0C0F 0C0C 0C0F 0000"
	$"000F C0C0 CFC0 C0F0 C0CF C0C0 C0CF 0000"
	$"000F 0C0C 0F0C 0F0F 0C0F 0C0C 0C0F 0000"
	$"000F C0C0 C0FF F0C0 FFF0 C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F C0C0 C0C0 C0C0 C0C0 C0C0 C0CF 0000"
	$"000F 0C0C 0C0C 0C0C 0C0C 0C0C 0C0F 0000"
	$"000F FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'ics#' (128, purgeable, preload) {
	{	/* array: 2 elements */
		/* [1] */
		$"0000 0000 0000 1FFC 2002 4E39 5145 5085"
		$"5145 4E39 6001 3FFF 3FFE 7000 E000 C0",
		/* [2] */
		$"0000 0000 0000 1FFC 3FFE 7FFF 7FFF 7FFF"
		$"7FFF 7FFF 7FFF 7FFF FFFE F000 E000 C0"
	}
};

resource 'ics#' (129) {
	{	/* array: 2 elements */
		/* [1] */
		$"7FF0 4038 402C 4DBC 4A84 4D84 4004 4FE4"
		$"4384 4384 4544 4544 4924 4924 4004 7FFC",
		/* [2] */
		$"7FF0 7FF8 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
		$"7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
	}
};

resource 'ics#' (130) {
	{	/* array: 2 elements */
		/* [1] */
		$"7FF0 4038 402C 403C 4E84 4004 4EE4 4004"
		$"4EE4 4004 4004 46C4 4544 46C4 4004 7FFC",
		/* [2] */
		$"7FF0 7FF8 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
		$"7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC 7FFC"
	}
};

resource 'ics4' (128, purgeable, preload) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 000E EEEE EEEE EE00"
	$"00E0 0000 0000 00E0 0E00 3330 0033 300E"
	$"0E03 0003 0300 030E 0E03 0000 3000 030E"
	$"0E03 0003 0300 030E 0E00 3330 0033 300E"
	$"0EE0 0000 0000 000E 00EE EEEE EEEE EEEE"
	$"00EE EEEE EEEE EEE0 0EEE 0000 0000 0000"
	$"EEE0 0000 0000 0000 EE"
};

resource 'ics4' (129) {
	$"0FFF FFFF FFFF 0000 0FC0 C0C0 C0FF F000"
	$"0F0C 0C0C 0CFC FF00 0FC0 FFCF F0FF FF00"
	$"0F0C FCFC FC0C 0F00 0FC0 FFCF F0C0 CF00"
	$"0F0C 0C0C 0C0C 0F00 0FC0 FFFF FFF0 CF00"
	$"0F0C 0CFF FC0C 0F00 0FC0 C0FF F0C0 CF00"
	$"0F0C 0F0F 0F0C 0F00 0FC0 CFCF CFC0 CF00"
	$"0F0C FC0F 0CFC 0F00 0FC0 F0CF C0F0 CF00"
	$"0F0C 0C0C 0C0C 0F00 0FFF FFFF FFFF FF"
};

resource 'ics4' (130) {
	$"0FFF FFFF FFFF 0000 0FC0 C0C0 C0FF F000"
	$"0F0C 0C0C 0CFD FF00 0FC0 C0C0 C0FF FF00"
	$"0F0C FFFC FC0C 0F00 0FC0 C0C0 C0C0 CF00"
	$"0F0C FFFC FFFC 0F00 0FC0 C0C0 C0C0 CF00"
	$"0F0C FFFC FFFC 0F00 0FC0 C0C0 C0C0 CF00"
	$"0F0C 0C0C 0C0C 0F00 0FC0 CFF0 FFC0 CF00"
	$"0F0C 0F0F 0F0C 0F00 0FC0 CFF0 FFC0 CF00"
	$"0F0C 0C0C 0C0C 0F00 0FFF FFFF FFFF FF"
};

resource 'ics8' (128, purgeable, preload) {
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 0000 0000 0000 0000 0000 0000 0000"
	$"0000 00FA FAFA FAFA FAFA FAFA FAFA 0000"
	$"0000 FA00 0000 0000 0000 0000 0000 FA00"
	$"00FA 0000 1D1D 1D00 0000 1D1D 1D00 00FA"
	$"00FA 001D 0000 001D 001D 0000 001D 00FA"
	$"00FA 001D 0000 0000 1D00 0000 001D 00FA"
	$"00FA 001D 0000 001D 001D 0000 001D 00FA"
	$"00FA 0000 1D1D 1D00 0000 1D1D 1D00 00FA"
	$"00FA FA00 0000 0000 0000 0000 0000 00FA"
	$"0000 FAFA FAFA FAFA FAFA FAFA FAFA FAFA"
	$"0000 FAFA FAFA FAFA FAFA FAFA FAFA FA00"
	$"00FA FAFA 0000 0000 0000 0000 0000 0000"
	$"FAFA FA00 0000 0000 0000 0000 0000 0000"
	$"FAFA"
};

resource 'ics8' (129) {
	$"00FF FFFF FFFF FFFF FFFF FFFF 0000 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 FFFF FF00 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 FF2B FFFF 0000"
	$"00FF F5F5 FFFF F5FF FFF5 FFFF FFFF 0000"
	$"00FF F5F5 FFF5 FFF5 FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 FFFF F5FF FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF F5F5 FFFF FFFF FFFF FFF5 F5FF 0000"
	$"00FF F5F5 F5F5 FFFF FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 FFFF FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 F5FF F5FF F5FF F5F5 F5FF 0000"
	$"00FF F5F5 F5FF F5FF F5FF F5F5 F5FF 0000"
	$"00FF F5F5 FFF5 F5FF F5F5 FFF5 F5FF 0000"
	$"00FF F5F5 FFF5 F5FF F5F5 FFF5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'ics8' (130) {
	$"00FF FFFF FFFF FFFF FFFF FFFF 0000 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 FFFF FF00 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 FF2B FFFF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 FFFF FFFF 0000"
	$"00FF F5F5 FFFF FFF5 FFF5 F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF F5F5 FFFF FFF5 FFFF FFF5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF F5F5 FFFF FFF5 FFFF FFF5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF F5F5 F5FF FFF5 FFFF F5F5 F5FF 0000"
	$"00FF F5F5 F5FF F5FF F5FF F5F5 F5FF 0000"
	$"00FF F5F5 F5FF FFF5 FFFF F5F5 F5FF 0000"
	$"00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000"
	$"00FF FFFF FFFF FFFF FFFF FFFF FFFF"
};

resource 'icl8' (128, purgeable, preload) {
	$"0000 0000 0000 0000 00FA FAFA FAFA FAFA"
	$"FAFA FAFA FAFA FAFA FAFA FAFA FAFA FA00"
	$"0000 0000 0000 0000 FA01 0101 0101 0101"
	$"0101 0101 0101 0101 0101 0101 0101 FA00"
	$"0000 0000 0000 0000 FAFA FAFA FAFA FAFA"
	$"FAFA FAFA FAFA FAFA FAFA FAFA FA01 FA00"
	$"0000 0000 0000 0000 FA02 0202 0202 0202"
	$"0202 0202 0202 0202 0202 0202 FA01 FA00"
	$"0000 0000 0000 0000 FA02 0202 0202 0202"
	$"0202 0202 0202 0202 0202 0202 FA01 FA00"
	$"0000 0000 0000 0000 FA02 0202 0202 0202"
	$"0202 0202 0202 0202 0202 0202 FA01 FA00"
	$"0000 0000 0000 0000 FCFC FCFC FCFC FCFC"
	$"FCFC FCFC FCFC FC02 0202 0202 FA01 FA00"
	$"0000 0000 0000 00FC FCFC FCFC FCFC FCFC"
	$"FCFC FCFC FCFC FCFC 0202 0202 FA01 FA00"
	$"0000 0000 0000 FCFC FC01 0101 8901 0101"
	$"0101 0101 015E 01FC FC02 0202 FA01 FA00"
	$"0000 0000 00FC FC01 015E 5E01 8989 0101"
	$"0101 0101 0101 5E01 FCFC 0202 FA01 FA00"
	$"0000 0000 FCFC 0101 0101 0189 8901 8989"
	$"0101 0101 0101 015E 01FC 0202 FA01 FA00"
	$"0000 0000 FCFC 0101 0101 8901 5E01 0101"
	$"8901 0101 0101 015E 01FC FC02 FA01 FA00"
	$"0000 0000 FCFC 0101 0189 0101 015E 0101"
	$"8901 0101 0101 015E 01FC FC02 FA01 FA00"
	$"0000 0000 FCFC 0101 0189 0101 015E 5E5E"
	$"8901 0101 0101 015E 01FC FC02 FA01 FA00"
	$"0000 0000 FCFC 0101 0189 0101 0101 0101"
	$"895E 0101 0101 015E 01FC FC02 FA01 FA00"
	$"0000 0000 FCFC 0101 0189 0101 0101 0189"
	$"0101 5E01 0101 5E01 01FC FC02 FA01 FA00"
	$"0000 0000 FCFC 0101 0101 8901 0101 0189"
	$"0101 015E 015E 0101 01FC FC02 FA01 FA00"
	$"0000 0000 FCFC 0101 0101 8901 0189 8901"
	$"0101 0101 5E01 0101 01FC FC02 FA01 FA00"
	$"0000 0000 FCFC 0101 0101 0189 8901 0101"
	$"0101 0101 5E01 0101 01FC FC02 FA01 FA00"
	$"0000 0000 00FC FC01 0101 0189 0101 0101"
	$"0101 0101 015E 0101 01FC FC02 FA01 FA00"
	$"0000 0000 00FC FCFC 0101 0189 0101 0101"
	$"0101 0101 015E 0101 FCFC 0202 FA01 FA00"
	$"0000 0000 00FC FCFC FC01 8901 0101 0101"
	$"0101 0101 015E 01FC FCFC 0202 FA01 FA00"
	$"0000 0000 FCFC FCFC FCFC FCFC FCFC FCFC"
	$"FCFC FCFC FCFC FCFC FC02 0202 FA01 FA00"
	$"0000 00FC FCFC FCFC FCFC FCFC FCFC FCFC"
	$"FCFC FCFC FCFC FCFC 0202 0202 FA01 FA00"
	$"0000 FCFC FCFC FC00 FA02 0202 0202 0202"
	$"0202 0202 0202 0202 0202 0202 FA01 FA00"
	$"00FC FCFC FCFC 0000 FA02 0202 0202 FFFF"
	$"FF02 0202 FFFF FF02 0202 0202 FA01 FA00"
	$"00FC FCFC FC00 0000 FA02 0202 02FF 0202"
	$"02FF 02FF 0202 02FF 0202 0202 FA01 FA00"
	$"00FC FCFC 0000 0000 FA02 0202 02FF 0202"
	$"0202 FF02 0202 02FF 0202 0202 FA01 FA00"
	$"0000 0000 0000 0000 FA02 0202 02FF 0202"
	$"02FF 02FF 0202 02FF 0202 0202 FA01 FA00"
	$"0000 0000 0000 0000 FA02 0202 0202 FFFF"
	$"FF02 0202 FFFF FF02 0202 0202 FA01 FA00"
	$"0000 0000 0000 0000 FA02 0202 0202 0202"
	$"0202 0202 0202 0202 0202 0202 FAFA 0000"
	$"0000 0000 0000 0000 FAFA FAFA FAFA FAFA"
	$"FAFA FAFA FAFA FAFA FAFA FAFA FA"
};

resource 'icl8' (129) {
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF 0000 0000 0000 0000 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF FF00 0000 0000 0000 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF 2BFF 0000 0000 0000 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF 2B2B FF00 0000 0000 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF 2B2B 2BFF 0000 0000 0000"
	$"0000 0000 FFF5 F5F5 F5FF FFFF F5F5 F5FF"
	$"FFFF F5F5 F5FF 2B2B 2B2B FF00 0000 0000"
	$"0000 0000 FFF5 F5F5 FFF5 F5F5 FFF5 FFF5"
	$"F5F5 FFF5 F5FF 2B2B 2B2B 2BFF 0000 0000"
	$"0000 0000 FFF5 F5F5 FFF5 F5F5 F5FF F5F5"
	$"F5F5 FFF5 F5FF FFFF FFFF FFFF FF00 0000"
	$"0000 0000 FFF5 F5F5 FFF5 F5F5 FFF5 FFF5"
	$"F5F5 FFF5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5FF FFFF F5F5 F5FF"
	$"FFFF F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5FF FFFF FFFF"
	$"FFFF FFFF FFFF F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5FF"
	$"FFFF F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5FF"
	$"FFFF F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 FFF5"
	$"FFF5 FFF5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 FFF5"
	$"FFF5 FFF5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5FF F5F5"
	$"FFF5 F5FF F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5FF F5F5"
	$"FFF5 F5FF F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 FFF5 F5F5"
	$"FFF5 F5F5 FFF5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 FFF5 F5F5"
	$"FFF5 F5F5 FFF5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5FF F5F5 F5F5"
	$"FFF5 F5F5 F5FF F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5FF F5F5 F5F5"
	$"FFF5 F5F5 F5FF F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 FFF5 F5F5 F5F5"
	$"FFF5 F5F5 F5F5 FFF5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 FFF5 F5F5 F5F5"
	$"FFF5 F5F5 F5F5 FFF5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5FF F5F5 F5F5 F5F5"
	$"FFF5 F5F5 F5F5 F5FF F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5FF F5F5 F5F5 F5F5"
	$"FFF5 F5F5 F5F5 F5FF F5F5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 FFF5 F5F5 F5F5 F5F5"
	$"FFF5 F5F5 F5F5 F5F5 FFF5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 FFF5 F5F5 F5F5 F5F5"
	$"FFF5 F5F5 F5F5 F5F5 FFF5 F5F5 FF00 0000"
	$"0000 0000 FFF5 F5FF F5F5 F5F5 F5F5 F5F5"
	$"FFF5 F5F5 F5F5 F5F5 F5FF F5F5 FF00 0000"
	$"0000 0000 FFF5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5F5 FF00 0000"
	$"0000 0000 FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF FF"
};

resource 'icl8' (130) {
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
	$"0000 00FF F5F5 F5FF FFFF FFF5 F5FF FFFF"
	$"FFFF FFF5 F5FF 2B2B 2B2B FF00 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5FF FFFF FFFF FFFF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFF5 F5FF FFFF"
	$"F5FF FFFF F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF F5F5 FFFF FFF5"
	$"FFFF F5F5 FFF5 FFFF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF F5F5 FFFF FFF5"
	$"FFFF F5F5 FFF5 FFFF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF F5F5 FFFF FFF5"
	$"FFFF F5F5 FFFF F5FF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF F5F5 FFFF FFF5"
	$"FFFF F5F5 FFFF F5FF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF F5F5 FFFF FFF5"
	$"FFFF F5F5 FFFF F5FF F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5FF FFFF FFF5 F5FF F5FF"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 FFFF FFF5 F5F5"
	$"FFFF FFF5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5FF F5F5 F5FF F5FF"
	$"F5F5 F5FF F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5FF F5F5 F5F5 FFF5"
	$"F5F5 F5FF F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5FF F5F5 F5FF F5FF"
	$"F5F5 F5FF F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 FFFF FFF5 F5F5"
	$"FFFF FFF5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF F5F5 F5F5 F5F5 F5F5 F5F5 F5F5"
	$"F5F5 F5F5 F5F5 F5F5 F5F5 F5FF 0000 0000"
	$"0000 00FF FFFF FFFF FFFF FFFF FFFF FFFF"
	$"FFFF FFFF FFFF FFFF FFFF FFFF"
};

/*------------------- Windows -------------------*/

resource 'WIND' (1129, "Floater") {
	{66, 18, 264, 163},
	documentProc,
	invisible,
	goAway,
	0x0,
	"Floating"
};

resource 'WIND' (1128, "Floating Window2") {
	{66, 18, 166, 90},
	3200,
	invisible,
	goAway,
	0x0,
	""
};

resource 'WIND' (132, "Floating Window2") {
	{256, 16, 324, 420},
	3200,
	invisible,
	goAway,
	0x0,
	""
};

resource 'WIND' (131, "Floating Window") {
	{46, 4, 250, 68},
	3200,
	invisible,
	goAway,
	0x0,
	""
};

data 'WDEF' (200, "Windoid") {
	$"600E 0000 5744 4546 00C8 0000 0000 0000"            /* `...WDEF.�...... */
	$"41FA FFEE 4E71 4E71 6000 01BC 4EFA 001E"            /* A...NqNq`..�N... */
	$"4EFA 002E 4EFA 003E 4EFA 0056 4EFA 0088"            /* N...N..>N..VN..� */
	$"4EFA 00A4 4EFA 00C2 4EFA 00DE 205F 3218"            /* N..�N..�N... _2. */
	$"3418 B058 57C9 FFFA 4A42 67FE 4EF0 20FC"            /* 4.�XW�..JBg.N. . */
	$"205F 3218 3418 B098 57C9 FFFA 4A42 67FE"            /*  _2.4.��W�..JBg. */
	$"4EF0 20FA 205F 3218 3418 B042 6E0A 9041"            /* N. . _2.4.�Bn.�A */
	$"6D06 D040 41F0 0002 3010 67FE 4EF0 0000"            /* m.�@A...0.g.N... */
	$"202F 0004 2F41 0004 222F 0008 2F5F 0004"            /*  /../A.."/../_.. */
	$"48E7 3C00 2400 2601 4842 C4C3 2800 2A01"            /* H.<.$.&.HB��(.*. */
	$"4845 C8C5 D444 4842 4242 C0C1 D082 4CDF"            /* HE���DHBBB��ЂL. */
	$"003C 221F 4E75 202F 0004 2F41 0004 222F"            /* .<".Nu /../A.."/ */
	$"0008 2F5F 0004 48E7 3100 4EBA 009C 4CDF"            /* ../_..H.1.N�.�L. */
	$"008C 221F 4E75 202F 0004 2F41 0004 222F"            /* .�".Nu /../A.."/ */
	$"0008 2F5F 0004 48E7 3100 4EBA 007C 2001"            /* ../_..H.1.N�.| . */
	$"4CDF 008C 221F 4E75 202F 0004 2F41 0004"            /* L..�".Nu /../A.. */
	$"222F 0008 2F5F 0004 48E7 3100 4EBA 002C"            /* "/../_..H.1.N�., */
	$"4CDF 008C 221F 4E75 202F 0004 2F41 0004"            /* L..�".Nu /../A.. */
	$"222F 0008 2F5F 0004 48E7 3100 4EBA 000C"            /* "/../_..H.1.N�.. */
	$"2001 4CDF 008C 221F 4E75 4A80 6A1C 4A81"            /*  .L..�".NuJ�j.J� */
	$"6A0C 4480 4481 4EBA 0020 4481 4E75 4480"            /* j.D�D�N�. D�NuD� */
	$"4EBA 0016 4480 4481 4E75 4A81 6A0A 4481"            /* N�..D�D�NuJ�j.D� */
	$"4EBA 0006 4480 4E75 2E3C 0000 FFFF B280"            /* N�..D�Nu.<....�� */
	$"6306 2200 7000 4E75 B087 620C 80C1 4840"            /* c.".p.Nu��b.��H@ */
	$"3200 4240 4840 4E75 B287 621A 2E00 4240"            /* 2.B@H@Nu��b...B@ */
	$"4840 80C1 4840 4847 3E00 4847 8EC1 3007"            /* H@��H@HG>.HG��0. */
	$"4847 3207 4E75 2400 2601 E288 E289 B287"            /* HG2.Nu$.&..�.��� */
	$"62F8 80C1 C087 3203 C2C0 2E03 4847 CEC0"            /* b.����2.��..HG�� */
	$"4847 D287 6508 9282 6204 4481 4E75 5340"            /* HG҇e.��b.D�NuS@ */
	$"60E4 0050 0001 4E56 FFFC 42AE FFFC 302E"            /* `..P..NV..B�..0. */
	$"000C 4EBA FE40 0000 0002 0072 0006 0044"            /* ..N�.@.....r...D */
	$"005E 206E 000E 4A28 006E 6700 0060 302E"            /* .^ n..J(.ng..`0. */
	$"000A 4EBA FE18 0002 000A 0000 0016 0004"            /* ..N�............ */
	$"004C 2F2E 000E 3F2E 0012 4EBA 0058 5C8F"            /* .L/...?...N�.X\� */
	$"603A 2F2E 000E 3F2E 0012 4EBA 01F2 5C8F"            /* `:/...?...N�..\� */
	$"602A 2F2E 0008 2F2E 000E 3F2E 0012 4EBA"            /* `*..../...?...N� */
	$"021C 4FEF 000A 48C0 2D40 FFFC 600E 2F2E"            /* ..O...H�-@..`./. */
	$"000E 3F2E 0012 4EBA 02C6 5C8F 202E FFFC"            /* ..?...N�.�\� ... */
	$"4E5E 205F 4FEF 000C 2E80 4ED0 4D41 494E"            /* N^ _O....�N�MAIN */
	$"2020 2020 4E56 FFCA 48E7 0110 3E2E 0008"            /*     NV.�H...>... */
	$"486E FFEE A898 206E 000A 2068 0076 2050"            /* Hn..�� n.. h.v P */
	$"43EE FFE6 5488 22D8 22D8 486E FFE6 3F3C"            /* C...T�"�"�Hn..?< */
	$"FFFF 3F3C FFFF A8A9 3F3C 0001 3F3C 0001"            /* ..?<..��?<..?<.. */
	$"A89B 486E FFE6 A8A1 3F3C 0001 3F3C 0001"            /* ��Hn..��?<..?<.. */
	$"A89B 4A47 662A 302E FFE8 5640 3F00 3F2E"            /* ��JGf*0...V@?.?. */
	$"FFEA A893 3F2E FFEC 3F2E FFEA A891 3F2E"            /* ..��?...?...��?. */
	$"FFEC 302E FFE6 0640 FFF7 3F00 A891 602E"            /* ..0....@..?.��`. */
	$"0C47 0002 6628 302E FFE8 0640 FFF7 3F00"            /* .G..f(0....@..?. */
	$"3F2E FFEA A893 3F2E FFEC 3F2E FFEA A891"            /* ?...��?...?...�� */
	$"3F2E FFEC 302E FFE6 5240 3F00 A891 486E"            /* ?...0...R@?.��Hn */
	$"FFCE 487A 03EA A866 082E 0000 FFE9 6604"            /* .�Hz..�f......f. */
	$"4A47 670E 082E 0000 FFE9 6720 0C47 0002"            /* JGg.......g .G.. */
	$"661A 41EE FFCE 2648 1013 4880 4640 1680"            /* f.A..�&H..H�F@.� */
	$"1753 0002 1753 0004 1753 0006 082E 0000"            /* .S...S...S...... */
	$"FFE7 6704 4A47 670E 082E 0000 FFE7 6628"            /* ..g.JGg.......f( */
	$"0C47 0002 6622 41EE FFCE 2D48 FFCA 206E"            /* .G..f"A..�-H.� n */
	$"FFCA 2010 E088 206E FFCA 2080 206E FFCA"            /* .� ..� n.� � n.� */
	$"226E FFCA 2350 0004 486E FFDE 2F2E 000A"            /* "n.�#P..Hn../... */
	$"3F07 4EBA 0242 4FEF 000A 3F3C 0001 3F3C"            /* ?.N�.BO...?<..?< */
	$"0001 A89B 486E FFDE 486E FFCE A8A5 486E"            /* ..��Hn..Hn.Ψ�Hn */
	$"FFDE A8A1 206E 000A 4A28 0070 673A 486E"            /* ..�� n..J(.pg:Hn */
	$"FFD6 486E FFDE 3F07 4EBA 0292 4FEF 000A"            /* .�Hn..?.N�.�O... */
	$"486E FFD6 3F3C FFFF 3F3C FFFF A8A9 486E"            /* Hn.�?<..?<..��Hn */
	$"FFD6 A8A3 486E FFD6 3F3C 0001 3F3C 0001"            /* .֨�Hn.�?<..?<.. */
	$"A8A9 486E FFD6 A8A1 486E FFEE A899 4CDF"            /* ��Hn.֨�Hn..��L. */
	$"0880 4E5E 4E75 4452 4157 4652 414D 4E56"            /* .�N^NuDRAWFRAMNV */
	$"FFF0 486E FFF8 2F2E 000A 3F2E 0008 4EBA"            /* ..Hn../...?...N� */
	$"01B6 4FEF 000A 486E FFF0 486E FFF8 3F2E"            /* .�O...Hn..Hn..?. */
	$"0008 4EBA 0228 4FEF 000A 486E FFF0 A8A4"            /* ..N�.(O...Hn..�� */
	$"4E5E 4E75 544F 4747 4C45 434C 4E56 FFEC"            /* N^NuTOGGLECLNV.. */
	$"2F0B 266E 000A 203C 0000 FFFF C0AE 000E"            /* /.&n.. <....��.. */
	$"3D40 FFFE 202E 000E 7210 E2A0 0280 0000"            /* =@.. ...r..�.�.. */
	$"FFFF 3D40 FFFC 4267 2F2E FFFC 2F2B 0072"            /* ..=@..Bg/.../+.r */
	$"A8E8 101F 4A00 6606 7000 6000 0074 4267"            /* �...J.f.p.`..tBg */
	$"2F2E FFFC 2F2B 0076 A8E8 101F 4A00 6706"            /* /.../+.v�...J.g. */
	$"7001 6000 005C 486E FFF4 2F0B 3F2E 0008"            /* p.`..\Hn../.?... */
	$"4EBA 0124 4FEF 000A 4267 2F2E FFFC 486E"            /* N�.$O...Bg/...Hn */
	$"FFF4 A8AD 101F 4A00 6734 486E FFEC 486E"            /* ..��..J.g4Hn..Hn */
	$"FFF4 3F2E 0008 4EBA 0184 4FEF 000A 4A2B"            /* ..?...N�.�O...J+ */
	$"0070 6716 4267 2F2E FFFC 486E FFEC A8AD"            /* .pg.Bg/...Hn..�� */
	$"101F 4A00 6704 7004 6006 7002 6002 7000"            /* ..J.g.p.`.p.`.p. */
	$"265F 4E5E 4E75 4649 4E44 5041 5254 4E56"            /* &_N^NuFINDPARTNV */
	$"FFF0 2D6E 000A FFFC 206E FFFC 43EE FFF4"            /* ..-n.... n..C... */
	$"41E8 0010 22D8 22D8 206E FFFC 3028 000A"            /* A..."�"� n..0(.. */
	$"4440 206E FFFC 3228 0008 4441 486E FFF4"            /* D@ n..2(..DAHn.. */
	$"3F00 3F01 A8A8 206E 000A 2F28 0076 486E"            /* ?.?.�� n../(.vHn */
	$"FFF4 A8DF 486E FFF4 3F3C FFFF 3F3C FFFF"            /* ..�.Hn..?<..?<.. */
	$"A8A9 4A6E 0008 6608 066E FFF6 FFF4 600E"            /* ��Jn..f..n....`. */
	$"0C6E 0002 0008 6606 066E FFF6 FFF6 206E"            /* .n....f..n.... n */
	$"000A 2F28 0072 486E FFF4 A8DF 486E FFF4"            /* ../(.rHn..�.Hn.. */
	$"3F3C 0001 3F3C 0001 A8A8 42A7 A8D8 205F"            /* ?<..?<..��B��� _ */
	$"2D48 FFF0 2F2E FFF0 486E FFF4 A8DF 206E"            /* -H../...Hn..�. n */
	$"000A 2F28 0072 2F2E FFF0 206E 000A 2F28"            /* ../(.r/... n../( */
	$"0072 A8E5 2F2E FFF0 A8D9 4E5E 4E75 4255"            /* .r�./...�.N^NuBU */
	$"494C 4452 4547 4E56 0000 206E 000A 2068"            /* ILDREGNV.. n.. h */
	$"0076 2050 226E 000E 5488 22D8 22D8 2F2E"            /* .v P"n..T�"�"�/. */
	$"000E 3F3C FFFF 3F3C FFFF A8A9 4A6E 0008"            /* ..?<..?<..��Jn.. */
	$"661A 206E 000E 3010 5240 206E 000E 3140"            /* f. n..0.R@ n..1@ */
	$"0004 206E 000E 0650 FFF6 6034 0C6E 0002"            /* .. n...P..`4.n.. */
	$"0008 661E 206E 000E 3028 0002 5240 206E"            /* ..f. n..0(..R@ n */
	$"000E 3140 0006 206E 000E 0668 FFF6 0002"            /* ..1@.. n...h.... */
	$"600E 2F2E 000E 4267 4267 4267 4267 A8A7"            /* `./...BgBgBgBg�� */
	$"4E5E 4E75 4745 5444 5241 4742 4E56 0000"            /* N^NuGETDRAGBNV.. */
	$"48E7 0030 4A6E 0008 6638 206E 000A 226E"            /* H..0Jn..f8 n.."n */
	$"000A 246E 000A 266E 000A 2F2E 000E 3028"            /* ..$n..&n../...0( */
	$"0002 5040 3F00 3011 5440 3F00 302A 0002"            /* ..P@?.0.T@?.0*.. */
	$"0640 000F 3F00 302B 0004 5540 3F00 A8A7"            /* .@..?.0+..U@?.�� */
	$"604C 0C6E 0002 0008 6636 206E 000A 226E"            /* `L.n....f6 n.."n */
	$"000A 246E 000A 266E 000A 2F2E 000E 3028"            /* ..$n..&n../...0( */
	$"0002 5440 3F00 3011 5040 3F00 302A 0006"            /* ..T@?.0.P@?.0*.. */
	$"5540 3F00 3013 0640 000F 3F00 A8A7 600E"            /* U@?.0..@..?.��`. */
	$"2F2E 000A 4267 4267 4267 4267 A8A7 4CDF"            /* /...BgBgBgBg��L. */
	$"0C00 4E5E 4E75 4745 5443 4C4F 5345 1041"            /* ..N^NuGETCLOSE.A */
	$"4130 3041 4130 3041 4130 3041 4130 3000"            /* A00AA00AA00AA00. */
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
