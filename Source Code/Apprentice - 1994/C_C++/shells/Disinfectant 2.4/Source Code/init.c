/*______________________________________________________________________

	init.c - Initialization Module
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
	
	This module takes care of program intialization.
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "rez.h"
#include "glob.h"
#include "misc.h"
#include "main.h"
#include "pref.h"
#include "help.h"
#include "abou.h"
#include "init.h"
#include "prog.h"

#pragma segment init

/*______________________________________________________________________

	MyGrowZone - Grow Zone Procedure.
	
	Entry:	cbNeeded = number of bytes needed.
_____________________________________________________________________*/


#pragma segment Main

static pascal long MyGrowZone (Size cbNeeded)

{
#pragma unused (cbNeeded)

	if (NoMemFullAlert) return 0;
	misc_Notify(requiresAttn, true);
	(void) utl_StopAlert(noMemID, nil, 0);
	ExitToShell();
};

#pragma segment init

/*______________________________________________________________________

	init_InitMem - Initialize Memory.
_____________________________________________________________________*/


#pragma segment Main

void init_InitMem (void)

{
	/* Increase stack space by 20,000 bytes */
	
	SetApplLimit(GetApplLimit() - 20000);
	
	/* Expand the application heap zone to the maximum. */
	
	MaxApplZone();
	
	/* Allocate a bunch of master pointers. */
	
	MoreMasters();
	MoreMasters();
	MoreMasters();
	MoreMasters();
};

#pragma segment init

/*______________________________________________________________________

	init_Initialize - Initialize the Program.
	
	Ths routine is put in its own segment, which is unloaded after
	it's used.
_____________________________________________________________________*/


void init_Initialize (void)

{
	Handle			rectHandle;			/* handle to rectangle list */
	short				i;						/* loop index */
	FCBPBRec			fcbBlock;			/* file info param block */
	EventRecord		event;				/* event record */
	Str255			apName;				/* name of Disinfectant application file */
	short				apRefNum;			/* refnum of Disinfectant applicaion file */
	Handle			apParam;				/* application parameters */
	FInfo				fndrInfo;			/* Finder info */
	
	/* Initialize all the managers. */

	InitGraf(&qd.thePort);
	InitFonts();
	FlushEvents(everyEvent, 0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	
	/* Establish a grow zone procedure to catch out of memory conditions. */ 
	
	CouldAlert(noMemID);
	SetGrowZone(MyGrowZone);
	
	/* Call WaitNextEvent a few times to force our window to initially appear
		in the foreground.  See TN 180. */
		
	utl_WaitNextEvent(everyEvent, &event, 0, nil);
	utl_WaitNextEvent(everyEvent, &event, 0, nil);
	
	/* Check to make sure we have HFS. */
	
	if (*(short*)FSFCBLen <= 0) {
		(void) utl_StopAlert(noHFSID, nil, 0);
		ExitToShell();
	};
	
	/* Get the ROM environment. */
	
	OldRom = utl_Rom64();
	
	/* Get the file ref nums of the system file and Sample. */
	
	SysRefNum = *((short*)SysMap);
	DfectRefNum = *((short*)CurMap);
	DfectVol = utl_GetApplVol();
	
	/* Get the system volume reference number and the directory id of
		the blessed folder. */
		
	SysVol = utl_GetSysVol();
	if (!utl_VolIsMFS(SysVol)) {
		fcbBlock.ioNamePtr = nil;
		fcbBlock.ioVRefNum = 0;
		fcbBlock.ioRefNum = SysRefNum;
		fcbBlock.ioFCBIndx = 0;
		(void) PBGetFCBInfo(&fcbBlock, false);
		SysDirID = fcbBlock.ioFCBParID;
	};
	
	/* Check to make sure all the packages we need exist.  If they don't 
		exist, put up an alert telling the user we require system 3.2 or 
		later and exit to shell. */
		
	if (!utl_CheckPack(listMgr, false) || !utl_CheckPack(stdFile, false) || 
		!utl_CheckPack(dskInit, false) || !utl_CheckPack(intUtil, false) || 
		!utl_CheckPack(bdConv, false)) {
		(void) utl_StopAlert(badSysID, nil, 0);
		ExitToShell();
	};
	
	/* Determine whether or not the system has the shutdown trap.
		(See TN 156).*/
	
	SysHasShutDown = NGetTrapAddress(_ShutDown & 0x3ff, ToolTrap) !=
		NGetTrapAddress(_Unimplemented & 0x3ff, ToolTrap);
		
	/* Get the long sleep time. */
	
	LongSleep = utl_GetLongSleep();
	
	/* Get the cursors. */
	
	Watch = GetCursor(watchCursor);
	HelpCurs = GetCursor(helpCursID);
	IBeamCurs = GetCursor(iBeamCursor);
	
	/* Initialize the menus. */

	SetMenuBar(GetNewMBar(mBarID));
	AddResMenu(GetMHandle(appleMID), (ResType) 'DRVR');
	DrawMenuBar();
	
	/* Initialize the drag rectangle. */
	
	DragRect = (***(RgnHandle*)GrayRgn).rgnBBox;
	InsetRect(&DragRect, dragSlop, dragSlop);
	
	/* Get the rectangle list. */
	
	rectHandle = GetResource(rectType, rectID);
	(void) memcpy(RectList, *rectHandle, sizeof(Rect)*numRects);
	ReleaseResource(rectHandle);
	
	/* Check Disinfectant's bundle bit.  If it's not set, set the bundle bit
		and clear the inited bit.  This tells the Finder to try again to read
		the bundle.
		
		This code is present because with version 2.0 some users reported that
		the Disinfectant file appeared on their desktop with no icon after being
		unstuffed.
		
		We only do this if the file has type APPL, to avoid setting the bundle bit
		on Think C project files in the Think C sample program.  (Thanks, Ephraim!) */
	
	GetAppParms(apName, &apRefNum, &apParam);
	if (!GetFInfo(apName, 0, &fndrInfo) && !(fndrInfo.fdFlags & fHasBundle) &&
		fndrInfo.fdType == 'APPL') {
		fndrInfo.fdFlags |= fHasBundle;
		fndrInfo.fdFlags &= ~fInited;
		SetFInfo(apName, 0, &fndrInfo);
	};
	
	/* Initialize the preferences. */
	
	misc_ReadPref();
	
	/* Reopen windows, in back-to-front order. */
	
	for (i = Prefs.numOpenWind-1; i >= 0; i--) {
		switch (Prefs.openWind[i]) {
			case mainWind:
				main_Open();
				break;
			case helpWind:
				help_Open(0);
				break;
			case prefWind:
				pref_Open();
				break;
			case abouWind:
				abou_Open();
				break;
		};
		while (EventAvail(updateMask | activMask, &event)) prog_Event();
	};
	
	/* Initialize the cursor. */

	InitCursor();
	
	/* Set initialization complete. */
	
	Initialized = true;
}
