#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#ifdef THINK_Rez
	#include <Types.r>
	#include <SysTypes.r>
#else
	#include "Types.r"
	#include "SysTypes.r"
#endif

#include "boxmaker templates.h"

resource 'flgs' (128) {
	EnterFolders,
	PassFolders,
	dontEnterInvisibles,
	dontPassInvisibles
};

resource 'STR ' (128, Locked) {
	"ll-R preferences"
};

resource 'STR ' (129, Locked) {
	"ll-R out"
};

resource 'STR ' (130, Locked) {
	"Save directory listing as:"
};

resource 'STR ' (131, Locked) {
	"	No read privileges for this folder. Skipping it"
};
//
// STR for in preferences file:
//
resource 'STR ' (-16397) {
	"This document describes user preferences for the application"
	" �ll-R�. You cannot open or print this document. To be ef"
	"fective, this document must be stored in the Preferences folder"
	" inside the System Folder."
};
//
// Finder related resources:
//
#define myInfo "ll-R 1.3.1"
#define myType '��**'

#include "boxmaker FinderInfo.r"
//
// ll-R main dialog (moved to a resource file)
//

//
// ID of folder to create outputfile in:
//
type 'Wher' {
	literal longint;
};

resource 'Wher' (128, Locked) {
	'desk'
};
