#define SystemSevenOrLater 1

#include <Types.r>
#include <SysTypes.r>

#include "boxmaker templates.h"
#include "boxmaker constants.h"

#define myInfo "Touch 1.3.1"

#include "boxmaker FinderInfo.r"

resource 'flgs' (128) {
	EnterFolders,
	PassFolders,
	dontEnterInvisibles,
	dontPassInvisibles
};

type 'tims' {
	array TypeArray {
		literal longint;	// granularity for one of the settings
	};
};

resource 'tims' (128) {
	{1, 60, 3600, 43200, 86400, -1}
};

resource 'STR ' (128, locked) {
	"Touch preferences"
};

resource 'STR ' (-16397) {
	"This document describes user preferences for the application"
	" ‘Touch’. You cannot open or print this document. To be ef"
	"fective, this document must be stored in the Preferences folder"
	" inside the System Folder."
};

resource 'DLOG' (kSettingsDialogID) {
	{60, 96, 200, 416},
	noGrowDocProc,
	invisible,
	goAway,
	0x00,
	kSettingsDialogID,
	"Touch Preferences",
	alertPositionMainScreen
};

resource 'DITL' (kSettingsDialogID) {
	{
		{32, 8, 50, 114},
		RadioButton {
			enabled,
			"Second"
		},

		{48, 8, 66, 114},
		RadioButton {
			enabled,
			"Minute"
		},

		{64, 8, 82, 114},
		RadioButton {
			enabled,
			"Hour"
		},

		{80, 8, 98, 114},
		RadioButton {
			enabled,
			"Half a day"
		},

		{96, 8, 114, 114},
		RadioButton {
			enabled,
			"Day"
		},

		{112, 8, 130, 114},
		RadioButton {
			enabled,
			"Ground Zero"
		},

		{32, 152, 50, 296},
		RadioButton {
			enabled,
			"Creation date"
		},

		{48, 152, 66, 296},
		RadioButton {
			enabled,
			"Modification date"
		},

		{64, 152, 82, 296},
		RadioButton {
			enabled,
			"Both"
		},

		{8, 8, 24, 128},
		StaticText {
			disabled,
			"Round dates to:"
		},

		{8, 152, 24, 227},
		StaticText {
			disabled,
			"Change:"
		}
	}
};
