/*
SetMouse.c

Sets the mouse location to the given point, which is specified in the local
coordinate system of the current port.

WARNING: In the Q & A Stack, Apple tells you how to do this, but warns that the
technique uses undocumented low-memory locations that are considered unsupported
and volatile and may change in future CPUs.

From "Code gadgets: Setting the mouse location", THINKin' CaP, 1(2):28-29, Fall 1990.

Copyright � 1991 SPLAsh Resources. All rights reserved. This source code is
copyrighted, but free. This means that programmers may use the code and
incorporate it into their own programs (commercial or otherwise) without payment
of any fees. However, the source code itself may not be sold or distributed on
electronic bulletin board services without written permission from SPLAsh
Resources.

SPLAsh Resources
1678 Shattuck Ave #302
Berkeley, CA  94709
(415) 527-0122

HISTORY:
2/25/91 dgp added to VideoToolbox
8/24/91	dgp	Made compatible with THINK C 5.0.
*/
#include "VideoToolbox.h"

#if THINK_C==1	/* old version */
	extern	Point	MTemp		:	0x828;		/* Low memory globals */
	extern	Point	RawMouse	:	0x82C;
	extern	Byte	CrsrNew		:	0x8CE;
	extern	Byte	CrsrCouple	:	0x8CF;
#else
	#include <SysEqu.h>
	#define MTemp (*(Point *)MTemp)
	#define RawMouse (*(Point *)RawMouse)
	#define CrsrNew (*(Byte *)CrsrNew)
	#define CrsrCouple (*(Byte *)CrsrCouple)
#endif

void SetMouse(Point	where)
{
	LocalToGlobal(&where);		/* Convert point to global coordinates	*/
	
	/* Quoting from the Q & A Stack: "If you wish to place the cursor in an	*/
	/* absolute location on the screen, you must set RawMouse, and MTemp to	*/
	/* the same value, and set CrsrNew to the same value as CrsrCouple."	*/
		
	MTemp=RawMouse=where;
	CrsrNew=CrsrCouple;
}
