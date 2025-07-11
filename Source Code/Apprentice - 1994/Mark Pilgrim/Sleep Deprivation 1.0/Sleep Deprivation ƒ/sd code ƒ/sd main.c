/**********************************************************************\

File:		sd main.c

Purpose:	This module handles the sleep proc dispatch, doing the
			Right Thing� on sleep request, demand, and wake up.


Sleep Deprivation -- graphic effects on sleep
Copyright (C) 1993 Mark Pilgrim & Dave Blumenthal

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
	
	sdDispatch((Ticks&0x7fffffff)%8);
	
	asm
	{
	@3	movem.l	(sp)+, a0-a5/d0-d7
	@4
	}
}

void sdDispatch(int whichWipe)
{
	GrafPtr			thePtr;
	int				oldMenuBarHeight;
	long			oldA5;
	QDGlobals		qd;				/* our QD globals. */
	GrafPort		gp;				/* our grafport. */
	GrafPtr			port;
	GrafPtr			savePort;
	
	GetPort(&savePort);
	oldMenuBarHeight=MBarHeight;
	MBarHeight=0;
	DrawMenuBar();

	/* get a value for A5, a structure that mirrors qd globals. */
	oldA5 = SetA5((long)&qd.end);
	InitGraf(&qd.thePort);
	OpenPort(&gp);
	port = &gp;
	
	HideCursor();
	
	if (whichWipe==0)		SpiralGyra(port, &qd.black);
	else if (whichWipe==1)	CircularWipe(port, &qd.black);
	else if (whichWipe==2)	BoxInWipe(port, &qd.black);
	else if (whichWipe==3)	Skipaline(port, &qd.black);
	else if (whichWipe==4)	RandomWipe(port, &qd.black);
	else if (whichWipe==5)	FullScrollUD(port, &qd.black);
	else if (whichWipe==6)	MrDo(port, &qd.black);
	else if (whichWipe==7)	MrDoOutdone(port, &qd.black);

// You would think a simple switch-case statement would suffice, but you would
// be mistaken.  In fact, the following switch-case causes a crash on a Powerbook
// 100.  This is most likely a THINK C bug; it is super-optimizing the switch-case
// so much that it no longer works on a 68000 machine.  The series of if-then-elses
// is less elegant to the programmer, but more elegant to the user, since it
// doesn't crash.  (:

#if 0
	switch (whichWipe)
	{
		case 0:	SpiralGyra(port, &qd.black);	break;
		case 1:	CircularWipe(port, &qd.black);	break;
		case 2:	BoxInWipe(port, &qd.black);		break;
		case 3:	Skipaline(port, &qd.black);		break;
		case 4:	RandomWipe(port, &qd.black);	break;
		case 5:	FullScrollUD(port, &qd.black);	break;
		case 6:	MrDo(port, &qd.black);			break;
		case 7:	MrDoOutdone(port, &qd.black);	break;
	}
#endif
	
	MBarHeight=oldMenuBarHeight;
	ShowCursor();
	ObscureCursor();
	
	ClosePort(&gp);
	SetA5(oldA5);
	SetPort(savePort);
}
