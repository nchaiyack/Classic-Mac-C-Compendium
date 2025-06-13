/* Square Mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

void main(void);

void main(void)
{
	VBLTask*		myVBL;
	unsigned long	temp;
	
	asm
	{
		move.l d0, myVBL
	}
	
	GetDateTime(&temp);
	switch (temp&0x00000003)
	{
		case 0:
			RawMouse.h++;
			mTemp.h++;
			break;
		case 1:
			RawMouse.v++;
			mTemp.v++;
			break;
		case 2:
			RawMouse.h--;
			mTemp.h--;
			break;
		case 3:
			RawMouse.v--;
			mTemp.v--;
			break;
	}
	
	CrsrNew = TRUE;
	myVBL->vblCount = 1;
}
