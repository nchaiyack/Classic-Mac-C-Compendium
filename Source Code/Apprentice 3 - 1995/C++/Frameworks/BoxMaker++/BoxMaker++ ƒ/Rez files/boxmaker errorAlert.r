#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#ifdef THINK_Rez
	#include <Types.r>
#else
	#include "Types.r"
#endif

#include "boxmaker constants.h"

resource 'ALRT' (kErrorAlertID, purgeable) {	// General Error Alert
	{80, 96, 208, 416},
	kErrorAlertID,
	{
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent,
		OK, visible, silent
	},
	alertPositionMainScreen
};

resource 'DITL' (kErrorAlertID, purgeable) {
	{
		{96, 232, 116, 304},
		Button {
			enabled,
			"OK"
		},

		{8, 48, 86, 307},
		StaticText {
			disabled,
			"^0"
		},

		{8, 8, 40, 40},
		Icon {
			disabled,
			0
		},

		{96, 11, 112, 100},
		StaticText {
			disabled,
			"Error #^1"
		}
	}
};
