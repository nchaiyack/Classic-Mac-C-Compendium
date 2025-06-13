#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#ifdef THINK_Rez
	#include <Types.r>
#else
	#include "Types.r"
#endif

#include "boxmaker templates.h"

resource 'flgs' (128) {
	EnterFolders,
	dontPassFolders,
	dontEnterInvisibles,
	dontPassInvisibles
};

resource 'typs' (128) {
	{
		'MooV'
	};
};

resource 'STR ' (128, Locked) {
	"µZak preferences"
};
//
// STR for in preferences file:
//
resource 'STR ' (-16397) {
	"This document describes user preferences for the application"
	" ‘µZak’. You cannot open or print this document. To be ef"
	"fective, this document must be stored in the Preferences folder"
	" inside the System Folder."
};

type 'cnfg' {
	unsigned longint;
};

resource 'TMPL' (131, "cnfg") {
	{
		"length of song queue", 'DLNG'
	}
};

resource 'cnfg' (128, locked) {
	512
};
