//
// Standard 'TMPL' resources for inclusion in any dropbox
//
#include "boxmaker templates.h"

resource 'TMPL' (128, "flgs") {
	{
		"enterFolders"   , 'DBYT',
		"passFolders"    , 'DBYT',
		"enterInvisibles", 'DBYT',
		"passInvisibles" , 'DBYT'
	}
};

resource 'TMPL' (129, "typs") {
	{
		"****"			, 'LSTB',
		"type to pass"	, 'TNAM',
		"****"			, 'LSTE'
	}
};

resource 'TMPL' (130, "tycr") {
	{
		"****"				, 'LSTB',
		"type to pass"		, 'TNAM',
		"creator to pass"	, 'TNAM',
		"****"				, 'LSTE'
	}
};
