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
#include	"Globals.h"
#include	"AEHandlers.h"
#include	"Animation.h"
#include	"ControlStore.h"
#include	"CursorBalloon.h"
#include	"Disasm.h"
#include	"DoEditDialog.h"
#include	"DoMenu.h"
#include	"DragManSim.h"
#include	"Dump.h"
#include	"InitCursorBalloon.h"
#include	"InitMenus.h"
#include	"Input.h"
#include	"Main.h"
#include	"Microprogram_Ed.h"
#include	"Registers.h"
#include	"SimAsm.h"
#include	"SimUtils.h"
#include	"SoundHandling.h"
#include	"Independents.h"

#if defined(FabSystem7orlater)

#pragma segment Main

static AEEventHandlerUPP	gmyHandleIO_UPP, gmyHandleGenericAlert_UPP;

#pragma segment Init

enum {
kREGFONT = 128
};

enum winds {
kWIND_Microprogram_Ed = 256,
kWIND_Animation,
kWIND_IO,
kWIND_Registers,
kWIND_Dump,
kWIND_Disasm
};

static void Init_My_Menus(void);
static void GestaltCheck(void);
static OSErr Init_Microprogram_Ed(void);
static OSErr Init_Animation(void);
static OSErr Init_Registers(void);
static OSErr Init_Disasm(void);
static Boolean InitPrefs(void);
static Boolean	GetFontNumber(ConstStr255Param fontName, short *fontNum);
static ListHandle MakeList(RectPtr theListRect, WindowPtr theWindow, short lines);
static TEHandle MakeTE(RectPtr Position);
static OSErr InitIO(void);
static OSErr InitDump(void);
static void FatalErrorAlert(short alertID, OSErr reason);

/* Init_My_Menus: sets up the menu bar */

static void Init_My_Menus(void)
{
	{
	register Handle menuBar;
	
	SetMenuBar(menuBar = GetNewMBar(kRes_Menu_Apple));
	DisposeHandle(menuBar);
	}

AppendResMenu(gMenu_Apple = GetMenuHandle(kRes_Menu_Apple),'DRVR');
gMenu_File = GetMenuHandle(kRes_Menu_File);
gMenu_Edit = GetMenuHandle(kRes_Menu_Edit);
gMenu_Windows = GetMenuHandle(kRes_Menu_Windows);
gMenu_Control = GetMenuHandle(kRes_Menu_Control);
gMenu_Assembler = GetMenuHandle(kRes_Menu_Assembler);
InsertMenu(gMenu_Memory = GetMenu(kRes_Menu_HMemory), hierMenu);
InsertMenu(gMenu_Registers = GetMenu(kRes_Menu_HRegs), hierMenu);
InsertMenu(gMenu_DisasmF = GetMenu(kRes_Menu_HDisasm), hierMenu);
InsertMenu(gMenu_DumpF = GetMenu(kRes_Menu_HDump), hierMenu);
gPopMenu = GetMenu(kRes_Menu_PopDump);
DrawMenuBar();
}

/* InitAll: initializes the whole environment */

void InitAll(void)
{
EventRecord	dummyEvent;
long	timingInfo;
Rect	startingRect, endingRect;
Point	dummyPt = {0, 0};
DialogPtr	agh;
register Handle	splash;
register OSErr	err;
register Boolean	prefsFileWasAbsent;

timingInfo = TickCount();

gMyGrowZoneUPP = NewGrowZoneProc(MyGrowZone);
SetGrowZone(gMyGrowZoneUPP);
MoreMasters();
MoreMasters();
MoreMasters();
MoreMasters();
MoreMasters();
MoreMasters();
InitGraf(&qd.thePort);
InitFonts();
InitWindows();
InitMenus();
TEInit();
InitDialogs(nil);

FlushEvents(everyEvent, 0);
InitCursor();

(void)UnloadScrap();

GestaltCheck();

gWatchHandle = GetCursor(watchCursor);
SetCursor(*gWatchHandle);

InitMySoundHandling();

gInstrClikLoopUPP = MyNewListClickLoopProc(instrClikLoop);
gCommentClikLoopUPP = MyNewListClickLoopProc(commentClikLoop);
gSwitchCursClikLoopUPP = MyNewListClickLoopProc(switchCursClikLoop);

(void)InitCursorBalloonManager();

splash = OpenSplash(&agh, nil);
SetPort(agh);
endingRect = agh->portRect;
LocalToGlobal(&topLeft(endingRect));
LocalToGlobal(&botRight(endingRect));
startingRect = endingRect;
InsetRect(&startingRect, ((endingRect.right - endingRect.left) >> 1) - 2,
			((endingRect.bottom - endingRect.top) >> 1) - 2);

timingInfo = TickCount() - timingInfo;
if (timingInfo >= 28)	/* unusually slow machine */
	gZoomFactor = 4;
else if (timingInfo >= 17)	/* Plus, SE, Classic Mac */
	gZoomFactor = 5;
else if (timingInfo >= 10)
	gZoomFactor = 6;
else if (timingInfo >= 4)	/* Quadra class Mac */
	gZoomFactor = 7;
else if (timingInfo >= 2)
	gZoomFactor = 8;
else	/* super-duper (RISC?) Macintosh */
	gZoomFactor = 9;

ZoomRectToRectAutoGrafPort(&startingRect, &endingRect, gZoomFactor);
ShowWindow(agh);
DrawDialog(agh);

gIBeamHandle = GetCursor(iBeamCursor);
gPlusHandle = GetCursor(plusCursor);

gMMemory = NewPtrClear(kSIZE_RAM);
gCsMemory = (union u_mir *)NewPtrClear(kSIZE_ASSMEM + kSIZE_CSMEM);
if (gCsMemory)
	gAssMemory = (((Ptr)gCsMemory) + kSIZE_CSMEM);

MyFillBuffer((long *)gAssMemory, numOfLongs(kSIZE_ASSMEM), 0x03030303);
Init_My_Menus();

gRegs[kREG_ZERO] = 0;
gRegs[kREG_ONE] = 1;
gRegs[kREG_MINUS1] = -1;

prefsFileWasAbsent = InitPrefs();
ResetMemory();
ResetRegisters();
UnloadSeg(ResetRegisters);
if (Init_Animation()) ExitToShell();
if (Init_Microprogram_Ed()) ExitToShell();
if (Init_Registers()) ExitToShell();
if (InitIO()) ExitToShell();
if (InitDump()) ExitToShell();
if (Init_Disasm()) ExitToShell();
SetMir(-1L);
SetMir(0L);

if (prefsFileWasAbsent)
	gPrefs.remembWind = true;

if(err = InstallRequiredAEHandlers(myHandleOAPP, myHandleODOC, myHandlePDOC, myHandleQUIT)) {
	FatalErrorAlert(kALRT_GENERICERROR, err);
	gDoneFlag = true;
	}

gmyHandleIO_UPP = NewAEEventHandlerProc(myHandleIO);
gmyHandleGenericAlert_UPP = NewAEEventHandlerProc(myHandleGenericAlert);
if(err = AEInstallEventHandler(kFCR_MINE, kAEmySignalIO, gmyHandleIO_UPP, 0, false)) {
	FatalErrorAlert(kALRT_GENERICERROR, err);
	gDoneFlag = true;
	}

if(err = AEInstallEventHandler(kCreat, kAEAlert, gmyHandleGenericAlert_UPP, 0, false)) {
	FatalErrorAlert(kALRT_GENERICERROR, err);
	gDoneFlag = true;
	}

(void)TEFromScrap();

DisposeSplash(splash, agh);
UnloadSeg(DisposeSplash);

(void) EventAvail(0, &dummyEvent);

if (gPrefs.AnimVisible)
	ShowWindowAndCheckMenu(gWPtr_Animation, kMItem_Animation);
if (gPrefs.DisasmVisible) {
	DecideActivation(&dummyEvent, gWPtr_Disasm, dummyPt, false);
	ShowWindowAndCheckMenu(gWPtr_Disasm, kMItem_Disasm);
	}
if (gPrefs.DumpVisible) {
	DecideActivation(&dummyEvent, gWPtr_Dump, dummyPt, false);
	ShowWindowAndCheckMenu(gWPtr_Dump, kMItem_Dump);
	}
if (gPrefs.IOVisible) {
	DecideActivation(&dummyEvent, gWPtr_IO, dummyPt, false);
	ShowWindowAndCheckMenu(gWPtr_IO, kMItem_IO);
	}
if (gPrefs.RegsVisible)
	ShowWindowAndCheckMenu(gWPtr_Registers, kMItem_Registers);

gILCBase = gRegs[kREG_PC];

InitCursor();
}

/* GestaltCheck: is this Mac set up with all we need to run? */

static void GestaltCheck(void)
{
#define	POWERMANAGER	((1L << gestaltPMgrExists)|(1L << gestaltPMgrCPUIdle))
#define TEMPMEMMANAGER	((1L << gestaltTempMemSupport)|(1L << gestaltRealTempMemory)|(1L << gestaltTempMemTracked))
long	Gresp;

if (TrapAvailable(_Gestalt)) {
	if (Gestalt(gestaltDragMgrAttr, &Gresp) == noErr)
		gDragManagerActive = Gresp & (1L << gestaltDragMgrPresent);
	if (Gestalt(gestaltQuickdrawVersion, &Gresp) == noErr)
		gHasColorQD = ((Byte)(Gresp >> 8)) > 0;
	if (Gestalt(gestaltPowerMgrAttr, &Gresp) == noErr)
		gPwrManagerIsPresent = ((Gresp & POWERMANAGER) == POWERMANAGER) != 0;
	if (Gestalt(gestaltAliasMgrAttr, &Gresp) == noErr)
		if (Gresp & (1L << gestaltAliasMgrPresent))
			if (Gestalt(gestaltAppleEventsAttr, &Gresp) == noErr)
				if (Gresp & (1L << gestaltAppleEventsPresent))
					if (Gestalt(gestaltDITLExtAttr, &Gresp) == noErr)
						if (Gresp & (1L << gestaltDITLExtPresent))
							if (Gestalt(gestaltFindFolderAttr, &Gresp) == noErr)
								if (Gresp & (1L << gestaltFindFolderPresent))
							if (Gestalt(gestaltFSAttr, &Gresp) == noErr)
								if (Gresp & (1L << gestaltHasFSSpecCalls))
							if (Gestalt(gestaltHelpMgrAttr, &Gresp) == noErr)
								if (Gresp & (1L << gestaltHelpMgrPresent))
							if (Gestalt(gestaltPopupAttr, &Gresp) == noErr)
								if (Gresp & (1L << gestaltPopupPresent))
					//		if (Gestalt(gestaltResourceMgrAttr, &Gresp) == noErr)
					//			if (Gresp & (1L << gestaltPartialRsrcs))
							if (Gestalt(gestaltStandardFileAttr, &Gresp) == noErr)
								if (Gresp & (1L << gestaltStandardFile58))
							if (Gestalt(gestaltTextEditVersion, &Gresp) == noErr)
								if (Gresp >= gestaltTE4)
							if (Gestalt(gestaltOSAttr, &Gresp) == noErr)
								if ((Gresp & TEMPMEMMANAGER) == TEMPMEMMANAGER)
							/* proceed because everything is OK */
								return;
	}
(void)StopAlert_UPP(kALRT_OLDSYSTEM, nil);
ExitToShell();
}

/* Init_Microprogram_Ed: initializes the Microprogram window */

static OSErr Init_Microprogram_Ed(void)
{
enum {
kR_RECTS = 128
};

register Handle	tempSH;
register ControlHandle *ControlScan;
register RgnHandle	theRgn;
register WindowPtr	w;
Rect	tempRect;
Point tempCell;
register short i;
register OSErr	err = appMemFullErr;

tempCell.h = 0;
	{
	register RectPtr	RectScan, *keyRScan;
	register short	numrects;
	short *numrectsPtr;

	RectScan = (RectPtr)((numrectsPtr = (short *)*Get1Resource('nrct', kR_RECTS)) + 1);
	for (numrects = *numrectsPtr, keyRScan = keyrects, i = 0; i < numrects; i++)
		*keyRScan++ = RectScan++;
	}
maxLLine[kL_COMMENTS] = 127;
tempSH = Get1Resource(krInstructions, kOPCODES);
maxLLine[kL_INSTR] = *(unsigned short *)*tempSH;
gWPtr_Microprogram_Ed = w = GetNewWindow(kWIND_Microprogram_Ed, NewPtrClear(sizeof(FabWindowRecord)), (WindowPtr)-1L);
if (w) {
	SetPort(w);
	if (gPrefs.remembWind)
		if (IsOnScreenWeak(gPrefs.MProgTopLeft))
			MoveWindow(w, gPrefs.MProgTopLeft.h, gPrefs.MProgTopLeft.v, false);
	TextFont(geneva);
	TextSize(9);
	for( ControlScan = &controls[kNUM_CONTROLS], i = kRES_CNTL+kNUM_CONTROLS-1;
		i >= kRES_CNTL; i-- )
		*--ControlScan = GetNewControl(i, w);
	Lists[kL_COMMENTS] = MakeList(keyrects[kKEY_LIST], w,
									maxLLine[kL_COMMENTS]+1);
	(*Lists[kL_COMMENTS])->lClickLoop = gCommentClikLoopUPP;
	Lists[kL_INSTR] = MakeList(keyrects[kKEY_INSTR], w,
								maxLLine[kL_INSTR]+1);
	(*Lists[kL_INSTR])->lClickLoop = gInstrClikLoopUPP;
	TextFont(systemFont);
	TextSize(12);
	TEs[kKEY_COMMENT] = MakeTE(keyrects[kKEY_COMMENT]);
	(void)TEFeatureFlag(teFOutlineHilite, teBitSet, TEs[kKEY_COMMENT]);
	TEs[kKEY_BRTO] = MakeTE(keyrects[kKEY_BRTO]);
	(void)TEFeatureFlag(teFOutlineHilite, teBitSet, TEs[kKEY_BRTO]);
	TextFont(geneva);
	TextSize(9);
		{
		register ROpcodePtr	instrScan;
		register SignedByte	savedState;
	
		savedState = WantThisHandleSafe(tempSH);
		instrScan = (ROpcodePtr)(((short *)(*tempSH))+1);
		for ( i = 0; i <= maxLLine[kL_INSTR]; i++, instrScan++) {
			tempCell.v = i;
			LSetCell(instrScan, 4, tempCell, Lists[kL_INSTR]);
			}
		HSetState(tempSH, savedState);
		}
	ResetMicroprogramWindow();
	UnloadSeg(ResetMicroprogramWindow);

	InitFabWindow((FabWindowPtr)w);
	SetActivate(w, Activate_Microprogram_Ed);
	SetUpdate(w, Update_Microprogram_Ed);
	SetGoAway(w, CloseMicroProg);
	SetContent(w, Do_Microprogram_Ed);

	for (i = kKEY_BRTO; i <= kKEY_INSTR; i++) {
		theRgn = NewRgn();
		tempRect = *keyrects[i];
		if (i > kKEY_COMMENT)
			tempRect.right -= kScrollbarAdjust;
		RectRgn(theRgn, &tempRect);
		InstallRgnHandler((FabWindowPtr)w, theRgn, nil, i <= kKEY_COMMENT ? gIBeamHandle : gPlusHandle,
					toMenu(kBalloons_MPrg, i + kNUM_CONTROLS + 1), 0, 0);
		}

	for (i = kPOPUP_ABUS; i <= kCHECK_ACTMAP; i++) {
		theRgn = NewRgn();
		RectRgn(theRgn, &(*controls[i])->contrlRect);
		InstallRgnHandler((FabWindowPtr)w, theRgn, nil, nil,
					toMenu(kBalloons_MPrg, i + 1), 0, 0);
		}

//	ResizeObjects(w);

	RecalcGlobalCoords((FabWindowPtr)w);

	err = noErr;
	}
return err;
}

/* Init_Animation: initializes the Animation Window */

/* da schiaffare dentro la routine successiva */
const short	initial[] = { kP_REGISTERS, kC_ALU2MSL1, kC_ALU2MSL2, kP_MIR, kP_TEXT, 0 };

static OSErr Init_Animation(void)
{
Rect	tempRect;
BitMap	offBits;
register WindowPtr	w;
register PicHandle	tempPH;
register RgnHandle	theRgn;
register long	rightLong;
register short	count, i;
register OSErr	err = appMemFullErr;

gWPtr_Animation = w = GetNewWindow(kWIND_Animation, NewPtrClear(sizeof(FabWindowRecord)), (WindowPtr)-1L);
if (w) {
	SetPort(w);
	if (gPrefs.remembWind)
		if (IsOnScreenWeak(gPrefs.AnimTopLeft))
			MoveWindow(w, gPrefs.AnimTopLeft.h, gPrefs.AnimTopLeft.v, false);
	ClipRect(&w->portRect);
	offBits.bounds = w->portRect;
	rightLong = ((offBits.bounds.right + 15) >> 4) << 1;
	offBits.rowBytes = rightLong;
	if (offScr = (GrafPtr)NewPtrClear(sizeof(GrafPort))) {
		OpenPort(offScr);
		if (offBits.baseAddr = NewPtrClear(rightLong * offBits.bounds.bottom)) {
			SetPortBits(&offBits);
			TextFont(monaco);
			ClipRect(&offBits.bounds);
			PenPat(&qd.gray);
			PenMode(notPatBic);
			mirUpdRgn = NewRgn();
				{
				register RgnHandle tempRgn = NewRgn();
				register PicHandle *PictScan;
				register RectPtr	RectScan;
			
				for(RectScan = rval, PictScan = images, count = kFIRST_PICT;
					count <= kLAST_PICT; count++) {
					register PicHandle tempPicH;
					
					tempPicH = GetPicture(count);
					*PictScan++ = tempPicH;
					tempRect = (*tempPicH)->picFrame;
					DrawPicture(tempPicH, &tempRect);
					RectRgn(tempRgn, &tempRect);
					PaintRgn(tempRgn);
					if( count <= kP_MPC ) {
						--tempRect.bottom;
						--tempRect.right;
						InsetRect(&tempRect, 3, 3);
						*RectScan++ = tempRect;
						}
					else if( count == kP_ALU ) {
						tempRect.bottom -= 4;
						tempRect.right -= 15;
						tempRect.left += 15;
						tempRect.top = tempRect.bottom - 15;
						*RectScan++ = tempRect;
						}
					else if( count == kP_MSL ) {
						tempRect.bottom -= 3;
						tempRect.top += 3;
						tempRect.right -= 3;
						tempRect.left += 19;
						*RectScan++ = tempRect;
						}
					else if( count == kP_MIR ) {
						register short	i;
			
						tempRect.bottom -= 2;
						++tempRect.left;
						tempRect.top = tempRect.bottom - 15;
						tempRect.right = tempRect.left + 11;
						for( i = 0; i <= 12; i++) {
							*RectScan++ = tempRect;
							RectRgn(tempRgn, &tempRect);
							UnionRgn(mirUpdRgn, tempRgn, mirUpdRgn);
							tempRect.left += 12;
							tempRect.right += 12;
							}
						tempRect.right += 11;
						*RectScan = tempRect;
						RectRgn(tempRgn, &tempRect);
						UnionRgn(mirUpdRgn, tempRgn, mirUpdRgn);
						}
					}
				DisposeRgn(tempRgn);
				}
			ChangedAllBoxes();
			ActivateObjs(initial);

			InitFabWindow((FabWindowPtr)w);
			SetUpdate(w, Update_Animation);
			SetGoAway(w, CloseAnimation);
			SetContent(w, Do_Animation);

			SetPort(w);
			for (i = kP_ALATCH - kFIRST_PICT; i <= kP_MAP - kFIRST_PICT; i++) {
				theRgn = NewRgn();
				tempPH = images[i];
				if (*tempPH == nil)
					LoadResource((Handle)tempPH);
				tempRect = (*tempPH)->picFrame;
				RectRgn(theRgn, &tempRect);
				InstallRgnHandler((FabWindowPtr)w, theRgn, nil, nil,
							toMenu(kBalloons_Anim, i + 1), 6, 0);
				}
		
		//	ResizeObjects(w);
		
			RecalcGlobalCoords((FabWindowPtr)w);

			err = noErr;
			}
		}
	}
return err;
}

/* Init_Registers: initializes the Registers window */

static OSErr Init_Registers(void)
{
enum {
kCNTL_BASE = 129
};

Str255	tempS;
FMetricRec	theMetrics;
Rect	tempRect;
register WindowPtr	w;
register Handle	myStrHandle;
register RgnHandle	theRgn;
register Fixed	tempFixed;
short	fontNumber, fontSize;
register short	theWidth, i;
register short	maxLength = 0;
register OSErr	err = appMemFullErr;

gWPtr_Registers = w = GetNewWindow(kWIND_Registers, NewPtrClear(sizeof(FabWindowRecord)), (WindowPtr)-1L);
if (w) {
	SetPort(w);
	if (gPrefs.remembWind)
		if (IsOnScreenWeak(gPrefs.RegsTopLeft))
			MoveWindow(w, gPrefs.RegsTopLeft.h, gPrefs.RegsTopLeft.v, false);
	(void)WantThisHandleSafe(myStrHandle = Get1Resource('REGF', kREGFONT));
	if (GetFontNumber((StringPtr)(*myStrHandle + 2), &fontNumber))
		fontSize = *(short *)*myStrHandle;
	else {
		fontNumber = monaco;
		fontSize = 9;
		}
	ReleaseResource(myStrHandle);
	TextFont(fontNumber);
	TextSize(fontSize);
	if (Ctrl_Base = GetNewControl(kCNTL_BASE, w)) {
		FontMetrics(&theMetrics);
		tempFixed = theMetrics.descent + theMetrics.ascent + theMetrics.leading;

		tempFixed = mySwap(tempFixed);

		regLineHeight = (short)tempFixed;
		
		regDistFromTop = *(short *)&theMetrics.ascent + kREG_HORZSEPLINE;
		HLock(myStrHandle = Get1Resource('STR#', kREG_NAMES));
		for(i = 0; i < kDISP_REGS; i++) {
			if (maxLength < (theWidth = StringWidth(GetPtrIndHString(myStrHandle, i))))
				maxLength = theWidth;
			}
		HUnlock(myStrHandle);
		tempFixed = maxLength;
		regVertSepLine = maxLength + kDIST_FROMLEFT + kDIST_FROMNAME;
		
		tempFixed = mySwap(tempFixed);
		
		maxLength = 0;
			{
			register Byte *myString;
			register Byte	j, rememberedCh, stopAt;
			SignedByte	savedState;
		
			savedState = WantThisHandleSafe(myStrHandle = (Handle)GetString(kSTR_BINALLOWED));
			myString = (Byte *)*myStrHandle;
			for (stopAt = *myString++, j = 0; j < stopAt; j++)
				if (maxLength < (theWidth = CharWidth(*myString++))) {
					maxLength = theWidth;
					rememberedCh = *(myString - 1);
					}
			HSetState(myStrHandle, savedState);
			for (myString = (Byte *)&tempS, j = 0; j <= 15; j++)
				*myString++ = rememberedCh;
			maxLength = TextWidth(&tempS, 0, 16);
			}
		
		tempFixed += ((long)maxLength << 16) +
			((const long)(kDIST_FROMLEFT + kDIST_FROMNAME + kDIST_FROMVERTSEP)<< 16);
		tempFixed = mySwap(tempFixed);
		SizeWindow(w, (short)tempFixed + 2,
					REG_TOP + regLineHeight * kDISP_REGS, false);
		SetControlValue(Ctrl_Base, gPrefs.remembWind ? gPrefs.RegsBase : kPOP_HEX);
		oldChoice = GetControlValue(Ctrl_Base);

		InitFabWindow((FabWindowPtr)w);
		SetUpdate(w, Update_Registers);
		SetGoAway(w, CloseRegisters);
		SetContent(w, Do_Registers);
		SetGetDragRect(w, getDragRectRegs);

		theRgn = NewRgn();
		tempRect = (*Ctrl_Base)->contrlRect;
		RectRgn(theRgn, &tempRect);
		InstallRgnHandler((FabWindowPtr)w, theRgn, nil, nil,
						toMenu(kBalloons_Regs, kBRegs_Base), 0, 0);

		tempRect.top = REG_TOP;
		tempRect.left = PRCT_L(w);
		tempRect.bottom = REG_TOP + regLineHeight;
		tempRect.right = PRCT_R(w);
		for (i = kBRegs_PC; i <= kBRegs_Lo8; i++) {
			theRgn = NewRgn();
			RectRgn(theRgn, &tempRect);
			InstallRgnHandler((FabWindowPtr)w, theRgn, nil, nil,
							toMenu(kBalloons_Regs, i), 0, 0);
			tempRect.top = tempRect.bottom;
			tempRect.bottom += regLineHeight;
			}

	//	ResizeObjects(w);
	
		RecalcGlobalCoords((FabWindowPtr)w);
		if (gDragManagerActive)
			(void) MyInstallHWindow(w);

		err = noErr;
		}
	}
//SetFractEnable(false);
return err;
}

/* Init_Disasm: initializes the Disassembler window */

static OSErr Init_Disasm(void)
{
enum {
rdisasmVScroll = 145
};

FMetricRec	theMetrics;
register Fixed	tempFix;
register WindowPtr	window;
register OSErr	err = appMemFullErr;

gWPtr_Disasm = window = GetNewWindow(kWIND_Disasm, NewPtrClear(sizeof(FabWindowRecord)), (WindowPtr)-1L);
if (window) {
	SetPort(window);
	if (gPrefs.remembWind)
		if (IsOnScreenWeak(gPrefs.DisasmTopLeft))
			MoveWindow(window, gPrefs.DisasmTopLeft.h, gPrefs.DisasmTopLeft.v, false);
	if (disasmVScroll = GetNewControl(rdisasmVScroll, window)) {
		TextFont(monaco);
		TextSize(9);
		TextMode(srcCopy);
		FontMetrics(&theMetrics);
		tempFix = theMetrics.descent + theMetrics.ascent + theMetrics.leading;
		tempFix = mySwap(tempFix);
		disasmLineHeight = (short)tempFix;
		disasmFromTop = *(short *)&theMetrics.ascent;
		disasmCWidMax = *(short *)&theMetrics.widMax;
		tempFix = FixMul(theMetrics.widMax, kMaxCharsInOneDisasmLineFixed);
		tempFix = mySwap(tempFix);
		SizeWindow(window, (short)tempFix + kScrollbarWidth + kDIST_FROMLEFT,
			gPrefs.remembWind ? gPrefs.DisasmHeight :
				((PRCT_B(window) - PRCT_T(window)) / disasmLineHeight) * disasmLineHeight, false);
		SetupVertScrollBar(window, disasmVScroll);
		SetupDisasmCtlMax(disasmVScroll);
		if (gPrefs.remembWind)
			SetControlValue(disasmVScroll, gPrefs.DisasmScrollVal);

		InitFabWindow((FabWindowPtr)window);
		SetActivate(window, Activate_Disasm);
		SetUpdate(window, Update_Disasm);
		SetGrow(window, Grow_Disasm);
		SetGoAway(window, CloseDisasm);
		SetContent(window, Do_Disasm);
		SetGetDragRect(window, getDragRectDisasm);

		InstallRgnHandler((FabWindowPtr)window, NewRgn(), RecalcDump, nil,
				toMenu(kBalloons_Dump, kBDisasm_Contents), 0, 0);

		ResizeObjects((FabWindowPtr)window);
		RecalcGlobalCoords((FabWindowPtr)window);

		if (gDragManagerActive)
			(void) MyInstallHWindow(window);
		err = noErr;
		}
	}
return err;
}

/* InitPrefs: loads preferences if present */

Boolean InitPrefs(void)
{
ParamBlockRec	myPB;
EventRecord	dummyEv;
FSSpec	myFSS;
register Handle	myStrHand, prefsBuffer;
short	prefsFRefNum;
Boolean	targetFolder, isAnAlias;
register OSErr	err;
register SignedByte	oldState;
register Boolean	prefsFileAbsent = false;

oldState = WantThisHandleSafe(myStrHand = (Handle)GetString(kSTR_PREFSFILENAME));
if ((err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
			&myFSS.vRefNum, &myFSS.parID)) == noErr) {
	err = FSMakeFSSpecCompat(myFSS.vRefNum, myFSS.parID, (ConstStr255Param)*myStrHand, &myFSS);
	if (err == fnfErr)
		prefsFileAbsent = true;
	else if (err == noErr)
		if ((err = ResolveAliasFile(&myFSS, true, &targetFolder, &isAnAlias)) == noErr)
			if (targetFolder)
				err = paramErr;
			else
				if ((err = FSpOpenDFCompat(&myFSS, fsRdPerm, &prefsFRefNum)) == noErr) {
					if (prefsBuffer = NewHandleGeneral(sizeof(struct myprefs))) {
						myPB.ioParam.ioCompletion = nil;
						myPB.ioParam.ioRefNum = prefsFRefNum;
						HLock(prefsBuffer);
						myPB.ioParam.ioBuffer = *prefsBuffer;
						myPB.ioParam.ioReqCount = sizeof(struct myprefs);
						myPB.ioParam.ioPosMode = fsFromStart;
						myPB.ioParam.ioPosOffset = 0L;
						(void)PBReadAsync(&myPB);
						while (myPB.ioParam.ioResult > 0) {
							SystemTask();
							(void)EventAvail(everyEvent, &dummyEv);
							}
						if ((err = myPB.ioParam.ioResult) == noErr)
							if (((struct myprefs *)*prefsBuffer)->prefsVersion == kCurPrefsVersion)
								gPrefs = *(struct myprefs *)*prefsBuffer;
						DisposeHandle(prefsBuffer);
						}
					(void)FSClose(prefsFRefNum);
					}
	}
HSetState(myStrHand, oldState);

return prefsFileAbsent;
}

/* GetFontNumber: gets font number from font name */

static Boolean	GetFontNumber(ConstStr255Param fontName, short *fontNum)
{
Str255	systemFontName;
register Boolean	retFlag;

GetFNum(fontName, fontNum);
if ((retFlag = (*fontNum != 0)) == false) {
	GetFontName(0, systemFontName);
	retFlag = EqualString(fontName, systemFontName, false, false);
	}
return retFlag;
}

/* MakeList: builds a list given a rectangle, the window, and
the initial number of lines */

static ListHandle MakeList(RectPtr theListRect, WindowPtr theWindow, short lines)
{
Rect	dataBounds;
Rect	tempRect;
Point	cSize = { 0, 0 };
register ListHandle	theList;

tempRect = *theListRect;
tempRect.right -= kScrollbarAdjust;
SetRect(&dataBounds, 0, 0, 1, lines);
theList = LNew(&tempRect, &dataBounds, cSize, 0, theWindow, true, false, false, true);
(*theList)->selFlags = lOnlyOne;
LSetSelect(true, cSize, theList);
LSetDrawingMode(true, theList);
return theList;
}

/* MakeTE: builds a TextEdit field, given the rectangle,
in the current window */

static TEHandle MakeTE(RectPtr Position)
{
Rect	tempRect;

tempRect = *Position;
FrameRect(Position);
InsetRect(&tempRect, 3, 3);
return(TENew(&tempRect, &tempRect));
}

/* InitIO: initializes the input/output window */

static OSErr InitIO(void)
{
enum {
rVScroll = 142,		/* vertical scrollbar control */
rHScroll			/* horizontal scrollbar control */
};

Rect	destRect, viewRect;
register WindowPtr	window;
register DocumentPeek doc;
register OSErr	err = appMemFullErr;

gWPtr_IO = window = GetNewWindow(kWIND_IO, NewPtrClear(sizeof(DocumentRecord)), (WindowPtr)-1L);
if (window) {
	SetPort(window);
	if (gPrefs.remembWind) {
		if (IsOnScreen(&gPrefs.IOUserState)) {
			(*(WStateDataHandle)((WindowPeek)window)->dataHandle)->userState = gPrefs.IOUserState;
			ZoomWindow(window, inZoomIn, false);
			}
		}
	doc = (DocumentPeek)window;
	GetTERect(window, &viewRect);
	destRect = viewRect;
	destRect.right = destRect.left +
						(qd.screenBits.bounds.right - qd.screenBits.bounds.left);
	if ( doc->docTE = TENew(&destRect, &viewRect) ) {
		(void)TEFeatureFlag(teFOutlineHilite, teBitSet, doc->docTE);
		TEAutoView(true, doc->docTE);
		if (doc->docVScroll = GetNewControl(rVScroll, window)) {
			if (doc->docHScroll = GetNewControl(rHScroll, window)) {
	/* adjust & draw the controls, draw the window */
				AdjustScrollbars(window, true);

				InitFabWindow((FabWindowPtr)window);
				SetActivate(window, DoActivateWindow);
				SetUpdate(window, DoUpdateWindow);
				SetGrow(window, DoGrowWindow);
				SetZoom(window, DoZoomWindow);
				SetGoAway(window, CloseIO);
				SetContent(window, DoContentClick);
				
				InstallRgnHandler((FabWindowPtr)window, NewRgn(), RecalcIO, gIBeamHandle,
						toMenu(kBalloons_Dump, kBIO_Contents), 0, 0);
				ResizeObjects((FabWindowPtr)window);
				RecalcGlobalCoords((FabWindowPtr)window);

//				InstallTrackingHandler(MyTrackingHandler, window, (void *) window);
//				InstallReceiveHandler(MyReceiveDropHandler, window, (void *) window);
				err = noErr;
				}
			}
		}
	}
return err;
} /* InitIO */

/* InitDump: initializes the Dump window */

static OSErr InitDump(void)
{
enum {
krdmpVScroll = 144,	/* vertical scrollbar control */
kCharsInOneLine = 0x002D0000
};

FMetricRec	theMetrics;
register Fixed	tempFix;
register WindowPtr	window;
register OSErr	err = appMemFullErr;

gWPtr_Dump = window = GetNewWindow(kWIND_Dump, NewPtrClear(sizeof(FabWindowRecord)), (WindowPtr)-1L);
if (window) {
	SetPort(window);
	if (gPrefs.remembWind)
		if (IsOnScreenWeak(gPrefs.DumpTopLeft))
			MoveWindow(window, gPrefs.DumpTopLeft.h, gPrefs.DumpTopLeft.v, false);
	if (dumpVScroll = GetNewControl(krdmpVScroll, window)) {
		TextFont(monaco);
		TextSize(9);
		TextMode(srcCopy);
		FontMetrics(&theMetrics);
		tempFix = theMetrics.descent + theMetrics.ascent + theMetrics.leading;
		tempFix = mySwap(tempFix);
		dumpLineHeight = (short)tempFix;
		dumpFromTop = *(short *)&theMetrics.ascent;
		dumpCWidMax = *(short *)&theMetrics.widMax;
		tempFix = FixMul(theMetrics.widMax, kCharsInOneLine);
		tempFix = mySwap(tempFix);
		SizeWindow(window, (short)tempFix + kScrollbarWidth + kDIST_FROMLEFT,
			gPrefs.remembWind ? gPrefs.DumpHeight :
				((PRCT_B(window) - PRCT_T(window)) / dumpLineHeight) * dumpLineHeight,
			false);
		SetupVertScrollBar(window, dumpVScroll);
		SetupDumpCtlMax(dumpVScroll);
		if (gPrefs.remembWind)
			SetControlValue(dumpVScroll, gPrefs.DumpScrollVal);

		InitFabWindow((FabWindowPtr)window);
		SetActivate(window, Activate_Dump);
		SetUpdate(window, Update_Dump);
		SetGrow(window, Grow_Dump);
		SetGoAway(window, CloseDump);
		SetContent(window, Do_Dump);
		SetGetDragRect(window, getDragRectDump);

		InstallRgnHandler((FabWindowPtr)window, NewRgn(), RecalcDump, nil,
				toMenu(kBalloons_Dump, kBDump_Contents), 0, 0);

		ResizeObjects((FabWindowPtr)window);
		RecalcGlobalCoords((FabWindowPtr)window);

		if (gDragManagerActive)
			(void) MyInstallHWindow(window);
		err = noErr;
		}
	}
return err;
} /* InitDump */

/* FatalErrorAlert: errors while initing windows */

static void FatalErrorAlert(short alertID, OSErr reason)
{
Str255	tempS;

MyNumToString(reason, tempS);
ParamText(tempS, nil, nil, nil);
(void)StopAlert_UPP(alertID, nil);
}

#endif

