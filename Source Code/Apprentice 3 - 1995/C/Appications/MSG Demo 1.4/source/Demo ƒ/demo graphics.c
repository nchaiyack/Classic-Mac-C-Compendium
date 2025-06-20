/**********************************************************************\

File:		demo graphics.c

Purpose:	This module handles updating the main window (via the
			offscreen bitmaps).

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
#include "demo graphics.h"
#include "util.h"
#include "program globals.h"

/* internal procedures for demo graphics.c only */
void SetupTheMainWindow(WindowDataHandle theData);
void OpenTheMainWindow(WindowDataHandle theData);
void ShutdownTheMainWindow(WindowDataHandle theData);
void DrawTheMainWindowColor(void);
void DrawTheMainWindowBW(void);

PicHandle		gPic1ColorPict;
PicHandle		gPic1BWPict;
PicHandle		gPic2ColorPict;
PicHandle		gPic2BWPict;

enum
{
	kPic1ColorID=200,
	kPic2ColorID,
	kPic1BWID,
	kPic2BWID
};

int MainWindowDispatch(ExtendedWindowDataHandle theData, int theMessage, unsigned long misc)
{
	int				theDepth;
	
	switch (theMessage)
	{
		case kUpdate:
			theDepth=misc&0x7fff;
			if (theDepth>2)
				DrawTheMainWindowColor();
			else
				DrawTheMainWindowBW();
			return kSuccess;
			break;
		case kOpen:
			OpenTheMainWindow(theData);
			return kSuccess;
			break;
		case kStartup:
			SetupTheMainWindow(theData);
			return kSuccess;
			break;
		case kShutdown:
			ShutdownTheMainWindow(theData);
			return kSuccess;
			break;
	}
	
	return kFailure;
}

void SetupTheMainWindow(WindowDataHandle theData)
{
	unsigned char	*titleStr="\pMSG Demo";
	
	(**theData).windowWidth=500;
	(**theData).windowHeight=300;
	(**theData).windowType=noGrowDocProc;
	(**theData).hasCloseBox=TRUE;
	Mymemcpy((**theData).windowTitle, titleStr, titleStr[0]+1);
	gPic1ColorPict=gPic1BWPict=gPic2ColorPict=gPic2BWPict=0L;
	OpenTheWindow((**theData).windowIndex);
}

void OpenTheMainWindow(WindowDataHandle theData)
{
	(**theData).offscreenNeedsUpdate=TRUE;
}

void ShutdownTheMainWindow(WindowDataHandle theData)
{
	if (ReleaseThePict(gPic1ColorPict))
		gPic1ColorPict=0L;
	if (ReleaseThePict(gPic1BWPict))
		gPic1BWPict=0L;
	if (ReleaseThePict(gPic2ColorPict))
		gPic2ColorPict=0L;
	if (ReleaseThePict(gPic2BWPict))
		gPic2BWPict=0L;
}

void DrawTheMainWindowColor(void)
{
	RGBColor	oldForeColor, oldBackColor;
	
	GetForeColor(&oldForeColor);
	GetBackColor(&oldBackColor);
	
	if (!gWhichWipe)
		gWhichPict=!gWhichPict;
	if (gWhichPict)
		DrawThePicture(&gPic1ColorPict, kPic1ColorID, 0, 0);
	else
		DrawThePicture(&gPic2ColorPict, kPic2ColorID, 0, 0);
	
	if (!gWhichWipe)
		gWhichPict=!gWhichPict;
	
	RGBForeColor(&oldForeColor);
	RGBBackColor(&oldBackColor);
}

void DrawTheMainWindowBW(void)
{
	if (!gWhichWipe)
		gWhichPict=!gWhichPict;
	if (gWhichPict)
		DrawThePicture(&gPic1BWPict, kPic1BWID, 0, 0);
	else
		DrawThePicture(&gPic2BWPict, kPic2BWID, 0, 0);
	
	if (!gWhichWipe)
		gWhichPict=!gWhichPict;
}
