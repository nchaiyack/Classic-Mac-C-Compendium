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
		"About Application...", noIcon, noKey, noMark, plain,
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
	{	/* array: 3 elements */
		/* [1] */
		"New", noIcon, "N", noMark, plain,
		/* [2] */
		"Close", noIcon, "W", noMark, plain,
		/* [3] */
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
	}
};

// End of File