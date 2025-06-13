// Standard Type Definitions

#include "Types.r"
#include "SysTypes.r"


// Include application resources
// (and NOT the Source Code Mgr (Projector) check ID!)
include "MacSPD.rsrc" not 'ckid';

// Generated automagically in Think C, so must hide here in MPW C!
resource 'SIZE' (-1, purgeable)
{
	reserved,
	ignoreSuspendResumeEvents,	// acceptSuspendResumeEvents
	reserved,
	canBackground,				// cannotBackground
	doesActivateOnFGSwitch,		// needsActivateOnFGSwitch
	backgroundAndForeground,	// onlyBackground
	dontGetFrontClicks,			// getFrontClicks
	ignoreAppDiedEvents,		// acceptAppDiedEvents
	is32BitCompatible,			// not32BitCompatible
	notHighLevelEventAware,		// notHighLevelEventAware
	onlyLocalHLEvents,			// localAndRemoteHLEvents
	notStationeryAware,			// isStationeryAware
	dontUseTextEditServices,	// useTextEditServices
	reserved,
	reserved,
	reserved,
	2000*1024,					// preferred mem size (Bytes)
	500*1024					// minimum mem size (Bytes)
};
