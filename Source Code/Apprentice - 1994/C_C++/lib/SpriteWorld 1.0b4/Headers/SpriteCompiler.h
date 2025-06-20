///--------------------------------------------------------------------------------------
//	SpriteCompiler.h
//
//	Created:	1/17/94
//	By:			Tony Myles
//
//	Copyright: � 1994 Tony Myles, All rights reserved worldwide
//
//	Description:	constants, structures, and prototypes for the sprite compiler
///--------------------------------------------------------------------------------------


#ifndef __SPRITECOMPILER__
#define __SPRITECOMPILER__

#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

enum
{
	kMaxInstructionSize = 8,
	kPixelCodeResType = 'PxCd'
};


typedef short PixelCode, *PixelCodePtr, **PixelCodeHdl;

typedef void (*BlitFuncPtr)(
	long srcStride,
	long destStride,
	Ptr srcBaseAddr,
	Ptr destBaseAddr);


#ifdef __cplusplus
extern "C" {
#endif

OSErr SWCompileColorIconResource(
	short iconResID,
	PixelCodeHdl* newPixCodeH);

OSErr SWParsePixels(
	PixMapHandle srcPixMapH,
	PixMapHandle maskPixMapH,
	PixelCodeHdl* newPixCodeH);

OSErr SWSavePixelCodeResource(
	PixelCodeHdl newPixCodeH,
	short pixCodeResID);

OSErr SWCompileMaskScanLine(
	PixelCodeHdl newPixCodeH,
	Ptr maskPixelP,
	long numberOfPixels,
	Boolean isLastScanLine);

OSErr SWGeneratePreamble(
	PixelCodeHdl srcPixCodeH);

OSErr SWGeneratePostamble(
	PixelCodeHdl srcPixCodeH);

#ifdef __cplusplus
};
#endif

#endif

