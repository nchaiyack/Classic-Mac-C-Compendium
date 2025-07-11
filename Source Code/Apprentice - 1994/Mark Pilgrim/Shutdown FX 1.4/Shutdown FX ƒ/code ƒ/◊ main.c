/**********************************************************************\

File:		main.c

Purpose:	This module handles the actual shutdown proc -- creating
			a grafport on the screen and clearing it (dispatching
			the graphic effects).
			

Shutdown FX -=- graphic effects on shutdown
Copyright (C) 1993-4, Mark Pilgrim & Dave Blumenthal

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

#include "main.h"
#include "init.h"
#include "fade headers.h"
#include "reversed fade headers.h"
#include "globals.h"
#include "prefs.h"
#include "Shutdown.h"
#define NUM_WIPES 44		/* wow */

Handle			gTheDitl;
DialogTHndl		gTheDlog;
unsigned char	gOnShutdown, gOnRestart, gSequential, gIsVirgin;
int				gWhichWipe;

void sfxShutdownMain(void)
{
	unsigned int	theKeys[8];
	int				resultCode;
	Boolean			oldRestart;
	
	SetUpA4();
	
	oldRestart=gOnRestart;
	GetKeys(&theKeys);
	if ((gIsVirgin) || (theKeys[3]&4))	/* first time, or option key held down */
	{
		resultCode=DoOptionsDialog(FALSE);
		if (resultCode==1)	/* shutdown */
		{
			if (gOnShutdown)
				DoTheDangFade();
		}
		else if (resultCode==2)	/* restart */
		{
			if (gOnRestart)
				DoTheDangFade();
			if (oldRestart)		/* take our restart proc out of queue if it's there */
				ShutDwnRemove((ProcPtr)sfxRestartMain);
			RestoreA4();
			ShutDwnStart();		/* initiate restart */
			ExitToShell();
		}
	}
	else if (gOnShutdown)
		DoTheDangFade();
	
	RestoreA4();
}

void sfxRestartMain(void)
{
	unsigned int	theKeys[8];
	int				resultCode;
	Boolean			oldShutdown;
	
	SetUpA4();
	
	oldShutdown=gOnShutdown;
	GetKeys(&theKeys);
	if ((gIsVirgin) || (theKeys[3]&4))	/* first time, or option key held down */
	{
		resultCode=DoOptionsDialog(TRUE);
		if (resultCode==2)	/* shutdown */
		{
			if (gOnShutdown)
				DoTheDangFade();
			if (oldShutdown)	/* take our shutdown proc out of queue if it's there */
				ShutDwnRemove((ProcPtr)sfxShutdownMain);
			RestoreA4();
			ShutDwnPower();		/* initiate shutdown */
			ExitToShell();
		}
		else if (resultCode==1)	/* restart */
		{
			if (gOnRestart)
				DoTheDangFade();
		}
	}
	else if (gOnRestart)
		DoTheDangFade();
	
	RestoreA4();
}

void DoTheDangFade(void)
{
	int				oldMenuBarHeight;
	long			oldA5;
	QDGlobals		qd;				/* our QD globals. */
	GrafPort		gp;				/* our grafport. */
	GrafPtr			savePort;
	int				whichWipe;
	THz				saveZone;
	unsigned long	temp;
	
	if (gSequential)
	{
		whichWipe=gWhichWipe;
		gWhichWipe++;
		if (gWhichWipe>=NUM_WIPES)
			gWhichWipe=0;
	}
	else
	{
		GetDateTime(&temp);
		whichWipe=(temp&0x7fffffff)%NUM_WIPES;
		gWhichWipe=whichWipe;
	}
	SaveThePrefs();
	
	GetPort(&savePort);
	oldMenuBarHeight=MBarHeight;
	MBarHeight=0;
	DrawMenuBar();

	/* get a value for A5, a structure that mirrors qd globals. */
	oldA5 = SetA5((long)&qd.end);
	InitGraf(&qd.thePort);
	OpenPort(&gp);
	
	HideCursor();
	
	saveZone=GetZone();
	SetZone(SysZone);

	if (whichWipe==0)		BoxInFade(gp.portRect, &qd.black);
	else if (whichWipe==1)	BoxOutFade(gp.portRect, &qd.black);
	else if (whichWipe==2)	CircleBulgeFade(gp.portRect, &qd.black);
	else if (whichWipe==3)	CircleInFade(gp.portRect, &qd.black);
	else if (whichWipe==4)	CircleOutFade(gp.portRect, &qd.black);
	else if (whichWipe==5)	CircleSerendipityFade(gp.portRect, &qd.black);
	else if (whichWipe==6)	CircularFade(gp.portRect, &qd.black);
	else if (whichWipe==7)	CornerCircleFade(gp.portRect, &qd.black);
	else if (whichWipe==8)	DiagonalFade(gp.portRect, &qd.black);
	else if (whichWipe==9)	FourCornerCenteredFade(gp.portRect, &qd.black);
	else if (whichWipe==10)	FourCornerFade(gp.portRect, &qd.black);
	else if (whichWipe==11)	FourCornerScrollFade(gp.portRect, &qd.black);
	else if (whichWipe==12)	FullScrollLRFade(gp.portRect, &qd.black);
	else if (whichWipe==13)	FullScrollUDFade(gp.portRect, &qd.black);
	else if (whichWipe==14)	HalvesScrollFade(gp.portRect, &qd.black);
	else if (whichWipe==15) HilbertFade(gp.portRect, &qd.black);
	else if (whichWipe==16)	MrDoOutdoneFade(gp.portRect, &qd.black);
	else if (whichWipe==17)	MrDoFade(gp.portRect, &qd.black);
	else if (whichWipe==18)	PourScrollFade(gp.portRect, &qd.black);
	else if (whichWipe==19)	RandomFade(gp.portRect, &qd.black);
	else if (whichWipe==20)	RescueRaidersFade(gp.portRect, &qd.black);
	else if (whichWipe==21)	ScissorsFade(gp.portRect, &qd.black);
	else if (whichWipe==22)	SkipalineLRFade(gp.portRect, &qd.black);
	else if (whichWipe==23)	SkipalineFade(gp.portRect, &qd.black);
	else if (whichWipe==24)	SlideFade(gp.portRect, &qd.black);
	else if (whichWipe==25)	SpiralGyraFade(gp.portRect, &qd.black);
	else if (whichWipe==26)	SplitScrollUDFade(gp.portRect, &qd.black);
	else if (whichWipe==27)	TwoCornerFade(gp.portRect, &qd.black);
	else if (whichWipe==28)	CircularFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==29)	CornerCircleFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==30)	DiagonalFadeDownRight(gp.portRect, &qd.black);
	else if (whichWipe==31)	FourCenteredFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==32)	FourCornerFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==33)	FullScrollLeftFade(gp.portRect, &qd.black);
	else if (whichWipe==34)	FullScrollUpFade(gp.portRect, &qd.black);
	else if (whichWipe==35)	HalvesScrollFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==36)	HilbertFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==37)	MrDoOutdoneFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==38)	MrDoFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==39)	PourScrollFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==40)	RescueRaidersFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==41)	ScissorsFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==42)	SlideFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==43)	TwoCornerFadeReversed(gp.portRect, &qd.black);

// You would think a simple switch-case statement would suffice, but you would
// be mistaken.  In fact, the analogous switch-case causes a crash on a Powerbook
// 100.  This is most likely a THINK C bug; it is super-optimizing the switch-case
// so much that it no longer works on a 68000 machine.  The series of if-elses
// is less elegant to the programmer, but more elegant to the user, since it
// doesn't crash.  (:

	SetZone(saveZone);
	
	MBarHeight=oldMenuBarHeight;
	ShowCursor();
	ObscureCursor();
	
	ClosePort(&gp);
	SetA5(oldA5);
	SetPort(savePort);
}

int DoOptionsDialog(Boolean needToSwapButtons)
{
	DialogTPtr		dtmp;
	DialogRecord	dlog;
	Handle			newDitl, itemH;
	Rect			*theRect, box;
	int				left, top, item, itemType;
	
	InitGraf(&thePort);
	SetCursor(&arrow);
	
	HLock(gTheDlog);
	newDitl=gTheDitl;
	HandToHand(&newDitl);

	theRect = (Rect*) *gTheDlog;
	left = (screenBits.bounds.right - (theRect->right - theRect->left)) / 2;
	top = (screenBits.bounds.bottom - (theRect->bottom - theRect->top)) / 3;
	
	if(top < (GetMBarHeight() + 1))
		top = GetMBarHeight() + 1;
	theRect->right += left - theRect->left;
	theRect->left = left;
	theRect->bottom += top - theRect->top;
	theRect->top = top;

	dtmp=*gTheDlog;
	NewDialog(&dlog, theRect, dtmp->title, dtmp->visible, dtmp->procID,
				(WindowPtr)-1L, dtmp->goAwayFlag, dtmp->refCon, newDitl);
	
	GetDItem(&dlog, 12, &itemType, &itemH, &box);
	InsetRect(&box, -4, -4);
	SetDItem(&dlog, 12, itemType, (Handle)OutlineDefaultButton, &box);
	GetDItem(&dlog, 6, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, gOnRestart ? 1 : 0);
	GetDItem(&dlog, 7, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, gOnShutdown ? 1 : 0);
	GetDItem(&dlog, 9, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, gSequential ? 1 : 0);
	GetDItem(&dlog, 10, &itemType, &itemH, &box);
	SetCtlValue((ControlHandle)itemH, gSequential ? 0 : 1);
	if (needToSwapButtons)
	{
		GetDItem(&dlog, 1, &itemType, &itemH, &box);
		SetCTitle((ControlHandle)itemH, "\pRestart");
		GetDItem(&dlog, 2, &itemType, &itemH, &box);
		SetCTitle((ControlHandle)itemH, "\pShutdown");
	}
	
	ShowWindow(&dlog);
	
	item=0;
	do
	{
		ModalDialog((ProcPtr)ProcOFilter, &item);
		switch (item)
		{
			case 6:		gOnRestart=(gOnRestart==0xFF) ? 0x00 : 0xFF;
						GetDItem(&dlog, 6, &itemType, &itemH, &box);
						SetCtlValue((ControlHandle)itemH, gOnRestart ? 1 : 0);
						if ((!gOnRestart) && (!gOnShutdown))
						{
							gOnShutdown=0xFF;
							GetDItem(&dlog, 7, &itemType, &itemH, &box);
							SetCtlValue((ControlHandle)itemH, 1);
						}
						break;
			case 7:		gOnShutdown=(gOnShutdown==0xFF) ? 0x00 : 0xFF;
						GetDItem(&dlog, 7, &itemType, &itemH, &box);
						SetCtlValue((ControlHandle)itemH, gOnShutdown ? 1 : 0);
						if ((!gOnRestart) && (!gOnShutdown))
						{
							gOnRestart=0xFF;
							GetDItem(&dlog, 6, &itemType, &itemH, &box);
							SetCtlValue((ControlHandle)itemH, 1);
						}
						break;
			case 9:		gSequential=0xFF;
						GetDItem(&dlog, 9, &itemType, &itemH, &box);
						SetCtlValue((ControlHandle)itemH, 1);
						GetDItem(&dlog, 10, &itemType, &itemH, &box);
						SetCtlValue((ControlHandle)itemH, 0);
						break;
			case 10:	gSequential=0x00;
						GetDItem(&dlog, 10, &itemType, &itemH, &box);
						SetCtlValue((ControlHandle)itemH, 1);
						GetDItem(&dlog, 9, &itemType, &itemH, &box);
						SetCtlValue((ControlHandle)itemH, 0);
						break;
		}
	}
	while ((item!=1) && (item!=2));
	
	gIsVirgin=0x00;
	SaveThePrefs();
	
	HideWindow(&dlog);
	CloseDialog(&dlog);
	
	HUnlock(gTheDlog);
	DisposeHandle(newDitl);
	
	return item;
}

pascal Boolean ProcOFilter(DialogPtr theDialog, EventRecord *theEvent, short *theItem)
{
	unsigned char	theChar;
	short			itemType;
	Handle			itemH;
	Rect			box;
	unsigned long	dummy;
	
	switch (theEvent->what)
	{
		case keyDown:
		case autoKey:
			theChar=theEvent->message & charCodeMask;
			if ((theChar==0x0d) || (theChar==0x03))		/* RETURN or ENTER */
			{
				*theItem=1;		/* as if the user selected item #1 */
				GetDItem(theDialog, 1, &itemType, &itemH, &box);
				HiliteControl((ControlHandle)itemH, 1);	/* flash button 1 by highlighting, */
				Delay(8, &dummy);						/* waiting 8 ticks, and then */
				HiliteControl((ControlHandle)itemH, 0);	/* unhighlighting -- believe */
				return TRUE;							/* it or not, that's Apple's */
			}											/* preferred method */
			break;
	}
	
	return FALSE;
}

pascal void OutlineDefaultButton(DialogPtr myDlog, short itemNum)
{
	short			itemType;
	Handle			itemH;
	Rect			box;
	
	GetDItem(myDlog, 1, &itemType, &itemH, &box);
	PenSize(3, 3);
	InsetRect(&box, -4, -4);
	FrameRoundRect(&box, 16, 16);
	PenNormal();
}
