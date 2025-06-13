/**********************************************************************\

File:		dialogs.c

Purpose:	This module handles positioning a dialog on the screen as
			per Human Interface Guidelines.  Also, a quick-and-dirty
			default button outline, and a proc filter for dialogs to
			map RETURN or ENTER to button 1 and ESCAPE or COMMAND-PERIOD
			to button 2.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program in a file named "GNU General Public License".
If not, write to the Free Software Foundation, 675 Mass Ave,
Cambridge, MA 02139, USA.

\**********************************************************************/

#include "dialogs.h"

void PositionDialog(ResType theType, short theID)
/* call this function BEFORE loading an alert or dialog box; this function will
   load the alert/dialog into memory and position it centered horizontally and
   positioned 1:3 vertically.  Then, load the alert/dialog right after the call
   to PositionDialog, and the alert/dialog will be loaded from the copy in
   memory, which has already been positioned correctly.  Cute, eh?  See error.c
   for an example. */
{
	Handle				theTemplate;	/* Handle to resource template	*/
	register Rect		*theRect;		/* Bounding box of dialog		*/
	register short		left;			/* Left side of centered rect	*/
	register short		top;			/* Top side of centered rect	*/
	
	/* The first field of the resource template for DLOG's and ALRT's */
	/* is its bounding box.  Get a pointer to this rectangle.  This   */
	/* handle dereferencing is safe since the remaining statements in */
	/* this function do not move memory (assignment and simple math). */

	theTemplate = GetResource(theType, theID);
	if (theTemplate == 0)
		return;
	theRect=(Rect*)*theTemplate;	/* bounding rectangle */
	
	left = (screenBits.bounds.right - (theRect->right - theRect->left)) / 2;
	top = (screenBits.bounds.bottom - (theRect->bottom - theRect->top)) / 3;
	if (top < (GetMBarHeight() + 1))	/* don't put it over menu bar */
		top = GetMBarHeight() + 1;

	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;
}

pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum)
/* Use this as the useritem for a dialog which needs an outlined default button. */
/* Make sure the default button is item 1 in the DITL. */
{
	short			itemType;
	Handle			itemH;
	Rect			box;
	
	GetDItem(myDlog, 1, &itemType, &itemH, &box);
	PenSize(3, 3);
	InsetRect(&box, -4, -4);
	FrameRoundRect(&box, 16, 16);
	PenNormal();
}

pascal Boolean ProcOFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
/* use this as the proc filter when calling ModalDialog -- it maps RETURN and
   ENTER to button 1, and ESCAPE and COMMAND-PERIOD to button 2.  Of course,
   button 1 should be the OK button and button 2 should be the cancel button. */
{
	unsigned char	theChar;
	short			itemType;
	Handle			itemH;
	Rect			box;
	unsigned long	dummy;
	
	switch (theEvent->what)	/* examine event record */
	{
		case keyDown:	/* keypress */
		case autoKey:
			theChar=theEvent->message & charCodeMask;	/* get ascii char value */
			if ((theChar==0x0d) || (theChar==0x03))		/* RETURN or ENTER */
			{
				*theItem=1;		/* as if the user selected item #1 */
				GetDItem(theDialog, 1, &itemType, &itemH, &box);
				HiliteControl((ControlHandle)itemH, 1);	/* flash button 1 by highlighting, */
				Delay(8, &dummy);						/* waiting 8 ticks, and then */
				HiliteControl((ControlHandle)itemH, 0);	/* unhighlighting -- believe */
				return TRUE;							/* it or not, that's Apple's */
			}											/* preferred method */
			if ((theChar==0x1b) ||	/* ESCAPE */
				((theEvent->modifiers & cmdKey) && (theChar=='.')))	/* COMMAND-PERIOD */
			{
				*theItem=2;		/* as if the user selected item #2 */
				GetDItem(theDialog, 2, &itemType, &itemH, &box);	/* same as above */
				HiliteControl((ControlHandle)itemH, 1);
				Delay(8, &dummy);
				HiliteControl((ControlHandle)itemH, 0);
				return TRUE;
			}
			break;
	}
	
	return FALSE;	/* no faking, proceed as planned */
}
