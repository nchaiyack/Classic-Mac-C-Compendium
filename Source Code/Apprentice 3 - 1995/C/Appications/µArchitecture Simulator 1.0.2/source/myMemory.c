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
#include	"Animation.h"
#include	"Disasm.h"
#include	"Dump.h"
#include	"myMemory.h"
#include	"Registers.h"
#include	"SimUtils.h"

#if defined(FabSystem7orlater)

#pragma segment Rare

/* myOpenFile: opens a dump file */

OSErr	myOpenFile(FSSpec *theFile, Ptr where, Size howmuch)
{
ParamBlockRec	myPB;
EventRecord	dummyEv;
short	myFileRefN;
register OSErr	err;

SetCursor(*gWatchHandle);
if ((err = FSpOpenDFCompat(theFile, fsRdPerm, &myFileRefN)) == noErr) {
	myPB.ioParam.ioCompletion = nil;
	myPB.ioParam.ioRefNum = myFileRefN;
	myPB.ioParam.ioBuffer = where;
	myPB.ioParam.ioReqCount = howmuch;
	myPB.ioParam.ioPosMode = fsFromStart;
	myPB.ioParam.ioPosOffset = 0L;
	(void)PBReadAsync(&myPB);
	while (myPB.ioParam.ioResult > 0) {
		SystemTask();
		(void)EventAvail(everyEvent, &dummyEv);
		}
	err = myPB.ioParam.ioResult;
	(void)FSClose(myFileRefN);
	if (where == (Ptr)&gRegs)
		ChangedAllRegisters();
	else {
		InvalDump();
		InvalDisasm();
		}
	}
SetCursor(&qd.arrow);
return(err);
}

/* mySaveFile: saves a dump file to disk */

OSErr mySaveFile(Ptr where, Size howmuch, OSType fType, short prompt, short defName)
{
StandardFileReply	mySFR;
Str255	tempFName;
ParamBlockRec	myPB;
EventRecord	dummyEv;
FSSpec	thisFSSpec;
long	thisDirID;
short	thisVRefNum, tmpFRefN;
register SignedByte	state1, state2;
register Handle	sH1 = (Handle)GetString(prompt);
register Handle	sH2 = (Handle)GetString(defName);
register OSErr	err;

err = noErr;
state1 = WantThisHandleSafe(sH1);
state2 = WantThisHandleSafe(sH2);
StandardPutFile((ConstStr255Param)*sH1, (ConstStr255Param)*sH2, &mySFR);
HSetState(sH2, state2);
HSetState(sH1, state1);
if (mySFR.sfGood) {
	SetCursor(*gWatchHandle);
	myPB.ioParam.ioCompletion = nil;
	myPB.ioParam.ioBuffer = where;
	myPB.ioParam.ioReqCount = howmuch;
	myPB.ioParam.ioPosMode = fsFromStart;
	if (mySFR.sfReplacing)
		if (0)	/* test if file locked (FSpExchange does not) */
			err = fLckdErr;
		else {
			MyNumToString(TickCount(), tempFName);
			if ((err = FindFolder(mySFR.sfFile.vRefNum, kTemporaryFolderType,
								kCreateFolder, &thisVRefNum, &thisDirID)) == noErr) {
				(void)FSMakeFSSpecCompat(thisVRefNum, thisDirID, (ConstStr255Param)&tempFName, &thisFSSpec);
				if ((err = FSpCreateCompat(&thisFSSpec, kFCR_MINE, fType,
									mySFR.sfScript)) == noErr)
					if ((err = FSpOpenDFCompat(&thisFSSpec, fsWrPerm, &tmpFRefN)) == noErr) {
						myPB.ioParam.ioRefNum = tmpFRefN;
						myPB.ioParam.ioPosOffset = 0L;
						(void)PBWriteAsync(&myPB);	// in temporary file
						while (myPB.ioParam.ioResult > 0) {
							SystemTask();
							(void)EventAvail(everyEvent, &dummyEv);
							}
						(void)FSClose(tmpFRefN);
						if ((err = myPB.ioParam.ioResult) == noErr) {
							if ((err = AddSTRRes2Doc(&thisFSSpec, kFCR_MINE,
													fType, kSTR_ApplicationName, mySFR.sfScript))
									== noErr)
								if ((err = FSpExchangeFilesCompat(&thisFSSpec, &(mySFR.sfFile)))
										== noErr)
									err = FSpDeleteCompat(&thisFSSpec);
							}
						else {
							(void)FSpDeleteCompat(&thisFSSpec);	// the temporary one
							if (err == dskFulErr) {
								if (CautionAlert_UPP(kALRT_NOSAFESAVE, myStdFilterProc) == ok) {
									if ((err = FSpOpenDFCompat(&(mySFR.sfFile), fsWrPerm,
														&tmpFRefN)) == noErr) {
										myPB.ioParam.ioRefNum = tmpFRefN;
										myPB.ioParam.ioPosOffset = 0L;
										(void)PBWriteAsync(&myPB);
										while (myPB.ioParam.ioResult > 0) {
											SystemTask();
											(void)EventAvail(everyEvent, &dummyEv);
											}
										(void)FSClose(tmpFRefN);
										err = myPB.ioParam.ioResult;
										}
									}
								else err = noErr; /* user clicked cancel in alert */
								}
							}
						}
				}
			}
/* not replacing an existing file */
	else if ((err = FSpCreateCompat(&(mySFR.sfFile), kFCR_MINE, fType, mySFR.sfScript)) == noErr)
		if ((err = FSpOpenDFCompat(&(mySFR.sfFile), fsWrPerm, &tmpFRefN)) == noErr) {
			myPB.ioParam.ioRefNum = tmpFRefN;
			myPB.ioParam.ioPosOffset = 0L;
			(void)PBWriteAsync(&myPB);
			while (myPB.ioParam.ioResult > 0) {
				SystemTask();
				(void)EventAvail(everyEvent, &dummyEv);
				}
			(void)FSClose(tmpFRefN);
			if ((err = myPB.ioParam.ioResult) == noErr)
				err = AddSTRRes2Doc(&(mySFR.sfFile), kFCR_MINE, fType,
									kSTR_ApplicationName, mySFR.sfScript);
			}
	SetCursor(&qd.arrow);
	}
return err;
}


OSErr	OpenProcessorState(FSSpecPtr theFile)
{
short	state[kNUMOFSTATEPARTS];	/* the registers & editable parts of my machine */
register OSErr	err;

if (noErr == (err = myOpenFile(theFile, (Ptr)&state, sizeof(state)))) {
	BlockMoveData((Ptr)&state, (Ptr)&gParts, sizeof(gParts));
	BlockMoveData((Ptr)(state + kNUMOFPARTS), (Ptr)&gRegs, sizeof(state) - sizeof(gParts));
	ChangedAllRegisters();
	ChangedAllBoxes();
	}
return err;
}

OSErr	SaveProcessorState(void)
{
short	state[kNUMOFSTATEPARTS];	/* the registers & editable parts of my machine */

BlockMoveData((Ptr)&gParts, (Ptr)&state, sizeof(gParts));
BlockMoveData((Ptr)&gRegs, (Ptr)(state + kNUMOFPARTS), sizeof(state) - sizeof(gParts));
return mySaveFile((Ptr)&state, sizeof(state), kFTY_REG, kSTR_REGSPROMPT, kSTR_REGSDEFNAME);
}

#endif

