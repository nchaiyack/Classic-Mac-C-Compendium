#ifdef THINK_Rez
	#include <Types.r>
#else
	#include "Types.r"
#endif

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
		"About BoxMaker++�", noIcon, noKey, noMark, plain,
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
		"Select File�", noIcon, "O", noMark, plain,
		"Preferences�", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Quit", noIcon, "Q", noMark, plain
	}
};
