///--------------------------------------------------------------------------------------
//	Application.c
//
//	Created:	Sunday, April 11, 1993
//	By:		Tony Myles
//
//	Copyright: � 1993 Tony Myles, All rights reserved worldwide.
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

#ifndef __SPRITEWORLDUTILS__
#include <SpriteWorldUtils.h>
#endif

#ifndef __APPLICATION__
#include "Application.h"
#endif

#include "Simple.h"


WindowPtr gWindowP = NULL;


void main(void)
{
	Initialize(kNumberOfMoreMastersCalls);

	if (CheckSystem())
	{
		CreateWindow();

		ShowWindow(gWindowP);

		PerformSimpleAnimation((CWindowPtr)gWindowP);

		DisposeWindow(gWindowP);
	}
	else
	{
		CantRunOnThisMachine();
	}

	ExitToShell();
}


void Initialize(short numberOfMasters)
{
	EventRecord tempEvent;

	MaxApplZone();

	while (numberOfMasters--)
	{
		MoreMasters();
	}

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


void CreateWindow(void)
{
	gWindowP = SWHasColorQuickDraw() ?
			GetNewCWindow(kWindowResID, NULL, (WindowPtr)-1L) :
			GetNewWindow(kWindowResID, NULL, (WindowPtr)-1L);

	if (gWindowP == NULL)
	{
		CantFindResource();
	}
}


void ErrorAlert(OSErr err, short errorStringIndex)
{
	Str255 messageString, errorString;

	GetIndString(messageString, kErrorStringListResID, errorStringIndex);

	if (messageString[0] == 0)
	{
		BlockMove(kSeriousDamageString, messageString, sizeof(kSeriousDamageString));
	}

	NumToString(err, errorString);

	ParamText(messageString, errorString, "\p", "\p");

	(void)StopAlert(kErrorAlertResID, NULL);
}


void FatalError(OSErr err)
{
	if (err != noErr)
	{
		ErrorAlert(err, kFatalErrorStringIndex);

		ExitToShell();
	}
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
