/* Restless Mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

unsigned long	me;
unsigned int	iter;

void header(void);
void main(void);

void header(void)
{
	asm
	{
		dc.l	0
		move.l a0, d0
		lea header, a0
		jmp main
	}
}

#include "SetUpA4.h"

void main(void)
{
	VBLTask*		myVBL;
	unsigned char	thisChar;
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	
	if (me != 'MMdl')
	{
		me = 'MMdl';
		iter=0;
	}
	
	thisChar=*((unsigned char*)(ROMBase+iter));
	switch ((thisChar&0x38)>>3)
	{
		case 0x00:
			RawMouse.h++;
			mTemp.h++;
			break;
		case 0x01:
		case 0x02:
		case 0x03:
			RawMouse.h--;
			mTemp.h--;
			break;
		case 0x04:
		case 0x05:
			RawMouse.v++;
			mTemp.v++;
			break;
		case 0x06:
		case 0x07:
			RawMouse.v--;
			mTemp.v--;
			break;
	}
	
	iter++;
	CrsrNew = TRUE;
	myVBL->vblCount = 3;
	RestoreA4();
}
