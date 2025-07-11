// AnimatedCursor_Utils.cp
// Copyright �1994 David Kreindler. All rights reserved.

// Memory Manager and Resource Manager wrapper functions for ACurHandles and CursHandles

// revision history
//	940319 DK: initial version

#ifdef __MWERKS__
#	pragma options align=mac68k
#endif

#include <memory.h>
#include <resources.h>

#ifdef __MWERKS__
#	pragma options align=reset
#endif

static void DetachResource(ACurHandle);
static void DisposeHandle(ACurHandle);
static void HLockHi(ACurHandle);
static void HNoPurge(ACurHandle);
static void HUnlock(ACurHandle);

static void ReleaseResource(CursHandle);
static void HNoPurge(CursHandle);
static void HLockHi(CursHandle);
static void HUnlock(CursHandle);

static ACurHandle GetACur(short aCurID);

inline void
DetachResource(ACurHandle aCurH) {
	::DetachResource((Handle)aCurH);
}

inline void
DisposeHandle(ACurHandle aCurH) {
	::DisposeHandle((Handle)aCurH);
}

inline void
HLockHi(ACurHandle aCurH) {
	::HLockHi((Handle)aCurH);
}

inline void
HNoPurge(ACurHandle aCurH) {
	::HNoPurge((Handle)aCurH);
}

inline void
HUnlock(ACurHandle aCurH) {
::HUnlock((Handle)aCurH);
}

inline void
ReleaseResource(CursHandle cursH) {
	::ReleaseResource((Handle)cursH);
}

inline void
HLockHi(CursHandle cursH) {
	::HLockHi((Handle)cursH);
}

inline void
HNoPurge(CursHandle cursH) {
	::HNoPurge((Handle)cursH);
}

inline void
HUnlock(CursHandle cursH) {
	::HUnlock((Handle)cursH);
}

inline ACurHandle
GetACur(short aCurResID) {
	return (ACurHandle)GetResource('acur', aCurResID);
}