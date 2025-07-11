/*
Copyright � 1993,1994,1995 Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"UtilsSys7.h"
#include	"AEHandlers.h"
#include	"CursorBalloon.h"
#include	"ControlStore.h"
#include	"Disasm.h"
#include	"Dump.h"
#include	"DoMenu.h"
#include	"FabTaskManager.h"
#include	"Globals.h"
#include	"InitMenus.h"
#include	"Input.h"
#include	"Main.h"
#include	"Microprogram_Ed.h"
#include	"Preferences.h"
#include	"Simulator.h"
#include	"SimUtils.h"

#if defined(FabSystem7orlater)

#pragma segment Main

extern void _DATAINIT(void);

static void DoDiskEvent(void);
static void MyZoom(WindowPtr whichWindow, short code);
static Boolean CheckTheStack(void);
static void AvoidMenuSelections(void);
static void WantMenuSelections(void);
static void AEErrorAlert(OSErr reason);

void main(void);


/* DomyKeyEvent: we got a key down, of course */

void DomyKeyEvent(EventRecord *evt)
{
enum {
kUndoKey = 0x7A10,
kCutKey = 0x7810,
kCopyKey = 0x6310,
kPasteKey = 0x7610,
kHelpKey = 0x7205,
kFwdDel = 0x757F,
kHomeKey = 0x7301,
kEndKey = 0x7704,
kPgUpKey = 0x740B,
kPgDnKey = 0x790C,
kClearKey = 0x471B,
kEnterKey = 0x4C03
};

register WindowPtr	frontW;
register short	txtStart;
register unsigned char	ch;

frontW = FrontWindow();
ch = CHARFROMMESSAGE(evt->message);
if (evt->modifiers & cmdKey) {
	AdjustMenus();
	Handle_My_Menu(MenuKey(ch));
	}
else {
	switch ((unsigned short)evt->message) {
		case kUndoKey	:	Handle_My_Menu((kRes_Menu_Edit << 16) + kMItem_Undo);
			break;
		case kCutKey	:	Handle_My_Menu((kRes_Menu_Edit << 16) + kMItem_Cut);
			if (gWPtr_Microprogram_Ed == frontW)
				Key_Microprogram_Ed(evt, true);
			break;
		case kCopyKey	:	Handle_My_Menu((kRes_Menu_Edit << 16) + kMItem_Copy);
			break;
		case kPasteKey	:	Handle_My_Menu((kRes_Menu_Edit << 16) + kMItem_Paste);
			if (gWPtr_Microprogram_Ed == frontW)
				Key_Microprogram_Ed(evt, true);
			break;
		case kHelpKey	:	(void)HMSetBalloons(1 - HMGetBalloons());
							RecalcMouseRegion(frontW, evt->where);
			break;
		case kFwdDel	:
			if (gTheInput) {
				if ((txtStart = (*gTheInput)->selStart) == (*gTheInput)->selEnd)
					TESetSelect(txtStart, txtStart + 1, gTheInput);
				TEDelete(gTheInput);
				if (gWPtr_Microprogram_Ed == frontW)
					Key_Microprogram_Ed(evt, true);
				}
			break;
		case kClearKey	:
			if (gTheInput) {
				TEDelete(gTheInput);
				if (gWPtr_Microprogram_Ed == frontW)
					Key_Microprogram_Ed(evt, true);
				}
			break;
		case kHomeKey	:
			if (gWPtr_Dump == frontW)
				DumpHome();
			else if (gWPtr_Disasm == frontW)
				DisasmHome();
			else if (gWPtr_IO == frontW)
				IOHome();
			break;
		case kEndKey	:
			if (gWPtr_Dump == frontW)
				DumpEnd();
			else if (gWPtr_Disasm == frontW)
				DisasmEnd();
			else if (gWPtr_IO == frontW)
				IOEnd();
			break;
		case kPgUpKey	:
			if (gWPtr_Dump == frontW)
				DumpPgUp();
			else if (gWPtr_Disasm == frontW)
				DisasmPgUp();
			else if (gWPtr_IO == frontW)
				IOPgUp();
			break;
		case kPgDnKey	:
			if (gWPtr_Dump == frontW)
				DumpPgDn();
			else if (gWPtr_Disasm == frontW)
				DisasmPgDn();
			else if (gWPtr_IO == frontW)
				IOPgDn();
			break;
		case kEnterKey:
			if (gTheInput) {
				TESelView(gTheInput);
				if (gWPtr_IO == frontW)
					AdjustScrollbars(frontW, false);
				}
			break;
		default:
			if (gWPtr_Microprogram_Ed == frontW)
				Key_Microprogram_Ed(evt, false);
			else
				DoKeyDown(frontW, ch, true);
		}
	}
}

/* DoDiskEvent: checks for inserted disks */

void DoDiskEvent(void)
{
if (HiWrd(gMyEvent.message)) {
//	myEvent.where.h = ((qd.screenBits.bounds.right - qd.screenBits.bounds.left)>>1) - (304 / 2);
//	myEvent.where.v = ((qd.screenBits.bounds.bottom - qd.screenBits.bounds.top) / 3) - (104 / 2);
	InitCursor();
	DILoad();
	(void)DIBadMount(gMyEvent.where, gMyEvent.message);
	DIUnload();
	}
}

/* DoZoom: a window was zoomed */

void MyZoom(WindowPtr whichWindow, short code)
{
Rect	globalPortRect, theSect, zoomRect;
GDHandle	nthDevice, dominantGDevice;
register RectPtr	rectP;
long	sectArea, greatestArea;
short	bias;

/* theEvent is a global EventRecord from the main event loop */
/* savePort is a global GrafPtr for scratch */

EraseRect(&whichWindow->portRect);

/* If there is the possibility of multiple gDevices, then we */
/* must check them to make sure we are zooming onto the right */
/* display device when zooming out. */
if ((code == inZoomOut) && gHasColorQD) {
	/* window's portRect must be converted to global coordinates */
	globalPortRect = whichWindow->portRect;
	LocalToGlobal(&topLeft(globalPortRect));
	LocalToGlobal(&botRight(globalPortRect));
	/* must calculate height of window's title bar */
	bias = globalPortRect.top - 1 - (*((WindowPeek)whichWindow)->strucRgn)->rgnBBox.top;
	nthDevice = GetDeviceList();
	greatestArea = 0;
	/* This loop checks the window against all the gdRects in the */
	/* gDevice list and remembers which gdRect contains the largest */
	/* portion of the window being zoomed. */
	while (nthDevice) {
		(void)SectRect(&globalPortRect, &(*nthDevice)->gdRect, &theSect);
		sectArea = (long)(theSect.right - theSect.left) * (theSect.bottom - theSect.top);
		if (sectArea > greatestArea) {
			greatestArea = sectArea;
			dominantGDevice = nthDevice;
			}
		nthDevice = GetNextDevice(nthDevice);
		}
	/* We must create a zoom rectangle manually in this case. */
	/* account for menu bar height as well, if on main device */
	if (dominantGDevice == GetMainDevice())
		bias += GetMBarHeight();
	rectP = &(*dominantGDevice)->gdRect;
	zoomRect.top = rectP->top + bias + 3;
	zoomRect.left = rectP->left + 3;
	zoomRect.bottom = rectP->bottom - 3;
	zoomRect.right = rectP->right - 3;
	/* Set up the WStateData record for this window. */
	(*(WStateDataHandle)((WindowPeek)whichWindow)->dataHandle)->stdState = zoomRect;
	}
ZoomWindow(whichWindow, code, true);

}

/* DoCloseWindow: a window must be closed */

void DoCloseWindow(WindowPtr w, short whichMenuItem)
{
HideWindow(w);
CheckItem(gMenu_Windows, whichMenuItem, false);
}


void DoUpdate(EventRecord *passEvt)
{
GrafPtr		savePort;
register WindowPtr	w;
register void (*theProc)(WindowPtr);

GetPort(&savePort);
SetPort(w = (WindowPtr)passEvt->message);
BeginUpdate(w);
if (IsFabWindow(w)) {
	theProc = ((FabWindowPtr)w)->updateProc;
	if (theProc)
		theProc(w);
	}
EndUpdate(w);
SetPort(savePort);
}

void DoActivate(EventRecord *passEvt)
{
register WindowPtr	whichWindow;

whichWindow = (WindowPtr)passEvt->message;
DecideActivation(passEvt, whichWindow, passEvt->where, passEvt->modifiers & activeFlag);
}

/* DecideActivation: common routine because of activate and
suspend&resume events */

void DecideActivation(EventRecord *evt, WindowPtr w, Point mouseGlob, Boolean active)
{
GrafPtr	savePort;
register void (*theActivProc)(EventRecord *, WindowPtr, Boolean);

GetPort(&savePort);
SetPort(w);

if (IsFabWindow(w)) {
	theActivProc = ((FabWindowPtr)w)->activateProc;
	if (theActivProc)
		theActivProc(evt, w, active);
	}
if (active) {
	RecalcMouseRegion(w, mouseGlob);
	}
SetPort(savePort);
}

void DoOSEvent(EventRecord *passEvt)
{
register WindowPtr	frontW = FrontWindow();
register Boolean	willbeActive;

if ((*(Byte *)&passEvt->message) == suspendResumeMessage) {
	InitCursor();
	if (willbeActive = (CHARFROMMESSAGE(passEvt->message) & resumeFlag)) {
		/* resume */
		gInTheForeground = true;
		if (CHARFROMMESSAGE(passEvt->message) & convertClipboardFlag)
			(void)TEFromScrap();
		}
	else {
		/* suspend */
		gInTheForeground = false;
		}
	if (frontW)
		DecideActivation(passEvt, frontW, passEvt->where, willbeActive);
	}

else if ((*(Byte *)&passEvt->message) == mouseMovedMessage) {
		RecalcMouseRegion(frontW, passEvt->where);
		}
}

void DoHighLevelEvent(EventRecord *evt)
{
OSErr	err;

err = AEProcessAppleEvent(evt);
if (err)
	if (err != userCanceledErr && err != errAEEventNotHandled) {
		AEErrorAlert(err);
		UnloadSeg(AEErrorAlert);
		}
}

void main(void)
{
GrafPtr		savePort;
WindowPtr	w;
register void (*theProc)(WindowPtr);
register void (*theDoGrowProc)(WindowPtr, EventRecord *);
register RectPtr	wStateP;
register short	code;

MaxApplZone();
//UnloadSeg(_DATAINIT);
InitAll();
UnloadSeg(InitAll);
do {
if (gRstatus)
	gSleep = 0L;
else if (gInTheForeground && gTheInput && ((*gTheInput)->selStart == (*gTheInput)->selEnd))
	gSleep = LMGetCaretTime();
else
	gSleep = ULONG_MAX;

	if (WaitNextEvent(everyEvent, &gMyEvent, gSleep, mouseRgn)) {
		switch (gMyEvent.what) {
			case nullEvent :
				DoIdle();
			case mouseDown :
				code = FindWindow(gMyEvent.where, &w);
				switch (code) {
					case inMenuBar :
						AdjustMenus();
						Handle_My_Menu(MenuSelect(gMyEvent.where));
						break;
					case inDrag :
						DragWindow(w, gMyEvent.where, &qd.screenBits.bounds);
						GetPort(&savePort);
						SetPort(w);
						if (((WindowPeek)w)->spareFlag) {
							wStateP = &(*(WStateDataHandle)((WindowPeek)w)->dataHandle)->userState;
							*wStateP = w->portRect;
							LocalToGlobal(&topLeft(*wStateP));
							LocalToGlobal(&botRight(*wStateP));
							}
						if (IsFabWindow(w)) {
							theProc = ((FabWindowPtr)w)->dragProc;
							if (theProc)
								theProc(w);
							RecalcGlobalCoords((FabWindowPtr)w);
							}
						(void) EventAvail(0, &gMyEvent);
						RecalcMouseRegion(w, gMyEvent.where);
						SetPort(savePort);
						break;
					case inGrow :
						GetPort(&savePort);
						SetPort(w);
						if (IsFabWindow(w)) {
							theDoGrowProc = ((FabWindowPtr)w)->growProc;
							if (theDoGrowProc)
								theDoGrowProc(w, &gMyEvent);
							ResizeObjects((FabWindowPtr)w);
							RecalcGlobalCoords((FabWindowPtr)w);
							}
						(void) EventAvail(0, &gMyEvent);
						RecalcMouseRegion(w, gMyEvent.where);
						SetPort(savePort);
						break;
					case inZoomIn :
					case inZoomOut :
						GetPort(&savePort);
						SetPort(w);
						if (TrackBox(w, gMyEvent.where, code)) {
							MyZoom(w, code);
							
							if (IsFabWindow(w)) {
								theProc = ((FabWindowPtr)w)->zoomProc;
								if (theProc)
									theProc(w);
								ResizeObjects((FabWindowPtr)w);
								RecalcGlobalCoords((FabWindowPtr)w);
								}
							}
						(void) EventAvail(0, &gMyEvent);
						RecalcMouseRegion(w, gMyEvent.where);
						SetPort(savePort);
						break;
					case inGoAway :
						if (TrackGoAway(w, gMyEvent.where)) {
							theProc = ((FabWindowPtr)w)->goAwayProc;
							if (theProc)
								theProc(w);
							}
						break;
					case inContent :
						if (w != FrontWindow()) 
							SelectWindow(w);
						else {
							if (IsFabWindow(w)) {
								GetPort(&savePort);
								SetPort(w);
								theDoGrowProc = ((FabWindowPtr)w)->contentProc;
								if (theDoGrowProc)
									theDoGrowProc(w, &gMyEvent);
								SetPort(savePort);
								}
							}
						break;
					case inSysWindow :
						SystemClick(&gMyEvent, w);
						break;
					}
				break;
			case keyDown:
			case autoKey:
				DomyKeyEvent(&gMyEvent);
				break;
			case updateEvt :
				DoUpdate(&gMyEvent);
				break;
			case diskEvt :
				DoDiskEvent();
				break;
			case activateEvt :
				DoActivate(&gMyEvent);
				break;
			case osEvt :
				DoOSEvent(&gMyEvent);
				break;
			case kHighLevelEvent:
				DoHighLevelEvent(&gMyEvent);
				break;
			}
		}
	else
		DoIdle();
	}
while (gDoneFlag == false);
SavePreferencesFile();
CleanUp();
}

/* DoIdle: so that we have something to do while the user has nothing to do */

void DoIdle(void)
{
enum {
kMIN_COMPUTE = 5L
};

long	tickc;
Boolean	stackProblems;

CheckCallQueue();
if (gInTheForeground)
	if (gTheInput)
		TEIdle(gTheInput);

if (gRstatus) {
	if (CheckTheStack() == false) {
		if (gPwrManagerIsPresent)
			(void)IdleUpdate();
		switch (gRstatus) {
			case kST_GOING:
				tickc = TickCount();
				do {
					ExecuteInstructionsGO();
					stackProblems = CheckTheStack();
					}
				while ((TickCount() - tickc < kMIN_COMPUTE) && (stackProblems == false));
				break;
			case kST_STEPASM:
				ExecuteInstructionsGO();
				StopIt();
				break;
			case kST_STEPMPROG:
				ExecuteInstructions(gSubclk++);
				gSubclk &= 3;
				if (gSubclk == 0)
					gRstatus = kST_STOPPED;
				break;
			case kST_STEPSUBCYC:
				ExecuteInstructions(gSubclk++);
				gSubclk &= 3;
				gRstatus = kST_STOPPED;
				break;
			}
		}
	}
}

Boolean CheckTheStack(void)
{
Boolean	result = true;

if ((unsigned short)gRegs[kREG_SP] > gPrefs.DefSPValue) {
	if (gRegs[kREG_PC] == kPCExitToShell)
		(void)NoteAlert_AE(kALRT_TOSHELL, myStdFilterProcNoCancel, myIdleFunct);
	else
		(void)StopAlert_AE(kALRT_STKUNDERFLOW, myStdFilterProcNoCancel, myIdleFunct);
	StopIt();
	}
else if ((unsigned short)gRegs[kREG_SP] < (gPrefs.DefSPValue - gPrefs.DefStkSize)) {
	(void)StopAlert_AE(kALRT_STKOVERFLOW, myStdFilterProcNoCancel, myIdleFunct);
	StopIt();
	}
else
	result = false;
return result;
}

/* AdjustMenus: we set the menus before MenuSelect or MenuKey */

void AdjustMenus(void)
{
register WindowPtr	window;
long	offset;
Boolean	undo = false;
Boolean	cutCopyClear = false;
Boolean	paste = false;
Boolean	selectAll = false;
Boolean	insertdelete = false;
Boolean	go = false;
Boolean	step = false;
Boolean	stop = false;

if (window = FrontWindow()) {
	if ( ISDAWINDOW(window) || isMovableModal(window)) {
		undo = true;				/* all editing is enabled for DA windows */
		cutCopyClear = true;
		paste = true;
		}
	else if ( ISAPPWINDOW(window) ) {
	/* Cut, Copy, and Clear is enabled for appl. windows with selections */
		if (gTheInput) {
			selectAll = true;
			if ( (*gTheInput)->selStart < (*gTheInput)->selEnd )
				cutCopyClear = true;
			}

		if ((window == gWPtr_Microprogram_Ed) && (keyDownDest == kKEY_LIST)) {
			cutCopyClear = true;
			if (theSelection[kL_COMMENTS] != maxLLine[kL_COMMENTS])
				insertdelete = true;
			}
	/* if there�s any text in the clipboard, paste is enabled */
		if ((GetScrap(nil, 'TEXT', &offset)  > 0) &&
			((gTheInput) || ((window == gWPtr_Microprogram_Ed) && (keyDownDest == kKEY_LIST))))
			paste = true;
		}
	if ( undo )
		EnableItem(gMenu_Edit, kMItem_Undo);
	else
		DisableItem(gMenu_Edit, kMItem_Undo);
	if ( cutCopyClear ) {
		EnableItem(gMenu_Edit, kMItem_Cut);
		EnableItem(gMenu_Edit, kMItem_Copy);
		EnableItem(gMenu_Edit, kMItem_Clear);
		}
	else {
		DisableItem(gMenu_Edit, kMItem_Cut);
		DisableItem(gMenu_Edit, kMItem_Copy);
		DisableItem(gMenu_Edit, kMItem_Clear);
		}
	if ( paste )
		EnableItem(gMenu_Edit, kMItem_Paste);
	else
		DisableItem(gMenu_Edit, kMItem_Paste);
	if ( selectAll )
		EnableItem(gMenu_Edit, kMItem_Select_All);
	else
		DisableItem(gMenu_Edit, kMItem_Select_All);
	if ( insertdelete ) {
		EnableItem(gMenu_Edit, kMItem_Insert);
		EnableItem(gMenu_Edit, kMItem_Delete);
		}
	else {
		DisableItem(gMenu_Edit, kMItem_Insert);
		DisableItem(gMenu_Edit, kMItem_Delete);
		}
	if (isMovableModal(window))
		AvoidMenuSelections();
	else
		WantMenuSelections();
	}
else {	/* no windows are open */
	DisableItem(gMenu_Edit, kMItem_Undo);	/* check this when implementing undo */
	DisableItem(gMenu_Edit, kMItem_Cut);
	DisableItem(gMenu_Edit, kMItem_Copy);
	DisableItem(gMenu_Edit, kMItem_Clear);
	DisableItem(gMenu_Edit, kMItem_Paste);
	DisableItem(gMenu_Edit, kMItem_Select_All);
	DisableItem(gMenu_Edit, kMItem_Insert);
	DisableItem(gMenu_Edit, kMItem_Delete);
	WantMenuSelections();
	}

if (((WindowPeek)gWPtr_Disasm)->visible)
	EnableItem(gMenu_Control, kMItem_Disasmfrom);
else
	DisableItem(gMenu_Control, kMItem_Disasmfrom);

if (((WindowPeek)gWPtr_Dump)->visible)
	EnableItem(gMenu_Control, kMItem_Dumpfrom);
else
	DisableItem(gMenu_Control, kMItem_Dumpfrom);

if (DocIsOpen) {
	if (gRstatus == kST_STOPPED) {
		go = true;
		step = true;
		}
	if (gRstatus == kST_GOING) {
		stop = true;
		}
	EnableItem(gMenu_Windows, kMItem_Microprogram);
	EnableItem(gMenu_File, kMItem_Close);
	EnableItem(gMenu_File, kMItem_Save_Control_St2);
	EnableItem(gMenu_File, kMItem_Save_CS_Stationery);
	EnableItem(gMenu_Control, kMItem_SelectCurMPC);
	}
else {
	DisableItem(gMenu_Windows, kMItem_Microprogram);
	DisableItem(gMenu_File, kMItem_Close);
	DisableItem(gMenu_File, kMItem_Save_Control_St2);
	DisableItem(gMenu_File, kMItem_Save_CS_Stationery);
	DisableItem(gMenu_Control, kMItem_SelectCurMPC);
	}
if (go)
	EnableItem(gMenu_Control, kMItem_Go);
else
	DisableItem(gMenu_Control, kMItem_Go);
if (step) {
	EnableItem(gMenu_Control, kMItem_StepSub);
	EnableItem(gMenu_Control, kMItem_StepMicro);
	EnableItem(gMenu_Control, kMItem_StepInstr);
	}
else {
	DisableItem(gMenu_Control, kMItem_StepSub);
	DisableItem(gMenu_Control, kMItem_StepMicro);
	DisableItem(gMenu_Control, kMItem_StepInstr);
	}
if (stop)
	EnableItem(gMenu_Control, kMItem_Stop);
else
	DisableItem(gMenu_Control, kMItem_Stop);
if (gRstatus == kST_GOING) {
	DisableItem(gMenu_Control, kMItem_ResetMem);
	DisableItem(gMenu_Control, kMItem_ResetRegs);
	}
else {
	EnableItem(gMenu_Control, kMItem_ResetMem);
	EnableItem(gMenu_Control, kMItem_ResetRegs);
	}
} /*AdjustMenus*/

void AvoidMenuSelections(void)
{
DisableItem(gMenu_File, 0);
DisableItem(gMenu_Apple, 1);
DisableItem(gMenu_Windows, 0);
DisableItem(gMenu_Control, 0);
DisableItem(gMenu_Assembler, 0);
}

void WantMenuSelections(void)
{
EnableItem(gMenu_File, 0);
EnableItem(gMenu_Apple, 1);
EnableItem(gMenu_Windows, 0);
EnableItem(gMenu_Control, 0);
EnableItem(gMenu_Assembler, 0);
}

#pragma segment Rare

/* AEErrorAlert: errors while handling AEvents */

static void AEErrorAlert(OSErr reason)
{
Str255	tempS;

MyNumToString(reason, tempS);
ParamText(tempS, nil, nil, nil);
(void)StopAlert_UPP(kALRT_AEPROCESSERR, myStdFilterProcNoCancel);
}

#pragma segment Main

#endif

