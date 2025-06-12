/*
	SynchTest.c
	
	Test program for exploring relationship between number of objects, object move,
	object update interval, and frame update interval
	
	1/29/94
	
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
#include "SynchGraphic.h"
#include "Meter.h"



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
#define iSetParams				1
#define iMeter					2
#define iTiming					3

#define	rAboutDialog	228		
#define rUserAlert		129
#define rPerfAlert		300
#define rSetParamsDialog	135

//Resource numbers of graphic PICTs
#define	rBkgPic 200

//Number of master pointer blocks needed

#define masterBlocksNeeded 10

//Globals

Boolean		gFinished;
WindowPtr	gAnimWindow;
Boolean		gSingleFrame = false;
Boolean		gDoOne = false;
Boolean		gMeterShown = true;

//Performance measurement

#define thirtySeconds 30L * 1000 * 1000		//µsec for timers -- never time out

TMTask	gTTimeTask, gATimeTask;
unsigned long gTotalTime = 0, gAnimTime = 0;


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
	
	return NewCWindow(nil, &windRect, "\pSynch Test", false, documentProc,
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
	if (bkg = NewBasicPICT(world, 'BKG ', 1, rBkgPic, srcCopy, 0, 0))
		return true;
	return false;
}

Boolean Initialize(void)
{
	Rect		animRect;
	GEWorldPtr	animWorld;
	
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
	if (!InitObjectGraphics((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load test graphic", 0);
		return false;
	}

	if (!LoadUsageMeterScene((GEWorldPtr) GetWRefCon(gAnimWindow))) {
		TellUser("\pCould not load usage meter", 0);
		return false;
	}
	//Reposition meter a little to the left
	MoveElement((GEWorldPtr) GetWRefCon(gAnimWindow), meterBkgID, -100, 0);
	
	
	//Turn animation on and show window
	SetNumberOfObjects((GEWorldPtr) GetWRefCon(gAnimWindow), 4);
	ActivateWorld((GEWorldPtr) GetWRefCon(gAnimWindow), true);
	ShowWindow(gAnimWindow);
	
}


void Shutdown(void)
{
	//Release system resources
	
	StopGETimer((GEWorldPtr) GetWRefCon(gAnimWindow));
	ExitToShell();
}



void RunMeterAnimation(short ms)
{
	static short	setting = 0;
	
	if (gMeterShown && (ms > 1)) {
		if (ms == setting)
			return;
		if (ms > setting)
			setting++;
		else
			if (ms < setting)
				setting--;
		SetMeterReading((GEWorldPtr) GetWRefCon(gAnimWindow), 2 * setting);
	}
	
}

void DisplayPerformance(long frames, long seconds)
{
	Str255 framesString, secondsString, fpsString;
	long fps;
	
	NumToString(frames, framesString);
	NumToString(seconds, secondsString);

	fps = (seconds > 0) ? frames / seconds : frames;
	NumToString(fps, fpsString);

	ParamText(framesString, secondsString, fpsString, "\p");
	NoteAlert(rPerfAlert, nil);
}

void DoTimingLoop(void)
{
	long ticks;
	short oObjUpdate;
	unsigned long frames, seconds;
	GEWorldPtr	world = (GEWorldPtr) GetWRefCon(gAnimWindow);
	
	oObjUpdate = GetUpdateInterval();
	SetUpdateInterval(world, 1);
	StartGETimer(world);
	ticks = TickCount();
	for (frames = 0; ((TickCount() - ticks) < (30 * 60)) && (!Button()); frames++) { 
		DoWorldUpdate(world, false);
	}
	
	seconds = ((TickCount() - ticks) / 60);
	
	DisplayPerformance(frames, seconds);
	//SetUpdateInterval(world, oPIntrvl);
	SetUpdateInterval(world, oObjUpdate);
	
}

//Dialog items
#define	dItmNObj	4
#define dItmMDist	6
#define dItmSep		8
#define dItmUpd		10
#define dItmLinear	11
#define dItmRandom	12
#define dItmCollide	13

void DoSetParams( void )
{

	DialogPtr 	frDialog;
	short		itemType;
	Handle		item;
	Rect		okRect;
	Rect		itemRect;
	short		itemHit;
	Str255		tempStr;
	ControlHandle	btnRandom, btnLinear, boxCollide;
	
	long		oNumObj, nNumObj;
	long		oMoveDist, nMoveDist;
	long		oSep, nSep;
	long		oObjUpdate, nObjUpdate;
	Boolean		oMovement, nMovement;	//linear (true) or random
	Boolean		collision;				//if random motion, true == collision enabled
	
	frDialog = GetNewDialog(rSetParamsDialog, nil, (WindowPtr) -1L);
	GetDItem(frDialog, ok, &itemType, &item, &okRect);
	
	//Present number of objects -> dialog
	oNumObj = GetNumberOfObjects();
	GetDItem(frDialog, dItmNObj, &itemType, &item, &itemRect);
	NumToString(oNumObj, tempStr);
	SetIText(item, tempStr);
	
	//Present move distance -> dialog
	oMoveDist = GetMoveDistance();
	GetDItem(frDialog, dItmMDist, &itemType, &item, &itemRect);
	NumToString(oMoveDist, tempStr);
	SetIText(item, tempStr);
	
	//Present object separation -> dialog
	oSep = GetObjSeparation();
	GetDItem(frDialog, dItmSep, &itemType, &item, &itemRect);
	NumToString(oSep, tempStr);
	SetIText(item, tempStr);
	
	//Present object update interval -> dialog
	oObjUpdate = GetUpdateInterval();
	GetDItem(frDialog, dItmUpd, &itemType, &item, &itemRect);
	NumToString(oObjUpdate, tempStr);
	SetIText(item, tempStr);
	
	//Get radio buttons, present object motion -> dialog
	oMovement = MotionIsLinear();
	nMovement = oMovement;
	GetDItem(frDialog, dItmLinear,  &itemType, (Handle *) &btnLinear, &itemRect);
	GetDItem(frDialog, dItmRandom,  &itemType, (Handle *) &btnRandom, &itemRect);
	GetDItem(frDialog, dItmCollide, &itemType, (Handle *) &boxCollide, &itemRect);
	SetCtlValue(btnLinear, oMovement?1:0);
	SetCtlValue(btnRandom, oMovement?0:1);
	collision = ObjCollisionActive();
	HiliteControl(boxCollide, oMovement?255:0);
	SetCtlValue(boxCollide, collision?1:0);
	
	//Do OK button
	InsetRect(&okRect, -4, -4);
	ShowWindow(frDialog);
	SetPort( (GrafPtr) frDialog);
	PenSize(3, 3);
	FrameRoundRect(&okRect, 16, 16);
	
	SelIText(frDialog, dItmNObj, 0, 32767);
	do {
		ModalDialog(nil, &itemHit);
		switch (itemHit) {
			case dItmLinear:
				nMovement = true;
				SetCtlValue(btnLinear, 1);
				SetCtlValue(btnRandom, 0);
				HiliteControl(boxCollide, 255);
				break;
			case dItmRandom:
				nMovement = false;
				SetCtlValue(btnLinear, 0);
				SetCtlValue(btnRandom, 1);
				HiliteControl(boxCollide, 0);
				break;
			case dItmCollide:
				SetCtlValue(boxCollide, (GetCtlValue(boxCollide) ==1)?0:1);
				break;
		}
	} while ( (itemHit != ok) && (itemHit != cancel) );
	
	if (itemHit == ok) {
		//Get new values
		GetDItem(frDialog, dItmNObj, &itemType, &item, &itemRect);
		GetIText(item, tempStr);
		StringToNum(tempStr, &nNumObj);
		GetDItem(frDialog, dItmMDist, &itemType, &item, &itemRect);
		GetIText(item, tempStr);
		StringToNum(tempStr, &nMoveDist);
		GetDItem(frDialog, dItmSep, &itemType, &item, &itemRect);
		GetIText(item, tempStr);
		StringToNum(tempStr, &nSep);
		GetDItem(frDialog, dItmUpd, &itemType, &item, &itemRect);
		GetIText(item, tempStr);
		StringToNum(tempStr, &nObjUpdate);
		if (nMoveDist != oMoveDist)
			SetMoveDistance((GEWorldPtr) GetWRefCon(gAnimWindow), nMoveDist);
		if (nSep != oSep)
			SetObjSeparation((GEWorldPtr) GetWRefCon(gAnimWindow), nSep);
		if (nObjUpdate != oObjUpdate)
			SetUpdateInterval((GEWorldPtr) GetWRefCon(gAnimWindow), nObjUpdate);
		if (oMovement != nMovement)
			SetObjMotionLinear(nMovement);
		if ((nNumObj != oNumObj) || (nSep != oSep) || (oMovement != nMovement))
			SetNumberOfObjects((GEWorldPtr) GetWRefCon(gAnimWindow), nNumObj);
		if (!nMovement) {
			collision = (GetCtlValue(boxCollide) == 1)?true:false;
			SetObjCollision((GEWorldPtr) GetWRefCon(gAnimWindow), collision);
		}
	}
	DisposDialog(frDialog);
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
			
			DoWorldUpdate((GEWorldPtr) GetWRefCon(gAnimWindow), false);
			
			RmvTime( (QElemPtr) &gATimeTask);
			gAnimTime = (thirtySeconds + gATimeTask.tmCount) / 1000;
			gDoOne = false;
		}
		AdjustCursor();
		if (gotEvent = WaitNextEvent(everyEvent, &event, 0L, nil)) 
			DoEvent(&event);
		RmvTime( (QElemPtr) &gTTimeTask);
		gTotalTime = (thirtySeconds + gTTimeTask.tmCount) / 1000; //milliseconds
		RunMeterAnimation(gAnimTime);
		
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
	EnableItem(menu, iSetParams);
	EnableItem(menu, iMeter);
	CheckItem(menu, iMeter, gMeterShown);
	EnableItem(menu, iTiming);
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
				case iSetParams:
					DoSetParams();
					break;
				case iMeter:
					gMeterShown = !gMeterShown;
					ShowElement((GEWorldPtr) GetWRefCon(gAnimWindow), meterBkgID, gMeterShown);
					ShowElement((GEWorldPtr) GetWRefCon(gAnimWindow), meterIndID, gMeterShown);
					break;
				case iTiming:
					DoTimingLoop();
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

