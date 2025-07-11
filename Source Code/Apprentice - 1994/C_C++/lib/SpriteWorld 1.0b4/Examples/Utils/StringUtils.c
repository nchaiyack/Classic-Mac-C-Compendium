///--------------------------------------------------------------------------------------
//	StringUtils.c
//
//	Created:	12/17/91 at 12:35:35 AM
//	By:		Tony Myles
//
//	Copyright: � 1991-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __STRINGUTILS__
#include "StringUtils.h"
#endif

#if MPW
#pragma segment Utils
#endif

///--------------------------------------------------------------------------------------
// PStrCpy
///--------------------------------------------------------------------------------------

void PStrCpy(
	Str255 srcStr,
	Str255 dstStr)
{
	BlockMove(srcStr, dstStr, 1 + srcStr[0]);
}


///--------------------------------------------------------------------------------------
// PStrCat
///--------------------------------------------------------------------------------------

void PStrCat(
	Str255 srcStr,
	Str255 dstStr)
{
	BlockMove(srcStr + 1, dstStr + dstStr[0] + 1, srcStr[0]);

	dstStr[0] += srcStr[0];
}

