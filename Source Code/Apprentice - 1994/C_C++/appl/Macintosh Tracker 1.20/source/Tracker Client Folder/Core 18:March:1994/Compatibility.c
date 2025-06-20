/* Compatibility.c */

#define COMPILING_COMPATIBILITY_C
#include "Compatibility.h"
#include "GestaltEqu.h"
#include "Traps.h"


CompatibleRec	Compatibility;


/* since this code is called by Main, we should do this: */
#if __option(mc68020)
	#define CodeFor68020
#else
	#define CodeFor68000
#endif
#pragma options(!mc68020)


void		InitCompatibility(void)
	{
		OSErr		Error;
		long		Result;

		Error = Gestalt(gestaltAppleEventsAttr,&Result);
		HasAppleEvents = ((Result & (1 << gestaltAppleEventsPresent)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltFPUType,&Result);
		HasMathCoprocessor = (Result != gestaltNoFPU) && (Error == noErr);

		Error = Gestalt(gestaltVMAttr,&Result);
		HasVirtualMemory = ((Result & (1 << gestaltVMPresent)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltOSAttr,&Result);
		HasGoodTemporaryMemory = ((Result & (1 << gestaltTempMemSupport)) != 0)
			&& ((Result & (1 << gestaltRealTempMemory)) != 0)
			&& ((Result & (1 << gestaltTempMemTracked)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltProcessorType,&Result);
		Has020orBetter = (Result >= gestalt68020) && (Error == noErr);

		Error = Gestalt(gestaltStandardFileAttr,&Result);
		HasFSSpecStandardFile = ((Result & (1 << gestaltStandardFile58)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltFSAttr,&Result);
		HasFSSpec = ((Result & (1 << gestaltHasFSSpecCalls)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltFindFolderAttr,&Result);
		HasFindFolder = ((Result & (1 << gestaltFindFolderPresent)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltQuickdrawFeatures,&Result);
		HasGreyishTextOr = ((Result & (1 << gestaltHasGrayishTextOr)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltAliasMgrAttr,&Result);
		HasAliasManager = ((Result & (1 << gestaltAliasMgrPresent)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltHelpMgrAttr,&Result);
		HasHelpManager = ((Result & (1 << gestaltHelpMgrPresent)) != 0) && (Error == noErr);

		Error = Gestalt(gestaltPowerMgrAttr,&Result);
		HasPowerManager = ((Result & (1 << gestaltPMgrExists)) != 0) && (Error == noErr);
	}


#ifdef CodeFor68020
	#pragma options(mc68020)
#endif
