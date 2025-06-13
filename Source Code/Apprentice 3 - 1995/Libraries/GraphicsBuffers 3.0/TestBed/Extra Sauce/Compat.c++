// Compat.c++

#include <Traps.h>
#include <OSUtils.h>


#include "Compat.h"

MacEnviron gEnviron;


#define TrapMask 0x0800


short NumToolboxTraps();
TrapType GetTrapType(short theTrap);
Boolean TrapAvailable(short theTrap);


short NumToolboxTraps() {
	if (NGetTrapAddress(_InitGraf, ToolTrap) ==
			NGetTrapAddress(0xAA6E, ToolTrap))
		return(0x0200);
	else
		return(0x0400);
} // END NumToolboxTraps

TrapType GetTrapType(short theTrap) {
	if ((theTrap & TrapMask) > 0)
		return(ToolTrap);
	else
		return(OSTrap);

} // END GetTrapType

Boolean TrapAvailable(short theTrap) {
	TrapType	tType;

	tType = GetTrapType(theTrap);
	if (tType == ToolTrap)
	theTrap = theTrap & 0x07FF;
	if (theTrap >= NumToolboxTraps())
		theTrap = _Unimplemented;

	return (NGetTrapAddress(theTrap, tType) !=
			NGetTrapAddress(_Unimplemented, ToolTrap));
} // END TrapAvailable

// ***************************************************************************

void CheckEnviron() {
	long gestaltInfo;
	SysEnvRec myEnv;
	
	// Get GESTALT availability
	gEnviron.gestaltAvail = TrapAvailable(_Gestalt);
	// Get WAITNEXTEVENT availability
	gEnviron.WNEAvail     = TrapAvailable(_WaitNextEvent);
	
	if (gEnviron.gestaltAvail) {
		// Get PROCESSOR type
		Gestalt(gestaltProcessorType, &gestaltInfo);
		gEnviron.cpu = gestaltInfo;
		
		// Get FPU availability
		Gestalt(gestaltFPUType, &gestaltInfo);
		gEnviron.fpu = gestaltInfo;
		
		// Get SYSTEM VERSION
		Gestalt(gestaltSystemVersion, &gestaltInfo);
		gEnviron.sysVersion = gestaltInfo;
		
		// Get whether COLOR QUICKDRAW is present or not
		SysEnvirons(curSysEnvVers, &myEnv);
		gEnviron.hasColor = gEnviron.hasCQD = myEnv.hasColorQD;
		if (gEnviron.hasColor)
			gEnviron.pixDepth = (**((**GetMainDevice()).gdPMap)).pixelSize;
			// Equivalent to GetMainDevice^^.gdPMap^^.pixelSize in Pascal!!
		else
			gEnviron.pixDepth = 1;
			
		if (gEnviron.pixDepth < 2)
			gEnviron.hasColor = FALSE;

		// Determine if we have TEMPORARY MEMORY support
		Gestalt(gestaltOSAttr, &gestaltInfo);
		if (BitTst(&gestaltInfo, 31-gestaltTempMemSupport))
			gEnviron.hasTempMem = TRUE;
		else
			gEnviron.hasTempMem = FALSE;
			
		// Do we have an APPLE SOUND CHIP installed?
		Gestalt(gestaltHardwareAttr, &gestaltInfo);
		if (BitTst(&gestaltInfo, 31-gestaltHasASC))
			gEnviron.hasSoundChip = TRUE;
		else
			gEnviron.hasSoundChip = FALSE;
	}
} // END CheckEnviron