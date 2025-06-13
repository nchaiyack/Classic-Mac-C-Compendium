/* Ghost mouse -- a Mousebroken module */
/* written by Mark Pilgrim, November 1993 */
/* This module placed in the public domain. */

#include "Retrace.h"

void main(void)
{
	VBLTask*		myVBL;
	
	asm
	{
		move.l d0, myVBL
	}
	ObscureCursor();
	myVBL->vblCount = 150;
}
