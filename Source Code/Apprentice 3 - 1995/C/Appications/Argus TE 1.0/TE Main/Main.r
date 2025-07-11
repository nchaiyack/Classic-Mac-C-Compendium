/**********************************************************************

    Segment: Main.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>

resource 'MBAR' (400, "Main MBAR") {
	{	/* array MenuArray: 3 elements */
		/* [1] */
		400,
		/* [2] */
		401,
		/* [3] */
		402
	}
};

resource 'MENU' (400, "Apple") {
	400,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{	/* array: 2 elements */
		/* [1] */
		"About Argus TE...", noIcon, noKey, noMark, plain,
		/* [2] */
		"-", noIcon, noKey, noMark, 1
	}
};

resource 'MENU' (401, "File") {
	401,
	textMenuProc,
	allEnabled,
	enabled,
	"File",
	{	/* array: 11 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"Open...", noIcon, "O", noMark, plain,
		/* [3] */
		"Close", noIcon, "W", noMark, plain,
		/* [4] */
		"-", noIcon, noKey, noMark, plain,
		/* [5] */
		"Save", noIcon, "S", noMark, plain,
		/* [6] */
		"Save As...", noIcon, noKey, noMark, plain,
		/* [7] */
		"-", noIcon, noKey, noMark, plain,
		/* [8] */
		"Page Setup...", noIcon, noKey, noMark, plain,
		/* [9] */
		"Print...", noIcon, "P", noMark, plain,
		/* [10] */
		"-", noIcon, noKey, noMark, plain,
		/* [11] */
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'MENU' (402, "Edit", preload) {
	402,
	textMenuProc,
	0x0,
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
		/* [7] */
		"-", noIcon, noKey, noMark, plain
		/* [8] */
		"Select All", noIcon, "A", noMark, plain
	}
};

// End of File