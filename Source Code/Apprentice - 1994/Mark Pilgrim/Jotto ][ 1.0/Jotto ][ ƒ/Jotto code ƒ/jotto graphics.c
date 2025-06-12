/**********************************************************************\

File:		jotto globals.c

Purpose:	This module handles Jotto-specific graphics (drawing to
			offscreen & onscreen bitmaps).


Jotto ][ -=- a simple word game, revisited
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

#include "jotto globals.h"
#include "jotto graphics.h"
#include "msg graphics.h"
#include "msg environment.h"
#include "util.h"

CIconHandle		gColorIcons[3];
Handle			gBWIcons[3];
PicHandle		gBackgroundPict;

void DrawBoardColor()
{
	RGBColor		oldForeColor, oldBackColor;
	GrafPtr			curPort;
	Rect			theRect;
	int				i;
	Str255			tempStr;
	char			oldChar;
	int				endi;
	unsigned char	*temp="\pYOU ARE A";
	unsigned char	*temp2="\pWINNER!";
	unsigned char	*temp3="\pClick the mouse button to continue";
	PicHandle		thePic;
	
	GetForeColor(&oldForeColor);
	GetBackColor(&oldBackColor);
	
	GetPort(&curPort);
	
	if (!gIsEndGame)
		DrawPicture(gBackgroundPict, &(curPort->portRect));
	else
	{
		thePic=GetPicture(134);
		FillRect(&(curPort->portRect), black);
		TextFont(144);
		TextSize(24);
		TextMode(srcXor);
		MoveTo(150-StringWidth(temp)/2, 140);
		DrawString(temp);
		SetRect(&theRect, 0, 150, 300, 220);
		DrawPicture(thePic, &theRect);
		MoveTo(150-StringWidth(temp2)/2, 250);
		DrawString(temp2);
		TextFont(geneva);
		TextSize(9);
		MoveTo(150-StringWidth(temp3)/2, 290);
		DrawString(temp3);
		ReleaseResource(thePic);
	}	
	DrawCurrentWord();
	
	if (!gIsEndGame)
	{
		if (gInProgress)
			HighlightChar();
		
		endi=(gInProgress) ? gNumTries : (gNumTries==MAX_TRIES) ? gNumTries : gNumTries+1;
		for (i=0; i<endi; i++)
		{
			MoveTo(20+gWindowWidth/2-4*CharWidth(' '),105+i*12);
			Mymemcpy((Ptr)((long)tempStr+1), gHumanWord[i], gNumLetters);
			tempStr[0]=gNumLetters;
			DrawString(tempStr);
			DrawString("\p    ");
			NumToString(gNumRight[i], tempStr);
			DrawChar(tempStr[1]);
		}
	
		theRect.top=90;
		theRect.bottom=122;
		theRect.left=43;
		theRect.right=theRect.left+32;
		for (i=0; i<3; i++)
		{
			PlotCIcon(&theRect, gColorIcons[i]);
			theRect.top+=35;
			theRect.bottom+=35;
		}
		PlotCIcon(&theRect, gColorIcons[2]);
		theRect.top+=35;
		theRect.bottom+=35;
		PlotCIcon(&theRect, gColorIcons[1]);
		
		oldChar=gWhichChar;
		gWhichChar=0x0a;
		DrawOneChar();
		gWhichChar=oldChar;
	}
	
	RGBForeColor(&oldForeColor);
	RGBBackColor(&oldBackColor);
}

void DrawBoardBW()
{
	int				i;
	GrafPtr			curPort;
	Rect			theRect;
	Str255			tempStr;
	char			oldChar;
	int				endi;
	unsigned char	*temp="\pYOU ARE A";
	unsigned char	*temp2="\pWINNER!";
	unsigned char	*temp3="\pClick the mouse button to continue";
	PicHandle		thePic;
	
	GetPort(&curPort);
	
	if (!gIsEndGame)
		DrawPicture(gBackgroundPict, &(curPort->portRect));
	else
	{
		FillRect(&(curPort->portRect), black);
		thePic=GetPicture(135);
		FillRect(&(curPort->portRect), black);
		TextFont(144);
		TextSize(24);
		TextMode(srcXor);
		MoveTo(150-StringWidth(temp)/2, 140);
		DrawString(temp);
		SetRect(&theRect, 0, 150, 300, 220);
		DrawPicture(thePic, &theRect);
		MoveTo(150-StringWidth(temp2)/2, 250);
		DrawString(temp2);
		TextFont(geneva);
		TextSize(9);
		MoveTo(150-StringWidth(temp3)/2, 290);
		DrawString(temp3);
		ReleaseResource(thePic);
	}
		
	DrawCurrentWord();
	
	if (!gIsEndGame)
	{
		if (gInProgress)
			HighlightChar();
		
		endi=(gInProgress) ? gNumTries : (gNumTries==MAX_TRIES) ? gNumTries : gNumTries+1;
		for (i=0; i<endi; i++)
		{
			MoveTo(20+gWindowWidth/2-4*CharWidth(' '),105+i*12);
			Mymemcpy((Ptr)((long)tempStr+1), gHumanWord[i], gNumLetters);
			tempStr[0]=gNumLetters;
			DrawString(tempStr);
			DrawString("\p    ");
			NumToString(gNumRight[i], tempStr);
			DrawChar(tempStr[1]);
		}
	
		theRect.top=90;
		theRect.bottom=122;
		theRect.left=43;
		theRect.right=theRect.left+32;
		for (i=0; i<3; i++)
		{
			PlotIcon(&theRect, gBWIcons[i]);
			theRect.top+=35;
			theRect.bottom+=35;
		}
		PlotIcon(&theRect, gBWIcons[2]);
		theRect.top+=35;
		theRect.bottom+=35;
		PlotIcon(&theRect, gBWIcons[1]);
		
		oldChar=gWhichChar;
		gWhichChar=0x0a;
		DrawOneChar();
		gWhichChar=oldChar;
	}
}

void DrawWordInList(void)
{
	int			i;
	Str255		tempStr;
	GrafPtr		curPort;
	
	GetPort(&curPort);
	SetPort(gMainWindow);
	
	TextFont(monaco);
	TextSize(9);
	
	MoveTo(20+gWindowWidth/2-4*CharWidth(' '),105+gNumTries*12);
	Mymemcpy((Ptr)((long)tempStr+1), gHumanWord[gNumTries], gNumLetters);
	tempStr[0]=gNumLetters;
	DrawString(tempStr);
	DrawString("\p    ");
	NumToString(gNumRight[gNumTries], tempStr);
	DrawChar(tempStr[1]);
	
	SetPort(curPort);
}

void DrawOneChar(void)
{
	Rect			theRect;
	char			thisChar;
	GrafPtr			curPort;
	
	GetPort(&curPort);	
	SetPort(gMainWindow);
	TextFont(144);
	TextSize(24);
	TextMode(srcXor);
	theRect.top=32;
	theRect.bottom=theRect.top+36;
	theRect.left=gWindowWidth/2-98-((gNumLetters==6) ? 20 : 0)+40*gWhichChar;
	theRect.right=theRect.left+36;
	FillRect(&theRect, black);
	thisChar=gHumanWord[gNumTries][gWhichChar];
	MoveTo(gWindowWidth/2-80-((gNumLetters==6) ? 20 : 0)+gWhichChar*40-CharWidth(thisChar)/2,
		(thisChar=='Q') ? 59 : 61);
	DrawChar(thisChar);
	SetPort(curPort);
}

void DrawCurrentWord(void)
{
	int			i;
	Rect		theRect;
	char		thisChar;
	
	TextFont(144);
	TextSize(24);
	TextMode(srcXor);
	theRect.top=31;
	theRect.bottom=theRect.top+38;
	theRect.left=gWindowWidth/2-99-((gNumLetters==6) ? 20 : 0);
	theRect.right=theRect.left+38;
	for (i=0; i<gNumLetters; i++)
	{
		FillRect(&theRect, black);
		ForeColor(whiteColor);
		FrameRect(&theRect);
		ForeColor(blackColor);
		theRect.left+=40;
		theRect.right+=40;
	}
	
	for (i=0; i<gNumLetters; i++)
	{
		thisChar=gHumanWord[gNumTries][i];
		MoveTo(gWindowWidth/2-80-((gNumLetters==6) ? 20 : 0)+i*40-CharWidth(thisChar)/2,
			(thisChar=='Q') ? 59 : 61);
		DrawChar(thisChar);
	}
	TextFont(monaco);
	TextSize(9);
}

void HighlightChar(void)
{
	Rect			theRect;
	
	theRect.top=32;
	theRect.bottom=theRect.top+36;
	theRect.left=gWindowWidth/2-98-((gNumLetters==6) ? 20 : 0)+40*gWhichChar;
	theRect.right=theRect.left+36;
	InvertRect(&theRect);
}

void InitJottoGraphics(void)
{
	int				i;
	
	for (i=0; i<3; i++)
	{
		if (gHasColorQD)
			gColorIcons[i]=GetCIcon(128+i);
		gBWIcons[i]=GetIcon(128+i);
	}
	gBackgroundPict=GetPicture(200);	
}

void ShutdownJottoGraphics(void)
{
	int			i;
	
	for (i=0; i<3; i++)
	{
		if (gHasColorQD)
			DisposeCIcon(gColorIcons[i]);
		ReleaseResource(gBWIcons[i]);
	}

	ReleaseResource(gBackgroundPict);
}
