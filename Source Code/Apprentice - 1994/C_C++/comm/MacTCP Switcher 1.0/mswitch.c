/*---------------------------------------------------------------------------

	MacTCP Switcher - A Program to Save and Restore MacTCP Settings
	
	John Norstad
	Academic Computing and Network Services
	Northwestern University
	
	j-norstad@nwu.edu
	
	Copyright © 1993, Northwestern University
	
---------------------------------------------------------------------------*/

#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Traps.h>
#include <Folders.h>
#include <Aliases.h>
#include <Script.h>
#include <Palettes.h>
#include <AppleTalk.h>

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

pascal OSErr SetDialogDefaultItem (DialogPtr theDialog,
	short newItem) = {0x303C,0x0304,0xAA68};

pascal OSErr SetDialogCancelItem (DialogPtr theDialog,
	short newItem) = {0x303C,0x0305,0xAA68};

pascal OSErr SetDialogTrackCursor (DialogPtr theDialog,
	Boolean tracks) = {0x303C,0x0306,0xAA68};
	
pascal OSErr GetStdFilterProc (ProcPtr *theProc) =
	{0x303C,0x0203,0xAA68};

#define kCreator 	'MCPS'
#define kFileType 	'MCPT'

#define kMacTCPPrepCreator		'mtcp'
#define kMacTCPPrepFileType		'mtpp'

#define kMacTCPConfigRsrcType	'ipln'
#define kMacTCPConfigRsrcID		128
#define kMacTCPDnrInfoRsrcType	'dnsl'
#define kMacTCPDnrInfoRsrcID	128

#define kMacTCPDriverName 		"\p.ipp"

#define kStringsID 		128
#define kMBarID 		128

#define kErrAlert 			128
#define kUnexpectedErrAlert 129
#define kSetMacTCPAlert		130
#define kRestartAlert		132
#define kMacTCPSetAlert		133

#define kNeedSystem7Alert	200

#define kSetMacTCPAlertSet		1
#define kSetMacTCPAlertCancel	2
#define kSetMacTCPAlertSave		3

#define kRestartAlertOK			1
#define kRestartAlertRestart	2

#define kMacTCPPrepStr			1
#define kSaveAsPromptStr		2
#define kMacTCPStr				3
#define kMacTCPNotFoundStr		4
#define kCanOnlyOpenOneDocStr	5

#define kAppleMenu 		128
#define kEmptyItem		1

#define kFileMenu 		129

#define kEditMenu 		130
#define kUndoItem 		1
#define kCutItem 		3
#define kCopyItem 		4
#define kPasteItem 		5
#define kClearItem 		6

typedef struct TDialogCommandKeyItem {
	short itemNumber;
	char key;
} TDialogCommandKeyItem;

static short gNumDialogCommandKeyItems;
static TDialogCommandKeyItem gDialogCommandKeyItems[2];

static unsigned short NumToolboxTraps (void)
{
	return NGetTrapAddress(_InitGraf, ToolTrap) == 
		NGetTrapAddress(0xAA6E, ToolTrap) ? 0x200 : 0x400;
}

static TrapType GetTrapType (unsigned short theTrap)
{
	return (theTrap & 0x0800) > 0 ? ToolTrap : OSTrap;
}

static Boolean TrapAvailable (unsigned short theTrap)
{
	TrapType tType;
	
	tType = GetTrapType(theTrap);
	if (tType == ToolTrap) {
		theTrap = theTrap & 0x07ff;
		if (theTrap >= NumToolboxTraps()) theTrap = _Unimplemented;
	}
	return NGetTrapAddress(theTrap, tType) != 
		NGetTrapAddress(_Unimplemented, ToolTrap);
}

static void DlgFlashButton (DialogPtr dlg, short item)
{
	short itemType;
	ControlHandle theItem;
	Rect box;
	long myticks;
	
	GetDItem(dlg, item, &itemType, (Handle*)&theItem, &box);
	HiliteControl(theItem, 1);
	Delay(8, &myticks);
	HiliteControl(theItem, 0);
}

static pascal Boolean DialogFilter (DialogPtr dlg, EventRecord *theEvent, 
	short *itemHit)
{
	GrafPtr savedPort;
	Boolean result = false;
	OSErr err;
	ProcPtr standardFilterProc;
	char key;
	TDialogCommandKeyItem *p, *pEnd;

	if ((theEvent->what == keyDown || theEvent->what == autoKey) &&
		(theEvent->modifiers & cmdKey) != 0)
	{
		key = toupper(theEvent->message & charCodeMask);
		p = gDialogCommandKeyItems;
		pEnd = p + gNumDialogCommandKeyItems;
		while (p < pEnd) {
			if (key == toupper(p->key)) {
				*itemHit = p->itemNumber;
				DlgFlashButton(dlg, p->itemNumber);
				return true;
			}
			p++;
		}
	}
	GetPort(&savedPort);
	SetPort(dlg);
	err = GetStdFilterProc(&standardFilterProc);
	if (err == noErr) {
		result = ((ModalFilterProcPtr)standardFilterProc)(dlg, theEvent, itemHit);
	}
	SetPort(savedPort);
	return result;
}

static void MyModalDialog (short dlgID, short okItem, short cancelItem, 
	StringPtr p1, StringPtr p2, short numCommandKeys, short *itemHit)
{
	DialogPtr dlg;
	short hit;
	
	InitCursor();
	dlg = GetNewDialog(dlgID, nil, (WindowPtr)-1);
	if (okItem != 0) SetDialogDefaultItem(dlg, okItem);
	if (cancelItem != 0) SetDialogCancelItem(dlg, cancelItem);
	ParamText(p1, p2, "\p", "\p");
	gNumDialogCommandKeyItems = numCommandKeys;
	ModalDialog(DialogFilter, &hit);
	DisposeDialog(dlg);
	if (itemHit != nil) *itemHit = hit;
}

static OSErr MyFSpOpenResFile (FSSpec *fSpec, short permission, short *refNum)
{
	OSErr err;
	
	*refNum = FSpOpenResFile(fSpec, permission);
	if (*refNum != -1) return noErr;
	err = ResError();
	if (err != noErr) return err;
	return resFNotFound;
}

static OSErr MyFSpCreateResFile (FSSpec *fSpec, OSType creator, OSType fileType,
	ScriptCode scriptTag)
{
	FSpCreateResFile(fSpec, creator, fileType, scriptTag);
	return ResError();
}

static OSErr MyGet1Resource (OSType type, short id, Handle *h)
{
	OSErr err;
	
	*h = Get1Resource(type, id);
	if (*h != nil) return noErr;
	err = ResError();
	if (err != noErr) return err;
	return resNotFound;
}

static OSErr MyAddResource (Handle h, OSType type, short id)
{
	Handle rsrc;
	OSErr err;
	
	rsrc = Get1Resource(type, id);
	if (rsrc != nil) {
		RmveResource(rsrc);
		err = ResError();
		if (err != noErr) return err;
		DisposeHandle(rsrc);
	}
	AddResource(h, type, id, "\p");
	return ResError();
}

static void ErrorMessage (short errNum)
{
	Str255 msg;

	GetIndString(msg, kStringsID, errNum);
	MyModalDialog(kErrAlert, ok, 0, msg, "\p", 0, nil);
}

static void UnexpectedErrorMessage (OSErr err)
{
	Str255 errStr;
	
	NumToString(err, errStr);
	MyModalDialog(kUnexpectedErrAlert, ok, 0, errStr, "\p", 0, nil);
}

static Boolean MacTCPIsOpen (void)
{
	DCtlHandle *unitTableEntryPtr, *unitTableEnd, dctlHandle;
	short flags;
	Ptr driver;

	unitTableEnd = UTableBase + UnitNtryCnt;
	for (unitTableEntryPtr = UTableBase; 
		unitTableEntryPtr < unitTableEnd; 
		unitTableEntryPtr++) 
	{
		dctlHandle = *unitTableEntryPtr;
		if (dctlHandle != nil) {
			flags = (**dctlHandle).dCtlFlags;
			driver = (**dctlHandle).dCtlDriver;
			if (flags & 0x40) driver = *(Handle)driver;
			if (EqualString(kMacTCPDriverName, (StringPtr)(driver+18), true, true))
				return (flags & 0x20) != 0;
		}
	}
	return false;
}

static void Restart (void)
{
	ProcessSerialNumber psn;
	AppleEvent message;
	AEAddressDesc targetAddr;
	OSErr err;
	Boolean targetAddrCreated = false;
	Boolean messageCreated = false;
	
	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kSystemProcess;
	err = AECreateDesc(typeProcessSerialNumber, (Ptr)&psn, 
		sizeof(ProcessSerialNumber), &targetAddr);
	if (err != noErr) goto exit;
	targetAddrCreated = true;
	err = AECreateAppleEvent(kCoreEventClass, kAERestart,
		&targetAddr, kAutoGenerateReturnID, kAnyTransactionID, &message);
	if (err != noErr) goto exit;
	messageCreated = true;
	err = AESend(&message, nil, kAENoReply | kAECanInteract, kAENormalPriority,
		kAEDefaultTimeout, nil, nil);
	if (err != noErr) goto exit;
	AEDisposeDesc(&message);
	AEDisposeDesc(&targetAddr);
	return;
	
exit:

	if (messageCreated) AEDisposeDesc(&message);
	if (targetAddrCreated) AEDisposeDesc(&targetAddr);
	UnexpectedErrorMessage(err);
}

static OSErr CopyResources (FSSpec *source, FSSpec *dest, OSType creator, OSType fileType)
{
	short refNum = -1;
	OSErr err;
	Handle ipConfig, dnrInfo;
	
	err = MyFSpOpenResFile(source, fsRdPerm, &refNum);
	if (err != noErr) goto exit;
	err = MyGet1Resource(kMacTCPConfigRsrcType, kMacTCPConfigRsrcID, &ipConfig);
	if (err != noErr) goto exit;
	DetachResource(ipConfig);
	err = MyGet1Resource(kMacTCPDnrInfoRsrcType, kMacTCPDnrInfoRsrcID, &dnrInfo);
	if (err != noErr) goto exit;
	DetachResource(dnrInfo);
	CloseResFile(refNum);
	refNum = -1;
	
	err = MyFSpOpenResFile(dest, fsRdWrPerm, &refNum);
	if (err == fnfErr) {
		err = MyFSpCreateResFile(dest, creator, fileType, smSystemScript);
		if (err != noErr) goto exit;
		err = MyFSpOpenResFile(dest, fsRdWrPerm, &refNum);
	}
	if (err != noErr) goto exit;
	err = MyAddResource(ipConfig, kMacTCPConfigRsrcType, kMacTCPConfigRsrcID);
	if (err != noErr) goto exit;
	err = MyAddResource(dnrInfo, kMacTCPDnrInfoRsrcType, kMacTCPDnrInfoRsrcID);
	if (err != noErr) goto exit;
	CloseResFile(refNum);
	return noErr; 
	
exit:

	if (refNum != -1) CloseResFile(refNum);
	return err;
}

static OSErr SaveMacTCPConfigToFile (void)
{
	Str255 prompt;
	StandardFileReply sfReply;
	FSSpec fSpec;
	FInfo fndrInfo;
	OSErr err;

	GetIndString(prompt, kStringsID, kSaveAsPromptStr);
	StandardPutFile(prompt, "\p", &sfReply);
	if (!sfReply.sfGood) return userCanceledErr;
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, true, 
		&fSpec.vRefNum, &fSpec.parID);
	if (err != noErr) return err;
	GetIndString(fSpec.name, kStringsID, kMacTCPPrepStr);
	err = FSpGetFInfo(&fSpec, &fndrInfo);
	if (err == fnfErr) {
		err = FindFolder(kOnSystemDisk, kControlPanelFolderType, true,
			&fSpec.vRefNum, &fSpec.parID);
		if (err != noErr) return err;
		GetIndString(fSpec.name, kStringsID, kMacTCPStr);
		err = FSpGetFInfo(&fSpec, &fndrInfo);
	}
	if (err != noErr) return err;
	return CopyResources(&fSpec, &sfReply.sfFile, kCreator, kFileType);
}

static pascal OSErr HandleAEOpenApp (AppleEvent *event, AppleEvent *reply, long refCon)
{
	OSErr err;

	err = SaveMacTCPConfigToFile();
	if (err == userCanceledErr) return noErr;
	if (err != noErr) goto exit;
	return noErr;
	
exit:

	if (err == fnfErr) {
		ErrorMessage(kMacTCPNotFoundStr);
	} else {
		UnexpectedErrorMessage(err);
	}
	return err;
}

static pascal OSErr HandleAEOpenDoc (AppleEvent *event, AppleEvent *reply, long refCon)
{
	OSErr err;
	AEDescList docList;
	long numItems;
	AEKeyword keywd;
	DescType returnedType;
	Size actualSize;
	FSSpec source, dest;
	short itemHit;
	
	err = AEGetParamDesc(event, keyDirectObject, typeAEList, &docList);
	if (err != noErr) goto exit;
	err = AECountItems(&docList, &numItems);
	if (err != noErr) goto exit;
	if (numItems != 1) {
		ErrorMessage(kCanOnlyOpenOneDocStr);
		return noErr;
	}
	err = AEGetNthPtr(&docList, 1, typeFSS, &keywd, &returnedType,
		(Ptr)&source, sizeof(source), &actualSize);
	if (err != noErr) goto exit;
	err = AEDisposeDesc(&docList);
	if (err != noErr) goto exit;
	while (true) {
		gDialogCommandKeyItems[0].itemNumber = kSetMacTCPAlertSave;
		gDialogCommandKeyItems[0].key = 'S';
		MyModalDialog(kSetMacTCPAlert, kSetMacTCPAlertSet, kSetMacTCPAlertCancel,
			source.name, "\p", 1, &itemHit);
		if (itemHit == kSetMacTCPAlertSet) break;
		if (itemHit == kSetMacTCPAlertCancel) return noErr;
		if (itemHit == kSetMacTCPAlertSave) {
			err = SaveMacTCPConfigToFile();
			if (err == userCanceledErr) return noErr;
			if (err != noErr) goto exit;
		}
	}
	err = FindFolder(kOnSystemDisk, kPreferencesFolderType, true, 
		&dest.vRefNum, &dest.parID);
	if (err != noErr) goto exit;
	GetIndString(dest.name, kStringsID, kMacTCPPrepStr);
	err = CopyResources(&source, &dest, kMacTCPPrepCreator, kMacTCPPrepFileType);
	if (err != noErr) goto exit;
	if (MacTCPIsOpen()) {
		gDialogCommandKeyItems[0].itemNumber = kRestartAlertRestart;
		gDialogCommandKeyItems[0].key = 'R';
		MyModalDialog(kRestartAlert, ok, 0, "\p", "\p", 1, &itemHit);
		if (itemHit == kRestartAlertRestart) Restart();
	} else {
		MyModalDialog(kMacTCPSetAlert, ok, 0, "\p", "\p", 0, nil);
	}
	return noErr;

exit:

	UnexpectedErrorMessage(err);
	return err;
}

static pascal OSErr HandleAEPrintDoc (AppleEvent *event, AppleEvent *reply, long refCon)
{
	return noErr;
}

static pascal OSErr HandleAEQuit (AppleEvent *event, AppleEvent *reply, long refCon)
{
	return noErr;
}

static void Init (void)
{
	EventRecord ev;
	long systemVersion;
	OSErr err;
	Boolean haveSystem7;

	MaxApplZone();
	
	InitGraf(&qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();

	FlushEvents(everyEvent,0);
	EventAvail(everyEvent, &ev);

	haveSystem7 = TrapAvailable(_Gestalt);
	if (haveSystem7) {
		err = Gestalt(gestaltSystemVersion, &systemVersion);
		haveSystem7 = err == noErr && systemVersion >= 0x0700;
	}
	if (!haveSystem7) {
		StopAlert(kNeedSystem7Alert, nil);
		ExitToShell();
	}

	SetMenuBar(GetNewMBar(kMBarID));
	AddResMenu(GetMHandle(kAppleMenu), 'DRVR');
	DrawMenuBar();

	AEInstallEventHandler(kCoreEventClass, kAEOpenApplication,
		HandleAEOpenApp, 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEOpenDocuments,
		HandleAEOpenDoc, 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEPrintDocuments,
		HandleAEPrintDoc, 0, false);
	AEInstallEventHandler(kCoreEventClass, kAEQuitApplication,
		HandleAEQuit, 0, false);
}

static void MainEvent (void)
{
	EventRecord ev;
	Boolean	gotEvt;

	while (true) {
		gotEvt = WaitNextEvent(everyEvent, &ev, 10, nil);
		if (gotEvt && ev.what == kHighLevelEvent) {
			AEProcessAppleEvent(&ev);
			break;
		}
	}
}

void main (void)
{
	Init();
	MainEvent();
}
