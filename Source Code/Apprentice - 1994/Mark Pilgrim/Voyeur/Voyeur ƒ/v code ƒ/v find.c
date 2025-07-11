/**********************************************************************\

File:		v find.c

Purpose:	This module handles finding text in a fork, and then
			finding it again.


Voyeur -- a no-frills file viewer
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

#include "v find.h"
#include "v structs.h"
#include "v hex.h"
#include "v file management.h"
#include "v error.h"
#include "msg graphics.h"
#include "msg dialogs.h"

Str255			findString;
int				gBufferOffset;

Boolean DoFindAgain(void)
{
	int				charnum;
	unsigned long	tempoff;
	Boolean			found;
	int				bufferoff;
	
	tempoff=offset[forknum]+gBufferOffset;
	bufferoff=gBufferOffset;
	charnum=1;
	found=FALSE;
	while ((!found) && (tempoff<totallen[forknum]))
	{
		if (buffer[bufferoff]==findString[charnum])
		{
			charnum++;
			if (charnum>findString[0])
				found=TRUE;
		}
		else charnum=1;
		
		if (!found)
		{
			tempoff++;
			bufferoff++;
			if (bufferoff==256)
			{
				GetBuffer(fileRefNum, tempoff, buffer);
				bufferoff=0;
			}
		}
	}
	
	if (found)
	{
		gBufferOffset=bufferoff;
		if (tempoff%256)
			offset[forknum]=(tempoff/256)*256;
		else
			offset[forknum]=tempoff;
		GetBuffer(fileRefNum, offset[forknum], buffer);
		UpdateBoard();
		return TRUE;
	}
	else
	{
		HandleError(patternNotFoundErr);
		GetBuffer(fileRefNum, offset[forknum], buffer);
		return FALSE;
	}
}

Boolean DoFind(void)
{
	DialogPtr	theDlog;
	int			itemSelected;
	Handle		itemH;
	short		item,itemType;
	Rect		box;
	
	PositionDialog('DLOG', findDialog);
	theDlog = GetNewDialog(findDialog, 0L, (WindowPtr)-1L);

	GetDItem(theDlog, 5, &itemType, &itemH, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDlog, 5, itemType, (Handle)OutlineDefaultButton, &box);
	
	GetDItem(theDlog,4,&itemType,&itemH,&box);
	SetIText(itemH, findString);
	SelIText(theDlog, 4, 0, 32767);
	SetWTitle((WindowPtr)theDlog, "\pFind");
	
	ShowWindow(theDlog);
	
	itemSelected=0;
	while ((itemSelected!=1) && (itemSelected!=2))
	{
		ModalDialog(0L, &itemSelected);
	}
	
	if (itemSelected==1)
	{
		GetDItem(theDlog,4,&itemType,&itemH,&box);
		GetIText(itemH, findString);
		if (findString[0]==0x00)
			itemSelected=2;
	}

	HideWindow(theDlog);
	DisposeDialog(theDlog);

	if (itemSelected==1)
		return DoFindAgain();
	return FALSE;
}
