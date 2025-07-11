// Copyright � 1992-1995 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// ExitToShellPatch.cp

#include "ExitToShellPatch.h"

// Most of this is simply stolen from UMacAppGlobals.cp


typedef pascal void (*ExitToShellType)(void);

enum
{
	uppExitToShellProcInfo = kPascalStackBased
};

#if USESROUTINEDESCRIPTORS
typedef UniversalProcPtr ExitToShellUPP;

#define CallExitToShellProc(userRoutine)		\
		CallUniversalProc((UniversalProcPtr)(userRoutine), uppExitToShellProcInfo)
#define NewExitToShellProc(userRoutine)  \
		(ExitToShellUPP) NewRoutineDescriptor((ProcPtr)(userRoutine), uppExitToShellProcInfo, GetCurrentISA())
#else
typedef ProcPtr ExitToShellUPP;

#define CallExitToShellProc(userRoutine)		\
		(*(userRoutine))()
#define NewExitToShellProc(userRoutine)		\
		(ExitToShellUPP)(userRoutine)
#endif
	
//==============================================================================
class MyTrapPatch : public TrapPatch
{
public:
	void Install(ExitToShellType routine);
};

inline void MyTrapPatch::Install(ExitToShellType routine)
{
	patchRoutine = NewExitToShellProc(StripLong(routine));
	FailNIL(patchRoutine);
	PatchTrap(_ExitToShell, patchRoutine);
}

MyTrapPatch pMyETSPatch;
struct FuncInfo
{
	ExitProcFunc fExitProc;
	FuncInfo *fNext;
};
static FuncInfo *gInfoHead = nil;

void ExecuteExitToShellPatches()
{
	FuncInfo *info = gInfoHead;
	while (info)
	{
		FailInfo fi;
		Try(fi) {
			info->fExitProc();
			fi.Success();
		}
		info = info->fNext;
	}
}

pascal void MyExitToShellCleanup()
{
	long OldA5 = SetCurrentA5();				// ***** Called from trap patches *****
	ExitToShellType oldTrapAddr = (ExitToShellType) pMyETSPatch.GetOldTrapAddr();
	pMyETSPatch.UnpatchTrap();					// Guaranteed not to fail 
	ExecuteExitToShellPatches();
	CallExitToShellProc(oldTrapAddr);
	SetA5(OldA5);
}

void MakeExitToShellPatch(ExitProcFunc exitProc)
{
	if (gInfoHead == nil)
		pMyETSPatch.Install(MyExitToShellCleanup);
	FuncInfo *info = new FuncInfo();
	info->fNext = gInfoHead;
	gInfoHead = info;
	info->fExitProc = exitProc;
}
