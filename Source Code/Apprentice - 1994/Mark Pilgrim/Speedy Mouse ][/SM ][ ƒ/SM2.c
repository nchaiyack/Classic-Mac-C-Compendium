/**********************************************************************\

File:		SM2.c

Purpose:	This module handles installing the VBL from a 'vbl '
			resource and initializes an array of 'mcky' data.
			

Speedy Mouse ][ -=- all the mouse, twice the speed, none of the hassle
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

#include "Retrace.h"

void main(void);

void main(void)
{
	VBLTask 		*vblPtr;
	Ptr				tempPtr;
	Handle			ourCode;
	int				i;
	Handle			tempHandle;
	
	tempPtr = NewPtrSys(56+sizeof(VBLTask));
	for (i=0; i<=6; i++)
	{
		tempHandle=GetResource('mcky', i);
		if (tempHandle==0L)
			LoadResource(tempHandle);
		if (tempHandle==0L)
		{
			DisposePtr(tempPtr);
			SysBeep(7);
			return;
		}
		HLock(tempHandle);
		BlockMove(*tempHandle, tempPtr+i*8, 8);
		HUnlock(tempHandle);
		ReleaseResource(tempHandle);
	}
	
	ourCode = GetResource('vbl ', 999);
	HLock(ourCode);
	DetachResource(ourCode);
	
	vblPtr=(VBLTask*)((long)tempPtr+56);
	vblPtr->qType = vType;
	vblPtr->vblAddr = (ProcPtr) *ourCode;
	vblPtr->vblCount = 1;
	vblPtr->vblPhase = 0;
	VInstall(vblPtr);
}
