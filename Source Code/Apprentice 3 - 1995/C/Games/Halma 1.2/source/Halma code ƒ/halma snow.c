/**********************************************************************\

File:		halma snow.c

Purpose:	This module handles making it snow while you play Halma.

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

#include "halma snow.h"
#include "program globals.h"

#define SNOW_WIDTH 125
#define SNOW_HEIGHT (gNumRows*30-35)
#define SNOW_TOP 10
#define SNOW_LEFT (gNumColumns*30+20)

static	Point			gSnow={-2,-2};
static	short			gSnowIter=5;
static	enum snow_types	*gSnowArray=0L;
static	Boolean			gCanSnow=FALSE;
static	short			gBigIter;

unsigned char	gSnowSpeed;
unsigned char	gSnowColor;
unsigned char	gSnowMutates;

void InitTheSnow(void)
{
	short			i;
	
	gSnowArray=(enum snow_types*)NewPtrClear((SNOW_HEIGHT+2)*SNOW_WIDTH);
	if (gSnowArray==0L)
	{
		gCanSnow=FALSE;
		return;
	}
	gCanSnow=TRUE;
	
	for (i=0; i<SNOW_WIDTH; i++)
		gSnowArray[((SNOW_HEIGHT+1)*SNOW_WIDTH)+i]=gSnowColor;
	
	ResetSnow();
}

void ShutDownTheSnow(void)
{
	if (gSnowArray!=0L)
		DisposePtr((Ptr)gSnowArray);
	gSnowArray=0L;
}

void SetSnowIter(void)
{
	gBigIter=0;
	gSnowIter=5;
}

void SnowIdle(WindowDataHandle theData)
{
	short			r;
	Boolean			notDoneYet;
	
	if (!gCanSnow)
		return;
	
	if (gSnowIter>0)	/* kludge dujour */
	{
		gSnowIter--;
		return;
	}
	
	if ((gSnow.h<0) && (gSnow.v<0))
	{
		gSnow.h=SNOW_LEFT+(Random()&0x7fff)%SNOW_WIDTH;
		gSnow.v=SNOW_TOP;
		if (gSnowSpeed==0x01)
		{
			gBigIter--;
			DrawSnowflakeToScreen(theData, gSnow.h, gSnow.v, snow_white);
		}
		if (SnowIsTouching(gSnow.h, gSnow.v)!=snow_black)
			NewSnow(theData);
	}
	
	notDoneYet=TRUE;
	while (notDoneYet)
	{
		if (gSnowSpeed==0x01)
		{
			gBigIter--;
			DrawSnowflakeToScreen(theData, gSnow.h, gSnow.v, snow_black);
		}
		r=(Random()&0x7fff)%3;
		switch (r)
		{
			case 0:	gSnow.v++;	break;
			case 1:	gSnow.h--;	break;
			case 2:	gSnow.h++;	break;
		}
		if (gSnow.h<SNOW_LEFT)
			gSnow.h+=SNOW_WIDTH;
		if (gSnow.h>=SNOW_LEFT+SNOW_WIDTH)
			gSnow.h-=SNOW_WIDTH;
		notDoneYet=(gSnowSpeed==0x03) ? (SnowIsTouching(gSnow.h, gSnow.v)==snow_black) : FALSE;
		if (gSnowSpeed==0x01)
		{
			gBigIter--;
			DrawSnowflakeToScreen(theData, gSnow.h, gSnow.v, snow_white);
		}
	}
	if (SnowIsTouching(gSnow.h, gSnow.v)!=snow_black)
	{
		DrawSnowflakeToScreen(theData, gSnow.h, gSnow.v, SnowIsTouching(gSnow.h, gSnow.v));
		gSnow.h=gSnow.v=-1;
	}
}

void DrawSnowflakeToScreen(WindowDataHandle theData, unsigned short thisX,
	unsigned short thisY, enum snow_types c)
{
	unsigned char	r;
	short			theColor;
	
	if (++gBigIter==50)
	{
		r=((**theData).windowDepth>2) ? (gSnowMutates) ? snow_red+((Random()&0x7fff)%7) :
			gSnowColor : snow_white;
		gBigIter=0;
	}
	else
	{
		r=c;
	}
	switch (r)
	{
		case snow_black:	theColor=blackColor;	break;
		case snow_red:		theColor=redColor;		break;
		case snow_green:	theColor=greenColor;	break;
		case snow_blue:		theColor=blueColor;		break;
		case snow_cyan:		theColor=cyanColor;		break;
		case snow_magenta:	theColor=magentaColor;	break;
		case snow_yellow:	theColor=yellowColor;	break;
		case snow_white:	theColor=whiteColor;	break;
	}
	
	if ((**theData).windowDepth<=2)
		theColor=whiteColor;
	
	SetPortToOffscreen(theData);
	PenMode(patCopy);
	ForeColor(theColor);
	MoveTo(thisX, thisY);
	Line(0,0);
	ForeColor(blackColor);
	RestorePortToScreen(theData);
	PenMode(patCopy);
	ForeColor(theColor);
	MoveTo(thisX, thisY);
	Line(0,0);
	ForeColor(blackColor);
	(**theData).offscreenNeedsUpdate=TRUE;
	gSnowArray[(SNOW_WIDTH*(thisY-SNOW_TOP+1))+thisX-SNOW_LEFT]=r;
}

enum snow_types SnowIsTouching(unsigned short thisX, unsigned short thisY)
{
	if ((thisY>SNOW_TOP) && (GetTheSnowPixel(thisX-1,thisY-1)!=snow_black))
		return (GetTheSnowPixel(thisX-1,thisY-1));
	else if ((thisY>SNOW_TOP) && (GetTheSnowPixel(thisX, thisY-1)!=snow_black))
		return (GetTheSnowPixel(thisX, thisY-1));
	else if ((thisY>SNOW_TOP) && (GetTheSnowPixel(thisX+1, thisY-1)!=snow_black))
		return (GetTheSnowPixel(thisX+1, thisY-1));
	else if (GetTheSnowPixel(thisX-1, thisY)!=snow_black)
		return (GetTheSnowPixel(thisX-1, thisY));
	else if (GetTheSnowPixel(thisX+1, thisY)!=snow_black)
		return (GetTheSnowPixel(thisX+1, thisY));
	else if (GetTheSnowPixel(thisX-1, thisY+1)!=snow_black)
		return (GetTheSnowPixel(thisX-1, thisY+1));
	else if (GetTheSnowPixel(thisX, thisY+1)!=snow_black)
		return (GetTheSnowPixel(thisX, thisY+1));
	else if (GetTheSnowPixel(thisX+1, thisY+1)!=snow_black)
		return (GetTheSnowPixel(thisX+1, thisY+1));
	else
		return snow_black;
}

enum snow_types GetTheSnowPixel(unsigned short thisX, unsigned short thisY)
{
	return gSnowArray[(SNOW_WIDTH*(thisY-SNOW_TOP+1))+thisX-SNOW_LEFT];
}

void NewSnow(WindowDataHandle theData)
{
	Rect			tempRect;
	
	SetRect(&tempRect, SNOW_LEFT, SNOW_TOP, SNOW_LEFT+SNOW_WIDTH, SNOW_TOP+SNOW_HEIGHT);
	SetPortToOffscreen(theData);
	FillRect(&tempRect, black);
	RestorePortToScreen(theData);
	FillRect(&tempRect, black);
	ShutDownTheSnow();
	InitTheSnow();
}

void ResetSnow(void)
{
	gSnow.h=-2;
	gSnow.v=-2;
	SetSnowIter();
}

void GetRidOfSnowflake(WindowDataHandle theData)
{
	if ((gSnow.h>0) && (gSnow.v>0))
		DrawSnowflakeToScreen(theData, gSnow.h, gSnow.v, snow_black);
	ResetSnow();
}
