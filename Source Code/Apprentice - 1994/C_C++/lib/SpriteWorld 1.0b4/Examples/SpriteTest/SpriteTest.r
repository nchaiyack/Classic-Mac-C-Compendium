/*
//	File:       SpriteTest.make
//	Created:    Tuesday, February 1, 1994 10:57:33 PM
//	By:			Tony Myles
//	Copyright © 1994 Tony Myles, All rights reserved worldwide.
*/

#include "Types.r"
#include "SysTypes.r"

resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	getFrontClicks,
	ignoreAppDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	notStationeryAware,
	dontUseTextEditServices,
	reserved,
	reserved,
	reserved,
	2048 * 1024,
	2048 * 1024	
};

INCLUDE "SpriteTest.¹.rsrc";
