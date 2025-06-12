/**********************************************************************\

File:		stoned mouse.c

Purpose:	This module handles installing the VBL from a 'vbl '
			resource.
			

Crooked Mouse -=- a mouse rotated 45 degrees
Copyright (C) 1993 Mark Pilgrim

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
	Handle			ourCode;
	
	ourCode = GetResource('vbl ', 671);
	HLock(ourCode);
	DetachResource(ourCode);
	
	vblPtr = (VBLTask*) NewPtrSys(sizeof(VBLTask));
	vblPtr->qType = vType;
	vblPtr->vblAddr = (ProcPtr) *ourCode;
	vblPtr->vblCount = 1;
	vblPtr->vblPhase = 0;
	VInstall(vblPtr);
}
