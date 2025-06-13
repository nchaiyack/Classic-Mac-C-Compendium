/******************************************************************************\

File:		dialutil.c

Purpose:	This module provides routines useful for dialog processing.
			

``Tetris Light'' - a simple implementation of a Tetris game.
Copyright (C) 1993 Hoylen Sue

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; see the file COPYING.  If not, write to the
Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.

\******************************************************************************/

#include "local.h"
#include "dialutil.h"

/*--------------------------------------------------------------------*/

static pascal void box_button(WindowPtr wind, INTEGER itemno)
/* This is the display routine for the userItem which draws a solid
   outline around the default button. */
{
	INTEGER	dummy;
	Rect	rectangle;
	Handle	handle;
	
	GetDItem(wind, itemno, &dummy, &handle, &rectangle);
	PenSize(3, 3);
	FrameRoundRect(&rectangle, 16, 16);
}

/*--------------------------------------------------------------------*/

void install_hilight_button(DialogPtr dp, INTEGER button_item, INTEGER usr_item)
/* This routine sets the given 'usr_item' to be a userItem whose role is
   to draw the highlighting box around the default button in a dialog
   box.  The rectangle of this user item is set to be around the 'button_item'
   of the given dialog pointed to by 'dp'. */
{
	INTEGER	type;
	Handle	hand;
	Rect	rect;
	
	GetDItem(dp, button_item, &type, &hand, &rect);
	InsetRect(&rect, -4, -4);
	SetDItem(dp, usr_item, userItem + itemDisable, box_button, &rect );
}

/*--------------------------------------------------------------------*/

void simulate_key_hit(DialogPtr dp, INTEGER button_item)
/* Flashes a push button control from the given dialog. This is a 
   commonly used routine for indicating to the user that the button
   has been hit when they used the keyboard equivalent for it. */
{
	LONGINT final;
	Handle h;
	Rect box;
	INTEGER dummy;
	
	GetDItem(dp, button_item, &dummy, &h, &box);
	
	HiliteControl(h, 1);
	Delay(8, &final);		/* 8 ticks is sufficient */
	HiliteControl(h, 0);
}

/*--------------------------------------------------------------------*/
