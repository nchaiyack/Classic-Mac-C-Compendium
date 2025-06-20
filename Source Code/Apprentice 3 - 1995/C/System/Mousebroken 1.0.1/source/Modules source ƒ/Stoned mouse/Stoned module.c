/* Stoned Mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"

extern Boolean	CrsrNew : 0x8CE;
extern Point	mTemp : 0x828;
extern Point	RawMouse : 0x82C;

void main(void);

void main(void)
{
	VBLTask*		myVBL;
	
	asm
	{
		move.l d0, myVBL
	}
	mTemp.h=RawMouse.h;
	mTemp.v=RawMouse.v;
	CrsrNew = TRUE;
	myVBL->vblCount = 1;
}
