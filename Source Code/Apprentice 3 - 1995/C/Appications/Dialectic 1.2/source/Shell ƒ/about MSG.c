/**********************************************************************\

File:		about MSG.c

Purpose:	This module handles displaying the "About MSG" splash
			screen.

\**********************************************************************/

#include "graphics.h"		/* needs to come first because it defines WindowDataHandle */
#include "about MSG.h"
#include "environment.h"

extern Point RawMouse : 0x82C;

/*-----------------------------------------------------------------------------------*/
/* internal stuff for about MSG.c                                                    */

void SetupTheAboutMSGWindow(WindowDataHandle theData);
void DrawTheAboutMSGWindow(void);
void DoTheMSGThing(WindowDataHandle theData);
void ActivateTheMSGWindow(void);
void DeactivateTheMSGWindow(WindowDataHandle theData);
void GetTheNextLine(Handle textHandle, unsigned long theSize, unsigned long *pos, Str255 theLine);
void DrawTheAboutString(Str255 theString, int theWidth, int *theRow);
void DrawTheCarpet(void);
void DrawTheString(Str255 theString, int theWidth, int* theRow);
void DrawCarpet(int x, int y, int len);

static int		gOldForegroundTime;

int AboutMSGBoxDispatch(ExtendedWindowDataHandle theData, int theMessage, unsigned long misc)
{
	int				theDepth;
	
	switch (theMessage)	/* see graphics.h for list of messages*/
	{
		case kNull:
			DoTheMSGThing(theData);
			return kSuccess;
			break;
		case kUpdate:
			DrawTheAboutMSGWindow();
			return kSuccess;
			break;
		case kActivate:
			ActivateTheMSGWindow();
			return kSuccess;
			break;
		case kDeactivate:
			DeactivateTheMSGWindow(theData);
			return kSuccess;
			break;
		case kKeydown:							/* close about box on keypress */
		case kMousedown:						/* or mouseclick */
			CloseTheWindow(theData);
			return kSuccess;
			break;
		case kStartup:
			SetupTheAboutMSGWindow(theData);
			return kSuccess;
			break;
	}
	
	return kFailure;		/* for all other messages, defer to default processing */
}

void SetupTheAboutMSGWindow(WindowDataHandle theData)
{
	(**theData).windowWidth=243;
	(**theData).windowHeight=243;
	(**theData).windowType=plainDBox;		/* plain rectangle */
	(**theData).windowTitle[0]=0x00;		/* null title, never shown */
	(**theData).hasCloseBox=FALSE;
}

void ActivateTheMSGWindow(void)
{
	gOldForegroundTime=gForegroundWaitTime;
	gForegroundWaitTime=0;
}

void DeactivateTheMSGWindow(WindowDataHandle theData)
{
	UpdateTheWindow(theData);
	gForegroundWaitTime=gOldForegroundTime;
}

void DoTheMSGThing(WindowDataHandle theData)
{
	Rect			sourceRect, destRect;
	Rect			mainRect;
	long			lr,tb;
	int				l,t;
	int				index;
	
	index=(**theData).windowIndex;
	mainRect=screenBits.bounds;
	if (PtInRect(RawMouse, &mainRect))
	{
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
		SetPort(gTheWindow[index]);
		CopyBits(&(WMgrPort->portBits), &(gTheWindow[index]->portBits),
			&sourceRect, &destRect, 0, 0L);
	}
}

void DrawTheAboutMSGWindow(void)
{
	int				row;
	Rect			sourceRect, destRect;
	GrafPtr			curPort;
	int				theWidth;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	theWidth=curPort->portRect.right-curPort->portRect.left;
	
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
	DrawTheString("\pMerriMac", theWidth, &row);
	DrawTheString("\pSoftware", theWidth, &row);
	DrawTheString("\pGroup �94", theWidth, &row);
	DrawTheString("\p", theWidth, &row);
	DrawTheString("\pStill enhancing", theWidth, &row);
	DrawTheString("\pthe flavor of", theWidth, &row);
	DrawTheString("\pyour Macintosh.", theWidth, &row);
}

void DrawTheString(Str255 theString, int theWidth, int* theRow)
{
	MoveTo((theWidth-StringWidth(theString))/2, *theRow);
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
