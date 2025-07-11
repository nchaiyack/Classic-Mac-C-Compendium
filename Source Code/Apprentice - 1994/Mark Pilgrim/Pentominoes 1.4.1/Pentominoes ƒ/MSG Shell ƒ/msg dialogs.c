/**********************************************************************\

File:		msg dialogs.c

Purpose:	This module handles positioning a dialog on the screen as
			per Human Interface Guidelines.  Also, a quick-and-dirty
			default button outline.


Pentominoes - a 2-D geometry board game
Copyright (C) 1993 Mark Pilgrim

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

#include "msg dialogs.h"

/******************************************************************************
 PositionDialog
 
		Center the bounding box of a dialog or alert in the upper third
		of the screen.  This is the preferred location according to the
		Human Interface Guidelines.
 ******************************************************************************/

void	PositionDialog(ResType theType, short theID)
{
	Handle				theTemplate;	/* Handle to resource template		*/
	register Rect		*theRect;		/* Bounding box of dialog			*/
	register short		left;			/* Left side of centered rect		*/
	register short		top;			/* Top side of centered rect		*/
	
		/* The first field of the resource template for DLOG's and ALRT's */
		/* is its bounding box.  Get a pointer to this rectangle.  This   */
		/* handle dereferencing is safe since the remaining statements in */
		/* this function do not move memory (assignment and simple math). */

	theTemplate = GetResource(theType, theID);
	if(theTemplate == 0)
		return;
	theRect = (Rect*) *theTemplate;
	
										/* Center horizontally on screen	*/
	left = (screenBits.bounds.right - (theRect->right - theRect->left)) / 2;

										/* Leave twice as much space below	*/
										/*   as above the rectangle			*/	
	top = (screenBits.bounds.bottom - (theRect->bottom - theRect->top)) / 3;
										/* Don't put rect under menu bar	*/
	if(top < (GetMBarHeight() + 1))
		top = GetMBarHeight() + 1;

	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;
}

pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum)
{
	short	itemType;
	Handle	itemH;
	Rect	box;
	
	GetDItem(myDlog, 1, &itemType, &itemH, &box);
	PenSize(3, 3);
	InsetRect(&box, -4, -4);
	FrameRoundRect(&box, 16, 16);
	PenNormal();
}
