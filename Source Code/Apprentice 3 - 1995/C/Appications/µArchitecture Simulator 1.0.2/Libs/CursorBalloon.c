/*
Copyright � 1993,1994 by Fabrizio Oddone
��� ��� ��� ��� ��� ��� ��� ��� ��� ���
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

//#pragma load "MacDump"


#include	"UtilsSys7.h"
#include	"CursorBalloon.h"

static void myShowBalloon(RgnHandle theGlobalCoordsRgn,
							unsigned long theBalloon,
							short variant);

/* globals for the manager */

RgnHandle	mouseRgn, wideOpenRgn;

/*
The window will "remember" an object; you pass the following characteristics:
�	the region enclosing the object, in local coordinates;
�	the procedure recalculating the object region (in local coordinates);
	this will be called automatically whenever the window size changes;
	if the object will never be resized, pass nil;
	Here is the prototype for the recalcProc:
void RecalcIO(WindowPtr w, RgnBalloonCursPtr theObj);
�	the cursor (B&W) associated with this object;
	if you want the standard arrow cursor, pass nil;
�	the balloon that will appear when Balloon Help is on;
	use this macro: toBalloon(ID_of_STR#_resource, index );
	if you do not want a balloon, pass 0L;
�	the variant of the balloon (see Inside Macintosh for the meaning of this)
	if you do not want a balloon, pass 0;
*/

OSErr InstallRgnHandler(FabWindowPtr w, RgnHandle whichRgn, void (*recalcProc)(FabWindowPtr, RgnBalloonCursPtr),
						CursHandle cursor,
						unsigned long balloon, short ballnVariant, short iID)
{
RgnBalloonCurs	t;
Point	mypt = { 0, 0};
register OSErr err = appMemFullErr;

t.recalcRgnProc = recalcProc;
t.zoneLocal = whichRgn;
t.curs = cursor;
t.myBalloon = balloon;
t.balloonVariant = ballnVariant;
t.itemID = iID;
t.zoneGlobal = NewRgn();
if (t.zoneGlobal) {
	LocalToGlobal(&mypt);
	CopyRgn(whichRgn, t.zoneGlobal);
//	OffsetRgn(t.zoneGlobal, -(((WindowPtr)w)->portBits.bounds.left), -(((WindowPtr)w)->portBits.bounds.top));
	OffsetRgn(t.zoneGlobal, mypt.h, mypt.v);
	
	if ((err = PtrAndHand(&t, (Handle)Zones(w), sizeof(RgnBalloonCurs))) == noErr)
		OneMoreObject(w);
	}
return err;
}

/* Recalculate the mouse region; you pass:
�	a WindowPtr (not necessarily a FabWindowPtr);
�	the mouse position (in global coordinates);
*/

void RecalcMouseRegion(WindowPtr w, Point mouse)
{
register RgnBalloonCursPtr	curObj;
register unsigned long	i;

if ((IsFabWindow(w)) && (NumObjects(w))) {
	CopyRgn(wideOpenRgn, mouseRgn);
	HLockHi((Handle)Zones(w));
	for (i = 1, curObj = *Zones(w); i <= NumObjects(w); i++, curObj++) {
		if (PtInRgn(mouse, curObj->zoneGlobal)) {
			if (curObj->curs)
				SetCursor(*curObj->curs);
			else
				SetCursor(&qd.arrow);
			if (curObj->myBalloon)
				if (HMGetBalloons())
					myShowBalloon(curObj->zoneGlobal, curObj->myBalloon,
									curObj->balloonVariant);
			CopyRgn(curObj->zoneGlobal, mouseRgn);
			break;
			}
		else
			DiffRgn(mouseRgn, curObj->zoneGlobal, mouseRgn);
		}
	if (i > NumObjects(w))
		SetCursor(&qd.arrow);
	HUnlock((Handle)Zones(w));
	}
else {
	CopyRgn(wideOpenRgn, mouseRgn);
	SetCursor(&qd.arrow);
	}
}

/* Resizes the dynamic objects of the window by calling the recalcProc
for every object */

void ResizeObjects(FabWindowPtr w)
{
register void (*theProc)(FabWindowPtr, RgnBalloonCursPtr);
register RgnBalloonCursPtr	curObj;
register unsigned long	i;

if (NumObjects(w)) {
	HLockHi((Handle)Zones(w));
	for (i = 1, curObj = *Zones(w); i <= NumObjects(w); i++, curObj++) {
		theProc = curObj->recalcRgnProc;
		if (theProc)
			theProc(w, curObj);
		}
	HUnlock((Handle)Zones(w));
	}
}

/* Calculates the regions for the given FabWindow in global coordinates,
so that they are ready for WaitNextEvent
*/

void RecalcGlobalCoords(FabWindowPtr w)
{
register RgnBalloonCursPtr	curObj;
register unsigned long	i;
Point	mypt = { 0, 0};

if (NumObjects(w)) {
	HLockHi((Handle)Zones(w));
	LocalToGlobal(&mypt);
	for (i = 1, curObj = *Zones(w); i <= NumObjects(w); i++, curObj++) {
		CopyRgn(curObj->zoneLocal, curObj->zoneGlobal);
// -(((WindowPtr)w)->portBits.bounds.left)
// -(((WindowPtr)w)->portBits.bounds.top)
		OffsetRgn(curObj->zoneGlobal, mypt.h, mypt.v);
		}
	HUnlock((Handle)Zones(w));
	}
}

/* myShowBalloon: common routine for showing up balloons */

static void myShowBalloon(RgnHandle theGlobalCoordsRgn,
							unsigned long theBalloon, short variant)
{
HMMessageRecord	helpMsg;
Point	myTip;

myTip.v = (*theGlobalCoordsRgn)->rgnBBox.top + 3;
myTip.h = (*theGlobalCoordsRgn)->rgnBBox.right - 3;
helpMsg.hmmHelpType = khmmStringRes;
*(unsigned long *)&helpMsg.u.hmmStringRes.hmmResID = theBalloon;
//helpMsg.u.hmmStringRes.hmmIndex = *(((short *)(&theBalloon)) +1);
if (hmBalloonAborted == HMShowBalloon(&helpMsg, myTip, &(*theGlobalCoordsRgn)->rgnBBox,
										nil, 0, variant, kHMRegularWindow))
	ForceMouseMovedEvent();
}

void DisposFabWindow(FabWindowPtr w)
{
register RgnBalloonCursHandle	tempH;
register RgnBalloonCursPtr	spanPtr;
register unsigned long	i;

tempH = Zones(w);
HLock((Handle)tempH);
spanPtr = *tempH;

for (i = 0; i < NumObjects(w); i++, spanPtr++) {
	DisposeRgn(spanPtr->zoneLocal);
	DisposeRgn(spanPtr->zoneGlobal);
/* we do nothing with the CursHandles since they might be system cursors */
	}
DisposeHandle((Handle)tempH);
// the caller knows whether CloseWindow or CloseDialog is to be called
}

void ForceMouseMovedEvent(void)
{
SetRectRgn(mouseRgn, 32761, 32761, 32765, 32765);
}

void InitFabWindow(FabWindowPtr w)
{
// not needed because we allocate the structure with NewPtrClear
/*
SetActivate(w, nil);
SetUpdate(w, nil);
SetDrag(w, nil);
SetGrow(w, nil);
SetZoom(w, nil);
SetGoAway(w, nil);
SetContent(w, nil);
NumObjects(w) = 0;
*/
Zones(w) = (RgnBalloonCursHandle)NewHandle(0);
if (((WindowPeek)w)->windowKind != dialogKind)
	((WindowPeek)w)->windowKind = kFabWindowClass;

}

