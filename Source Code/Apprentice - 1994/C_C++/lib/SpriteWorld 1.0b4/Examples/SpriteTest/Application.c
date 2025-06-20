///--------------------------------------------------------------------------------------
//	Application.c
//
//	Created:	Sunday, April 11, 1993
//	By:			Tony Myles
//
//	Copyright: � 1993-94 Tony Myles, All rights reserved worldwide.
///--------------------------------------------------------------------------------------


#if THINK_C
#ifndef __BDC__
#include <BDC.h>
#endif
#else
#ifndef __PACKAGES__
#include <Packages.h>
#endif
#endif

#ifndef __APPLEEVENTS__
#include <AppleEvents.h>
#endif

#ifndef __OSUTILS__
#include <OSUtils.h>
#endif

#ifndef __DESK__
#include <Desk.h>
#endif

#ifndef __DIALOGS__
#include <Dialogs.h>
#endif

#ifndef __DISKINIT__
#include <DiskInit.h>
#endif

#ifndef __EPPC__
#include <EPPC.h>
#endif

#ifndef __EVENTS__
#include <Events.h>
#endif

#ifndef __ERRORS__
#include <Errors.h>
#endif

#ifndef __FONTS__
#include <Fonts.h>
#endif

#ifndef __GESTALTEQU__
#include <GestaltEqu.h>
#endif

#ifndef __MENUS__
#include <Menus.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __OSEVENTS__
#include <OSEvents.h>
#endif

#ifndef __TEXTEDIT__
#include <TextEdit.h>
#endif

#ifndef __TRAPS__
#include <Traps.h>
#endif

#ifndef __TOOLUTILS__
#include <ToolUtils.h>
#endif

#ifndef __WINDOWS__
#include <Windows.h>
#endif

#ifndef __SPRITEWORLD__
#include "SpriteWorld.h"
#endif

#ifndef __SPRITEWORLDUTILS__
#include "SpriteWorldUtils.h"
#endif

#ifndef __APPLICATION__
#include "Application.h"
#endif

#ifndef __SPRITETEST__
#include "SpriteTest.h"
#endif

#ifndef __ABOUT__
#include "About.h"
#endif


Boolean gIsRunning = true;
Boolean gInBackGround = false;
Boolean gHasWaitNextEvent = false;
WindowPtr gWindowP = NULL;
SpriteTestPtr gSpriteTestP = NULL;


void main(void)
{
	OSErr err = noErr;

	if (Initialize(kNumberOfMoreMastersCalls))
	{
		if (CheckSystem() && CheckMemory())
		{
			if (HasAppleEvents())
			{
				err = InstallAppleEventHandlers();
	
				if (err != noErr)
				{
					ErrorAlert(err, kUnknownErrorStringIndex);
				}
			}
	
			CreateMenuBar();
			CreateWindow();
	
			if (EnterApplication())
			{
				ServiceEvents();
			}
	
			ExitApplication();
		}
	}

	ExitToShell();
}


Boolean Initialize(
	short numberOfMasters)
{
	OSErr err = noErr;
	EventRecord tempEvent;

	if (kStackNeeded > StackSpace())
	{
			// new address is heap size + current stack - needed stack
		SetApplLimit((Ptr)((long)GetApplLimit() - kStackNeeded + StackSpace()));

		err = MemError();
	}

	if (err == noErr)
	{
		MaxApplZone();

		while ((err == noErr) && (numberOfMasters--))
		{
			MoreMasters();

			err = MemError();
		}
	}

	if (err == noErr)
	{
		InitGraf(&qd.thePort);
		InitFonts();
		InitWindows();
		InitMenus();
		TEInit();
		InitDialogs(NULL);
		InitCursor();
		FlushEvents(everyEvent, 0);

		(void)EventAvail(everyEvent, &tempEvent);
		(void)EventAvail(everyEvent, &tempEvent);
		(void)EventAvail(everyEvent, &tempEvent);

		gHasWaitNextEvent = HasWaitNextEvent();
	}

		// if we get an error here, we can�t do jack
		// don�t even TRY to put up an alert

	return err == noErr;
}


Boolean CheckSystem(void)
{
	OSErr	err;
	Boolean isSystemGood = true;
	long	gestaltResult;

	err = Gestalt(gestaltTimeMgrVersion, &gestaltResult);

	isSystemGood = (err == noErr) && (gestaltResult >= gestaltStandardTimeMgr);

	if (!isSystemGood)
	{
		CantRunOnThisMachine();
	}

	return isSystemGood;
}


Boolean CheckMemory(void)
{
	Boolean isEnoughMemory;
	long heapNeeded, heapSize;
	GDHandle mainGDeviceH;
	Rect worldRect;
	short pixelSize;

	if (SWHasColorQuickDraw())
	{
		mainGDeviceH = GetMainDevice();

		worldRect = (**mainGDeviceH).gdRect;
		pixelSize = (**(**mainGDeviceH).gdPMap).pixelSize;
	}
	else
	{
		worldRect = qd.screenBits.bounds;
		pixelSize = 1;
	}

	heapNeeded = 3 * ((((worldRect.right - worldRect.left) *
							(worldRect.bottom - worldRect.top)) * pixelSize) / 8);
	heapNeeded += 100000;

	heapSize = (long)GetApplLimit() - (long)ApplicZone();

	isEnoughMemory = (heapSize > heapNeeded);

	if (!isEnoughMemory)
	{
		ErrorAlert(memFullErr, kNotEnoughMemoryStringIndex);
	}

	return isEnoughMemory;
}


Boolean HasAppleEvents(void)
{
	Boolean hasAppleEvents;
	OSErr err;
	long gestaltResult;

	err = Gestalt(gestaltAppleEventsAttr, &gestaltResult);

	if (err == noErr)
	{
		hasAppleEvents = (gestaltResult & (1 << gestaltAppleEventsPresent)) != 0;
	}
	else
	{
		hasAppleEvents = false;
	}

	return hasAppleEvents;
}


#ifndef NewAEEventHandlerProc
#define NewAEEventHandlerProc(x) ((EventHandlerProcPtr)x)
#endif 

OSErr InstallAppleEventHandlers(void)
{
	OSErr err = noErr;

	err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, NewAEEventHandlerProc(HandleOpenApp), 0, false);

	if (err == noErr)
	{
		err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, NewAEEventHandlerProc(HandleOpenDoc), 0, false);
	}

	if (err == noErr)
	{
		err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, NewAEEventHandlerProc(HandlePrintDoc), 0, false);
	}

	if (err == noErr)
	{
		err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, NewAEEventHandlerProc(HandleQuit), 0, false);
	}

	return err;
}


void CreateMenuBar(void)
{
	Handle menuBarH;

	menuBarH = GetNewMBar(kMenuBarResID);

	if (menuBarH != NULL)
	{
		SetMenuBar(menuBarH);
		AddResMenu(GetMHandle(kAppleMenuID), 'DRVR');
		DrawMenuBar();
	}
	else
	{
		CantFindResource();
	}
}


void CreateWindow(void)
{
	gWindowP = SWHasColorQuickDraw() ?
			GetNewCWindow(kWindowResID, NULL, (WindowPtr)-1L) :
			GetNewWindow(kWindowResID, NULL, (WindowPtr)-1L);

	if (gWindowP != NULL)
	{
		SizeWindow(gWindowP, qd.screenBits.bounds.right, qd.screenBits.bounds.bottom, false);
		MoveWindow(gWindowP, 0, 0, false);
	}
	else
	{
		CantFindResource();
	}
}


Boolean EnterApplication(void)
{
	OSErr	err;

	err = SWEnterSpriteWorld();

	if (err == noErr)
	{
		err = CreateSpriteTest(&gSpriteTestP, (CWindowPtr)gWindowP);
	}

	if (err == noErr)
	{
		ShowWindow(gWindowP);
		UpdateSpriteTest(gSpriteTestP);
		ValidRect(&gWindowP->portRect);
	}

	if (err != noErr)
	{
		ErrorAlert(err, kUnknownErrorStringIndex);
	}

	return err == noErr;
}


void ExitApplication(void)
{
	DisposeSpriteTest(gSpriteTestP);

	SWExitSpriteWorld();

	DisposeWindow(gWindowP);
}


void ServiceEvents(void)
{
	Boolean haveEvent;
	EventRecord event;
	long sleepTime;
	RgnHandle mouseRgn = gHasWaitNextEvent ? NewRgn() : NULL;

	while (gIsRunning)
	{
		if (gHasWaitNextEvent)
		{
			sleepTime = gInBackGround ? kBackGroundSleepTime : kForeGroundSleepTime;

			haveEvent = WaitNextEvent(everyEvent, &event, sleepTime, mouseRgn);
		}
		else
		{
			SystemTask();
			haveEvent = GetNextEvent(everyEvent, &event);
		}

		if (haveEvent)
		{
			DispatchEvent(&event);
		}
		else
		{
			HandleNullEvent();
		}
	}

	if (mouseRgn != NULL)
	{
		DisposeRgn(mouseRgn);
	}
}


void DispatchEvent(
	EventRecord* event)
{
	switch(event->what)
	{
		case mouseDown:
			HandleMouseEvent(event);
			break;
		case mouseUp:
			break;
		case keyUp:
			break;
		case keyDown:
		case autoKey:
			HandleKeyEvent((char)(event->message & charCodeMask), event->modifiers);
			break;
		case updateEvt:
			HandleUpdateEvent((WindowPtr)event->message);
			break;
		case diskEvt:
			HandleDiskEvent(event->message);
			break;
		case activateEvt:
			HandleActivateEvent((WindowPtr)event->message);
			break;
		case networkEvt:
			break;
		case driverEvt:
			break;
		case app1Evt:
			break;
		case app2Evt:
			break;
		case app3Evt:
			break;
		case osEvt:
			HandleOSEvent(event->message);
			break;
		case kHighLevelEvent:
			AEProcessAppleEvent(event);
			break;
		default:
		break;
	}
}


void HandleMouseEvent(
	EventRecord* event)
{
	WindowPtr whichWindow;
	short partCode;

	partCode = FindWindow(event->where, &whichWindow);

	switch (partCode)
	{
		case inDesk:		
			break;
		case inMenuBar:
			AdjustMenuItems();
			HandleMenuCommand(MenuSelect(event->where));
			break;
		case inSysWindow:
			SystemClick(event, whichWindow);
			break;
		case inContent:
			break;
		case inDrag:
			break;
		case inGrow:
			break;
		case inGoAway:
			break;
		case inZoomIn:
		case inZoomOut:
			break;
		default:			
			break;
	}
}


void HandleKeyEvent(
	char key,
	short modifiers)
{
	if ((modifiers & cmdKey) != 0)
	{
		AdjustMenuItems();
		HandleMenuCommand(MenuKey(key));
	}
	else
	{
			// if the spacebar is hit, fill the screen with black
			// useful for debugging purposes
		if (key == ' ')
		{
			#if dangerousPattern
			FillRect(&gWindowP->portRect, qd.black);
			#else
			FillRect(&gWindowP->portRect, &qd.black);
			#endif
		}
	}
}


void HandleUpdateEvent(
	WindowPtr updateWindowP)
{
	if (updateWindowP == gWindowP)
	{
		SetPort(updateWindowP);
		BeginUpdate(updateWindowP);

		UpdateSpriteTest(gSpriteTestP);

		EndUpdate(updateWindowP);
	}
}


void HandleActivateEvent(
	WindowPtr updateWindowP)
{
#if MPW
#pragma unused(updateWindowP)
#endif
}


void HandleOSEvent(
	long message)
{
	if ((message >> 24) == suspendResumeMessage)
	{
		if ((message & resumeFlag) != 0)
		{
			gInBackGround = false;
		}
		else
		{
			gInBackGround = true;
		}
	}
}


void HandleDiskEvent(
	long message)
{
	OSErr err;
	Point dialogLocation = {100, 100};

	if ((message & 0xFFFF0000) != noErr)
	{
		err = DIBadMount(dialogLocation, message);

		if (err != noErr)
		{
			ErrorAlert(err, kUnknownErrorStringIndex);
		}
	}
}


void HandleNullEvent(void)
{
	RunSpriteTest(gSpriteTestP);
}


void HandleMenuCommand(
	long menuItemIdentifier)
{
	short menuIdent = HiWord(menuItemIdentifier);
	short menuItem = LoWord(menuItemIdentifier);

	switch (menuIdent)
	{
		case kAppleMenuID:
		{
			HandleAppleMenuCommand(menuItem);
			break;
		}

		case kFileMenuID:
		{
			HandleFileMenuCommand(menuItem);
			break;
		}

		case kEditMenuID:
		{
			HandleEditMenuCommand(menuItem);
			break;
		}

		case kSpriteMenuID:
		{
			HandleSpriteMenuCommand(menuItem);
			break;
		}

		case kTestMenuID:
		{
			HandleTestMenuCommand(menuItem);
			break;
		}
	}

	HiliteMenu(0);
}


void HandleAppleMenuCommand(
	short menuItem)
{
	Str255 deskAccName;

	switch (menuItem)
	{
		case kAboutItem:
		{
			DisplayAboutBox();
			break;
		}

		default:
		{
			GetItem(GetMHandle(kAppleMenuID), menuItem, deskAccName);
			OpenDeskAcc(deskAccName);
			break;
		}
	}
}


void HandleFileMenuCommand(
	short menuItem)
{
	switch (menuItem)
	{
		case kQuitItem:
		{
			gIsRunning = false;
			break;
		}
	}
}


void HandleEditMenuCommand(
	short menuItem)
{
	(void)SystemEdit(menuItem);
}


void HandleSpriteMenuCommand(
	short menuItem)
{
	switch (menuItem)
	{
		case kCreateSpriteItem:
		{
			HandleCreateSpriteCommand(gSpriteTestP);
			break;
		}

		case kCompileSpriteItem:
		{
			CompileSpriteCommand(gSpriteTestP);
			break;
		}

		case kHideTitleItem:
		{
			HandleSpriteTestTitleCommand(gSpriteTestP);
			break;
		}

		case kHideBallsItem:
		{
			HandleBouncingBallsCommand(gSpriteTestP);
			break;
		}

	}
}


void HandleTestMenuCommand(
	short menuItem)
{
	switch (menuItem)
	{
		case kTestItem:
		{
			if (gSpriteTestP->isCommandActive[kCopyBitsTestCommand])
				CopyBitsSpeedTestCommand(gSpriteTestP);
			else if (gSpriteTestP->isCommandActive[kBlitPixieTestCommand])
				BlitPixieSpeedTestCommand(gSpriteTestP);
			else
				RunCompiledSpriteCommand(gSpriteTestP, false);
			break;
		}

		case kCopyBitsTestItem:
		{
			gSpriteTestP->isCommandActive[kCopyBitsTestCommand] = true;
			gSpriteTestP->isCommandActive[kBlitPixieTestCommand] = false;
			gSpriteTestP->isCommandActive[kRunCompiledSpriteCommand] = false;
			break;
		}

		case kBlitPixieTestItem:
		{
			gSpriteTestP->isCommandActive[kCopyBitsTestCommand] = false;
			gSpriteTestP->isCommandActive[kBlitPixieTestCommand] = true;
			gSpriteTestP->isCommandActive[kRunCompiledSpriteCommand] = false;
			break;
		}

		case kRunCompiledSpriteItem:
		{
			gSpriteTestP->isCommandActive[kCopyBitsTestCommand] = false;
			gSpriteTestP->isCommandActive[kBlitPixieTestCommand] = false;
			gSpriteTestP->isCommandActive[kRunCompiledSpriteCommand] = true;
			break;
		}

		case kCollisionDetectionItem:
		{
			gSpriteTestP->isCommandActive[kCollisionDetectionCommand] =
					!gSpriteTestP->isCommandActive[kCollisionDetectionCommand];
			break;
		}
	}
}


void AdjustMenuItems(void)
{
	MenuHandle tempMenuH;

		// adjust the test menu
	tempMenuH = GetMHandle(kSpriteMenuID);

	CheckItem(tempMenuH, kHideTitleItem, gSpriteTestP->isCommandActive[kSpriteTestTitleCommand]);
	CheckItem(tempMenuH, kHideBallsItem, gSpriteTestP->isCommandActive[kBouncingBallsCommand]);

		// adjust the sprite menu
	tempMenuH = GetMHandle(kTestMenuID);

	CheckItem(tempMenuH, kCopyBitsTestItem, gSpriteTestP->isCommandActive[kCopyBitsTestCommand]);
	CheckItem(tempMenuH, kBlitPixieTestItem, gSpriteTestP->isCommandActive[kBlitPixieTestCommand]);
	CheckItem(tempMenuH, kRunCompiledSpriteItem, gSpriteTestP->isCommandActive[kRunCompiledSpriteCommand]);
	CheckItem(tempMenuH, kCollisionDetectionItem, gSpriteTestP->isCommandActive[kCollisionDetectionCommand]);

	if ((!SWHasColorQuickDraw()) ||
		((**gSpriteTestP->spriteWorldP->backFrameP->framePort.colorGrafP->portPixMap).pixelSize != 8))
	{
			// disable some of the tests if we are not in 8 bit
		DisableItem(tempMenuH, kBlitPixieTestItem);
		DisableItem(tempMenuH, kRunCompiledSpriteItem);
	}

#if defined(powerc) || defined(__powerc)
	DisableItem(tempMenuH, kRunCompiledSpriteItem);
#endif
}


pascal OSErr HandleOpenApp(
	AppleEvent srcAppleEvent,
	AppleEvent replyAppleEvent,
	long refCon)
{
#if MPW
#pragma unused(srcAppleEvent,replyAppleEvent,refCon)
#endif

	return noErr;
}


pascal OSErr HandleOpenDoc(
	AppleEvent srcAppleEvent,
	AppleEvent replyAppleEvent,
	long refCon)
{
#if MPW
#pragma unused(srcAppleEvent,replyAppleEvent,refCon)
#endif
	return errAEEventNotHandled;
}


pascal OSErr HandlePrintDoc(
	AppleEvent srcAppleEvent,
	AppleEvent replyAppleEvent,
	long refCon)
{
#if MPW
#pragma unused(srcAppleEvent,replyAppleEvent,refCon)
#endif
	return errAEEventNotHandled;
}


pascal OSErr HandleQuit(
	AppleEvent srcAppleEvent,
	AppleEvent replyAppleEvent,
	long refCon)
{
#if MPW
#pragma unused(srcAppleEvent,replyAppleEvent,refCon)
#endif
	gIsRunning = false;

	return noErr;
}


void ErrorAlert(
	OSErr err,
	short errorStringIndex)
{
	Str255 messageString, errorString;

		// make sure we don't know what this error is...
	if (errorStringIndex == kUnknownErrorStringIndex)
	{
		switch (err)
		{
			case memFullErr:
				errorStringIndex = kNotEnoughMemoryStringIndex;
				break;

			case resNotFound:
				errorStringIndex = kCantFindResourceStringIndex;
				break;
		}
	}

	GetIndString(messageString, kErrorStringListResID, errorStringIndex);

	if (messageString[0] == 0)
	{
		BlockMove(kSeriousDamageString, messageString, sizeof(kSeriousDamageString));
	}

	NumToString(err, errorString);

	ParamText(messageString, errorString, "\p", "\p");

	(void)StopAlert(kErrorAlertResID, NULL);
}


void CantFindResource(void)
{
	OSErr err;
	
	err = ResError();
	
	if (err == noErr)
	{
		err = resNotFound;
	}

	ErrorAlert(err, kCantFindResourceStringIndex);

	ExitToShell();
}


void CantRunOnThisMachine(void)
{
	(void)StopAlert(kCantRunAlertResID, NULL);
}


short NumToolboxTraps(void)
{
	return	(NGetTrapAddress(_InitGraf, ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
				?	0x0200 :	0x0400;
}


TrapType GetTrapType(
	short trap)
{
	#define TrapMask 0x0800

	return ((trap & TrapMask) != 0) ? ToolTrap : OSTrap;
}


Boolean TrapAvail(
	short trap)
{
	TrapType	tType;

	tType = GetTrapType(trap);
	if (tType == ToolTrap)
	{
		trap = trap & 0x07FF;
	}

	if (trap >= NumToolboxTraps())
	{
		trap = _Unimplemented;
	}

	return NGetTrapAddress(trap, tType) !=
			 NGetTrapAddress(_Unimplemented, ToolTrap);
}


Boolean HasWaitNextEvent(void)
{
	return TrapAvail(_WaitNextEvent);
}


