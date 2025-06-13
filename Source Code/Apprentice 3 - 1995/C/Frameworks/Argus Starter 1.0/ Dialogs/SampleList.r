/**********************************************************************

    Segment: SampleList.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>

resource 'DLOG' (510, "List Dialog") {
	{45, 10, 317, 205},
	movableDBoxProc, /* movableDBoxProc or noGrowDocProc */
	invisible,
	goAway,
	0x0,
	510,
	"List Dialog"
};

resource 'DITL' (510, "List Dialog") {
	{	/* array DITLarray: 1 elements */
        /* [1] */
		{242, 68, 262, 128},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{5, 5, 229, 190},
		UserItem {
			disabled
		}
	}
};

// End of File