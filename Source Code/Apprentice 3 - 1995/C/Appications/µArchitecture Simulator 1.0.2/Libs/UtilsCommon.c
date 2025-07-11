/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"

#include	"Independents.h"
#include	"MovableModal.h"

static void CommonCopyToClip(short id);

#if !(defined(powerc) || defined (__powerc)) || defined(FabSystem7orlater)

#if	!defined(FabNoSegmentDirectives)
#pragma segment Main
#endif

static AEEventHandlerUPP	gmyOAPP_UPP, gmyODOC_UPP, gmyPDOC_UPP, gmyQUIT_UPP;

#if defined(powerc) || defined (__powerc)

pascal StringPtr 	PLstrcpy(StringPtr str1, ConstStr255Param str2)
{
BlockMoveData(str2, str1, StrLength(str2) + 1);
return str1;
}

pascal StringPtr 	PLstrncpy(StringPtr str1, ConstStr255Param str2, short num)
{
BlockMoveData(str2, str1, 1 + MIN(StrLength(str2), num));
return str1;
}

pascal StringPtr	PLstrcat(StringPtr str1, ConstStr255Param str2)
{
BlockMoveData(str2 + 1, str1 + StrLength(str1) + 1, StrLength(str2));
StrLength(str1) += StrLength(str2);
return str1;
}

pascal StringPtr	PLstrncat(StringPtr str1, ConstStr255Param str2, short num)
{
BlockMoveData(str2 + 1, str1 + StrLength(str1) + 1, MIN(num, StrLength(str2)));
StrLength(str1) += StrLength(str2);
return str1;
}

void MyZeroBuffer(long *p, long s)
{
for ( ; s-- >= 0; *p++ = 0L);
}

void MyFillBuffer(long *p, long s, long filler)
{
for ( ; s-- >= 0; *p++ = filler);
}

long mySwap(long s)
{
long	t;

t = s >> 16;
*(short *)&t = (short) s;
return t;
}

#endif

/* AddSTRRes2Doc: adds the standard 'STR ' resource
to a newly saved document */

OSErr AddSTRRes2Doc(FSSpec *theDoc, OSType myFcrea, OSType myFtype, short STRid, ScriptCode lScript)
{
register Handle	msgString;
register short	fRefn;
register OSErr	err;

DetachResource(msgString = (Handle)GetString(STRid));
FSpCreateResFileCompat(theDoc, myFcrea, myFtype, lScript);
if ((err = ResError()) == noErr)
	if ((fRefn = FSpOpenResFileCompat(theDoc, fsRdWrPerm)) == -1)
		err = ResError();
	else {
		AddResource(msgString, 'STR ', STRid, "\p");
		if ((err = ResError()) == noErr) {
			SetResAttrs(msgString, GetResAttrs(msgString) | resPurgeable);
			err = ResError();
			}
		else
			DisposeHandle(msgString);
		CloseResFile(fRefn);
		}
return err;
}

/* AddSTRHand2Doc: adds the standard 'STR ' resource
to a newly saved document */

OSErr AddSTRHand2Doc(FSSpec *theDoc, OSType myFcrea, OSType myFtype, StringHandle msgString, ScriptCode lScript)
{
register short	fRefn;
register OSErr	err;

FSpCreateResFileCompat(theDoc, myFcrea, myFtype, lScript);
if ((err = ResError()) == noErr)
	if ((fRefn = FSpOpenResFileCompat(theDoc, fsRdWrPerm)) == -1)
		err = ResError();
	else {
		AddResource((Handle)msgString, 'STR ', kSTR_ApplicationName, "\p");
		if ((err = ResError()) == noErr) {
			SetResAttrs((Handle)msgString, GetResAttrs((Handle)msgString) | resPurgeable);
			err = ResError();
			}
		else
			DisposeHandle((Handle)msgString);
		CloseResFile(fRefn);
		}
return err;
}

OSErr AddRes2Doc(FSSpec *theDoc, Handle thisHandle, ResType rType, short rID)
{
register Handle	existHandle;
register short	fRefn;
register OSErr	err;

if ((fRefn = FSpOpenResFileCompat(theDoc, fsRdWrPerm)) == -1)
	err = ResError();
else {
//	if (noErr == (err = HandToHand(&thisHandle)))
	SetResLoad(false);
	existHandle = Get1Resource(rType, rID);
	SetResLoad(true);
	if (existHandle) {
		RemoveResource(existHandle);
		DisposeHandle(existHandle);
		}
	AddResource(thisHandle, rType, rID, "\p");
	if ((err = ResError()) == noErr) {
		SetResAttrs(thisHandle, GetResAttrs(thisHandle) | resPurgeable);
		err = ResError();
		}
	CloseResFile(fRefn);
	}
return err;
}

/* SendmyAE: we send an Apple Event with the specified ID to ourselves */

void SendmyAE(AEEventClass myclass, AEEventID myAEvtID, AEIdleProcPtr IdleFunct, AESendMode theMode)
{
AEIdleUPP	IdleFunctUPP = NewAEIdleProc(IdleFunct);
AppleEvent	myAEvent;
AppleEvent	myAEReply;
AEAddressDesc	targetAddress;
ProcessSerialNumber myPSN = { 0, kCurrentProcess };
register OSErr	err;

if ((err = AECreateDesc(typeProcessSerialNumber, (Ptr)&myPSN, sizeof(ProcessSerialNumber),
	&targetAddress)) == noErr) {
	if ((err = AECreateAppleEvent(myclass, myAEvtID, &targetAddress,
		kAutoGenerateReturnID, kAnyTransactionID, &myAEvent)) == noErr) {
		err = AESend(&myAEvent, &myAEReply, theMode,
			kAENormalPriority, kNoTimeOut, IdleFunctUPP, 0L);
		(void)AEDisposeDesc(&myAEvent);
		}
	(void)AEDisposeDesc(&targetAddress);
	}
if (IdleFunctUPP)
	DisposeRoutineDescriptor(IdleFunctUPP);
}

/* SendmyAEPShort: we send an Apple Event with the specified ID to ourselves */

void SendmyAEPShort(AEEventClass myclass, AEEventID myAEvtID, AEIdleProcPtr IdleFunct, AESendMode theMode, short theAlert)
{
AEIdleUPP	IdleFunctUPP = NewAEIdleProc(IdleFunct);
AppleEvent	myAEvent;
AppleEvent	myAEReply;
AEAddressDesc	targetAddress;
ProcessSerialNumber myPSN = { 0, kCurrentProcess };
register OSErr	err;

if ((err = AECreateDesc(typeProcessSerialNumber, (Ptr)&myPSN, sizeof(ProcessSerialNumber),
	&targetAddress)) == noErr) {
	if ((err = AECreateAppleEvent(myclass, myAEvtID, &targetAddress,
		kAutoGenerateReturnID, kAnyTransactionID, &myAEvent)) == noErr) {

		if ((err = AEPutParamPtr(&myAEvent, keyDirectObject, typeShortInteger, (Ptr)&theAlert, sizeof(theAlert))) == noErr)
			err = AESend(&myAEvent, &myAEReply, theMode,
				kAENormalPriority, kNoTimeOut, IdleFunctUPP, 0L);
		(void)AEDisposeDesc(&myAEvent);
		}
	(void)AEDisposeDesc(&targetAddress);
	}
if (IdleFunctUPP)
	DisposeRoutineDescriptor(IdleFunctUPP);
}

/* ZoomRectToRect: draws zooming gray rectangles from startingRect to endingRect
in the current port; numofDivs is the density of rectangles in the interval */

void ZoomRectToRect(RectPtr startingRect, RectPtr endingRect, short numofDivs)
{
PenState	curPen;
Rect	trailingRect = { 0, 0, 0, 0 };
Rect	tempRect;
Handle	rectStorH;
short	mtop, mleft, mbottom, mright;
register long	ttopleft, tbotright;
register short *spntr;
register long *lpntr;
register short	i, j, tsh;

if (numofDivs < (CHAR_BIT * sizeof(short))) {
	if (rectStorH = NewHandleGeneral(numofDivs * sizeof(Rect))) {
		GetPenState(&curPen);
		PenPat(&qd.gray);
		PenMode(patXor);
		mtop = endingRect->top - startingRect->top;
		mleft = endingRect->left - startingRect->left;
		mbottom = endingRect->bottom - startingRect->bottom;
		mright = endingRect->right - startingRect->right;
		spntr = (short *)*rectStorH;
		for (i = 1; i <= numofDivs; i++) {
			*spntr++ = mtop >>= 1;
			*spntr++ = mleft >>= 1;
			*spntr++ = mbottom >>= 1;
			*spntr++ = mright >>= 1;
			}
		HLock(rectStorH);
		for (i = 1; i < (1 << numofDivs); i++) {
			ttopleft = *(long *)startingRect;
			tbotright = ((long *)startingRect)[1];
			lpntr = (long *)*rectStorH;
			tsh = i << ((CHAR_BIT * sizeof(short)) - numofDivs);
			for (j = 1; j <= numofDivs; j++) {
				if (tsh < 0) {
					ttopleft += *lpntr++;
					tbotright += *lpntr++;
					}
				else
					lpntr += 2;
				tsh <<= 1;
				}
			
			*(long *)&tempRect = ttopleft;
			((long *)&tempRect)[1] = tbotright;
			FrameRect(&tempRect);
			FrameRect(&trailingRect);
			trailingRect = tempRect;
			}
		FrameRect(&tempRect);
		DisposeHandle(rectStorH);
		SetPenState(&curPen);
		}
	}
}

void ZoomRectToRectAutoGrafPort(RectPtr startingRect, RectPtr endingRect, short numofDivs)
{
GrafPtr		savePort;
GrafPort	ZoomPort;

GetPort(&savePort);
OpenPort(&ZoomPort);
ZoomRectToRect(startingRect, endingRect, numofDivs);
ClosePort(&ZoomPort);
SetPort(savePort);
}

/* NewHandleGeneral: makes a new handle in main or temporary memory, arguably
in the more suitable heap zone */

Handle NewHandleGeneral(Size blockSize)
{
register Handle	myH;
OSErr	err;

if (TempFreeMem() > FreeMem()) {
	if ((myH = TempNewHandle(blockSize, &err)) == nil) {
		ReserveMem(blockSize);
		myH = NewHandle(blockSize);
		}
	}
else {
	ReserveMem(blockSize);
	if ((myH = NewHandle(blockSize)) == nil)
		myH = TempNewHandle(blockSize, &err);
	}
return(myH);
}

/* Get1ResGeneral: like Get1Resource, but loads in main or temporary memory */

Handle Get1ResGeneral(ResType theType,short theID)
{
register Handle	emptyH, destH;
register Size	myResSize;

SetResLoad(false);
emptyH = Get1Resource(theType, theID);
SetResLoad(true);
destH = NewHandleGeneral(myResSize = GetResourceSizeOnDisk(emptyH));
if (destH) {
	HLock(destH);
	ReadPartialResource(emptyH, 0L, *destH, myResSize);
	HUnlock(destH);
	}
ReleaseResource(emptyH);
return(destH);
}

OSType FindFinderAtEaseProcess(ProcessSerialNumber *theProcess)
{
ProcessInfoRec infoRec;
ProcessSerialNumber process = {0L, kNoProcess};
register OSType	retVal = '????';

infoRec.processInfoLength = sizeof(ProcessInfoRec);
infoRec.processName = nil;
infoRec.processAppSpec = nil;

while (GetNextProcess(&process) == noErr) {
	if (noErr == (GetProcessInformation(&process, &infoRec))) {
		if ((infoRec.processType == (unsigned long)'FNDR') &&
			((infoRec.processSignature == 'MACS') || (infoRec.processSignature == 'mfdr'))) {
			
			retVal = infoRec.processSignature;
			*theProcess = process;
			break;
			}
		}
	}
return retVal;
}

OSType FindFinderProcess(void)
{
ProcessInfoRec infoRec;
ProcessSerialNumber process = {0, kNoProcess};
register OSType	retVal = '????';

infoRec.processInfoLength = sizeof(ProcessInfoRec);
infoRec.processName = nil;
infoRec.processAppSpec = nil;

while (GetNextProcess(&process) == noErr) {
	if (noErr == (GetProcessInformation(&process, &infoRec))) {
		if ((infoRec.processType == (unsigned long)'FNDR') &&
			(infoRec.processSignature == 'MACS')) {
			
			retVal = infoRec.processSignature;
			break;
			}
		}
	}
return retVal;
}

/* SendShutdownToFinder: we tell the Finder (or At Ease) to Shutdown (!) */

OSErr SendShutdownToFinder(AEIdleProcPtr myIdleFunct, Boolean wantShutDown)
{
AEIdleUPP	myIdleFunctUPP = NewAEIdleProc(myIdleFunct);
AppleEvent	myAEvent;
AppleEvent	myAEReply;
ProcessSerialNumber	process;
AEAddressDesc	targetAddress;
OSType	WantedCreator;
register OSErr	err = errAEDescNotFound;

WantedCreator = FindFinderAtEaseProcess(&process);
if ('????' != WantedCreator)
	if ((err = AECreateDesc(typeApplSignature, (Ptr)&WantedCreator, sizeof(OSType),
		&targetAddress)) == noErr) {
		if ((err = AECreateAppleEvent(kAEFinderEvents, wantShutDown ? kAEShutDown : kAERestart , &targetAddress,
			kAutoGenerateReturnID, kAnyTransactionID, &myAEvent)) == noErr) {
			err = AESend(&myAEvent, &myAEReply, kAENoReply | kAEAlwaysInteract,
				kAENormalPriority, kNoTimeOut, myIdleFunctUPP, nil);
			(void)AEDisposeDesc(&myAEvent);
			if (err == noErr)
				(void)SetFrontProcess(&process);
			}
		(void)AEDisposeDesc(&targetAddress);
		}
if (myIdleFunctUPP)
	DisposeRoutineDescriptor(myIdleFunctUPP);
return err;
}

short StopAlert_UPP(short alertID, ModalFilterProcPtr filterProc)
{
ModalFilterUPP	filterProcUPP = nil;
register short	butHit;

if (filterProc)
	filterProcUPP = NewModalFilterProc(filterProc);
InitCursor();
butHit = StopAlert(alertID, filterProcUPP);
if (filterProcUPP)
	DisposeRoutineDescriptor(filterProcUPP);
return butHit;
}

short StopAlert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct)
{

SendmyAE(kCreat, kAEAlert, IdleFunct, kAENoReply | kAEAlwaysInteract);
return StopAlert_UPP(alertID, filterProc);
}

short CautionAlert_UPP(short alertID, ModalFilterProcPtr filterProc)
{
ModalFilterUPP	filterProcUPP = nil;
register short	butHit;

if (filterProc)
	filterProcUPP = NewModalFilterProc(filterProc);
InitCursor();
butHit = CautionAlert(alertID, filterProcUPP);
if (filterProcUPP)
	DisposeRoutineDescriptor(filterProcUPP);
return butHit;
}

short CautionAlert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct)
{

SendmyAE(kCreat, kAEAlert, IdleFunct, kAENoReply | kAEAlwaysInteract);
return CautionAlert_UPP(alertID, filterProc);
}

short NoteAlert_UPP(short alertID, ModalFilterProcPtr filterProc)
{
ModalFilterUPP	filterProcUPP = nil;
register short	butHit;

if (filterProc)
	filterProcUPP = NewModalFilterProc(filterProc);
InitCursor();
butHit = NoteAlert(alertID, filterProcUPP);
if (filterProcUPP)
	DisposeRoutineDescriptor(filterProcUPP);
return butHit;
}

short NoteAlert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct)
{

SendmyAE(kCreat, kAEAlert, IdleFunct, kAENoReply | kAEAlwaysInteract);
return NoteAlert_UPP(alertID, filterProc);
}

short Alert_UPP(short alertID, ModalFilterProcPtr filterProc)
{
ModalFilterUPP	filterProcUPP = nil;
register short	butHit;

if (filterProc)
	filterProcUPP = NewModalFilterProc(filterProc);
InitCursor();
butHit = Alert(alertID, filterProcUPP);
if (filterProcUPP)
	DisposeRoutineDescriptor(filterProcUPP);
return butHit;
}

short Alert_AE(short alertID, ModalFilterProcPtr filterProc, AEIdleProcPtr IdleFunct)
{

SendmyAE(kCreat, kAEAlert, IdleFunct, kAENoReply | kAEAlwaysInteract);
return Alert_UPP(alertID, filterProc);
}

/* my Dialog box manager */

void FlashButton(DialogPtr dlg, short item)
{
Rect	txtBox;
ControlHandle	itH;
long	dummy;
short	typ;

GetDialogItem(dlg, item, &typ, (Handle *)&itH, &txtBox);
HiliteControl(itH, 1);
Delay(4L, &dummy);
HiliteControl(itH, 0);
}

static void FrameRoundBW(const RectPtr theRect, ConstPatternParam bpat, short ovalP);

enum {
kIsColorPort = 0xC000,
kButtonFrameSize = 3,	/* button frame's pen size */
kButtonFrameInset = -4	/* inset rectangle adjustment around button */
};

void OutlineButton(DialogPtr oftheDialog, short myItem)
{
RGBColor	foreCol, backCol, savedForeCol;
PenState	curPen;
PatPtr	thePat;
GrafPtr	savePort;
Rect	txtBox;
Handle	itH;
short	typ;

GetDialogItem(oftheDialog, myItem, &typ, &itH, &txtBox);
GetPort(&savePort);
SetPort(oftheDialog);

InsetRect(&txtBox, kButtonFrameInset, kButtonFrameInset);
typ = ((txtBox.bottom - txtBox.top) >> 1) + 2;

thePat = (*(ControlHandle)itH)->contrlHilite == 0 ? &qd.black : &qd.gray;

GetPenState(&curPen);
PenNormal();
PenSize(kButtonFrameSize, kButtonFrameSize);
if ((((CGrafPtr)(*(ControlHandle)itH)->contrlOwner)->portVersion & kIsColorPort) == kIsColorPort) {
	if ((*(ControlHandle)itH)->contrlHilite) {
		GetForeColor(&foreCol);
		savedForeCol = foreCol;
		GetBackColor(&backCol);
		if (GetGray(GetGDevice(), &backCol, &foreCol)) {
			RGBForeColor(&foreCol);
			FrameRoundRect(&txtBox, typ, typ);
			RGBForeColor(&savedForeCol);
			}
		else
			FrameRoundBW(&txtBox, thePat, typ);
		}
	else
		FrameRoundRect(&txtBox, typ, typ);
	}
else {
	FrameRoundBW(&txtBox, thePat, typ);
	}

SetPenState(&curPen);
SetPort(savePort);
}

void FrameRoundBW(const RectPtr theRect, ConstPatternParam bpat, short ovalP)
{
PenPat(bpat);
FrameRoundRect(theRect, ovalP, ovalP);
}

short HandleDialog(ModalFilterProcPtr filterProc,
					dialogItemsPtr things,
					void (*initProc)(DialogPtr),
					void (*userProc)(DialogPtr, Handle, short),
					short resId)
{
ModalFilterUPP	filterProcUPP = NewModalFilterProc(filterProc);
Rect	box;
Handle	item;
GrafPtr	port;
register long	myRefCon;
register DialogPtr	dPtr;
register dialogItemsPtr	spanPtr;
dialogItemsPtr	itemToBeActivated;
short	type, theItemHit = memFullErr;
short	theGroup, lastItemClosingDialog = cancel;
register short	theType, iNum;
register Boolean	editTextExists = false;
Boolean	dialoging;
#if !defined(FabSystem7orlater)
long	Gresp;
Boolean	dialogManager7Present = false;
#endif

InitCursor();
dPtr = GetNewDialog(resId, nil, (DialogPtr)-1L);
if (dPtr) {
	GetPort(&port);
	SetPort(dPtr);

#if !defined(FabSystem7orlater)
	if (Gestalt(gestaltDITLExtAttr, &Gresp) == noErr)
		if (Gresp & (1L << gestaltDITLExtPresent))
			dialogManager7Present = true;
#endif

#pragma mark Setup
	
	spanPtr = things;
	while (iNum = spanPtr->itemNumber) {
		myRefCon = spanPtr->refCon;
		GetDialogItem(dPtr, iNum, &type, &item, &box);
		theType = type & itemDisable ? type - itemDisable : type;
		switch(theType) {
			case ctrlItem+btnCtrl:
#if !defined(FabSystem7orlater)
				if (dialogManager7Present)
#endif
					{
					if (myRefCon == 1L)
						(void) SetDialogDefaultItem(dPtr, iNum);
					else if (myRefCon == 2L)
						lastItemClosingDialog = iNum;
					if (iNum == cancel)
						(void) SetDialogCancelItem(dPtr, iNum);
					}
				break;
			case ctrlItem+chkCtrl:
			case ctrlItem+radCtrl:
				if (myRefCon > 0) {
					SetControlValue((ControlHandle)item, 1);
					if (userProc)
						userProc(dPtr, item, iNum);
					}
/*				else if (myRefCon == 0) {
					if (userProc)
						userProc(dPtr, item, iNum);
					}*/
				else if (myRefCon < 0)
					HiliteControl((ControlHandle)item, 255);
				break;
			case ctrlItem+resCtrl:
				if (myRefCon)
					SetControlValue((ControlHandle)item, myRefCon);
				break;
			case statText:
				if (myRefCon)
					SetDialogItemText(item, (StringPtr)myRefCon);
				break;
			case editText:
				SetDialogItemText(item, (StringPtr)myRefCon);
				if(editTextExists == false) {
					SelectDialogItemText(dPtr, iNum, 0, SHRT_MAX);
					editTextExists = true;
					}
				break;
	//		case iconItem:
	//			break;
	//		case picItem:
	//			break;
			case userItem:
				SetDialogItem(dPtr, iNum, type, (Handle)myRefCon, &box);
				break;
			}
		spanPtr++;
		}
#if !defined(FabSystem7orlater)
	if (dialogManager7Present)
#endif
		(void) SetDialogTracksCursor(dPtr, editTextExists);
	if (initProc)
		initProc(dPtr);
	ShowWindow(dPtr);
	
#pragma mark Event Loop
	
	dialoging = true;
	do {
		ModalDialog(filterProcUPP, &theItemHit);
		if (theItemHit) {
			GetDialogItem(dPtr, theItemHit, &type, &item, &box);
			switch (type) {
				case ctrlItem+btnCtrl:
					if ((theItemHit >= ok) && (theItemHit <= lastItemClosingDialog))
						dialoging = false;
					else if (userProc)
						userProc(dPtr, item, theItemHit);
					break;
				case ctrlItem+chkCtrl:
					SetControlValue((ControlHandle)item, 1 - GetControlValue((ControlHandle)item));
					if (userProc)
						userProc(dPtr, item, theItemHit);
					break;
				case ctrlItem+radCtrl:
					if (GetControlValue((ControlHandle)item) == 0) {
						SetControlValue((ControlHandle)item, 1);
						for (spanPtr = things; (++spanPtr)->itemNumber != theItemHit; )
							;
						itemToBeActivated = spanPtr;
						for (theGroup = spanPtr->group; (--spanPtr)->group == theGroup; )
							;
						while ((++spanPtr)->refCon <= 0L)
							;
						GetDialogItem(dPtr, spanPtr->itemNumber, &type, &item, &box);
						SetControlValue((ControlHandle)item, 0);
						spanPtr->refCon = 0L;
						itemToBeActivated->refCon = 1L;
						if (userProc)
							userProc(dPtr, item, theItemHit);
						}
					break;
				case ctrlItem+resCtrl:
				case editText:
					if (userProc)
						userProc(dPtr, item, theItemHit);
					break;
				}
			}
		}
	while (dialoging);
	
#pragma mark Return Settings
	
	if (theItemHit == ok) {
		spanPtr = things;
		while (iNum = spanPtr->itemNumber) {
			GetDialogItem(dPtr, iNum, &type, &item, &box);
			theType = type & itemDisable ? type - itemDisable : type;
			switch(theType) {
				case ctrlItem+chkCtrl:
				case ctrlItem+resCtrl:
					spanPtr->refCon = GetControlValue((ControlHandle)item);
					break;
				case editText:
					GetDialogItemText(item, (StringPtr)spanPtr->refCon);
					break;
		//		case iconItem:
		//			break;
		//		case picItem:
		//			break;
		//		case userItem:
		//			break;
				}
			spanPtr++;
			}
		}
	DisposeDialog(dPtr);
	SetPort(port);
	}
else
	SysBeep(30);
if (filterProcUPP)
	DisposeRoutineDescriptor(filterProcUPP);
InitCursor();
return theItemHit;
}

/* About box manager */

#if	!defined(FabNoSegmentDirectives)
#pragma segment About
#endif

/* myAbout: the events directed against the about box are checked with
EventAvail, so that we can discard the about box and process
important events later in the main event loop */

void myAbout(void (*theIdleProc)(void),
			UserItemUPP DrawQTPict,
			void (*UpdateProc)(EventRecord *),
			void (*ActivateProc)(EventRecord *)
			)
{
EventRecord	evrec;
DialogPtr	agh, dPtr;
short	iHit;
register Handle	splash;
register Boolean	exit = false;

splash = OpenSplash(&agh, DrawQTPict);
if (splash) {
	ShowWindow(agh);
	do {
		SystemTask();
		if (EventAvail(everyEvent, &evrec)) {
			switch(evrec.what) {
				case mouseDown:
				case keyDown:
				case autoKey:
					exit = true;
					FlushEvents(mDownMask | mUpMask | keyDownMask | autoKeyMask, 0);
					break;
				case mouseUp:
					(void)GetNextEvent(mUpMask, &evrec);
					break;
				case updateEvt :
					(void)GetNextEvent(updateMask, &evrec);
					if ((DialogPtr)evrec.message == agh) {
						BeginUpdate(agh);
						UpdateDialog(agh, agh->visRgn);
						EndUpdate(agh);
						}
					else
						if (IsDialogEvent(&evrec))
							(void) DialogSelect(&evrec, &dPtr, &iHit);
						else
							UpdateProc(&evrec);
					break;
				case diskEvt :
				case osEvt :
				case kHighLevelEvent:
					exit = true;
					break;
				case activateEvt :
					(void)GetNextEvent(activMask, &evrec);
					if ((DialogPtr)evrec.message != agh) {
						if (IsDialogEvent(&evrec))
							(void) DialogSelect(&evrec, &dPtr, &iHit);
						else
							ActivateProc(&evrec);
						}
					break;
				case driverEvt :
					(void)GetNextEvent(driverMask, &evrec);
					break;
				}
			}
		else {
			theIdleProc();
			}
		}
	while (exit == false);
	DisposeSplash(splash, agh);
	}
else
	SysBeep(30);
}

/* OpenSplash: puts up the about window */

Handle OpenSplash(DialogPtr *pass, UserItemUPP DrawQTPict)
{
enum {
kDLOG_About = 256,
kItemPICT = 1
};

Str15	tempS = "\p";
Rect	box;
register DialogPtr	dPtr;
register VersRecHndl	myvers;
register Handle	hand;
Handle	item;
short	type;

hand = NewHandle(sizeof(DialogRecord));
if (hand) {
	HLockHi(hand);
	myvers = (VersRecHndl)Get1Resource('vers', 1);
	if (myvers) {
		(void) PLstrncpy(tempS, (*myvers)->shortVersion, 15);
		ReleaseResource((Handle)myvers);
		}
	else
		tempS[0] = 0;
	ParamText((ConstStr255Param)&tempS, nil, nil, nil);
	*pass = dPtr = GetNewDialog(kDLOG_About, *hand, (DialogPtr)-1L);
	if (dPtr) {
		if (DrawQTPict) {
			GetDialogItem(dPtr, kItemPICT, &type, &item, &box);
			SetDialogItem(dPtr, kItemPICT, type, (Handle)DrawQTPict, &box);
			}
		}
	else {
		DisposeHandle(hand);
		hand = nil;
		}
	}
return(hand);
}

/* DisposeSplash: gets rid of the about window */

void DisposeSplash(Handle dialog, DialogPtr pass)
{
CloseDialog(pass);
DisposeHandle(dialog);
}

void myMovableModalAbout(
			Point *aboutTopLeft,
			Point *creditsTopLeft,
			void (*AdjustMenus)(void),
			void (*Handle_My_Menu)(long),
			void (*DomyKeyEvent)(EventRecord *),
			void (*DoUpdate)(EventRecord *),
			void (*DoActivate)(EventRecord *),
			void (*DoHiLevEvent)(EventRecord *),
			void (*DoOSEvent)(EventRecord *),
			void (*DoIdle)(void),
			unsigned long minSleep
			)
{
Str15	tempS = "\p";
dialogItems	things[] = {{ ok, 0, 1L },
						{ cancel, 0, 0L },
						{ 0, 0, 0L}
						};
dialogItems	credits[] = {{ ok, 0, 1L },
						{ 0, 0, 0L}
						};
register VersRecHndl	myvers;

myvers = (VersRecHndl)Get1Resource('vers', 1);
if (myvers) {
	(void) PLstrncpy(tempS, (*myvers)->shortVersion, 15);
	ReleaseResource((Handle)myvers);
	}
else
	tempS[0] = 0;
ParamText((ConstStr255Param)&tempS, nil, nil, nil);
switch (HandleMovableModalDialog((dialogItemsPtr)&things, aboutTopLeft, nil, nil, nil, nil, nil,
			AdjustMenus,
			Handle_My_Menu,
			DomyKeyEvent,
			nil,
			nil,
			DoUpdate,
			DoActivate,
			DoHiLevEvent,
			DoOSEvent,
			DoIdle,
			minSleep,
			kDLOG_MovableAbout)) {
	case ok:
		break;
	case cancel:
		(void) HandleMovableModalDialog((dialogItemsPtr)&credits, creditsTopLeft, nil, nil, nil, nil, nil,
			AdjustMenus,
			Handle_My_Menu,
			DomyKeyEvent,
			nil,
			nil,
			DoUpdate,
			DoActivate,
			DoHiLevEvent,
			DoOSEvent,
			DoIdle,
			minSleep,
			kDLOG_MovableCredits);
		break;
	}
}

#if	!defined(FabNoSegmentDirectives)
#pragma segment Main
#endif

#if defined(powerc) || defined (__powerc)
SignedByte WantThisHandleSafe(Handle myH)
{
SignedByte	cMemTags;

cMemTags = HGetState(myH);
HLockHi(myH);
return cMemTags;
}

#endif


void CopyEMailAddrToClip(void)
{
CommonCopyToClip(kSTR_EMAILADDR);
}

void CopyWWWURLToClip(void)
{
CommonCopyToClip(kSTR_WWWURL);
}

void CommonCopyToClip(short id)
{
StringHandle	theStr;

if (theStr = GetString(id)) {
	HLockHi((Handle)theStr);
	if (noErr == ZeroScrap())
		(void) PutScrap(**theStr, 'TEXT', *theStr + 1);
	HUnlock((Handle)theStr);
	(void)TEFromScrap();
	}
}

/* GetFontNumber: gets font number from font name */

Boolean GetFontNumber(ConstStr255Param fontName, short *fontNum)
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

/* this is taken from "Macintosh Programming Secrets"
by Scott Knaster & Keith Rollin;
optimization by Fabrizio Oddone */

Boolean CmdPeriod(EventRecord *theEvent)
{
#define	kModifiersMask	(0xFF00 & ~cmdKey)

Handle	hKCHR;
long	/*virtualKey, */keyInfo, state, keyCID;
short	keyCode;
Boolean	result = false;

if ((theEvent->what == keyDown) || (theEvent->what == autoKey)) {
	if (theEvent->modifiers & cmdKey) {
//		virtualKey = (theEvent->message >> 8) >> 8;
		keyCode = (theEvent->modifiers & kModifiersMask) | ((unsigned short)theEvent->message >> 8);
		state = 0;
		keyCID = GetScript(GetScriptManagerVariable(smKeyScript), smScriptKeys);
		hKCHR = GetResource('KCHR', keyCID);
		if (hKCHR) {
			keyInfo = KeyTranslate(*hKCHR, keyCode, &state);
			ReleaseResource(hKCHR);
			}
		else
			keyInfo = theEvent->message;
		
		if (((char)keyInfo == '.') || ((char)(keyInfo >> 16) == '.'))
			result = true;
		}
	}

return result;
}

/* other useful stuff to detect keypresses */

Boolean CmdPeriodOrEsc(EventRecord *theEvent)
{
return ((unsigned short)theEvent->message == kEscapeKey) || CmdPeriod(theEvent);
}

Boolean CmdPeriodOrEscConfirm(EventRecord *theEvent, short alertID, ModalFilterProcPtr filterProc)
{
register Boolean	retflag;

retflag = CmdPeriodOrEsc(theEvent);
if (retflag) {
	ModalFilterUPP	filterProcUPP = NewModalFilterProc(filterProc);
	
	InitCursor();
	retflag = CautionAlert(alertID, filterProcUPP) == cancel;
	if (filterProcUPP)
		DisposeRoutineDescriptor(filterProcUPP);
	}

return retflag;
}

#if	!defined(FabNoSegmentDirectives)
#pragma segment Registration
#endif

short gHowManyDollars;

/* RegistrationFilter: the filter filters entered characters, of course */

static void RegInitProc(DialogPtr myDPtr);
static Boolean RegistrationPreProcessKey(EventRecord *thEv, DialogPtr theD);
static void RegistrationPostProcessKey(EventRecord *thEv, DialogPtr theD);

Boolean RegistrationPreProcessKey(EventRecord *thEv, DialogPtr theD)
{
short	iHit;
unsigned char	keypressed;
Boolean	result = true;

keypressed = CHARFROMMESSAGE(thEv->message);
iHit = ((DialogPeek)theD)->editField + 1;
if (iHit == kItemQuantity) {
	if (keypressed >= 32 && ((thEv->modifiers & cmdKey) == 0)) {
		result = ( Munger((Handle)GetString(kSTR_RegDECALLOWED), 1L,
						&keypressed, 1L, 0L, 0L) >= 0L );
		}
	}
return result;
}

void RegistrationPostProcessKey(EventRecord */*thEv*/, DialogPtr theD)
{

if ((((DialogPeek)theD)->editField + 1) == kItemQuantity)
	RegInitProc(theD);
}

void HandleRegistration(/*ModalFilterProcPtr filterProc,*/
						Point *regDTopLeft,
						long (*regCfgInfo)(Handle, long, StringPtr),
						void (*AdjustMenus)(void),
						void (*Handle_My_Menu)(long),
						void (*DomyKeyEvent)(EventRecord *),
						void (*DoUpdate)(EventRecord *),
						void (*DoActivate)(EventRecord *),
						void (*DoHiLevEvent)(EventRecord *),
						void (*DoOSEvent)(EventRecord *),
						void (*DoIdle)(void),
						unsigned long minSleep,
						short howManyDollars)
{
enum {
kNumStrings = 13,
kMyRegText = 256
};

StringPtr	regInfo[kNumStrings];
StdFileResult	theChosenFile;
ParamBlockRec	myPB;
EventRecord	dummyEv;
Str63	tempS = "\p";
long	gResp;
short	tmpFRefN;
const short	zero = 0x0130;
const short	dash = 0x012D;
const char	optspace = '�';
Handle	regText;
VersRecHndl	myvers;
StringHandle	sh;
StringPtr	myAppName = nil;
long	lOffset;
short	i;
OSErr	err;
SignedByte	savedState;

dialogItems	things[] = {{ ok, 0, 0L },
						{ cancel, 0, 0L },
						{ kItemUserName, 0, 0L },
						{ kItemCompany, 0, 0L },
						{ kItemAddress, 0, 0L },
						{ kItemCity, 0, 0L },
						{ kItemState, 0, 0L },
						{ kItemZIP, 0, 0L },
						{ kItemCountry, 0, 0L },
						{ kItemE_mail, 0, 0L },
						{ kItemPhone, 0, 0L },
						{ kItemFAX, 0, 0L },
						{ kItemQuantity, 0, 0L },
						{ kItemCopyFrom, 0, 0L },
						{ kItemComments, 0, 0L },
						{ 0, 0, 0L}
						};

lOffset = (long)NewPtrClear(kNumStrings * sizeof(Str255));
if (lOffset) {
	things[kItemUserName-1].refCon = lOffset;
	regInfo[0] = (StringPtr)lOffset;
	lOffset += (kNumStrings - 1) * sizeof(Str255);
	for (i = kItemComments - 1; i >= kItemCompany - 1; i--, lOffset -= sizeof(Str255)) {
		things[i].refCon = lOffset;
		regInfo[i-2] = (StringPtr)lOffset;
		}
	
	(void)PLstrcpy((StringPtr)things[kItemQuantity-1].refCon, "\p2");
	
	gHowManyDollars = howManyDollars;
	sh = GetString(kSTR_ApplicationName);
	if (sh) {
		savedState = WantThisHandleSafe((Handle)sh);
		myAppName = *sh;
		}
	
	ParamText(myAppName, (ConstStr255Param)&zero, (ConstStr255Param)&zero, nil);
	if (sh)
		HSetState((Handle)sh, savedState);
	do
		if (HandleMovableModalDialog((dialogItemsPtr)&things, regDTopLeft, nil, nil, RegInitProc, nil, nil,
				AdjustMenus,
				Handle_My_Menu,
				DomyKeyEvent,
				RegistrationPreProcessKey,
				RegistrationPostProcessKey,
				DoUpdate,
				DoActivate,
				DoHiLevEvent,
				DoOSEvent,
				DoIdle,
				minSleep,
				kDLOG_Register) == ok)
			do {
				if ((err = AskForDestFile(&theChosenFile, nil, nil)) == dupFNErr) {
					InitCursor();
					(void)StopAlert(kALRT_REGCANNOTREPLACE, nil);
					}
				}
			while (err == dupFNErr);
		else
			err = dupFNErr;
	while (err == userCanceledErr);
	if (err == noErr) {
		regText = Get1Resource('TEXT', kMyRegText);
		if (regText) {
			DetachResource(regText);
			myvers = (VersRecHndl)Get1Resource('vers', 1);
			if (myvers) {
				(void) PLstrcpy(tempS, (*myvers)->shortVersion);
				ReleaseResource((Handle)myvers);
				}
			lOffset = Munger(regText, 0L, &optspace, 1L, &tempS[1], tempS[0]);
			for (i = kItemUserName-3; i <= kItemFAX-3; i++)
				lOffset = Munger(regText, lOffset, &optspace, 1L, &regInfo[i][1], regInfo[i][0]);
			for (i = kItemCopyFrom-3; i <= kItemComments-3; i++)
				lOffset = Munger(regText, lOffset, &optspace, 1L, &regInfo[i][1], regInfo[i][0]);
			
			/* Mac model */
			if (Gestalt(gestaltMachineType, &gResp) == noErr) {
				myAppName = GetPtrIndHString(GetResource('STR#', kMachineNameStrID), (unsigned short)(gResp - 1L));
				lOffset = Munger(regText, lOffset, &optspace, 1L, &myAppName[1], myAppName[0]);
				}
			else {
				lOffset = Munger(regText, lOffset, &optspace, 1L, &((char *)&dash)[1], 1L);
				}
			/* Sys version */
			if (Gestalt(gestaltSystemVersion, &gResp) == noErr) {
				BCDVersNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* addr mode info */
			if (Gestalt(gestaltAddressingModeAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* A/UX info */
			if (Gestalt(gestaltAUXVersion, &gResp) == noErr) {
				BCDVersNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Hardware info */
			if (Gestalt(gestaltHardwareAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* VM info */
			if (Gestalt(gestaltVMAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Power Manager info */
			if (Gestalt(gestaltPowerMgrAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Processor type info */
			if (Gestalt(gestaltProcessorType, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* System architecture info */
			if (Gestalt(gestaltSysArchitecture, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Native processor info */
			if (Gestalt(gestaltNativeCPUtype, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* System 7 tuner info */
			if (Gestalt('bugz', &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Hardware System Updater info */
			if (Gestalt('bugy', &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Hardware System Updater 3 info */
			if (Gestalt('bugx', &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* AppleScript info */
			if (Gestalt(gestaltAppleScriptVersion, &gResp) == noErr) {
				BCDLongVersNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* QuickTime info */
			if (Gestalt(gestaltQuickTimeVersion, &gResp) == noErr) {
				BCDLongVersNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Thread Manager info */
			if (Gestalt(gestaltThreadMgrAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Display Manager attr */
			if (Gestalt(gestaltDisplayMgrAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Display Manager version */
			if (Gestalt(gestaltDisplayMgrVers, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Drag Manager attr */
			if (Gestalt(gestaltDragMgrAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* AE attr */
			if (Gestalt(gestaltAppleEventsAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Finder attr */
			if (Gestalt(gestaltFinderAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* FS attr */
			if (Gestalt(gestaltFSAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* OS attr */
			if (Gestalt(gestaltOSAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* Standard File attr */
			if (Gestalt(gestaltStandardFileAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* TextEdit attr */
			if (Gestalt(gestaltTEAttr, &gResp) == noErr) {
				MyNumToString(gResp, tempS);
				}
			else {
				*(short *)tempS = dash;
				}
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			/* custom configuration */
			if (regCfgInfo)
				lOffset = regCfgInfo(regText, lOffset, tempS);
			
			/* quantity */
			lOffset = Munger(regText, lOffset, &optspace, 1L, &regInfo[kItemQuantity-3][1], regInfo[kItemQuantity-3][0]);
			StringToNum(regInfo[kItemQuantity-3], &gResp);
			MyNumToString(howManyDollars * sqrt(gResp), tempS);
	//		UnloadSeg(sqrt);
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
			IUDateString(LMGetTime(), longDate, tempS);
			lOffset = Munger(regText, lOffset, &optspace, 1L, &tempS[1], tempS[0]);
// 6
			if ((err = FSpCreateCompat(&theChosenFile.destFile, 'ttxt', 'TEXT', theChosenFile.theScript)) == noErr) {
				if ((err = FSpOpenDFCompat(&theChosenFile.destFile, fsWrPerm, &tmpFRefN)) == noErr) {
// 6
					HLockHi(regText);
					myPB.ioParam.ioCompletion = nil;
					myPB.ioParam.ioBuffer = *regText;
					myPB.ioParam.ioReqCount = InlineGetHandleSize(regText);
					myPB.ioParam.ioPosMode = fsFromStart;
					myPB.ioParam.ioRefNum = tmpFRefN;
					myPB.ioParam.ioPosOffset = 0L;
					(void)PBWriteAsync(&myPB);
					while (myPB.ioParam.ioResult > 0) {
						SystemTask();
						(void)EventAvail(everyEvent, &dummyEv);
						}
					HUnlock(regText);
					(void)FSClose(tmpFRefN);
					InitCursor();
					MyNumToString(myPB.ioParam.ioResult, tempS);
					ParamText(theChosenFile.destFile.name, tempS, nil, nil);
					if (myPB.ioParam.ioResult == noErr) {
						(void) NoteAlert(kALRT_REGSUCCESSSAVED, nil);
						}
					else if (myPB.ioParam.ioResult == dskFulErr) {
// 6
						(void) FSpDeleteCompat(&theChosenFile.destFile);
// 6
						(void) StopAlert(kALRT_REGDISKFULL, nil);
						}
					else
						(void) StopAlert(kALRT_REGUNKNOWNWRITEERROR, nil);
					}
				else
					(void) StopAlert(kALRT_REGCOULDNOTOPENFORWRITING, nil);
				}
			else
				(void) StopAlert(kALRT_REGCOULDNOTCREATE, nil);
			DisposeHandle(regText);
			}
		}
	DisposePtr((Ptr)regInfo[0]);
	}
else
	SysBeep(30);
}

void RegInitProc(DialogPtr myDPtr)
{
Str255	textStr, discountStr;
Rect	iRect;
//GrafPtr	savePort;
Handle	iHandle;
long	qty;
short	iType;
//register RgnHandle	updtRgn;

GetDialogItem(myDPtr, kItemQuantity, &iType, &iHandle, &iRect);
GetDialogItemText(iHandle, textStr);
StringToNum(textStr, &qty);
MyNumToString(gHowManyDollars * qty, textStr);
MyNumToString(gHowManyDollars * sqrt(qty), discountStr);
//UnloadSeg(sqrt);
ParamText(nil, textStr, discountStr, nil);
GetDialogItem(myDPtr, kItemTotals, &iType, &iHandle, &iRect);
// correggere con InvalRect !!!!
/*updtRgn = NewRgn();
if (updtRgn) {
	RectRgn(updtRgn, &iRect);
	UpdtDialog(myDPtr, updtRgn);
	DisposeRgn(updtRgn);
	}
*/
//GetPort(&savePort);
//SetPort(myDPtr);
//DebugStr("\pInvalRect imminent");
InvalRect(&iRect);
//SetPort(savePort);
}

OSErr AskForDestFile(StdFileResultPtr whichFile, ConstStr255Param prompt, StringPtr docName)
{
enum {
kSTR_PROMPT = 300,
kSTR_DEFNAME
};

StandardFileReply	mySFR;
register Handle	sH1;
register Handle	sH2;
register SignedByte	state1, state2;
register OSErr	err = userCanceledErr;

// 6
FabStandardPutFile(prompt ? prompt : (sH1 = (Handle)GetString(kSTR_PROMPT),
										state1 = WantThisHandleSafe(sH1),
										(ConstStr255Param)*sH1),
				docName ? docName : (sH2 = (Handle)GetString(kSTR_DEFNAME),
									state2 = WantThisHandleSafe(sH2),
									(ConstStr255Param)*sH2),
				&mySFR);
// 6

if (docName == nil)
	HSetState(sH2, state2);
if (prompt == nil)
	HSetState(sH1, state1);

if (mySFR.sfGood) {
	if (mySFR.sfReplacing)
		err = dupFNErr;
	else {
		err = noErr;
		whichFile->destFile = mySFR.sfFile;
		whichFile->theScript = mySFR.sfScript;
		if (docName)
			(void) PLstrncpy(docName, mySFR.sfFile.name, 31);
		}
	}
return err;
}

void BCDVersNumToString(long num, StringPtr str)
{
Str15	tmpstr;
const short	onepoint = 0x012E;
register long	tmp;

str[0] = 0;
tmp = num;
tmp >>= 8;
tmp &= 0xFF;
MyNumToString(tmp, str);
(void) PLstrcat(str, (StringPtr)&onepoint);
tmp = num;
tmp >>= 4;
tmp &= 0xF;
MyNumToString(tmp, tmpstr);
(void) PLstrcat(str, (StringPtr)&tmpstr);
tmp = num;
tmp &= 0xF;
if (tmp) {
	MyNumToString(tmp, tmpstr);
	(void) PLstrcat(str, (StringPtr)&onepoint);
	(void) PLstrcat(str, (StringPtr)&tmpstr);
	}
}

void BCDLongVersNumToString(long num, StringPtr str)
{
Str15	tmpstr;
const short	onepoint = 0x012E;
const short	devStr = 0x0164;
const short	alphaStr = 0x0161;
const short	betaStr = 0x0162;
const short	releaseStr = 0x0172;
register long	tmp;

str[0] = 0;
tmp = num;
tmp >>= 24;
tmp &= 0xFF;
MyNumToString(tmp, str);
(void) PLstrcat(str, (StringPtr)&onepoint);
tmp = num;
tmp >>= 20;
tmp &= 0xF;
MyNumToString(tmp, tmpstr);
(void) PLstrcat(str, (StringPtr)&tmpstr);
tmp = num;
tmp >>= 16;
tmp &= 0xF;
if (tmp) {
	MyNumToString(tmp, tmpstr);
	(void) PLstrcat(str, (StringPtr)&onepoint);
	(void) PLstrcat(str, (StringPtr)&tmpstr);
	}
tmp = num;
tmp >>= 8;
tmp &= 0xFF;
switch (tmp) {
	case developStage:
		(void) PLstrcat(str, (StringPtr)&devStr);
		break;
	case alphaStage:
		(void) PLstrcat(str, (StringPtr)&alphaStr);
		break;
	case betaStage:
		(void) PLstrcat(str, (StringPtr)&betaStr);
		break;
	case finalStage:
		(void) PLstrcat(str, (StringPtr)&releaseStr);
		break;
	}
tmp = num;
tmp &= 0xFF;
if (tmp) {
	MyNumToString(tmp, tmpstr);
	(void) PLstrcat(str, (StringPtr)&tmpstr);
	}
}

#if	!defined(FabNoSegmentDirectives)
#pragma segment Init
#endif

/* Code from Apple� converted by Stefan Kurth in super-optimized (!) C */

/* InitGraf is always implemented (trap $A86E). If the trap table is big
  enough, trap $AA6E will always point to either Unimplemented or some other
  trap, but will never be the same as InitGraf. Thus, you can check the size
  of the trap table by asking if the address of trap $A86E is the same as
  $AA6E. */

#define NumToolboxTraps	((GetToolTrapAddress(_InitGraf) == GetToolTrapAddress(0xAA6E)) ? 0x0200 : 0x0400)

/* Determines the type of a trap based on its trap number. If bit 11 is clear,
  then it is an OS trap. Otherwise, it is a Toolbox trap. */
/* OS traps start with A0, Tool with A8 or AA. */

#define GetTrapType(which)	(((which) & 0x0800) ? ToolTrap : OSTrap)
#define IsToolTrap(trap)	((trap) & 0x0800)

/* Check to see if a given trap is implemented. This is only used by the
  Initialize routine in this program, so we put it in the Initialize segment.*/

Boolean TrapAvailable(short theTrap)
{
register UniversalProcPtr	theTrapAddress;
register short	temp;

temp = theTrap;
if(IsToolTrap(temp)) {
	temp &= 0x07FF;
	if( temp >= NumToolboxTraps)
		return false;
	theTrapAddress = GetToolTrapAddress(temp);
	}
else
	theTrapAddress = GetOSTrapAddress(temp);
return( GetToolTrapAddress(_Unimplemented) != theTrapAddress);
}

OSErr InstallRequiredAEHandlers(AEEventHandlerProcPtr myOAPP,
								AEEventHandlerProcPtr myODOC,
								AEEventHandlerProcPtr myPDOC,
								AEEventHandlerProcPtr myQUIT)
{
OSErr	err;

gmyOAPP_UPP = NewAEEventHandlerProc(myOAPP);
gmyODOC_UPP = NewAEEventHandlerProc(myODOC);
gmyPDOC_UPP = NewAEEventHandlerProc(myPDOC);
gmyQUIT_UPP = NewAEEventHandlerProc(myQUIT);
if(noErr == (err = AEInstallEventHandler(kCoreEventClass, kAEOpenApplication, gmyOAPP_UPP, 0, false)))
	if(noErr == (err = AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments, gmyODOC_UPP, 0, false)))
		if(noErr == (err = AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments, gmyPDOC_UPP, 0, false)))
			if(noErr == (err = AEInstallEventHandler(kCoreEventClass, kAEQuitApplication, gmyQUIT_UPP, 0, false)))
				;

return err;
}

#endif

