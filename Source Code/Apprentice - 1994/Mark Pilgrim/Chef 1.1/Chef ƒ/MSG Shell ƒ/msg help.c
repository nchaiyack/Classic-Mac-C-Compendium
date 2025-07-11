/**********************************************************************\

File:		msg help.c

Purpose:	This module handles displaying the different help windows.


Chef -=- convert text to Swedish chef talk
Copyright �1994, Mark Pilgrim

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

#include "msg help.h"
#include "msg graphics.h"
#include "msg menus.h"
#include "program globals.h"

int				gWhichHelp;
Handle			gHelpIcon[4];

void InitHelp(void)
{
	int				i;
	
	for (i=0; i<4; i++)
		gHelpIcon[i]=GetIcon(128+i);
}

void DrawTheHelp(Boolean isColor)
{
	GrafPtr			curPort;
	int				row;
	Handle			textHandle;
	Str255			theLine;
	unsigned long	pos;
	unsigned long	theSize;
	unsigned char	theChar;
	Rect			iconRect;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));

	TextFont(geneva);
	TextSize(9);
	row=63;
	textHandle=GetResource('TEXT', 399+gWhichHelp);
	if (textHandle==0L)
		return;
	if (*textHandle==0L)
		LoadResource(textHandle);
	if (*textHandle==0L)
		return;
	pos=0L;
	theSize=SizeResource(textHandle);
	do
	{
		theLine[0]=0x00;
		while ((pos<theSize) && ((theChar=*((unsigned char*)(((long)(*textHandle))+(pos++))))!=0x0d))
		{
			if (theChar=='^')
				AddNameToString(theLine, APPLICATION_NAME);
			else
			theLine[++theLine[0]]=theChar;
		}
		MoveTo(8, row);
		DrawString(theLine);
		row+=12;
	}
	while (pos<theSize);
	ReleaseResource(textHandle);
	
	MoveTo(0, 46);
	Line(300, 0);
	
	SetRect(&iconRect, 20, 7, 52, 39);
	PlotIcon(&iconRect, gHelpIcon[0]);
	OffsetRect(&iconRect, 228, 0);
	PlotIcon(&iconRect, gHelpIcon[1]);
	
	GetItem(gHelpMenu, gWhichHelp, theLine);
	TextFace(bold);
	MoveTo(150-StringWidth(theLine)/2, 26);
	DrawString(theLine);
	TextFace(0);
}

void AddNameToString(Str255 theLine, Str255 theStringToAdd)
{
	int			i;
	
	for (i=1; i<=theStringToAdd[0]; i++)
		theLine[++theLine[0]]=theStringToAdd[i];
}

void HelpEvent(void)
{
	Point		mouseLoc;
	Rect		iconRect;
	Boolean		isHilited;
	
	GetMouse(&mouseLoc);
	SetRect(&iconRect, 20, 7, 52, 39);
	if (PtInRect(mouseLoc, &iconRect))
	{
		isHilited=FALSE;
		while (StillDown())
		{
			GetMouse(&mouseLoc);
			if (PtInRect(mouseLoc, &iconRect))
			{
				if (!isHilited)
					PlotIcon(&iconRect, gHelpIcon[2]);
				isHilited=TRUE;
			}
			else
			{
				if (isHilited)
					PlotIcon(&iconRect, gHelpIcon[0]);
				isHilited=FALSE;
			}
		}
		if (isHilited)
		{
			gWhichHelp--;
			if (gWhichHelp==0)
				gWhichHelp=gNumHelp;
			UpdateHelpWindow();
		}
	}
	else
	{
		OffsetRect(&iconRect, 228, 0);
		if (PtInRect(mouseLoc, &iconRect))
		{
			isHilited=FALSE;
			while (StillDown())
			{
				GetMouse(&mouseLoc);
				if (PtInRect(mouseLoc, &iconRect))
				{
					if (!isHilited)
						PlotIcon(&iconRect, gHelpIcon[3]);
					isHilited=TRUE;
				}
				else
				{
					if (isHilited)
						PlotIcon(&iconRect, gHelpIcon[1]);
					isHilited=FALSE;
				}
			}
			if (isHilited)
			{
				gWhichHelp++;
				if (gWhichHelp>gNumHelp)
					gWhichHelp=1;
				UpdateHelpWindow();
			}
		}
	}
}

void HelpKeyEvent(char keyPressed)
{
	switch (keyPressed)
	{
		case 0x1c:
			gWhichHelp--;
			if (gWhichHelp==0)
				gWhichHelp=gNumHelp;
			UpdateHelpWindow();
			break;
		case 0x1d:
			gWhichHelp++;
			if (gWhichHelp>gNumHelp)
				gWhichHelp=1;
			UpdateHelpWindow();
			break;
	}
}

void ShutDownHelp(void)
{
	int			i;
	
//	for (i=0; i<4; i++)
//		ReleaseResource(gHelpIcon[i]);
}
