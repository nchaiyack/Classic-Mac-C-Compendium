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
#include	"Assembler.h"
#include	"ControlStore.h"
#include	"DoMenu.h"
#include	"Globals.h"
#include	"Main.h"
#include	"myMemory.h"
#include	"SimUtils.h"

//#pragma segment Main

static OSErr myGotRequiredParams(const AppleEvent *theAppleEvent);


pascal OSErr myHandleOAPP(const AppleEvent *theAppleEvent, AppleEvent */*reply*/,
					long /*handlerRefcon*/)
{
//#pragma unused (reply, handlerRefcon)

AEIdleUPP	myIdleFunctUPP = NewAEIdleProc(myIdleFunct);
register OSErr	err;

if ((err = myGotRequiredParams(theAppleEvent)) == noErr)
	if ((err = AESetInteractionAllowed(kAEInteractWithAll)) == noErr)
		if ((err = AEInteractWithUser(kNoTimeOut, 0L, myIdleFunctUPP)) == noErr)
			if (DocIsOpen == false) {
				DoNew();
				UnloadSeg(DoNew);
				DoMenuWindows(kMItem_Microprogram);
				}
if (myIdleFunctUPP)
	DisposeRoutineDescriptor(myIdleFunctUPP);
return err;
}

pascal OSErr myHandleODOC(const AppleEvent *theAppleEvent, AppleEvent */*reply*/,
					long /*handlerRefcon*/)
{
//#pragma unused (reply, handlerRefcon)

AEIdleUPP	myIdleFunctUPP = NewAEIdleProc(myIdleFunct);
FSSpec	myFSS;
FInfo	myInfo;
AEDescList	docList;
AEKeyword	keywd;
DescType	returnedType;
Size	actualSize;
long	itemsInList;
WindowPtr	ww;
register long	i;
register OSErr	err;
Boolean	movableModalInFront = false;

if (noErr == (err = AEGetParamDesc(theAppleEvent, keyDirectObject, typeAEList, &docList))) {
	if ((err = myGotRequiredParams(theAppleEvent)) == noErr)
		if ((err = AESetInteractionAllowed(kAEInteractWithAll)) == noErr)
			if ((err = AEInteractWithUser(kNoTimeOut, 0L, myIdleFunctUPP)) == noErr)
				if ((err = AECountItems(&docList, &itemsInList)) == noErr) {
					ww = FrontWindow();
					if (ww && isMovableModal(ww))
						movableModalInFront = true;
					else
					for (i = 1; i <= itemsInList; i++) {
						if ((err = AEGetNthPtr(&docList, i, typeFSS, &keywd, &returnedType,
												(Ptr)&myFSS, sizeof(myFSS), &actualSize)) == noErr) {
							if ((err = FSpGetFInfoCompat(&myFSS, &myInfo)) == noErr) {
								switch (myInfo.fdType) {
/* we should get the script code from the Apple Event; how??? */
#ifndef __SCRIPT__
#define	smSystemScript	-1
#endif
									case kFTY_CSTOREPAD	:	
									case kFTY_CSTORE	:	
										if (ReadyToTerminate()) {
											err = myOpenCSFile(&myFSS, smSystemScript,
																(myInfo.fdFlags & kfIsStationery) != 0);
											UnloadSeg(myOpenCSFile);
											}
										break;
									case kFTY_RAM	:	err = myOpenFile(&myFSS, gMMemory, kSIZE_RAM);
														UnloadSeg(myOpenFile);
										break;
									case kFTY_REG	:	err = OpenProcessorState(&myFSS);
														UnloadSeg(OpenProcessorState);
										break;
									case 'TEXT'	:	err = myAsmFile(&myFSS);
													UnloadSeg(myAsmFile);
										break;
									default :	err = paramErr;
									}
								}
							}
						}
					}
	(void)AEDisposeDesc(&docList);
	}
if (movableModalInFront) {
	(void) StopAlert_UPP(kALRT_PLEASEDISMISSMOVABLEMODAL, nil);
	err = noErr;
	}
if (myIdleFunctUPP)
	DisposeRoutineDescriptor(myIdleFunctUPP);
return err;
}

pascal OSErr myHandlePDOC(const AppleEvent */*theAppleEvent*/, AppleEvent */*reply*/,
					long /*handlerRefcon*/)
{
//#pragma unused (theAppleEvent, reply, handlerRefcon)

return errAEEventNotHandled;
}

pascal OSErr myHandleQUIT(const AppleEvent *theAppleEvent, AppleEvent */*reply*/,
					long /*handlerRefcon*/)
{
//#pragma unused (reply, handlerRefcon)

register OSErr	err;

err = myGotRequiredParams(theAppleEvent);
return(err ? err : (ReadyToTerminate() ? (gDoneFlag = true, noErr) : userCanceledErr));
}

static OSErr myGotRequiredParams(const AppleEvent *theAppleEvent)
{
DescType	returnedType;
Size	actualSize;
register OSErr	err;

err = AEGetAttributePtr(theAppleEvent, keyMissedKeywordAttr, typeWildCard,
						&returnedType, 0L, 0, &actualSize);
return(err ? (err == errAEDescNotFound ? noErr : err) : errAEEventNotHandled);
}

pascal Boolean myIdleFunct(EventRecord *event, long *sleepTime, RgnHandle *mouseRg)
{
switch (event->what) {
	case nullEvent:
		*mouseRg = (RgnHandle)nil;
		*sleepTime = ULONG_MAX;
		break;
	case updateEvt:
		DoUpdate(event);
		break;
	case activateEvt:
		DoActivate(event);
		break;
	case osEvt:
		DoOSEvent(event);
		break;
	}
return false;
}

pascal OSErr myHandleIO(const AppleEvent *theAppleEvent, AppleEvent */*reply*/,
					long /*handlerRefcon*/)
{
//#pragma unused (reply, handlerRefcon)

AEIdleUPP	myIdleFunctUPP = NewAEIdleProc(myIdleFunct);
register OSErr	err;

if ((err = myGotRequiredParams(theAppleEvent)) == noErr)
	if ((err = AESetInteractionAllowed(kAEInteractWithSelf)) == noErr)
		if ((err = AEInteractWithUser(kNoTimeOut, 0L, myIdleFunctUPP)) == noErr) {
			gInTheForeground = true;
			InitCursor();
//			PostEvent(keyDown, 3);
//			DebugStr("\pCheck menu flags");
			if ((*gMenu_Windows)->enableFlags & 1L &&
				(*gMenu_Windows)->enableFlags & (1L << kMItem_IO))
				DoMenuWindows(kMItem_IO);
			}
if (myIdleFunctUPP)
	DisposeRoutineDescriptor(myIdleFunctUPP);
return err;
}

pascal OSErr myHandleGenericAlert(const AppleEvent *theAppleEvent, AppleEvent */*reply*/,
							long /*handlerRefcon*/)
{
//#pragma unused (reply, handlerRefcon)

AEIdleUPP	myIdleFunctUPP = NewAEIdleProc(myIdleFunct);
register OSErr	err;

if ((err = myGotRequiredParams(theAppleEvent)) == noErr)
	if ((err = AESetInteractionAllowed(kAEInteractWithSelf)) == noErr)
		if ((err = AEInteractWithUser(kNoTimeOut, 0L, myIdleFunctUPP)) == noErr) {
			gInTheForeground = true;
			InitCursor();
			}
if (myIdleFunctUPP)
	DisposeRoutineDescriptor(myIdleFunctUPP);
return err;
}

