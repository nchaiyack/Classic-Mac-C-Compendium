/**********************************************************************\

File:		msg about.c

Purpose:	This module handles the about boxes and that funky animation
			in the "About MSG" about box.


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

#include "msg about.h"
#include "msg graphics.h"
#include "msg prefs.h"

extern Point RawMouse : 0x82C;

PicHandle		gAboutColorPict;
PicHandle		gAboutBWPict;

void DoTheMSGThing(void)
{
	/* assumes "About MSG" window (gTheWindow[kAboutMSG]) is the front window */
	
	Rect			sourceRect, destRect;
	Rect			mainRect;
	long			lr,tb;
	int				l,t;
	
	mainRect=gMainScreenBounds;
	mainRect.top-=MBarHeight;
	if (PtInRect(RawMouse, &mainRect))
	{
		mainRect=gMainScreenBounds;
		lr=RawMouse.h-mainRect.left;
		lr*=81;
		lr/=mainRect.right-mainRect.left;
		tb=RawMouse.v-mainRect.top;
		tb*=81;
		tb/=mainRect.bottom-mainRect.top;
		if (tb<0)
			tb=0;
		l=RawMouse.h-lr;
		t=RawMouse.v-tb;
		SetRect(&sourceRect, l, t, l+81, t+81);
		SetRect(&destRect, 81, 81, 162, 162);
		CopyBits(&(WMgrPort->portBits), &(gTheWindow[kAboutMSG]->portBits),
			&sourceRect, &destRect, 0, 0L);
	}
}

void ShowInformation(void)
{
	OpenTheWindow(kAbout);
}

void ShowSplashScreen(void)
{
	OpenTheWindow(kAboutMSG);
}

void DrawTheCarpet(void)
{
	int				row;
	Rect			sourceRect, destRect;
	GrafPtr			curPort;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	
	DrawCarpet(9,234,3);
	SetRect(&sourceRect, 0, 216, 27, 243);
	destRect=sourceRect;
	OffsetRect(&destRect, 27, 0);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	OffsetRect(&destRect, -27, -27);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	OffsetRect(&destRect, 0, -27);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	OffsetRect(&destRect, 27, 0);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	SetRect(&sourceRect, 0, 162, 27, 243);
	destRect=sourceRect;
	OffsetRect(&destRect, 54, 0);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	SetRect(&sourceRect, 0, 162, 81, 243);
	destRect=sourceRect;
	OffsetRect(&destRect, 81, 0);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	OffsetRect(&destRect, -81, -81);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	OffsetRect(&destRect, 0, -81);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	OffsetRect(&destRect, 81, 0);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	SetRect(&sourceRect, 0, 0, 81, 243);
	destRect=sourceRect;
	OffsetRect(&destRect, 162, 0);
	CopyBits(&(curPort->portBits), &(curPort->portBits),
		&sourceRect, &destRect, 0, 0L);
	
	TextFont(geneva);
	TextSize(9);
	TextMode(srcXor);
	row=92;
	DrawTheString("\pMerriMac", &row);
	DrawTheString("\pSoftware", &row);
	DrawTheString("\pGroup �94", &row);
	DrawTheString("\p", &row);
	DrawTheString("\pStill enhancing", &row);
	DrawTheString("\pthe flavor of", &row);
	DrawTheString("\pyour Macintosh.", &row);
}

void DrawTheString(Str255 theString, int* theRow)
{
	MoveTo((gWindowWidth[kAboutMSG]-StringWidth(theString))/2, *theRow);
	DrawString(theString);
	*theRow+=11;
}

void DrawCarpet(int x, int y, int len)
{
	Rect			box;
	int				iter;

	x-=len*2;
	y+=len*2;
	for (iter=0; iter<8; iter++)
	{
		box.left=x-len;
		box.right=x+2*len;
		box.top=y-2*len;
		box.bottom=y+len;
		FillRect(&box, black);
		if (len>1) DrawCarpet(x,y,len/3);
		box.left=x;
		box.right=x+len;
		box.bottom=y;
		box.top=y-len;
		FillRect(&box, white);
		switch (iter)
		{
			case 0:
			case 1: x+=len*3; break;
			case 2:
			case 3: y-=len*3; break;
			case 4:
			case 5: x-=len*3; break;
			case 6:
			case 7: y+=len*3; break;
		}
	}
}

void DrawTheAboutBox(Boolean isColor)
{
	int				row;
	Handle			textHandle;
	Str255			theLine;
	unsigned long	pos;
	unsigned long	theSize;
	GrafPtr			curPort;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	DrawThePicture(isColor ? &gAboutColorPict : &gAboutBWPict, isColor ? kAboutColorID :
		kAboutBWID, 0, 0);
	
	TextFont(geneva);
	TextSize(9);
	TextMode(srcCopy);
	if (isColor)
		ForeColor(blueColor);
	row=16;
	textHandle=GetResource('TEXT', 128);
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
		GetTheNextLine(textHandle, theSize, &pos, theLine);
		DrawTheAboutString(theLine, &row);
	}
	while (pos<theSize);
	ReleaseResource(textHandle);
	if (isColor)
		ForeColor(redColor);
	DrawTheAboutString(gMyName, &row);
	DrawTheAboutString(gMyOrg, &row);
	if (isColor)
		ForeColor(blackColor);
}

void GetTheNextLine(Handle textHandle, unsigned long theSize, unsigned long *pos, Str255 theLine)
{
	unsigned char	theChar;
	
	theLine[0]=0x00;
	while ((*pos<theSize) && ((theChar=*((unsigned char*)(((long)(*textHandle))+((*pos)++))))!=0x0d))
		theLine[++theLine[0]]=theChar;
}

void DrawTheAboutString(Str255 theString, int *theRow)
{
	MoveTo(160-StringWidth(theString)/2, *theRow);
	DrawString(theString);
	*theRow+=12;
}
