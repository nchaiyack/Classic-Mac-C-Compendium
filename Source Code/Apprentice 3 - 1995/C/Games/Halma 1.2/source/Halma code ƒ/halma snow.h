/**********************************************************************\

File:		halma snow.h

Purpose:	This is the header file for halma snow.c.

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

#ifndef _SNOW_H_
#define _SNOW_H_

#include "graphics.h"

enum snow_types
{
	snow_black=0x00,
	snow_red,
	snow_green,
	snow_blue,
	snow_cyan,
	snow_magenta,
	snow_yellow,
	snow_white
};

void InitTheSnow(void);
void ShutDownTheSnow(void);
Boolean SnowIsInitializedQQ(void);
void SetSnowIter(void);
void SnowIdle(WindowDataHandle theData);
void DrawSnowflakeToScreen(WindowDataHandle theData, unsigned short thisX,
	unsigned short thisY, enum snow_types c);
enum snow_types SnowIsTouching(unsigned short thisX, unsigned short thisY);
enum snow_types GetTheSnowPixel(unsigned short thisX, unsigned short thisY);
void NewSnow(WindowDataHandle theData);
void ResetSnow(void);
void GetRidOfSnowflake(WindowDataHandle theData);
#endif
