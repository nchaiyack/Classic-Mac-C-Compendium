///--------------------------------------------------------------------------------------
//	SWCommonHeaders.h
//
//	Created:	3/9/94 at 12:09:40 AM
//	By:			Tony Myles
//
//	Copyright: © 1994 Tony Myles, All rights reserved worldwide
//
//	Description:	common macros, constants, and stuff, used throughout SpriteWorld
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#define __SWCOMMON__

///--------------------------------------------------------------------------------------
//	sprite world macros
///--------------------------------------------------------------------------------------

#if defined(powerc) || defined(__powerc)
#define SW_PPC 1
#else 
#define SW_PPC 0
#endif

#define SW_MIN(a, b) ((a) < (b) ? (a) : (b))
#define SW_MAX(a, b) ((a) > (b) ? (a) : (b))

	// change this to '#define SW_FUNC pascal'
	// to build a SpriteWorld library callable from pascal
#define SW_FUNC

	// this is obsolete, but is maintained here for backward
	// compatibility with existing developer's sources
#define SW_PASCAL SW_FUNC


#define kBitsPerByte 8

#if __MWERKS__

#define SW_ASM_FUNC asm
#define SW_ASM_BEGIN
#define SW_ASM_END

#elif THINK_C

#define SW_ASM_FUNC
#define SW_ASM_BEGIN asm{
#define SW_ASM_END }

#endif

#endif /*__SWCOMMON__*/

