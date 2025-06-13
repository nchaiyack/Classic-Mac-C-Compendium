/**********************************************************************\

File:		msg dialogs.c

Purpose:	This module handles positioning the error alert (or anything
			else, but that's all we use it for in this control panel)
			in accordance with Human Interface Guidelines.
			

Mousebroken -=- your computer isn't truly broken until it's mousebroken
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
#include "cdev globals.h"
#include "GestaltEQU.h"

void	PositionDialog(ResType theType, short theID)
{
	Handle				theTemplate;	/* Handle to resource template		*/
	long				oldA5;
	QDGlobals			qd;
	GrafPort			gp;
	GrafPtr				port;
	GrafPtr				savePort;
	register Rect		*theRect;		/* Bounding box of dialog			*/
	register short		left;			/* Left side of centered rect		*/
	register short		top;			/* Top side of centered rect		*/
	
	GetPort(&savePort);
	oldA5 = SetA5((long)&qd.end);
	InitGraf(&qd.thePort);
	OpenPort(&gp);
	SetCursor(&qd.arrow);
	
	theTemplate = GetResource(theType, theID);
	if(theTemplate == 0)
		return;
	theRect = (Rect*) *theTemplate;
	
										/* Center horizontally on screen	*/
	left = (qd.screenBits.bounds.right - (theRect->right - theRect->left)) / 2;

										/* Leave twice as much space below	*/
										/*   as above the rectangle			*/	
	top = (qd.screenBits.bounds.bottom - (theRect->bottom - theRect->top)) / 3;
										/* Don't put rect under menu bar	*/
	if(top < (GetMBarHeight() + 1))
		top = GetMBarHeight() + 1;

	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;

	ClosePort(&gp);
	SetA5(oldA5);
	SetPort(savePort);
}
