/**********************************************************************\

File:		halma board size.c

Purpose:	This module handles doing the "board size" window.

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

#include "halma board size.h"
#include "environment.h"
#include "util.h"
#include "prefs.h"
#include "program globals.h"

/* internal procedures for halma board size.c only */
void SetupTheBoardSizeWindow(WindowDataHandle theData);
void ShutDownTheBoardSizeWindow(WindowDataHandle theData);
void InitializeTheBoardSizeWindow(WindowDataHandle theData);
void OpenTheBoardSizeWindow(WindowDataHandle theData);
void IdleInBoardSizeWindow(WindowDataHandle theData);
void KeyPressedInBoardSizeWindow(WindowDataHandle theData, unsigned char theChar);
void MouseClickedInBoardSizeWindow(WindowDataHandle theData, Point thePoint);
void DisposeTheBoardSizeWindow(WindowDataHandle theData);
void ActivateTheBoardSizeWindow(WindowDataHandle theData);
void DeactivateTheBoardSizeWindow(WindowDataHandle theData);
void DrawTheBoardSizeWindow(WindowDataHandle theData, short theDepth);

static	short			gOldForegroundTime;		/* stored foreground wait time */
static	PicHandle		gSliderPict;

short BoardSizeWindowDispatch(WindowDataHandle theData, short theMessage,
	unsigned long misc)
{
	short			theDepth;
	unsigned char	theChar;
	Point			thePoint;
	
	switch (theMessage)
	{
		case kNull:
			IdleInBoardSizeWindow(theData);
			return kSuccess;
			break;
		case kUpdate:
			theDepth=misc&0x7fff;
			DrawTheBoardSizeWindow(theData, theDepth);
			return kSuccess;
			break;
		case kInitialize:
			InitializeTheBoardSizeWindow(theData);
			return kSuccess;
			break;
		case kOpen:
			OpenTheBoardSizeWindow(theData);
			return kSuccess;
			break;
		case kKeydown:
			theChar=misc&charCodeMask;
			KeyPressedInBoardSizeWindow(theData, theChar);
			return kSuccess;
			break;
		case kMousedown:
 			thePoint.h=(misc>>16)&0x7fff;
 			thePoint.v=misc&0x7fff;
 			MouseClickedInBoardSizeWindow(theData, thePoint);
 			return kSuccess;
 			break;
 		case kActivate:
 			ActivateTheBoardSizeWindow(theData);
 			return kSuccess;
 			break;
 		case kDeactivate:
 			DeactivateTheBoardSizeWindow(theData);
 			return kSuccess;
 			break;
		case kStartup:
			SetupTheBoardSizeWindow(theData);
			return kSuccess;
			break;
		case kDispose:
			DisposeTheBoardSizeWindow(theData);
			return kSuccess;
			break;
		case kShutdown:
			ShutDownTheBoardSizeWindow(theData);
			return kSuccess;
			break;
	}
	
	return kFailure;
}

void SetupTheBoardSizeWindow(WindowDataHandle theData)
{
	unsigned char	*titleStr="\pBoard Size";
	
	(**theData).maxDepth=1;
	(**theData).windowWidth=154;
	(**theData).windowHeight=120;
	(**theData).windowType=noGrowDocProc;
	(**theData).hasCloseBox=TRUE;
	Mymemcpy((Ptr)((**theData).windowTitle), (Ptr)titleStr, titleStr[0]+1);
	
	gSliderPict=0L;
}

void ShutDownTheBoardSizeWindow(WindowDataHandle theData)
{
	gSliderPict=ReleaseThePict(gSliderPict);
}

void InitializeTheBoardSizeWindow(WindowDataHandle theData)
{
	(**theData).initialTopLeft.h =
		screenBits.bounds.left + (((screenBits.bounds.right -
		screenBits.bounds.left) - (**theData).windowWidth) / 2);
	(**theData).initialTopLeft.v =
		screenBits.bounds.top + (((screenBits.bounds.bottom -
		screenBits.bounds.top) - (**theData).windowHeight) / 3);
}

void OpenTheBoardSizeWindow(WindowDataHandle theData)
{
	(**theData).offscreenNeedsUpdate=TRUE;
}

void IdleInBoardSizeWindow(WindowDataHandle theData)
{
}

void KeyPressedInBoardSizeWindow(WindowDataHandle theData, unsigned char theChar)
{
	switch (theChar)
	{
		case 0x0d:
		case 0x03:
		case 0x1b:
			CloseTheWindow((ExtendedWindowDataHandle)theData);
			break;
	}
}

void MouseClickedInBoardSizeWindow(WindowDataHandle theData, Point thePoint)
{
	Point			startMouseLoc, tempMouseLoc;
	Rect			sliderRect, wholeRect;
	Point			temp, startPt, lastPt;
	
	SetRect(&sliderRect, ROW_LEFT-4, ROW_TOP+5+(9-gNumRows)*12,
		ROW_LEFT+18, ROW_TOP+15+(9-gNumRows)*12);
	SetRect(&wholeRect, ROW_LEFT-4, ROW_TOP+4, ROW_LEFT+18, ROW_TOP+74);
	GetMouse(&startMouseLoc);
	if (PtInRect(startMouseLoc, &sliderRect))
	{
		gNumRows=0;
		startPt.h=sliderRect.left-1;
		startPt.v=sliderRect.top;
		lastPt=temp=startPt;
		while (StillDown())
		{
			GetMouse(&tempMouseLoc);
			temp.v=startPt.v+tempMouseLoc.v-startMouseLoc.v;
			if (temp.v<wholeRect.top)
				temp.v=wholeRect.top;
			else if (temp.v>wholeRect.bottom-10)
				temp.v=wholeRect.bottom-10;
			if (lastPt.v!=temp.v)
			{
				gNumRows=-temp.v;
				(**theData).offscreenNeedsUpdate=TRUE;
				UpdateTheWindow((ExtendedWindowDataHandle)theData);
			}
			lastPt=temp;
		}
		temp.v=wholeRect.top+((temp.v-wholeRect.top+6)/12)*12;
		if (lastPt.v!=temp.v)
		{
			gNumRows=-temp.v;
			(**theData).offscreenNeedsUpdate=TRUE;
			UpdateTheWindow((ExtendedWindowDataHandle)theData);
		}
		gNumRows=9-((temp.v-wholeRect.top)/12);
	}
	else
	{
		SetRect(&sliderRect, COL_LEFT-4, COL_TOP+5+(9-gNumColumns)*12,
			COL_LEFT+18, COL_TOP+15+(9-gNumColumns)*12);
		SetRect(&wholeRect, COL_LEFT-4, COL_TOP+4, COL_LEFT+18, COL_TOP+74);
		GetMouse(&startMouseLoc);
		if (PtInRect(startMouseLoc, &sliderRect))
		{
			gNumColumns=0;
			startPt.h=sliderRect.left-1;
			startPt.v=sliderRect.top;
			lastPt=temp=startPt;
			while (StillDown())
			{
				GetMouse(&tempMouseLoc);
				temp.v=startPt.v+tempMouseLoc.v-startMouseLoc.v;
				if (temp.v<wholeRect.top)
					temp.v=wholeRect.top;
				else if (temp.v>wholeRect.bottom-10)
					temp.v=wholeRect.bottom-10;
				if (lastPt.v!=temp.v)
				{
					gNumColumns=-temp.v;
					(**theData).offscreenNeedsUpdate=TRUE;
					UpdateTheWindow((ExtendedWindowDataHandle)theData);
				}
				lastPt=temp;
			}
			temp.v=wholeRect.top+((temp.v-wholeRect.top+6)/12)*12;
			if (lastPt.v!=temp.v)
			{
				gNumColumns=-temp.v;
				(**theData).offscreenNeedsUpdate=TRUE;
				UpdateTheWindow((ExtendedWindowDataHandle)theData);
			}
			gNumColumns=9-((temp.v-wholeRect.top)/12);
		}
	}
}

void DisposeTheBoardSizeWindow(WindowDataHandle theData)
{
}

void ActivateTheBoardSizeWindow(WindowDataHandle theData)
{
	gOldForegroundTime=gForegroundWaitTime;
	gForegroundWaitTime=0;
}

void DeactivateTheBoardSizeWindow(WindowDataHandle theData)
{
	gForegroundWaitTime=gOldForegroundTime;
}

void DrawTheBoardSizeWindow(WindowDataHandle theData, short theDepth)
{
	Rect			theRect, theOtherRect;
	short			i;
	GrafPtr			curPort;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	
	SetRect(&theRect, ROW_LEFT, ROW_TOP, ROW_LEFT+12, ROW_TOP+79);
	theOtherRect=theRect;
	OffsetRect(&theOtherRect, COL_LEFT-ROW_LEFT, 0);
	FrameRoundRect(&theRect, 10, 9);
	FrameRoundRect(&theOtherRect, 10, 9);
	InsetRect(&theRect, 2, 2);
	InsetRect(&theOtherRect, 2, 2);
	FrameRoundRect(&theRect, 7, 7);
	FrameRoundRect(&theOtherRect, 7, 7);
	InsetRect(&theRect, 1, 1);
	InsetRect(&theOtherRect, 1, 1);
	FillRoundRect(&theRect, 7, 7, gray);
	FillRoundRect(&theOtherRect, 7, 7, gray);
	TextSize(9);
	TextMode(srcOr);
	TextFont(geneva);
	TextFace(0);
	for (i=0; i<=5; i++)
	{
		MoveTo(ROW_LEFT-3, ROW_TOP+9+i*12);
		Line(-3, 0);
		MoveTo(COL_LEFT-3, COL_TOP+9+i*12);
		Line(-3, 0);
		MoveTo(ROW_LEFT-16, ROW_TOP+13+i*12);
		DrawChar('9'-i);
		MoveTo(COL_LEFT-16, COL_TOP+13+i*12);
		DrawChar('9'-i);
	}
	if (gNumRows>0)
		gSliderPict=DrawThePicture(gSliderPict, sliderID, ROW_LEFT-5,
			ROW_TOP+4+(9-gNumRows)*12);
	else
		gSliderPict=DrawThePicture(gSliderPict, sliderID, ROW_LEFT-5, -gNumRows);
	
	if (gNumColumns>0)
		gSliderPict=DrawThePicture(gSliderPict, sliderID, COL_LEFT-5,
			COL_TOP+4+(9-gNumColumns)*12);
	else
		gSliderPict=DrawThePicture(gSliderPict, sliderID, COL_LEFT-5, -gNumColumns);
	
	MoveTo(ROW_LEFT-13, ROW_TOP-10);
	DrawString("\pRows");
	MoveTo(COL_LEFT-18, COL_TOP-10);
	DrawString("\pColumns");
}
