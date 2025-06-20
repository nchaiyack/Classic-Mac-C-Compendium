/* LittleButtonImages.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#define Compiling_LittleButtonImages
#include "LittleButtonImages.h"
#include "Screen.h"


/* these bitmaps are 12x12 pixels */
Bitmap*											PlusSignNormal;
Bitmap*											PlusSignMouseDown;
Bitmap*											MinusSignNormal;
Bitmap*											MinusSignMouseDown;


static unsigned char				oPlusSignNormal[] =
	{
		0xFF,0xF0,0x80,0x10,0x80,0x10,0x86,0x10,0x86,0x10,0x9F,0x90,
		0x9F,0x90,0x86,0x10,0x86,0x10,0x80,0x10,0x80,0x10,0xFF,0xF0
	};

static unsigned char				oPlusSignMouseDown[] =
	{
		0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xF9,0xF0,0xF9,0xF0,0xE0,0x70,
		0xE0,0x70,0xF9,0xF0,0xF9,0xF0,0xFF,0xF0,0xFF,0xF0,0xFF,0xF0
	};

static unsigned char				oMinusSignNormal[] =
	{
		0xFF,0xF0,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0x9F,0x90,
		0x9F,0x90,0x80,0x10,0x80,0x10,0x80,0x10,0x80,0x10,0xFF,0xF0
	};

static unsigned char				oMinusSignMouseDown[] =
	{
		0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xE0,0x70,
		0xE0,0x70,0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xFF,0xF0,0xFF,0xF0
	};


/* initialize the plus and minus bitmaps */
MyBoolean					InitializeLittleButtonImages(void)
	{
		PlusSignNormal = MakeBitmap(oPlusSignNormal,12,12,2);
		if (PlusSignNormal == NIL)
			{
			 FailurePoint1:
				return False;
			}
		PlusSignMouseDown = MakeBitmap(oPlusSignMouseDown,12,12,2);
		if (PlusSignMouseDown == NIL)
			{
			 FailurePoint2:
				DisposeBitmap(PlusSignNormal);
				goto FailurePoint1;
			}
		MinusSignNormal = MakeBitmap(oMinusSignNormal,12,12,2);
		if (MinusSignNormal == NIL)
			{
			 FailurePoint3:
				DisposeBitmap(PlusSignMouseDown);
				goto FailurePoint2;
			}
		MinusSignMouseDown = MakeBitmap(oMinusSignMouseDown,12,12,2);
		if (MinusSignMouseDown == NIL)
			{
			 FailurePoint4:
				DisposeBitmap(MinusSignNormal);
				goto FailurePoint3;
			}
		return True;
	}


/* dispose of the little button images */
void							ShutdownLittleButtonImages(void)
	{
		DisposeBitmap(PlusSignNormal);
		DisposeBitmap(PlusSignMouseDown);
		DisposeBitmap(MinusSignNormal);
		DisposeBitmap(MinusSignMouseDown);
	}
