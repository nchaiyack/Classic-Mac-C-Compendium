/* Corner Mouse -- a Mousebroken mouse module */
/* written 11/93 by Mark Pilgrim */
/* This module placed in the public domain. */

#include "Retrace.h"
#include "GestaltEQU.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;
extern Point RawMouse : 0x82C;

Rect			gMainScreenBounds;
unsigned long	me;
Boolean			isInCorner;
Boolean			goingToCorner;
int				stickCount;
int				newX,newY;

#define MAX_STICK_COUNT		30
#define GRADIENT			5

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
		isInCorner=FALSE;
	}
	
	isDormant=FALSE;
	if (!isInCorner)
	{
		newX=(RawMouse.h-gMainScreenBounds.left<
			(gMainScreenBounds.right-gMainScreenBounds.left)/2) ?
			gMainScreenBounds.left : gMainScreenBounds.right-1;
		newY=(RawMouse.v-gMainScreenBounds.top<
			(gMainScreenBounds.bottom-gMainScreenBounds.top)/2) ?
			gMainScreenBounds.top : gMainScreenBounds.bottom-1;
		RawMouse.h+=(newX-RawMouse.h)/GRADIENT;
		RawMouse.v+=(newY-RawMouse.v)/GRADIENT;
		mTemp.h=RawMouse.h;
		mTemp.v=RawMouse.v;
		if ((RawMouse.h-newX>-GRADIENT) && (RawMouse.h-newX<GRADIENT) &&
			(RawMouse.v-newY>-GRADIENT) && (RawMouse.v-newY<GRADIENT))
		{
			isInCorner=TRUE;
			stickCount=MAX_STICK_COUNT;
		}
	}
	else
	{
		if ((RawMouse.h!=newX) || (RawMouse.v!=newY))
		{
			RawMouse.h=mTemp.h=newX;
			RawMouse.v=mTemp.v=newY;
			stickCount--;
			if (!stickCount)
			{
				isInCorner=FALSE;
				isDormant=TRUE;
			}
		}
	}
	
	CrsrNew = TRUE;
	myVBL->vblCount = isDormant ? 15000 : 1;
	RestoreA4();
}
