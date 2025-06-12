/*
	Breakout.c
	
	Breakout demo for Graphic Elements library
	
	3/7/94
	
	Al Evans
	
	Note: This is a fairly generic application shell.
	The interesting parts are all in BGame.h and .c
*/

//Load precompiled symbol table if under MPW
#ifdef applec
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#else
#include <QDOffscreen.h>
#endif

#include <Timer.h>
#include <GestaltEqu.h>
#include "GUtilities.h"
#include "GraphElements.h"
#include "BGame.h"


//define HISTOGRAM to collect times history
#undef HISTOGRAM

#ifdef HISTOGRAM
#include "StdIO.h"
#endif


//Menu Commands

#define	rMenuBar				128		/* application's menu bar */

#define	mApple					128		/* Apple menu */
#define	iAbout					1

#define	mFile					129		/* File menu */
#define iNewGame				1
#define	iQuit					2

#define	mEdit					130		/* Edit menu */
#define	iUndo					1
#define	iCut					3
#define	iCopy					4
#define	iPaste					5
#define	iClear					6

#define mSpecial				131
#define iSingleFrame			1

#define	rAboutDialog			228		


//Number of master pointer blocks needed

#define masterBlocksNeeded 10

//Globals

Boolean		gFinished;
WindowPtr	gAnimWindow;
Boolean		gSingleFrame = false;
Boolean		gDoOne = false;

//Performance measurement

#define thirtySeconds 30L * 1000 * 1000		//µsec for timers -- never time out

TMTask	gTTimeTask, gATimeTask;
unsigned long gTotalTime = 0, gAnimTime = 0;

#ifdef HISTOGRAM
long	animTimes[100];
long	mainTimes[100];
#endif

//Forward declarations

#ifdef applec
extern void _DataInit();	//reference so that we can unload it
#endif

Boolean Initialize( void );
void Shutdown(void);
void EventLoop( void );

main()
{
#ifdef applec
	UnloadSeg((Ptr) _DataInit);
#endif
	
	MaxApplZone();
	if ( Initialize() )
		EventLoop();
	Shutdown();
}


//Initialization

Boolean AdequateSystem(void)
{
	OSErr		err;
	long		response;
	Boolean		ok;
	
	//We need 68020 or better
	err = Gestalt(gestaltProcessorType, &response);
	ok = (!err) & (response >= gestalt68020);
	
	//We need System 7 or later
	err = Gestalt(gestaltSystemVersion, &response);
	ok = (!err) & (((response & 0xFFFF) / 256) >= 7);
	
	//We need color QD & offscreen GWorlds
	err = Gestalt(gestaltQuickdrawVersion, &response);
	ok = ok & (!err) & (response >= gestalt32BitQD);
	err = Gestalt(gestaltQuickdrawFeatures, &response);
	ok = ok & (!err) & (response >= 3); 		//hasColor & deep GWorlds
	
	return ok;
	
}

//Make window centered on main graphic device
WindowPtr MakeWindow(short wHSize, short wVSize)
{
	GDHandle	mainDevice;
	Rect		devRect;
	short		hOffst, vOffst;
	Rect		windRect;
	
	mainDevice = GetMainDevice();
	devRect = (**mainDevice).gdRect;
	hOffst = (devRect.right - devRect.left - wHSize) / 2;
	if (hOffst < 0) hOffst = 0;
	vOffst = (devRect.bottom - devRect.top - wVSize) / 2;
	if (vOffst < 0 ) vOffst = 0;
	windRect.left = hOffst;
	windRect.right = hOffst + wHSize;
	windRect.top = vOffst;
	windRect.bottom = vOffst + wVSize; 
	
	return NewCWindow(nil, &windRect, "\pGEBreakout", false, documentProc,
						(WindowPtr) -1L, false, 0L);
}

void DoNothing( void )
{
}

void InitPerformanceTiming(void)
{
	//Init Timers
	gTTimeTask.tmAddr = (TimerProcPtr) DoNothing;
	gTTimeTask.tmWakeUp = 0;
	gTTimeTask.tmReserved = 0;
	
	gATimeTask.tmAddr =  (TimerProcPtr) DoNothing;
	gATimeTask.tmWakeUp = 0;
	gATimeTask.tmReserved = 0;
	
}

Boolean Initialize(void)
{
	Rect		animRect;
	GEWorldPtr	animWorld;
#ifdef HISTOGRAM
	short	count;
#endif
	
	gFinished = false;
	
	InitSystem(masterBlocksNeeded);
	
	
	if (!AdequateSystem()) {
		TellUser("\pSorry, more powerful system required", 0);
		return false;
	}
	if (!LoadMenus(rMenuBar)) {
		TellUser("\pCould not load menus", 0);
		return false;
	}
	
	//DebugStr("\pInitializing...");
	//Create window and install animation
	if (gAnimWindow = MakeWindow(gWindWidth, gWindHeight)){
		animRect.left = 0;
		animRect.right = gWindWidth;
		animRect.top = 0;
		animRect.bottom = gWindHeight;
		if (animWorld = NewGEWorld((CWindowPtr) gAnimWindow, &animRect, nil))
			SetWRefCon(gAnimWindow, (long) animWorld);
		else {
			TellUser("\pCould not install animation in window", 0);
			return false;
		}
	}
	else {
		TellUser("\pCould not create window", 0);
		return false;
	}
	
	InitPerformanceTiming();
	qd.randSeed = TickCount();
	
	//Load game graphics
	
	if (!LoadBreakoutGame(animWorld)) {
		TellUser("\pCould not load game graphics", 0);
		return false;
	}
		
#ifdef HISTOGRAM
	for (count = 0; count < 100; count++) {
		animTimes[count] = 0;
		mainTimes[count] = 0;
	}
#endif

	
	//Turn animation on and show window
	ShowWindow(gAnimWindow);
	ActivateWorld((GEWorldPtr) GetWRefCon(gAnimWindow), true);
	return true;
	
}

#ifdef HISTOGRAM
void DumpHistograms(void)
{
	short	count;
	FILE*	dump;
	
	dump = fopen("HistoDump", "w");
	for (count = 0; count < 100; count++) 
		fprintf(dump, "%d\t%d\t%d\n", count, mainTimes[count], animTimes[count]);
	fflush(dump);
	fclose(dump);
}
#endif

void Shutdown(void)
{
	//Release system resources
	
#ifdef HISTOGRAM
	DumpHistograms();
#endif

	DisposeGEWorld((GEWorldPtr) GetWRefCon(gAnimWindow));
	
	ExitToShell();
}


#ifdef HISTOGRAM
void DoHistogram(void)
{
	if ((gTotalTime - gAnimTime) < 100)
		mainTimes[(gTotalTime - gAnimTime)]++;
	else
		mainTimes[99]++;
	if (gAnimTime < 100)
		animTimes[gAnimTime]++;
	else
		animTimes[99]++;
}
#endif

pascal Boolean AboutFilter(DialogPtr dialog, EventRecord *event, short *item)
{
#pragma unused (dialog)
	DoWorldUpdate((GEWorldPtr) GetWRefCon(gAnimWindow), false);
	if ((event->what == mouseDown) || (event->what == keyDown)) {
		*item = ok;
		return true;
	}
	else return false;
}

void DoAboutBox(void)
{
	DialogPtr	aboutDialog;
	short		itemHit;
	
	aboutDialog = GetNewDialog(rAboutDialog, nil, (WindowPtr) -1L);
	ModalDialog(AboutFilter, &itemHit);
	DisposDialog(aboutDialog);
}


//Event Handling

void EventLoop( void )
{
	Boolean		gotEvent;
	EventRecord	event;
	
	void DoEvent (EventRecord *event);
	void AdjustCursor( void);

	do {
	
#ifdef HISTOGRAM
		InsTime( (QElemPtr) &gTTimeTask);
		InsTime( (QElemPtr) &gATimeTask);
		PrimeTime( (QElemPtr) &gTTimeTask, -thirtySeconds);
#endif

		if (!gSingleFrame || gDoOne) {
		
#ifdef HISTOGRAM
			PrimeTime( (QElemPtr) &gATimeTask, -thirtySeconds);
#endif

			DoWorldUpdate((GEWorldPtr) GetWRefCon(gAnimWindow), false);
			
#ifdef HISTOGRAM
			RmvTime( (QElemPtr) &gATimeTask);
			gAnimTime = (thirtySeconds + gATimeTask.tmCount) / 1000;
#endif

			gDoOne = false;
		}
		AdjustCursor();
		if (gotEvent = WaitNextEvent(everyEvent, &event, 0L, nil)) 
			DoEvent(&event);
			
#ifdef HISTOGRAM
		RmvTime( (QElemPtr) &gTTimeTask);
		gTotalTime = (thirtySeconds + gTTimeTask.tmCount) / 1000; //milliseconds
		DoHistogram();
#endif

	} while (!gFinished);
}

void DoEvent (EventRecord *event)
{
	short		part;
	WindowPtr	window;
	char		key;
	
	//Prototypes
	void AdjustMenus( void );
	void DoMenuCommand(long menuResult);
	void DoActivate(WindowPtr window, Boolean becomingActive);
	void DoUpdate(WindowPtr window);
	
	switch ( event->what ) {
		case mouseDown:
			part = FindWindow(event->where, &window);
			switch ( part ) {
				case inMenuBar:
					AdjustMenus();
					DoMenuCommand(MenuSelect(event->where));
					break;
				case inSysWindow:
					SystemClick(event, window);
					break;
				case inContent:
					if ( window != FrontWindow() ) 
						SelectWindow(window);
					if (MouseDownInSensor((GEWorldPtr) GetWRefCon(gAnimWindow), event->where))
						;;
					break;
				case inDrag:
					DragWindow(window, event->where, &qd.screenBits.bounds);
					break;
			}
			break;
		case keyDown:
			key = event->message & charCodeMask;
			if ( event->modifiers & cmdKey ) {
				AdjustMenus();
				DoMenuCommand(MenuKey(key));
			}
			else {
				switch (key) {
					case 'U':
					case 'u':
						MoveGEWorld((GEWorldPtr) GetWRefCon(gAnimWindow), 0, -50);
						break;
					case 'D':
					case 'd':
						MoveGEWorld((GEWorldPtr) GetWRefCon(gAnimWindow), 0, 50);
						break;
					case 'R':
					case 'r':
						MoveGEWorld((GEWorldPtr) GetWRefCon(gAnimWindow), 50, 0);
						break;
					case 'L':
					case 'l':
						MoveGEWorld((GEWorldPtr) GetWRefCon(gAnimWindow), -50, 0);
						break;
				}
			}
			gDoOne = true;
			break;
		case activateEvt:
			DoActivate((WindowPtr) event->message, (event->modifiers & activeFlag) != 0);
			break;
		case updateEvt:
			DoUpdate((WindowPtr) event->message);
			break;
		case osEvt:
			switch ((event->message >> 24) & 0x0FF) {
				case suspendResumeMessage:
					gInBackground = (event->message & resumeFlag) == 0;
					DoActivate(FrontWindow(), !gInBackground);
					break;
			}
			break;
		}
}

void DoActivate(WindowPtr window, Boolean becomingActive)
{
#pragma unused (window)
	//Could start and stop animation here
	ActivateWorld((GEWorldPtr) GetWRefCon(gAnimWindow), becomingActive);
}


void DoUpdate(WindowPtr window)
{
	Rect geRect;
	GEWorldPtr geWorld;

	if (window == gAnimWindow)
	{
		SetPort( (GrafPtr) window );
		//Protect GEWorld rect before updating window, since we will draw it
		geWorld = (GEWorldPtr) GetWRefCon(gAnimWindow);
		geRect = geWorld->animationRect;
		RectOffset(&geRect, geWorld->worldFocus.h, geWorld->worldFocus.v);
		ValidRect(&geRect);
		
		BeginUpdate(window);
			FillRgn(((GrafPtr) window)->visRgn, qd.gray);
		EndUpdate(window);
		
		DoWorldUpdate((GEWorldPtr) GetWRefCon(gAnimWindow), true);
	}
}

void AdjustMenus( void )
{
	WindowPtr	window;
	MenuHandle	menu;
	
	window = FrontWindow();
	
	menu = GetMHandle(mFile);
	EnableItem(menu, iNewGame);
	EnableItem(menu, iQuit);

	menu = GetMHandle(mEdit);
	if (window = gAnimWindow) {
		DisableItem(menu, iUndo);
		DisableItem(menu, iCut);
		DisableItem(menu, iCopy);
		DisableItem(menu, iClear);
		DisableItem(menu, iPaste);
	}
	else {
		EnableItem(menu, iUndo);
		EnableItem(menu, iCut);
		EnableItem(menu, iCopy);
		EnableItem(menu, iClear);
		EnableItem(menu, iPaste);
	}
	menu = GetMHandle(mSpecial);
	//Set up special items!
	CheckItem(menu, iSingleFrame, gSingleFrame);
}

void DoMenuCommand(long menuResult)
{
	short		menuID;
	short		menuItem;
	Str255		daName;
	short		daRefNum;

	menuID = HiWord(menuResult);
	menuItem = LoWord(menuResult);
	switch ( menuID ) {
		case mApple:
			switch ( menuItem ) {
				case iAbout:
					DoAboutBox();
					break;
				default:			/* all other items in this menu are DAs */
					GetItem(GetMHandle(mApple), menuItem, daName);
					daRefNum = OpenDeskAcc(daName);
					break;
			}
			break;
		case mFile:
			switch(menuItem) {
				case iNewGame:
					NewBreakoutGame((GEWorldPtr) GetWRefCon(gAnimWindow));
					ActivateWorld((GEWorldPtr) GetWRefCon(gAnimWindow), true);
					ShowWindow(gAnimWindow);
					break;
				case iQuit:
					gFinished = true;
			}
			break;
		case mEdit:
			(void) SystemEdit(menuItem-1);
			break;
		case mSpecial:
			//Add special items
			switch ( menuItem ) {
				case iSingleFrame:
					gSingleFrame = !gSingleFrame;
					break;
			}
			break;
	}
	HiliteMenu(0);
}

//Utility routines


void AdjustCursor( void )
{
	WindowPtr	window;
	
	window = FrontWindow();
	
	if ( (window == gAnimWindow) && (!gInBackground) ) {
		SetCursor(&qd.arrow);
	}
}

