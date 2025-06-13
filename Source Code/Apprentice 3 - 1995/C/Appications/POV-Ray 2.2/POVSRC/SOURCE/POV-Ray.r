/*
==============================================================================
Project:	POV-Ray

Version:	2.0

File Name:	POV-Ray.r

Description:
	MPW Rez sourcefile.
	This is currently here just to load the precompiled resources
	into the application.

Related Files:
	Pov.c: Main program file
	POV.¹.rsrc: What rsrcs get appended
	POV-Ray.make: Who makes me
------------------------------------------------------------------------------
Author:
	Eduard [esp] Schwan
------------------------------------------------------------------------------
	from Persistence of Vision Raytracer
	Copyright 1993 Persistence of Vision Team
------------------------------------------------------------------------------
	NOTICE: This source code file is provided so that users may experiment
	with enhancements to POV-Ray and to port the software to platforms other 
	than those supported by the POV-Ray Team.  There are strict rules under
	which you are permitted to use this file.  The rules are in the file
	named POVLEGAL.DOC which should be distributed with this file. If 
	POVLEGAL.DOC is not available or for more info please contact the POV-Ray
	Team Coordinator by leaving a message in CompuServe's Graphics Developer's
	Forum.  The latest version of POV-Ray may be found there as well.

	This program is based on the popular DKB raytracer version 2.12.
	DKBTrace was originally written by David K. Buck.
	DKBTrace Ver 2.0-2.12 were written by David K. Buck & Aaron A. Collins.
------------------------------------------------------------------------------
More Info:
	This Macintosh version of POV-Ray was created and compiled by Jim Nitchals
	(Think 5.0) and Eduard Schwan (MPW 3.2), based (loosely) on the original
	port by Thomas Okken and David Lichtman, with some help from Glenn Sugden.

	For bug reports regarding the Macintosh version, you should contact:
	Eduard [esp] Schwan
		CompuServe: 71513,2161
		Internet: jl.mactech@applelink.apple.com
		AppleLink: JL.MacTech
	Jim Nitchals
		Compuserve: 73117,3020
		America Online: JIMN8
		Internet: jimn8@aol.com -or- jimn8@applelink.apple.com
		AppleLink: JIMN8
------------------------------------------------------------------------------
Change History:
	920308	[esp]	Created.
	920415	[esp]	Added QuickTime SC stuff
	920429	[esp]	Removed QuickTime SC stuff and updated header comment refs
	920825	[esp]	Updated SIZE flag names for MPW 3.2.3 compatibility
	930817	[esp]	Updated App mem size from 2350 to 2500 since max_syms went to 1000
	930829	[esp]	Added the split resource files included
	931001	[esp]	version 2.0 finished (Released on 10/4/93)
==============================================================================
*/

// Standard Type Definitions

#include "Types.r"
#include "SysTypes.r"


// Include application resources
// (and NOT the Source Code Mgr (Projector) check ID!)
// Main resources
include "POV.¹.rsrc" not 'ckid';

// Balloon help resources
// #ifdef DO_BALLOON_HELP
// include "POVHelp.rsrc" not 'ckid';

// Generated automagically in Think C, so must hide here in MPW C!
resource 'SIZE' (-1, purgeable)
{
	reserved,
	acceptSuspendResumeEvents,	// ignoreSuspendResumeEvents
	reserved,
	canBackground,				// cannotBackground
	doesActivateOnFGSwitch,		// needsActivateOnFGSwitch
	backgroundAndForeground,	// onlyBackground
	dontGetFrontClicks,			// getFrontClicks
	ignoreAppDiedEvents,		// acceptAppDiedEvents
	is32BitCompatible,			// not32BitCompatible
	isHighLevelEventAware,		// notHighLevelEventAware
	localAndRemoteHLEvents,		// onlyLocalHLEvents
	notStationeryAware,			// isStationeryAware
	dontUseTextEditServices,	// useTextEditServices
	reserved,
	reserved,
	reserved,
	2500*1024,					// preferred mem size (Bytes)
	1000*1024					// minimum mem size (Bytes)
};

