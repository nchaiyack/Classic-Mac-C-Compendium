///--------------------------------------------------------------------------------------
//	DebugUtils.c
//
//	Created:	12/17/91 at 12:35:35 AM
//	By:		Tony Myles
//
//	Copyright: � 1991-94 Tony Myles, All rights reserved worldwide.
//
//	Description:	utility routines used for debugging purposes
///--------------------------------------------------------------------------------------


#if SW_PPC || __MWERKS__
#ifndef __TEXTUTILS__
#include <TextUtils.h>
#endif
#elif THINK_C
#ifndef __BDC__
#include <BDC.h>
#endif
#elif MPW
#ifndef __PACKAGES__
#include <Packages.h>
#endif
#endif 

#ifndef __SEGLOAD__
#include <SegLoad.h>
#endif

#ifndef __STRINGUTILS__
#include "StringUtils.h"
#endif

#ifndef __DEBUGUTILS__
#include "DebugUtils.h"
#endif



#if MPW
#pragma segment Utils
#endif


///--------------------------------------------------------------------------------------
// DebugNum
///--------------------------------------------------------------------------------------

void DebugNum(
	long bugNum)
{
	Str255 bugNumStr;

		// convert the bug number to a string
	NumToString(bugNum, bugNumStr);

		// do it
	DebugStr(bugNumStr);
}


///--------------------------------------------------------------------------------------
// DebugStrNum
///--------------------------------------------------------------------------------------

void DebugStrNum(
	Str255 bugStr,
	long bugNum)
{
	Str255 tmpStr, bugNumStr;

		// copy the bugStr to a local str and append a colon+space to it
	PStrCpy(bugStr, tmpStr);
	PStrCat("\p: ", tmpStr);

		// convert the bugNum to a string and append it to the local string
	NumToString(bugNum, bugNumStr);
	PStrCat(bugNumStr, tmpStr);

		// do it.
	DebugStr(tmpStr);
}


///--------------------------------------------------------------------------------------
// FatalError
///--------------------------------------------------------------------------------------

void FatalError(
	OSErr err,
	Str255 errMsgStr)
{
	if (err != noErr)
	{
		if (errMsgStr == NULL)
		{
			DebugNum((long)err);
		}
		else
		{
			DebugStrNum(errMsgStr, (long)err);
		}

		ExitToShell();
	}
}
