#include	"UtilsSys7.h"
#include	"CursorBalloon.h"
#include	"Independents.h"
#include	"MovableModal.h"

static void RecalcTextItems(FabWindowPtr w, RgnBalloonCursPtr theObj);
static void DoMovableModalUpdate(DialogRef dPtr);
static void DimOrBlackenItems(EventRecord *e, DialogRef w, Boolean active);
static void DoMovableModalMenuEdit(short theItem);
static void DoMovableModalDrag(WindowRef w, Point p);
static void getDragRectMovMod(WindowRef w, RectPtr r);
static TEHandle GetTEIfVisible(DialogRef dPtr, short editTextExists);

static Handle	sControlItemList = nil;
static short	sDefaultItem = 0, sCancelItem = 0;

#define	FabSetDialogDefaultItem(d, i)	sDefaultItem = i
#define	FabSetDialogCancelItem(d, i)	sCancelItem = i

short HandleMovableModalDialog(/*ModalFilterProcPtr filterProc,*/
					dialogItemsPtr things,
					Point *dialogTopLeft,
					Boolean (*canAcceptDrag)(const HFSFlavor *),
					OSErr (*dialogDragReaction)(WindowPtr, const HFSFlavor *),
					void (*initProc)(DialogPtr),
					void (*cleanupProc)(DialogPtr),
					void (*userProc)(DialogPtr, Handle, short),
					void (*AdjustMenus)(TEHandle),
					void (*Handle_My_Menu)(long),
					void (*DomyKeyEvent)(EventRecord *),
					Boolean (*PreProcessKeyDown)(EventRecord *, DialogPtr),
					void (*PostProcessKeyDown)(EventRecord *, DialogPtr),
					void (*DoUpdate)(EventRecord *),
					void (*DoActivate)(EventRecord *),
					void (*DoHiLevEvent)(EventRecord *),
					void (*DoOSEvent)(EventRecord *),
					void (*DoIdle)(void),
					unsigned long minimumSleep,
					short resId)
{
EventRecord	lMyEvent;
Rect	box;
Handle	item;
GrafPtr	port;
long	myRefCon;
DialogRef	dPtr;
DialogRef	dSelPtr;
WindowRef	w;
Ptr	myStorage;
unsigned long	movablemodalSleep;
dialogItemsPtr	spanPtr;
dialogItemsPtr	itemToBeActivated;
RgnHandle	theRgn;
long	menuSelected;
short	wMenu;
short	code, type, theItemHit = memFullErr;
short	theGroup, theFakeItem, lastItemClosingDialog = cancel;
short	theType, iNum;
short	editTextExists = 0;
unsigned char	keypressed;
Boolean	dialoging;

InitCursor();
myStorage = NewPtrClear(sizeof(FabWindowRecord));
if (myStorage) {
	dPtr = GetNewDialog(resId, myStorage, (WindowRef)-1L);
	if (dPtr) {
		GetPort(&port);
		SetGrafPortOfDialog(dPtr);
	
		InitFabWindow((FabWindowPtr)dPtr);
		SetUpdate(dPtr, DoMovableModalUpdate);
		SetActivate(dPtr, DimOrBlackenItems);
		SetGetDragRect(dPtr, getDragRectMovMod);
		SetCanIAcceptDrag(dPtr, canAcceptDrag);
		SetDragReaction(dPtr, dialogDragReaction);
		
		if (dialogTopLeft) {
			if (IsOnScreenWeak(*dialogTopLeft))
				MoveWindow(GetDialogWindow(dPtr), dialogTopLeft->h, dialogTopLeft->v, false);
			}

		sDefaultItem = 0;
		sCancelItem = 0;
		
		spanPtr = things;
		sControlItemList = NewHandle(0);
		if (sControlItemList) {
			while (iNum = spanPtr->itemNumber) {
				myRefCon = spanPtr->refCon;
				GetDialogItem(dPtr, iNum, &type, &item, &box);
				theType = type & itemDisable ? type - itemDisable : type;
				switch(theType) {
					case ctrlItem+btnCtrl:
							{
							if (myRefCon == 1L)
								FabSetDialogDefaultItem(dPtr, iNum)
								;
							else if (myRefCon == 2L)
								lastItemClosingDialog = iNum;
							if (iNum == cancel)
								FabSetDialogCancelItem(dPtr, iNum)
								;
							}
						SetControlReference((ControlHandle)item, (*(ControlHandle)item)->contrlHilite);
						PtrAndHand(&item, sControlItemList, sizeof item);
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
						SetControlReference((ControlHandle)item, (*(ControlHandle)item)->contrlHilite);
						PtrAndHand(&item, sControlItemList, sizeof item);
						break;
					case ctrlItem+resCtrl:
						if (myRefCon)
							SetControlValue((ControlHandle)item, myRefCon);
						SetControlReference((ControlHandle)item, (*(ControlHandle)item)->contrlHilite);
						PtrAndHand(&item, sControlItemList, sizeof item);
						break;
					case statText:
						if (myRefCon)
							SetDialogItemText(item, (StringPtr)myRefCon);
						break;
					case editText:
						SetDialogItemText(item, (StringPtr)myRefCon);
						if(editTextExists == 0) {
							editTextExists = iNum;
							}
						theRgn = NewRgn();
						RectRgn(theRgn, &box);
						InstallRgnHandler((FabWindowPtr)dPtr, theRgn, RecalcTextItems, GetCursor(iBeamCursor),
									0, 0, iNum);
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
			item = nil;
			PtrAndHand(&item, sControlItemList, sizeof item);
			if (editTextExists) {
				ResizeObjects((FabWindowPtr)dPtr);
				RecalcGlobalCoords((FabWindowPtr)dPtr);
				(void) EventAvail(0, &lMyEvent);
				RecalcMouseRegion(dPtr, lMyEvent.where);
				movablemodalSleep = GetCaretTime();
				}
			else
				movablemodalSleep = -1L;
			if (initProc)
				initProc(dPtr);
//			(void)TEFeatureFlag(teFOutlineHilite, TEBitSet, ((DialogPeek)dPtr)->textH);
			SelectDialogItemText(dPtr, editTextExists, 0, SHRT_MAX);
			ShowWindow(GetDialogWindow(dPtr));
		
			dialoging = true;
			HiliteMenu(0);
			AdjustMenus(GetTEIfVisible(dPtr, editTextExists));
			DrawMenuBar();

			movablemodalSleep = MIN(minimumSleep, movablemodalSleep);
			do {
				if (WaitNextEvent(everyEvent - diskMask, &lMyEvent, movablemodalSleep, mouseRgn)) {
					if (IsDialogEvent(&lMyEvent)) {
						theFakeItem = 0;
						switch (lMyEvent.what) {
							//case mouseDown :
							//	DebugStr("\pMouse Down!");
							//	break;
							case keyDown:
							case autoKey:
								keypressed = CHARFROMMESSAGE(lMyEvent.message);
								if (PreProcessKeyDown ? PreProcessKeyDown(&lMyEvent, dPtr) : true) {
									if (keypressed == 3) {
										theFakeItem = sDefaultItem;
										if (theFakeItem == 0)
											theFakeItem = ok;
										}
									else if (keypressed == 13 && sDefaultItem) {
										theFakeItem = sDefaultItem;
										}
									else if (CmdPeriodOrEsc(&lMyEvent)) {
										theFakeItem = sCancelItem;
										}
									else if (keypressed == 27) { // clear key
										//SelIText(dPtr, ((DialogPeek)dPtr)->editField + 1, 0, SHRT_MAX);
										DialogDelete(dPtr);
										}
									else if ((unsigned short)lMyEvent.message == kHelpKey) {
										(void)HMSetBalloons(1 - HMGetBalloons());
										//RecalcMouseRegion(dPtr, lMyEvent.where);
										}
									else if (lMyEvent.modifiers & cmdKey) {
										AdjustMenus(GetTEIfVisible(dPtr, editTextExists));
										menuSelected = MenuKey(keypressed);
										if ((wMenu = HiWord(menuSelected)) && wMenu == kRes_Menu_Edit) {
											DoMovableModalMenuEdit(LoWord(menuSelected));
											HiliteMenu(0);
											}
										else
											Handle_My_Menu(menuSelected);
										}
									else
										(void) DialogSelect(&lMyEvent, &dSelPtr, &theItemHit);
									if (theFakeItem)
										FlashButton(dPtr, theFakeItem);
									if (PostProcessKeyDown)
										PostProcessKeyDown(&lMyEvent, dPtr);
									}
								break;
							case updateEvt:
								if ((DialogPtr)lMyEvent.message == dPtr) {
									BeginUpdate(GetDialogWindow(dPtr));
									DoMovableModalUpdate(dPtr);
									EndUpdate(GetDialogWindow(dPtr));
									}
								else
									DoUpdate(&lMyEvent);
								break;
							case activateEvt :
								DoActivate(&lMyEvent);
								break;
							case kHighLevelEvent:
								DoHiLevEvent(&lMyEvent);
								break;
							case osEvt :
								DoOSEvent(&lMyEvent);
								break;
							default:
								if (DialogSelect(&lMyEvent, &dSelPtr, &theItemHit)) {
									if (dSelPtr == dPtr && theItemHit) {
										theFakeItem = theItemHit;
										}
									}
							}
						if (theFakeItem) {
							GetDialogItem(dPtr, theFakeItem, &type, &item, &box);
							switch (type) {
								case ctrlItem+btnCtrl:
									if ((theFakeItem >= ok) && (theFakeItem <= lastItemClosingDialog))
										dialoging = false;
									else if (userProc)
										userProc(dPtr, item, theFakeItem);
									break;
								case ctrlItem+chkCtrl:
									SetControlValue((ControlHandle)item, 1 - GetControlValue((ControlHandle)item));
									if (userProc)
										userProc(dPtr, item, theFakeItem);
									break;
								case ctrlItem+radCtrl:
									if (GetControlValue((ControlHandle)item) == 0) {
										SetControlValue((ControlHandle)item, 1);
										for (spanPtr = things; (++spanPtr)->itemNumber != theFakeItem; )
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
											userProc(dPtr, item, theFakeItem);
										}
									break;
								case ctrlItem+resCtrl:
								case editText:
								case userItem:
									if (userProc)
										userProc(dPtr, item, theFakeItem);
									break;
								}
							}
						}
					else switch (lMyEvent.what) {
						case mouseDown :
							code = FindWindow(lMyEvent.where, &w);
							switch (code) {
								case inMenuBar :
									AdjustMenus(GetTEIfVisible(dPtr, editTextExists));
									menuSelected = MenuSelect(lMyEvent.where);
									if ((wMenu = HiWord(menuSelected)) && wMenu == kRes_Menu_Edit) {
										DoMovableModalMenuEdit(LoWord(menuSelected));
										HiliteMenu(0);
										}
									else
										Handle_My_Menu(menuSelected);
									break;
								case inSysWindow :
									SystemClick(&lMyEvent, w);
									AdjustMenus(nil);
									break;
								case inContent :
									SysBeep(30);
									break;
								case inDrag :
									DoMovableModalDrag(w, lMyEvent.where);
									break;
								case inGoAway :
									if (TrackGoAway(w, lMyEvent.where)) {
			
										}
									break;
		/*						case inZoomIn :
								case inZoomOut :
									break;
		*/						}
							break;
						case keyDown:
						case autoKey:
							DomyKeyEvent(&lMyEvent);
							break;
						case updateEvt :
							DoUpdate(&lMyEvent);
							break;
						case activateEvt :
							DoActivate(&lMyEvent);
							break;
						case kHighLevelEvent:
							DoHiLevEvent(&lMyEvent);
							break;
						case osEvt :
							DoOSEvent(&lMyEvent);
							break;
						}
					}
				else {
					if (IsDialogEvent(&lMyEvent))
						(void) DialogSelect(&lMyEvent, &dSelPtr, &theItemHit);
					DoIdle();
					}
				}
			while (dialoging);
		
			if (theFakeItem == ok) {
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
			DisposeHandle(sControlItemList);
			sControlItemList = nil;
			if (cleanupProc)
				cleanupProc(dPtr);
			}
		if (dialogTopLeft) {
			*dialogTopLeft = topLeft(GetWindowPort(GetDialogWindow(dPtr))->portRect);
			LocalToGlobal(dialogTopLeft);
			}
		DisposFabWindow((FabWindowPtr)dPtr);
		CloseDialog(dPtr);
		SetPort(port);
		AdjustMenus(nil);
		DrawMenuBar();
		}
	else
		SysBeep(30);
	DisposePtr(myStorage);
	}
else
	SysBeep(30);
InitCursor();
return theFakeItem;
}

void RecalcTextItems(FabWindowPtr w, RgnBalloonCursPtr theObj)
{
Rect	tempRect;
Handle	item;
short	type;

GetDialogItem((DialogPtr)w, theObj->itemID, &type, &item, &tempRect);
RectRgn(theObj->zoneLocal, &tempRect);
}

void DoMovableModalUpdate(DialogRef dPtr)
{
UpdateDialog(dPtr, dPtr->visRgn);
if (sDefaultItem)
	OutlineButton(dPtr, sDefaultItem);
}

void DimOrBlackenItems(EventRecord *passEvt, DialogRef w, Boolean active)
{
ControlHandle	itemHandle;
ControlHandle	*scanPtr;
DialogPtr	theDialog;
short	theItemHit;
unsigned char	hiliting;

if (sControlItemList) {
	HLockHi(sControlItemList);
	scanPtr = (ControlHandle *)*sControlItemList;
	while (itemHandle = *scanPtr++) {
		if (active)
			hiliting = GetControlReference(itemHandle);
		else {
			SetControlReference(itemHandle, (*itemHandle)->contrlHilite);
			hiliting = 255;
			}
		if (hiliting != (*itemHandle)->contrlHilite) {
			HiliteControl(itemHandle, hiliting);
			ValidRect(&(*itemHandle)->contrlRect);
			}
		}
	HUnlock(sControlItemList);
	}
if (sDefaultItem)
	OutlineButton(w, sDefaultItem);
(void) DialogSelect(passEvt, &theDialog, &theItemHit);
}

void DoMovableModalMenuEdit(short theItem)
{
DialogRef	dd;

if (SystemEdit(theItem - 1) == false) {
	dd = FrontWindow();
	if (dd && isMovableModal(dd))
		switch (theItem) {
//		case kMItem_Undo:
//			break;
			case kMItem_Cut:
				DialogCut(dd);
				ZeroScrap();
				(void) TEToScrap();
//				PostEvent(keyDown, kCutKey);
				break;
			case kMItem_Copy:
				DialogCopy(dd);
				ZeroScrap();
				(void) TEToScrap();
//				PostEvent(keyDown, kCopyKey);
				break;
			case kMItem_Paste:
				(void) TEFromScrap();
				DialogPaste(dd);
//				PostEvent(keyDown, kPasteKey);
				break;
			case kMItem_Clear:
				DialogDelete(dd);
				break;
			case kMItem_Select_All:
				SelectDialogItemText(dd, ((DialogPeek)dd)->editField + 1, 0, SHRT_MAX);
				break;
			}
	}
}

void DoMovableModalDrag(WindowRef w, Point p)
{
EventRecord	tempEvt;
GrafPtr		savePort;
WindowRef	f;
register void (*theProc)(WindowPtr);
//register RectPtr	wStateP;

f = FrontWindow();
if (w != f && isMovableModal(f))
	SysBeep(30);
else {
	DragWindow(w, p, &qd.screenBits.bounds);
	GetPort(&savePort);
	SetPortWindowPort(w);
	if (IsFabWindow(w)) {
		theProc = ((FabWindowPtr)w)->dragProc;
		if (theProc)
			theProc(w);
		RecalcGlobalCoords((FabWindowPtr)w);
		}
	(void) EventAvail(0, &tempEvt);
	RecalcMouseRegion(w, tempEvt.where);
	SetPort(savePort);
	}
}

void getDragRectMovMod(WindowRef w, RectPtr r)
{
*r = GetWindowPort(w)->portRect;
}

void FabHiliteControl(ControlRef theControl, short hiliteState)
{
HiliteControl(theControl, hiliteState);
SetControlReference(theControl, (*theControl)->contrlHilite);
}

TEHandle GetTEIfVisible(DialogPtr dPtr, short editTextExists)
{
Rect	tempRect, resultR;
Handle	item;
short	type;
TEHandle	retTE = nil;

if (editTextExists && GetDialogKeyboardFocusItem(dPtr) > 0) {
	GetDialogItem(dPtr, GetDialogKeyboardFocusItem(dPtr), &type, &item, &tempRect);
	if (SectRect(&tempRect, &GetWindowPort(GetDialogWindow(dPtr))->portRect, &resultR))
		retTE = ((DialogPeek)dPtr)->textH;
	}
return retTE;
}

void EnableCutCopyPaste(MenuRef editMenu, TEHandle activeTE)
{
long	offset;

if (activeTE) {
	EnableItem(editMenu, kMItem_Select_All);
	if ( (*activeTE)->selStart < (*activeTE)->selEnd ) {
		EnableItem(editMenu, kMItem_Cut);
		EnableItem(editMenu, kMItem_Copy);
		EnableItem(editMenu, kMItem_Clear);
		}
	else {
		DisableItem(editMenu, kMItem_Cut);
		DisableItem(editMenu, kMItem_Copy);
		DisableItem(editMenu, kMItem_Clear);
		}
	if (GetScrap(nil, 'TEXT', &offset)  > 0)
		EnableItem(editMenu, kMItem_Paste);
	else
		DisableItem(editMenu, kMItem_Paste);
	}
else {
	DisableCutCopyPaste(editMenu);
	}
}

void DisableCutCopyPaste(MenuRef editMenu)
{
short	i;

DisableItem(editMenu, kMItem_Undo);
for (i = kMItem_Cut; i <= kMItem_Select_All; i++)
	DisableItem(editMenu, i);
}

