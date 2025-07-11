/**********************************************************************\

File:		msg graphics.h

Purpose:	This is the header file for msg graphics.c


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

#define		NUM_WINDOWS				4

enum
{
	kMainWindow=0,
	kAbout,
	kAboutMSG,
	kHelp
};

enum
{
	kAboutColorID=134,
	kAboutBWID=135
};

extern	Boolean			gInitedWindowBounds[NUM_WINDOWS];
extern	Rect			gMainScreenBounds;
extern	Rect			gWindowBounds[NUM_WINDOWS];
extern	GDHandle		gBiggestDevice;
extern	WindowPtr		gTheWindow[NUM_WINDOWS];
extern	int				gWindowWidth[NUM_WINDOWS];
extern	int				gWindowHeight[NUM_WINDOWS];
extern	Str255			gWindowTitle[NUM_WINDOWS];
extern	int				gWindowType[NUM_WINDOWS];
extern	Boolean			gOffscreenNeedsUpdate[NUM_WINDOWS];
extern	int				gNumHelp;

void InitMSGGraphics(void);
void ShutDownMSGGraphics(void);
void OpenTheWindow(int index);
void GetMainScreenBounds(void);
int GetWindowDepth(int index);
void UpdateTheWindow(int index);
void UpdateTheWindowColor(int index);
void UpdateTheWindowBW(int index);
void CloseTheWindow(int index);
void UpdateHelpWindow(void);
void DrawThePicture(PicHandle *thePict, int whichPict, int x, int y);
void ReleaseThePict(PicHandle *thePict);
