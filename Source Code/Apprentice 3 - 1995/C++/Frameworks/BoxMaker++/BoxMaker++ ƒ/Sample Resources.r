#define SystemSevenOrLater 1

#include <Types.r>

#include "boxmaker templates.h"
//
// Include this if you want to limit the file types passed to your dropper:
//
resource 'typs' (128, Locked) {
	{
		'Test',
		'This',
		'That',
		'More',
		'Last'
	}
};

resource 'tycr' (128, Locked) {
	{
		'TEXT', 'ttxt',
		'PICT', 'MPNT'
	}
};
//
// 'EnterFolders', 'PassInvisibles', and 'EnterInvisibles'
// will all be set to false if the 'flgs' resource is absent.
//
// the flag 'passFolders' is set by the constructor. It can
// not be changed by messing around with the 'flgs' resource
// because the 'OpenDoc' handler may get upset about that.
//
resource 'flgs' (128) {
	EnterFolders,
	dontPassFolders,
	dontEnterInvisibles,
	dontPassInvisibles
};
