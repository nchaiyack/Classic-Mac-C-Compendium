#ifndef SystemSevenOrLater
	#define SystemSevenOrLater 1
#endif

#ifdef THINK_Rez
	#include <Pict.r>
	#include <Types.r>
	#include <SysTypes.r>
#else
	#include <Pict.r>
	#include <Types.r>
	#include <SysTypes.r>
#endif

#include "boxmaker templates.h"

resource 'typs' (128) {
	{
		'PICT'
	}
};

resource 'flgs' (128) {
	EnterFolders,
	dontPassFolders,
	dontEnterInvisibles,
	dontPassInvisibles
};

resource 'MENU' (128) {	/* Apple Menu */
	128,
	textMenuProc,
	0x7FFFFFFD,
	enabled,
	apple,
	{
		"About BoxMaker++�", noIcon, noKey, noMark, plain,
		"-", noIcon, noKey, noMark, plain
	}
};

resource 'MENU' (129) {	/* File Menu */
	129,
	textMenuProc,
	0x7FFFFFFF,
	enabled,
	"File",
	{
		"Make file monochrome�", noIcon, "O", noMark, plain,
		"Quit", noIcon, "Q", noMark, plain
	}
};

resource 'FREF' (128) { 'APPL', 0, ""};
resource 'FREF' (129) { 'PICT', 1, ""};
resource 'FREF' (130) { 'fold', 2, ""};

#define myType '��PC'

data myType (0) {};

resource 'BNDL' (0) {
	myType,
	0,
	{
		'FREF',
		{
			0, 128,
			1, 129,
			2, 130,
		},

		'ICN#',
		{
			0, 128,
			1, 0,
			2, 0,
		}
	}
};

resource 'MBAR' (128) {
	{
		128,
		129
	}
};
//
// Finder related resources:
//
resource 'vers' (1) {
	0x01,
	0x31,
	release,
	0x00,
	verUS,
	"1.3.1",
	"Monochromize 1.3.1,\nbuilt using BoxMaker++ 1.3.1"
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
