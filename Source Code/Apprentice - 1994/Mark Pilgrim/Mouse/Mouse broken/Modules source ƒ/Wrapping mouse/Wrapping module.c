/* Wrapping Mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"
#include "GestaltEQU.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

Rect			gMainScreenBounds;
unsigned long	me;

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
	
	RememberA0();
	SetUpA4();
	
	asm
	{
		move.l d0, myVBL
	}
	
	if (me != 'MMdl')
	{
		isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);
		gMainScreenBounds=(isHuman || (gestalt_temp < gestalt8BitQD)) ? screenBits.bounds :
			(**GetMainDevice()).gdRect;
		me = 'MMdl';
	}
	
	if (RawMouse.h==gMainScreenBounds.left)
	{
		RawMouse.h=mTemp.h=gMainScreenBounds.right-2;
		CrsrNew = TRUE;
	}
	else if (RawMouse.h==gMainScreenBounds.right-1)
	{
		RawMouse.h=mTemp.h=gMainScreenBounds.left+1;
		CrsrNew = TRUE;
	}
	
	if (RawMouse.v==gMainScreenBounds.top)
	{
		RawMouse.v=mTemp.v=gMainScreenBounds.bottom-2;
		CrsrNew = TRUE;
	}
	else if (RawMouse.v==gMainScreenBounds.bottom-1)
	{
		RawMouse.v=mTemp.v=gMainScreenBounds.top+1;
		CrsrNew = TRUE;
	}
	
	myVBL->vblCount = 1;
	RestoreA4();
}
