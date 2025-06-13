/**********************************************************************

    Segment: ArgusHelp.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>

resource 'DLOG' (598, "Help Dialog") {
	{45, 10, 330, 405},
	movableDBoxProc, /* movableDBoxProc or noGrowDocProc */
	invisible,
	goAway,
	0x0,
	598,
	"Application Help"
};

resource 'DITL' (598, "Help Dialog") {
	{	/* array DITLarray: 3 elements */
        /* [1] */
		{254, 322, 274, 382},
		Button {
			enabled,
			"OK"
		},
        /* [2] */
		{254, 249, 274, 309},
		Button {
			enabled,
			"Print..."
		},
        /* [3] */
		{254, 176, 274, 236},
		Button {
			enabled,
			"Save..."
		},
		{0, 0, 0, 0},
		HelpItem {
		  disabled,
		  HMScanhdlg
		  {598}
		}
	}
};

data 'TEXT' (598, "Help Text") {
	$"4170 706C 6963 6174 696F 6E20 4865 6C70" /* Application Help */
	$"2054 6578 742E 2E2E"                     /*  Text... */
};

// End of File