/*______________________________________________________________________

	main.c - Main Window Manager.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/


#pragma load "precompile"
#include "vol.h"
#include "rep.h"
#include "rpp.h"
#include "scn.h"
#include "utl.h"
#include "rez.h"
#include "glob.h"
#include "wstm.h"
#include "misc.h"
#include "scan.h"
#include "mssg.h"
#include "prog.h"
#include "help.h"
#include "main.h"
#include "unmount.h"
#include "mssg.h"

#pragma segment main

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static WindowPtr			MainWindow;			/* ptr to main window */
static WindowRecord		MainWRecord;		/* main window record */
static WindowObject		MainWindObject;	/* main window object */

static long					DiskMessage;		/* disk insert message from
															event record */
static Str255				WaitMsg;				/* disk insert wait message for
															display in window */
static Rect					WaitMsgRect;		/* disk insert wait message rect */
static Handle				FloppyH;				/* handle to small floppy icon */
static Boolean				FloppyBlack;		/* true if floppy wait, and blinking
															small icon is black */

/*______________________________________________________________________

	main_SetPort - Set the Current GrafPort to the Main Window.
_____________________________________________________________________*/


void main_SetPort (void)

{
	SetPort(MainWindow);
};

/*______________________________________________________________________

	main_DoScan - Do a Scan.
	
	Entry:	theMenu = menu number.
				theItem = item number in menu.
_____________________________________________________________________*/


void main_DoScan (short theMenu, short theItem)

{
	ScanKind				scanKind;			/* kind of scan */
	ScanOp				scanOp;				/* scanning operation */
	
	SetPort(MainWindow);
	switch (theMenu) {
		case scanMID:
			scanOp = checkOp;
			break;
	};
	switch (theItem) {
		case fileCommand:
			scanKind = fileScan;
			break;
		case folderCommand:
			scanKind = foldScan;
			break;
		case floppiesCommand:
			scanKind = autoScan;
			break;
		case allDisksCommand:
			scanKind = allScan;
			break;
		case someDisksCommand:
			scanKind = volSetScan;
			break;
		case sysFileCommand:
			scanKind = sysFileScan;
			break;
		case sysFolderCommand:
			scanKind = sysFoldScan;
			break;
		case desktopCommand:
			scanKind = desktopScan;
			break;
	};
	scan_DoScan(scanKind, scanOp);
}

/*______________________________________________________________________

	main_WaitInsert - Wait for a Disk Insertion Event.
	
	Entry:	firstDisk = true if waiting for first disk.
	
	Exit:		*vRefNum = volume reference number of inserted disk.
				*canceled = true if canceled (Cancel button or cmd/.)
_____________________________________________________________________*/


void main_WaitInsert (Boolean firstDisk, short *vRefNum, 
	Boolean *canceled)

{
	long				lastTick;			/* tick count at last disk icon invert */
	long				nowTick;				/* current tick count */
	Rect				eraseRect;			/* rectangle to erase */													
	
	SetPort(MainWindow);
	FloppyWait = true;
	FloppyBlack = false;
	lastTick = TickCount();
	FloppyH = GetResource('SICN', floppyIconID);
	eraseRect = WaitMsgRect = RectList[volNameRect];
	InsetRect(&eraseRect, -2, -2);
	eraseRect.right += 500;
	EraseRect(&eraseRect);
	WaitMsgRect.right += 500;
	GetIndString(WaitMsg, strListID, firstDisk ? firstDiskStr : nextDiskStr);
	InvalRect(&eraseRect);
	InvalRect(&RectList[volIconRect]);
	while (FloppyWait) {
		if (InForeground) {
			nowTick = TickCount();
			if (nowTick >= lastTick + invertInterval) {
				lastTick = nowTick;
				InvertRect(&RectList[volIconRect]);
				FloppyBlack = !FloppyBlack;
			};
		};
		prog_Event();
		if (!FloppyWait && !Canceled) {
			if (Prefs.scanningStation && ((DiskMessage >> 16) & 0xffff)) {
				Eject(nil, DiskMessage & 0xffff);
				FloppyWait = true;
				firstDisk = false;
				mssg_BadDisk();
			} else {
				FloppyWait = utl_DoDiskInsert(DiskMessage, vRefNum);
			};
		};
	};
	*canceled = Canceled;
	InvalRect(&eraseRect);
	InvalRect(&RectList[volIconRect]);
}

/*______________________________________________________________________

	Update - Process an Update Event.
_____________________________________________________________________*/


static void Update (void)

{
	Handle				iconHandle;		/* handle to icon */
	short					oldFont;			/* saved font number */
	short					oldSize;			/* saved font size */
	short					baseLine;		/* baseline for counter */
	short					right;			/* right coord of counter title */
	Str255				str;				/* multi-purpose string */
	
	SetPort(MainWindow);
	
	/*	Draw the controls. */
	
	DrawControls(MainWindow);
	
	/* Draw the small volume icon and the volume name in the
		system font and size. */
	
	oldFont = qd.thePort->txFont;
	oldSize = qd.thePort->txSize;
	TextFont(systemFont);
	TextSize(0);
	if (FloppyWait) {
		utl_PlotSmallIcon(&RectList[volIconRect], FloppyH);
		if (FloppyBlack) InvertRect(&RectList[volIconRect]);
		TextBox(WaitMsg+1, *WaitMsg, &WaitMsgRect, teJustLeft);
	} else {
		vol_DoUpdate();
	};
	TextFont(oldFont); 
	TextSize(oldSize);
	
	/*	Draw the small folder and file icons. */
	
	iconHandle = GetResource('SICN', folderIconID);
	utl_PlotSmallIcon(&RectList[foldIconRect], iconHandle);
	iconHandle = GetResource('SICN', fileIconID);
	utl_PlotSmallIcon(&RectList[fileIconRect], iconHandle);
	
	/* Draw the counters. */
	
	baseLine = RectList[counterRect].top + 12;
	right = RectList[counterRect].right;
	GetIndString(str, strListID, counter1);
	MoveTo(right - StringWidth(str), baseLine);
	DrawString(str);
	NumToString(NumScanned, str);
	MoveTo(right+5, baseLine);
	DrawString(str);
	baseLine += 12;
	GetIndString(str, strListID, counter2);
	MoveTo(right - StringWidth(str), baseLine);
	DrawString(str);
	NumToString(NumInfected, str);
	MoveTo(right+5, baseLine);
	DrawString(str);
	baseLine += 12;
	GetIndString(str, strListID, counter3);
	MoveTo(right - StringWidth(str), baseLine);
	DrawString(str);
	NumToString(NumErrors, str);
	MoveTo(right+5, baseLine);
	DrawString(str);
		
	/*	Update the folder and file names and the thermometer rectangle. */
	
	scn_Update(&RectList[thermRect]);
	
	/*	Draw the report. */
	
	rep_Update(Report);
	
	/* Draw the grow icon. */
	
	utl_DrawGrowIcon(MainWindow);
}
	
/*______________________________________________________________________

	Activate - Process an Activate Event.
_____________________________________________________________________*/


static void Activate (void)

{
	rep_Activate(Report, true);
	utl_DrawGrowIcon(MainWindow);
	if (!Scanning) vol_Verify();
}
	
/*______________________________________________________________________

	Deactivate - Process a Deactivate Event.
_____________________________________________________________________*/


static void Deactivate (void)

{
	rep_Activate(Report, false);
	utl_DrawGrowIcon(MainWindow);
}

/*______________________________________________________________________

	Click - Process Mouse Down Event.
	
	Entry:		where = mouse down location, local coords.
					modifiers = modifier keys.
_____________________________________________________________________*/


static void Click (Point where, short modifiers)

{
	ControlHandle		whichControl;	/* handle to selected control */
	short					vRefNum;			/* ref num of vol to be ejected */
	Rect					repRect;			/* report rectangle */
	
	/* Check for and process mouse down in report rectangle scroll bar. */
	
	rep_GetRect(Report, &repRect);
	if (PtInRect(where, &repRect)) {
		rep_Scroll(Report, where);
		return;
	};
	
	/* Check for and process mouse down in the volume name rectangle. */
	
	if (!Scanning) {
		(void) vol_DoPopUp(where, mainPopUpID);
		misc_HiliteScan();
	};
	
	/* Check for and process mouse down in a control. */
	
	(void) FindControl(where, MainWindow, &whichControl);
	if (whichControl != nil) {
		if (TrackControl(whichControl, where, nil)) {
			switch ((**whichControl).contrlRfCon) {
				case driveID:
					vol_DoDrive();
					break;
				case ejectID:
					if (!vol_GetSel(&vRefNum)) {
						misc_CheckEject(vRefNum);
						vol_DoEject();
					};
					misc_HiliteScan();
					break;
				case scanID:
					scan_DoButton(checkOp, modifiers);
					break;
				case cancelID:
					Canceled = true;
					Scanning = FloppyWait = false;
					break;
				case quitID:
					Done = true;
					if (Scanning) {
						Scanning = FloppyWait = false;
						Canceled = true;
						InitCursor();
					};
					break;
				case resetID:
					if (NumScanned || NumInfected || NumErrors) {
						mssg_ClearCounters( RectList[counterRect].top,
							RectList[counterRect].right);
					};
					break;
			};
		};
	};
}

/*______________________________________________________________________

	Help - Process Mouse Down Event in Help Mode.
	
	Entry:		where = mouse down location, local coords.
_____________________________________________________________________*/


static void Help (Point where)

{
	short				reportLineNum;	/* line number of selected report line */
	short				tag;				/* tag to jump to in help window */
	short				i;					/* loop index */
	Rect				repRect;			/* report rectangle */
	
	rep_GetRect(Report, &repRect);
	if (PtInRect(where, &repRect)) { 
	
		/* Report rectangle. */
		
		if (where.h >= repRect.right-16) {
			help_Open(tagReport);
		} else {
			reportLineNum = rep_Click(Report, where);
			if (reportLineNum >= 0) {
				tag = mssg_LookupTag(reportLineNum);
				if (tag > 0) {
					help_Open(tag);
				} else {
					help_Open(tagReport);
				};
			} else {
				help_Open(tagReport);
			};
		};
		
	} else if (where.v <= RectList[counterRect].top + 36 &&
		where.h >= RectList[thermRect].left) {
	
		/* Upper right corner of the window. */
		
		help_Open(tagUpperRight);
		
	} else {
	
		/* Check for mouse down in a button. */
	
		for (i = 0; i < numControls; i++) {
			if (PtInRect(where, &(**Controls[i]).contrlRect)) break;
		};
		if (i < numControls) {
			help_Open(tagFirstButton+i);
		} else {
			help_Open(tagMainWind);
		};
	
	};
}

/*______________________________________________________________________

	ChangeSize - Process Window Size Change.
	
	Entry:	height = new window height.
_____________________________________________________________________*/


static void ChangeSize (short height)

{
	rep_Height(Report, height - 6);
	if (Scanning) rep_Jump(Report, rep_GetSize(Report), true);
}

/*______________________________________________________________________

	Grow - Process Window Grow Operation.
	
	Entry:	height = new window height.
				width = new window width.
_____________________________________________________________________*/


static void Grow (short height, short width)

{
	utl_InvalGrow(MainWindow);
	SizeWindow(MainWindow, width, height, true);
	utl_InvalGrow(MainWindow);
	ChangeSize(height);
}
	
/*______________________________________________________________________

	Zoom - Process Window Zoom Operation.
_____________________________________________________________________*/


static void Zoom (void)

{
	ChangeSize(MainWindow->portRect.bottom - MainWindow->portRect.top);
}
	
/*______________________________________________________________________

	Key - Process Key Down Event.
	
	Entry:	key = ascii code of key pressed.
				modifiers = modifier keys.
_____________________________________________________________________*/


static void Key (short key, short modifiers)

{
	if (Scanning) return;
	if (key == upArrow || key == downArrow) {
		rep_Key(Report, key, modifiers);
	} else if (key == tabKey && 
		!(**Controls[driveID-firstControl]).contrlHilite) {
		vol_DoDrive();
		misc_HiliteScan();
	};
}
	
/*______________________________________________________________________

	Close - Close the Window.
_____________________________________________________________________*/


static void Close (void)

{
	Prefs.mainState.moved = MainWindObject.moved;
	wstm_Save(MainWindow, &Prefs.mainState);
	CloseWindow(MainWindow);
	MainWindow = nil;
};
	
/*______________________________________________________________________

	main_Disk - Process Disk Insertion Event.
	
	Entry:	message = message field from event record.
_____________________________________________________________________*/


void main_Disk (long message)

{
	SetPort(MainWindow);
	if (FloppyWait) {
		DiskMessage = message;
		FloppyWait = false;
		return;
	} else if (!Scanning) {
		SetPort(MainWindow);
		(void) vol_DoInsert(message);
		misc_HiliteScan();
	};
}
	
/*______________________________________________________________________

	main_Save - Process Save Command.
	
	Exit:		function result = true if file saved, false if canceled
					or error.
_____________________________________________________________________*/


Boolean main_Save (void)

{
	Str255			prompt;			/* SFPutFile prompt string */
	Boolean			good;				/* true if report saved, false if
												canceled by user */
	OSErr				rCode;			/* result code */
	Str255			rCodeStr;		/* result code as a string */
	long				savedTrapAddr;	/* saved UnmountVol trap address */
	
	/* Get the prompt string. */
	
	GetIndString(prompt, strListID, putPromptStr);
	
	/* If scanning, patch the UnmountVol trap to prevent volume unmounting. */
	
	if (Scanning) {
		
		/* Patch the UnmountVol trap to prevent volumn unmounting. */
		
		savedTrapAddr = NGetTrapAddress(_UnmountVol & 0x3ff, ToolTrap);
		NSetTrapAddress((long)UNMOUNT, _UnmountVol & 0x3ff, ToolTrap);
	};
	
	/* Call rep_Save to save the file. */
	
	rCode = rep_Save(Report, prompt, "", Prefs.repCreator, &good, MenuPick);
	
	/* Handle errors. */
	
	if (rCode) {
		good = false;
		if (rCode == dskFulErr) {
			utl_StopAlert(diskFullID, nil, 0);
		} else if (rCode == fLckdErr) {
			utl_StopAlert(fileLockedID, nil, 0);
		} else {
			NumToString(rCode, rCodeStr);
			ParamText(rCodeStr, nil, nil, nil);
			utl_StopAlert(unexpectedSaveID, nil, 0);
		};
	};
	
	/* If scanning, restore the UnmountVol trap. */
	
	if (Scanning) {
		NSetTrapAddress(savedTrapAddr, _UnmountVol & 0x3ff, ToolTrap);
	} else {
		main_SetPort();
		vol_Verify();
		misc_HiliteScan();
	};

	return good;
}
	
/*______________________________________________________________________

	PageSetup - Process Page Setup Command.
	
	Exit:		function result = error code.
_____________________________________________________________________*/

static OSErr PageSetup (void)

{
	Boolean			canceled;
	
	misc_ValPrint(&Prefs.mainPrint, true);
	Prefs.mainPrint.menuPick = MenuPick;
	return rpp_StlDlog(&Prefs.mainPrint, &canceled);
}
	
/*______________________________________________________________________

	Print - Process Print Command.
	
	Entry:	printOne = true if Print One command.
	
	Exit:		function result = error code.
_____________________________________________________________________*/


static OSErr Print (Boolean printOne)

{
	Str255			titleTmpl;	/* template for date, time, pnum in headers */
	Str255			docName;		/* document name */
	
	GetIndString(titleTmpl, strListID, prRepTmpl);
	GetIndString(docName, strListID, prRepTitle);
	Prefs.mainPrint.title = docName;
	Prefs.mainPrint.titleTmpl = titleTmpl;
	Prefs.mainPrint.docName = docName;
	misc_ValPrint(&Prefs.mainPrint, true);
	Prefs.mainPrint.menuPick = MenuPick;
	return rpp_Print(Report, printOne, &Prefs.mainPrint);
		
}
	
/*______________________________________________________________________

	Edit - Process Edit Command.
	
	Entry:	cmd = which Edit command.
_____________________________________________________________________*/


static void Edit (EditCmd cmd)

{
	if (cmd != clearCmd) return;
	misc_ClearReport();
}	
	
/*______________________________________________________________________

	Adjust - Adjust Menus.
_____________________________________________________________________*/


static void Adjust (void)

{
	MenuHandle				fileM;					/* handle to file menu */
	MenuHandle				editM;					/* handle to edit menu */
	MenuHandle				scanM;					/* handle to scan menu */
	
	fileM = GetMHandle(fileMID);
	editM = GetMHandle(editMID);
	scanM = GetMHandle(scanMID);
	DisableItem(fileM, closeCommand);
	if (Scanning) {
		DisableItem(fileM, saveAsCommand);
		DisableItem(fileM, pageSetupCommand);
		DisableItem(fileM, printCommand);
		DisableItem(fileM, printOneCommand);
		DisableItem(editM, 0);
		DisableItem(scanM, 0);
	} else {
		EnableItem(fileM, saveAsCommand);
		EnableItem(fileM, pageSetupCommand);
		EnableItem(fileM, printCommand);
		EnableItem(fileM, printOneCommand);
		DisableItem(editM, undoCommand);
		DisableItem(editM, cutCommand);
		DisableItem(editM, copyCommand);
		DisableItem(editM, pasteCommand);
		EnableItem(editM, clearCommand);
		EnableItem(editM, 0);
		EnableItem(scanM, 0);
	};
}
	
/*______________________________________________________________________

	main_Open - Open Main Window.
_____________________________________________________________________*/


void main_Open (void)

{
	Handle			floppyH;				/* handle to small floppy icon */
	Handle			hardH;				/* handle to small hard drive icon */
	short				i;						/* loop index */
	short				fNum;					/* font number */
	
	/* Get the main window and restore its state. */
	
	MainWindow = wstm_Restore(false, mainWindID, (Ptr)&MainWRecord,
		&Prefs.mainState);
	SetPort(MainWindow);
	
	/* Initialize the window object. */
	
	((WindowPeek)MainWindow)->refCon = (long)&MainWindObject;
	MainWindObject.windKind = mainWind;
	MainWindObject.moved = Prefs.mainState.moved;
	SetRect(&MainWindObject.sizeRect, MainWindow->portRect.right, 
		minMainSize, MainWindow->portRect.right, 0x7fff);
	MainWindObject.update = Update;
	MainWindObject.activate = Activate;
	MainWindObject.deactivate = Deactivate;
	MainWindObject.resume = nil;
	MainWindObject.suspend = nil;
	MainWindObject.click = Click;
	MainWindObject.help = Help;
	MainWindObject.grow = Grow;
	MainWindObject.zoom = Zoom;
	MainWindObject.key = Key;
	MainWindObject.close = Close;
	MainWindObject.disk = main_Disk;
	MainWindObject.save = main_Save;
	MainWindObject.pageSetup = PageSetup;
	MainWindObject.print = Print;
	MainWindObject.edit = Edit;
	MainWindObject.adjust = Adjust;
	MainWindObject.periodic = nil;
	MainWindObject.dialogPre = nil;
	MainWindObject.dialogPost = nil;
	
	/* Set the window's font to Geneva 9.  If Geneva doesn't exist
		use the application font.  This is the font that will be used in
		the report window. */
	
	if (!utl_GetFontNumber("\pGeneva", &fNum)) fNum = applFont;
	TextFont(fNum);
	TextSize(9);
	
	/*	Get the window controls.
		Inactivate the cancel button. */
	
	for (i=0; i<numControls; i++) {
		Controls[i] = utl_GetNewControl(firstControl+i, MainWindow);
	};
	HiliteControl(Controls[cancelID-firstControl], 255);
	
	/* Initialize the volume selection module. */
	
	floppyH = GetResource('SICN', floppyIconID);
	hardH = GetResource('SICN', hardDriveIconID);
	(void) vol_Init( 
		Controls[driveID-firstControl],
		Controls[ejectID-firstControl],
		&RectList[volNameRect],
		&RectList[volIconRect],
		floppyH,
		hardH,
		true,
		true);
		
	/* Initialize the report. */
	
	rep_Init(&RectList[reportRect], MainWindow, 0, 0, 0, 0, &Report);
	rep_Height(Report, MainWindow->portRect.bottom - 6);
	rep_Fill(Report, repHeadID, false);
	
	/* Show the main window. */
	
	utl_LockControls(MainWindow);
	ShowWindow(MainWindow);
}