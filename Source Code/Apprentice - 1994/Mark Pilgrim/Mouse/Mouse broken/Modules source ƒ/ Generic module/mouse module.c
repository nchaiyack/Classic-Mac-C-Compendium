/* Generic mouse module -- a foundation for building modules for Mousebroken */
/* version 1.0 -- November 29, 1993 by Mark Pilgrim                          */
/* This module placed in the public domain.                                  */

/* Hint: Try removing MacTraps from your project file.  This module needs it only  */
/*       because it uses screenBits & Gestalt.  If your module doesn't do anything */
/*       funky like that, you may not need MacTraps.                               */

/* Reminder: Don't forget to customize the resource file to add your own icon,  */
/*           copyright message, module info string, Get Info info strings.      */
/*           The icon is cicn 668 and ICON 668.  The copyright message is       */
/*           STR# 668, string 1.  The module info string is STR# 668, string 2. */
/*           The Get Info strings are vers 1 and vers 2.                        */

/* Project Type:		Code Resource				*/
/* File Type:			'MMdl'						*/
/* File Creator:		'MBrk'						*/
/* Resource Type:		'vbl '						*/
/* Resource ID:			668							*/
/* Resource name:		(irrelevant)				*/
/* Resource attributes:	system heap + locked (0x50)	*/
/* Custom header									*/
/* No multi-segment									*/

#include "Retrace.h"				/* always need */
#include "GestaltEQU.h"				/* delete if don't check environment */

extern Boolean CrsrNew : 0x8CE;		/* delete if don't change mouse location */
extern Point MouseOffset : 0x8DA;	/* delete if don't change mouse offset */
extern Point mTemp : 0x828;			/* delete if don't change mouse location */
extern Point RawMouse : 0x82C;		/* delete if don't change mouse location */
extern char keyMap[8] : 0x174;		/* delete if don't check keyboard */

Rect			gMainScreenBounds;	/* delete if module doesn't depend on screen bounds */
unsigned long	me;					/* delete if module has no other global variables */

void header(void);					/* delete if don't use global variables */
void main(void);					/* always need :) */

void header(void)					/* delete function if don't use global variables */
{
	asm
	{
		dc.l	0					/* a place to store A4 (to access global variables) */
		move.l a0, d0
		lea header, a0
		jmp main
	}
}

#include "SetUpA4.h"				/* delete if don't use global variables */

void main(void)						/* the main thing -- always need */
{
	VBLTask*		myVBL;			/* pointer to VBL structure -- always need */
	long			gestalt_temp;	/* delete if don't check environment */
	OSErr			isHuman;		/* delete if don't check environment */
	
	RememberA0();					/* delete if don't use global variables */
	SetUpA4();						/* delete if don't use global variables */
	
	asm								/* move VBL structure pointer into local variable */
	{								/* always need this so we can set vblCount later */
		move.l d0, myVBL			/* (see below) */
	}
	
	if (me != 'MMdl')				/* delete "if" statement if don't need initialization */
	{
		isHuman = Gestalt(gestaltQuickdrawVersion, &gestalt_temp);	/* Quickdraw version? */
		gMainScreenBounds=(isHuman || (gestalt_temp < gestalt8BitQD)) ? screenBits.bounds :
			(**GetMainDevice()).gdRect;	/* delete if don't need screen bounds */
		me = 'MMdl';				/* need so we don't re-initialize */
	}
	
	/* put lots o' neat stuff here */
	
	CrsrNew = TRUE;					/* to redraw mouse; use only if mouse location changes */
	myVBL->vblCount = 1;			/* how many ticks until this code gets called again */
	RestoreA4();					/* delete if don't use global variables */
}
