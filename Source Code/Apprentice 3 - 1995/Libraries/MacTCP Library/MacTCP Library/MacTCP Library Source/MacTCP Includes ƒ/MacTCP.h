/*
	MacTCP.h
	
	Header file for MacTCP.c
	
	01/31/94 dn - Created.
	
*/

#pragma once

#ifndef __H_MacTCP__
#define __H_MacTCP__

#ifndef __H_MacTCPIncludes__
#include <MacTCPIncludes.h>
#endif

typedef pascal void (*ExitToShellProcPtr)(void);

enum {
	uppExitToShellProcInfo=kPascalStackBased
};

#if USESROUTINEDESCRIPTORS
	typedef UniversalProcPtr ExitToShellUPP;
	
	#define CallExitToShellProc(routine)\
		CallUniversalProc((UniversalProcPtr)routine,uppExitToShellProcInfo)
	#define NewExitToShellProc(routine)\
		(ExitToShellUPP)NewRoutineDescriptor((ProcPtr)(routine),uppExitToShellProcInfo,GetCurrentISA())
#else
	typedef ExitToShellProcPtr ExitToShellUPP;
	
	#define CallExitToShellProc(routine) \
		(*routine)()
	#define NewExitToShellProc(routine) \
		(ExitToShellUPP)(routine)
#endif

#define DisposeExitToShellProc(routine) \
	DisposeRoutineDescriptor(routine)

#ifdef __cplusplus
extern "C" {
#endif

OSErr OpenMacTCP(short*,Boolean patchExitToShell);

OSErr KillMacTCP(void);

#ifdef __cplusplus
}
#endif

#endif /* __H_MacTCP__ */
