/**********************************************************************\

File:		v meat.c

Purpose:	This module handles the options under the Options menu:
			going forward one block, going backward one block, etc.
			These procedures actually change the internal variables
			that tell us what block we're on and then update the
			screen accordingly.  Also, changing the fork EOF is
			handled here.


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

#include "v meat.h"
#include "v structs.h"
#include "v hex.h"
#include "v file management.h"
#include "v error.h"
#include "msg graphics.h"
#include "msg dialogs.h"

void ChangeEOF(void)
{
	DialogPtr		theDlog;
	int				itemSelected;
	Handle			itemH;
	short			item,itemType;
	Rect			box;
	Str255			tempStr;
	unsigned long	temptotal;
	OSErr			isHuman;
	
	PositionDialog('DLOG', eofDialog);
	theDlog = GetNewDialog(eofDialog, 0L, (WindowPtr)-1L);

	GetDItem(theDlog, 5, &itemType, &itemH, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDlog, 5, itemType, (Handle)OutlineDefaultButton, &box);
	
	GetDItem(theDlog,4,&itemType,&itemH,&box);
	LongToHexString(totallen[forknum], tempStr);
	SetIText(itemH, tempStr);
	SelIText(theDlog, 4, 0, 32767);
	SetWTitle((WindowPtr)theDlog, "\pSet EOF");
	
	ShowWindow(theDlog);
	
	itemSelected=0;
	while ((itemSelected!=1) && (itemSelected!=2))
	{
		ModalDialog(0L, &itemSelected);
	}
	
	if (itemSelected==1)
	{
		GetDItem(theDlog,4,&itemType,&itemH,&box);
		GetIText(itemH, tempStr);
		if (tempStr[0]==0x00)
			itemSelected=2;
	}

	HideWindow(theDlog);
	DisposeDialog(theDlog);

	if (itemSelected==1)
	{
		if (!ValidHex(tempStr))
			HandleError(invalidHexErr);
		else if (tempStr[0]>8)
			HandleError(offsetTooLargeErr);
		else
		{
			temptotal=HexStringToLong(tempStr);
			isHuman=SetEOF(fileRefNum, temptotal);
			if (isHuman!=noErr)
			{
				SetEOF(fileRefNum, totallen[forknum]);
				HandleError(diskFullErr);
			}
			else
			{
				totallen[forknum]=temptotal;
				if (offset[forknum]+gBufferOffset>=temptotal)
				{
					if (temptotal%256)
					{
						offset[forknum]=(temptotal/256)*256;
						gBufferOffset=temptotal-offset[forknum];
					}
					else
					{
						offset[forknum]=(temptotal==0L) ? 0 : temptotal-256;
						gBufferOffset=0;
					}
				}
				GetBuffer(fileRefNum, offset[forknum], buffer);
				UpdateBoard();
			}
		}
	}
}

void DoDataFork(void)
{
	int			resultCode;
	
	if (forknum==1)
	{
		CloseTheFile(fileRefNum);
		forknum=0;
		gBufferOffset=0;
		resultCode=OpenTheDataFork(&editFile, &fileRefNum);
		
		if (resultCode==allsWell)
			resultCode=GetBuffer(fileRefNum, offset[forknum], buffer);

		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			CloseMainWindow();
		}
		else
			UpdateBoard();
	}
}

void DoResourceFork(void)
{
	int			resultCode;
	
	if (forknum==0)
	{
		CloseTheFile(fileRefNum);
		forknum=1;
		gBufferOffset=0;
		resultCode=OpenTheResourceFork(&editFile, &fileRefNum);

		if (resultCode==allsWell)
			resultCode=GetBuffer(fileRefNum, offset[forknum], buffer);
		
		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			CloseMainWindow();
		}
		else
			UpdateBoard();
	}
}

void GoForward(void)
{
	int			resultCode;
	
	if (offset[forknum]+256L<totallen[forknum])
	{
		offset[forknum]+=256L;
		gBufferOffset=0;
		resultCode=GetBuffer(fileRefNum, offset[forknum], buffer);
		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			CloseMainWindow();
		}
		
		UpdateBoard();
	}
	else SysBeep(7);
}

void GoBack(void)
{
	int			resultCode;
	
	if (offset[forknum]>=256L)
	{
		offset[forknum]-=256L;
		gBufferOffset=0;
		resultCode=GetBuffer(fileRefNum, offset[forknum], buffer);
		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			CloseMainWindow();
		}
		else
			UpdateBoard();
	}
	else SysBeep(7);
}

void GoBeginning(void)
{
	int			resultCode;
	
	if (offset[forknum]!=0)
	{
		offset[forknum]=0L;
		gBufferOffset=0;
		resultCode=GetBuffer(fileRefNum, offset[forknum], buffer);
		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			CloseMainWindow();
		}
		else
			UpdateBoard();
	}
}

void GoEnd(void)
{
	int				resultCode;
	unsigned long	temp;
	
	if (totallen[forknum]%256)
		temp=(totallen[forknum]/256)*256;
	else
		temp=totallen[forknum]-256;
	
	if (offset[forknum]!=temp)
	{
		offset[forknum]=temp;
		gBufferOffset=0;
		resultCode=GetBuffer(fileRefNum, offset[forknum], buffer);
		if (resultCode!=allsWell)
		{
			HandleError(resultCode);
			CloseMainWindow();
		}
		else
			UpdateBoard();
	}
}

void GoOffset(void)
{
	DialogPtr		theDlog;
	int				itemSelected;
	Handle			itemH;
	short			item,itemType;
	Rect			box;
	Str255			tempStr;
	int				i;
	unsigned long	tempoff;
			
	PositionDialog('DLOG', offsetDialog);
	theDlog = GetNewDialog(offsetDialog, 0L, (WindowPtr)-1L);

	GetDItem(theDlog, 5, &itemType, &itemH, &box);
	InsetRect(&box, -4, -4);
	SetDItem(theDlog, 5, itemType, (Handle)OutlineDefaultButton, &box);
	
	GetDItem(theDlog,4,&itemType,&itemH,&box);
	SetIText(itemH, findString);
	SelIText(theDlog, 4, 0, 32767);
	SetWTitle((WindowPtr)theDlog, "\pGo to offset");
	
	ShowWindow(theDlog);
	
	itemSelected=0;
	while ((itemSelected!=1) && (itemSelected!=2))
	{
		ModalDialog(0L, &itemSelected);
	}
	
	if (itemSelected==1)
	{
		GetDItem(theDlog,4,&itemType,&itemH,&box);
		GetIText(itemH, tempStr);
		if (tempStr[0]==0x00)
			itemSelected=2;
	}

	HideWindow(theDlog);
	DisposeDialog(theDlog);

	if (itemSelected==1)
	{
		if (!ValidHex(tempStr))
			HandleError(invalidHexErr);
		else if (tempStr[0]>8)
			HandleError(offsetTooLargeErr);
		else
		{
			tempoff=HexStringToLong(tempStr);
			if (tempoff>=totallen[forknum])
				HandleError(offsetTooLargeErr);
			else
			{
				if (tempoff%256)
				{
					offset[forknum]=(tempoff/256)*256;
					gBufferOffset=tempoff-offset[forknum];
				}
				else
				{
					offset[forknum]=tempoff;
					gBufferOffset=0;
				}
				GetBuffer(fileRefNum, offset[forknum], buffer);
				UpdateBoard();
			}
		}
	}
}
