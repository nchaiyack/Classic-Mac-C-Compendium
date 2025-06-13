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
//
// Finder related resources:
//
#ifndef myType
	#define myType_defined_here
	#define myType '¶Ä**'
#endif

data myType (0) {};

resource 'FREF' (128) { '****', 0, ""};
resource 'FREF' (129) { 'fold', 1, ""};
resource 'FREF' (130) { 'disk', 2, ""};

resource 'BNDL' (0) {
	myType,
	0,
	{
		'FREF',		{ 0, 128,		1, 129,			2, 130},
		'ICN#',		{ 0,   0,		1,   0,			2,   0}
	}
};

resource 'vers' (1) {
	0x01,
	0x31,
	release,
	0x00,
	verUS,
	"1.3.1",
	#ifdef myInfo
		myInfo ",\nbuilt using BoxMaker++ 1.3.1"
	#else
		"Built using BoxMaker++ 1.3.1"
	#endif
};

resource 'vers' (2) {
	0x01,
	0x31,
	release,
	0x00,
	verUS,
	"1.3.1",
	"by Reinder Verlinde"
};

resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	multiFinderAware,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	102400,			// preferred size
	51200			// minimum size
};

#ifdef myType_defined_here
	#undef myType_defined_here
	#undef myType
#endif
