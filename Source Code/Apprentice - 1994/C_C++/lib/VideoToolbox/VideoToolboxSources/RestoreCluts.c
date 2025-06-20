/*
RestoreCluts.c
Developing programs that modify the clut can be annoying, because one frequently
aborts the program for one reason or another and then faces an editing environment
in which text is hard to see because of the weird clut settings. No longer.

Somewhere near the beginning of your program insert the line:

_atexit(RestoreCluts);

This tells the THINK C runtime environment to run RestoreCluts() at exit, even if
it's an abnormal exit, e.g. escape to shell from MacsBug.

CAUTION: the atexit() and _atexit() routines probably should not be used in
MATLAB code resources. The reason is that they will only be invoked when MATLAB
exits, after your code resources have already been flushed, so you'll crash.

For reasons that I don't understand, Apple's RestoreDeviceClut doesn't seem to 
do anything, even though my GDRestoreDeviceClut works fine.

HISTORY:
4/25/92 dgp	wrote it. Why didn't I think of this sooner?
12/8/92 dgp simplified code, based on Apple's Snippet: 
			RestoreColorsSlam() in ColorReset.c.
12/9/92	dgp	replaced call to Apple's RestoreDeviceClut by my GDRestoreDeviceClut, 
			since Apple's routine doesn't seem to do anything.
12/15/92 dgp Renamed to GDRestoreDeviceClut for consistentcy with
			Apple's capitalization of RestoreDeviceClut.
2/22/93  dhb & dgp Converted for MATLAB
*/

#include "VideoToolbox.h"
#include <Menus.h>

void RestoreCluts(void)
{
	// RestoreDeviceClut(NULL);	//  Apple's version, restore all cluts
	GDRestoreDeviceClut(NULL);	// my version, restore all cluts
	// Apple suggests making these two calls as well, to force redrawing of
	// everything.
	PaintBehind(NULL,GetGrayRgn());
	DrawMenuBar();
}