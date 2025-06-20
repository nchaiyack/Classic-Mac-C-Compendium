#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#ifdef THINK_Rez
	#include <Types.r>
#else
	#include "Types.r"
#endif

#include "boxmaker constants.h"
//
// Alerts:
//
resource 'ALRT' (kAboutAlertID, purgeable) {
	{90, 172, 262, 468},
	kAboutAlertID,
	{
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (kAboutAlertID, purgeable) {
	{
		{144, 216, 164, 288},
		Button {
			enabled,
			"OK"
		},

		{8, 8, 128, 288},
		StaticText {
			disabled,
			"This program was made using BoxMaker++ by Reinder Verlinde."
			"\n\nBoxMaker is a C++ class for making DropBoxes. It is based"
			" on DropShell 1.0a by Leonard Rosenthol, Stephan Somogyi"
			" and Marshall Clow."
		},
		{132, 8, 164, 144},
		StaticText {
			disabled,
			"1.3.1, Compiled"
			"\nwith Symantec C++"
		}
	}
};
