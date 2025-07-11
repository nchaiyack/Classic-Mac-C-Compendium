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
#include	"ControlStore.h"
#include	"DoMenu.h"
#include	"Microprogram_Ed.h"
#include	"SimUtils.h"

#if defined(FabSystem7orlater)

#pragma segment Rare

const short controlStoreObject[] = {kP_CONTSTORE, 0 };

short	myFileRefN;
Boolean	DocIsOpen;

static FSSpec	workingFile;
static ScriptCode	wkFileScript;
static Boolean	dirtyCSFlag = false;

/*====== internal routines ======*/
static OSErr gotSave(Handle buf);
static OSErr gotSaveAs(Handle buf, OSType);
static OSErr SafeSave(Handle wrBuf, FSSpec *onWhichFile, ScriptCode);

/* RevertFile: we revert to the last saved version of the doc */

OSErr RevertFile(void)
{
FSSpec	tempNewFile = workingFile;

FSClose(myFileRefN);
myFileRefN = 0;
DocumentIsDirty(false);
return myOpenCSFile(&tempNewFile, wkFileScript, false);
}

/* myOpenCSFile: we read all that stuff (the microprogram) from the file */

OSErr myOpenCSFile(FSSpec *theFile, ScriptCode theScript, Boolean StationeryDoc)
{
Str255	tempS;
ParamBlockRec	myPB;
EventRecord	dummyEv;
Rect	tempRect;
GrafPtr	savePort;
register Handle	tempBuffer;
unsigned long	fileSize;
short	length;
register OSErr	err;
register Boolean	isLocked = false;

SetCursor(*gWatchHandle);
GetPort(&savePort);
SetPort(gWPtr_Microprogram_Ed);
if ((err = FSpOpenDFCompat(theFile, StationeryDoc ? fsRdPerm : fsRdWrPerm, &myFileRefN)) == permErr) {
	err = FSpOpenDFCompat(theFile, fsRdPerm, &myFileRefN);
	isLocked = true;
	}
if (err == noErr) {
	if ((err = GetEOF(myFileRefN, (long *)&fileSize)) == noErr) {
		if ((tempBuffer = NewHandleGeneral(fileSize)) != 0L) {
			myPB.ioParam.ioCompletion = 0L;
			myPB.ioParam.ioRefNum = myFileRefN;
			HLock(tempBuffer);
			myPB.ioParam.ioBuffer = *tempBuffer;
			myPB.ioParam.ioReqCount = fileSize;
			myPB.ioParam.ioPosMode = fsFromStart;
			myPB.ioParam.ioPosOffset = 0L;
			(void)PBReadAsync(&myPB);
			while (myPB.ioParam.ioResult > 0) {
				SystemTask();
				(void)EventAvail(everyEvent, &dummyEv);
				}
			HUnlock(tempBuffer);
			if ((err = myPB.ioParam.ioResult) == noErr) {
				Point	tempCell;
				register unsigned char *spanningPtr;
				register short	i;

				LSetDrawingMode(false, Lists[kL_COMMENTS]);
				DoNew();
				BlockMoveData(*tempBuffer, gCsMemory, kSIZE_CSMEM + kSIZE_ASSMEM);
				tempCell.h = 0;
				HLock(tempBuffer);
				for (spanningPtr = (unsigned char *)*tempBuffer + kSIZE_CSMEM + kSIZE_ASSMEM,
					i = 0; i <= maxLLine[kL_COMMENTS]; i++) {

					tempCell.v = i;
					LSetCell(spanningPtr + 1, *spanningPtr, tempCell, Lists[kL_COMMENTS]);
					spanningPtr += *spanningPtr + 1;
					}
				HUnlock(tempBuffer);
				LSetDrawingMode(true, Lists[kL_COMMENTS]);
				if (StationeryDoc == false) {
					workingFile = *theFile;
					wkFileScript = theScript;
					SetWTitle(gWPtr_Microprogram_Ed, theFile->name);
					}
				DocumentIsDirty(false);
			/* get cell contents and put into TextEdit field */
				length = 255;	/* maximum length of text */
				tempCell.h = 0;
				tempCell.v = theSelection[kL_COMMENTS];
				LGetCell(&tempS, &length, tempCell, Lists[kL_COMMENTS]);
				TESetText(&tempS, length, TEs[kKEY_COMMENT]);
				RefreshTE(kKEY_COMMENT);
		//		UnloadSeg(RefreshTE);
				SetMir((*(gCsMemory + theSelection[kL_COMMENTS])).cstore);
				tempRect = *keyrects[kKEY_LIST];
				tempRect.right -= kScrollbarAdjust;
				InvalRect(&tempRect);
				DoMenuWindows(kMItem_Microprogram);
				}
			DisposeHandle(tempBuffer);
			}
		else err = MemError();
		}
	if (err || StationeryDoc || isLocked)
		(void)FSClose(myFileRefN);
	}
if (err || StationeryDoc || isLocked)
	myFileRefN = 0;
SetPort(savePort);
InitCursor();
return err;
}

/* mySaveCSFile: the user has chosen Save or Save As� from the menus */

OSErr mySaveCSFile(short selector)
{
Point	tempCell;
register Handle	readyBuffer;
register OSErr	err;

SetCursor(*gWatchHandle);
tempCell.h = 0;
tempCell.v = theSelection[kL_COMMENTS];
ChangedListSelection(tempCell, kL_COMMENTS, false);
if (readyBuffer = PrepareBufferFromList()) {
	switch (selector) {
		case kGOT_SAVE: err = gotSave(readyBuffer);
			break;
		case kGOT_SAVEAS: err = gotSaveAs(readyBuffer, kFTY_CSTORE);
			break;
		case kGOT_SAVESTATIONERY: err = gotSaveAs(readyBuffer, kFTY_CSTOREPAD);
			break;
		}
	DisposeHandle(readyBuffer);
	}
else err = MemError();
InitCursor();
return err;
}

/* gotSave: the user has told us to Save. Is he working at a previously saved
document or not? */

static OSErr gotSave(Handle buf)
{
register OSErr	err;

if (myFileRefN != 0)
	err = SafeSave(buf, &workingFile, wkFileScript);
else
	err = gotSaveAs(buf, kFTY_CSTORE);
return err;
}

/* gotSaveAs: we must definitely show up the dialog box for "save-a-file"
and save it if asked for */

static OSErr gotSaveAs(Handle buf, OSType theType)
{
enum {
kSTR_CSPROMPT = 135,
kSTR_CSDEFNAME
};

StandardFileReply	mySFR;
FInfo	fndrInfo;
register Handle	sH1;
register Handle	sH2;
short	tmpFRefN;
register SignedByte	state1, state2;
register OSErr	err;

state1 = WantThisHandleSafe(sH1 = (Handle)GetString(kSTR_CSPROMPT));
state2 = WantThisHandleSafe(sH2 = (Handle)GetString(kSTR_CSDEFNAME));
InitCursor();
StandardPutFile((ConstStr255Param)*sH1, (ConstStr255Param)*sH2, &mySFR);
SetCursor(*gWatchHandle);
if (mySFR.sfGood) {
	if (mySFR.sfReplacing) {
		if (noErr == (err = FSpGetFInfoCompat(&mySFR.sfFile, &fndrInfo))) {
			if (fndrInfo.fdType == theType)
				err = SafeSave(buf, &mySFR.sfFile, mySFR.sfScript);
			else
				(void)StopAlert_UPP(kALRT_WRONGEXISTINGDOC, myStdFilterProcNoCancel);
			}
		}
/* not replacing an existing file */
	else if ((err = FSpCreateCompat(&mySFR.sfFile, kFCR_MINE, theType, mySFR.sfScript)) == noErr)
		if ((err = FSpOpenDFCompat(&mySFR.sfFile, fsRdWrPerm, &tmpFRefN)) == noErr)
			if ((err = WriteMicroprogramData(buf, &mySFR.sfFile, theType, tmpFRefN, mySFR.sfScript)) == noErr) {
				if (theType == kFTY_CSTORE) {
					if (myFileRefN)	/* close old working file if any */
						(void)FSClose(myFileRefN);
					myFileRefN = tmpFRefN;	/* this is the new work file */
					workingFile = mySFR.sfFile;
					wkFileScript = mySFR.sfScript;
					SetWTitle(gWPtr_Microprogram_Ed, mySFR.sfFile.name);
					DocumentIsDirty(false);
					}
				else if (theType == kFTY_CSTOREPAD) {
					(void) FSpSetIsStationery(&mySFR.sfFile);
					(void)FSClose(tmpFRefN);
					}
				}
			else {	/* error writing data */
				(void)FSClose(tmpFRefN);
				(void)FSpDeleteCompat(&mySFR.sfFile);
				}
	}
else err = 1;
HSetState(sH1, state1);
HSetState(sH2, state2);
return(err);
}

/* SafeSave: we should do a safe save of the document
(a previously saved version already exists) */

OSErr SafeSave(Handle buf, FSSpec *onWhichFile, ScriptCode myFScript)
{
Str255	tempFName;
FSSpec	thisFSSpec;
long	thisDirID;
short	thisVRefNum, newFileRefn;
register OSErr	err;

if (0)	/* test if file locked (FSpExchange does not) */
	err = fLckdErr;
else {
	MyNumToString(TickCount(), tempFName);
	if ((err = FindFolder(onWhichFile->vRefNum, kTemporaryFolderType, kCreateFolder,
					&thisVRefNum, &thisDirID)) == noErr) {
		(void)FSMakeFSSpecCompat(thisVRefNum, thisDirID, (ConstStr255Param)&tempFName, &thisFSSpec);
		if ((err = FSpCreateCompat(&thisFSSpec, kFCR_MINE, kFTY_CSTORE,
							myFScript)) == noErr)
			if ((err = FSpOpenDFCompat(&thisFSSpec, fsRdWrPerm, &newFileRefn)) == noErr)
				if ((err = WriteMicroprogramData(buf, &thisFSSpec, kFTY_CSTORE, newFileRefn, myFScript)) == noErr) {
					if ((err = FSpExchangeFilesCompat(&thisFSSpec, onWhichFile)) == noErr) {
						if (myFileRefN)
							(void)FSClose(myFileRefN);
						myFileRefN = newFileRefn;
						workingFile = *onWhichFile;
						wkFileScript = myFScript;
						SetWTitle(gWPtr_Microprogram_Ed, onWhichFile->name);
						DocumentIsDirty(false);
						err = FSpDeleteCompat(&thisFSSpec);
						}
					}
				else {
					(void)FSClose(newFileRefn);
					(void)FSpDeleteCompat(&thisFSSpec);	// the temporary one
					if (err == dskFulErr) {
						(void)StopAlert_UPP(kALRT_NOSAVE, myStdFilterProcNoCancel);
						err = 1;
						}
					}
		}
	}
return(err);
}

/* WriteMicroprogramData: writes physically the data into the file */

OSErr WriteMicroprogramData(Handle prepBuffer, FSSpec *theFSpec, OSType theType, short theOpenedFile,
						ScriptCode theFileSc)
{
ParamBlockRec	myPB;
EventRecord	dummyEv;
register OSErr	err;

myPB.ioParam.ioCompletion = 0L;
myPB.ioParam.ioBuffer = (Ptr)gCsMemory;
myPB.ioParam.ioReqCount = kSIZE_CSMEM + kSIZE_ASSMEM;
myPB.ioParam.ioPosMode = fsFromStart;
myPB.ioParam.ioRefNum = theOpenedFile;
myPB.ioParam.ioPosOffset = 0L;
(void)PBWriteAsync(&myPB);
while (myPB.ioParam.ioResult > 0) {
	SystemTask();
	(void)EventAvail(everyEvent, &dummyEv);
	}
if ((err = myPB.ioParam.ioResult) == noErr) {
	myPB.ioParam.ioBuffer = *prepBuffer;
	myPB.ioParam.ioReqCount = InlineGetHandleSize(prepBuffer);
	myPB.ioParam.ioPosMode = fsAtMark;
	myPB.ioParam.ioRefNum = theOpenedFile;
	myPB.ioParam.ioPosOffset = 0L;
	HLock(prepBuffer);
	(void)PBWriteAsync(&myPB);
	while (myPB.ioParam.ioResult > 0) {
		SystemTask();
		(void)EventAvail(everyEvent, &dummyEv);
		}
	HUnlock(prepBuffer);
	if ((err = myPB.ioParam.ioResult) == noErr)
		err = AddSTRRes2Doc(theFSpec, kFCR_MINE, theType, kSTR_ApplicationName, theFileSc);
	}
return err;
}

/* DoNew: starts afresh with a new microprogram */

void DoNew(void)
{
enum {
kSTR_UNTITLED = 137
};

register StringHandle	UntitledStr;
register SignedByte	oldstate;

ResetMicroprogramWindow();
DocIsOpen = true;
ActivateObjs(controlStoreObject);
if (UntitledStr = GetString(kSTR_UNTITLED)) {
	oldstate = WantThisHandleSafe((Handle)UntitledStr);
	SetWTitle(gWPtr_Microprogram_Ed, *UntitledStr);
	(void) PLstrcpy(workingFile.name, *UntitledStr);
	HSetState((Handle)UntitledStr, oldstate);
	}
}

/* ResetMicroprogramWindow: wipes out the window to an empty one */

void ResetMicroprogramWindow(void)
{
Str255	tempS;
register Point tempCell = { 0, 0};
short	leng;
register short	i;

for ( i = 0; i <= maxLLine[kL_COMMENTS]; i++) {
	tempCell.v = i;
	LClrCell(tempCell, Lists[kL_COMMENTS]);
	}
tempCell.v = 0;
leng = 255;
LGetCell(&tempS, &leng, tempCell, Lists[kL_COMMENTS]);
TESetText(&tempS, leng, TEs[kKEY_COMMENT]);
keyDownDest = kKEY_BRTO;

	{
	register union u_mir tempmir;

	tempmir.cstore = 0L;
	tempmir.bits.c = 15;
	tempmir.bits.dsc = 1;
	SetControlsFromMir(tempmir);
//	UnloadSeg(SetControlsFromMir);
	}
}

/* PrepareBufferFromList: sets up a buffer containing all the info
about the microprogram, so that it is ready to be saved into a file. */

Handle PrepareBufferFromList(void)
{
Str255	tempS;
Point	tempCell;
register Handle	myH = nil;
short	len;
register OSErr	err;
register short i;

if (Lists[kL_COMMENTS] && gCsMemory)
	if (myH = NewHandleGeneral(0L)) {
		tempCell.h = 0;
		for (i = 0; i <= maxLLine[kL_COMMENTS]; i++) {
			tempCell.v = i;
			len = 255;
			LGetCell(&tempS[1], &len, tempCell, Lists[kL_COMMENTS]);
			tempS[0] = len;
			if (err = PtrAndHand(&tempS, myH, len + 1))
				break;
			}
		if (err) {
			DisposeHandle(myH);
			myH = nil;
			}
		}
return myH;
}

#pragma segment Main

/* ReadyToTerminate: we decide if we are ready to quit gracefully;
shall we save an unsaved document? Let the user choose� */

Boolean ReadyToTerminate(void)
{
enum {
kPUSH_SAVE = 1,
kPUSH_CANCEL,
kPUSH_DONTSAVE
};

register OSErr	err;

if (gRstatus) {
	if (CautionAlert_UPP(kALRT_CPURUNNING, myStdFilterProc) == cancel)
		return false;
	}
if (dirtyCSFlag) {
	ParamText(workingFile.name, nil, nil, nil);
	switch (CautionAlert_UPP(kALRT_SAVE, myStdFilterProc)) {
		case kPUSH_SAVE:
			err = mySaveCSFile(kGOT_SAVE);
			UnloadSeg(mySaveCSFile);
			if (err) {
				if (err != 1)
					ErrorAlert(err);
				return false;
				}
			break;
		case kPUSH_CANCEL:
			return false;
			break;
		case kPUSH_DONTSAVE:
			break;
		}
	}
if (myFileRefN) {
	FSClose(myFileRefN);
	myFileRefN = 0;
	}
DocumentIsDirty(false);
DocIsOpen = false;
DeactivateObjs(controlStoreObject);
MyZeroBuffer((long *)gCsMemory, numOfLongs(kSIZE_ASSMEM + kSIZE_CSMEM));
CloseMicroProg(gWPtr_Microprogram_Ed);
SetMir(0L);
return true;
}

/* DocumentIsDirty: we are told if the document has been modified
since last save. */

void DocumentIsDirty(Boolean dirtiness)
{
if (dirtyCSFlag != dirtiness) {
	if (dirtyCSFlag = dirtiness) {
		EnableItem(gMenu_File, kMItem_Save_Control_St);
		}
	else {
		DisableItem(gMenu_File, kMItem_Save_Control_St);
		}
	if (myFileRefN && dirtyCSFlag)
		EnableItem(gMenu_File, kMItem_Revert_to_Saved);
	else
		DisableItem(gMenu_File, kMItem_Revert_to_Saved);
	}
}

#endif

