/**********************************************************************\

File:		about.c

Purpose:	This module handles displaying the about box.

\**********************************************************************/

#include "graphics.h"		/* needs to come first because it defines WindowDataHandle */
#include "about.h"
#include "prefs.h"

/*-----------------------------------------------------------------------------------*/
/* internal stuff for about.c                                                        */

void SetupTheAboutBox(WindowDataHandle theData);
void DrawTheAboutBox(Boolean isColor);
void GetTheNextLine(Handle textHandle, unsigned long theSize, unsigned long *pos, Str255 theLine);
void DrawTheAboutString(Str255 theString, int theWidth, int *theRow);


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
		case kStartup:
			SetupTheAboutBox(theData);
			return kSuccess;
			break;
	}
	
	return kFailure;		/* for all other messages, defer to default processing */
}

void SetupTheAboutBox(WindowDataHandle theData)
{
	(**theData).windowWidth=184;
	(**theData).windowHeight=216;
	(**theData).windowType=altDBoxProc;		/* shadowed window */
	(**theData).windowTitle[0]=0x00;		/* null title, never shown */
	(**theData).hasCloseBox=FALSE;
}

void DrawTheAboutBox(Boolean isColor)
{
	int				row;
	Handle			textHandle;
	Str255			theLine;
	unsigned long	pos;
	unsigned long	theSize;
	GrafPtr			curPort;
	int				theWidth;
	
	GetPort(&curPort);
	EraseRect(&(curPort->portRect));
	theWidth=(curPort->portRect.right)-(curPort->portRect.left);	/* for centering */
	
	TextFont(geneva);
	TextSize(9);
	TextMode(srcCopy);	/* straight copy, no bit-biddling */
	if (isColor)
		ForeColor(blueColor);
	row=16;
	textHandle=GetResource('TEXT', 128);	/* in .rsrc file */
	if (textHandle==0L)						/* if not there, don't draw anything (duh) */
		return;
	if (*textHandle==0L)					/* there, but SetResLoad is false */
		LoadResource(textHandle);			/* so get it already! */
	if (*textHandle==0L)					/* if empty, don't draw anything */
		return;
	pos=0L;
	theSize=SizeResource(textHandle);		/* size in bytes of text */
	do
	{
		GetTheNextLine(textHandle, theSize, &pos, theLine);	/* gets one line at a time */
		DrawTheAboutString(theLine, theWidth, &row);		/* draw one line */
	}														/* (Quickdraw doesn't do <CR>) */
	while (pos<theSize);
	ReleaseResource(textHandle);	/* important!  otherwise we'll get memory leaks */
	DrawTheAboutString("\pThis copy is registered to", theWidth, &row);
	if (isColor)
		ForeColor(redColor);
	DrawTheAboutString(gMyName, theWidth, &row);
	DrawTheAboutString(gMyOrg, theWidth, &row);
	if (isColor)
		ForeColor(blackColor);
}

void GetTheNextLine(Handle textHandle, unsigned long theSize, unsigned long *pos, Str255 theLine)
{
	unsigned char	theChar;
	
	theLine[0]=0x00;
	/* the beauty of C -- this just gathers characters in theLine (pascal string) until
	   (1) the end of the text, or (2) a return character */
	while ((*pos<theSize) && ((theChar=*((unsigned char*)(((long)(*textHandle))+((*pos)++))))!=0x0d))
		theLine[++theLine[0]]=theChar;
}

void DrawTheAboutString(Str255 theString, int theWidth, int *theRow)
{
	MoveTo((theWidth-StringWidth(theString))/2, *theRow);	/* center text in window */
	DrawString(theString);
	*theRow+=12;	/* Quickdraw doesn't advance row position for us */
}
