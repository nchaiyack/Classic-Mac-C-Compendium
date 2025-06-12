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

#include "graphics.h"		/* needs to come first because it defines WindowDataHandle */
#include "about.h"
#include "prefs.h"
#include "util.h"

/*-----------------------------------------------------------------------------------*/
/* internal stuff for about.c                                                        */

void SetupTheAboutBox(WindowDataHandle theData);
void OpenTheAboutBox(WindowDataHandle theData);
void ShutdownTheAboutBox(WindowDataHandle theData);
void DrawTheAboutBox(Boolean isColor);
void GetTheNextLine(Handle textHandle, unsigned long theSize, unsigned long *pos, Str255 theLine);
void DrawTheAboutString(Str255 theString, int *theRow);

PicHandle		gAboutColorPict;
PicHandle		gAboutBWPict;
Boolean			gUseAlternate;

enum
{
	kAboutColorID=134,
	kAboutBWID
};

int AboutBoxDispatch(ExtendedWindowDataHandle theData, int theMessage, unsigned long misc)
{
	int				theDepth;
	
	switch (theMessage)	/* see graphics.h for list of messages*/
	{
		case kKeydown:							/* close about box on keypress */
		case kMousedown:						/* or mouseclick */
			CloseTheWindow(theData);
			return kSuccess;
			break;
		case kUpdate:
			theDepth=misc&0x7fff;				/* pixel depth */
			DrawTheAboutBox((theDepth>2));		/* we only care if it's color or not */
			return kSuccess;
			break;
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
	unsigned char	*theTitle="\pAbout MSG Demo";
	
	(**theData).windowWidth=380;
	(**theData).windowHeight=216;
	(**theData).windowType=noGrowDocProc;	/* plain vanilla window */
	Mymemcpy((**theData).windowTitle, theTitle, theTitle[0]+1);
	(**theData).hasCloseBox=TRUE;
	gAboutColorPict=gAboutBWPict=0L;
	gUseAlternate=FALSE;
}

void OpenTheAboutBox(WindowDataHandle theData)
{
	unsigned int	theKeys[8];
	
	GetKeys(&theKeys);
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
	if (ReleaseThePict(gAboutColorPict))
		gAboutColorPict=0L;
	if (ReleaseThePict(gAboutBWPict))
		gAboutBWPict=0L;
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
	MoveTo(293-StringWidth(theString)/2, *theRow);
	DrawString(theString);
	*theRow+=12;
}
