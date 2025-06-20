/**********************************************************************\

File:		sd main.c

Purpose:	This module handles the sleep proc dispatch, doing the
			Right Thing� on sleep request, demand, and wake up.

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

#include "sd main.h"
#include "sd init.h"
#include "globals.h"
#include "fade headers.h"
#include "reversed fade headers.h"

#define NUM_WIPES 50

void sdMain(void)
{
	// sorry about the assembly, but the codes are passed to the sleep proc in
	// register d0, which is tough to check from C.
	
	asm
	{
		cmpi.l	#sleepRequest, d0
		bne		@1
		clr.l	d0
		bra		@4
	@1	cmpi.l	#sleepWakeUp, d0
		bne		@2
		movem.l	a0-a5/d0-d7, -(sp)
	}
	
	// the system will not automatically redraw the screen; since we've blacked
	// it out, we need to manually redraw it when the computer wakes up.
	
	DrawMenuBar();
	PaintOne(0L,GrayRgn);
	PaintBehind(WindowList,GrayRgn);
	
	asm
	{
		bra		@3
	@2	cmpi.l	#sleepDemand, d0
		bne		@4
		movem.l	a0-a5/d0-d7, -(sp)
	}
	
	sdDispatch((Ticks&0x7fffffff)%NUM_WIPES);
	
	asm
	{
	@3	movem.l	(sp)+, a0-a5/d0-d7
	@4
	}
}

void sdDispatch(int whichWipe)
{
	int				oldMenuBarHeight;
	long			oldA5;
	QDGlobals		qd;				/* our QD globals. */
	GrafPort		gp;				/* our grafport. */
	GrafPtr			savePort;
	THz				saveZone;
	GrafPtr			thePort;
	
	SetUpA4();
	
	GetPort(&savePort);
	oldMenuBarHeight=MBarHeight;
	MBarHeight=0;
	DrawMenuBar();

	/* get a value for A5, a structure that mirrors qd globals. */
	oldA5 = SetA5((long)&qd.end);
	InitGraf(&qd.thePort);
	OpenPort(&gp);
	thePort=&gp;
	SetPort(thePort);
	
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
	else if (whichWipe==7)	DiagonalFade(gp.portRect, &qd.black);
	else if (whichWipe==8)	FourCornerFade(gp.portRect, &qd.black);
	else if (whichWipe==9)	FullScrollLRFade(gp.portRect, &qd.black);
	else if (whichWipe==10)	FullScrollUDFade(gp.portRect, &qd.black);
	else if (whichWipe==11)	HalvesScrollFade(gp.portRect, &qd.black);
	else if (whichWipe==12) HilbertFade(gp.portRect, &qd.black);
	else if (whichWipe==13)	MrDoOutdoneFade(gp.portRect, &qd.black);
	else if (whichWipe==14)	MrDoFade(gp.portRect, &qd.black);
	else if (whichWipe==15)	PourScrollFade(gp.portRect, &qd.black);
	else if (whichWipe==16)	RandomFade(gp.portRect, &qd.black);
	else if (whichWipe==17)	RescueRaidersFade(gp.portRect, &qd.black);
	else if (whichWipe==18)	SkipalineLRFade(gp.portRect, &qd.black);
	else if (whichWipe==19)	SkipalineFade(gp.portRect, &qd.black);
	else if (whichWipe==20)	SpiralGyraFade(gp.portRect, &qd.black);
	else if (whichWipe==21)	CircularFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==22)	DiagonalFadeDownRight(gp.portRect, &qd.black);
	else if (whichWipe==23)	FourCornerFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==24)	FullScrollLeftFade(gp.portRect, &qd.black);
	else if (whichWipe==25)	FullScrollUpFade(gp.portRect, &qd.black);
	else if (whichWipe==26)	HalvesScrollFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==27)	HilbertFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==28)	MrDoOutdoneFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==29)	MrDoFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==30)	PourScrollFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==31)	RescueRaidersFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==32)	HGRFade(gp.portRect, &qd.black);
	else if (whichWipe==33)	HGR2Fade(gp.portRect, &qd.black);
	else if (whichWipe==34)	QuadrantFade2(gp.portRect, &qd.black);
	else if (whichWipe==35)	QuadrantFade(gp.portRect, &qd.black);
	else if (whichWipe==36)	QuadrantScroll2Fade(gp.portRect, &qd.black);
	else if (whichWipe==37)	QuadrantScrollFade(gp.portRect, &qd.black);
	else if (whichWipe==38)	SkipalineLR2PassFade(gp.portRect, &qd.black);
	else if (whichWipe==39)	Skipaline2PassFade(gp.portRect, &qd.black);
	else if (whichWipe==40)	HGRFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==41)	HGR2FadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==42)	QuadrantFade2Reversed(gp.portRect, &qd.black);
	else if (whichWipe==43)	QuadrantFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==44)	QuadrantScroll2FadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==45)	QuadrantScrollFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==46)	SkipalineLR2PassFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==47)	Skipaline2PassFadeReversed(gp.portRect, &qd.black);
	else if (whichWipe==48) SlideFade(gp.portRect, &qd.black);
	else if (whichWipe==49) SlideFadeReversed(gp.portRect, &qd.black);
	
// You would think a simple switch-case statement would suffice, but you would
// be mistaken.  In fact, the following switch-case causes a crash on a Powerbook
// 100.  This is most likely a THINK C bug; it is super-optimizing the switch-case
// so much that it no longer works on a 68000 machine.  The series of if-then-elses
// is less elegant to the programmer, but more elegant to the user, since it
// doesn't crash.  (:

	SetZone(saveZone);
	
	MBarHeight=oldMenuBarHeight;
	ShowCursor();
	ObscureCursor();
	
	ClosePort(&gp);
	SetA5(oldA5);
	SetPort(savePort);
	
	RestoreA4();
}
