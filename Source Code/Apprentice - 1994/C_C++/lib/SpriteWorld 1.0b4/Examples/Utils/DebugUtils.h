///--------------------------------------------------------------------------------------
//	DebugUtils.h
//
//	Created:	12/17/91 at 12:35:35 AM
//	By:			Tony Myles
//
//	Copyright � 1991-94 Tony Myles, All rights reserved worldwide.
//
//	Description:	prototypes for debugging utility routines
///--------------------------------------------------------------------------------------


#ifndef __DEBUGUTILS__
#define __DEBUGUTILS__

#ifndef __TYPES__
#include <Types.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif


void DebugNum(long bugNum);
void DebugStrNum(Str255 bugStr, long bugNum);
void FatalError(OSErr err, Str255 errMsgStr);


#ifdef __cplusplus
};
#endif
#endif