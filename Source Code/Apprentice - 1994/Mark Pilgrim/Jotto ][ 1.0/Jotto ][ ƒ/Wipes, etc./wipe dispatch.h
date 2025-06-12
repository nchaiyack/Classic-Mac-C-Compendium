/**********************************************************************\

File:		wipe dispatch.h

Purpose:	This is the header file for wipe dispatch.c


Jotto ][ -=- a simple word game, revisited
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

void DoTheDissolve(GrafPtr myGrafPtr);
void DoTheWipe(GrafPtr myGrafPtr);
void BoxOutWipe(GrafPtr, GrafPtr, int, int);
void CircleIn(GrafPtr, GrafPtr, int, int);
void CircleOut(GrafPtr, GrafPtr, int, int);
void CircularWipe(GrafPtr, GrafPtr, int, int);
void DiagonalWipe(GrafPtr, GrafPtr, int, int);
void FourCorner(GrafPtr, GrafPtr, int, int);
void FullScrollUD(GrafPtr, GrafPtr, int, int);
void MrDo(GrafPtr, GrafPtr, int, int);
void RescueRaiders(GrafPtr, GrafPtr, int, int);
void Skipaline(GrafPtr, GrafPtr, int, int);
void SlideWipe(GrafPtr, GrafPtr, int, int);
void SpiralGyra(GrafPtr, GrafPtr, int, int);
void DissolveBox(GrafPtr, GrafPtr, Rect*, Rect*);
