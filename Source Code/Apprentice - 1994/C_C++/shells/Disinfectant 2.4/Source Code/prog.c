/*______________________________________________________________________

	Sample - Disinfectant Sample Code.
	
	Version 2.4.

	John Norstad
	Academic Computing and Network Services
	Northwestern University
	2129 Sheridan Road
	Evanston, IL 60208
 
	Bitnet: jln@nuacc
	Internet: jln@acns.nwu.edu
	AppleLink: a0173
	CompuServe: 76666,573
	
	Copyright � 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/


/*______________________________________________________________________
	
	prog.c - Sample Main Module.

	This is the main module for Sample.  It contains the basic
	main event loop processing.
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "rez.h"
#include "glob.h"
#include "wstm.h"
#include "abou.h"
#include "pref.h"
#include "help.h"
#include "main.h"
#include "misc.h"
#include "scan.h"
#include "init.h"
#include "prog.h"

#pragma segment prog

extern void _DataInit();

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static WindowPtr			Front;				/* ptr to front window */
static WindowObject		*FrontObj;			/* ptr to front window object */
static WindKind			FrontKind;			/* front window kind */

/*_____________________________________________________________________

	Command - Process a Command.
	
	Entry:		mResult = 16/menu number, 16/item number.
_____________________________________________________________________*/


static void Command (long mResult)

{
	short 				theMenu;				/* menu number */
	short					theItem;				/* item number */
	Str255				daName;				/* desk accessory name */
	GrafPtr 				savePort;			/* saved grafport */

	/* Extact the item and menu numbers. */

	theItem = mResult & 0xffff;
	theMenu = (mResult >> 16) & 0xffff;
	
	/* Check for and process help mode. */
	
	if (HelpMode && (theMenu != appleMID || theItem <= helpCommand)) {
		if (theMenu) 
			help_Open(tagCmdBase + tagCmdMult*(theMenu-appleMID) + theItem);
		HiliteMenu(0);
		return;
	};

	switch (theMenu) {
	
		/* Process the command. */
	
		case appleMID:
		
			if (theItem == aboutCommand) {
				abou_Open();
			} else if (theItem == helpCommand) {
				help_Open(0);
			} else {
				GetItem(GetMHandle(appleMID), theItem, daName);
				GetPort(&savePort);
				(void) OpenDeskAcc(daName);
				SetPort(savePort);
			};
			break;

		case fileMID:
		
			switch (theItem) {
				case closeCommand:
					if (FrontKind == daWind) {
						CloseDeskAcc(((WindowPeek)Front)->windowKind);
					} else {
						if (FrontObj->close) (*FrontObj->close)();
					};
					break;
				case saveAsCommand:
					if (FrontObj->save) (*FrontObj->save)();
					break;
				case pageSetupCommand:
					if (FrontObj->pageSetup) {
						PrOpen();
						misc_PrintError((*FrontObj->pageSetup)());
						PrClose();
					};
					break;
				case printCommand:
					if (FrontObj->print) {
						PrOpen();
						misc_PrintError((*FrontObj->print)(false));
						PrClose();
					};
					break;
				case printOneCommand:
					if (FrontObj->print) {
						PrOpen();
						misc_PrintError((*FrontObj->print)(true));
						PrClose();
					};
					break;
				case prefsCommand:
					pref_Open();
					break;
				case quitCommand:
					Done = true;
					if (Scanning) {
						Scanning = FloppyWait = false;
						Canceled = true;
						InitCursor();
					};
					break;
				default:
					break;
			}
			break;
			
		case editMID:
			if (!SystemEdit(theItem-1) && FrontObj->edit) 
				(*FrontObj->edit)(theItem-1);
			break;
			
		case scanMID:
		
			main_DoScan(theMenu, theItem);
			break;

		default:
			break;

	}; /* switch */
	
	/* Turn off the menu hiliting and return. */

	HiliteMenu(0);
}
	
/*______________________________________________________________________

	Adjust - Adjust Menus.
_____________________________________________________________________*/


static void Adjust (void)

{
	static WindKind		oldFrontKind = mainWind;	
															/* previous front window type */
	static Boolean			oldScanning = false;	/* previous scanning context */
	static Boolean			oldHelpMode = false;	/* previous help mode */
	static Boolean			enableFlags[numMenus] = {true, true, true, true};
															/* menu enabled flags */
															
	MenuHandle				fileM;					/* handle to file menu */
	MenuHandle				editM;					/* handle to edit menu */
	MenuHandle				scanM;					/* handle to scan menu */
	char						*pMenuList;				/* pointer into menu list */
	char						*pMenuListEnd;			/* pointer to end of menu list */
	Boolean					reDrawMenuBar;			/* true if menu bar must be redrawn */
	short						menuIndex;				/* index into menu list */
	MenuHandle				hMenu;					/* handle to menu */
	Boolean					enabled;					/* true if menu enabled */
	
	fileM = GetMHandle(fileMID);
	editM = GetMHandle(editMID);
	scanM = GetMHandle(scanMID);
	
	FrontKind = misc_GetWindKind(FrontWindow());
	
	if (HelpMode) {
		if (oldHelpMode) return;
		EnableItem(fileM, closeCommand);
		EnableItem(fileM, saveAsCommand);
		EnableItem(fileM, pageSetupCommand);
		EnableItem(fileM, printCommand);
		EnableItem(fileM, printOneCommand);
		EnableItem(editM, undoCommand);
		EnableItem(editM, cutCommand);
		EnableItem(editM, copyCommand);
		EnableItem(editM, pasteCommand);
		EnableItem(editM, clearCommand);
		EnableItem(editM, 0);
		EnableItem(scanM, 0);
	} else {
		if (oldFrontKind == FrontKind  && oldScanning == Scanning &&
			oldHelpMode == HelpMode) return;
		if (FrontKind == daWind) {
			EnableItem(fileM, closeCommand);
			DisableItem(fileM, saveAsCommand);
			DisableItem(fileM, pageSetupCommand);
			DisableItem(fileM, printCommand);
			DisableItem(fileM, printOneCommand);
			if (Scanning) {
				DisableItem(scanM, 0);
			} else {
				EnableItem(scanM, 0);
			};
			EnableItem(fileM, 0);
			EnableItem(editM, undoCommand);
			EnableItem(editM, cutCommand);
			EnableItem(editM, copyCommand);
			EnableItem(editM, pasteCommand);
			EnableItem(editM, clearCommand);
			EnableItem(editM, 0);
		} else{
			if (FrontObj->adjust) (*FrontObj->adjust)();
		};
	};
	
	/* Update the menu enabled flags, and redraw the menu bar
		if necessary. */
		
	pMenuList = **(char***)MenuList;
	pMenuListEnd = pMenuList + 6*numMenus;
	pMenuList += 6;
	reDrawMenuBar = false;
	menuIndex = 0;
	while (pMenuList <= pMenuListEnd) {
		hMenu = *(MenuHandle*)(pMenuList);
		enabled = (**hMenu).enableFlags & 1;
		if (enabled != enableFlags[menuIndex]) {
			reDrawMenuBar = true;
			enableFlags[menuIndex] = enabled;
		};
		pMenuList += 6;
		menuIndex++;
	};
	if (reDrawMenuBar) DrawMenuBar();
	
	oldFrontKind = FrontKind;
	oldScanning = Scanning;
	oldHelpMode = HelpMode;
}

/*_____________________________________________________________________

	prog_Event - Fetch and Handle the Next Event.
				
	This routine should be called to do all non-modal event processing.
	It handles suspend, resume, and update events properly.  It also
	keeps the initial integrity checksum going.
_____________________________________________________________________*/


void prog_Event (void)

{
	EventRecord		event;			/* event */
	short				mask;				/* event mask */
	WindowPtr		whichWindow;	/* ptr to window clicked in */
	WindowObject	*whichObj;		/* ptr to object for window clicked in */
	short				partCode;		/* window part code */
	short				key;				/* ascii code of key pressed */
	unsigned long	newSize;			/* new window size */
	long				sleep;			/* sleep time */
	Boolean			eventAvail;		/* WaitNextEvent function result */
	Point				where;			/* location of mouse down */
	GrafPtr			savedPort;		/* saved grafport */
	short				item;				/* dialog item */
	Boolean			passEvent;		/* true if dialog event should be handled
												same as normal event */
	Boolean			keyEvent;		/* true if keyDown or autoKey event */
	Boolean			discardEvent;	/* true if dialog event should be discarded */
	
	/* Save the current grafport. */
	
	GetPort(&savedPort);
	
	/* Get the front window and a pointer to its object. */
	
	Front = FrontWindow();
	FrontObj = (WindowObject*)((WindowPeek)Front)->refCon;

	/* Adjust menus. */

	Adjust();

	/* Take care of any periodic tasks for all open windows. */
	
	whichWindow = FrontWindow();
	while (whichWindow) {
		if (!utl_IsDAWindow(whichWindow)) {
			whichObj = (WindowObject*)((WindowPeek)whichWindow)->refCon;
			if (whichObj->periodic) {
				SetPort(whichWindow);
				(*whichObj->periodic)();
			};
		};
		whichWindow = (WindowPtr)((WindowPeek)whichWindow)->nextWindow;
	};
	SetPort(savedPort);
		
	/* Initialize the event mask and the sleep time. */
	
	mask = everyEvent;
	if ((Scanning && !FloppyWait) || !InForeground) mask ^= diskMask;
	if (!Initialized) mask = updateMask | activMask;
	sleep = (Scanning || InForeground) ? 0 : LongSleep;
		
	/* Get the next event, if any. */
	
	eventAvail = utl_WaitNextEvent(mask, &event, sleep, nil);
	
	/* Process extended keyboard keys. */
	
	if (keyEvent = event.what == keyDown || event.what == autoKey) {
		key = event.message & charCodeMask;
		switch (key) {
			case homeKey:
				key = upArrow;
				event.modifiers |= cmdKey | shiftKey;
				break;
			case endKey:
				key = downArrow;
				event.modifiers |= cmdKey | shiftKey;
				break;
			case helpKey:
				key = '?';
				event.modifiers |= cmdKey;
				break;
			case pageUpKey:
				key = upArrow;
				event.modifiers |= cmdKey;
				break;
			case pageDownKey:
				key = downArrow;
				event.modifiers |= cmdKey;
				break;
		};
		event.message = (event.message & ~charCodeMask) | key;
	};
	
	/* Check for and process dialog event.
	
		The event is passed through to the main body of code following if it
		is a suspend or resume event (osEvt), a disk insertion event (diskEvt),
		a command key event, or a mousedown event in help mode.
		
		Otherwise, all key events are preprocessed, and are discarded if the
		preprocessor decides they are not legal.
		
		Remaining legal events are processed by DialogSelect.
		
		If DialogSelect returns true (an enabled item was involved), then the
		item is postprocessed. */
	
	if (IsDialogEvent(&event)) {
		passEvent = event.what == osEvt ||
			event.what == diskEvt ||
			keyEvent && (event.modifiers & cmdKey) ||
			event.what == mouseDown && HelpMode;
		if (!passEvent) {
			discardEvent = false;
			if (keyEvent && FrontObj->dialogPre)
				discardEvent = !(*FrontObj->dialogPre)(event.message & charCodeMask);
			if (!discardEvent && DialogSelect(&event, &whichWindow, &item)) {
				whichObj = (WindowObject*)((WindowPeek)whichWindow)->refCon;
				if (whichObj->dialogPost) {
					SetPort(whichWindow);
					(*whichObj->dialogPost)(item);
				};
			};
			SetPort(savedPort);
			return;
		};
	};
	
	/* Return if there's no event to handle. */
	
	if (!eventAvail) return;
	
	/* Process the event. */
	
	switch (event.what) {
	
		case mouseDown:
			
			partCode = FindWindow(event.where, &whichWindow);
			if (whichWindow && !utl_IsDAWindow(whichWindow)) 
				whichObj = (WindowObject*)((WindowPeek)whichWindow)->refCon;
		
			switch (partCode) {
				
				case inSysWindow:
				
					SystemClick(&event, whichWindow);
					break;
					
				case inMenuBar:
				
					MenuPick = true;
					Command(MenuSelect(event.where));
					break;
					
				case inGoAway:
				
					if (TrackGoAway(whichWindow, event.where))
						if (whichObj->close) (*whichObj->close)();
					break;
					
				case inDrag:
				
					DragWindow(whichWindow, event.where, &DragRect);
					whichObj->moved = true;
					break;
					
				case inGrow:
				
					SetPort(whichWindow);
					newSize = GrowWindow(whichWindow, event.where, 
						&whichObj->sizeRect);
					if (newSize && whichObj->grow) 
						(*whichObj->grow)(newSize >> 16, newSize & 0xffff);
					whichObj->moved = true;
					break;
					
				case inZoomIn:
				case inZoomOut:
				
					if (TrackBox(whichWindow, event.where, partCode)) {
						SetPort(whichWindow);
						if (partCode == inZoomOut) wstm_ComputeStd(whichWindow);
						EraseRect(&whichWindow->portRect);
						ZoomWindow(whichWindow, partCode, false);
						if (whichObj->zoom) (*whichObj->zoom)();
						whichObj->moved = true;
					};
					break;
					
				case inContent:
				
					if (whichWindow != FrontWindow()) {
						SelectWindow(whichWindow);
					} else {
						SetPort(whichWindow);
						where = event.where;
						GlobalToLocal(&where);
						if (HelpMode) {
							if (whichObj->help) (*whichObj->help)(where);
						} else {
							if (whichObj->click) 
								(*whichObj->click)(where, event.modifiers);
						};
					};
					break;
					
			};
			
			break;
			
		case keyDown:
		case autoKey:
				
			SetPort(Front);
			key = event.message & charCodeMask;
			if ((event.modifiers & cmdKey) && key != upArrow &&
				key != downArrow) {
				if (key == '?' || key == '/') {
					HelpMode = !HelpMode;
					misc_SetCursor();
				} else if (key == '.' && HelpMode) {
					HelpMode = false;
					InitCursor();
				} else if (key == '.' && Scanning) {
					Scanning = FloppyWait = false;
					Canceled = true;
				} else {
					MenuPick = false;
					Command(MenuKey(event.message & charCodeMask));
				};
			} else if (key == escapeKey && (HelpMode || Scanning)) {
				if (HelpMode) {
					HelpMode = false;
					InitCursor();
				} else {
					Scanning = FloppyWait = false;
					Canceled = true;
				};
			} else {
				if (FrontObj->key) (*FrontObj->key)(key, event.modifiers);
			};
			break;
			
		case updateEvt:
		
			whichWindow = (WindowPtr)event.message;
			whichObj = (WindowObject*)((WindowPeek)whichWindow)->refCon;
			SetPort(whichWindow);
			BeginUpdate(whichWindow);
			EraseRect(&whichWindow->portRect);
			if (whichObj->update) (*whichObj->update)();
			EndUpdate(whichWindow);
			break;
			
		case activateEvt:
		
			whichWindow = (WindowPtr)event.message;
			whichObj = (WindowObject*)((WindowPeek)whichWindow)->refCon;
			SetPort(whichWindow);
			if (event.modifiers & activeFlag) {
				if (whichObj->activate) (*whichObj->activate)();
			} else {
				if (whichObj->deactivate) (*whichObj->deactivate)();
			};
			break;
			
		case diskEvt:
		
			main_Disk(event.message);
			break;
			
		case osEvt:	/* Suspend or resume event */
		
			/* Activate or deactivate the frontmost window */
		
			if (((event.message >> 24) & 0xff) != suspendResumeMessage) break;
			InForeground = event.message & 1;
			if (FrontKind != daWind) {
				SetPort(Front);
				if (InForeground) {
					if (FrontObj->activate) (*FrontObj->activate)();
				} else {
					if (FrontObj->deactivate) (*FrontObj->deactivate)();
				};
			};
			
			/* Send suspend or resume message to all open windows. */
			
			whichWindow = FrontWindow();
			while (whichWindow) {
				if (!utl_IsDAWindow(whichWindow)) {
					whichObj = (WindowObject*)((WindowPeek)whichWindow)->refCon;
					if (InForeground) {
						if (whichObj->resume) (*whichObj->resume)();
					} else {
						if (whichObj->suspend) (*whichObj->suspend)();
					};
				};
				whichWindow = (WindowPtr)((WindowPeek)whichWindow)->nextWindow;
			};
			
			/* Adjust the cursor.  If resume event, remove notification. */
			
			if (InForeground) {
				misc_SetCursor();
				if (Notified) {
					Notified = false;
					if (Prefs.notifOption != notifAlert) 
						NMRemove((QElemPtr)&NotifRec);
				};
			} else {
				InitCursor();
			};
			break;
			
	};
	
	/* Restore the saved grafPort. */
	
	SetPort(savedPort);

};
	
/*______________________________________________________________________

	Main - The Main Program.
_____________________________________________________________________*/


void main(void)

{
	/* Initialize. */

	UnloadSeg(_DataInit);
	init_InitMem();
	init_Initialize();
	UnloadSeg(init_Initialize);
	
	/* If scanning station, start scan. */
	
	if (Prefs.scanningStation) scan_DoScan(autoScan, checkOp);

	/*	Main event loop. */
	
	while (!Done) prog_Event();
	
	/*	Terminate. */
	
	misc_WritePref();
}
