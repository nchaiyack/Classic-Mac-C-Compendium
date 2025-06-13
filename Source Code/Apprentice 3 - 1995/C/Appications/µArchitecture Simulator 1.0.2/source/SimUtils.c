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
#include	"ControlStore.h"
#include	"Dump.h"
#include	"Disasm.h"
#include	"Input.h"
#include	"Main.h"
#include	"Preferences.h"
#include	"Registers.h"
#include	"SimUtils.h"
#include	"Conversions.h"

#if defined(FabSystem7orlater)

#define	LOWSHORTADDR(l)	(((short *)(&l))+1)

#pragma segment Main


/* FromStringToNum: common code for translating a string with a specified
format to a number */

void FromStringToNum(ConstStr255Param sourceStr, long *destN, short whichFormat)
{

switch( whichFormat ) {
	case kPOP_DEC:	StringToNum(sourceStr, destN);
					break;
	case kPOP_HEX:	HexStringToShort(sourceStr, LOWSHORTADDR(*destN));
					break;
	case kPOP_OCT:	OctStringToShort(sourceStr, LOWSHORTADDR(*destN));
					break;
	case kPOP_BIN:	BinStringToShort(sourceStr, LOWSHORTADDR(*destN));
					break;
	case kPOP_TEXT:	*(char *)LOWSHORTADDR(*destN) = sourceStr[1];
					*((char *)LOWSHORTADDR(*destN) + 1) = sourceStr[2];
					break;
	}
}

/* FromNumToString: common code for translating a number into a
string with a specified format */

void FromNumToString(Str255 destStr, short sourceN, short whichFormat)
{

switch( whichFormat ) {
	case kPOP_DEC:	MyNumToString(sourceN, destStr);
					break;
	case kPOP_HEX:	ShortToHexString(sourceN, destStr);
					break;
	case kPOP_OCT:	ShortToOctString(sourceN, destStr);
					break;
	case kPOP_BIN:	ShortToBinString(sourceN, destStr);
					break;
	case kPOP_TEXT:	destStr[0] = 2;
					destStr[1] = *((char *)&sourceN);
					destStr[2] = *((char *)&sourceN + 1);
					break;
	}
}

/* MyGrowZone: the Grow Zone proc! */

pascal long MyGrowZone(Size /*qNeeded*/)
{
//#pragma unused	(qNeeded)

#ifndef __SCRIPT__
#define	smSystemScript	-1
#endif

ParamBlockRec	myPB;
Str63	tempFName;
EventRecord	dummyEv;
FSSpec	thisFSSpec;
short	state[kNUMOFSTATEPARTS];	/* the registers & editable parts of my machine */
register Handle	dangerousHandle = GZSaveHnd();
register Handle	readyBuffer;
register long	tempA5;
long	thisDirID;
short	thisVRefNum, tmpFRefN;
register OSErr	err;
register Boolean	somethingInTheTrash = false;

tempA5 = SetCurrentA5();
InitCursor();
if (gWPtr_Animation)
	if (offScr) {
		if (offScr->portBits.baseAddr)
			DisposePtr(offScr->portBits.baseAddr);
		if (offScr->visRgn && offScr->clipRgn)
			ClosePort(offScr);
		DisposePtr((Ptr)offScr);
		}
if (gWPtr_Animation &&
	gWPtr_Registers &&
	Ctrl_Base &&
	gWPtr_IO &&
	gWPtr_Microprogram_Ed &&
	gWPtr_Dump &&
	dumpVScroll &&
	gWPtr_Disasm &&
	disasmVScroll)
		SavePreferencesFile();
if (gWPtr_Disasm) {
	DisposeWindow(gWPtr_Disasm);
	gWPtr_Disasm = nil;
	}
if (gWPtr_Dump) {
	DisposeWindow(gWPtr_Dump);
	gWPtr_Dump = nil;
	}
if (gWPtr_IO) {
	if (((DocumentPeek)gWPtr_IO)->docTE != (TEHandle)dangerousHandle)
		if (((DocumentPeek)gWPtr_IO)->docTE) {
			TEDispose(((DocumentPeek)gWPtr_IO)->docTE);
			((DocumentPeek)gWPtr_IO)->docTE = nil;
			}
	DisposeWindow(gWPtr_IO);
	gWPtr_IO = nil;
	}
if (gWPtr_Registers) {
	DisposeWindow(gWPtr_Registers);
	gWPtr_Registers = nil;
	}
if (gWPtr_Animation) {
	DisposeWindow(gWPtr_Animation);
	gWPtr_Animation = nil;
	}
/* we should have freed some memory */
if (gMMemory) {
	if ((err = FindFolder(kOnSystemDisk, kTemporaryFolderType, kCreateFolder,
							&thisVRefNum, &thisDirID)) == noErr) {
		MyNumToString(TickCount(), tempFName);
		(void)FSMakeFSSpecCompat(thisVRefNum, thisDirID, tempFName, &thisFSSpec);
		if ((err = FSpCreateCompat(&thisFSSpec, kFCR_MINE, kFTY_RAM,
							smSystemScript)) == noErr)
			if ((err = FSpOpenDFCompat(&thisFSSpec, fsWrPerm, &tmpFRefN)) == noErr) {
				myPB.ioParam.ioCompletion = nil;
				myPB.ioParam.ioBuffer = gMMemory;
				myPB.ioParam.ioReqCount = kSIZE_RAM;
				myPB.ioParam.ioPosMode = fsFromStart;
				myPB.ioParam.ioRefNum = tmpFRefN;
				myPB.ioParam.ioPosOffset = 0L;
				(void)PBWriteAsync(&myPB);	// in temporary file
				while (myPB.ioParam.ioResult > 0) {
					SystemTask();
					(void)EventAvail(everyEvent, &dummyEv);
					}
				(void)FSClose(tmpFRefN);
				DisposePtr(gMMemory);
				gMMemory = nil;
				if ((err = myPB.ioParam.ioResult) == noErr) {
					somethingInTheTrash = true;
					err = AddSTRRes2Doc(&thisFSSpec, kFCR_MINE, kFTY_RAM,
										kSTR_ApplicationName, smSystemScript);
					}
				}
		MyNumToString(TickCount() + 1, tempFName);
		(void)FSMakeFSSpecCompat(thisVRefNum, thisDirID, tempFName, &thisFSSpec);
		if ((err = FSpCreateCompat(&thisFSSpec, kFCR_MINE, kFTY_REG,
							smSystemScript)) == noErr)
			if ((err = FSpOpenDFCompat(&thisFSSpec, fsWrPerm, &tmpFRefN)) == noErr) {
				BlockMoveData((Ptr)&gParts, (Ptr)&state, sizeof(gParts));
				BlockMoveData((Ptr)&gRegs, (Ptr)(state + kNUMOFPARTS), sizeof(state) - sizeof(gParts));
				myPB.ioParam.ioCompletion = nil;
				myPB.ioParam.ioBuffer = (Ptr)&state;
				myPB.ioParam.ioReqCount = sizeof(state);
				myPB.ioParam.ioPosMode = fsFromStart;
				myPB.ioParam.ioRefNum = tmpFRefN;
				myPB.ioParam.ioPosOffset = 0L;
				(void)PBWriteAsync(&myPB);	// in temporary file
				while (myPB.ioParam.ioResult > 0) {
					SystemTask();
					(void)EventAvail(everyEvent, &dummyEv);
					}
				(void)FSClose(tmpFRefN);
				if ((err = myPB.ioParam.ioResult) == noErr) {
					somethingInTheTrash = true;
					err = AddSTRRes2Doc(&thisFSSpec, kFCR_MINE, kFTY_REG,
										kSTR_ApplicationName, smSystemScript);
					}
				}
		readyBuffer = PrepareBufferFromList();
		UnloadSeg(PrepareBufferFromList);
		if (readyBuffer) {
			MyNumToString(TickCount() + 2, tempFName);
			(void)FSMakeFSSpecCompat(thisVRefNum, thisDirID, (ConstStr255Param)&tempFName, &thisFSSpec);
			if ((err = FSpCreateCompat(&thisFSSpec, kFCR_MINE, kFTY_CSTORE,
								smSystemScript)) == noErr)
				if ((err = FSpOpenDFCompat(&thisFSSpec, fsWrPerm, &tmpFRefN)) == noErr) {
					err = WriteMicroprogramData(readyBuffer, &thisFSSpec, kFTY_CSTORE, tmpFRefN,
												smSystemScript);
					UnloadSeg(WriteMicroprogramData);
					(void)FSClose(tmpFRefN);
					if (err == noErr)
						err = AddSTRRes2Doc(&thisFSSpec, kFCR_MINE, kFTY_CSTORE,
											kSTR_ApplicationName, smSystemScript);
					}
			}
		}
	}
if (gMMemory) {
	DisposePtr(gMMemory);
	gMMemory = nil;
	}
if (somethingInTheTrash)
	(void)StopAlert_UPP(kALRT_MEMTOOFULL, nil);
tempA5 = SetA5(tempA5);
ExitToShell();
}

/* SetupVertScrollBar: adjusts the vertical scrollbar to the right border
of the window */

void SetupVertScrollBar(WindowPtr wind, ControlHandle scrollB)
{
MoveControl(scrollB, PRCT_R(wind) - kScrollbarAdjust, PRCT_T(wind) - 1);
SizeControl(scrollB, kScrollbarWidth, PRCT_B(wind) - PRCT_T(wind) - 13);
}

/* SetupHorzScrollBar: adjusts the horizontal scrollbar to the bottom border
of the window */

void SetupHorzScrollBar(WindowPtr wind, ControlHandle scrollB)
{
MoveControl(scrollB, PRCT_L(wind) - 1, PRCT_B(wind) - kScrollbarAdjust);
SizeControl(scrollB, PRCT_R(wind) - PRCT_L(wind) - 13, kScrollbarWidth);
}

/* myStdFilterProc: standard filter proc for all alerts & simple dialogs
WITH a cancel button */

pascal Boolean myStdFilterProc(DialogPtr theD, EventRecord *thEv, short *iHit)
{
GrafPtr	savePort;
Boolean	retVal;

GetPort(&savePort);
SetPort(theD);
switch (thEv->what) {
	case updateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoUpdate(thEv);
			thEv->what = nullEvent;
			return false;
			}
		break;
	case activateEvt:
		if (thEv->modifiers & activeFlag) {
			(void)SetDialogDefaultItem(theD, ((DialogPeek)theD)->aDefItem);
			(void)SetDialogCancelItem(theD, cancel);
			(void)SetDialogTracksCursor(theD, false);
			}
		if (theD != (DialogPtr)thEv->message) {
			DoActivate(thEv);
			thEv->what = nullEvent;
			return false;
			}
		break;
	}
retVal = StdFilterProc(theD, thEv, iHit);
SetPort(savePort);
return retVal;
}

/* myStdFilterProcNoCancel: standard filter proc for all alerts & simple dialogs
WITHOUT a cancel button */

pascal Boolean myStdFilterProcNoCancel(DialogPtr theD, EventRecord *thEv, short *iHit)
{
GrafPtr	savePort;
register Boolean	retVal;

GetPort(&savePort);
SetPort(theD);
switch (thEv->what) {
	case updateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoUpdate(thEv);
			thEv->what = nullEvent;
			return false;
			}
		break;
	case activateEvt:
		if (thEv->modifiers & activeFlag) {
			(void)SetDialogDefaultItem(theD, ((DialogPeek)theD)->aDefItem);
			(void)SetDialogTracksCursor(theD, false);
			}
		if (theD != (DialogPtr)thEv->message) {
			DoActivate(thEv);
			thEv->what = nullEvent;
			return false;
			}
		break;
	}
retVal = StdFilterProc(theD, thEv, iHit);
SetPort(savePort);
return(retVal);
}

/* general error alert displaying error code */

void ErrorAlert(OSErr reason)
{
Str255	tempS;

MyNumToString(reason, tempS);
ParamText(tempS, nil, nil, nil);
(void)StopAlert_UPP(kALRT_GENERICERROR, myStdFilterProcNoCancel);
}

#pragma segment Rare

/* ResetRegisters: sets the main registers to their default values */

void ResetRegisters(void)
{
gRegs[kREG_PC] = gPrefs.DefPCValue;
gRegs[kREG_SP] = gPrefs.DefSPValue;
gParts[kP_MPC - kFIRST_PICT] = 0;
}

/* ResetMemory: sets some important "System" memory locations */

void ResetMemory(void)
{
register Handle IllHandler = Get1Resource(krHandlers, kIllegalHandlerCode);

((short *)gMMemory)[gPrefs.DefSPValue] = kPCExitToShell;
((short *)gMMemory)[kIllegalVector] = kIllegalHandler;
BlockMoveData(*IllHandler, (Ptr)((short *)gMMemory + kIllegalHandler),
			InlineGetHandleSize(IllHandler));
}

#pragma segment Main

#endif

