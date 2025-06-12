/*
		RgnMaster.c++
		by Hiep Dam, 3G Software.
		March 1994.
		Last Update April 1994
		Contact: America Online: Starlabs
		         Delphi        : StarLabs
		         Internet      : starlabs@aol.com
		                      or starlabs@delphi.com

		Guts, or shell of the application; mostly Macintosh interface stuff.
*/

#include <QDOffscreen.h>
#include "Compat.h"
#include "QDUtils.h"
#include "DialogUtils.h"
#include "Regions.h"
#include "RgnWork.h"
#include "Help Window.h"
#include "About.h"
#include "StringUtils.h"	// Courtesy of Tony Myles.
#include "AppCore.h"
#include "KeyUtils.h"
#include "RgnMaster.h"

// ---------------------------------------------------------------------------

void InitMac();
void SetupDialog();
void LoopIt();
void HandleEvent(EventRecord *theEvent);
void HandleMenu(long menuResult);

// ---------------------------------------------------------------------------

// Globals
short gAppFileRef;
Boolean gDone = false;
DialogPtr gMainDialog;
MenuHandle gFileMenu, gEditMenu, gAppleMenu;
short gCurrentItem = -1;
Boolean gShowHelp = false;
Boolean gPreviewAlways = true;

// ---------------------------------------------------------------------------

void main() {
	InitMac();
	SetCursor(*GetCursor(watchCursor));
	CheckEnviron();
	if (gEnviron.sysVersion < 0x0700) {
		(void)Alert(kWrongSysVersionAlertID, nil);
		ExitToShell();
	}
	SetupDialog();
	SetCursor(&arrow);
	LoopIt();
} // END main

void LoopIt() {
	EventRecord theEvent;
	short itemHit;
	DialogPtr theDialog;
	
	do {
		UpdateHelpStatus();
		if (GetNextEvent(everyEvent, &theEvent)) {
			if (IsDialogEvent(&theEvent) && DialogSelect(&theEvent, &theDialog, &itemHit)) {
				switch(itemHit) {
					case convertAndReplaceItem:
						ConvertAndReplace();
					break;

					case convertOnlyItem:
						ConvertOnly();
					break;

					case viewClipboardItem:
						ShowClipboard();
					break;
					
					case rgnInfoItem:
						ShowRegionInfo();
					break;

					case viewRgnItem:
						ViewRegion();
					break;

					case copyToClipItem:
						CopyRegionToClipboard();
					break;

					case saveToAppItem:
						SaveRegionToApplication();
					break;

					case saveToFileItem:
						SaveRegionToFile();
					break;

					case optionsItem:
						DoOptionsDialog();
					break;

					case helpWindItem:
						ShowHelpWindow();
					break;

					case aboutItem:
						DoRgnMasterAbout();
					break;

					case quitItem:
						gDone = true;
					break;

					case previewCheckbx:
						gPreviewAlways = !GetCtlValue((ControlHandle)GetDItemHdl(gMainDialog, previewCheckbx));
						SetCtlValue((ControlHandle)GetDItemHdl(gMainDialog, previewCheckbx), gPreviewAlways);
					break;
					case helpCheckbx:
						gShowHelp = !GetCtlValue((ControlHandle)GetDItemHdl(gMainDialog, helpCheckbx));
						SetCtlValue((ControlHandle)GetDItemHdl(gMainDialog, helpCheckbx), gShowHelp);
						if (!gShowHelp) {
							Rect helpRect;
							GetDItemRect(gMainDialog, helpItem, &helpRect);
							InsetRect(&helpRect, 1, 1);
							EraseRect(&helpRect);
							gCurrentItem = -1;
						}
					break;
				}
			}
			else
				HandleEvent(&theEvent);
		}
	} while (!gDone);

	SaveOptions();
} // END LoopIt

// ---------------------------------------------------------------------------

void HandleEvent(EventRecord *theEvent) {
	short i;

	switch(theEvent->what) {
		case mouseDown:
			short windowPart;
			WindowPtr theWindow;
			windowPart = FindWindow(theEvent->where, &theWindow);
			switch (windowPart) {
				case inMenuBar:
					HandleMenu(MenuSelect(theEvent->where));
				break;

				case inDrag:
					Rect limitRect = screenBits.bounds;
					InsetRect(&limitRect, 20, 20);
					DragWindow(theWindow, theEvent->where, &limitRect);
				break;

				case inContent:
					if (FrontWindow() != theWindow)
						SelectWindow(theWindow);
					else if (theWindow == gHelpWindow) {
						SetPort(gHelpWindow);
						GlobalToLocal(&theEvent->where);
						ScrollHelpWindow(theEvent->where);
					}
				break;

				case inGoAway:
					if (TrackGoAway(theWindow, theEvent->where) && theWindow == gHelpWindow)
						HideHelpWindow();
				break;
			} // END switch
		break;

		case updateEvt:
			BeginUpdate((WindowPtr)theEvent->message);
			if ((WindowPtr)theEvent->message == gHelpWindow)
				UpdateHelpWindow();
			EndUpdate((WindowPtr)theEvent->message);
		break;

		case activateEvt:
			if ((theEvent->modifiers & activeFlag) != 0) {	// Activate event
				SetCursor(&arrow);	// Don't get an I-beam from previous word-proc app!
				if ((WindowPtr)theEvent->message == gHelpWindow)
					HiliteControl(gScrollbar, 0);
				else if ((WindowPtr)theEvent->message == gMainDialog) {
					for (i = convertAndReplaceItem; i <= helpCheckbx; i++)
						HiliteControl((ControlHandle)GetDItemHdl(gMainDialog, i), 0);
				}
			}
			else {	// Deactivate event
				if ((WindowPtr)theEvent->message == gHelpWindow)
					HiliteControl(gScrollbar, 255);
				else if ((WindowPtr)theEvent->message == gMainDialog) {
					for (i = convertAndReplaceItem; i <= helpCheckbx; i++)
						HiliteControl((ControlHandle)GetDItemHdl(gMainDialog, i), 255);
				}				
			}
		break;
		
		case app4Evt:
			if (theEvent->message & 1) {	// Resume event
				if (FrontWindow() == gMainDialog)
					for (i = convertAndReplaceItem; i <= helpCheckbx; i++)
						HiliteControl((ControlHandle)GetDItemHdl(gMainDialog, i), 0);
				else if (FrontWindow() == gHelpWindow)
					HiliteControl(gScrollbar, 0);				
			}
			else {	// Suspend event
				for (i = convertAndReplaceItem; i <= helpCheckbx; i++)
					HiliteControl((ControlHandle)GetDItemHdl(gMainDialog, i), 255);
				HiliteControl(gScrollbar, 255);				
			}
		break;

		case keyDown: case autoKey:
			char theChar;
			if (theEvent->modifiers & cmdKey) {
				theChar = theEvent->message & charCodeMask;
				switch(theChar) {
					case 'a': case 'A':
						if (FrontWindow() != gMainDialog)
							return;
						PushButton(gMainDialog, convertAndReplaceItem);
						ConvertAndReplace();
					break;

					case 'b': case 'B':
						if (FrontWindow() != gMainDialog)
							return;
						PushButton(gMainDialog, convertOnlyItem);
						ConvertOnly();
					break;

					case 'r': case 'R':
						if (FrontWindow() != gMainDialog)
							return;
						PushButton(gMainDialog, viewRgnItem);
						ViewRegion();
					break;

					case 'c': case 'C':
						if (FrontWindow() != gMainDialog)
							return;
						PushButton(gMainDialog, copyToClipItem);
						HandleMenu(MenuKey(theChar));
					break;

					case 'o': case 'O':
						if (FrontWindow() == gMainDialog)
							PushButton(gMainDialog, optionsItem);
						HandleMenu(MenuKey(theChar));
					break;

					case 'i': case 'I':
						if (FrontWindow() != gMainDialog)
							return;
						PushButton(gMainDialog, rgnInfoItem);
						ShowRegionInfo();
					break;

					case 'q': case 'Q':
						if (FrontWindow() == gMainDialog)
							PushButton(gMainDialog, quitItem);
						HandleMenu(MenuKey(theChar));
					break;

					case 's': case 'S':
						if (FrontWindow() != gMainDialog)
							return;
						PushButton(gMainDialog, saveToFileItem);
						SaveRegionToFile();
					break;

					case 'h': case 'H':
						if (FrontWindow() == gMainDialog)
							PushButton(gMainDialog, helpWindItem);
						HandleMenu(MenuKey(theChar));
					break;

					case 'p': case 'P':
						if (FrontWindow() == gMainDialog)
							PushButton(gMainDialog, viewClipboardItem);
						HandleMenu(MenuKey(theChar));
					break;

					default:
						HandleMenu(MenuKey(theChar));
					break;
				} // END switch
			}
		break;
	} // END switch
} // END HandleEvent

// ---------------------------------------------------------------------------

void HandleMenu(long menuResult) {
	short menuID = HiWord(menuResult);
	short menuItem = LoWord(menuResult);
	HiliteMenu(0);
	
	switch(menuID) {
		case kAppleMenuID:
			if (menuItem == 1)
				DoRgnMasterAbout();
		break;

		case kFileMenuID:
			switch(menuItem) {
				case 1: // Options...
					DoOptionsDialog();
				break;
				case 3: // Help Window...
					ShowHelpWindow();
				break;
				case 5: // Quit
					gDone = true;
				break;
			} // END switch
		break;

		case kEditMenuID:
			switch(menuItem) {
				case 4:
					CopyRegionToClipboard();
				break;
	
				case 7:
					ShowClipboard();
				break;
			} // END switch
		break;
	} // END switch
} // END HandleMenu

// ---------------------------------------------------------------------------

void SetupDialog() {
	Handle rgnTypeHdl;

	gAppFileRef = CurResFile();

	if (InitRegionMaker(&screenBits.bounds, 'Rgn ')) {
		SysBeep(0);
		ExitToShell();
	}

	rgnTypeHdl = Get1NamedResource('OPTN', "\pOptions");
	if (rgnTypeHdl == nil) {
		rgnTypeHdl = NewHandleClear(sizeof(ResType));
		**(ResType**)rgnTypeHdl = 'Rgn ';
		AddResource(rgnTypeHdl, 'OPTN', Unique1ID('OPTN'), "\pOptions");
		ChangedResource(rgnTypeHdl);
		WriteResource(rgnTypeHdl);
	}
	SetRegionType(**(ResType**)rgnTypeHdl);
	ReleaseResource(rgnTypeHdl);
	
	gAppleMenu = GetMenu(kAppleMenuID);
	InsertMenu(gAppleMenu, 0);
	gFileMenu = GetMenu(kFileMenuID);
	InsertMenu(gFileMenu, 0);
	gEditMenu = GetMenu(kEditMenuID);
	InsertMenu(gEditMenu, 0);

	gMainDialog = GetNewDialog(kMainDialogID, nil, (WindowPtr)-1);
	SetPort(gMainDialog);
	TextFont(geneva);
	TextSize(9);

	SetUserProc(gMainDialog, frameItem1, (ProcPtr)FrameItemRect);
	SetUserProc(gMainDialog, frameItem2, (ProcPtr)FrameGrayRect);
	SetUserProc(gMainDialog, frameItem3, (ProcPtr)FrameGrayRect);
	SetUserProc(gMainDialog, frameItem4, (ProcPtr)FrameGrayRect);
	SetUserProc(gMainDialog, helpItem, (ProcPtr)DrawHelpStatus);

	SetCtlValue((ControlHandle)GetDItemHdl(gMainDialog, helpCheckbx), gShowHelp);
	SetCtlValue((ControlHandle)GetDItemHdl(gMainDialog, previewCheckbx), gPreviewAlways);

	InitHelpWindow();

	ShowWindow(gMainDialog);
	DrawMenuBar();
} // END SetupDialog


void InitMac() {
	InitGraf(&thePort);
	InitFonts();
	FlushEvents(everyEvent,0);
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
} // END InitMac

// ---------------------------------------------------------------------------



// END RgnMaster.c++