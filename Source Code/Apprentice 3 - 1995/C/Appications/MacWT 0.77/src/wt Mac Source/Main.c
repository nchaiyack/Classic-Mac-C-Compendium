/*
** File:		Main.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright � 1995 Nikol Software
** All rights reserved.
*/



#include "AEHandler.h"
#include "Failure.h"
#include "wt.h"
#include "MacWT.h"
#include "Constants.h"
#include "Menu.h"
#include "StringUtils.h"
#include "Init.h"
#include "FileUtils.h"



// profiling code
#include "Profiler.h"

#if __powerc
#define SUFFIX "PPC"
#else
#define SUFFIX "68K"
#endif



EventRecord	gTheEvent;
Boolean		quitting = false;
Boolean		gPaused = false;
Boolean		gShowFPS = true;
Boolean		gGameOn = false;
Boolean		gTrueColor = false;
Boolean		gKeyboardControl = true;
Boolean		gUseQuickdraw = true;
short		gScreenDepth = 0;
Str255		gWorldFile = "\p";
Str15		gWTVersion = "\pUnknown";

#if	__ppcc
QDGlobals 	qd;
#endif


static void InitMacWT(short *width, short *height);



/*****************************************************************************/




static void InitMacWT(short *width, short *height)
{
	unsigned char		**versRsrc;
	Str255				WindowTitle;

	// remember the version

	if (!gWorldFile[0])
		if (!SelectWorldFile(gWorldFile))
			ExitToShell();

	GetFilenameFromPathname(gWorldFile, WindowTitle);

	if ((versRsrc = (unsigned char **)Get1Resource('vers', 1)) != 0)
		pcpy(gWTVersion, *versRsrc + 6);

	/* Open MacWT window (well, dialog)... */
	gWindow = GetNewDialog(rWindowID, nil, (WindowPtr)-1);
	if (!gWindow)
		Fail(ResError(), __FILE__, __LINE__, TRUE);

	GetScreenSize(width, height);

	SetPort(gWindow);
	TextFont(geneva);
	TextSize(9);
	TextFace(bold);
	ShowWindow(gWindow);
}


/*****************************************************************************/



void main(void)
{
	short	width, height;
	unsigned char kmp[16];


	MaxApplZone();							/* Expand the heap so code segments load at the top. */

	Initialize(32, kMinHeap, kMinSpace);	/* Initialize the program. */

	GetKeys((long *)kmp);
	if MacKeyDown(kmp, 0x3A) QuitEverythingButMe();
	
	InitAppleEvents();
	DoAdjustMenus();

	UnloadSeg((Ptr)Initialize);				/* Initialize can't be in Main! */

	while (GetAndProcessEvent())
		;
	
#if __profile__
	if (!ProfilerInit(collectDetailed, bestTimeBase, 20, 5))
	{
#endif

	InitMacWT(&width, &height);

	p2c(gWorldFile);
	WTMain((char *)gWorldFile, width, height, false);

#if __profile__
		ProfilerDump("\pMacWT(" SUFFIX ").prof");
		ProfilerTerm();
	}
#endif
}
