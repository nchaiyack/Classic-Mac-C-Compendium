/*
Copyright й 1993,1994,1995 Fabrizio Oddone
еее еее еее еее еее еее еее еее еее еее
This source code is distributed as freeware:
you may copy, exchange, modify this code.
You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"UtilsSys7.h"
#include	"Globals.h"
#include	"Animation.h"
#include	"Assembler.h"
#include	"ControlStore.h"
#include	"Disasm.h"
#include	"DoMenu.h"
#include	"Dump.h"
#include	"Input.h"
#include	"Main.h"
#include	"Microprogram_Ed.h"
#include	"myMemory.h"
#include	"Preferences.h"
#include	"Registers.h"
#include	"Simulator.h"
#include	"SimUtils.h"

#if defined(FabSystem7orlater)

//#pragma segment Main

short lastCommand = kST_STOPPED;

static void DoMenuApple(short theItem);
static void DoMenuEdit(short theItem);
static void DoMenuMemory(short theItem);
static void DoMenuRegs(short theItem);
static void DoMenuHDisasm(short theItem);
static void DoMenuHDump(short theItem);
static void DoMenuControl(short theItem);
static void CompletePreviousInstruction(void);
static void DoMenuAssembler(short theItem);

void DoMenuApple(short theItem)
{
Str255	DAName;

if (theItem == kMItem_About__Simulato) {
//	myAbout(DoIdle, nil, DoUpdate, DoActivate);
	myMovableModalAbout(gPrefs.remembWind ? &gPrefs.aboutTL : nil,
		gPrefs.remembWind ? &gPrefs.creditsTL : nil,
		AdjustMenus,
		Handle_My_Menu,
		DomyKeyEvent,
		DoUpdate,
		DoActivate,
		DoHighLevelEvent,
		DoOSEvent,
		DoIdle,
		gSleep
		);
	UnloadSeg(myAbout);
	}
else {
	AdjustMenus();
	GetMenuItemText(gMenu_Apple, theItem, DAName);
	(void)OpenDeskAcc(DAName);
	}
}

void DoMenuFile(short theItem)
{
register OSErr	err;

switch (theItem) {
	case kMItem_New:
		if (ReadyToTerminate()) {
			DoNew();
			UnloadSeg(DoNew);
			DoMenuWindows(kMItem_Microprogram);
			}
		break;
	case kMItem_Open_Control_St: {
		StandardFileReply	mySFR;
		SFTypeList	myTypeList;

		if (ReadyToTerminate()) {
			myTypeList[0] = kFTY_CSTORE;
			myTypeList[1] = kFTY_CSTOREPAD;
			StandardGetFile(0L, 2, myTypeList, &mySFR);
			if (mySFR.sfGood) {
				err = myOpenCSFile(&mySFR.sfFile, mySFR.sfScript,
										(mySFR.sfFlags & kfIsStationery) != 0);
				UnloadSeg(myOpenCSFile);
				if (err)
					ErrorAlert(err);
				}
			}
		}
		break;
	case kMItem_Close:
		(void)ReadyToTerminate();
		break;
	case kMItem_Save_Control_St:
		err = mySaveCSFile(kGOT_SAVE);
		UnloadSeg(mySaveCSFile);
		if (err)
			if (err != 1)
				ErrorAlert(err);
		break;
	case kMItem_Save_Control_St2:
		err = mySaveCSFile(kGOT_SAVEAS);
		UnloadSeg(mySaveCSFile);
		if (err)
			if (err != 1)
				ErrorAlert(err);
		break;
	case kMItem_Save_CS_Stationery:
		err = mySaveCSFile(kGOT_SAVESTATIONERY);
		UnloadSeg(mySaveCSFile);
		if (err)
			if (err != 1)
				ErrorAlert(err);
		break;
	case kMItem_Revert_to_Saved:
		if (CautionAlert_UPP(kALRT_REVERT, myStdFilterProc) == ok) {
			err = RevertFile();
			UnloadSeg(RevertFile);
			if (err)
				ErrorAlert(err);
			}
		break;
/*	case kMItem_Page_Setup:
		PageSetup();
		UnloadSeg(PageSetup);
		break;
	case kMItem_Print:
		Print_The_Data();
		UnloadSeg(Print_The_Data);
		break;*/
	case kMItem_Prefs:
		Preferences();
		UnloadSeg(Preferences);
		break;
	case kMItem_Quit:
		if (ReadyToTerminate())
			gDoneFlag = true;
		break;
	}
}

void DoMenuEdit(short theItem)
{
register Ptr	scPtr;
register union u_mir *scanPtr;
register union u_mir	tmpmir;
register OSErr	err;
register Byte	val;

if (SystemEdit(theItem - 1) == false) {
	switch (theItem) {
/* not implemented yet */
//		case kMItem_Undo:
//			break;
		case kMItem_Cut:
			if (gTheInput) {
				TECut(gTheInput);
				if (gWPtr_Microprogram_Ed == FrontWindow())
					Microprog_TextWasModified();
				else if (gWPtr_IO == FrontWindow())
					AdjustScrollbars(gWPtr_IO, false);
				if ((err = ZeroScrap()) == noErr)
					err = TEToScrap();
				if (err)
					ErrorAlert(err);
				}
			else if ((gWPtr_Microprogram_Ed == FrontWindow()) &&
						(keyDownDest == kKEY_LIST))
				if (err = SendClipMsg(kCLIPMSG_CUT))
					ErrorAlert(err);
			break;

		case kMItem_Copy:
			if (gTheInput) {
				TECopy(gTheInput);
				if ((err = ZeroScrap()) == noErr)
					err = TEToScrap();
				if (err)
					ErrorAlert(err);
				}
			else if ((gWPtr_Microprogram_Ed == FrontWindow()) &&
						(keyDownDest == kKEY_LIST))
				if (err = SendClipMsg(kCLIPMSG_COPY))
					ErrorAlert(err);
			break;

		case kMItem_Paste:
			if (gTheInput) {
				TEPaste(gTheInput);
				if (gWPtr_Microprogram_Ed == FrontWindow())
					Microprog_TextWasModified();
				else if (gWPtr_IO == FrontWindow())
					AdjustScrollbars(gWPtr_IO, false);
				}
			else if ((gWPtr_Microprogram_Ed == FrontWindow()) &&
						(keyDownDest == kKEY_LIST))
				if (err = SendClipMsg(kCLIPMSG_PASTE))
					ErrorAlert(err);
			break;

		case kMItem_Clear:
			if (gTheInput) {
				TEDelete(gTheInput);
				if (gWPtr_Microprogram_Ed == FrontWindow())
					Microprog_TextWasModified();
				else if (gWPtr_IO == FrontWindow())
					AdjustScrollbars(gWPtr_IO, false);
				}
			else if ((gWPtr_Microprogram_Ed == FrontWindow()) &&
						(keyDownDest == kKEY_LIST))
				if (err = SendClipMsg(kCLIPMSG_CLEAR))
					ErrorAlert(err);
			break;
		case kMItem_Select_All:
			if (gTheInput)
				TESetSelect(0, SHRT_MAX, gTheInput);
			break;
		case kMItem_Insert:
//			LSetDrawingMode(false, Lists[kL_COMMENTS]);
			BlockMoveData(gCsMemory + theSelection[kL_COMMENTS],
				gCsMemory + theSelection[kL_COMMENTS] + 1,
				sizeof(union u_mir)*(maxLLine[kL_COMMENTS] - theSelection[kL_COMMENTS]));
			gCsMemory[theSelection[kL_COMMENTS]].cstore = 0L;
			for (scanPtr = gCsMemory; scanPtr < (gCsMemory + maxLLine[kL_COMMENTS]);
					tmpmir = *scanPtr++)
				if (tmpmir.bits.addr >= theSelection[kL_COMMENTS])
					if (tmpmir.bits.addr != 0x7F) { /* WARNING: non-portable form */
						tmpmir.bits.addr++;
						*(scanPtr - 1) = tmpmir;
						}
			for (scPtr = gAssMemory; scPtr < (gAssMemory + kSIZE_ASSMEM); val = *scPtr++)
				if (val >= theSelection[kL_COMMENTS]) {
					val++;
					*(scPtr - 1) = val;
					}
			LDelRow(1, maxLLine[kL_COMMENTS], Lists[kL_COMMENTS]);
			LAddRow(1, theSelection[kL_COMMENTS]++, Lists[kL_COMMENTS]);
			DocumentIsDirty(true);
//			LSetDrawingMode(true, Lists[kL_COMMENTS]);
			LAutoScroll(Lists[kL_COMMENTS]);
			break;
		case kMItem_Delete:
//			LSetDrawingMode(false, Lists[kL_COMMENTS]);
			for (scanPtr = gCsMemory; scanPtr < (gCsMemory + maxLLine[kL_COMMENTS]);
					tmpmir = *scanPtr++)
				if (tmpmir.bits.addr >= theSelection[kL_COMMENTS])
					if (tmpmir.bits.addr) {
						tmpmir.bits.addr--;
						*(scanPtr - 1) = tmpmir;
						}
			for (scPtr = gAssMemory; scPtr < (gAssMemory + kSIZE_ASSMEM); val = *scPtr++)
				if (val > theSelection[kL_COMMENTS]) {
					val--;
					*(scPtr - 1) = val;
					}
			SelectLLine(kL_COMMENTS, theSelection[kL_COMMENTS] + 1);
			BlockMoveData(gCsMemory + theSelection[kL_COMMENTS],
				gCsMemory + theSelection[kL_COMMENTS] - 1,
				sizeof(union u_mir)*(maxLLine[kL_COMMENTS] - theSelection[kL_COMMENTS] + 1));
			gCsMemory[maxLLine[kL_COMMENTS]].cstore = 0L;
			LDelRow(1, --theSelection[kL_COMMENTS], Lists[kL_COMMENTS]);
			LAddRow(1, maxLLine[kL_COMMENTS], Lists[kL_COMMENTS]);
			DocumentIsDirty(true);
//			LSetDrawingMode(true, Lists[kL_COMMENTS]);
			LAutoScroll(Lists[kL_COMMENTS]);
			break;
		case kMItem_CopyEmailAddr:
			CopyEMailAddrToClip();
			break;
		case kMItem_CopyWWWURL:
			CopyWWWURLToClip();
			break;
		}
	}
}

void DoMenuWindows(short theItem)
{
register WindowPtr	wind;

switch (theItem) {
	case kMItem_Animation:
		wind = gWPtr_Animation;
		break;
	case kMItem_Registers:
		wind = gWPtr_Registers;
		break;
	case kMItem_IO:
		wind = gWPtr_IO;
		break;
	case kMItem_Microprogram:
		PrepareOpenMicroprogram();
		wind = gWPtr_Microprogram_Ed;
		break;
	case kMItem_Disasm:
		wind = gWPtr_Disasm;
		break;
	case kMItem_Dump:
		wind = gWPtr_Dump;
		break;
	}
ShowWindowAndCheckMenu(wind, theItem);
SelectWindow(wind);
}

void ShowWindowAndCheckMenu(WindowPtr w, short theMenuItem)
{
if (((WindowPeek)w)->visible == false) {
	CheckItem(gMenu_Windows, theMenuItem, true);
	SetPort(w);
	ShowWindow(w);
	}
}

void DoMenuMemory(short theItem)
{
register OSErr	err;

switch (theItem) {
	case kMItem_Open: {
		StandardFileReply	mySFR;
		SFTypeList	myTypeList;

		myTypeList[0] = kFTY_RAM;
		StandardGetFile(0L, 1, myTypeList, &mySFR);
		if (mySFR.sfGood) {
			if (err = myOpenFile(&mySFR.sfFile, gMMemory, kSIZE_RAM))
				ErrorAlert(err);
			UnloadSeg(myOpenFile);
			}
		}
		break;
	case kMItem_Save_As:
		if (err = mySaveFile(gMMemory, kSIZE_RAM, kFTY_RAM, kSTR_RAMPROMPT, kSTR_RAMDEFNAME))
			ErrorAlert(err);
		UnloadSeg(mySaveFile);
		break;
	}
}

void	DoMenuRegs(short theItem)
{
register OSErr	err;

switch (theItem) {
	case kMItem_Open: {
		StandardFileReply	mySFR;
		SFTypeList	myTypeList;

		myTypeList[0] = kFTY_REG;
		StandardGetFile(0L, 1, myTypeList, &mySFR);
		if (mySFR.sfGood) {
			if (err = OpenProcessorState(&mySFR.sfFile))
				ErrorAlert(err);
			UnloadSeg(OpenProcessorState);
			}
		}
		break;
	case kMItem_Save_As:
		if (err = SaveProcessorState())
			ErrorAlert(err);
		UnloadSeg(SaveProcessorState);
		break;
	}
}

static void	DoMenuHDisasm(short theItem)
{
SetControlValue(disasmVScroll, (unsigned short)gRegs[theItem - 1] >> 1);
InvalDisasm();
}

static void	DoMenuHDump(short theItem)
{
SetControlValue(dumpVScroll, (unsigned short)gRegs[theItem - 1] >> 3);
InvalDump();
}

/* da schiaffare dentro la routine successiva */
const short toDeact[] = {	kP_MPC2INC, kP_INCR, kP_MPC2CST, kP_CST2MIR,
							kP_REG2LTCH1, kP_REG2LTCH2, kP_ALATCH, kP_BLATCH,
							kP_AMUX2ALU, kP_BLTCH2ALU, kP_ALU, kP_ALU2SH,
							kP_SHIFTER, kP_SH2REGS1, kP_SH2REGS2, kP_SH2REGS3,
							kP_MAR, kP_BLTCH2MAR1, kP_BLTCH2MAR2,
							kP_MBR, kP_SH2MBR1, kP_SH2MBR2, kP_MAP, kP_MAPREGS,
							kP_ALTCH2AMUX, kP_AMUX, kP_MBR2AMUX,
							kP_INC2MMUX1, kP_INC2MMUX2, kP_MMUX, kP_MPC,
							kC_ADDR1, kC_ADDR2, kP_MMUX2MPC, 0 };

void DoMenuControl(short theItem)
{
short lastCommand = kST_STOPPED;
register OSErr	err;

switch (theItem) {
	case kMItem_Go:
		gRstatus = kST_GOING;
		CompletePreviousInstruction();
		lastCommand = kST_GOING;
		break;
	case kMItem_Stop:
		StopIt();
		lastCommand = kST_STOPPED;
		break;
	case kMItem_StepSub:
		gRstatus = kST_STEPSUBCYC;
		if (lastCommand != kST_STEPSUBCYC) {
			DeactivateObjs(toDeact);
			lastCommand = kST_STEPSUBCYC;
			}
		break;
	case kMItem_StepMicro:
		gRstatus = kST_STEPMPROG;
		lastCommand = kST_STEPMPROG;
		break;
	case kMItem_StepInstr:
		gRstatus = kST_STEPASM;
		CompletePreviousInstruction();
		lastCommand = kST_STEPASM;
		break;
	case kMItem_ResetMem:
		if ((err = CautionAlert_UPP(kALRT_RESETMEM, myStdFilterProc)) == ok) {
			MyZeroBuffer((long *)gMMemory, numOfLongs(kSIZE_RAM));
			ResetMemory();
			InvalDump();
			InvalDisasm();
			}
		break;
	case kMItem_ResetRegs:
		if ((err = CautionAlert_UPP(kALRT_RESETREGS, myStdFilterProc)) == ok) {
			ResetRegisters();
			UnloadSeg(ResetRegisters);
			ChangedRegister(kREG_PC);
			ChangedRegister(kREG_SP);
			ChangedBox(kP_MPC - kFIRST_PICT);
			}
		break;
	case kMItem_SelectCurMPC:
		SelectLLine(kL_COMMENTS, gParts[kP_MPC - kFIRST_PICT]);
		DoMenuWindows(kMItem_Microprogram);
		break;
	}
}

/* CompletePreviousInstruction: keeps in sync the processor when
alternating different step modes with continuous execution */

static void CompletePreviousInstruction(void)
{
while (gSubclk) {
	ExecuteInstructions(gSubclk++);
	gSubclk &= 3;
	}
}

void DoMenuAssembler(short theItem)
{
register OSErr	err;

switch (theItem) {
	case kMItem_Assemble_file: {
		StandardFileReply	mySFR;
		SFTypeList	myTypeList;

		myTypeList[0] = 'TEXT';
		StandardGetFile(0L, 1, myTypeList, &mySFR);
		if (mySFR.sfGood) {
			if (err = myAsmFile(&mySFR.sfFile))
				ErrorAlert(err);
			UnloadSeg(myAsmFile);
			}
		}
		break;
	case kMItem_AsmPrefs:
		DoAsmPrefsDialog();
		UnloadSeg(DoAsmPrefsDialog);
		break;
	}
}

void Handle_My_Menu(long myMenu)
{
register short	theMenu, theItem;

theMenu = HiWrd(myMenu);
if (theMenu) {
	theItem = LoWrd(myMenu);
	switch (theMenu) {
		case kRes_Menu_HMemory:
			DoMenuMemory(theItem);
			break;
		case kRes_Menu_HRegs:
			DoMenuRegs(theItem);
			break;
		case kRes_Menu_HDisasm:
			DoMenuHDisasm(theItem);
			break;
		case kRes_Menu_HDump:
			DoMenuHDump(theItem);
			break;
		case kRes_Menu_Apple:
			DoMenuApple(theItem);
			break;
		case kRes_Menu_File:
			DoMenuFile(theItem);
			break;
		case kRes_Menu_Edit:
			DoMenuEdit(theItem);
			break;
		case kRes_Menu_Windows:
			DoMenuWindows(theItem);
			break;
		case kRes_Menu_Control:
			DoMenuControl(theItem);
			break;
		case kRes_Menu_Assembler:
			DoMenuAssembler(theItem);
			break;
		}
	}
HiliteMenu(0);
}

#endif

