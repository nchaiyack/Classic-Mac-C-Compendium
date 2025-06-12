/**********************************************************************\

File:		dialectic graphics.c

Purpose:	This module handles drawing the clipboard contents into
			a window.


Dialectic -=- dialect text conversion extraordinare
Copyright ©1994, Mark Pilgrim

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

#include "graphics.h"
#include "environment.h"
#include "dialectic graphics.h"
#include "util.h"

/* internal procedures for dialectic graphics.c only */
void SetupTheClipboardWindow(WindowDataHandle theData);
void InitializeTheClipboardWindow(WindowDataHandle theData);
void OpenTheClipboardWindow(WindowDataHandle theData);
void SuspendTheClipboardWindow(WindowDataHandle theData);
void ResumeTheClipboardWindow(WindowDataHandle theData);
void DrawTheClipboardWindow(void);

int ClipboardWindowDispatch(ExtendedWindowDataHandle theData, int theMessage, unsigned long misc)
{
	switch (theMessage)
	{
		case kUpdate:
			DrawTheClipboardWindow();
			return kSuccess;
			break;
		case kInitialize:
			InitializeTheClipboardWindow(theData);
			return kSuccess;
		case kOpen:
			OpenTheClipboardWindow(theData);
			return kSuccess;
			break;
		case kSuspend:
			SuspendTheClipboardWindow(theData);
			return kSuccess;
			break;
		case kResume:
			ResumeTheClipboardWindow(theData);
			return kSuccess;
			break;
		case kStartup:
			SetupTheClipboardWindow(theData);
			return kSuccess;
			break;
	}
	
	return kFailure;
}

void SetupTheClipboardWindow(WindowDataHandle theData)
{
	unsigned char	*titleStr="\pClipboard";
	
	(**theData).windowWidth=350;
	(**theData).windowHeight=200;
	(**theData).windowBounds.top=
		screenBits.bounds.bottom-(**theData).windowHeight-10;
	(**theData).windowBounds.left=10;
	(**theData).windowType=noGrowDocProc;
	(**theData).hasCloseBox=TRUE;
	Mymemcpy((**theData).windowTitle, titleStr, titleStr[0]+1);
}

void InitializeTheClipboardWindow(WindowDataHandle theData)
{
	(**theData).initialTopLeft.v=(**theData).windowBounds.top-9;
	(**theData).initialTopLeft.h=(**theData).windowBounds.left;
}

void OpenTheClipboardWindow(WindowDataHandle theData)
{
	(**theData).offscreenNeedsUpdate=TRUE;
}

void SuspendTheClipboardWindow(WindowDataHandle theData)
{
	HideWindow(gTheWindow[(**theData).windowIndex]);
}

void ResumeTheClipboardWindow(WindowDataHandle theData)
{
	OpenTheWindow(kClipboard);
}

void DrawTheClipboardWindow(void)
{
	GrafPtr			curPort;
	Handle			inputHandle;
	unsigned long	i;
	unsigned long	theSize;
	int				col, row;
	Boolean			notDoneYet;
	unsigned char	theChar;
	int				w, h;
	
	GetPort(&curPort);
	
	EraseRect(&(curPort->portRect));
	w=curPort->portRect.right-curPort->portRect.left;
	h=curPort->portRect.bottom-curPort->portRect.top;
	
	LoadScrap();
	inputHandle=NewHandle(0L);
	if ((theSize=GetScrap(inputHandle, 'TEXT', &i))==noTypeErr)
		return;
	
	TextFont(monaco);
	TextSize(9);
	
	i=0L;
	col=5;
	row=12;
	notDoneYet=TRUE;
	MoveTo(col, row);
	HLock(inputHandle);
	while ((i<theSize) && (notDoneYet))
	{
		theChar=*(unsigned char*)((unsigned long)(*inputHandle)+(i++));
		DrawChar(theChar);
		col+=CharWidth(theChar);
		if ((col>w-5) || (theChar==0x0d))
		{
			col=5;
			row+=12;
			MoveTo(col, row);
			notDoneYet=(row<h-5);
		}
	}
	
	DisposeHandle(inputHandle);
}
