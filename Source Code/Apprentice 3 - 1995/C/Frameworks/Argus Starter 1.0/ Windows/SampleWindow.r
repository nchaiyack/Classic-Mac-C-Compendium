/**********************************************************************

    Segment: SampleWindow.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>

resource 'WIND' (400, "Main WIND") {
	{45, 5, 223, 125},
	noGrowDocProc,
	invisible,
	goAway,
	0x0,
	"Window"
};

resource 'CNTL' (400, "Dialog") {
	{13, 30, 33, 90},
	0,
	visible,
	0,
	0,
	pushButProc,
	0,
	"Dialog"
};

resource 'CNTL' (401, "List") {
	{46, 30, 66, 90},
	0,
	visible,
	0,
	0,
	pushButProc,
	0,
	"List"
};

resource 'CNTL' (402, "AEvent") {
	{79, 30, 99, 90},
	0,
	visible,
	0,
	0,
	pushButProc,
	0,
	"AEvent"
};

resource 'CNTL' (403, "Error") {
	{112, 30, 132, 90},
	0,
	visible,
	0,
	0,
	pushButProc,
	0,
	"Error"
};

resource 'CNTL' (404, "TBD") {
	{145, 30, 165, 90},
	0,
	visible,
	0,
	0,
	pushButProc,
	0,
	"???"
};

// End of File