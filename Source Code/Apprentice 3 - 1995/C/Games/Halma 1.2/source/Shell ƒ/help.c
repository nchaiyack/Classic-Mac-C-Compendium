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

#include "help.h"
#include "environment.h"
#include "util.h"
#include "buttons.h"
#include "timing.h"
#include "program globals.h"

#define DEAD_SPACE_TOP		10
#define DEAD_SPACE_LEFT		10
#define DEAD_SPACE_BOTTOM	5
#define DEAD_SPACE_RIGHT	10
#define	TEXT_RECT_WIDTH		405
#define	TEXT_RECT_HEIGHT	250
#define	BUTTON_WIDTH		80
#define	BUTTON_HEIGHT		17
#define BUTTON_GAP_H		15
#define BUTTON_GAP_V		5

#define MAX_MAIN_TOPICS		5
#define	MAX_SUB_TOPICS		6

#define	MAX_XREFS			6

#define MAIN_TOPIC_ID		600
#define POPUP_MENU_ID		100

typedef unsigned char	**CharHandle;

typedef struct
{
	long			offset;
	short			lineHeight;
	short			fontDescent;
	short			fontNum;
	unsigned char	fontStyle;
	unsigned char	unused1;
	short			fontSize;
	short			unused2;
	short			unused3;
	short			unused4;
} OneStyle;

typedef struct
{
	short		numStyles;
	OneStyle	theStyle[31];
} StylRec, *StylPtr, **StylHandle;

enum
{
	kLeft=0,
	kCenter
};

short			gNumMainTopics;
short			gNumSubTopics[MAX_MAIN_TOPICS];
short			gNumXRefs[MAX_MAIN_TOPICS][MAX_SUB_TOPICS];

Str31			gMainTopicTitle[MAX_MAIN_TOPICS];
Rect			gMainTopicRect[MAX_MAIN_TOPICS];

Str31			gSubTopicTitle[MAX_MAIN_TOPICS][MAX_SUB_TOPICS];
short			gSubTopicID[MAX_MAIN_TOPICS][MAX_SUB_TOPICS];

short			gXRefIndex[MAX_MAIN_TOPICS][MAX_SUB_TOPICS][MAX_XREFS];
Rect			gXRefRect[MAX_XREFS];

short			gMainTopicShowing;		/* saved in prefs file */
short			gSubTopicShowing;		/* saved in prefs file */

Rect			gTextRect;
CharHandle		gTheText;
StylHandle		gTheStyle;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for help.c                                                         */

static	void SetupTheHelpWindow(WindowDataHandle theData);
static	void ShutdownTheHelpWindow(WindowDataHandle theData);
static	void InitializeTheHelpWindow(WindowDataHandle theData);
static	void OpenTheHelpWindow(WindowDataHandle theData);
static	void KeyPressedInHelpWindow(WindowDataHandle theData, unsigned char keyPressed);
static	void MouseClickedInHelpWindow(WindowDataHandle theData, Point mouseLoc);
static	void DrawTheHelpWindow(short theDepth);
static	void DrawTheText(CharHandle theText, StylHandle theStyleHandle, short theJust,
				short theMode, Rect theRect);
static	void DrawTheShadowBox(Rect theRect);
static	short ParseRawTitle(Str255 theTitle, short *xRef, short *numXRefs);
static	void GoToPage(WindowDataHandle theData, short mainTopic, short subTopic,
				Boolean updateNow);
static	void PushInSubTopic(WindowDataHandle theData);
static	void PullOutSubTopic(WindowDataHandle theData, short mainTopic);
static	void GetTextResources(short mainTopic, short subTopic);
static	void DisposeTextResources(void);
static	void CalculateXRefInfo(short index, short *mainTopic, short *subTopic,
				Str255 name);
static	Boolean MouseInModelessPopUp(MenuHandle theMenu, short *theChoice,
				Rect *theRect, short menuResID);


short HelpWindowDispatch(WindowDataHandle theData, short theMessage, unsigned long misc)
{
	unsigned char	theChar;
	Point			thePoint;
	short			theDepth;
	
	switch (theMessage)
	{
		case kUpdate:
			theDepth=misc&0x7fff;
			DrawTheHelpWindow(theDepth);
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
	short			i,j;
	unsigned char	*helpStr="\pHelp";
	Handle			temp;
	short			strID;
	
	temp=GetResource('STR#', MAIN_TOPIC_ID);
	gNumMainTopics=**((short**)temp);
	ReleaseResource(temp);
	for (i=0; i<gNumMainTopics; i++)
	{
		GetIndString(gMainTopicTitle[i], MAIN_TOPIC_ID, i+1);
		strID=ParseRawTitle(gMainTopicTitle[i], 0L, 0L);
		
		for (j=0; j<5; j++)
			gMainTopicTitle[i][++gMainTopicTitle[i][0]]=' ';
		
		SetRect(	&gMainTopicRect[i],
					DEAD_SPACE_LEFT+(BUTTON_WIDTH+BUTTON_GAP_H)*i,
					DEAD_SPACE_TOP,
					DEAD_SPACE_LEFT+(BUTTON_WIDTH+BUTTON_GAP_H)*i+BUTTON_WIDTH,
					DEAD_SPACE_TOP+BUTTON_HEIGHT);
		
		temp=GetResource('STR#', strID);
		gNumSubTopics[i]=**((short**)temp);
		ReleaseResource(temp);
		
		for (j=0; j<gNumSubTopics[i]; j++)
		{
			GetIndString(gSubTopicTitle[i][j], strID, j+1);
			gSubTopicID[i][j]=ParseRawTitle(gSubTopicTitle[i][j], gXRefIndex[i][j],
				&(gNumXRefs[i][j]));
		}
	}
	
	gTheText=0L;
	gTheStyle=0L;
	GoToPage(0L, gMainTopicShowing, gSubTopicShowing, FALSE);
	
	SetRect(&gTextRect, DEAD_SPACE_LEFT, DEAD_SPACE_TOP+BUTTON_HEIGHT+BUTTON_GAP_V,
		DEAD_SPACE_LEFT+TEXT_RECT_WIDTH,
		DEAD_SPACE_TOP+BUTTON_HEIGHT+BUTTON_GAP_V+TEXT_RECT_HEIGHT);
	
	(**theData).maxDepth=8;
	(**theData).windowWidth=DEAD_SPACE_LEFT+TEXT_RECT_WIDTH+DEAD_SPACE_RIGHT;
	(**theData).windowHeight=BUTTON_GAP_V+DEAD_SPACE_TOP+BUTTON_HEIGHT+TEXT_RECT_HEIGHT+
		DEAD_SPACE_BOTTOM;
	(**theData).windowType=noGrowDocProc;	/* document-looking thing */
	(**theData).hasCloseBox=TRUE;
	(**theData).windowBounds.top=50;
	(**theData).windowBounds.left=6;
	SetIndWindowTitle(kHelp, helpStr);
	
	if (gIsVirgin)
		OpenTheIndWindow((**theData).windowIndex);
}

void ShutdownTheHelpWindow(WindowDataHandle theData)
{
	DisposeTextResources();
}

void InitializeTheHelpWindow(WindowDataHandle theData)
{
	(**theData).initialTopLeft.v=(**theData).windowBounds.top-9;
	(**theData).initialTopLeft.h=(**theData).windowBounds.left;
}

void OpenTheHelpWindow(WindowDataHandle theData)
{
	(**theData).offscreenNeedsUpdate=TRUE;
}

void KeyPressedInHelpWindow(WindowDataHandle theData, unsigned char keyPressed)
{
	short			oldTopic;
	
	ObscureCursor();
	
	switch (keyPressed)
	{
		case 0x1d:										/* right arrow */
			gSubTopicShowing++;
			if (gSubTopicShowing>=gNumSubTopics[gMainTopicShowing])
			{
				gSubTopicShowing=0;
				gMainTopicShowing++;
				if (gMainTopicShowing>=gNumMainTopics)
					gMainTopicShowing=0;
			}
			GoToPage(theData, gMainTopicShowing, gSubTopicShowing, TRUE);
			break;
		case 0x1c:										/* left arrow */
			gSubTopicShowing--;
			if (gSubTopicShowing<0)
			{
				gMainTopicShowing--;
				if (gMainTopicShowing<0)
					gMainTopicShowing=gNumMainTopics-1;
				gSubTopicShowing=gNumSubTopics[gMainTopicShowing]-1;
			}
			GoToPage(theData, gMainTopicShowing, gSubTopicShowing, TRUE);
			break;
	}
}

void MouseClickedInHelpWindow(WindowDataHandle theData, Point mouseLoc)
{
	short			i;
	Str255			name;
	short			newMain, newSub;
	MenuHandle		theMenu;
	Rect			menuRect;
	
	for (i=0; i<gNumXRefs[gMainTopicShowing][gSubTopicShowing]; i++)
	{
		if (PtInRect(mouseLoc, &gXRefRect[i]))
		{
			CalculateXRefInfo(gXRefIndex[gMainTopicShowing][gSubTopicShowing][i],
				&newMain, &newSub, name);
			
			if ((newMain!=-1) && (newSub!=-1))
			{
				if (Track3DButton(&gXRefRect[i], name, 0L, (**theData).windowDepth, TRUE))
				{
					GoToPage(theData, newMain, newSub, TRUE);
					return;
				}
			}
		}
	}
	
	newMain=-1;
	
	for (i=0; i<gNumMainTopics; i++)
	{
		if (PtInRect(mouseLoc, &gMainTopicRect[i]))
		{
			newMain=i;
			i=gNumMainTopics;
		}
	}
	
	if (newMain!=-1)
	{
		Draw3DButton(&gMainTopicRect[newMain], gMainTopicTitle[newMain], 0L,
			(**theData).windowDepth, TRUE, TRUE);
		
		theMenu=NewMenu(POPUP_MENU_ID, "\p");
		for (i=0; i<gNumSubTopics[newMain]; i++)
		{
			AppendMenu(theMenu, gSubTopicTitle[newMain][i]);
			CheckItem(theMenu, i+1, ((newMain==gMainTopicShowing) && (i==gSubTopicShowing)));
		}
		
		menuRect.top=gMainTopicRect[newMain].bottom-1;
		menuRect.left=gMainTopicRect[newMain].left+1;
		if (MouseInModelessPopUp(theMenu, &newSub, &menuRect, POPUP_MENU_ID))
		{
			GoToPage(theData, newMain, newSub-1, TRUE);
		}
		else
		{
			Draw3DButton(&gMainTopicRect[newMain], gMainTopicTitle[newMain], 0L,
				(**theData).windowDepth, FALSE, TRUE);
		}
		
		DisposeHandle((Handle)theMenu);
	}
}

void DrawTheHelpWindow(short theDepth)
{
	GrafPtr			curPort;
	short			i;
	Rect			tempRect;
	RgnHandle		triangleRgn;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	
	DrawTheShadowBox(gTextRect);
	if (gTheText!=0L)
	{
		tempRect=gTextRect;
		InsetRect(&tempRect, 8, 4);
		DrawTheText(gTheText, gTheStyle, kLeft, srcOr, tempRect);
	}
	
	for (i=0; i<gNumMainTopics; i++)
	{
		Draw3DButton(&gMainTopicRect[i], gMainTopicTitle[i], 0L, theDepth, FALSE, TRUE);
	}
	
}

void DrawTheText(CharHandle theText, StylHandle theStyleHandle, short theJust,
	short theMode, Rect theRect)
{
	short			i, numStyles;
	long			textPos;
	long			maxOffset;
	Str255			thisLine;
	Boolean			notDoneYet;
	unsigned char	thisChar;
	short			theRow, theCol;
	unsigned char	lastEnd, thisEnd;
	Boolean			overRun;
	
	numStyles=(**theStyleHandle).numStyles;
	textPos=0L;
	theRow=theRect.top+(**theStyleHandle).theStyle[0].fontDescent+1;
	theCol=theRect.left;
	thisLine[0]=0x00;
	lastEnd=0;
	for (i=0; i<numStyles; i++)
	{
		if (i==numStyles-1)
			maxOffset=GetHandleSize((Handle)theText);
		else
			maxOffset=(**theStyleHandle).theStyle[i+1].offset;
		
		TextFont((**theStyleHandle).theStyle[i].fontNum);
		TextFace((**theStyleHandle).theStyle[i].fontStyle);
		TextSize((**theStyleHandle).theStyle[i].fontSize);
		TextMode(theMode);
		
		while (textPos<maxOffset)
		{
			notDoneYet=TRUE;
			while ((textPos<maxOffset) && (notDoneYet))
			{
				thisChar=thisLine[++thisLine[0]]=(*theText)[textPos++];
				notDoneYet=((thisChar!=' ') && (thisChar!=0x0d));
			}
			
			thisEnd=thisLine[0];
			overRun=(theRect.right-theCol<=StringWidth(thisLine));
			
			if ((overRun) || (thisChar==0x0d) || (textPos==maxOffset))
			{
				if (overRun)
					thisLine[0]=lastEnd;
				if (theJust==kCenter)
					MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
							theCol, theRow);
				else
					MoveTo(theCol, theRow);
				theCol+=StringWidth(thisLine);
				DrawString(thisLine);
				if (overRun)
				{
					BlockMove(&thisLine[lastEnd+1], &thisLine[1], thisEnd-lastEnd+1);
					if (thisEnd>=lastEnd)
					{
						thisLine[0]=thisEnd-lastEnd-1;
						textPos--;
					}
					else
						thisEnd=thisLine[0]=0x00;
				}
				else thisLine[0]=0x00;
				if ((overRun) || (thisChar==0x0d))
				{
					theRow+=(**theStyleHandle).theStyle[i].lineHeight;
					theCol=theRect.left;
				}
			}
			
			lastEnd=thisEnd;
		}
		
		if (thisLine[0]!=0x00)
		{
			if (theJust==kCenter)
				MoveTo((theRect.right-theRect.left-StringWidth(thisLine))/2+
						theCol, theRow);
			else
				MoveTo(theCol, theRow);
			theCol+=StringWidth(thisLine);
			DrawString(thisLine);
			thisLine[0]=0x00;
		}
	}
	TextMode(srcOr);
}

void DrawTheShadowBox(Rect theRect)
{
	theRect.right-=2;
	theRect.bottom-=2;
	FrameRect(&theRect);
	MoveTo(theRect.left+3, theRect.bottom+1);
	Line(theRect.right-theRect.left-2, 0);
	Line(0, -theRect.bottom+theRect.top+3);
	MoveTo(theRect.left+3, theRect.bottom);
	Line(theRect.right-theRect.left-3, 0);
	Line(0, -theRect.bottom+theRect.top+4);
}

short ParseRawTitle(Str255 theTitle, short *xRef, short *numXRefs)
{
	Str255			numStr;
	long			result;
	short			i,j;
	Boolean			gotbullet;
	Boolean			moreXRefs;
	
	if (xRef!=0L)
	{
		*numXRefs=0;
		for (j=1, gotbullet=FALSE; ((j<=theTitle[0]) && (!gotbullet)); j++)
			gotbullet=(theTitle[j]=='%');
		if (gotbullet)
		{
			i=j;
			do
			{
				numStr[0]=0x00;
				while ((numStr[0]<=theTitle[0]-i) &&
					(((numStr[numStr[0]]=theTitle[i+(numStr[0]++)]))!=' ')) {}
				if (numStr[numStr[0]]==' ')
				{
					moreXRefs=TRUE;
					i+=numStr[0];
					numStr[0]--;
				}
				else moreXRefs=FALSE;
				StringToNum(numStr, &result);
				xRef[(*numXRefs)++]=result;
			}
			while (moreXRefs);
			theTitle[0]=j-2;
		}
	}
	numStr[0]=0x00;
	while ((numStr[numStr[0]]=theTitle[++numStr[0]])!=' ') {}
	theTitle[0]-=numStr[0];
	Mymemcpy((Ptr)&theTitle[1], (Ptr)&theTitle[numStr[0]+1], theTitle[0]);
	numStr[0]--;
	StringToNum(numStr, &result);
	return result;
}

void GoToPage(WindowDataHandle theData, short mainTopic, short subTopic,
	Boolean updateNow)
{
	DisposeTextResources();
	GetTextResources(mainTopic, subTopic);
	gMainTopicShowing=mainTopic;
	gSubTopicShowing=subTopic;
	if (updateNow)
	{
		(**theData).offscreenNeedsUpdate=TRUE;
		UpdateTheWindow((ExtendedWindowDataHandle)theData);
	}
}

void GetTextResources(short mainTopic, short subTopic)
{
	short			resID;
	
	DisposeTextResources();
	resID=gSubTopicID[mainTopic][subTopic];
	gTheText=(CharHandle)GetResource('TEXT', resID);
	gTheStyle=(StylHandle)GetResource('styl', resID);
}

void DisposeTextResources(void)
{
	if (gTheText!=0L)
		ReleaseResource((Handle)gTheText);
	if (gTheStyle!=0L)
		ReleaseResource((Handle)gTheStyle);
	gTheText=0L;
	gTheStyle=0L;
}

void CalculateXRefInfo(short index, short *mainTopic, short *subTopic, Str255 name)
{
	short			theMain, theSub;
	unsigned char	*bad="\pBad XRef!";
	
	theMain=gNumMainTopics-1;
	theSub=gNumSubTopics[theMain]-1;
	while ((theMain>=0) && (index!=gSubTopicID[theMain][theSub]))
	{
		theSub--;
		if (theSub<0)
		{
			theMain--;
			theSub=gNumSubTopics[theMain];
		}
	}
	
	if (theMain<0)
	{
		Mymemcpy((Ptr)name, (Ptr)bad, bad[0]+1);
		*mainTopic=*subTopic=-1;
	}
	else
	{
		Mymemcpy((Ptr)name, (Ptr)gSubTopicTitle[theMain][theSub],
			gSubTopicTitle[theMain][theSub][0]+1);
		*mainTopic=theMain;
		*subTopic=theSub;
	}
}

Boolean MouseInModelessPopUp(MenuHandle theMenu, short *theChoice, Rect *theRect,
	short menuResID)
{
	Point			popLoc;
	short			chosen;
	short			newChoice;
	
	InsertMenu(theMenu, -1);
	popLoc.h=theRect->left;
	popLoc.v=theRect->top;
	LocalToGlobal(&popLoc);
	CalcMenuSize(theMenu);
	chosen=PopUpMenuSelect(theMenu, popLoc.v, popLoc.h, 0);
	DeleteMenu(menuResID);
	
	if (chosen!=0)
	{
		newChoice=LoWord(chosen);
		if (newChoice!=*theChoice)
		{
			*theChoice=newChoice;
			return TRUE;
		}
	}
	
	return FALSE;
}
