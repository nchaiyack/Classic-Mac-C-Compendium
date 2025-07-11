/* GrowIcon.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "GrowIcon.h"
#include "Screen.h"


static unsigned char				RawGrowIconEnabled[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x9F,0xC1,
		0x90,0x41,0x90,0x79,0x90,0x49,0x90,0x49,
		0x90,0x49,0x9F,0xC9,0x84,0x09,0x84,0x09,
		0x87,0xF9,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static unsigned char				RawGrowIconDisabled[] =
	{
		0xFF,0xFF,0x80,0x01,0x80,0x01,0x80,0x01,
		0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,
		0x80,0x01,0x80,0x01,0x80,0x01,0x80,0x01,
		0x80,0x01,0x80,0x01,0x80,0x01,0xFF,0xFF
	};

static Bitmap*		GrowIconEnabled = NIL;
static Bitmap*		GrowIconDisabled = NIL;


/* allocate the internal bitmap of the grow icon */
MyBoolean				InitializeGrowIcon(void)
	{
		GrowIconEnabled = MakeBitmap(RawGrowIconEnabled,16,16,2);
		if (GrowIconEnabled == NIL)
			{
			 FailurePoint1:
				return False;
			}
		GrowIconDisabled = MakeBitmap(RawGrowIconDisabled,16,16,2);
		if (GrowIconDisabled == NIL)
			{
			 FailurePoint2:
				DisposeBitmap(GrowIconEnabled);
				goto FailurePoint1;
			}
		return True;
	}


/* dispose of the internal bitmap of the grow icon */
void						ShutdownGrowIcon(void)
	{
		DisposeBitmap(GrowIconEnabled);
		DisposeBitmap(GrowIconDisabled);
	}


/* get a pointer to the bitmap.  The flag is used to choose the appropriate bitmap. */
/* True means the window is active, so the grow icon should be returned.  False means */
/* the window is inactive, so the empty box should be returned */
struct Bitmap*	GetGrowIcon(MyBoolean GrowIconFlag)
	{
		if (GrowIconFlag)
			{
				return GrowIconEnabled;
			}
		 else
			{
				return GrowIconDisabled;
			}
	}
