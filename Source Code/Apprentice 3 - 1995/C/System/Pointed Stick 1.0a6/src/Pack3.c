#include "Global.h"
#include "Pack3.h"
#include "CEToolbox.h"			// CETurnOnOff
#include "Conversions.h"		// HandleToReply
#include "GetItemList.h"		// StartLookingForList
#include "MenubarCDEF.h"		// ZMenuKey
#include "MenuSetup.h"			// For GetNthOpenableFileObject()
#include "Munge Dialog.h"		// MungeNextDialog
#include "OtherMenu.h"			// gStringToFind
#include "PLStringFuncs.h"		// PStringCopy
#include "Utilities.h"			// GetVersionString, MyOpenResFile

#include <StdDef.h>				// offsetof() macro


enum {kIdle, kSelectName, kSelectOpen, kBuildMenus};

UniversalProcPtr	gOldStandardFile;
short				gStandardFileMode = kNotFaking;
FileObjectHdl		gFileToReturn;
Boolean				gScarfedItYet;
short				gRefNum = -1;
short				gSelector;

Boolean				pCloseIt;
short				pOldValue;

static long			pSavedA4;				// static because also in InitGraf.c
static long			pSavedReturnAddress;	// static because also in InitGraf.c

long				pParameters;
short				pOrigSelector;
ProcPtr				pDlgHook;
ProcPtr				pFileFilter;
ProcPtr				pModalFilter;

short				pCurVRefNum;
long				pCurDirID;
short				pSelectedCellTheLastTimeILooked;

Boolean				pSputchNextScroll = FALSE;

const Point			pZeroPoint = {0, 0};

short				pIdleState = kIdle;
Str63				pNameToFindLater;				// ¥¥¥ Allocate dynamically
Boolean				pOpenFileAfterFind = FALSE;

short				gOffsetToReply[] = {
						offsetof(SFPutFileParameters, reply),
						offsetof(SFGetFileParameters, reply),
						offsetof(SFPPutFileParameters, reply),
						offsetof(SFPGetFileParameters, reply),
						offsetof(StandardPutFileParameters, reply),
						offsetof(StandardGetFileParameters, reply),
						offsetof(CustomPutFileParameters, reply),
						offsetof(CustomGetFileParameters, reply)
					};

short				pOffsetToDlgHook[] = {
						offsetof(SFPutFileParameters, dlgHook),
						offsetof(SFGetFileParameters, dlgHook),
						offsetof(SFPPutFileParameters, dlgHook),
						offsetof(SFPGetFileParameters, dlgHook),
						-1,
						-1,
						offsetof(CustomPutFileParameters, dlgHook),
						offsetof(CustomGetFileParameters, dlgHook)
					};

short				pOffsetToFileFilter[] = {
						-1,
						offsetof(SFGetFileParameters, fileFilter),
						-1,
						offsetof(SFPGetFileParameters, fileFilter),
						-1,
						offsetof(StandardGetFileParameters, fileFilter),
						-1,
						offsetof(CustomGetFileParameters, fileFilter)
					};

short				pOffsetToNumTypes[] = {
						-1,
						offsetof(SFGetFileParameters, numTypes),
						-1,
						offsetof(SFPGetFileParameters, numTypes),
						-1,
						offsetof(StandardGetFileParameters, numTypes),
						-1,
						offsetof(CustomGetFileParameters, numTypes)
					};

short				pOffsetToTypeList[] = {
						-1,
						offsetof(SFGetFileParameters, typeList),
						-1,
						offsetof(SFPGetFileParameters, typeList),
						-1,
						offsetof(StandardGetFileParameters, typeList),
						-1,
						offsetof(CustomGetFileParameters, typeList)
					};

short				pOffsetToModalFilter[] = {
						-1,
						-1,
						offsetof(SFPPutFileParameters, filterProc),
						offsetof(SFPGetFileParameters, filterProc),
						-1,
						-1,
						offsetof(CustomPutFileParameters, filterProc),
						offsetof(CustomGetFileParameters, filterProc)
					};

short				pOffsetToYD[] = {
						-1,
						-1,
						-1,
						-1,
						-1,
						-1,
						offsetof(CustomPutFileParameters, yourDataPtr),
						offsetof(CustomGetFileParameters, yourDataPtr)
					};

short				pParameterSizes[] = {
						sizeof(SFPutFileParameters),
						sizeof(SFGetFileParameters),
						sizeof(SFPPutFileParameters),
						sizeof(SFPGetFileParameters),
						sizeof(StandardPutFileParameters),
						sizeof(StandardGetFileParameters),
						sizeof(CustomPutFileParameters),
						sizeof(CustomGetFileParameters)
					};

short				pDialogIDs[] = {
						putDlgID,
						getDlgID,
						-1,					// These guys come with their own
						-1,
						sfPutDialogID,
						sfGetDialogID,
						-1,
						-1,
					};

void				FakePack3(short selector);

void				DialogHookEntry(void);
pascal short		DlgHook(short item, DialogPtr theDialog);
pascal short		DlgHookYD(short item, DialogPtr theDialog, Ptr myDataPtr);
Boolean				CommonDialogHook(short item, DialogPtr theDialog, short* newItem);

void				FileFilterEntry(void);
pascal Boolean		FileFilter(ParmBlkPtr PB);
pascal Boolean		FileFilterYD(ParmBlkPtr PB, void *yourDataPtr);

void				ModalFilterEntry(void);
pascal Boolean		ModalFilter(DialogPtr theDialog, EventRecord *theEvent,
								short *itemHit);
pascal Boolean		ModalFilterYD(DialogPtr theDialog, EventRecord *theEvent,
								short *itemHit, void *yourDataPtr);
Boolean				CommonModalFilter(DialogPtr theDialog, EventRecord *theEvent,
								short *itemHit);

Boolean				HandleIdleEvent(short item, DialogPtr theDialog, short* newItem);
void				GetItemIndex(StringPtr stringToFind, short *index);
pascal short		TestSFEntry(Ptr cellPtr, Ptr testPtr, short cellLen, short testLen);
void				SelectFileInList(ListHandle theList, short itemIndex);
short				HandleMenuSelection(DialogPtr dlg, short whichMenu, short menuAndItem);

short				GoToDirectoryItem(short);
short				GoToFileItem(short);
void				ReFilterMenus(DialogPtr theDialog);
short				ListItem(void);

void				RememberReply(void);

void*				GetReplyPtr(void);
short				GetNumTypes(void);
OSType*				GetTypesPtr(void);
void*				GetFilterProcPtr(void);
void*				GetYDPtr(void);


//--------------------------------------------------------------------------------
/*
	This is the entry point to most of what Pointed Stick does. ItÕs pretty
	gnarly, so IÕd better get this down while I can still figure it out.
	
	The idea behind this patch to Standard File is to do two things:

		¥ Make sure everything is OK for me to hook in, and
		¥ Hook me in.
	
	The first thing we do is set up our A4, strip off the return address
	and save it for later, and take a look at the selector. If the selector
	is not one that we know (from 1 to 8, inclusive), we just call the
	standard Standard File without any funny stuff. ItÕs called just as
	if we didnÕt exist (except that we are tail-patched, here).
	
	Next, we try to open our INITÕs resource fork. WeÕll need a lot of
	things from it, like MENUs, ALRTs, etc. We open the resource fork
	using MyOpenResFile, which opens the resource fork, shuffles the
	resource chain so that my map appears just before the System resource
	map and -- presumably -- after the applicationÕs. If we canÕt open
	our resource fork, we blow out in the same way as if the selector
	was out of range.
	
	At this point, we do one of two things. If the user had selected a
	file from the hierarchical menu we attached to the Open menu item,
	we are in the middle of puppet-stringing a response. If this is the
	case, we call FakePack3, which fills in the reply record with something
	appropriate, and then return to the caller just as if Standard File
	had really interacted with the user. This is identical in concept
	with what MultiFinder does with non-Apple Event aware applications.
	
	If we arenÕt puppet-stringing, then itÕs time for us to hook ourselves
	in to Standard File. This consists of two phases. First, we can only
	hook ourselves in properly if the caller makes one of those ÒSFPÓ or
	ÒCustomÓ calls. If they didnÕt, the first thing we do is convert
	the parameters on the stack and change the selector so it looks to
	Standard File like the caller _did_ make a custom call. Second, we
	replace the dlgHook, filterProc, and modalFilter parameters with
	pointers to our own procedures, saving the callerÕs so that we can
	chain to them when necessary.
	
	Now weÕre almost ready to fall into Standard File. ThereÕre only
	a few things we still have to do. First, we put ourself into
	a state that will cause us to munge the next dialog created so that
	it has the menubar across the top. Second, we put ourself into a
	state that will start the sequence of events that will find the
	handle to the ListRecord holding the files and directories appearing
	in the dialog (see the comments in GetItemList.c to see how this
	happens). Third, we clear out the variables holding the previous
	volume and directory we were in. These values are used for rebounding.
	We set them to illegal values to force the rebound routines to
	select the initial file. Finally, we turn off CEToolbox hotkeys so
	that the user canÕt bring up MiniFinder while weÕre in Standard
	File. If the user did that, Standard File would re-enter itself,
	which is a Bad Thing.
	
	Finally, we chain to the original Standard File entry point. At
	this point, the dialog appears and the user interacts with it. When
	the user closes the dialog, it goes away, and Standard File returns
	to us. All we have to do now is turn the CEToolbox hotkeys back on
	and close our resource file. Once thatÕs done, we return to the
	caller.
*/

pascal void MyPack3()
{
	#ifdef BREAK_ON_ENTRY
	DebugStr("\pBreaking on MyPack3");
	#endif

	SetUpA4();
	asm {
		move.l	(sp)+,pSavedA4
		move.l	(sp)+,pSavedReturnAddress
		move.w	(sp),D0
		beq		@NoFunnyStuff				; skip if selector is zero
		cmp.w	#customGetFile,D0
		bhi		@NoFunnyStuff				; skip if selector is too large
		bsr		ControlKeyIsDown
		tst.b	D0
		bne		@NoFunnyStuff				; skip if control key is down
		
		pea		pCloseIt
		move.w	#fsRdPerm,-(sp)				; push access priv for our resource file
		move.l	gMe,A0						; push name for our resource file
		move.l	A0,-(sp)
		jsr		MyOpenResFile				; try to open it
		add		#4+4+2,A7					; remove my parameters
		moveq	#-1,D1						; see if we failed (check refNum == -1)
		cmp.w	D1,D0
		beq		@NoFunnyStuff				; if failed, skip special effects
		move.w	D0,gRefNum					; save refnum so we can close it later
		
		move.w	gStandardFileMode,D0
		beq.s	@HookInMyProcs

		bsr		FakePack3					; do special effects
		
		lea		pParameterSizes,A0			; pull off StdFile parameters. Get size
		move.w	(sp),D0						; ...of parameters from table
		add.w	D0,D0
		move.w	-2(A0,D0.W),D0
		add.w	D0,A7						; add size to stack pointer

		bra		@CloseUp
	
	@HookInMyProcs
		move.w	(sp)+,D0					; put selector in D0
		move.w	D0,pOrigSelector			; save what the user passed in
		sub.w	#1,D0						; make it zero based
		btst	#1,D0						; is this a Custom or P routine?
		bne.s	@NoConversionNeeded			; yes, so donÕt convert it

		move.w	D0,D1						; use D1 for table index
		add.w	D1,D1						; double for 2 byte table entries

		lea		pDialogIDs,A0
		move.w	(A0,D1.W),-(sp)				; push on our dialog ID

		btst	#2,D0						; is this an old style routine?
		beq.s	@HookInMyProcsOldStyle		; yes, so donÕt push mondo parms

		move.l	pZeroPoint,-(sp)			; location
		clr.l	-(sp)						; dialog hook
		clr.l	-(sp)						; modal dialog filter
		clr.l	-(sp)						; active list
		clr.l	-(sp)						; activate proc

	@HookInMyProcsOldStyle
		clr.l	-(sp)						; your data / modal dialog filter
		add.w	#2,D0						; convert selector to Custom/P routine

	@NoConversionNeeded
		move.w	D0,D1						; use D1 for table index
		add.w	D1,D1						; double for 2 byte table entries

		add.w	#1,D0						; make selector 1-base again
		move.w	D0,-(sp)					; put the possibly modified selector back

		move.w	D0,gSelector				; remember selector

		move.l	A7,pParameters				; remember pointer to all parameters
											; (including the selector)

		lea		pOffsetToDlgHook,A0			; remember dlgHook
		move.w	(A0,D1.W),D0
		move.l	(A7,D0.W),pDlgHook
		lea		DialogHookEntry,A0			; put in my own
		move.l	A0,(A7,D0.W)

		lea		pOffsetToFileFilter,A0		; remember fileFilter
		move.w	(A0,D1.W),D0
		bmi.s	@DontSwapFileFilter			; but only if itÕs got one
		move.l	(A7,D0.W),pFileFilter
		tst.l	pFileFilter
		beq.s	@DontSwapFileFilter			; donÕt supply one if he didnÕt
		lea		FileFilterEntry,A0			; put in my own
		move.l	A0,(A7,D0.W)

	@DontSwapFileFilter
		lea		pOffsetToModalFilter,A0		; remember modalFilter
		move.w	(A0,D1.W),D0
		move.l	(A7,D0.W),pModalFilter
		lea		ModalFilterEntry,A0			; put in my own
		move.l	A0,(A7,D0.W)

		jsr		MungeNextDialog
		jsr		StartLookingForList
		clr.w	pCurVRefNum(A4)
		clr.l	pCurDirID(A4)
		move.w	#-1,pSelectedCellTheLastTimeILooked(A4)

		pea		pOldValue					; Turn off hotkeys so we donÕt
		move.w	#-1,-(sp)					; re-enter ourselves by bringing
		jsr		CETurnOnOff					; up the MiniFinder.
		add.w	#6,sp

		move.l	gOldStandardFile,A0
		jsr		(A0)

		pea		pOldValue
		move.w	pOldValue,-(sp)
		jsr		CETurnOnOff
		add.w	#6,sp

	@CloseUp
		tst.b	pCloseIt
		beq.s	@continue

		move.w	gRefNum,-(sp)				; close our resource file
		CloseResFile
		moveq	#-1,D1
		move.w	D1,gRefNum

		bra.s	@continue

	@NoFunnyStuff
		move.l	gOldStandardFile,A0			; here if we are to call old StdFile with
		jsr		(A0)						; ...no tricks.

	@continue
		move.l	pSavedReturnAddress,-(sp)
		move.l	pSavedA4,A4
	}
}


//--------------------------------------------------------------------------------
/*
	This function is called if gStandardFileMode is non-zero. A non-zero
	value means that the standard file dialog must be avoided, and that we
	should simply fill in the reply record with the contents of gFileToReturn
	and return. On the way out, we clear out gStandardFileMode.
*/

void	FakePack3(short selector)
{
	if (IsGetFile(selector)) {
		HandleToReply(gFileToReturn,
				*(void**) ((char*) &selector + gOffsetToReply[selector-1]));
	} else {
		ReportError(kPutOnPuppet, 0);
	}
	gStandardFileMode = kNotFaking;
}


//--------------------------------------------------------------------------------

void	DialogHookEntry(void)
{
	SetUpA4();
	asm {
		cmp.w	#standardPutFile,gSelector
		move.l	(sp)+,A4
		blt.s	@oldHook
		bra		DlgHookYD

	@oldHook
		bra		DlgHook
	}
}


//--------------------------------------------------------------------------------

pascal short	DlgHook(short item, DialogPtr theDialog)
{
	Boolean			result;
	short			newItem;

	SetUpA4();
	newItem = item;
	result = CommonDialogHook(item, theDialog, &newItem);
	if (!result && (pDlgHook != NIL))
		newItem = ((DlgHookProcPtr) pDlgHook)(newItem, theDialog);
	
	if (newItem == sfHookRebuildList) {
		ReFilterMenus(theDialog);
	}
	
	if (newItem == sfItemOpenButton || newItem == sfItemCancelButton)
		gScarfedItYet = TRUE;

	RestoreA4();
	return newItem;
}


//--------------------------------------------------------------------------------

pascal short	DlgHookYD(short item, DialogPtr theDialog, Ptr myDataPtr)
{
	Boolean			result;
	short			newItem;

	SetUpA4();
	newItem = item;
	result = CommonDialogHook(item, theDialog, &newItem);
	if (!result && (pDlgHook != NIL))
		newItem = ((DlgHookYDProcPtr) pDlgHook)(newItem, theDialog, myDataPtr);

	if (newItem == sfHookRebuildList) {
		ReFilterMenus(theDialog);
	}

	if (newItem == sfItemOpenButton || newItem == sfItemCancelButton)
		gScarfedItYet = TRUE;

	RestoreA4();
	return newItem;
}


//--------------------------------------------------------------------------------

Boolean	CommonDialogHook(short item, DialogPtr theDialog, short* newItem)
{
	Boolean			handledIt;
	short			menuNumber;
	short			menuItem;
	ControlHandle	control;
	Boolean			directoryChanged;
	Cell			cell;

	handledIt = FALSE;
	if (((WindowPeek) theDialog)->refCon == sfMainDialogRefCon) {

		switch (item) {
			case sfHookFirstCall:
				pIdleState = kBuildMenus;
				break;
			case sfHookNullEvent:
				handledIt = HandleIdleEvent(item, theDialog, newItem);
				break;
			case sfItemOpenButton:
				/*
					Note: we remember the reply here because of something
					really goofy that THINK C does. Normally, youÕd think
					that it would be sufficient to check reply.good on
					the way out of PACK 3, and remember the selected file
					if itÕs set to TRUE. However, THINK C has a dialog
					hook procedure that remembers the name in the reply
					record, and then modifies it (I think it puts a 0x7F
					in the second character). This means that we have to
					scarf the selected name before THINK C can change it.
				*/
				RememberReply();
				break;
			default:
				if (item == (gMyItemBase + kMenubar)) {
					control = (ControlHandle) GetItemHandle(theDialog, gMyItemBase + kMenubar);
					menuNumber = GetCtlValue(control);
					menuItem = GetCtlMax(control);
					*newItem = HandleMenuSelection(theDialog, menuNumber, menuItem);
					handledIt = TRUE;
				}
				break;
		}

		//
		// We constantly poll the current selection. This is so we can remember
		// the last selection no matter what the user does: Open the file, Cancel
		// the dialog, or even change directories or volumes.
		//
		if (gList != NIL && IsGetFile(gSelector)) {
			*(long *) &cell = 0;
			if (LGetSelect(TRUE, &cell, gList)) {
				directoryChanged = LMGetCurDirStore() != pCurDirID || -LMGetSFSaveDisk() != pCurVRefNum;
				if (directoryChanged || cell.v != pSelectedCellTheLastTimeILooked) {
					RememberSelectionForRebound();
					pSelectedCellTheLastTimeILooked = cell.v;
				}
			}
		}

	}

	return handledIt;
}


//--------------------------------------------------------------------------------

void	FileFilterEntry()
{
	SetUpA4();
	asm {
		cmp.w	#standardPutFile,gSelector
		move.l	(sp)+,A4
		blt.s	@oldHook
		bra		FileFilterYD
	
	@oldHook
		bra		FileFilter
	}
}


//--------------------------------------------------------------------------------

pascal Boolean	FileFilter(ParmBlkPtr PB)
{
	Boolean	result;

	SetUpA4();
	result = ((FileFilterProcPtr) pFileFilter)(PB);
	RestoreA4();
	return result;
}


//--------------------------------------------------------------------------------

pascal Boolean	FileFilterYD(ParmBlkPtr PB, void *yourDataPtr)
{
	Boolean	result;

	SetUpA4();
	if (pOrigSelector == standardGetFile)
		result = ((FileFilterProcPtr) pFileFilter)(PB);
	else
		result = ((FileFilterYDProcPtr) pFileFilter)(PB, yourDataPtr);
	RestoreA4();
	return result;
}


//--------------------------------------------------------------------------------

void	ModalFilterEntry()
{
	SetUpA4();
	asm {
		cmp.w	#standardPutFile,gSelector
		move.l	(sp)+,A4
		blt.s	@oldHook
		bra		ModalFilterYD
	
	@oldHook
		bra		ModalFilter
	}
}


//--------------------------------------------------------------------------------

pascal Boolean	ModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *itemHit)
{
	Boolean	result;

	SetUpA4();
	result = CommonModalFilter(theDialog, theEvent, itemHit);
	if (!result && (pModalFilter != NIL))
		result = ((ModalFilterProcPtr) pModalFilter)(theDialog, theEvent, itemHit);
	RestoreA4();
	return result;
}


//--------------------------------------------------------------------------------

pascal Boolean	ModalFilterYD(DialogPtr theDialog, EventRecord *theEvent,
								short *itemHit, void *yourDataPtr)
{
	Boolean	result;

	SetUpA4();
	result = CommonModalFilter(theDialog, theEvent, itemHit);
	if (!result && (pModalFilter != NIL))
		result = ((ModalFilterYDProcPtr) pModalFilter)(theDialog, theEvent,
														itemHit, yourDataPtr);
	RestoreA4();
	return result;
}


//--------------------------------------------------------------------------------
/*
	We intercept the modal filter routine to take care of key and mouse
	events. We check for key events so that we can intercept Cmd-key
	sequences; these are not reported to the dialog procedure, so we
	snag them here. We check for mouse down events so that we can set
	up the menus in our menubar before the Dialog Manager calls TrackControl.
*/

Boolean	CommonModalFilter(DialogPtr theDialog, EventRecord *theEvent, short *itemHit)
{
	ControlHandle	control;
	char			key;
	long			menuAndItem;
	MenuHandle		menu;
	RgnHandle		menuRgn;
	Rect			menuRect;

	if (((WindowPeek) theDialog)->refCon == sfMainDialogRefCon) {
		control = (ControlHandle) GetItemHandle(theDialog, gMyItemBase + kMenubar);
	
		menu = ZGetMHandle(control, kOtherMenuResID);
		if (StrLength(gStringToFind) > 0)
			EnableItem(menu, iFindAgain);
		else
			DisableItem(menu, iFindAgain);

		if (theEvent->what == keyDown) {
			if ((theEvent->modifiers & cmdKey) != 0) {
				key = theEvent->message & charCodeMask;
				if (key == '/')
					key = '?';
				menuAndItem = ZMenuKey(control, key);
				if (menuAndItem != 0) {
					*itemHit = HandleMenuSelection(theDialog, HiWrd(menuAndItem), LoWrd(menuAndItem));
					return TRUE;
				}
			}
//		} else if (theEvent->what == mouseDown) {
//			menu = ZGetMHandle(control, kOtherMenuResID);
//			if (Length(gStringToFind) > 0)
//				EnableItem(menu, iFindAgain);
//			else
//				DisableItem(menu, iFindAgain);
		} else if (theEvent->what == updateEvt) {
			if ((DialogPtr) theEvent->message == theDialog) {
				menuRect = (**control).contrlRect;
				menuRgn = NewRgn();
				RectRgn(menuRgn, &menuRect);
				UnionRgn(menuRgn, ((WindowPeek)theDialog)->port.clipRgn,
					((WindowPeek)theDialog)->port.clipRgn);
				DisposeRgn(menuRgn);
			}
		}
	}

	return FALSE;
}


//--------------------------------------------------------------------------------

Boolean	HandleIdleEvent(short item, DialogPtr theDialog, short* newItem)
{
	Boolean			handledIt = FALSE;
	short			index;
	ControlHandle	control;

	switch (pIdleState) {
	
		case kIdle:
			if (pOpenFileAfterFind) {
				pOpenFileAfterFind = FALSE;
				pIdleState = kSelectOpen;
			}

			break;

		case kSelectName:
			/*
				kSelectName mode is used to select a file name after weÕve
				changed directories, and we are using the old StdFile selectors.
				The directory is changed in GoToFile by slamming SFSaveDisk and
				CurDirStore and returning sfRebuildList. The next time we get an
				idle event, we are switched to the specified directory and need
				to select the right name. If we can find the name, we select it
				and then, if necessary, prep ourselves to open the file at the
				next idle event.
			*/

			GetItemIndex(pNameToFindLater, &index);
			if (index >= 0) {
				SelectFileInList(gList, index);
				pIdleState = kIdle;
			}
			handledIt = TRUE;
			break;

		case kSelectOpen:
			*newItem = sfItemOpenButton;
			pIdleState = kIdle;
			break;

		case kBuildMenus:
			control = (ControlHandle) GetItemHandle(theDialog, gMyItemBase + kMenubar);
			if (control != NIL) {
				FilterDirectories();
				FilterFilesByHookAndList();
				RebuildMenus(control);
			} else {
				DebugStr("\pCan't find control.");
			}
			pIdleState = kIdle;
			break;
	}

	return handledIt;
}


//--------------------------------------------------------------------------------

void	MyLSetSelect(Boolean setIt, Cell* originalItem, ListHandle lHandle)
{
	FileObjectHdl	reboundObject;
	short			negSFSaveDisk;
	long			curDirStore;
	Rect*			visibleArea;

	if (gList == lHandle && setIt)
	{
		curDirStore = LMGetCurDirStore();
		negSFSaveDisk = -LMGetSFSaveDisk();
		if (curDirStore != pCurDirID || negSFSaveDisk != pCurVRefNum)
		{
			pCurVRefNum = negSFSaveDisk;
			pCurDirID = curDirStore;

			if ((originalItem->v == 0) && IsGetFile(gSelector))
			{
				reboundObject = GetReboundItem(pCurVRefNum, pCurDirID);
				if (reboundObject != NIL)
				{
					GetItemIndex(*(**reboundObject).fName, &originalItem->v);
					LSetSelect(TRUE, *originalItem, lHandle);
					LAutoScroll(lHandle);
					
					visibleArea = &(**lHandle).visible;
					if (visibleArea->left != 0 || visibleArea->top != 0)
						pSputchNextScroll = TRUE;
			
					pSelectedCellTheLastTimeILooked = originalItem->v;
						/*	pSelectedCellTheLastTimeILooked is used to determine when
							to update our rebound list with a new item. Since we know
							the list is still up-to-date, set pSelectedCellTheLastTimeILooked
							with the current index to prevent our trying to save
							information we donÕt need to. */
				}
			}
		}
	}
}


//--------------------------------------------------------------------------------

void	MyLScroll(short* dCols, short* dRows, ListHandle lHandle)
{
	if (gList == lHandle && pSputchNextScroll)
	{
		*dCols = 0;
		*dRows = 0;
		pSputchNextScroll = FALSE;
	}
}


//--------------------------------------------------------------------------------

void	GetItemIndex(StringPtr stringToFind, short* index)
{
	Cell			cell;

	*index = -1;
	*(long*) &cell = 0;
	(**gList).maxIndex += 2;			// ¥¥¥ to fix bug???
	if (LSearch(stringToFind, 0, TestSFEntry, &cell, gList))
		*index = cell.v;
	(**gList).maxIndex -= 2;			// ¥¥¥ to fix bug???
}


//--------------------------------------------------------------------------------

pascal short TestSFEntry(Ptr cellPtr, Ptr testPtr, short cellLen, short testLen)
{
	short	result = 1;		// default to not matching (yes, Ò1Ó means no match)
	FSSpec	cellSpec;
	
	SetUpA4();

	SFEntryToFSSpec(cellPtr, &cellSpec);
	if (PLstrcompare(cellSpec.name, (StringPtr) testPtr) == 0)
		result = 0;		// means that they match
	
	RestoreA4();
	
	return result;
}


//--------------------------------------------------------------------------------
/*
	Select the cell in the Standard File list of files. There are three cases
	to take care of:
	
		- no files are selected. Select our file.
		- a file is selected that is different than the one we want. Deselect
		  it and select our file.
		- a file is selected that is already the one we want. Don't do
		  anything.
*/

void	SelectFileInList(ListHandle theList, short itemIndex)
{
	Cell		cell;
	Boolean		selectNewCell = TRUE;

	if ((**theList).dataBounds.bottom > 0) {
	
		*(long*) &cell = 0;
		if (LGetSelect(TRUE, &cell, theList)) {
			if (cell.v != itemIndex) {
				LSetSelect(FALSE, cell, theList);
			} else {
				selectNewCell = FALSE;
			}
		}
		
		if (selectNewCell)
		{
			cell.v = itemIndex;
			LSetSelect(TRUE, cell, theList);
			LAutoScroll(theList);
		}
	}
}


//--------------------------------------------------------------------------------

short	HandleMenuSelection(DialogPtr dlg, short whichMenu, short menuItem)
{
	short	result = sfHookNullEvent;

	if (menuItem != 0) {

		switch (whichMenu) {
			case kDirectoryMenuResID:
				result = GoToDirectoryItem(menuItem);
				break;
			case kFileMenuResID:
				result = GoToFileItem(menuItem);
				pOpenFileAfterFind = TRUE;
				break;
			case kOtherMenuResID:
				result = HandleOtherMenu(dlg, menuItem);
				break;
		}
	}

	return result;
}


//--------------------------------------------------------------------------------

short	GoToDirectoryItem(short index)
{
	FileObjectHdl	directory;
	FSSpec			spec;
	short			result = sfHookNullEvent;

	directory = GetNthOpenableFileObject(gDirectoryList, index-1);
	if (directory != NIL) {
		if (FileHandleExists(directory)) {
			spec.vRefNum = GetVolRefNum(directory);
			spec.parID = (**directory).u.dirID;
			return GoToDirectory(&spec);
		} else {
			DeleteFileObject(gDirectoryList, directory);
		}
	}
	return sfHookNullEvent;
}


//--------------------------------------------------------------------------------

short	GoToDirectory(FSSpecPtr spec)
{
	FileObjectHdl	reboundObject;

	spec->name[0] = 0;			// ¥¥¥ setting field directly dangerous here?

	if (IsGetFile(gSelector)) {
		reboundObject = GetReboundItem(spec->vRefNum, spec->parID);
		if (reboundObject != NIL)
			FSMakeFSSpec(spec->vRefNum, spec->parID, *(**reboundObject).fName, spec);
	}

	return GoToFile(spec);
}


//--------------------------------------------------------------------------------

short	GoToFileItem(short index)
{
	FileObjectHdl	file;
	FSSpec			spec;

	file = GetNthOpenableFileObject(gFileList, index-1);
	if (file != NIL) {
		if (HandleToFSSpec(file, &spec) == noErr) {
			return GoToFile(&spec);
		} else {
			DeleteFileObject(gFileList, file);
		}
	}
	return sfHookNullEvent;
}


//--------------------------------------------------------------------------------
/*
	Set things up to move to the directory a file is in and select it. There
	are several things to consider here:
	
	- If using the new-style Standard File selectors, we can simply fill in
	  the reply record and return sfHookChangeSelection.
	- If using the old-style Standard File selectors, we need to change
	  SFSaveDisk and CurDirStore, prepare ourselves to select the name when
	  we get an idle event, and return sfHookRebuildList so that the directory
	  gets changed.
	- If we are already in the right volume/directory, don't rebuild the file
	  list; just select the new name.
	- The name field of the FSSpec may or may not contain a name. If it
	  doesnÕt, we are merely changing directories and would like the rebound
	  item to be chosen back in the idle event handler. If the name field does
	  contain a name, we would like to inhibit the selection of the rebound item.
	  Since our idle event handler selects the rebound item any time it notices
	  that the directory has changed, we set pCurVRefNum and pCurDirID so that the
	  idle event handler thinks the directory hasnÕt changed.
*/

short	GoToFile(FSSpecPtr spec)
{
	if (StrLength(spec->name) > 0)
	{
		pCurVRefNum = -LMGetSFSaveDisk();
		pCurDirID = LMGetCurDirStore();
	}

	if (IsNewCall(gSelector))
	{
		FSSpecToReply(spec, GetReplyPtr());
		return sfHookChangeSelection;
	}
	else
	{
		if (LMGetSFSaveDisk() != -spec->vRefNum || LMGetCurDirStore() != spec->parID)
		{
			LMSetSFSaveDisk(-spec->vRefNum);
			LMSetCurDirStore(spec->parID);
			if (StrLength(spec->name) > 0)
			{
				PStringCopy(pNameToFindLater, spec->name);
				pIdleState = kSelectName;
			}
			return sfHookRebuildList;
		}
		else
		{
			if (StrLength(spec->name) > 0)
			{
				PStringCopy(pNameToFindLater, spec->name);
				pIdleState = kSelectName;
			}
			return sfHookNullEvent;
		}
	}
}


//--------------------------------------------------------------------------------

void	ReFilterMenus(DialogPtr theDialog)
{
	ControlHandle	control;
	MenuHandle		menu;

	if (gMyItemBase > 0) {
		control = (ControlHandle) GetItemHandle(theDialog, gMyItemBase + kMenubar);
	
		menu = ZGetMHandle(control, kDirectoryMenuResID);
		if (menu != NIL) {
			FilterDirectories();
			AddDirectories(menu);
		} else {
			DebugStr("\pCan't find directory menu.");
		}
	
		menu = ZGetMHandle(control, kFileMenuResID);
		if (menu != NIL) {
			FilterFilesByHookAndList();
			AddFiles(menu);
		} else {
			DebugStr("\pCan't find file menu.");
		}
	}
}


//--------------------------------------------------------------------------------

short	ListItem()
{
	if (IsNewCall(gSelector))
		return sfItemFileListUser;
	else if (IsGetFile(gSelector))
		return getNmList;
	else
		return 8;
}


//--------------------------------------------------------------------------------

void	RememberReply()
{
	FSSpec			spec;

	/*
		Might be better to use:

		FSMakeFSSpec(-SFSaveDisk, CurDirStore, IsNewCall(gSelector)
							? ((StandardFileReply*) GetReplyPtr())->sfFile.name
							: ((SFReply*) GetReplyPtr())->fName, &spec);
	*/

	SelectionToFSSpec(gList, &spec);
	RememberFileAndParent(&spec);
}


//--------------------------------------------------------------------------------

Boolean	CanOpenFile(FSSpec* file, OSType* itsType, OSErr* err)
{
	Boolean			result = FALSE;
	HParamBlockRec	pb;
	CInfoPBRec		cpb;
	short			numTypes;
	short			typeNumber;
	OSType*			types;
	void*			fileFilter;

	if (gSelector >= customPutFile) {
		cpb.hFileInfo.ioCompletion = NIL;
		cpb.hFileInfo.ioNamePtr = file->name;
		cpb.hFileInfo.ioVRefNum = file->vRefNum;
		cpb.hFileInfo.ioFDirIndex = 0;
		cpb.hFileInfo.ioDirID = file->parID;
		*err = PBGetCatInfoSync(&cpb);
	} else {
		pb.fileParam.ioCompletion = NIL;
		pb.fileParam.ioNamePtr = file->name;
		pb.fileParam.ioVRefNum = file->vRefNum;
		pb.fileParam.ioFVersNum = 0;
		pb.fileParam.ioFDirIndex = 0;
		pb.fileParam.ioDirID = file->parID;
		*err = PBHGetFInfoSync(&pb);
	}

	if (*err == noErr) {
		if (gSelector >= customPutFile) {
			*itsType = cpb.hFileInfo.ioFlFndrInfo.fdType;
		} else {
			*itsType = pb.fileParam.ioFlFndrInfo.fdType;
		}
	
		numTypes = GetNumTypes();
		if (numTypes < 0) {
			result = TRUE;
		} else {
			//
			// Note: If numTypes == 0, result is left at FALSE,
			//		 which is what we want.
			//
			types = GetTypesPtr();
			for (typeNumber = 0; typeNumber < numTypes; typeNumber++) {
				if (types[typeNumber] == kOpenAnyFile || types[typeNumber] == *itsType) {
					result = TRUE;
				}
			}
		}

		fileFilter = GetFilterProcPtr();
		if (result && fileFilter != NIL)
			result = !((gSelector < customPutFile)
				? ((FileFilterProcPtr) fileFilter)((ParmBlkPtr) &pb)
				: ((FileFilterYDProcPtr) fileFilter)((ParmBlkPtr) &cpb, GetYDPtr()));
	}

	return result;
}


//--------------------------------------------------------------------------------

void*	GetReplyPtr()
{
	return *(void**) (pParameters + gOffsetToReply[gSelector-1]);
}


//--------------------------------------------------------------------------------

short	GetNumTypes()
{
	if (pOffsetToNumTypes[gSelector-1] == -1)
		return 0;
	else
		return *(short*) (pParameters + pOffsetToNumTypes[gSelector-1]);
}


//--------------------------------------------------------------------------------

OSType*	GetTypesPtr()
{
	if (pOffsetToTypeList[gSelector-1] == -1)
		return NIL;
	else
		return *(OSType**) (pParameters + pOffsetToTypeList[gSelector-1]);
}


//--------------------------------------------------------------------------------

void*	GetFilterProcPtr()
{
	if (pOffsetToFileFilter[gSelector-1] == -1)
		return NIL;
	else
		return *(void**) (pParameters + pOffsetToFileFilter[gSelector-1]);
}


//--------------------------------------------------------------------------------

void*	GetYDPtr()
{
	if (pOffsetToYD[gSelector-1] == -1)
		return NIL;
	else
		return *(void**) (pParameters + pOffsetToYD[gSelector-1]);
}

