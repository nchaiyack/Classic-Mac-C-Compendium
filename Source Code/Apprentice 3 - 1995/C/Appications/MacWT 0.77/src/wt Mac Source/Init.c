/*
** File:		Init.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1994 Teacher Support Software
** All rights reserved.
*/


#include	"Init.h"
#include	"AEHandler.h"
#include	"Failure.h"
#include 	<Palettes.h>
#include	"Menus.h"
#include	"MacGame.h"

#include	"Constants.h"


static	void	StandardInitialization(short callsToMoreMasters);
static	void	InitToolBox(void);
static	void	StandardMenuSetup(short MBARID, short AppleMenuID);
static	void	PullApplicationToFront(void);


/*****************************************************************************/




static	void	InitToolBox(void)
{
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
}



/*****************************************************************************/



static	void	StandardInitialization(short callsToMoreMasters)
{
	InitToolBox();

	while (callsToMoreMasters--) MoreMasters();

	PullApplicationToFront();
}



/*****************************************************************************/




static	void	PullApplicationToFront(void)
{
#define kBroughtToFront 3

	EventRecord event;
	short		count;

	for (count = 1; count <= kBroughtToFront; count++)
		EventAvail(everyEvent, &event);
}



/*****************************************************************************/




void	Initialize(short moreMasters, long minHeap, long minSpace)
{
	long		total, contig, qCPU, qFPU;
	GDHandle	theGDevice;

	StandardInitialization(moreMasters);
		
	/* We used to make a check for memory at this point by examining
	** ApplLimit, ApplicZone, and StackSpace and comparing that to the minimum
	** size we told the Finder we needed.  This did not work well because it
	** assumed too much about the relationship between what we asked
	** the Finder for and what we would actually get back, as well as how to
	** measure it.  Instead, we will use an alternate method comprised of
	** two steps. */

	/* It is better to first check the size of the application heap against a
	** value that you have determined is the smallest heap the application can
	** reasonably work in.  This number should be derived by examining the
	** size of the heap that is actually provided by the Finder when the
	** minimum size requested is used.  The derivation of the minimum size
	** requested from MultiFinder is described in DTS.Lib.h.  The check should
	** be made because the preferred size can end up being set smaller than
	** the minimum size by the user.  This extra check acts to ensure that
	** your application is starting from a solid memory foundation. */

	if ((long)GetApplLimit() - (long)ApplicZone() < kMinMem)
		Fail(MemError(), __FILE__, __LINE__, TRUE);

	/* Next, make sure that enough memory is free for your application to run.
	** It is possible for a situation to arise where the heap may have been of
	** required size, but a large scrap was loaded which left too little
	** memory.  To check for this, call PurgeSpace and compare the result with
	** a value that you have determined is the minimum amount of free memory
	** your application needs at initialization.  This number can be derived
	** several different ways.  One way that is fairly straightforward is to
	** run the application in the minimum size configuration as described
	** previously.  Call PurgeSpace at initialization and examine the value
	** returned.  However, you should make sure that this result is not being
	** modified by the scrapÕs presence.  You can do that by calling ZeroScrap
	** before calling PurgeSpace.  Make sure to remove that call before
	** shipping, though. */

	PurgeSpace(&total, &contig);
	if (total < minSpace)
		Fail(MemError(), __FILE__, __LINE__, TRUE);

	/* The extra benefit to waiting until after the Toolbox Managers have been
	** initialized to check memory is that we can now give the user an alert
	** to tell him/her what happened.  Although it is possible that the memory
	** situation could be worsened by displaying an alert, MultiFinder would
	** gracefully exit the application with an informative alert if memory
	** became critical.  Here we are acting more in a preventative manner to
	** avoid future disaster from low-memory problems. */

#if !__powerc
    Gestalt(gestaltSystemVersion, &total);
	Gestalt(gestaltFPUType, &qFPU);
	Gestalt(gestaltProcessorType, &qCPU);
    total = (total >> 8) & 0xf;
    if ((total < 7) || (qCPU < 3) || !qFPU)
    	{
        StopAlert(rBadStartAlert, nil);
        ExitToShell();
    	}
#endif

	theGDevice = GetMainDevice();

	if ( !HasDepth( theGDevice, 8, 0, 0 ) )
    	{
        StopAlert(rBadStartAlert, nil);
        ExitToShell();
    	}

	if ( (gScreenDepth = (*(*theGDevice)->gdPMap)->pixelSize) != 8)
		SetDepth( theGDevice, 8, 0, 0 );
	
	StandardMenuSetup(rMenuBar, mApple);

	qd.randSeed = TickCount();
}



/*****************************************************************************/




void	StandardMenuSetup(short MBARID, short AppleMenuID)
{
	Handle 		menuBar = GetNewMBar(MBARID);			/* Read menus into menu bar. */
	MenuHandle	menu = GetMHandle(mFile);
	
	
	if (!menuBar)
		Fail(ResError(), __FILE__, __LINE__, TRUE);

	SetMenuBar(menuBar);							/* Install menus. */
	DisposHandle(menuBar);
	AddResMenu(GetMHandle(AppleMenuID), 'DRVR');	/* Add DA names to Apple menu. */
	
	SetItemMark(menu, kShowFPS, (gShowFPS) ? checkMark : noMark);
		
	DrawMenuBar();
}


