/* Speedy Mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

unsigned long	me;
int				oldX, oldY;

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
	int				vx,vy;
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	
	if (me != 'MMdl')
	{
		me = 'MMdl';
		oldX=RawMouse.h;
		oldY=RawMouse.v;
	}
	
	vx=RawMouse.h-oldX;
	vy=RawMouse.v-oldY;
	
	if ((vx>=-100) && (vx<=100) && (vy>=-100) && (vy<=100))
	{
		RawMouse.h+=vx;
		mTemp.h+=vx;
		RawMouse.v+=vy;
		mTemp.v+=vy;
		if ((vx!=0) || (vy!=0))
			CrsrNew = TRUE;
	}
		
	oldX=RawMouse.h;
	oldY=RawMouse.v;
	
	myVBL->vblCount = 1;
	RestoreA4();
}
