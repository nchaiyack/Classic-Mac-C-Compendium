/* Unhelpful mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"
#include "GestaltEQU.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

Rect			gMainScreenBounds;
unsigned long	me;
Rect			helpRect;
int				oldX,oldY;

#define LEFTSIDE		-69
#define RIGHTSIDE		-37
#define TOPSIDE			0
#define BOTTOMSIDE		MBarHeight+2

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
	Boolean			isDormant;
	
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
		SetRect(&helpRect, gMainScreenBounds.right+LEFTSIDE,
							gMainScreenBounds.top+TOPSIDE,
							gMainScreenBounds.right+RIGHTSIDE,
							gMainScreenBounds.top+BOTTOMSIDE);
		OffsetRect(&helpRect, gMainScreenBounds.left, gMainScreenBounds.top);
		oldX=RawMouse.h;
		oldY=RawMouse.v;
	}
	
	if (PtInRect(RawMouse, &helpRect))
	{
		if (oldX<=helpRect.left)
			RawMouse.h=mTemp.h=helpRect.right+1;
		else if (oldX>=helpRect.right)
			RawMouse.h=mTemp.h=helpRect.left-1;
		else
			RawMouse.v=mTemp.v=helpRect.bottom+1;
		CrsrNew = TRUE;
	}
	
	oldX=RawMouse.h;
	oldY=RawMouse.v;
	
	myVBL->vblCount = 1;
	RestoreA4();
}
