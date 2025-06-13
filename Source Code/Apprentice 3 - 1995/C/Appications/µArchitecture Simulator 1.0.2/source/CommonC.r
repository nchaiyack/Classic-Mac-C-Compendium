/*
Copyright © 1993,1994 by Fabrizio Oddone
¥¥¥ ¥¥¥ ¥¥¥ ¥¥¥ ¥¥¥ ¥¥¥ ¥¥¥ ¥¥¥ ¥¥¥ ¥¥¥
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

#define SystemSevenOrLater 1

#include "SysTypes.r"
#include "Types.r"

#include "BalloonC.r"

/*
resource 'SIZE' (-1, purgeable) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreChildDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	isStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	311296,
	278528
};
*/

resource 'STR ' (138, purgeable, preload) {
	"µSimulator Prefs"
};

data 'HAND' (128, purgeable, preload) {
	$"E400"
};

resource 'nrct' (128, locked, preload) {
	{	/* array RectArray: 5 elements */
		/* [1] */
		{155, 451, 177, 485},
		/* [2] */
		{250, 258, 272, 486},
		/* [3] */
		{7, 70, 271, 244},
		/* [4] */
		{7, 5, 271, 52},
		/* [5] */
		{219, 257, 170, 430}
	}
};

resource 'open' (128) {
	'µSIM', { 'µPRG', 'sPRG', 'µRAM', 'µREG', 'TEXT' }
};

