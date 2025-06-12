/**********************************************************************\

File:		v.c

Purpose:	This module handles Voyeur initialization and shutdown,
			opening a new file (dispatch routine), and handling clicks
			and keypresses in the main window.


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

#include "v.h"
#include "v meat.h"
#include "v structs.h"
#include "v files.h"
#include "v file management.h"
#include "v find.h"
#include "msg menus.h"
#include "msg graphics.h"
#include "msg environment.h"

unsigned char	hexchar[2][256];
unsigned char	asciichar[256];

void InitProgram(void)
{
	int				i,j,k;
	AppFile			myFile;
	
	gWindowWidth=440;
	gWindowHeight=225;
	
	gHelpHeight=200;
	gHelpWidth=300;
		
	findString[0]=0x00;
	gBufferOffset=0;
	
	for (i=0; i<16; i++)
	{
		for (j=0; j<16; j++)
		{
			hexchar[0][i*16+j]=(i<10) ? i+'0' : i+'A'-10;
			hexchar[1][i*16+j]=(j<10) ? j+'0' : j+'A'-10;
		}
	}
	StuffHex(&asciichar[0],		"\p2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E202122232425262728292A2B2C2D2E2F303132333435363738393A3B3C3D3E3F");
	StuffHex(&asciichar[64],	"\p404142434445464748494A4B4C4D4E4F505152535455565758595A5B5C5D5E5F606162636465666768696A6B6C6D6E6F707172737475767778797A7B7C7D7E2E");
	StuffHex(&asciichar[128],	"\p808182838485868788898A8B8C8D8E8F909192939495969798999A9B9C9D9E9FA0A1A2A3A4A5A6A7A8A9AAABACADAEAFB0B1B2B3B4B5B6B7B8B9BABBBCBDBEBF");
	StuffHex(&asciichar[192],	"\pC0C1C2C3C4C5C6C7C8C92ECBCCCDCECFD0D1D2D3D4D5D6D7D8D92E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E2E");
	
	CountAppFiles(&i, &j);
	if ((j>0) && (i==0))
	{
		GetAppFiles(1, &myFile);
		MyMakeFSSpec(myFile.vRefNum, 0, myFile.fName, &editFile);
		OpenTheFile(&editFile, &fileRefNum);
		for (k=1; k<=j; k++)
			ClrAppFiles(k);
	}
}

int NewEditWindow(void)
{
	int			resultCode;
	
	editFile.name[0]=0x00;
	if (GetSourceFile(&editFile, !gHasAppleEvents))
		return OpenTheEditWindow();
	else return userCancelErr;
}

int OpenTheEditWindow(void)
{
	int				resultCode;
	
	resultCode=OpenTheFile(&editFile, &fileRefNum);
	if (resultCode!=allsWell)
		return resultCode;
	OpenMainWindow();
	ObscureCursor();
	gInProgress=TRUE;
	AdjustMenus();
	DrawMenuBar();
	
	return allsWell;
}

void ProgramUndo(void)
{
}

void ProgramEvent(void)
{
	Point		mouseLoc;
	Rect		theRect;
	
	if (gInProgress)
	{
	}
}

void ProgramKeyEvent(char charPressed)
{
	int				i;
	
	if (gInProgress)
	{
		ObscureCursor();
		switch (charPressed)
		{
			case 0x1d: /* right arrow */
			case 'N':
			case 'n':
				GoForward();
				break;
			case 0x1c: /* left arrow */
			case 'P':
			case 'p':
				GoBack();
				break;
			case 'D':
			case 'd':
				DoDataFork();
				break;
			case 'R':
			case 'r':
				DoResourceFork();
				break;
			case 'B':
			case 'b':
				GoBeginning();
				break;
			case 'E':
			case 'e':
				GoEnd();
				break;
			case 'F':
			case 'f':
				DoFind();
				break;
			case 'G':
			case 'g':
				if (findString[0]!=0x00)
					DoFindAgain();
				break;
		}
	}
}

void ShutDownProgram(void)
{
	if (gMainWindow!=0L)
		CloseMainWindow();
}
