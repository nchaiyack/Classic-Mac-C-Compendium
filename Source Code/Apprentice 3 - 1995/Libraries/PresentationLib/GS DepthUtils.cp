/*
	GameShell DepthUtils
	�1995 Chris K. Thomas.  All Rights Reserved.
	
	Monitor Switching routines
	
	*** If I could find documentation for
	the new Display Manager (which is in Sys7.5 and
	PowerROMs), I'd use that instead.
	
	^^^ I will retain this stuff for compatibility
	and use the Display Manager where it is available,
	in a future version.
*/

#include "gs DepthUtils.h"

static short sOldDepth = 0;

Boolean GSSwitchDepth(depth inDepth)
// switch the screen to a particular depth
{
	short	mode = HasDepth(GetMainDevice(), inDepth, gdDevType, 1);
	Boolean	outGood = true;
	
	if(mode == 0)
	{
		outGood = false;
	}
	else
	{
		OSErr theErr;
		
		sOldDepth = (**(**GetMainDevice()).gdPMap).pixelSize;
		
		theErr = SetDepth(GetMainDevice(), mode, gdDevType, 1);
		
		if(theErr != noErr)
		{
			outGood = false;
		}
	}
	
	return outGood;
}

void GSRestoreDepth()
// restore old screen depth
{
	OSErr theErr;
	theErr = SetDepth(GetMainDevice(), sOldDepth, gdDevType, 1);
}