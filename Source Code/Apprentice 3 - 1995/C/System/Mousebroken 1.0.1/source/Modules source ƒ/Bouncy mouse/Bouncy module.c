/* Bouncy Mouse -- a mouse module for Mousebroken */
/* written by Mark Pilgrim, November 1993 */
/* This module placed in the public domain. */

#include "Retrace.h"
#include "GestaltEQU.h"

extern Boolean CrsrNew : 0x8CE;
extern Point mTemp : 0x828;

Rect			gMainScreenBounds;
int				direction;
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
	VBLTask*		myVBL;			/* pointer to VBL structure  */
	long			gestalt_temp;
	OSErr			isHuman;
	
	RememberA0();					/* for global variables */
	SetUpA4();						/* for global variables */
	
	asm								/* move VBL structure pointer into local variable */
	{								/* need this so we can set vblCount later */
		move.l d0, myVBL			/* (see below) */
	}
	
	if (me != 'MMdl')
	{
		isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);	/* Quickdraw version? */
		gMainScreenBounds=(isHuman || (gestalt_temp < gestalt8BitQD)) ? screenBits.bounds :
			(**GetMainDevice()).gdRect;
		me = 'MMdl';				/* need so we don't re-initialize */
		direction=1;				/* to start off the bouncing */
	}
	
	switch (direction)
	{
		case 1:
			mTemp.h--;
			mTemp.v--;
			if (mTemp.h<=gMainScreenBounds.left)
				direction=4;
			else if (mTemp.v<=gMainScreenBounds.top)
				direction++;
			break;
		case 2:
			mTemp.h--;
			mTemp.v++;
			if (mTemp.v>=gMainScreenBounds.bottom-1)
				direction--;
			else if (mTemp.h<=gMainScreenBounds.left)
				direction++;
			break;
		case 3:
			mTemp.h++;
			mTemp.v++;
			if (mTemp.h>=gMainScreenBounds.right-1)
				direction--;
			else if (mTemp.v>=gMainScreenBounds.bottom-1)
				direction++;
			break;
		case 4:
			mTemp.h++;
			mTemp.v--;
			if (mTemp.h>=gMainScreenBounds.right-1)
				direction=1;
			else if (mTemp.v<=gMainScreenBounds.top)
				direction--;
			break;
	}
		
	CrsrNew = TRUE;			/* to redraw mouse */
	myVBL->vblCount = 1;	/* how many ticks until this code gets called again */
	RestoreA4();
}
