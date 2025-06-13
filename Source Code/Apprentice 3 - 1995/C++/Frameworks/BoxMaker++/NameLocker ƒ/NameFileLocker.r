#define SystemSevenOrLater 1

#ifdef THINK_Rez
	#include <Types.r>
	#include <SysTypes.r>
#else
	#include "Types.r"
	#include "SysTypes.r"
#endif

#include "boxmaker templates.h"
#include "boxmaker constants.h"

#define myInfo "Name/File(Un)Locker 1.3.1"

#include "boxmaker FinderInfo.r"

resource 'flgs' (128) {
	dontEnterFolders,
	PassFolders,
	dontEnterInvisibles,
	dontPassInvisibles
};
//
// The 'what' resource specifies what the locker should do
//
resource 'TMPL' (131, "what") {
	{
		"file action"	, 'DBYT',
		"name action"	, 'DBYT'
	}
};

#ifndef THINK_Rez
	type 'what' {
		byte  ignoreFileLock, lockFile, dontLockFile;
		byte  ignoreNameLock, lockName, dontLockName;
	};
#endif
