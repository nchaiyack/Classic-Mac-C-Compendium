/**********************************************************************\

File:		cdev meat.c

Purpose:	This module handles installing & de-installing the VBL.
			

Go Sit In The Corner -=- not you, just the cursor
Copyright ©1994, Mark Pilgrim

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

#include "cdev globals.h"
#include "cdev meat.h"
#include "cdev prefs.h"
#include "cdev.h"
#include "Retrace.h"
#include "GestaltEQU.h"

void RemoveTheVBL(PrefHandle cdevStorage)
{
	if (((**cdevStorage).ourVBLPtr>=(unsigned long)RAMBase) &&
		((**cdevStorage).ourVBLPtr<(unsigned long)MemTop) &&
		((**cdevStorage).ourCodePtr>=(unsigned long)RAMBase) &&
		((**cdevStorage).ourCodePtr<(unsigned long)MemTop) &&
		(GetPtrSize((Ptr)((**cdevStorage).ourVBLPtr))==12+sizeof(VBLTask)) &&
		(VRemove((VBLTask*)((long)((**cdevStorage).ourVBLPtr)+12))==noErr))
	{
		DisposePtr((Ptr)((**cdevStorage).ourVBLPtr));
		DisposeHandle((Handle)((**cdevStorage).ourCodePtr));
	}
}

int InstallTheVBL(PrefHandle cdevStorage)
{
	Ptr				tempPtr;
	VBLTask			*vblPtr;
	Handle			ourCode;
	Rect			mainRect;
	
	GetMainScreenBounds(&mainRect);
	ourCode=0L;
	ourCode=GetResource('vbl ', 999);
	if (ourCode!=0L)
	{
		if (*ourCode==0L)
			LoadResource(ourCode);
		if (*ourCode==0L)
			return kCantGetResource;
		
		HLock(ourCode);
		DetachResource(ourCode);
		
		tempPtr=NewPtrSys(12+sizeof(VBLTask));
		*((unsigned char*)tempPtr)=(**cdevStorage).always;
		*((unsigned long*)((long)tempPtr+4))=(**cdevStorage).numTicks;
		*((int*)((long)tempPtr+8))=(((**cdevStorage).whichCorner==0x00) ||
			((**cdevStorage).whichCorner==0x03)) ? mainRect.left : mainRect.right;
		*((int*)((long)tempPtr+10))=(((**cdevStorage).whichCorner==0x00) ||
			((**cdevStorage).whichCorner==0x01)) ? mainRect.top : mainRect.bottom;
		
		vblPtr = (VBLTask*) ((long)tempPtr+12);
		vblPtr->qType = vType;
		vblPtr->vblAddr = (ProcPtr) *ourCode;
		vblPtr->vblCount = 1;
		vblPtr->vblPhase = 0;
		VInstall(vblPtr);
		(**cdevStorage).ourCodePtr=(unsigned long)ourCode;
		(**cdevStorage).ourVBLPtr=(unsigned long)tempPtr;
	}
	else return kCantGetResource;

	return prefs_allsWell;
}

void GetMainScreenBounds(Rect* theRect)
{
	long				oldA5;
	QDGlobals			qd;
	GrafPort			gp;
	GrafPtr				savePort;

	GetPort(&savePort);
	oldA5 = SetA5((long)&qd.end);
	InitGraf(&qd.thePort);
	OpenPort(&gp);
	SetCursor(&qd.arrow);
	
	SetRect(theRect, qd.screenBits.bounds.left, qd.screenBits.bounds.top,
		qd.screenBits.bounds.right, qd.screenBits.bounds.bottom);
	
	ClosePort(&gp);
	SetA5(oldA5);
	SetPort(savePort);
}
