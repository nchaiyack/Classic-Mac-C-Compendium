// UseAsm.h											<tur 26-Feb-94>
//
// After Robert S. Mah's original
//

#pragma once

#ifndef	__UseAsm__
#define	__UseAsm__

#define	USE_ASM	0

#if	!__powerc
#if __MWERKS__
#	undef  USE_ASM
#	define USE_ASM		1
#	define ASM_FUNC	asm
#	define ASM_BEGIN
#	define ASM_END
#elif THINK_C
#	undef  USE_ASM
#	define USE_ASM		1
#	define ASM_FUNC
#	define ASM_BEGIN	asm{
#	define ASM_END		}
#else
#	define USE_ASM		0
#endif
#endif	/* !__powerc */

#endif	/* __UseAsm__ */

