/**********************************************************************\

File:		help.c

Purpose:	This module handles displaying the different help windows.

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

#include "graphics.h"			/* must come first because it defines WindowDataHandle */
#include "help.h"
#include "menus.h"
#include "util.h"
#include "program globals.h"

int				gNumHelp;
int				gWhichHelp;
int				gLastHelp;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for help.c                                                         */

void SetupTheHelpWindow(WindowDataHandle theData);
void ShutdownTheHelpWindow(WindowDataHandle theData);
void InitializeTheHelpWindow(WindowDataHandle theData);
void OpenTheHelpWindow(WindowDataHandle theData);
void KeyPressedInHelpWindow(WindowDataHandle theData, unsigned char keyPressed);
void MouseClickedInHelpWindow(WindowDataHandle theData, Point mouseLoc);
void DrawTheHelp(void);
void AddNameToString(Str255 theLine, Str255 theStringToAdd);

static	Handle			gHelpIcon[4];


int HelpWindowDispatch(ExtendedWindowDataHandle theData, int theMessage, unsigned long misc)
{
	unsigned char	theChar;
	Point			thePoint;
	
	switch (theMessage)
	{
		case kUpdate:
			DrawTheHelp();
			return kSuccess;
			break;
		case kKeydown:
			theChar=misc&charCodeMask;
			KeyPressedInHelpWindow(theData, theChar);
			return kSuccess;
			break;
		case kMousedown:
			thePoint.h=(misc>>16)&0x7fff;
			thePoint.v=misc&0x7fff;
			MouseClickedInHelpWindow(theData, thePoint);
			return kSuccess;
			break;
		case kOpen:
			OpenTheHelpWindow(theData);
			return kSuccess;
			break;
		case kInitialize:
			InitializeTheHelpWindow(theData);
			return kSuccess;
			break;
		case kStartup:
			SetupTheHelpWindow(theData);
			return kSuccess;
			break;
		case kShutdown:
			ShutdownTheHelpWindow(theData);
			return kSuccess;
			break;
	}
	
	return kFailure;		/* revert to default processing for all other messages */
}

void SetupTheHelpWindow(WindowDataHandle theData)
{
	int				i;
	unsigned char	*helpStr="\pHelp";
	
	for (i=0; i<4; i++)
		gHelpIcon[i]=GetIcon(128+i);		/* get icons from .rsrc file */
	gNumHelp=CountMItems(gHelpMenu);		/* total # of help screens */
	gLastHelp=0;							/* # of last selected help screen */
	
	(**theData).windowWidth=300;
	(**theData).windowHeight=250;
	(**theData).windowType=noGrowDocProc;	/* document-looking thing */
	(**theData).hasCloseBox=TRUE;
	(**theData).windowBounds.top=50;
	(**theData).windowBounds.left=10;
	Mymemcpy((**(gTheWindowData[kHelp])).windowTitle, helpStr, helpStr[0]+1);
}

void ShutdownTheHelpWindow(WindowDataHandle theData)
{
	int			i;
	
	for (i=0; i<4; i++)
		ReleaseResource(gHelpIcon[i]);			/* clean up by disposing icons */
}

void InitializeTheHelpWindow(WindowDataHandle theData)
{
	(**theData).initialTopLeft.v=(**theData).windowBounds.top-9;
	(**theData).initialTopLeft.h=(**theData).windowBounds.left;
}

void OpenTheHelpWindow(WindowDataHandle theData)
{
	if (gWhichHelp!=gLastHelp)					/* if new help screen, force update */
		(**theData).offscreenNeedsUpdate=TRUE;	/* of offscreen GWorld/bitmap */
}

void KeyPressedInHelpWindow(WindowDataHandle theData, unsigned char keyPressed)
{
	switch (keyPressed)
	{
		case 0x1c:										/* left arrow */
			gLastHelp=gWhichHelp;						/* save last screen # */
			gWhichHelp--;								/* decrement screen # */
			if (gWhichHelp==0)							/* wrap around */
				gWhichHelp=gNumHelp;
			OpenTheWindow((**theData).windowIndex);		/* display new screen */
			break;
		case 0x1d:										/* right arrow */
			gLastHelp=gWhichHelp;						/* save last screen # */
			gWhichHelp++;								/* increment screen # */
			if (gWhichHelp>gNumHelp)					/* wraparound */
				gWhichHelp=1;
			OpenTheWindow((**theData).windowIndex);		/* display new screen */
			break;
	}
}

void MouseClickedInHelpWindow(WindowDataHandle theData, Point mouseLoc)
{
	Rect		iconRect;
	Boolean		isHilited;
	
	SetRect(&iconRect, 20, 7, 52, 39);		/* rectangle where left arrow icon is */
	if (PtInRect(mouseLoc, &iconRect))		/* if we hit it... */
	{
		isHilited=FALSE;
		while (StillDown())					/* track mouse */
		{
			GetMouse(&mouseLoc);			/* returns point in window's local coords */
			if (PtInRect(mouseLoc, &iconRect))	/* if still in icon's rectangle */
			{
				if (!isHilited)				/* hilight icon if not already hilighted */
					PlotIcon(&iconRect, gHelpIcon[2]);
				isHilited=TRUE;				/* so we know */
			}
			else							/* else we moved outside icon's rectangle */
			{
				if (isHilited)				/* dehilight if highlighted */
					PlotIcon(&iconRect, gHelpIcon[0]);
				isHilited=FALSE;			/* so we know */
			}
		}
		if (isHilited)						/* if we were still in icon's rect on mouseup */
		{
			gLastHelp=gWhichHelp;			/* save last help screen # */
			gWhichHelp--;					/* decrement screen # */
			if (gWhichHelp==0)				/* wrap around */
				gWhichHelp=gNumHelp;
			OpenTheWindow((**theData).windowIndex);		/* display new help screen */
		}
	}
	else	/* do same thing as above, only with the right arrow icon */
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
				gLastHelp=gWhichHelp;
				gWhichHelp++;
				if (gWhichHelp>gNumHelp)
					gWhichHelp=1;
				OpenTheWindow((**theData).windowIndex);
			}
		}
	}
}

void DrawTheHelp(void)
{
	GrafPtr			curPort;
	int				row;
	Handle			textHandle;
	Str255			theLine;
	unsigned long	pos;
	unsigned long	theSize;
	unsigned char	theChar;
	Rect			iconRect;
	int				theWidth;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	
	theWidth=curPort->portRect.right-curPort->portRect.left;
	
	TextFont(geneva);
	TextSize(9);
	row=63;
	textHandle=GetResource('TEXT', 399+gWhichHelp);		/* get text from .rsrc file */
	if (textHandle==0L)		/* if not there, abort */
		return;
	if (*textHandle==0L)	/* if there but SetResLoad=FALSE, load it already! */
		LoadResource(textHandle);
	if (*textHandle==0L)	/* if still no luck, abort */
		return;
	pos=0L;
	theSize=SizeResource(textHandle);	/* size of text */
	while (pos<theSize)
	{
		theLine[0]=0x00;
		/* the beauty of C -- this just gathers characters in theLine (pascal string)
		   until (1) the end of the text, or (2) a return character */
		while ((pos<theSize) && ((theChar=*((unsigned char*)(((long)(*textHandle))+(pos++))))!=0x0d))
		{
			if (theChar=='^')		/* ^ character means insert program name */
				AddNameToString(theLine, APPLICATION_NAME);
			else theLine[++theLine[0]]=theChar;
		}
		MoveTo(8, row);
		DrawString(theLine);		/* draw one line of help */
		row+=12;
	}
	
	ReleaseResource(textHandle);	/* important!  or we'll get memory leaks */
	
	MoveTo(0, 46);
	Line(300, 0);
	
	SetRect(&iconRect, 20, 7, 52, 39);
	PlotIcon(&iconRect, gHelpIcon[0]);		/* plot left arrow icon */
	OffsetRect(&iconRect, 228, 0);
	PlotIcon(&iconRect, gHelpIcon[1]);		/* plot right arrow icon */
	
	GetItem(gHelpMenu, gWhichHelp, theLine);	/* get help screen title from help menu */
	TextFace(bold);
	MoveTo((theWidth-StringWidth(theLine))/2, 26);		/* center it */
	DrawString(theLine);
	TextFace(0);
}

void AddNameToString(Str255 theLine, Str255 theStringToAdd)
{
	int			i;
	
	for (i=1; i<=theStringToAdd[0]; i++)	/* insert application name into help string */
		theLine[++theLine[0]]=theStringToAdd[i];
}
