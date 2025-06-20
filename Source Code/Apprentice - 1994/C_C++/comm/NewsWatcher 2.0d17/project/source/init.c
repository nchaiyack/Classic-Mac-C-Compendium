/*----------------------------------------------------------------------------

	init.c

	This module handles initialization of the program.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <GestaltEqu.h>
#include <Traps.h>
#include <stdio.h>
#include <string.h>

#include "glob.h"
#include "init.h"
#include "menus.h"
#include "util.h"
#include "dlgutil.h"
#include "aevt.h"
#include "prefs.h"
#include "nntp.h"
#include "full.h"
#include "newsrc.h"
#include "killfile.h"
#include "log.h"



/*----------------------------------------------------------------------------
	SetUpMenus 
	
	Draws the pull-down menus onto the screen.	 
----------------------------------------------------------------------------*/

static void SetUpMenus (void)
{
	Handle mbar;
	
	mbar = GetNewMBar(kMBarID);
	SetMenuBar(mbar);
	AddResMenu(GetMHandle(kAppleMenu), 'DRVR');
	
	#ifndef FILTERS
	{
		short item;
		MenuHandle theMenu;
		Str255 theCommand;
		
		theMenu = GetMHandle(kSearchMenu);
		for (item = kGroupKillItem; item <= kKillSubjectItem; item++) {
			GetItem(theMenu, item, theCommand);
			BlockMove(theCommand+1, theCommand+2, *theCommand+1);
			theCommand[1] = '�';
			(*theCommand)++;
			SetItem(theMenu, item, theCommand);
		}
	}
	#endif
	
	DrawMenuBar();
}



/*----------------------------------------------------------------------------
	Init 
		
	Initializes the program. 	
----------------------------------------------------------------------------*/

void Init (void)
{
	EventRecord ev;
	CStr255 msg;
	Boolean mightHaveNewGroups = true;
	short sizeofTPrefRec;
	FSSpec **docList = nil;
	short numDocs;
	long systemVersion;
	long quickdrawVersion;
	OSErr err;
	
	extern Boolean TrapAvailable (unsigned long theTrap);	/* defined in dnr.c! */
	
	gLifeBoat = NewHandle(kLifeBoatSize);
	if (MemError() == noErr)
		gSinking = false;
	MoveHHi(gLifeBoat);
	HLock(gLifeBoat);
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
	InitCursorCtl(nil);

	FlushEvents(everyEvent,0);
	EventAvail(everyEvent,&ev);

	if (!TrapAvailable(_Gestalt)) {
		ErrorMessage("NewsWatcher requires System 7.0 or later.");
		ExitToShell();
	}
	err = Gestalt(gestaltSystemVersion, &systemVersion);
	if (err != noErr || systemVersion < 0x0700) {
		ErrorMessage("NewsWatcher requires System 7.0 or later.");
		ExitToShell();
	}
	
	err = Gestalt(gestaltQuickdrawVersion, &quickdrawVersion);
	gHasColorQD = err == noErr && quickdrawVersion >= gestalt8BitQD;

	gDesktopExtent = (**GetGrayRgn()).rgnBBox;
	gDragRect = gDesktopExtent;
	InsetRect(&gDragRect,4,4);
	SetRect(&gWindLimits,kMinWindWidth,kMinWindHeight,6000,6000);
	gIBeamCurs = **(GetCursor(iBeamCursor));
	SpinCursor(0);
	
	sizeofTPrefRec = sizeof(TPrefRec);
	if (sizeofTPrefRec != kTPrefRecSize) {
		sprintf(msg, "sizeof(TPrefRec) is %d, should be %d", sizeofTPrefRec,
			kTPrefRecSize);
		ErrorMessage(msg);
		ExitToShell();
	}

	SetUpMenus();
	
/*ErrorMessage("NewsWatcher 2.0d16 special test version for Stephan Somogyi! Please do not redistribute!");*/

	InitializeAppleEvents();
	GetInitialDocList(&docList, &numDocs);
	LocatePrefsFile(docList, numDocs);
	FSMakeFSSpec(gPrefsFile.vRefNum, gPrefsFile.parID, kFilterName, &gFilterFile);
	gStartupOK = ReadPrefs();
	if (gPrefs.logActionsToFile) OpenLogFile();

	#ifdef FILTERS
		if (gStartupOK) gStartupOK = ReadKillFile();
	#endif

	if (gStartupOK) gStartupOK = StartNNTP();
	
	if (gStartupOK && gNumGroups == 0) {
		gStartupOK = ReadGroupsFromServer();
		mightHaveNewGroups = false;
	}
	
	if (gStartupOK) gStartupOK = CreateFullGroupListWindow();
	
	if (gStartupOK && mightHaveNewGroups && gPrefs.checkForNewGroups)
		CheckForNewGroups();

	if (gStartupOK) {
		if (gPrefs.autoFetchnewsrc) {
			DoGetGroupListFromHost(true);
			gCancel = false;
		}
		OpenDocList(docList, numDocs);
	}
	
	if (gStartupOK) CreateNewGroupListWindow();
	
	if (!gStartupOK) {
		EndNNTP();
		strcpy(msg, "An error occurred during the startup process, ");
		strcat(msg, "or you canceled the startup process. ");
		strcat(msg, "You may adjust NewsWatcher�s preferences, ");
		strcat(msg, "but you will not be able to read or post news.");
		ErrorMessage(msg);
	}
	
	gStartingUp = false;
}
