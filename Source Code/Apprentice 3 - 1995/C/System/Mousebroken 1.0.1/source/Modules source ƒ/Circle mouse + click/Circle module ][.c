/* Circle Mouse ][ -- a Mousebroken mouse module */
/* written 12/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"
#include "GestaltEQU.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

unsigned long	me;
int				vx,vy;
Boolean			xIncreasing, yIncreasing;

#define RADIUS		5		/* actual radius is (1+2+...+(RADIUS-1))+(RADIUS/2) */
#define TIMEDELAY	1

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
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	
	if (me != 'MMdl')
	{
		me = 'MMdl';
		vx=RADIUS;
		vy=0;
		xIncreasing=FALSE;
		yIncreasing=TRUE;
	}
	
	if (Button())
	{
		RawMouse.h+=vx;
		mTemp.h+=vx;
		RawMouse.v+=vy;
		mTemp.v+=vy;
		
		vx+=xIncreasing ? 1 : -1;
		vy+=yIncreasing ? 1 : -1;
		
		xIncreasing=(vx==RADIUS) ? FALSE : (vx==-RADIUS) ? TRUE : xIncreasing;
		yIncreasing=(vy==RADIUS) ? FALSE : (vy==-RADIUS) ? TRUE : yIncreasing;
		
		CrsrNew = TRUE;
	}
	
	myVBL->vblCount = TIMEDELAY;
	RestoreA4();
}
