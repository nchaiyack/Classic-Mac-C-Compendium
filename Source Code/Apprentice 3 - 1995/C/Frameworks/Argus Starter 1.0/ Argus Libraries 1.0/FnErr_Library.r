/**********************************************************************

    Segment: FnErr_Library.r

***********************************************************************/

/* THINK Rez resource format */
#include <Types.r>

resource 'DITL' (900, "Error Alert") {
	{	/* array DITLarray: 2 elements */
		/* [1] */
		{59, 288, 79, 348},
		Button {
			enabled,
			"OK"
		},
		/* [2] */
		{10, 75, 54, 343},
		StaticText {
			enabled,
			"^0^1^2^3"
		}
	}
};

resource 'ALRT' (900, "Error Alert") {
	{94, 80, 183, 438},
	900,
	{	/* array: 4 elements */
		/* [1] */
		OK, visible, sound1,
		/* [2] */
		OK, visible, sound1,
		/* [3] */
		OK, visible, sound1,
		/* [4] */
		OK, visible, sound1,
	},
};

resource 'STR ' (900, "General Error") {
	"Sorry, but a fatal error just occured!"
};

resource 'STR ' (901, "System Error") {
	"Sorry, you must have System 7 or later to run this application."
};

resource 'STR ' (902, "Resource Error") {
	"Error: Resource could not be found."
};

resource 'STR ' (903, "Can't Handle AppleEvent Error") {
	"Error: Could not handle AppleEvent."
};

// End of File