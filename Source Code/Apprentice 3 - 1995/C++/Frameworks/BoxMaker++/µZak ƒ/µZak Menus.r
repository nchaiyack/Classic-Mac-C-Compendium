#include <Types.r>

#include "boxmaker constants.h"
//
// Menus and the Menu bar:
//
resource 'MBAR' (kMBarID) {	{ kAppleMenuID, kFileMenuID}};

resource 'MENU' (kAppleMenuID) {
	kAppleMenuID,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{
		"About µZak…", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (kFileMenuID) {
	kFileMenuID,
	textMenuProc,
	0x7FFFFFFB,
	enabled,
	"File",
	{
		"Select Movie file to play…", noIcon, "O", noMark, plain,
		"Preferences…", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Quit", noIcon, "Q", noMark, plain
	}
};
