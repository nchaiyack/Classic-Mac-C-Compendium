/**********************************************************************\

File:		about.c

Purpose:	This module handles displaying the about box.

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

#include "about.h"
#include "prefs.h"
#include "util.h"

/*-----------------------------------------------------------------------------------*/
/* internal stuff for about.c                                                        */

void SetupTheAboutBox(WindowDataHandle theData);
void InitializeTheAboutBox(WindowDataHandle theData);
void OpenTheAboutBox(WindowDataHandle theData);
void ResizeTheAboutBox(WindowDataHandle theData);
void ShutdownTheAboutBox(WindowDataHandle theData);
void DrawTheAboutBox(Boolean isColor);
void GetTheNextLine(Handle textHandle, unsigned long theSize, unsigned long *pos, Str255 theLine);
void DrawTheAboutString(Str255 theString, short *theRow, short theCenter);

Boolean			gUseAlternate;

enum
{
	kAboutColorID=134
};

short AboutBoxDispatch(WindowDataHandle theData, short theMessage, unsigned long misc)
{
	short			theDepth;
	
	switch (theMessage)	/* see graphics.h for list of messages*/
	{
		case kKeydown:							/* close about box on keypress */
		case kMousedown:						/* or mouseclick */
			CloseTheWindow((ExtendedWindowDataHandle)theData);
			return kSuccess;
			break;
		case kUpdate:
			theDepth=misc&0x7fff;				/* pixel depth */
			DrawTheAboutBox((theDepth>2));		/* we only care if it's color or not */
			return kSuccess;
			break;
		case kInitialize:
			InitializeTheAboutBox(theData);
			return kFailure;
		case kOpen:
			OpenTheAboutBox(theData);
			return kSuccess;
			break;
		case kStartup:
			SetupTheAboutBox(theData);
			return kSuccess;
			break;
		case kShutdown:
			ShutdownTheAboutBox(theData);
			return kSuccess;
			break;
	}
	
	return kFailure;		/* for all other messages, defer to default processing */
}

void SetupTheAboutBox(WindowDataHandle theData)
{
	unsigned char	*theTitle="\pAbout Halma";
	
	(**theData).windowType=noGrowDocProc;	/* plain vanilla window */
	Mymemcpy((Ptr)((**theData).windowTitle), (Ptr)theTitle, theTitle[0]+1);
	(**theData).hasCloseBox=TRUE;
	(**theData).maxDepth=8;
	gUseAlternate=FALSE;
}

void InitializeTheAboutBox(WindowDataHandle theData)
{
	(**theData).windowWidth=170;
	(**theData).windowHeight=216;
}

void OpenTheAboutBox(WindowDataHandle theData)
{
	KeyMap			rawKeys;
	unsigned short	theKeys[8];
	
	GetKeys(rawKeys);
	Mymemcpy((Ptr)theKeys, (Ptr)rawKeys, sizeof(rawKeys));
	if (theKeys[3]&4)	/* option key down? */
	{
		if (!gUseAlternate)
		{
			gUseAlternate=TRUE;
			(**theData).offscreenNeedsUpdate=TRUE;
		}
	}
	else
	{
		if (gUseAlternate)
		{
			gUseAlternate=FALSE;
			(**theData).offscreenNeedsUpdate=TRUE;
		}
	}
}

void ShutdownTheAboutBox(WindowDataHandle theData)
{
}

void DrawTheAboutBox(Boolean isColor)
{
	RGBColor		oldForeColor, oldBackColor;
	short			row;
	Handle			textHandle;
	Str255			theLine;
	unsigned long	pos;
	unsigned long	theSize;
	GrafPtr			curPort;
	short			theCenter;
	RGBColor		myGray={49152, 49152, 49152};
	
	GetPort(&curPort);
	if (isColor)
	{
		GetForeColor(&oldForeColor);
		GetBackColor(&oldBackColor);
		RGBForeColor(&myGray);
		PaintRect(&(curPort->portRect));
		RGBForeColor(&oldForeColor);
		RGBBackColor(&oldBackColor);
		TextMode(srcOr);
	}
	else
	{
		FillRect(&(curPort->portRect), black);
		TextMode(srcXor);
	}
	
	theCenter=(curPort->portRect.right-curPort->portRect.left)/2;
	
	TextFont(geneva);
	TextSize(9);
	row=16;
	textHandle=GetResource('TEXT', gUseAlternate ? 129 : 128);
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
		DrawTheAboutString(theLine, &row, theCenter);
	}
	while (pos<theSize);
	ReleaseResource(textHandle);
	if (isColor)
		ForeColor(redColor);
	DrawTheAboutString(gMyName, &row, theCenter);
	DrawTheAboutString(gMyOrg, &row, theCenter);
	if (isColor)
		ForeColor(blackColor);
	TextMode(srcCopy);
	TextSize(12);
	TextFont(0);
}

void GetTheNextLine(Handle textHandle, unsigned long theSize, unsigned long *pos, Str255 theLine)
{
	unsigned char	theChar;
	
	theLine[0]=0x00;
	while ((*pos<theSize) && ((theChar=*((unsigned char*)(((long)(*textHandle))+((*pos)++))))!=0x0d))
		theLine[++theLine[0]]=theChar;
}

void DrawTheAboutString(Str255 theString, short *theRow, short theCenter)
{
	MoveTo(theCenter-StringWidth(theString)/2, *theRow);
	DrawString(theString);
	*theRow+=12;
}
