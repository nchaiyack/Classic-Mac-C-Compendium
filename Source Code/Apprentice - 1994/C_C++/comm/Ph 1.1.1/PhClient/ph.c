/*_____________________________________________________________________

  	Ph - Mac Ph Client.
	
	John Norstad
	Academic Computing and Network Services
	Northwestern University
	j-norstad@nwu.edu
	
	Copyright � Northwestern University, 1991, 1992.

Version 1.2b1. 11/4/92.

- (Pete Resnick). The site list popup menu sometimes extends beyond the
left edge of query windows. Fixed.
- (Pete Resnick). The Cancel button did not properly cancel in the
change default server window. Fixed.

____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "utl.h"
#include "mtcp.h"
#include "glob.h"
#include "edit.h"
#include "abou.h"
#include "oop.h"
#include "query.h"
#include "help.h"
#include "serv.h"
#include "fsu.h"
#include "site.h"

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static Ptr		ReserveBlock = nil;			/* reserve memory block */
static Ptr		LastGaspBlock = nil;			/* last gasp memory block */

/*______________________________________________________________________

	GrowZone - Grow Zone Procedure.
	
	Entry:	cbNeeded = number of bytes needed.
_____________________________________________________________________*/


static pascal long GrowZone (Size cbNeeded)

{
#pragma unused (cbNeeded)

	if (ReserveBlock) {
		DisposPtr(ReserveBlock);
		ReserveBlock = nil;
		glob_Error(stringsID, memoryLow, nil);
		return 1;
	} else {
		if (LastGaspBlock) {
			DisposPtr(LastGaspBlock);
			LastGaspBlock = nil;
		}
		glob_BringToFront();
		utl_StopAlert(noMemID, nil, 0);
		ExitToShell();
		return 0;
	}
}

/*_____________________________________________________________________

	BuildMenu - Build a Menu from an Item List.
	
	Entry:		h = handle to item list.
					menu = handle to menu.
_____________________________________________________________________*/

static void BuildMenu (Handle h, MenuHandle menu)

{
	short			item;				/* item number */
	char			*p;				/* pointer into item list */
	char			*pEnd;			/* pointer to end of item list */
	
	HLock(h);
	p = *h;
	pEnd = p + GetHandleSize(h);
	item = CountMItems(menu);
	while (p < pEnd) {
		if (*p) {
			item++;
			AppendMenu(menu, "\p ");
			SetItem(menu, item, p);
		}
		p += *p+1;
	}
	HUnlock(h);
	CalcMenuSize(menu);
}

/*_____________________________________________________________________

	ReadPrefs - Read Prefs File.
_____________________________________________________________________*/

#pragma segment init

static void ReadPrefs (void)

{
	FSSpec				fSpec;			/* prefs file spec */
	short					refNum;			/* resource file refnum */
	Boolean				error;			/* true if error encountered */
	OSErr					rCode;			/* error code */
	Handle				h;					/* handle to resource */
	short					size;				/* size of resource */
	WindState			*pState;			/* pointer to window state */
	short					i;					/* loop index */
	Str255				newDefaultServer;	/* new default server */
	Boolean				isFolderAlias;	/* true if folder alias */
	Boolean				isAlias;			/* true if alias */
	
	/* Build the prefs file spec. */
	
	GetIndString(fSpec.name, stringsID, prefsFileName);
	error = fsu_FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		&fSpec.vRefNum, &fSpec.parID);
		
	/* Resolve aliases. */
	
	if (utl_GestaltFlag(gestaltAliasMgrAttr, gestaltAliasMgrPresent)) {
		rCode = ResolveAliasFile(&fSpec, true, &isFolderAlias, &isAlias);
		error = rCode || isFolderAlias;
	}
	
	/* Open the prefs resource file with read-only permission. */
	
	refNum = -1;
	if (!error) {
		error = fsu_FSpOpenResFile(&fSpec, fsRdPerm, &refNum);
	};
	
	/* Read the WPOS resource. */
	
	if (!error) {
		h = GetResource('WPOS', 128);
		error = !h || GetHandleSize(h) != (6 + 3*numPosSave)*sizeof(WindState);
	}
	if (!error) {
		pState = (WindState*)*h;
		TranState = *pState++;
		NewState = *pState++;
		OpenState = *pState++;
		PswdState = *pState++;
		LoginState = *pState++;
		SiteState = *pState++;
		memcpy(QueryStates, pState, numPosSave*sizeof(WindState));
		pState += numPosSave;
		memcpy(EditStates, pState, numPosSave*sizeof(WindState));
		pState += numPosSave;
		memcpy(HelpStates, pState, numPosSave*sizeof(WindState));
	}
	
	/* Read the MISC resource. */
	
	if (!error) {
		h = GetResource('MISC', 128);
		error = !h || GetHandleSize(h) != 11;
	}
	if (!error) {
		NumSites = *(short*)(*h);
		LastHelpUpdate = *(unsigned long*)(*h+2);
		LastSiteUpdate = *(unsigned long*)(*h+6);
		HelpNative = *(Boolean*)(*h+10);
	}
	
	/* Read the DSRV resurce. */
	
	if (!error) {
		h = GetResource('DSRV', 128);
		error = !h;
	}
	if (!error) {
		utl_CopyPString(DefaultServer, *h);
	}
	
	/* Read the SITE resource and build the site popup menu. */
	
	if (!error) {
		h = GetResource('SITE', 128);
		error = !h;
	}
	if (!error) {
		size = GetHandleSize(h);
		if (size) {
			SiteMenu = NewMenu(popupID, "\p");
			BuildMenu(h, SiteMenu);
		} else {
			SiteMenu = nil;
		}
	}
	
	/* Read the SERV and DOMA resources. */
	
	if (SiteMenu) {
		if (!error) {
			Servers = GetResource('SERV', 128);
			error = !Servers;
		}
		if (!error) DetachResource(Servers);
		if (!error) {
			Domains = GetResource('DOMA', 128);
			error = !Domains;
		}
		if (!error) DetachResource(Domains);
	}
	
	/* Read the HSRV resource. */
	
	if (!error) {
		h = GetResource('HSRV', 128);
		error = !h;
	}
	if (!error) {
		utl_CopyPString(HelpServer, *h);
	}
	
	/* Read the HELP resource and build the HELP menu. */
	
	if (!error) {
		h = GetResource('HELP', 128);
		error = !h;
	}
	if (!error) {
		if (GetHandleSize(h)) {
			DetachResource(h);
			HelpTopics = h;
		} else {
			HelpTopics = nil;
		}
		help_BuildHelpMenu();
	}
	
	/* If any error was encountered, initialize everything for the
		first time using defaults. */
		
	if (error) {
		TranState.moved = false;
		LoginState.moved = false;
		NewState.moved = false;
		OpenState.moved = false;
		PswdState.moved = false;
		SiteState.moved = false;
		for (i = 1; i <= numPosSave; i++)
			QueryStates[i].moved = EditStates[i].moved = HelpStates[i].moved = false;
		SiteMenu = nil;
		h = GetResource('STR ', defServerID);
		utl_CopyPString(DefaultServer, *h);
		serv_GetSiteList(DefaultServer);
		utl_CopyPString(newDefaultServer, DefaultServer);
		while (true) {
			if (site_DoDialog(newDefaultServer, siteLabelMsg1, siteLabelMsg2)) 
				ExitToShell();
			if (!EqualString(newDefaultServer, DefaultServer, true, true)) {
				if (rCode = serv_GetSiteList(newDefaultServer)) {
					glob_ErrorCode(rCode);
					continue;
				}
				utl_CopyPString(DefaultServer, newDefaultServer);
			}
			break;
		}
		if (rCode = serv_GetHelpList(DefaultServer)) {
			glob_ErrorCode(rCode);
			ExitToShell();
		}
	}
	
	/* Close the resource file. */
	
	if (refNum != -1) CloseResFile(refNum);
}

#pragma segment Main

/*_____________________________________________________________________

	BuildMenuItemList - Build List of Menu Items.
	
	Entry:	menu = handle to menu, or nil if none.
				first = index in menu of first item in list.
	
	Exit:		function result = handle to list of menu items.
_____________________________________________________________________*/

static Handle BuildMenuItemList (MenuHandle menu, short first)

{
	Handle			h;				/* handle to list */
	short				size;			/* size of list */
	short				numItems;	/* number of items */
	short				i;				/* loop index */
	Str255			str;			/* menu item */
	
	h = NewHandle(0);
	if (!menu) return h;
	size = 0;
	numItems = CountMItems(menu);
	for (i = first; i <= numItems; i++) {
		GetItem(menu, i, str);
		if (str[1] == '-') break;
		SetHandleSize(h, size + *str + 1);
		utl_CopyPString(*h + size, str);
		size += *str+1;
	}
	return h;
}

/*_____________________________________________________________________

	WritePrefs - Write Prefs File.
_____________________________________________________________________*/

static void WritePrefs (void)

{
	FSSpec				fSpec;			/* prefs file spec */
	short					refNum;			/* resource file refnum */
	OSErr					rCode;			/* error code */
	Boolean				error;			/* true if error encountered */
	Handle				h;					/* handle to resource */
	WindState			*pState;			/* pointer to window state */
	short					numTypes;		/* number of resource types */
	ResType				theType;			/* resource type */
	short					numRez;			/* number of resources of type theType */
	Boolean				isFolderAlias;	/* true if folder alias */
	Boolean				isAlias;			/* true if alias */
	
	/* Build the prefs file spec. */
	
	GetIndString(fSpec.name, stringsID, prefsFileName);
	error = fsu_FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
		&fSpec.vRefNum, &fSpec.parID);
		
	/* Resolve aliases. */
	
	if (utl_GestaltFlag(gestaltAliasMgrAttr, gestaltAliasMgrPresent)) {
		rCode = ResolveAliasFile(&fSpec, true, &isFolderAlias, &isAlias);
		error = !rCode && isFolderAlias;
	}
	
	/* Open the prefs resource file with read/write permission.
		If it doesn't exist, create it. */
		
	refNum = -1;
	if (!error) {
		rCode = fsu_FSpOpenResFile(&fSpec, fsRdWrPerm, &refNum);
		if (rCode == fnfErr) {
			rCode = fsu_FSpCreateResFile(&fSpec, 'PHED', 'PHEP');
			if (!rCode) rCode = fsu_FSpOpenResFile(&fSpec, fsRdWrPerm, &refNum);
		};
		error = rCode;
	};
	
	if (!error) {
	
		/* Remove any existing resources. */
		
		numTypes = Count1Types();
		while (numTypes--) {
			Get1IndType(&theType, 1);
			numRez = Count1Resources(theType);
			while (numRez--) RmveResource(Get1IndResource(theType, 1));
		}
		
		/* Write the WPOS resource. */
		
		h = NewHandle((6 + 3*numPosSave)*sizeof(WindState));
		pState = (WindState*)*h;
		*pState++ = TranState;
		*pState++ = NewState;
		*pState++ = OpenState;
		*pState++ = PswdState;
		*pState++ = LoginState;
		*pState++ = SiteState;
		memcpy(pState, QueryStates, numPosSave*sizeof(WindState));
		pState += numPosSave;
		memcpy(pState, EditStates, numPosSave*sizeof(WindState));
		pState += numPosSave;
		memcpy(pState, HelpStates, numPosSave*sizeof(WindState));
		AddResource(h, 'WPOS', 128, "\p");
		
		/* Write the MISC resource. */
		
		h = NewHandle(11);
		*(short*)(*h) = NumSites;
		*(unsigned long*)(*h+2) = LastHelpUpdate;
		*(unsigned long*)(*h+6) = LastSiteUpdate;
		*(Boolean*)(*h+10) = HelpNative;
		AddResource(h, 'MISC', 128, "\p");
		
		/* Write the DSRV resource. */
		
		h = NewHandle(*DefaultServer+1);
		utl_CopyPString(*h, DefaultServer);
		AddResource(h, 'DSRV', 128, "\p");
		
		/* Write the HELP resource. */
		
		h = HelpTopics;
		if (!h) h = NewHandle(0);
		AddResource(h, 'HELP', 128, "\p");
		
		/* Write the HSRV resource. */
		
		h = NewHandle(*HelpServer+1);
		utl_CopyPString(*h, HelpServer);
		AddResource(h, 'HSRV', 128, "\p");
		
		/* Write the SITE resource. */
		
		h = BuildMenuItemList(SiteMenu, 1);
		AddResource(h, 'SITE', 128, "\p");
		
		/* Write the SERV and DOMA resources. */
		
		if (SiteMenu) {
			AddResource(Servers, 'SERV', 128, "\p");
			AddResource(Domains, 'DOMA', 128, "\p");
		}
	}
		
	/* Close the resource file. */
	
	if (refNum != -1) CloseResFile(refNum);
}

/*_____________________________________________________________________

	Terminate - Terminate program.
	
	Entry:	interactionPermitted = true if interaction permitted.
_____________________________________________________________________*/

static void Terminate (Boolean interactionPermitted) 

{
	if (edit_Terminate(interactionPermitted)) return;
	help_Terminate();
	query_Terminate();
	WritePrefs();
	mtcp_Terminate();
	Done = true;
}

/*_____________________________________________________________________

	DoCommand - Process a Menu Command.
	
	Entry:	w = nil.
				theMenu = menu index.
				theItem = item index.
				
	Exit:		function result = true if command handled.
_____________________________________________________________________*/

#pragma segment command

Boolean DoCommand (WindowPtr w, short theMenu, short theItem)

{
#pragma unused (w)

	WindowPtr			v;						/* traverses window list */
	OSErr					rCode;				/* error code */
	Str255				newDefaultServer;	/* new default server */

	switch (theMenu) {
		case fileID:
			switch (theItem) {
				case updateCmd:
					if (rCode = serv_GetSiteList(DefaultServer)) glob_ErrorCode(rCode);
					return true;
				case changeDefCmd:
					HiliteMenu(0);
					utl_CopyPString(newDefaultServer, DefaultServer);
					while (true) {
						if (site_DoDialog(newDefaultServer, 
							siteLabelMsg1, siteLabelMsg2)) return true;
						if (!EqualString(newDefaultServer, DefaultServer, true, true)) {
							if (rCode = serv_GetSiteList(newDefaultServer)) {
								glob_ErrorCode(rCode);
								continue;
							}
							utl_CopyPString(DefaultServer, newDefaultServer);
							query_NewDefaultSite();
							if (rCode = serv_GetHelpList(DefaultServer)) 
								glob_ErrorCode(rCode);
						}
						return true;
					}
				case quitCmd:
					Terminate(true);
					return true;
			}
			return false;
		case windowsID:
			if (theItem == 1) return true;
			v = FrontWindow();
			while (v && --theItem) v = (WindowPtr)((WindowPeek)v)->nextWindow;
			if (v) SelectWindow(v);
			return true;
	}
	return false;
}

#pragma segment Main

/*_____________________________________________________________________

	IgnoreEvent - Ignore an AppleEvent.
	
	Entry:	event = pointer to event.
				reply = pointer to reply event.
				refCon = AppleEvent refCon.
	
	Exit:		function result = noErr.
	
	This is the handler for the Open Application, Open Documents,
	and Print Documents events.
_____________________________________________________________________*/

pascal OSErr IgnoreEvent (AppleEvent *event, AppleEvent *reply, long refCon)

{
#pragma unused (event, reply, refCon)

	return noErr;
}

/*_____________________________________________________________________

	QuitEvent - Quit Event Handler.
	
	Entry:	event = pointer to event.
				reply = pointer to reply event.
				refCon = AppleEvent refCon.
	
	Exit:		function result = result code.
				Done = true.
_____________________________________________________________________*/

pascal OSErr QuitEvent (AppleEvent *event, AppleEvent *reply, long refCon)

{
#pragma unused (reply, refCon)

	OSErr			rCode;			/* result code */
		
	if (rCode = utl_CheckAERequiredParams(event)) return rCode;
	Terminate(utl_InteractionPermitted());
	return noErr;
}

/*_____________________________________________________________________

	Initialize - Initialize the Program.
_____________________________________________________________________*/

#pragma segment init

static void Initialize (void)

{
	OSErr				rCode;			/* result code */
	short				i;					/* loop index */
	WindowPtr		w;					/* pointer to dummy window */
	FontInfo			fInfo;			/* font info */
	Handle			h;					/* handle to resource */
	long				port;				/* server port number */
	long				SysVersion;		/* system version */
	MenuHandle		helpMenu7;		/* handle to System 7 help menu */
	short				numItems;		/* number of items in System 6 help menu */
	Str255			itemString;		/* help menu command */
	
	/* Establish a grow zone procedure to catch out of memory conditions. */ 
	
	CouldAlert(noMemID);
	SetGrowZone(GrowZone);
	ReserveBlock = NewPtr(140*1024);
	LastGaspBlock = NewPtr(2*1024);
	
	/* Initialize the menus. */

	SetMenuBar(GetNewMBar(mBarID));
	AddResMenu(GetMHandle(appleID), (ResType) 'DRVR');
	AppleMenu = GetMHandle(appleID);
	FileMenu = GetMHandle(fileID);
	EditMenu = GetMHandle(editID);
	ProxyMenu = GetMHandle(proxyID);
	WindowsMenu = GetMHandle(windowsID);
	HelpMenu = GetMHandle(helpID);
	HaveBalloons = utl_GestaltFlag(gestaltHelpMgrAttr, gestaltHelpMgrPresent) && 
		!HMGetHelpMenuHandle(&helpMenu7);	
	if (HaveBalloons) {
		numItems = CountMItems(HelpMenu);
		for (i = 1; i <= numItems; i++) {
			GetItem(HelpMenu, i, itemString);
			AppendMenu(helpMenu7, itemString);
		}
		DeleteMenu(helpID);
		NumMenus = numMenus6-1;
		HelpMenu = helpMenu7;
	} else {
		HelpMenuLength = CountMItems(HelpMenu);
		NumMenus = numMenus6;
	}
	HelpMenuLength = CountMItems(HelpMenu);
	DrawMenuBar();
	
	/* Check for System 6.0.5 or later. */
	
	if (Gestalt(gestaltSystemVersion, &SysVersion) || SysVersion < 0x0605) {
		glob_Error(stringsID, msgNeed604, nil);
		ExitToShell();
	}
		
	/* Initialize the cursors. */

	IBeamHandle = GetCursor(iBeamCursor);
	WatchHandle = GetCursor(watchCursor);
	SetCursor(*WatchHandle);
	for (i = 0; i < numBB; i++) BBArray[i] = GetCursor(beachCursorID + i);
	
	/* Initialize the regions. */
	
	BigClipRgn = NewRgn();
	SetRectRgn(BigClipRgn, 0x8000, 0x8000, 0x7FFF, 0x7FFF);
		
	/* Initialize the font. */
	
	w = GetNewWindow(editWindID, nil, (WindowPtr)-1);
	SetPort(w);
	GetFNum(font, &FontNum);
	if (!FontNum) {
		glob_Error (stringsID, msgNeedMonaco, nil);
		ExitToShell();
	}
	TextFont(FontNum);
	TextSize(fontSize);
	GetFontInfo(&fInfo);
	Ascent = fInfo.ascent;
	Descent = fInfo.descent;
	Leading = fInfo.leading;
	LineHeight = Ascent + Descent + Leading;
	if (LineHeight & 1) LineHeight++;
	DisposeWindow(w);
	
	/* Initialize Apple events. */
	
	if (utl_GestaltFlag(gestaltAppleEventsAttr, gestaltAppleEventsPresent)) {
		AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
			(ProcPtr)IgnoreEvent, 0, false);
		AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
			(ProcPtr)IgnoreEvent, 0, false);
		AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
			(ProcPtr)IgnoreEvent, 0, false);
		AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
			(ProcPtr)QuitEvent, 0, false);
	}
	
	/* Get the server port number */
	
	h = GetResource('STR ', portID);
	HLock(h);
	StringToNum(*h, &port);
	PhPort = port;
	ReleaseResource(h);
	
	/* Initialize MacTCP */
	
	rCode = mtcp_Initialize();
	if (rCode) {
		if (rCode == fnfErr) rCode = badUnitErr;
		glob_ErrorCode(rCode);
		ExitToShell();
	}
	
	/* Read the Prefs file. */
	
	ReadPrefs();
	
	/* Add DoCommand to the list of command handlers. */
	
	oop_RegisterCommandHandler(DoCommand);
	
	/* Initialize the other modules. */
	
	edit_Init();
	query_Init();
	help_Init();
	abou_Init();
	
	InitCursor();
}

#pragma segment Main

/*_____________________________________________________________________

	AdjustWindowsMenu - Adjust Windows Menu
_____________________________________________________________________*/

static void AdjustWindowsMenu (void)

{
	short					nItems;		/* number of items in menu */
	WindowPtr			v;				/* traverses window list */
	short					item;			/* menu item number */
	Str255				ttl;			/* window title */
	Str255				cmd;			/* menu command */

	nItems = CountMItems(WindowsMenu);
	v = FrontWindow();
	item = 0;
	while (v) {
		GetWTitle(v, ttl);
		if (*ttl) {
			item++;
			if (item > nItems) break;
			GetItem(WindowsMenu, item, cmd);
			if (!EqualString(ttl, cmd, true, true)) break;
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
	if (!v && item == nItems) {
		if (nItems) {
			EnableItem(WindowsMenu, 0);
		} else {
			DisableItem(WindowsMenu, 0);
		}
		return;
	}
	for (item = nItems; item > 0; item--) DelMenuItem(WindowsMenu, item);
	v = FrontWindow();
	item = 0;
	while (v) {
		GetWTitle(v, ttl);
		if (*ttl) {
			item++;
			AppendMenu(WindowsMenu, "\p ");
			SetItem(WindowsMenu, item, ttl);
		}
		v = (WindowPtr)((WindowPeek)v)->nextWindow;
	}
	CalcMenuSize(WindowsMenu);
	if (item) {
		CheckItem(WindowsMenu, 1, true);
		EnableItem(WindowsMenu, 0);
	} else {
		DisableItem(WindowsMenu, 0);
	}
}

/*_____________________________________________________________________

	AdjustMenus - Adjust Menus.
_____________________________________________________________________*/

static void AdjustMenus (WindowPtr top)

{
	static Boolean			mBarState[numMenus6] = {true, true, false,
																false, true, true};
															/* menu enabled flags */
	WindKind					kind;						/* top window kind */
	EditLoginStatus		editLoginStatus;		/* edit login status */
	short						i;							/* loop index */
	short						numItems;				/* number of items in help menu */
	short						editFieldNum;			/* edit field number */
		
	/* Hack to fix problem with old versions of Moire. */
	
	if (***(short***)MenuList < 6*NumMenus) return;
	
	/* Get top window and top window kind. */
	
	kind = oop_GetWindKind(top);
	
	/* Adjust menu commands. */
	
	if (kind >= firstModal) {
		DisableItem(AppleMenu, 0);
		DisableItem(FileMenu, newQueryCmd);
		DisableItem(FileMenu, newPhCmd);
		DisableItem(FileMenu, openPhCmd);
		DisableItem(FileMenu, closeCmd);
		DisableItem(FileMenu, savePhCmd);
		DisableItem(FileMenu, loginCmd);
		DisableItem(FileMenu, logoutCmd);
		DisableItem(FileMenu, paswdCmd);
		DisableItem(FileMenu, deletePhCmd);
		DisableItem(FileMenu, updateCmd);
		DisableItem(FileMenu, changeDefCmd);
		DisableItem(ProxyMenu, 0);
		if (HaveBalloons) {
			numItems = CountMItems(HelpMenu);
			DisableItem(HelpMenu, HelpMenuLength-1);
			for (i = HelpMenuLength+1; i <= numItems; i++) 
				DisableItem(HelpMenu, i);
		} else {
			DisableItem(HelpMenu, 0);
		}
		DisableItem(WindowsMenu, 0);
		if (kind == tranModal) {
			DisableItem(EditMenu, 0);
		} else {
			DisableItem(EditMenu, undoCmd);
			editFieldNum = utl_GetDialogEditFieldNum(top);
			if (kind == pswdModal ||
				kind == loginModal && editFieldNum == loginPswdField ||
				kind == newModal && 
					(editFieldNum == newPswdField1 || editFieldNum == newPswdField2)) {
				DisableItem(EditMenu, cutCmd);
				DisableItem(EditMenu, copyCmd);
			} else {
				EnableItem(EditMenu, cutCmd);
				EnableItem(EditMenu, copyCmd);
			}
			EnableItem(EditMenu, pasteCmd);
			EnableItem(EditMenu, clearCmd);
			EnableItem(EditMenu, selectAllCmd);
			EnableItem(EditMenu, 0);
		}
	} else {
		AdjustWindowsMenu();
		EnableItem(AppleMenu, 0);
		if (HaveBalloons) {
			numItems = CountMItems(HelpMenu);
			EnableItem(HelpMenu, HelpMenuLength-1);
			for (i = HelpMenuLength+1; i <= numItems; i++) 
				EnableItem(HelpMenu, i);
		} else {
			EnableItem(HelpMenu, 0);
		}
		EnableItem(FileMenu, newQueryCmd);
		EnableItem(FileMenu, loginCmd);
		EnableItem(FileMenu, updateCmd);
		EnableItem(FileMenu, changeDefCmd);
		if (CountMItems(ProxyMenu)) {
			EnableItem(ProxyMenu, 0);
		} else {
			DisableItem(ProxyMenu, 0);
		}
		editLoginStatus = edit_GetStatus(nil, nil);
		if (editLoginStatus == editHero) {
			EnableItem(FileMenu, newPhCmd);
		} else {
			DisableItem(FileMenu, newPhCmd);
		}
		if (editLoginStatus == editHero && kind == editWind) {
			EnableItem(FileMenu, deletePhCmd);
		} else {
			DisableItem(FileMenu, deletePhCmd);
		}
		if (editLoginStatus == editNotLoggedIn) {
			DisableItem(FileMenu, openPhCmd);
			DisableItem(FileMenu, logoutCmd);
		} else {
			EnableItem(FileMenu, openPhCmd);
			EnableItem(FileMenu, logoutCmd);
		}
		if (top) {
			EnableItem(FileMenu, closeCmd);
			EnableItem(EditMenu, copyCmd);
			EnableItem(EditMenu, 0);
		} else {
			DisableItem(FileMenu, closeCmd);
			DisableItem(EditMenu, 0);
		}
		if (kind == editWind && edit_IsDirty(top)) {
			EnableItem(FileMenu, savePhCmd);
		} else {
			DisableItem(FileMenu, savePhCmd);
		}
		if (kind == editWind) {
			EnableItem(FileMenu, paswdCmd);
		} else {
			DisableItem(FileMenu, paswdCmd);
		}
		if (kind == daWind) {
			EnableItem(EditMenu, undoCmd);
		} else {
			DisableItem(EditMenu, undoCmd);
		}
		if (kind == daWind || kind == editWind ||
			(kind == queryWind && query_CanEditField(top))) {
			EnableItem(EditMenu, cutCmd);
			EnableItem(EditMenu, pasteCmd);
			EnableItem(EditMenu, clearCmd);
		} else {
			DisableItem(EditMenu, cutCmd);
			DisableItem(EditMenu, pasteCmd);
			DisableItem(EditMenu, clearCmd);
		}
		if (top && kind != daWind) {
			EnableItem(EditMenu, selectAllCmd);
		} else {
			DisableItem(EditMenu, selectAllCmd);
		}
	}
	
	/* Update the menu enabled flags, and redraw the menu bar
		if necessary. */
	
	utl_RedrawMenuBar(mBarState, NumMenus);
}

/*_____________________________________________________________________

	Main Program.
_____________________________________________________________________*/

void main(void)

{
	/* Initialize. */

	oop_Init(appleID, aboutCmd, fileID, closeCmd, editID, selectAllCmd, 
		AdjustMenus, nil, 20, 20000);
	Initialize();
	UnloadSeg(Initialize);
	
	/* Handle events until done. */
	
	while (!Done) oop_DoEvent(nil, everyEvent, 
		oop_InForeground() ? shortSleep : longSleep, nil);
}
