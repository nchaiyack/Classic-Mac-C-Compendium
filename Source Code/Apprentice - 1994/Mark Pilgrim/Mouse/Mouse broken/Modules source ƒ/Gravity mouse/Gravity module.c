/* Gravity Mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"
#include "GestaltEQU.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;

Rect			gMainScreenBounds;
unsigned long	me;
Boolean			fallingNow;
int				gap;

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
	long			gestalt_temp;
	OSErr			isHuman;
	Boolean			gHasColorQD;
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	
	if (me != 'MMdl')
	{
		isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
		gHasColorQD = !(isHuman || (gestalt_temp < gestalt8BitQD));
		gMainScreenBounds = (gHasColorQD) ? (**GetMainDevice()).gdRect : screenBits.bounds;
		me = 'MMdl';
		fallingNow=FALSE;
	}
	
	if (!fallingNow)
		gap=1;
	
	fallingNow=TRUE;
	mTemp.v+=gap/5;
	if (mTemp.v>gMainScreenBounds.bottom)
		fallingNow=FALSE;
	gap++;
	CrsrNew = TRUE;

	myVBL->vblCount = (fallingNow) ? 1 : ((TickCount()&0x7fffffff)%4096)+2500;
	RestoreA4();
}
