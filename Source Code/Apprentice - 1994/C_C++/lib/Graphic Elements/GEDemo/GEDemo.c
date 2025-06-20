/*
	GEDemo.c
	
	Demo program for Graphic Elements library
	
	11/1/93
	
	Al Evans
*/

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
#include "Sensors.h"
#include "Cannon.h"
#include "Walk.h"
#include "Pogo.h"
#include "Sign.h"
#include "Meter.h"
#include "Grabber.h"

//define HISTOGRAM to collect times history
#undef HISTOGRAM

#ifdef HISTOGRAM
#include "StdIO.h"
#endif

//define PERFORMANCE to collect profiling data
#undef PERFORMANCE
#ifdef PERFORMANCE
#include <Perf.h>
#endif


//Menu Commands

#define	rMenuBar				128		/* application's menu bar */

#define	mApple					128		/* Apple menu */
#define	iAbout					1

#define	mFile					129		/* File menu */
#define	iQuit					1

#define	mEdit					130		/* Edit menu */
#define	iUndo					1
#define	iCut					3
#define	iCopy					4
#define	iPaste					5
#define	iClear					6

#define mSpecial				131
#define iShoot					1
#define iSingleFrame			2
#define iSignText				3
#define iSpeed					4
#define iGrabber				5


#define	rAboutDialog	228		
#define rUserAlert		129
#define rSetTextDialog	135
#define rFrameRateDialog 130
#define rSpeedDialog	131

//Resource numbers of graphic PICTs
#define	rBrickPic 200

//Number of master pointer blocks needed

#define masterBlocksNeeded 10

//Globals

#ifdef PERFORMANCE
TP2PerfGlobals		thePGlobals;
#endif

Boolean		gFinished;
WindowPtr	gAnimWindow;
Boolean		gSingleFrame = false;
Boolean		gDoOne = false;

//Performance measurement

#define thirtySeconds 30L * 1000 * 1000		//�sec for timers -- never time out

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
	
	return NewCWindow(nil, &windRect, "\pAnimation Demo", false, documentProc,
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

Boolean LoadBackground(GEWorldPtr world)
{
	GrafElPtr	bkg;
	if (bkg = NewBasicPICT(world, 'BKG ', 1, rBrickPic, srcCopy, 0, 0))
		return true;
	return false;
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
	if (gAnimWindow = MakeWindow(512, 364)){
		animRect.left = 0;
		animRect.right = 512;
		animRect.top = 0;
		animRect.bottom = 364;
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
	
	//Load graphics
	
	if (!LoadBackground((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load background", 0);
		return false;
	}
	if (!LoadCannonScene((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load cannon scene", 0);
		return false;
	}
	
	if (!LoadBalconyScene((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load animated walk", 0);
		return false;
	}
	
	if (!LoadPogoScene((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load pogo stick", 0);
		return false;
	}
	
	if (!LoadSignScene((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load sign", 0);
		return false;
	}
	
	if (!LoadUsageMeterScene((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load usage meter", 0);
		return false;
	}
	if (!MakeGrabber((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not create grabber", 0);
		return false;
	}
	
#ifdef HISTOGRAM
	for (count = 0; count < 100; count++) {
		animTimes[count] = 0;
		mainTimes[count] = 0;
	}
#endif
	
#ifdef PERFORMANCE
	thePGlobals = nil;
	if (!InitPerf( &thePGlobals, 10, 8, true, true, "\pCODE", 0, "\p", false, 0L, 0L, 0) ){
		TellUser("\pCould not initialize profiling", 0);
		return false;
	}
#endif
	
	//Turn animation on and show window
	ActivateWorld((GEWorldPtr) GetWRefCon(gAnimWindow), true);
	ShowWindow(gAnimWindow);
	
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
#ifdef PERFORMANCE
	OSErr err;
#endif
	//Release system resources
	
#ifdef HISTOGRAM
	DumpHistograms();
#endif
#ifdef PERFORMANCE
	err = PerfDump(thePGlobals, "\pPerform.out", false, 0);
#endif
	StopGETimer((GEWorldPtr) GetWRefCon(gAnimWindow));
	ExitToShell();
}

#ifdef HISTOGRAM

void RunMeterAnimation(void)
{
	static short	setting = 0;
	short			thisSetting;
	if ((gTotalTime - gAnimTime) < 100)
		mainTimes[(gTotalTime - gAnimTime)]++;
	else
		mainTimes[99]++;
	if (gAnimTime < 100)
		animTimes[gAnimTime]++;
	else
		animTimes[99]++;
	
	if (gTotalTime == 0) gTotalTime = 1;
	thisSetting = (gAnimTime * 100) / gTotalTime;
	if (thisSetting == setting)
		return;
	if (thisSetting > setting)
		setting++;
	else
		if (thisSetting < setting)
			setting--;
	SetMeterReading((GEWorldPtr) GetWRefCon(gAnimWindow), setting);
}	

#else

void RunMeterAnimation(void)
{
	static short	setting = 0;
	short			thisSetting;
	
	if (gTotalTime == 0) gTotalTime = 1;
	thisSetting = (gAnimTime * 100) / gTotalTime;
	if (thisSetting == setting)
		return;
	if (thisSetting > setting)
		setting++;
	else
		if (thisSetting < setting)
			setting--;
	SetMeterReading((GEWorldPtr) GetWRefCon(gAnimWindow), setting);
	
}
#endif

void DoTimingLoop(void)
{
}

pascal Boolean SignFilter(DialogPtr dialog, EventRecord *event, short *item)
{
#pragma unused (dialog)

TEHandle	text;
	
	if ((event->what == keyDown) || (event->what == autoKey)) {
		switch (event->message & charCodeMask) {
		case 0x0d:
		case 0x03:
			*item = ok;
			return true;
			break;
		case 0x1b:
			*item = cancel;
			return true;
			break;
		default:
			text = ((DialogPeek) dialog)->textH;
			//If 15 chars already and trying to insert
			if ((GetHandleSize((**text).hText) > 14) && ((**text).selStart == (**text).selEnd)) {
				SysBeep(3);
				*item = 4;
				return true;
			}
			else
				return false;
		}
	}
	else return false;
}

void DoSetSignText( void )
{
	DialogPtr 	sgnDialog;
	short		itemType;
	Handle		item;
	Rect		okRect;
	Rect		itemRect;
	short		itemHit;
	Str255		signText;
	
	sgnDialog = GetNewDialog(rSetTextDialog, nil, (WindowPtr) -1L);
	GetDItem(sgnDialog, ok, &itemType, &item, &okRect);
	GetDItem(sgnDialog, 4, &itemType, &item, &itemRect);
	GetSignText(signText);
	SetIText(item, signText);
	InsetRect(&okRect, -4, -4);
	ShowWindow(sgnDialog);
	SetPort( (GrafPtr) sgnDialog);
	PenSize(3, 3);
	FrameRoundRect(&okRect, 16, 16);
	SelIText(sgnDialog, 4, 0, 32767);
	do
		ModalDialog(SignFilter, &itemHit);
	while ( (itemHit != ok) && (itemHit != cancel) );
	if (itemHit == ok) {
		GetIText(item, signText);
		SetSignText((GEWorldPtr) GetWRefCon(gAnimWindow), signText);
	}
	
	DisposDialog(sgnDialog);
}

void DoSetSpeed( void )
{
	DialogPtr spdDialog;
	short		itemType;
	Handle		item;
	Rect		okRect;
	Rect		itemRect;
	short		itemHit;
	Str255		tmrSpdStr;
	long		timerSpeed;
	
	spdDialog = GetNewDialog(rSpeedDialog, nil, (WindowPtr) -1L);
	GetDItem(spdDialog, ok, &itemType, &item, &okRect);
	GetDItem(spdDialog, 4, &itemType, &item, &itemRect);
	timerSpeed = GetGETimerRate((GEWorldPtr) GetWRefCon(gAnimWindow));
	timerSpeed = (100 * timerSpeed) >> 16;
	NumToString( (long) timerSpeed, tmrSpdStr);
	SetIText(item, tmrSpdStr);
	InsetRect(&okRect, -4, -4);
	ShowWindow(spdDialog);
	SetPort( (GrafPtr) spdDialog);
	PenSize(3, 3);
	FrameRoundRect(&okRect, 16, 16);
	SelIText(spdDialog, 4, 0, 32767);
	do
		ModalDialog(nil, &itemHit);
	while ( (itemHit != ok) && (itemHit != cancel) );
	if (itemHit == ok) {
		GetIText(item, tmrSpdStr);
		StringToNum(tmrSpdStr, &timerSpeed);
		timerSpeed = (timerSpeed << 16) / 100;
		SetGETimerRate((GEWorldPtr) GetWRefCon(gAnimWindow), timerSpeed);
	}
	
	DisposDialog(spdDialog);
}

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
		InsTime( (QElemPtr) &gTTimeTask);
		InsTime( (QElemPtr) &gATimeTask);
		PrimeTime( (QElemPtr) &gTTimeTask, -thirtySeconds);
		if (!gSingleFrame || gDoOne) {
			PrimeTime( (QElemPtr) &gATimeTask, -thirtySeconds);
			
#ifdef PERFORMANCE
			(void) PerfControl(thePGlobals, true);
#endif

			DoWorldUpdate((GEWorldPtr) GetWRefCon(gAnimWindow), false);
			
#ifdef PERFORMANCE
			(void) PerfControl(thePGlobals, false);
#endif
			RmvTime( (QElemPtr) &gATimeTask);
			gAnimTime = (thirtySeconds + gATimeTask.tmCount) / 1000;
			gDoOne = false;
		}
		AdjustCursor();
		if (gotEvent = WaitNextEvent(everyEvent, &event, 0L, nil)) 
			DoEvent(&event);
		RmvTime( (QElemPtr) &gTTimeTask);
		gTotalTime = (thirtySeconds + gTTimeTask.tmCount) / 1000; //milliseconds
		RunMeterAnimation();
		
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
					StopGETimer((GEWorldPtr) GetWRefCon(gAnimWindow));
					AdjustMenus();
					DoMenuCommand(MenuSelect(event->where));
					StartGETimer((GEWorldPtr) GetWRefCon(gAnimWindow));
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
				StopGETimer((GEWorldPtr) GetWRefCon(gAnimWindow));
				AdjustMenus();
				DoMenuCommand(MenuKey(key));
				StartGETimer((GEWorldPtr) GetWRefCon(gAnimWindow));
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
#pragma unused (becomingActive)
	//Could start and stop animation here
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
	CheckItem(menu, iGrabber, GrabberActive((GEWorldPtr) GetWRefCon(gAnimWindow)));
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
			if (menuItem == iQuit)
				gFinished = true;
			break;
		case mEdit:
			(void) SystemEdit(menuItem-1);
			break;
		case mSpecial:
			//Add special items
			switch ( menuItem ) {
				case iShoot:
					ShootCannon((GEWorldPtr) GetWRefCon(gAnimWindow), sensorOn);
					break;
				case iSingleFrame:
					gSingleFrame = !gSingleFrame;
					break;
				case iSignText:
					DoSetSignText();
					break;
				case iSpeed:
					DoSetSpeed();
					break;
				case iGrabber:
					ActivateGrabber((GEWorldPtr) GetWRefCon(gAnimWindow), 
									!GrabberActive((GEWorldPtr) GetWRefCon(gAnimWindow)));
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

