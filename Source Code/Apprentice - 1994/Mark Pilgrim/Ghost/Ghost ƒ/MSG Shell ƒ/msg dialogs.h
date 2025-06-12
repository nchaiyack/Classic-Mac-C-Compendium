/**********************************************************************\

File:		msg dialogs.h

Purpose:	This is the header file for msg dialogs.c


Ghost -=- a classic word-building challenge
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

enum
{
	colorSplashDialog = 128,
	bwSplashDialog,
	standardAlert,
	colorPersonalDialog,
	bwPersonalDialog,
	generalAlert,
	colorInfoDialog,
	bwInfoDialog,
	
	chooseIconDialog=200,
	addPlayerDialog,
	
	integrityCheckFailAlert=307,
	
	prefsErrorAlert = 400
};

void		PositionDialog(ResType theType, short theID);
pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum);
