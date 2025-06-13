#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#include <Types.r>
#include <SysTypes.r>

#include "boxmaker templates.h"
#include "boxmaker constants.h"

resource 'flgs' (128) {
	dontEnterFolders,
	dontPassFolders,
	dontEnterInvisibles,
	dontPassInvisibles
};

#define myInfo "Typical 1.3.1"

#include "boxmaker FinderInfo.r"

resource 'MBAR' (kMBarID) { {kAppleMenuID, kFileMenuID}};

resource 'MENU' (kAppleMenuID) {
	kAppleMenuID,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{
		"About BoxMaker++…", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (kFileMenuID) {
	kFileMenuID,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"File",
	{
		"Typify File…", noIcon, "O", noMark, plain,
		"-", noIcon, noKey, noMark, plain,
		"Quit", noIcon, "Q", noMark, plain
	}
};
