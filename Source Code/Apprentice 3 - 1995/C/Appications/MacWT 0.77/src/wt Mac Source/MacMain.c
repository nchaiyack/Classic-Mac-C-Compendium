/*
** File:		MacMain.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
*/



#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <AppleEvents.h>
#include "AEHandler.h"
#include "Failure.h"
#include "wt.h"
#include "framebuf.h"
#include "graphics.h"
#include "MacGame.h"
#include "Constants.h"
#include "Menu.h"
#include "StringUtils.h"
#include "Init.h"

#if applec || __ppcc
#include <PLStringFuncs.h>
#endif



/**/


EventRecord	gTheEvent;
Boolean		quitting = false;
Boolean		gUseQuickDraw = true;
Boolean		gPaused = false;
Boolean		gShowFPS = true;
Boolean		gGameOn = false;
char		gWorldFileName[256];
Str15		gWTVersion = "\pUnknown";

#if	__ppcc
QDGlobals 	qd;
#endif


/**/


const unsigned char	nullStr[1] = {0};


/**/


static WindowPtr	gMainWindow;
static SysEnvRec	gTheSysEnv;
static Boolean		gInBackground = false;




/**/


static int PathNameFromDirID(int vRefNum, long dirID, Str255 fullPathName)
{
	Str255		dirName;
	DirInfo		dirInf;
	OSErr		err;

	*fullPathName = 0;

	dirInf.ioNamePtr = dirName;
	dirInf.ioDrParID = dirID;

	do {
		dirInf.ioVRefNum = vRefNum;
		dirInf.ioFDirIndex = -1; 				// -1 means use ioDrDirIDÉ
		dirInf.ioDrDirID = dirInf.ioDrParID;

		err = PBGetCatInfoSync((CInfoPBPtr)&dirInf);

		if (err == noErr) {
			dirName[++dirName[0]] = ':';

			if (dirName[0] + fullPathName[0] > 255)
				err = bdNamErr; 				// too big to eat!
			else {
				pcat(dirName, fullPathName);
				pcpy(fullPathName, dirName);
			}
		}

	} while (dirInf.ioDrDirID != fsRtDirID && err == noErr);

	return err;

}	// PathNameFromDirID


/**/




static void init(void)
{
	OSErr				err;
	Handle				mbh;
	WStateData			**wHndl;
	StandardFileReply	sfReply;
	Str255				titleStr;
	unsigned char		**versRsrc;

	MaxApplZone();					/* Expand the heap so code segments load at the top. */

	Initialize(32, kMinHeap, kMinSpace);	/* Initialize the program. */
	
	//BeginResSpinning(128);
	
	InitAppleEvents();
	DoAdjustMenus();

	UnloadSeg((Ptr)Initialize);		/* Initialize can't be in Main! */
	
	//StopSpinning();

	/* Catch as many failures as possible now... */

	err = SysEnvirons(curSysEnvVers, &gTheSysEnv);
	if (err || !gTheSysEnv.hasColorQD || gTheSysEnv.systemVersion < 0x0700 ||
							((long)GetApplLimit() - (long)ApplicZone()) < kMinMem) {
		StopAlert(rBadStartAlert, nil);
		ExitToShell();
	}

	// remember the version

	if ((versRsrc = (unsigned char **)Get1Resource('vers', 1)) != 0)
		pcpy(gWTVersion, *versRsrc + 6);

	strcpy((char *)gWorldFileName, DEFAULT_WORLD_FILE);

	// starting up with option key down ==> allow the user to use the default world file!

	if (!(gTheEvent.modifiers & optionKey)) {
		SFTypeList			sfList = {'TEXT'};
		Str255				worldName;

		StandardGetFile(nil, 1, sfList, &sfReply);

		if (sfReply.sfGood) {
			PathNameFromDirID(sfReply.sfFile.vRefNum, sfReply.sfFile.parID, worldName);
			pcpy(titleStr, sfReply.sfFile.name);
			pcat(worldName, sfReply.sfFile.name);
			p2c(worldName);
			strcpy(gWorldFileName, (char *)worldName);
		}
	}

	/* Open window... */
	gMainWindow = GetNewCWindow(rWindowID, nil, (WindowPtr)-1);
	if (!gMainWindow)
		Fail(ResError(), __FILE__, __LINE__, TRUE);
		
	SetPort(gMainWindow);
	TextFont(monaco);
	TextSize(12);
	TextMode(srcCopy);
	SizeWindow(gMainWindow, SCREEN_WIDTH, SCREEN_HEIGHT + kBottomBorder, false);

	ShowWindow(gMainWindow);

	// Kludge the zoomRects so we toggle between "micro" and "normal" sizes.

	wHndl = (WStateData **)((WindowPeek)gMainWindow)->dataHandle;
	(*wHndl)->stdState.top = (*wHndl)->userState.top;
	(*wHndl)->stdState.left = (*wHndl)->userState.left;
	(*wHndl)->stdState.bottom = (*wHndl)->stdState.top + (SCREEN_HEIGHT<<1) + kBottomBorder;
	(*wHndl)->stdState.right = (*wHndl)->stdState.left + (SCREEN_WIDTH<<1);

	gWTFTWindow = gMainWindow;
}


/**/


void TogglePause(void)
{
	Str255	aStr;

	if (gGameOn)
		{
		MenuHandle menu = GetMHandle(mFile);
		gPaused = !gPaused;
	
		GetIndString(aStr, rUtilityStrs, (gPaused) ? rStrContinue : rStrPause);
		SetItem(menu, kPause, aStr);
		}
}



/**/


int GetAndProcessEvent(void)
{
	WindowPtr		whichWindow;
	long			menuResult;
	short			partCode;
	int				result = gotNoEvent;
	char			ch;

	if (WaitNextEvent(everyEvent, &gTheEvent, gInBackground ? kBackTime : kFrontTime, nil)) {

		result = gotOtherEvent;

		switch (gTheEvent.what) {

		case mouseDown:

			switch (partCode = FindWindow(gTheEvent.where, &whichWindow)) {

			case inSysWindow:
				SystemClick(&gTheEvent, whichWindow);
				break;

			case inMenuBar:
				DoAdjustMenus();
				DoMenuCommand(MenuSelect(gTheEvent.where));
				break;

			case inDrag:
				SelectWindow(whichWindow);
				DragWindow(whichWindow, gTheEvent.where, &qd.screenBits.bounds);
				break;

			case inContent:
				if (whichWindow != FrontWindow())
					SelectWindow(whichWindow);
				break;

			case inGoAway:
				if (TrackGoAway(whichWindow, gTheEvent.where)) {
					ExitToShell();
				}
				break;

			case inZoomIn:
			case inZoomOut:
				if (TrackBox(whichWindow, gTheEvent.where, partCode)) {
					SetPort(whichWindow);
					EraseRect(&whichWindow->portRect);
					ZoomWindow(whichWindow, partCode, whichWindow == FrontWindow());
					InvalRect(&whichWindow->portRect);
				}
				break;

			default:
				break;

			}								// switch (FindWindow)
			break;


		case keyDown:
			ObscureCursor();
			// FALL THRU

		case autoKey:
			ch = gTheEvent.message;			// automagic "& charCodeMask" :)
			if (gTheEvent.modifiers & cmdKey) {
				DoAdjustMenus();
				menuResult = MenuKey(ch);
				if (menuResult & 0xFFFF0000) {
					DoMenuCommand(menuResult);
					break;		// out of this switch if it was a menu command
				}
			}
			else
			if (ch == kTab || ch == kEsc)	// tab/esc ==> pause game
				TogglePause();

			// FALL THRU

		case keyUp:				// if we ever switch these on...
			result = gotKeyEvent;
			break;


		case updateEvt:
			whichWindow = (WindowPtr)gTheEvent.message;
			BeginUpdate(whichWindow);

			if (whichWindow == gMainWindow)
				RefreshWTWindow();

			EndUpdate(whichWindow);
			break;

		case diskEvt:
			break;

		case activateEvt:
			break;

		case app4Evt:
			if ((gTheEvent.message << 31) == 0) {	// suspend event
				if (!gPaused)
					TogglePause();
				gInBackground = true;
				}
			else
				{
				gInBackground = false;
				SetPort(gMainWindow);
				}
			break;

		case kHighLevelEvent:
			AEProcessAppleEvent(&gTheEvent);
			break;

		default:
			break;
		}										// switch (gTheEvent.what)
	}

	return result;
}


/**/


void main(void)
{
	char	*myArgs[3];

	extern int WTMain(int argc, char **argv);

	// chocks away...

	init();

	myArgs[0] = "wtft";

	myArgs[1] = (char *)gWorldFileName;
	myArgs[2] = 0;

	while (GetAndProcessEvent())
		;

	WTMain(2, myArgs);
}
