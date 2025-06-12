// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// Nuntius.r

include "Nuntius" 'CODE';
include "Nuntius.ResEdit";
include "Nuntius.AdLib" not 'ALIB';
include "Misc.rsrc";
include "Strings.rsrc";

/* ================================   SIZE		================================== */
type 'SIZE' {
		boolean					reserved;
		boolean 				ignoreSuspendResumeEvents, acceptSuspendResumeEvents;
		boolean					reserved;
		boolean					cannotBackground, canBackground;
		boolean					needsActivateOnFGSwitch, doesActivateOnFGSwitch;			
		boolean					backgroundAndForeground, onlyBackground;					
		boolean					dontGetFrontClicks,	getFrontClicks;					
		boolean					ignoreAppDiedEvents, acceptAppDiedEvents;			
		boolean					not32BitCompatible,	is32BitCompatible;				
		boolean					notHighLevelEventAware, isHighLevelEventAware;			
		boolean					onlyLocalHLEvents, localAndRemoteHLEvents;			
		boolean					notStationeryAware, isStationeryAware;				
		boolean					dontUseTextEditServices, useTextEditServices;			 
		boolean					reserved;						
		boolean					reserved;						
		boolean					reserved;						
		unsigned longint;										
		unsigned longint;										
#define	reserved		false
};

resource 'SIZE' (-1) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreAppDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	isStationeryAware,				
	reserved,
	reserved,
	reserved,
	reserved,
#if qDebug
	2500 * 1024,
#else
	2500 * 1024,
#endif
	1000 * 1024
};

resource 'SIZE' (0) {
	reserved,
	acceptSuspendResumeEvents,
	reserved,
	canBackground,
	doesActivateOnFGSwitch,
	backgroundAndForeground,
	dontGetFrontClicks,
	ignoreAppDiedEvents,
	is32BitCompatible,
	isHighLevelEventAware,
	localAndRemoteHLEvents,
	isStationeryAware,				
	reserved,
	reserved,
	reserved,
	reserved,
#if qDebug
	2500 * 1024,
#else
	2500 * 1024,
#endif
	1000 * 1024
};
