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
#include	"Disasm.h"
#include	"DoEditDialog.h"
#include	"DoMenu.h"
#include	"DragManSim.h"
#include	"Dump.h"
#include	"Globals.h"
#include	"MovableModal.h"
#include	"Preferences.h"
#include	"Main.h"
#include	"Registers.h"
#include	"Conversions.h"

#if defined(FabSystem7orlater)

static Boolean PrefsPreProcessKey(EventRecord *thEv, DialogPtr theD);
static pascal Boolean PrefsFilter(DialogPtr theD, EventRecord *thEv, short *iHit);
static void SetTopLeft(Point *thePt, WindowPtr w);

#pragma segment Rare

enum {
kItemRememberWindPos = 3,
kItemContinuousDumpScroll,
kItemInfLoopsDetect,
kItemInitialPCVal,
kItemInitialSPVal,
kItemStackSize
};

void Preferences(void)
{
enum {
kDLOG_Prefs = 262
};

Str255	InitPCStr, InitSPStr, StackSizeStr;
long	dummy;

dialogItems	things[] = {{ ok, 0, 1L },
						{ cancel, 0, 0L },
						{ kItemRememberWindPos, 0, 0L },
						{ kItemContinuousDumpScroll, 0, 0L },
						{ kItemInfLoopsDetect, 0, 0L },
						{ kItemInitialPCVal, 0, 0L },
						{ kItemInitialSPVal, 0, 0L },
						{ kItemStackSize, 0, 0L },
						{ 0, 0, 0L}
						};

things[kItemRememberWindPos-1].refCon = gPrefs.remembWind;
things[kItemContinuousDumpScroll-1].refCon = gPrefs.NeXTScroll;
things[kItemInfLoopsDetect-1].refCon = gPrefs.infLoopsDetect;
things[kItemInitialPCVal-1].refCon = (long)&InitPCStr;
things[kItemInitialSPVal-1].refCon = (long)&InitSPStr;
things[kItemStackSize-1].refCon = (long)&StackSizeStr;

ShortToHexString(gPrefs.DefPCValue, InitPCStr);
ShortToHexString(gPrefs.DefSPValue, InitSPStr);
MyNumToString(gPrefs.DefStkSize, StackSizeStr);

if (HandleMovableModalDialog(things, gPrefs.remembWind ? &gPrefs.generalPrefsTL : nil, nil, nil, nil, nil, nil,
	AdjustMenus,
	Handle_My_Menu,
	DomyKeyEvent,
	PrefsPreProcessKey,
	nil,
	DoUpdate,
	DoActivate,
	DoHighLevelEvent,
	DoOSEvent,
	DoIdle,
	ULONG_MAX,
	kDLOG_Prefs) == ok) {
	gPrefs.remembWind = things[kItemRememberWindPos-1].refCon;
	gPrefs.NeXTScroll = things[kItemContinuousDumpScroll-1].refCon;
	gPrefs.infLoopsDetect = things[kItemInfLoopsDetect-1].refCon;
	HexStringToShort(InitPCStr, (short *)&gPrefs.DefPCValue);
	HexStringToShort(InitSPStr, (short *)&gPrefs.DefSPValue);
	StringToNum(StackSizeStr, &dummy);
	gPrefs.DefStkSize = dummy;
	}
}

Boolean PrefsPreProcessKey(EventRecord *thEv, DialogPtr theD)
{
short	iHit;
unsigned char	keypressed;
Boolean	result = true;

keypressed = CHARFROMMESSAGE(thEv->message);
if ((keypressed >= 'a') && (keypressed <= 'z')) {
	keypressed -= 'a' - 'A';	/* cambiare! */
	CHARFROMMESSAGE(thEv->message) = keypressed;
	}
iHit = ((DialogPeek)theD)->editField + 1;
if (keypressed >= 32 && ((thEv->modifiers & cmdKey) == 0)) {
	switch (iHit) {
		case kItemInitialPCVal:
		case kItemInitialSPVal:
			result = ( Munger((Handle)GetString(kSTR_HEXALLOWED), 1L, &keypressed,
							1L, 0L, 0L) >= 0L );
			break;
		case kItemStackSize:
			result = ( Munger((Handle)GetString(kSTR_DECALLOWED), 1L, &keypressed,
							1L, 0L, 0L) >= 0L );
			break;
		}
	}
return result;
}

/* PrefsFilter: the filterProc routine for the Prefs dialog */
/*
static pascal Boolean PrefsFilter(DialogPtr theD, EventRecord *thEv, short *iHit)
{
GrafPtr	savePort;
unsigned char	keypressed;
register Boolean	retVal;

switch(thEv->what) {
	case keyDown	:
	case autoKey	:
		keypressed = CHARFROMMESSAGE(thEv->message);
		if ((keypressed >= 'a') && (keypressed <= 'z')) {
			keypressed -= 'a' - 'A';	// cambiare!
			CHARFROMMESSAGE(thEv->message) = keypressed;
			}
		if ((keypressed >= 32) && (keypressed != 0x7F) && ((thEv->modifiers & cmdKey) == 0)) {
			*iHit = ((DialogPeek)theD)->editField + 1;
			switch( *iHit ) {
				case kItemInitialPCVal:
				case kItemInitialSPVal:
					return( Munger((Handle)GetString(kSTR_HEXALLOWED), 1L, &keypressed,
									1L, 0L, 0L) < 0L );
					break;
				case kItemStackSize:
					return( Munger((Handle)GetString(kSTR_DECALLOWED), 1L, &keypressed,
									1L, 0L, 0L) < 0L );
					break;
				}
			}
		break;
	case updateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoUpdate(thEv);
			*iHit = kfakeUpdateItem;
			return true;
			}
		break;
	case activateEvt:
		if (theD != (DialogPtr)thEv->message) {
			DoActivate(thEv);
			*iHit = kfakeUpdateItem;
			return true;
			}
		break;
	}
GetPort(&savePort);
SetPort(theD);
retVal = StdFilterProc(theD, thEv, iHit);
SetPort(savePort);
return retVal;
}
*/
#pragma segment CleanUp

/* SavePreferencesFile: in the end we save our gPrefs */

void SavePreferencesFile(void)
{
ParamBlockRec	myPB;
EventRecord	dummyEv;
FSSpec	myFSS;
register Handle	myStrHand;
short	prefsFRefNum;
Boolean	targetFolder, isAnAlias;
register OSErr	err;
register SignedByte	oldState;

oldState = WantThisHandleSafe(myStrHand = (Handle)GetString(kSTR_PREFSFILENAME));
if ((err = FindFolder(kOnSystemDisk, kPreferencesFolderType, kCreateFolder,
			&myFSS.vRefNum, &myFSS.parID)) == noErr) {
	err = FSMakeFSSpecCompat(myFSS.vRefNum, myFSS.parID, (ConstStr255Param)*myStrHand, &myFSS);
	if ((err == noErr) || (err == fnfErr)) {
		if (err == fnfErr)
#ifndef __SCRIPT__
#define	smSystemScript	-1
#endif
			err = FSpCreateCompat(&myFSS, '????', kPreferencesFolderType, smSystemScript);
		if (err == noErr)
			if ((err = ResolveAliasFile(&myFSS, true, &targetFolder, &isAnAlias)) == noErr)
				if (targetFolder)
					err = paramErr;
				else
					if ((err = FSpOpenDFCompat(&myFSS, fsWrPerm, &prefsFRefNum)) == noErr) {
						SetTopLeft(&gPrefs.AnimTopLeft, gWPtr_Animation);
						SetTopLeft(&gPrefs.RegsTopLeft, gWPtr_Registers);
						gPrefs.RegsBase = GetControlValue(Ctrl_Base);
						gPrefs.IOUserState = (*(WStateDataHandle)((WindowPeek)gWPtr_IO)->dataHandle)->userState;
						SetTopLeft(&gPrefs.MProgTopLeft, gWPtr_Microprogram_Ed);
						SetTopLeft(&gPrefs.DumpTopLeft, gWPtr_Dump);
						gPrefs.DumpHeight = PRCT_B(gWPtr_Dump) - PRCT_T(gWPtr_Dump);
						gPrefs.DumpScrollVal = GetControlValue(dumpVScroll);
						SetTopLeft(&gPrefs.DisasmTopLeft, gWPtr_Disasm);
						gPrefs.DisasmHeight = PRCT_B(gWPtr_Disasm) - PRCT_T(gWPtr_Disasm);
						gPrefs.DisasmScrollVal = GetControlValue(disasmVScroll);
						gPrefs.AnimVisible = ((WindowPeek)gWPtr_Animation)->visible;
						gPrefs.RegsVisible = ((WindowPeek)gWPtr_Registers)->visible;
						gPrefs.IOVisible = ((WindowPeek)gWPtr_IO)->visible;
						gPrefs.DumpVisible = ((WindowPeek)gWPtr_Dump)->visible;
						gPrefs.DisasmVisible = ((WindowPeek)gWPtr_Disasm)->visible;
						myPB.ioParam.ioCompletion = 0L;
						myPB.ioParam.ioRefNum = prefsFRefNum;
						myPB.ioParam.ioBuffer = (Ptr)&gPrefs;
						myPB.ioParam.ioReqCount = sizeof(struct myprefs);
						myPB.ioParam.ioPosMode = fsFromStart;
						myPB.ioParam.ioPosOffset = 0L;
						(void)PBWriteAsync(&myPB);
						while (myPB.ioParam.ioResult > 0) {
							SystemTask();
							(void)EventAvail(everyEvent, &dummyEv);
							}
						(void)FSClose(prefsFRefNum);
						if ((err = myPB.ioParam.ioResult) == noErr)
							err = AddSTRRes2Doc(&myFSS, '????', kPreferencesFolderType,
												kSTR_NOOPENORPRINT, smSystemScript);
						}
		}
	}
HSetState(myStrHand, oldState);
}

/* SetTopLeft: common routine which calculates the topLeft coordinate
of a GrafPort╒s portRect in global coordinates;
it _changes_ the current port */

static void SetTopLeft(Point *thePt, GrafPtr w)
{
*thePt = topLeft(w->portRect);
SetPort(w);
LocalToGlobal(thePt);
}

void CleanUp(void)
{
if (gDragManagerActive) {
	MyRemoveHWindow(gWPtr_Registers);
	MyRemoveHWindow(gWPtr_Disasm);
	MyRemoveHWindow(gWPtr_Dump);
	}
}

#endif

