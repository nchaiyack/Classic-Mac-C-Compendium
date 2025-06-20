/*______________________________________________________________________

	utl.c - Utilities.
	
	Copyright � 1988-1991 Northwestern University.
	
	This module exports miscellaneous reusable utility routines.
_____________________________________________________________________*/

#pragma load "precompile"
#include "utl.h"


#pragma segment utl

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static SysEnvRec		TheWorld;		/* system environment record */
static Boolean			GotSysEnviron = false;	
												/* true if sys environ has been
													gotten */
	
			
static CursHandle 	*CursArray;		/* ptr to array of cursor handles */
static short 			NumCurs;			/* number of cursors to rotate */
static short 			TickInterval;	/* number of ticks between rotations */
static short 			CurCurs;			/* index of current cursor */
static long 			LastTick;		/* tick count at loast rotation */

/* The following global variables are used by the MetFilter, 
	utl_ModalDialog, utl_ModalDialogContinue, and utl_StopAlert functions */

static ModalFilterProcPtr	Filter;	/* dialog filter proc */
static short			DefaultItem;	/* item number of default button */
static short			CancelItem;		/* item number of cancel button */
static CursHandle		IBeamHandle;	/* handle to IBeam cursor */

/*______________________________________________________________________

	GetSysEnvirons - Get System Environment.
	
	Exit:		global variable TheWorld = system environment record.
				global variable GotSysEnviron = true.
_____________________________________________________________________*/

static void GetSysEnvirons (void)

{
	if (!GotSysEnviron) {
		(void) SysEnvirons(curSysEnvVers, &TheWorld);
		GotSysEnviron = true;
	}
}

/*______________________________________________________________________

	MetaFilter - Dialog Filter Proc.
	
	Entry:	theDialog = pointer to dialog record.
				theEvent = pointer to event record.
	
	Exit:		if command-period or escape typed and CancelItem is non-zero:
					function result = true.
					itemHit = CancelItem.
				else if return or enter typed and DefaultItem is non-zero:
					function result = true.
					itemHit = DefaultItem.
				else if user specified a filterProc on the call to
				utl_StopAlert, utl_ModalDialog, or utl_ModalDialogContinue:
					user's filterProc called, function result and itemHit
					returned by user's filterProc.
				else:
					function result = false.
					itemHit = undefined.
					
	This filter proc also takes care of flashing the default and cancel
	buttons when they are selected via key presses, and changing the cursor
	to an Ibeam over textedit fields.
_____________________________________________________________________*/

static pascal Boolean MetaFilter (DialogPtr theDialog,
	EventRecord *theEvent, short *itemHit)
	
{
	Point				where;					/* mouse loc */
	short				numItems;				/* number of items in dialog */
	Boolean			SetCursorToIBeam;		/* true if mouse is over textedit field */
	short				i;							/* loop index */
	short				itemType;				/* item type */
	Handle			item;						/* handle to item */
	Rect				box;						/* item rectangle */
	
	SetPort(theDialog);
	GetMouse(&where);
	numItems = **(short**)(((DialogPeek)theDialog)->items) + 1;
	SetCursorToIBeam = false;
	for (i = 1; i <= numItems; i++) {
		GetDItem(theDialog, i, &itemType, &item, &box);
		if ((itemType & 0x7f) == editText && PtInRect(where, &box)) {
			SetCursorToIBeam = true;
			break;
		}
	}
	SetCursor(SetCursorToIBeam ? *IBeamHandle : &qd.arrow);
	if (theEvent->what != keyDown && theEvent->what != autoKey) {
		if (Filter) {
			return (*Filter)(theDialog, theEvent, itemHit);
		} else {
			return false;
		}
	} else if (*itemHit = utl_CheckSpecialDialogKey(theDialog, 
		theEvent->message & charCodeMask, theEvent->modifiers, 
		DefaultItem, CancelItem)) {
		return true;
	} else if (Filter) {
		return (*Filter)(theDialog, theEvent, itemHit);
	} else {
		return false;
	}
}

/*______________________________________________________________________

	MagicAlert - Present a Magic Alert.
	
	Entry:	tmpl = error message template.
				dlogID = resource id of error message dialog.
				filterProc = pointer to filter proc.
				p0,p1,p2,p3 = parameters to plug into template.
				lastBtn = item number of last button.
				staticText = item number of static text field.
				outline = item number of useritem to outline default button.
				defaultItem = item number of default button.
				cancelItem = item number of cancel button, or 0 if none.
				
	Exit:		function result = item hit.
				
	This helper function is used by utl_ErrorAlert and utl_SaveChangesAlert.
_____________________________________________________________________*/

static short MagicAlert (Str255 tmpl, short dlogID, 
	ModalFilterProcPtr filterProc, Str255 p0, Str255 p1,
	Str255 p2, Str255 p3, short lastBtn, short staticText, short outline,
	short defaultItem, short cancelItem)

{
	GrafPtr			savedPort;		/* saved grafport */
	Str255			msg;				/* the error message */
	DialogPtr		dlog;				/* dialog pointer */
	short				itemType;		/* item type */
	Handle			item;				/* handle to item */
	Rect				box;				/* item rectangle */
	short				boxHeight;		/* height of message display rectangle */
	TEHandle			edit;				/* handle to TextEdit record */
	short				height;			/* height of displayed text */
	short				delta;			/* adjustment to height */
	Rect				contrlRect;		/* OK button rectangle */
	Rect				dlogRect;		/* dialog portrect */
	short				itemHit;			/* item hit */
	short				i;					/* loop index */
		
	GetPort(&savedPort);
		
	/* Create the dialog. */
	
	dlog = GetNewDialog(dlogID, nil, (WindowPtr)-1);
	SetPort(dlog);
	
	/* Plug parameters into the message. */
	
	utl_PlugParams(tmpl, msg, p0, p1, p2, p3);
		
	/* Get TextEdit to figure out how tall the text will be when displayed. */
		
	GetDItem(dlog, staticText, &itemType, &item, &box);
	edit = TENew(&box, &box);
	TESetText(msg+1, *msg, edit);
	TECalText(edit);
	height = (**edit).nLines * (**edit).lineHeight;
	TEDispose(edit);
	
	/* Adjust the height of the dialog, the height of the static text item,
		and the position of the buttons. */
		
	boxHeight = box.bottom - box.top;
	delta = height - boxHeight;
	SizeWindow(dlog, dlog->portRect.right, dlog->portRect.bottom + delta, false);
	box.bottom += delta;
	SetDItem(dlog, staticText, itemType, item, &box);
	for (i = 1; i <= lastBtn; i++) {
		GetDItem(dlog, i, &itemType, &item, &box);
		contrlRect = (**((ControlHandle)item)).contrlRect;
		MoveControl((ControlHandle)item, contrlRect.left, contrlRect.top + delta);
		OffsetRect(&box, 0, delta);
		SetDItem(dlog, i, itemType, item, &box);
	}
	
	/* Center the dialog on the main screen. */
	
	dlogRect = dlog->portRect;
	utl_CenterDlogRect(&dlogRect, true);
	MoveWindow(dlog, dlogRect.left, dlogRect.top, false);
	
	/* Set the user proc to utl_OutlineDefaultButton. */
	
	GetDItem(dlog, outline, &itemType, &item, &box);
	SetDItem(dlog, outline, itemType, (Handle)utl_OutlineDefaultButton, &box);
	
	/* Show the dialog and wait for the user to dismiss it. Use the
		metafilter proc to treat Return and Enter the same as a click on
		the OK button. */
		
	ParamText(msg, nil, nil, nil);
	ShowWindow(dlog);
	Filter = filterProc;
	DefaultItem = defaultItem;
	CancelItem = cancelItem;
	ModalDialog(MetaFilter, &itemHit);
	DisposDialog(dlog);
	SetPort(savedPort);
	return itemHit;
}

/*______________________________________________________________________

	utl_AppendDITL - Append DITL to End of Dialog.
	
	Entry:	theDialog = pointer to dialog.
				theDITLID = rsrc id of DITL.
					
	Exit:		function result = item number of first appended item.
	
	The dialog window is expanded to accomodate the new items, and the
	new items are offset to appear at the bottom of the dialog.
	
	This routine is particularly useful for appending items to the
	standard Page Setup and Print Job dialogs.  (See TN 95).  It was
	written by Lew Rollins of Apple's Human-Systems Interface Group,
	in MPW Pascal.  I translated it to MPW C.
	
	The only significant difference between this routine and Rollin's
	version is that this version does not release the DITL resource.
_____________________________________________________________________*/

short utl_AppendDITL (DialogPtr theDialog, short theDITLID)

{
	typedef struct DITLItem {
		Handle			itmHndl;				/* handle or proc ptr */
		Rect				itmRect;				/* display rect */
		char				itmType;				/* item type */
		unsigned char	itmData;				/* item data length byte */
	} DITLItem;
	
	typedef struct itemList {
		short				dlgMaxIndex;		/* num items - 1 */
		DITLItem			DITLItems[1];		/* array of DITL items */
	} itemList;
	
	short				offset;			/* item offset */
	Rect				maxRect;			/* max dialog rect size so far */
	Handle			hDITL;			/* handle to DITL */
	DITLItem			*pItem;			/* pointer to item being appended */
	itemList			**hItems;		/* handle to DLOG's item list */
	short				sizeDITL;		/* size of DLOG's item list */
	short				firstItem;		/* item num of first appended item */
	short				newItems;		/* number of new items */
	short				dataSize;		/* size of data for current item */
	short				i;					/* loop index */
	
	/* Initialize. */
	
	maxRect = theDialog->portRect;
	offset = maxRect.bottom;
	maxRect.bottom -= 5;
	maxRect.right -= 5;
	hItems = (itemList**)(((DialogPeek)theDialog)->items);
	sizeDITL = GetHandleSize((Handle)hItems);
	firstItem = (**hItems).dlgMaxIndex + 2;
	hDITL = GetResource('DITL', theDITLID);
	HLock(hDITL);
	newItems = **(short**)hDITL + 1;
	PtrAndHand(*hDITL+2, (Handle)hItems,
		GetHandleSize(hDITL)-2);
	(**hItems).dlgMaxIndex += newItems;
	HUnlock(hDITL);
	HLock((Handle)hItems);
	pItem = (DITLItem*)((char*)(*hItems) + sizeDITL);
	
	/* Main loop.  Add each item to dialog item list. */
	
	for (i = 1; i <= newItems; i++) {
		OffsetRect(&pItem->itmRect, 0, offset);
		UnionRect(&pItem->itmRect, &maxRect, &maxRect);
		switch (pItem->itmType & 0x7f) {
			case ctrlItem+btnCtrl:
			case ctrlItem+chkCtrl:
			case ctrlItem+radCtrl:
				pItem->itmHndl = (Handle)NewControl(theDialog, 
					&pItem->itmRect, &pItem->itmData, true, 0, 0, 1, 
					pItem->itmType & 0x03, 0);
				break;
			case ctrlItem+resCtrl:
				pItem->itmHndl = (Handle)GetNewControl(
					*(short*)(&pItem->itmData+1),
					theDialog);
				(**((ControlHandle)(pItem->itmHndl))).contrlRect = 
					pItem->itmRect;
				break;
			case statText:
			case editText:
				PtrToHand(&pItem->itmData+1, &pItem->itmHndl, pItem->itmData);
				break;
			case iconItem:
				pItem->itmHndl = GetIcon(*(short*)(&pItem->itmData+1));
				break;
			default:
				pItem->itmHndl = nil;
		}
		dataSize = (pItem->itmData + 1) & 0xfffe;
		pItem = (DITLItem*)((char*)pItem + dataSize + sizeof(DITLItem));
	}
	
	/* Finish up. */
	
	HUnlock((Handle)hItems);
	maxRect.bottom += 5;
	maxRect.right += 5;
	SizeWindow(theDialog, maxRect.right, maxRect.bottom, true);
	return firstItem;
}

/*______________________________________________________________________

	utl_BuildDialogIBeamRgn - Build Dialog IBeam Region.
	
	Entry:	theDialog = pointer to dialog record.
	
	Exit:		function result = handle to ibeam region.
_____________________________________________________________________*/

RgnHandle utl_BuildDialogIBeamRgn (DialogPtr theDialog)

{
	short			numItems;			/* number of items */
	RgnHandle	ibRgn;				/* ibeam region */
	RgnHandle	rgn;					/* scratch region */
	short			itemType;			/* item type */
	Handle		item;					/* handle to item */
	Rect			box;					/* item rectangle */
	short			i;						/* loop index */
	
	ibRgn = NewRgn();
	rgn = NewRgn();
	numItems = **(short**)(((DialogPeek)theDialog)->items) + 1;
	for (i = 1; i <= numItems; i++) {
		GetDItem(theDialog, i, &itemType, &item, &box);
		if ((itemType & 0x7f) == editText) {
			RectRgn(rgn, &box);
			UnionRgn(ibRgn, rgn, ibRgn);
		}
	}
	DisposeRgn(rgn);
	return ibRgn;
}

/*______________________________________________________________________

	utl_CenterDlogRect - Center a dialog rectangle.
	
	Entry:	rect = rectangle.
				centerMain = true to center on main (menu bar) screen.
				centerMain = false to center on the screen containing
					the maximum intersection with the frontmost window.
	
	Exit:		rect = rectangle offset so that it is centered on
					the specified screen, with twice as much space below
					the rect as above.
					
	See HIN 6.
_____________________________________________________________________*/

void utl_CenterDlogRect (Rect *rect, Boolean centerMain)

{
	Rect		screenRect;			/* screen rectangle */
	short		mBHeight;			/* menu bar height */
	GDHandle	gd;					/* gdevice */
	Rect		windRect;			/* window rectangle */
	Boolean	hasMB;				/* true if screen contains menu bar */

	mBHeight = utl_GetMBarHeight();
	if (centerMain) {
		screenRect = qd.screenBits.bounds;
	} else {
		utl_GetWindGD(FrontWindow(), &gd, &screenRect, &windRect, &hasMB);
		if (!hasMB) mBHeight = 0;
	}
	OffsetRect(rect,
		(screenRect.right + screenRect.left - rect->right - rect->left) >> 1,
		(screenRect.bottom + ((screenRect.top + mBHeight - rect->top)<<1) - 
			rect->bottom + 7) / 3);
}

/*______________________________________________________________________

	utl_CenterRect - Center a rectangle on the main screen.
	
	Entry:	rect = rectangle.
	
	Exit:		rect = rectangle offset so that it is centered on
					the main screen.
_____________________________________________________________________*/

void utl_CenterRect (Rect *rect)

{
	Rect		screenRect;			/* main screen rectangle */
	short		mBHeight;			/* menu bar height */

	mBHeight = utl_GetMBarHeight();
	screenRect = qd.screenBits.bounds;
	OffsetRect(rect,
		(screenRect.right + screenRect.left - rect->right - rect->left) >> 1,
		(screenRect.bottom - screenRect.top + mBHeight - 
			rect->bottom - rect->top) >> 1);
}

/*______________________________________________________________________

	utl_CheckAERequiredParams - Check Required Apple Event Parameters.
	
	Entry:	event = pointer to Apple event.
	
	Exit:		function result = noErr if all required parameters feteched.
				function result = errAEEventNotHandled if some event 
					contains unfetched required parameters.
				function result = error code if some other error.
	
	See IM VI 6-47.
_____________________________________________________________________*/

OSErr utl_CheckAERequiredParams (AppleEvent *event)

{
	DescType			theType;
	long				actualSize;
	OSErr				rCode;
	
	rCode = AEGetAttributePtr(event, keyMissedKeywordAttr, typeWildCard,
		&theType, nil, 0, &actualSize);
	if (rCode == errAEDescNotFound) {
		return noErr;
	} else if (rCode == noErr) {
		return errAEEventNotHandled;
	} else {
		return rCode;
	}
}

/*______________________________________________________________________

	utl_CheckPack - Check to see if a package exists.
	
	Entry:	packNum = package number.
				preload = true to preload package.
					
	Exit:		function result = true if package exists.
_____________________________________________________________________*/

Boolean utl_CheckPack (short packNum, Boolean preload)

{
	short			trapNum;			/* trap number */
	Handle		h;					/* handle to PACK resource */
	
	/* Check to make sure the trap exists, by comparing its trap address to
		the trap address of the unimplemented trap. */
	
	trapNum = packNum + 0x1e7;
	if (NGetTrapAddress(trapNum & 0x3ff, ToolTrap) == 
		NGetTrapAddress(_Unimplemented & 0x3ff, ToolTrap)) return false;
		
	/* Check to make sure the package exists on the System file or in 
		ROM.  If it's not in ROM make it nonpurgeable, if requested. */
	
	if (preload) {
		if (utl_Rom64()) {
			h = GetResource('PACK', packNum);
			if (!h) return false;
			HNoPurge(h);
		} else {
			*(unsigned short*)RomMapInsert = 0xFF00;
			h = GetResource('PACK', packNum);
			if (!h) return false;
			if ((*(unsigned long*)h & 0x00FFFFFF) < 
				*(unsigned long*)ROMBase) {
				h = GetResource('PACK', packNum);
				HNoPurge(h);
			}
		}
		return true;
	} else {
		SetResLoad(false);
		if (!utl_Rom64()) *(unsigned short*)RomMapInsert = 0xFF00;
		h = GetResource('PACK', packNum);
		SetResLoad(true);
		if (h) return true; else return false;
	}
}

/*______________________________________________________________________

	utl_CheckSpecialDialogKey - Check for Special Dialog Key.
	
	Entry:	theDialog = pointer to dialog record.
				theKey = ascii code of the key pressed.
				modifiers = modifiers from event record.
				defaultItem = default button item number, or 0 if none.
				cancelItem = cancel button item number, or 0 if none.
				
	Exit:		function result = item number of special button, or 0 if
					none.
					
	Return and enter are treated the same as clicks on the default button.
	Escape and command-period are treated the same as clicks on the cancel
	button.
	Buttons are flashed.
_____________________________________________________________________*/

short utl_CheckSpecialDialogKey (DialogPtr theDialog, char theKey,
	short modifiers, short defaultItem, short cancelItem)
	
{
	if (defaultItem && (theKey == returnKey || theKey == enterKey)) {
		utl_FlashDialogButton(theDialog, defaultItem);
		return defaultItem;
	} else if (cancelItem && 
		(theKey == escapeKey || ((modifiers & cmdKey) && theKey == '.'))) {
		utl_FlashDialogButton(theDialog, cancelItem);
		return cancelItem;
	} else {
		return 0;
	}
}

/*______________________________________________________________________

	utl_CopyPString - Copy Pascal String.
	
	Entry:	dest = destination string.
				source = source string.
_____________________________________________________________________*/

void utl_CopyPString (Str255 dest, Str255 source)

{
	memcpy(dest, source, *source+1);
}

/*______________________________________________________________________

	utl_CouldDrag - Determine if a window could be dragged to a location.
	
	Entry:	windRect = window rectangle, in global coords.
				offset = pixel offset used in DragRect calls.
	
	Exit:		function result = true if the window could have been
					dragged to the specified position.
					
	This routine is used when restoring windows to saved positions.  According
	to HIN 6, we must check to see if the window "could have been dragged to
	the saved position."
	
	The "offset" parameter is usually 4.  When initializing the boundary rectangle
	for DragWindow calls, normally the boundary rectangle of the desktop gray
	region is inset by 4 pixels.  If some value other than 4 is used, it should
	be passed to CouldDrag as the "offset" parameter.
	
	The algorithm used is the following:  The routine computes the four squares
	at the corners of the title bar.  "true" is returned if and only if at least one 
	of these four squares is completely contained within the desktop gray region.
	
	Three pixels are added to the offset to err on the side of requiring a larger
	portion of the drag bar to be visible.  
_____________________________________________________________________*/

Boolean utl_CouldDrag (Rect *windRect, short offset)

{
	RgnHandle			rgn;			/* scratch region handle */
	Boolean				could;		/* function result */
	short					corner;		/* which corner */
	Rect					r;				/* corner rectangle */

	rgn = NewRgn();
	could = false;
	offset += 3;
	for (corner = 1; corner <= 4; corner++) {
		switch (corner) {
			case 1:
				r.top = windRect->top - titleBarHeight;
				r.left = windRect->left;
				break;
			case 2:
				r.top = windRect->top - offset;
				r.left = windRect->left;
				break;
			case 3:
				r.top = windRect->top - titleBarHeight;
				r.left = windRect->right - offset;
				break;
			case 4:
				r.top = windRect->top - offset;
				r.left = windRect->right - offset;
				break;
		}
		r.bottom = r.top + offset;
		r.right = r.left + offset;
		RectRgn(rgn, &r);
		DiffRgn(rgn, *(RgnHandle*)GrayRgn, rgn);
		if (EmptyRgn(rgn)) {
			could = true;
			break;
		}
	}
	DisposeRgn(rgn);
	return could;
}

/*______________________________________________________________________

	utl_CountProcesses - Count Processes.
	
	Exit:		function result = error code.
				count = number of active processes.
				
	System 7.0 only.
_____________________________________________________________________*/

OSErr utl_CountProcesses (short *count)
	
{
	OSErr						rCode;				/* result code */
	ProcessSerialNumber	psn;					/* process serial number */

	*count = 0;
	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kNoProcess;
	while (!(rCode = GetNextProcess(&psn))) (*count)++;
	if (rCode == procNotFound) rCode = noErr;
	return rCode;
}

/*______________________________________________________________________

	utl_DILoad - Load Disk Initialization Package.
	
	Exit:		Disk initialization package loaded.
				
	This routine is identical to the DILoad routine (see IM II-396),
	except that it closes any resource files opened by the routine.  This is
	necessary to undo a bug in the DaynaFile software.  DaynaFile patches
	DILoad.  The patch opens a resource file without closing it.  The
	effect on Disinfectant if we don't do anything about this is that 
	the DaynaFile icon is displayed in Disinfectant's main window instead of
	Disinfectant's icon.  
_____________________________________________________________________*/

void utl_DILoad (void)

{
	short			curResFile;			/* ref num of current resource file
												before calling DILoad */
	short			topResFile;			/* ref num of top resource file
												after calling DILoad */
	
	curResFile = CurResFile();
	DILoad();
	while ((topResFile = CurResFile()) != curResFile) CloseResFile(topResFile);
}

/*______________________________________________________________________

	utl_DoDiskInsert - Handle a disk inserted event.
	
	Entry:	message = message field from disk insertion event record
					= 16/MountVol return code, 16/drive number.
	
	Exit:		vRefNum = vol ref num of inserted volume.
				function result = error code.
	
	If MountVol returned an error code, the disk initialization package
	is called to initialize the disk.
_____________________________________________________________________*/

OSErr utl_DoDiskInsert (long message, short *vRefNum)

{
	OSErr				rCode;				/* result code */
	short				driveNum;			/* drive number */
	HParamBlockRec	pBlock;				/* vol info param block */
	Handle			dlgHandle;			/* handle to disk init dialog */
	Rect				dlgRect;				/* disk init dialog rectangle */
	Point				where;				/* location of disk init dialog */
	short				curVol;				/* index in VCB queue */
	
	/* Get result code and drive number from event message. */
	
	rCode = (message >> 16) & 0xffff;
	driveNum = message & 0xffff;
	
	/* If the result code indicates an error, call DIBadMount to initialize
		the disk. */
		
	if (rCode) {
		
		/* Center the disk initialization package dialog. */
		
		DILoad();
		dlgHandle = GetResource('DLOG', -6047);
		dlgRect = **(Rect**)dlgHandle;
		utl_CenterDlogRect(&dlgRect, false);
		SetPt(&where, dlgRect.left, dlgRect.top);
		
		/* Call DIBadMount. */
		
		if (rCode = DIBadMount(where, message)) return rCode;
		
	}
	
	/* Search mounted volumes to find inserted one. */
	
	pBlock.volumeParam.ioNamePtr = nil;
	curVol = 0;
	while (true) {
		pBlock.volumeParam.ioVolIndex = ++curVol;
		pBlock.volumeParam.ioVRefNum = 0;
		if (rCode = PBHGetVInfo(&pBlock, false)) return rCode;
		if (pBlock.volumeParam.ioVDrvInfo == driveNum) break;
	}
	*vRefNum = pBlock.volumeParam.ioVRefNum;
	return noErr;
}	

/*______________________________________________________________________

	utl_DrawGrowIcon - Draw Grow Icon.
	
	Entry:			theWindow = pointer to window.
	
	This routine is identical to the Window Manager routine 
	DrawGrowIcon, except that it does not draw the lines enclosing the
	scroll bars.
_____________________________________________________________________*/

void utl_DrawGrowIcon (WindowPtr theWindow) 

{
	RgnHandle			clipRgn;			/* saved clip region */
	Rect					clipRect;		/* clip rectangle */
	
	clipRgn = NewRgn();
	GetClip(clipRgn);
	clipRect = theWindow->portRect;
	clipRect.left = clipRect.right - 15;
	clipRect.top = clipRect.bottom - 15;
	ClipRect(&clipRect);
	DrawGrowIcon(theWindow);
	SetClip(clipRgn);
	DisposeRgn(clipRgn);
}

/*______________________________________________________________________

	utl_Ejectable - Test for ejectable volume.
	
	Entry:	vRefNum = volume reference number.
	
	Exit:		function result = true if volume is on an ejectable drive.
_____________________________________________________________________*/

Boolean utl_Ejectable (short vRefNum)
	
{
	HParamBlockRec	pBlock;				/* vol info param block */
	short				driveNum;			/* driver number of cur vol */
	DrvQEl			*curDrive;			/* ptr to current drive queue element */
	OSErr				rCode;				/* result code */
	unsigned char	flagByte;			/* drive queue element flag byte */
	
	/* Get driveNum = drive number of drive containing volume. */
	
	pBlock.volumeParam.ioNamePtr = nil;
	pBlock.volumeParam.ioVolIndex = 0;
	pBlock.volumeParam.ioVRefNum = vRefNum;;
	if (rCode = PBHGetVInfo(&pBlock, false)) return false;
	driveNum = pBlock.volumeParam.ioVDrvInfo;
	
	/*	Walk the drive queue until we find driveNum.  The second byte in
		the four flag bytes preceding the drive queue element is 8 or $48 
		if the drive is nonejectable. */
	
	curDrive = (DrvQEl*)(GetDrvQHdr())->qHead;
	while (true) {
		if (curDrive->dQDrive == driveNum) {
			flagByte = *((Ptr)curDrive - 3);
			return (flagByte != 8 && flagByte != 0x48);
		}
		curDrive = (DrvQEl*)curDrive->qLink;
		if (!curDrive) return false;
	}
}

/*______________________________________________________________________

	utl_ErrorAlert - Present an Error Alert.
	
	Entry:	tmpl = error message template.
				dlogID = resource id of error message dialog.
				filterProc = pointer to filter proc.
				p0,p1,p2,p3 = parameters to plug into template.
				
	The error message dialog must have:
	item 1 = OK button.
	item 2 = stop icon.
	item 3 = static text field with content "^0".
	item 4 = user item used to outline the OK button.
	
	This function adjusts the height of the alert and the static text
	field so that they are just high enough to contain the message.
_____________________________________________________________________*/

void utl_ErrorAlert (Str255 tmpl, short dlogID,
	ModalFilterProcPtr filterProc, Str255 p0, Str255 p1,
	Str255 p2, Str255 p3)

{
	SysBeep(0);
	MagicAlert(tmpl, dlogID, filterProc, p0, p1, p2, p3, 1, 3, 4, 1, 0);
}

/*______________________________________________________________________

	utl_ErrorAlertRez - Present an Error Alert.
	
	Entry:	rezID = resource id of STR# resource containing message
					template.
				index = index in STR# resource of message template.
				dlogID = resource id of error message dialog.
				filterProc = pointer to filter proc.
				p0, p1, p2, p3 = parameters to plug into template.
_____________________________________________________________________*/

void utl_ErrorAlertRez (short rezID, short index, short dlogID,
	ModalFilterProcPtr filterProc, 
	Str255 p0, Str255 p1, Str255 p2, Str255 p3)
	
{
	Str255			tmpl;			/* message template */
	
	GetIndString(tmpl, rezID, index);
	utl_ErrorAlert(tmpl, dlogID, filterProc, p0, p1, p2, p3);
}

/*______________________________________________________________________

	utl_FixStdFile - Fix Standard File Pacakge.
	
	This routine should be called before calling the Standard File 
	package if there's any chance that SFSaveDisk might specify
	a volume that has been unmounted.  Standard File gets confused if this
	happens and presents an alert telling the user that a "system error"
	has occurred.
	
	This routine checks to make sure that SFSaveDisk specifies a volume 
	that is still mounted.  If not, it sets it to the first mounted 
	volume, and it sets CurDirStore to the root directory on that volume.
_____________________________________________________________________*/
					
void utl_FixStdFile (void)

{
	ParamBlockRec	vBlock;			/* vol info param block */
												
	vBlock.volumeParam.ioNamePtr = nil;
	vBlock.volumeParam.ioVRefNum = -*(short*)SFSaveDisk;
	vBlock.volumeParam.ioVolIndex = 0;
	if (PBGetVInfo(&vBlock, false)) {
		vBlock.volumeParam.ioVolIndex = 1;
		(void) PBGetVInfo(&vBlock, false);
		*(short*)SFSaveDisk = -vBlock.volumeParam.ioVRefNum;
		if (*(short*)FSFCBLen > 0) *(long*)CurDirStore = fsRtDirID;
	}
}

/*______________________________________________________________________

	utl_FlashButton - Flash Button.
	
	Entry:	theButton = handle to button control..
				
	The push button is inverted for 8 ticks.  See HIN #10.
_____________________________________________________________________*/

void utl_FlashButton (ControlHandle theButton)

{
	Rect			box;					/* control rectangle */
	short			roundFactor;		/* rounded corner factor for InvertRoundRect */
	long			tickEnd;				/* tick count to end flash */
	
	box = (**theButton).contrlRect;
	roundFactor = (box.bottom - box.top)>>1;
	InvertRoundRect(&box, roundFactor, roundFactor);
	tickEnd = TickCount() + 8;
	while (TickCount() < tickEnd);
	InvertRoundRect(&box, roundFactor, roundFactor);
}

/*______________________________________________________________________

	utl_FlashDialogButton - Flash Dialog Button.
	
	Entry:	theDialog = pointer to dialog.
				itemNo = item number of button to flash.
				
	The push button is inverted for 8 ticks.  See HIN #10.
_____________________________________________________________________*/

void utl_FlashDialogButton (DialogPtr theDialog, short itemNo)

{
	short			itemType;			/* item type */
	Handle		item;					/* item handle */
	Rect			box;					/* item rectangle */
	
	GetDItem(theDialog, itemNo, &itemType, &item, &box);
	SetPort(theDialog);
	utl_FlashButton((ControlHandle)item);
}

/*______________________________________________________________________

	utl_FrameItem
	
	Entry:	theWindow = pointer to dialog window.
				itemNo = dialog item number.
	
	Exit:		dialog item rectangle framed.
				
	This function is for use as a Dialog Manager user item procedure.
	It is particularly useful for drawing rules (straight lines).  Simply
	define the user item rectangle with bottom=top+1 or right=left+1.
_____________________________________________________________________*/

pascal void utl_FrameItem (WindowPtr theWindow, short itemNo)

{
	short			itemType;			/* item type */
	Handle		item;					/* item handle */
	Rect			box;					/* item rectangle */

	GetDItem(theWindow, itemNo, &itemType, &item, &box);
	FrameRect(&box);
}

/*______________________________________________________________________

	utl_GestaltFlag - Get Gestalt Flag.
	
	Entry:	selector = Gestalt selector.
				bitNum = flag bit number.
	
	Exit:		function result = true if flag set.
_____________________________________________________________________*/

Boolean utl_GestaltFlag (OSType selector, short bitNum)

{
	long			result;			/* result from call to Gestalt */
	
	return !Gestalt(selector, &result) &&
		((result >> bitNum) & 1);
}

/*______________________________________________________________________

	utl_GetApplVol - Get the volume reference number of the application volume.
	
	Exit:		function result = volume reference number of the volume 
					containing the current application.
_____________________________________________________________________*/

short utl_GetApplVol (void)

{
	short			vRefNum;				/* vol ref num */

	GetVRefNum(*(short*)(CurMap), &vRefNum);
	return vRefNum;
}

/*______________________________________________________________________

	utl_GetBlessedWDRefNum - Get the working directory reference number of
		the Blessed Folder.
	
	Exit:		function result = wdRefNum of blessed folder
_____________________________________________________________________*/

short utl_GetBlessedWDRefNum (void)

{
	if (!GotSysEnviron) GetSysEnvirons();
	return TheWorld.sysVRefNum;
}

/*______________________________________________________________________

	utl_GetDialogControlValue - Get Dialog Control Value.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of control.
	
	Exit:		function result = value of control.
_____________________________________________________________________*/

short utl_GetDialogControlValue (DialogPtr theDialog, short itemNo)

{
	short		itemType;			/* item type */
	Handle	item;					/* handle to item */
	Rect		box;					/* item rectangle */

	GetDItem(theDialog, itemNo, &itemType, &item, &box);
	return GetCtlValue((ControlHandle)item);
}

/*______________________________________________________________________

	utl_GetDialogEditFieldNum - Get Dialog Edit Field Number.
	
	Entry:	theDialog = pointer to dialog record.
	
	Exit:		function result = field number of current TextEdit field.
_____________________________________________________________________*/

short utl_GetDialogEditFieldNum (DialogPtr theDialog)

{
	return ((DialogPeek)theDialog)->editField + 1;
}

/*______________________________________________________________________

	utl_GetDialogNumericValue - Get Dialog Numeric Value.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of control.
	
	Exit:		function result = item value.
_____________________________________________________________________*/

long utl_GetDialogNumericValue (DialogPtr theDialog, short itemNo)

{
	short		itemType;			/* item type */
	Handle	item;					/* handle to item */
	Rect		box;					/* item rectangle */
	Str255	theText;				/* item text */
	long		theNum;				/* the item value */

	GetDItem(theDialog, itemNo, &itemType, &item, &box);
	GetIText(item, theText);
	StringToNum(theText, &theNum);
	return theNum;
}

/*______________________________________________________________________

	utl_GetDialogPopupValue - Get Dialog Popup Menu Value.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of control.
	
	Exit:		val = value of currently selected menu item.
	
	(System 7 only)
_____________________________________________________________________*/

void utl_GetDialogPopupValue (DialogPtr theDialog, short itemNo, 
	Str255 val)
	
{
	short				itemType;		/* item type */
	ControlHandle	theControl;		/* handle to control */
	Rect				box;				/* item rectangle */
	MenuHandle		theMenu;			/* handle to menu */
	
	GetDItem(theDialog, itemNo, &itemType, (Handle*)&theControl, &box);
	theMenu = (**((popupPrivateData**)(**theControl).contrlData)).mHandle;
	GetItem(theMenu, GetCtlValue(theControl), val);
}

/*______________________________________________________________________

	utl_GetDialogText - Get Dialog Item Text.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of statText or editText item.
	
	Exit:		theText = the text.
_____________________________________________________________________*/

void utl_GetDialogText (DialogPtr theDialog, short itemNo, Str255 theText)

{
	short		itemType;			/* item type */
	Handle	item;					/* handle to item */
	Rect		box;					/* item rectangle */

	GetDItem(theDialog, itemNo, &itemType, &item, &box);
	GetIText(item, theText);
}

/*______________________________________________________________________

	utl_GetFontNumber - Get Font Number.
	
	Entry:	fontName = font name.
	
	Exit:		function result = true if font exists.
				fontNum = font number.
				
	Copied from TN 191.
_____________________________________________________________________*/

Boolean utl_GetFontNumber (Str255 fontName, short *fontNum)

{
	Str255		systemFontName;
	
	GetFNum(fontName, fontNum);
	if (*fontNum) {
		return true;
	} else {
		GetFontName(0, systemFontName);
		return EqualString(fontName, systemFontName, false, false);
	}
}

/*______________________________________________________________________

	utl_GetLongSleep - Get Long Sleep Time.
	
	Exit:			function result = long sleep time.
	
	Returns the largest positive long integer (0x7fffffff) if the 
	system version is > 0x04ff, else returns 50.  See TN 177.
_____________________________________________________________________*/

long utl_GetLongSleep (void)

{
	if (!GotSysEnviron) GetSysEnvirons();
	return (TheWorld.systemVersion > 0x04ff) ? 0x7fffffff : 50;
}

/*______________________________________________________________________

	utl_GetMBarHeight - Get Menu Bar Height
	
	Exit:		function result = menu bar height.
	
	See TN 117.
_____________________________________________________________________*/

short utl_GetMBarHeight (void)

{
	static short		mBHeight = 0;
	
	if (!mBHeight) {
		mBHeight = utl_Rom64() ? 20 : *(short*)MBarHeight;
	}
	return mBHeight;
}

/*______________________________________________________________________

	utl_GetNewControl - Get New Control.
	
	Entry:	controlID = resource id of CNTL resource.
				theWindow = pointer to window record.
				
	Exit:		function result = handle to new control record.
				
	This routine is identical to the Control Manager routine GetNewControl,
	except it does not release or make purgeable the CNTL resource.
_____________________________________________________________________*/

ControlHandle utl_GetNewControl (short controlID, 
	WindowPtr theWindow)
	
{
	Rect			boundsRect;			/* boundary rectangle */
	short			value;				/* initial control value */
	Boolean		visible;				/* true if visible */
	short			max;					/* max control value */
	short			min;					/* min control value */
	short			procID;				/* window proc id */
	long			refCon;				/* refCon field for window record */
	Str255		title;				/* window title */
	Handle		theRez;				/* handle to CNTL resource */
	
	theRez = GetResource('CNTL', controlID);
	boundsRect = *(Rect*)(*theRez);
	value = *(short*)(*theRez+8);
	visible = *(Boolean*)(*theRez+10);
	max = *(short*)(*theRez+12);
	min = *(short*)(*theRez+14);
	procID = *(short*)(*theRez+16);
	refCon = *(long*)(*theRez+18);
	utl_CopyPString(title, (unsigned char*)*theRez+22);
	return NewControl(theWindow, &boundsRect, title, visible, value,
		min, max, procID, refCon);
}

/*______________________________________________________________________

	utl_GetNewDialog - Get New Dialog.
	
	Entry:	dialogID = resource id of DLOG resource.
				dStorage = pointer to dialog record.
				behind = window to insert in back of.
				
	Exit:		function result = pointer to new dialog record.
				
	This routine is identical to the Dialog Manager routine GetNewDialog,
	except it does not release or make purgeable the DLOG resource.
_____________________________________________________________________*/

DialogPtr utl_GetNewDialog (short dialogID, Ptr dStorage, 
	WindowPtr behind)
	
{
	Rect			boundsRect;			/* boundary rectangle */
	short			procID;				/* window proc id */
	Boolean		visible;				/* true if visible */
	Boolean		goAwayFlag;			/* true if window has go away box */
	long			refCon;				/* refCon field for window record */
	Str255		title;				/* window title */
	Handle		theRez;				/* handle to DLOG resource */
	short			itemID;				/* rsrc id of item list */
	Handle		items;				/* handle to item list */
	
	theRez = GetResource('DLOG', dialogID);
	boundsRect = *(Rect*)(*theRez);
	procID = *(short*)(*theRez+8);
	visible = *(Boolean*)(*theRez+10);
	goAwayFlag = *(Boolean*)(*theRez+12);
	refCon = *(long*)(*theRez+14);
	itemID = *(short*)(*theRez+18);
	utl_CopyPString(title, (unsigned char*)*theRez+20);
	items = GetResource('DITL', itemID);
	return NewDialog(dStorage, &boundsRect, title, visible, procID, behind,
		goAwayFlag, refCon, items);
}

/*______________________________________________________________________

	utl_GetNewWindow - Get New Window.
	
	Entry:	windowID = resource id of WIND resource.
				wStorage = pointer to window record.
				behind = window to insert in back of.
				
	Exit:		function result = pointer to new window record.
				
	This routine is identical to the Window Manager routine GetNewWindow,
	except it does not release or make purgeable the WIND resource.
_____________________________________________________________________*/

WindowPtr utl_GetNewWindow (short windowID, Ptr wStorage, 
	WindowPtr behind)
	
{
	Rect			boundsRect;			/* boundary rectangle */
	short			procID;				/* window proc id */
	Boolean		visible;				/* true if visible */
	Boolean		goAwayFlag;			/* true if window has go away box */
	long			refCon;				/* refCon field for window record */
	Str255		title;				/* window title */
	Handle		theRez;				/* handle to WIND resource */
	
	theRez = GetResource('WIND', windowID);
	boundsRect = *(Rect*)(*theRez);
	procID = *(short*)(*theRez+8);
	visible = *(Boolean*)(*theRez+10);
	goAwayFlag = *(Boolean*)(*theRez+12);
	refCon = *(long*)(*theRez+14);
	utl_CopyPString(title, (unsigned char*)*theRez+18);
	return NewWindow(wStorage, &boundsRect, title, visible, procID, behind,
		goAwayFlag, refCon);
}

/*______________________________________________________________________

	utl_GetSysDirID - Get Directory ID of Blessed Folder.
	
	Exit:		function result = directory id of blessed folder, or 0 if
					MFS volume.
_____________________________________________________________________*/

long utl_GetSysDirID (void)

{
	short			sysVol;			/* sys vol ref num */
	FCBPBRec		fcbBlock;		/* file info param block */
	
	sysVol = utl_GetSysVol();
	if (utl_VolIsMFS(sysVol)) {
		return 0;
	} else {
		fcbBlock.ioNamePtr = nil;
		fcbBlock.ioVRefNum = 0;
		fcbBlock.ioRefNum = *((short*)SysMap);
		fcbBlock.ioFCBIndx = 0;
		(void) PBGetFCBInfo(&fcbBlock, false);
		return fcbBlock.ioFCBParID;
	}
}

/*______________________________________________________________________

	utl_GetSysVol - Get the volume reference number of the system volume.
	
	Exit:		function result = volume reference number of the volume 
					containing the currently active system file.
_____________________________________________________________________*/

short utl_GetSysVol (void)

{
	
	short			vRefNum;				/* vol ref num */

	GetVRefNum(*(short*)(SysMap), &vRefNum);
	return vRefNum;
}

/*______________________________________________________________________

	utl_GetSysWD - Get WDRefNum of Blessed Folder.
	
	Exit:		function result = WDRefNum of Blessed Folder.
_____________________________________________________________________*/

short utl_GetSysWD (void)

{
	if (!GotSysEnviron) GetSysEnvirons();
	return TheWorld.sysVRefNum;
}

/*______________________________________________________________________

	utl_GetWindGD - Get the GDevice containing a window.
	
	Entry:	theWindow = pointer to window.
	
	Exit:		gd = handle to GDevice, or nil if no color QD.
				screenRect = bounding rectangle of GDevice, or 
					qd.screenBits.bounds if no color QD.
				windRect = content rectangle of window, in global
					coords.
				hasMB = true if this screen contains the menu bar.
				
	The routine determines the GDevice (screen) containing the maximum
	intersection with a window.  See TN 79.
_____________________________________________________________________*/

void utl_GetWindGD (WindowPtr theWindow, GDHandle *gd, 
	Rect *screenRect, Rect *windRect, Boolean *hasMB)
	
{
	GrafPtr			savePort;			/* saved grafport */
	Rect				sectRect;			/* intersection rect */
	GDHandle			curDevice;			/* current GDevice */
	GDHandle			dominantDevice;	/* dominant GDevice */
	long				sectArea;			/* intersection area */
	long				maxArea;				/* max intersection area */
	
	*windRect = theWindow->portRect;
	GetPort(&savePort);
	SetPort(theWindow);
	LocalToGlobal((Point*)&windRect->top);
	LocalToGlobal((Point*)&windRect->bottom);
	if (utl_HaveColor()) {
		windRect->top -= titleBarHeight;
		curDevice = GetDeviceList();
		maxArea = 0;
		dominantDevice = nil;
		while (curDevice) {
			if (TestDeviceAttribute(curDevice, screenDevice) &&
				TestDeviceAttribute(curDevice, screenActive)) {
				SectRect(windRect, &(**curDevice).gdRect, &sectRect);
				sectArea = (long)(sectRect.right - sectRect.left) * 
					(long)(sectRect.bottom - sectRect.top);
				if (sectArea > maxArea) {
					maxArea = sectArea;
					dominantDevice = curDevice;
				}
			}
			curDevice = GetNextDevice(curDevice);
		}
		windRect->top += titleBarHeight;
		if (dominantDevice) {
			*gd = dominantDevice;
			*screenRect = (**dominantDevice).gdRect;
			*hasMB = dominantDevice == GetMainDevice();
		} else {
			*gd = nil;
			*screenRect = qd.screenBits.bounds;
			*hasMB = true;
		}	
	} else {
		*gd = nil;
		*screenRect = qd.screenBits.bounds;
		*hasMB = true;
	}
	SetPort(savePort);
}

/*______________________________________________________________________

	utl_GetVolFilCnt - Get the number of files on a volume.
	
	Entry:	volRefNum = volume reference number of volume.
	
	Exit:		function result = number of files on volume.
	
	For serdver volumes this function always returns 0.
_____________________________________________________________________*/

long utl_GetVolFilCnt (short volRefNum)

{
	HParamBlockRec		pBlock;		/* param block for PHBGetVInfo */
	
	pBlock.volumeParam.ioNamePtr = nil;
	pBlock.volumeParam.ioVRefNum = volRefNum;
	pBlock.volumeParam.ioVolIndex = 0;
	(void) PBHGetVInfo(&pBlock, false);
	return pBlock.volumeParam.ioVFilCnt;
}

/*______________________________________________________________________

	utl_HaveColor - Determine if system has color QuickDraw.

	Exit:		function result = true if we have color QD.
_____________________________________________________________________*/

Boolean utl_HaveColor (void)

{
	if (!GotSysEnviron) GetSysEnvirons();
	return TheWorld.hasColorQD;
}

/*______________________________________________________________________

	utl_HaveSound - Determine if system has the Sound Manager.

	Exit:		function result = true if we have sound.
_____________________________________________________________________*/

Boolean utl_HaveSound (void)

{
	if (!GotSysEnviron) GetSysEnvirons();
	return (TheWorld.systemVersion >= 0x0602);
}

/*______________________________________________________________________

	utl_HGetState - Get Memory Manager Flag Byte
	
	Entry:	h = handle.
	
	Exit:		function result = Flag byte.
				
	This routine is identical to the standard HGetState Memory Manager
	trap, except it also works with the 64K ROM.
_____________________________________________________________________*/

char utl_HGetState (Handle h)

{
	static short	hgs = 2;		/* 0 if HGetState does not exist
											1 if HGetState exists
											2 if we don't yet know (first call) */
	
	/* Find out whether the HGetState trap is implemented if this is 
		the first call. */
	
	if (hgs == 2) {
		if (!GotSysEnviron) GetSysEnvirons();
		if (TheWorld.machineType < 0) {
			hgs = false;
		} else {
			hgs = (NGetTrapAddress(_HGetState & 0x3ff, OSTrap) == 
				NGetTrapAddress(_Unimplemented & 0x3ff, OSTrap)) ? 0 : 1;
		}
	}
	
	/* If HGetState exists, call it, else get the flag bytes from the
		top byte of the master pointer. */
		
	if (hgs) {
		return HGetState(h);
	} else {
		return *(char*)h;
	}
}

/*______________________________________________________________________

	utl_HSetState - Set Memory Manager Flag Byte
	
	Entry:	h = handle.
				flags = flag byte.
				
	This routine is identical to the standard HSetState Memory Manager
	trap, except it also works with the 64K ROM.
_____________________________________________________________________*/

void utl_HSetState (Handle h, char flags)

{
	static short	hss = 2;		/* 0 if HSetState does not exist
											1 if HSetState exists
											2 if we don't yet know (first call) */
	
	/* Find out whether the HGetState trap is implemented if this is 
		the first call. */
	
	if (hss == 2) {
		if (!GotSysEnviron) GetSysEnvirons();
		if (TheWorld.machineType < 0) {
			hss = false;
		} else {
			hss = (NGetTrapAddress(_HSetState & 0x3ff, OSTrap) == 
				NGetTrapAddress(_Unimplemented & 0x3ff, OSTrap)) ? 0 : 1;
		}
	}
	
	/* If HSetState exists, call it, else set the flag bytes in the
		top byte of the master pointer. */
		
	if (hss) {
		HSetState(h, flags);
	} else {
		*(char*)h = flags;
	}
}

/*______________________________________________________________________

	utl_InitSpinCursor - Initialize animated cursor.
	
	Entry:	cursArray = array of handles to cursors.
				numCurs = number of cursors to rotate.
				tickInterval = interval between cursor rotations.
_____________________________________________________________________*/

void utl_InitSpinCursor (CursHandle *cursArray, short numCurs, 
	short tickInterval)

{
	CursHandle		h;

	CursArray = cursArray;
	CurCurs = 0;
	NumCurs = numCurs;
	TickInterval = tickInterval;
	LastTick = TickCount();
	h = *cursArray;
	SetCursor(*h);
}

/*______________________________________________________________________

	utl_InteractionPermitted - Check to See if User Interaction is Permitted.
	
	Exit:		function result = true if user interaction is permitted.
	
	This function checks the various rules concerning Apple event user
	interaction, as specified in IM VI pp 6-51 through 6-54.
	
	The function returns true if the system does not support Apple events,
	or if there is no current Apple event.
_____________________________________________________________________*/

Boolean utl_InteractionPermitted (void)

{
	AppleEvent			event;			/* the current Apple event */
	DescType				typeCode;		/* descriptor type code */
	Size					actualSize;		/* actual size of field data */
	short					interact;		/* event interaction level */
	short					source;			/* event source (see AppleEvents.h) */
	AEInteractAllowed	level;			/* permitted interaction level */

	if (!utl_GestaltFlag(gestaltAppleEventsAttr, 
		gestaltAppleEventsPresent)) return true;
	if (AEGetTheCurrentEvent(&event) || 
		event.descriptorType == typeNull) return true;
	if (AEGetAttributePtr(&event, keyInteractLevelAttr, typeShortInteger,
		&typeCode, (Ptr)&interact, sizeof(short), &actualSize)) return true;
	if (interact & kAENeverInteract) return false;
	if (AEGetAttributePtr(&event, keyEventSourceAttr, typeShortInteger,
		&typeCode, (Ptr)&source, sizeof(short), &actualSize)) return true;
	if (source <= kAESameProcess) return true;
	if (AEGetInteractionAllowed(&level)) return true;
	if (source == kAELocalProcess && level >= kAEInteractWithLocal) return true;
	return level == kAEInteractWithAll;
}

/*______________________________________________________________________

	utl_InvalGrow - Invalidate Grow Icon.
	
	Entry:			theWindow = pointer to window.
	
	This routine should be called before and after calling SizeWindow
	for windows with grow icons.
_____________________________________________________________________*/

void utl_InvalGrow (WindowPtr theWindow)

{
	Rect			r;			/* rect to be invalidated */
	
	r = theWindow->portRect;
	r.top = r.bottom - 15;
	r.left = r.right - 15;
	InvalRect(&r);
}

/*______________________________________________________________________

	utl_IsDAWindow - Check to see if a window is a DA.
	
	Entry:	theWindow = pointer to dialog window.
				
	
	Exit:		function result = true if DA window.
_____________________________________________________________________*/

Boolean utl_IsDAWindow (WindowPtr theWindow)

{
	return ((WindowPeek)theWindow)->windowKind < 0;
}

/*______________________________________________________________________

	utl_IsLaser - Check Printer for LaserWriter.
	
	Entry:	hPrint = handle to print record.
	
	Exit:		function result = true if LaserWriter.
	
_____________________________________________________________________*/

Boolean utl_IsLaser (THPrint hPrint)

{
	unsigned char	wDev;				/* printer device */

	wDev = (**hPrint).prStl.wDev >> 8;
	return wDev==3 || wDev==4;
}

/*______________________________________________________________________

	utl_KillAllProcesses - Kill All Processes.
	
	Entry:	idleProc = pointer to AppleEvent idle proc.
	
	Exit:		function result = error code.
				allKilled = true if all other processes have been killed.
				
	System 7.0 only.
	
	Alll processes are killed except for the current process.
	Users are given the oportunity to save unsaved docs.
	
	allKilled will be false if the user cancels an alert that is presented
	when an app is asked to quit, then switches to some other process
	without quiting the app.
_____________________________________________________________________*/

OSErr utl_KillAllProcesses (utl_AEIdleProcPtr idleProc, Boolean *allKilled)

{
	ProcessSerialNumber	psn;				/* psn of system process */
	AppleEvent				message;			/* quit AppleEvent */
	AEAddressDesc			targetAddr;		/* target address */
	OSErr						rCode;			/* result code */
	short						count;			/* process count */
	
	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kSystemProcess;
	if (rCode = AECreateDesc(typeProcessSerialNumber, (Ptr)&psn, 
		sizeof(ProcessSerialNumber), &targetAddr)) return rCode;
	if (rCode = AECreateAppleEvent(kCoreEventClass, kAEQuitAll,
		&targetAddr, kAutoGenerateReturnID, kAnyTransactionID, &message)) goto exit2;
	if (rCode = AESend(&message, nil, kAEWaitReply | kAECanInteract, kAENormalPriority,
		kAEDefaultTimeout, (IdleProcPtr)idleProc, nil)) goto exit1;
	if (rCode = utl_CountProcesses(&count)) goto exit1;
	*allKilled = count == 1;
	rCode = noErr;
exit1:
	AEDisposeDesc(&message);
exit2:
	AEDisposeDesc(&targetAddr);
	return rCode;
}

/*______________________________________________________________________

	utl_KillOneProcess - Kill One Process.
	
	Entry:	idleProc = pointer to AppleEvent idle proc.
				psn = process serial number of process to be killed.
				
	Exit:		function result = error code.
	
	System 7.0 only. The user is give an opportunity to save unsaved docs.
	
	The target process does not have to be AppleEvent-aware! It may even
	be a backbround-only process.
_____________________________________________________________________*/

OSErr utl_KillOneProcess (utl_AEIdleProcPtr idleProc, ProcessSerialNumber *psn)

{
	AppleEvent				message;			/* quit AppleEvent */
	AEAddressDesc			targetAddr;		/* target address */
	ProcessInfoRec			info;				/* process info */
	OSErr						rCode;			/* result code */
	EventRecord				theEvent;		/* event record */
	long						sleepTime;		/* sleep time */
	RgnHandle				mouseRgn;		/* mouse region */
	
	if (rCode = AECreateDesc(typeProcessSerialNumber, (Ptr)psn, 
		sizeof(ProcessSerialNumber), &targetAddr)) return rCode;
	if (rCode = AECreateAppleEvent(kCoreEventClass, kAEQuitApplication,
		&targetAddr, kAutoGenerateReturnID, kAnyTransactionID, &message)) goto exit2;
	rCode = AESend(&message, nil, kAENoReply | kAECanInteract, kAENormalPriority,
		kAEDefaultTimeout, (IdleProcPtr)idleProc, nil);
	if (rCode && rCode != procNotFound) goto exit1;
	info.processInfoLength = sizeof(ProcessInfoRec);
	info.processName = nil;
	info.processAppSpec = nil;
	do {
		GetNextEvent(everyEvent, &theEvent);
		(*idleProc)(&theEvent, &sleepTime, &mouseRgn);
	} while (!GetProcessInformation(psn, &info));
	rCode = noErr;
exit1:
	AEDisposeDesc(&message);
exit2:
	AEDisposeDesc(&targetAddr);
	return rCode;
}

/*______________________________________________________________________

	utl_LocateFinder - Locate the Finder.
				
	Exit:		function result = error code.
				finderFound = true if Finder running.
				finderFSpec = FSpec of Finder, if Finder running. Undefined
					if Finder not running.
	
	System 7.0 only.
_____________________________________________________________________*/

OSErr utl_LocateFinder (Boolean *finderFound, FSSpec *finderFSpec)
	
{
	OSErr						rCode;				/* result code */
	ProcessInfoRec			info;					/* process info */
	FSSpec					fSpec;				/* process file spec */
	ProcessSerialNumber	psn;					/* process serial number */

	*finderFound = false;
	info.processInfoLength = sizeof(ProcessInfoRec);
	info.processName = nil;
	info.processAppSpec = &fSpec;
	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kNoProcess;
	while (!(rCode = GetNextProcess(&psn))) {
		if (rCode = GetProcessInformation(&psn, &info)) return rCode;
		if (info.processType == 'FNDR' && info.processSignature == 'MACS') {
			*finderFound = true;
			*finderFSpec = fSpec;
			return noErr;
		}
	}
	if (rCode == procNotFound) rCode = noErr;
	return rCode;
}

/*______________________________________________________________________

	utl_LockControls - Lock Window Controls
	
	Entry:	theWindow = pointer to window.
	
	This routine moves all the control records in a window high and
	locks them.  It should be called immediately after creating a new
	window, and before drawing it.  It works around errors in the Control
	Manager which showed up when I was testing with TMON's heap scramble and
	purge option.
_____________________________________________________________________*/

void utl_LockControls (WindowPtr theWindow)

{
	ControlHandle		theControl;		/* handle to control */

	theControl = ((WindowPeek)theWindow)->controlList;
	while (theControl) {
		MoveHHi((Handle)theControl);
		HLock((Handle)theControl);
		theControl = (**theControl).nextControl;
	}
}

/*_____________________________________________________________________

	utl_ModalDialog - Present a Modal Dialog

	Entry:	dlogID = dialog ID.
				filterProc = pointer to filter proc.
				defaultItem = item number of default button, or 0 if none.
				cancelItem = item number of cancel button, or 0 if none.
				parameters following "theDialog" = list of useritem handlers, in
					the same order as the useritems in the dialog.
					
	Exit:		itemHit = item number of enabled item.
				theDialog = pointer to the dialog.
	
	This function centers the dialog on the main screen before presenting it,
	and it offers an easy way to plug in your useritem handlers.  
	
	If requested, Return or Enter is treated the same as a click on the
	default button.
	
	If requested, Command/Period or the Escape key is treated the same as a
	click on the Cancel button.
	
	The caller must dispose of the dialog.
_____________________________________________________________________*/

void utl_ModalDialog (short dlogID, ModalFilterProcPtr filterProc, 
	short defaultItem, short cancelItem, short *itemHit, DialogPtr *theDialog, ...)

{
	va_list			ap;				/* points to list of useritem handlers */
	Handle			hdlog;			/* handle to dialog */
	DialogPtr		dlog;				/* pointer to dialog */
	short				numItems;		/* number of items in dialog */
	short				i;					/* loop index */
	short				itemType;		/* item type */
	Handle			item;				/* handle to item */
	Rect				box;				/* item rectangle */
	
	InitCursor();
	va_start(ap, theDialog);
	hdlog = GetResource('DLOG', dlogID);
	HLock(hdlog);
	utl_CenterDlogRect(*(Rect**)hdlog, true);
	HUnlock(hdlog);
	dlog = GetNewDialog(dlogID, nil, (WindowPtr)-1);
	numItems = **(short**)(((DialogPeek)dlog)->items) + 1;
	IBeamHandle = GetCursor(iBeamCursor);
	for (i = 1; i <= numItems; i++) {
		GetDItem(dlog, i, &itemType, &item, &box);
		if ((itemType & 0x7f) == userItem) {
			item = va_arg(ap, Handle);
			SetDItem(dlog, i, itemType, item, &box);
		}
	}
	va_end(ap);
	ShowWindow((WindowPtr)dlog);
	Filter = filterProc;
	DefaultItem = defaultItem;
	CancelItem = cancelItem;
	ModalDialog(MetaFilter, itemHit);
	*theDialog = dlog;
}

/*_____________________________________________________________________

	utl_ModalDialogContinue - Continue Modal Dialog

	Entry:	filterProc = pointer to filter proc.
				defaultItem = item number of default button, or 0 if none.
				cancelItem = item number of cancel button, or 0 if none.
					
	Exit:		itemHit = item number of enabled item.
	
	This function should be called to continue a modal dialog if the dialog
	was initially presented via a call to utl_ModalDialog.
_____________________________________________________________________*/

void utl_ModalDialogContinue (ModalFilterProcPtr filterProc,
	short defaultItem, short cancelItem, short *itemHit)
	
{
	Filter = filterProc;
	DefaultItem = defaultItem;
	CancelItem = cancelItem;
	ModalDialog(MetaFilter, itemHit);
}

/*_____________________________________________________________________

	utl_OutlineDefaultButton - Outline Default Button.

	Entry:	theWindow = pointer to dialog window.
				itemNo = item number of outline useritem.
				
	This routine is for use as a useritem handler to outline default
	buttons in dialogs.  The default button must be the first item
	in the dialog item list.
_____________________________________________________________________*/

pascal void utl_OutlineDefaultButton (WindowPtr theWindow, short itemNo)

{
#pragma unused (itemNo)

	short				itemType;		/* item type */
	Handle			item;				/* handle to item */
	Rect				box;				/* item rectangle */

	GetDItem(theWindow, ok, &itemType, &item, &box);
	InsetRect(&box, -4, -4);
	PenSize(3, 3);
	FrameRoundRect(&box, 16, 16);
	PenSize(1, 1);
}	

/*______________________________________________________________________

	utl_PlotSmallIcon - Draw a small icon.
	
	Entry:	theRect = rectangle in which to draw the small icon.
				theHandle = handle to small icon.
					
	For best results, the rectangle should be exactly 16 pixels square.
_____________________________________________________________________*/

void utl_PlotSmallIcon (Rect *theRect, Handle theHandle)

{
	BitMap		srcBits;			/* Source bitmap for CopyBits */
	
	MoveHHi(theHandle);
	HLock(theHandle);
	srcBits.baseAddr = *theHandle;
	srcBits.rowBytes = 2;
	SetRect(&srcBits.bounds, 0, 0, 16, 16);
	CopyBits(&srcBits, &qd.thePort->portBits, &srcBits.bounds, theRect,
		srcCopy, nil);
	HUnlock(theHandle);
}

/*______________________________________________________________________

	utl_PlugParams - Plug parameters into message.
	
	Entry:	line1 = input line.
				p0, p1, p2, p3 = parameters.
					
	Exit:		line2 = output line.
					
	This routine works just like the toolbox routine ParamText.
	The input line may contain place-holders ^0, ^1, ^2, and ^3, 
	which are replaced by the parameters p0-p3.  The input line
	must not contain any other ^ characters.  The input and output lines
	may not be the same string.  Pass nil for parameters which don't
	occur in line1.  If the output line exceeds 255 characters it's
	truncated.
_____________________________________________________________________*/

void utl_PlugParams (Str255 line1, Str255 line2, Str255 p0, 
	Str255 p1, Str255 p2, Str255 p3)

{
	unsigned char		*in;			/* pointer to cur pos in input line */
	unsigned char		*out;			/* pointer to cur pos in output line */
	unsigned char		*inEnd;		/* pointer to end of input line */
	unsigned char		*outEnd;		/* pointer to end of output line */
	unsigned char		*param;		/* pointer to param to be plugged */
	short					len;			/* length of param */
	
	in = line1+1;
	out = line2+1;
	inEnd = line1 + 1 + *line1;
	outEnd = line2 + 256;
	while (in < inEnd ) {
		if (*in == '^') {
			in++;
			if (in >= inEnd) break;
			switch (*in++) {
				case '0':
					param = p0;
					break;
				case '1':
					param = p1;
					break;
				case '2':
					param = p2;
					break;
				case '3':
					param = p3;
					break;
				default:
					continue;
			}
			if (!param) continue;
			len = *param;
			if (out + len > outEnd) len = outEnd - out;
			memcpy(out, param+1, len);
			out += len;
		} else {
			if (out >= outEnd) break;
			*out++ = *in++;
		}
	}
	*line2 = out - (line2+1);
}

/*______________________________________________________________________

	utl_RedrawMenuBar - Redraw Menu Bar.
	
	Entry:	mBarState = array of flags. Each flag is true if the
					previous state of the corresponding menu was enabled.
				nMenus = number of menus.
				
	Exit:		Menu bar redrawn only if some menu has changed state.
				mBarState array updated.
_____________________________________________________________________*/

void utl_RedrawMenuBar (Boolean mBarState[], short nMenus)

{
	char						*pMenuList;				/* pointer into menu list */
	char						*pMenuListEnd;			/* pointer to end of menu list */
	Boolean					reDrawMenuBar;			/* true if menu bar must be redrawn */
	short						menuIndex;				/* index into menu list */
	MenuHandle				hMenu;					/* handle to menu */
	Boolean					enabled;					/* true if menu enabled */

	pMenuList = **(char***)MenuList;
	if (*(short*)pMenuList < 6*nMenus) return;
	pMenuListEnd = pMenuList + 6*nMenus;
	pMenuList += 6;
	reDrawMenuBar = false;
	menuIndex = 0;
	while (pMenuList <= pMenuListEnd) {
		hMenu = *(MenuHandle*)(pMenuList);
		enabled = (**hMenu).enableFlags & 1;
		if (enabled != mBarState[menuIndex]) {
			reDrawMenuBar = true;
			mBarState[menuIndex] = enabled;
		}
		pMenuList += 6;
		menuIndex++;
	}
	if (reDrawMenuBar) DrawMenuBar();
}

/*______________________________________________________________________

	utl_Restart - Restart
	
	Exit:		function result = error code.

	System 7 only. 
	
	Users are given the oportunity to save unsaved docs.
_____________________________________________________________________*/

OSErr utl_Restart (void)

{
	ProcessSerialNumber	psn;				/* psn of system process */
	AppleEvent				message;			/* restart AppleEvent */
	AEAddressDesc			targetAddr;		/* target address */
	OSErr						rCode;			/* result code */
	
	psn.highLongOfPSN = 0;
	psn.lowLongOfPSN = kSystemProcess;
	if (rCode = AECreateDesc(typeProcessSerialNumber, (Ptr)&psn, 
		sizeof(ProcessSerialNumber), &targetAddr)) return rCode;
	if (rCode = AECreateAppleEvent(kCoreEventClass, kAERestart,
		&targetAddr, kAutoGenerateReturnID, kAnyTransactionID, &message)) goto exit2;
	if (rCode = AESend(&message, nil, kAENoReply | kAECanInteract, kAENormalPriority,
		kAEDefaultTimeout, nil, nil)) goto exit1;
	rCode = noErr;
exit1:
	AEDisposeDesc(&message);
exit2:
	AEDisposeDesc(&targetAddr);
	return rCode;
}

/*______________________________________________________________________

	utl_RestoreWindowPos - Restore Window Position.
	
	Entry:	theWindow = pointer to window.
				userState = saved user state rectangle for the window.
				zoomed = true if window in zoomed state when saved.
				offset = pixel offset used in DragRect calls.
				computeStdState = pointer to function to compute standard state.
				computeDefState = pointer to function to compute default state.
	
	Exit:		window position, size, and zoom state restored.
				userState = new user state.
				
	See HIN 6: "When reopening a movable window, check its saved 
	position.  If the window is in a position to which the user could
	have dragged it, then leave it there.  If the window can be zoomed
	and was in the zoomed state when it was last closed, put it in the
	zoomed state again.  (Note that the current and previous zoomed states
	are not necessarily the same, since the window may be reopened on a
	different monitor.)  If the window is not in a position to which the
	user could have dragged it, then it must be relocated, so use the
	default location.  However, do not automatically use the default size
	when using the default location; if the entire window would be visible
	using the default location and stored size, then use the stored size."
	
	The "offset" parameter is usually 4.  When initializing the boundary rectangle
	for DragWindow calls, normally the boundary rectangle of the desktop gray
	region is inset by 4 pixels.  If some value other than 4 is used, it should
	be passed to RestoreWindowPos as the "offset" parameter.
	
	The computeStdState function is passed a pointer to the window.  Given
	the userState in the window zoom info, it must compute the standard
	(zoom) state in the window zoom info.  This is an application-dependent
	function.
	
	The computeDefState function must determine the default position and
	size of a new window, and return the result as a rectangle.  This may
	involve invocation of a staggering algorithm or some other algorithm.
	This is an application-dependent function.
_____________________________________________________________________*/

void utl_RestoreWindowPos (WindowPtr theWindow, Rect *userState, 
	Boolean zoomed, short offset,
	utl_ComputeStdStatePtr computeStdState,
	utl_ComputeDefStatePtr computeDefState)

{
	WindowPeek		w;					/* window pointer */
	short				userHeight;		/* height of userState */
	short				userWidth;		/* width of userState */
	Rect				r;					/* scratch rectangle */
	RgnHandle		rgn;				/* scratch region */
	Rect				stdState;		/* standard state */
	short				windHeight;		/* window height */
	short				windWidth;		/* window width */

	w = (WindowPeek)theWindow;
	if (!utl_CouldDrag(userState, offset)) {
		userHeight = userState->bottom - userState->top;
		userWidth = userState->right - userState->left;
		(*computeDefState)(theWindow, userState);
		if (!zoomed) {
			r = *userState;
			r.bottom = r.top + userHeight;
			r.right = r.left + userWidth;
			r.top -= titleBarHeight;
			InsetRect(&r, -1, -1);
			rgn = NewRgn();
			RectRgn(rgn, &r);
			DiffRgn(rgn, *(RgnHandle*)GrayRgn, rgn);
			if (EmptyRgn(rgn)) {
				userState->bottom = userState->top + userHeight;
				userState->right = userState->left + userWidth;
			}
			DisposeRgn(rgn);
		}
	}
	MoveWindow(theWindow, userState->left, userState->top, false);
	windHeight = userState->bottom - userState->top;
	windWidth = userState->right - userState->left;
	SizeWindow(theWindow, windWidth, windHeight, true);
	if (w->dataHandle && w->spareFlag) {
		(**((WStateData**)w->dataHandle)).userState = *userState;
		(*computeStdState)(theWindow);
		if (zoomed) {
			stdState = (**((WStateData**)w->dataHandle)).stdState;
			MoveWindow(theWindow, stdState.left, stdState.top, false);
			windHeight = stdState.bottom - stdState.top;
			windWidth = stdState.right - stdState.left;
			SizeWindow(theWindow, windWidth, windHeight, true);
		}
	}
}

/*______________________________________________________________________

	utl_ReviveFinder - Revive the Finder Process.
	
	Entry:	fSpec = Finder file spec.
	
	Exit:		function result = error code.
	
	System 7.0 only.
	
	This function can be used to bring the Finder back after it has
	been killed by a call to utl_KillAllProcesses.
_____________________________________________________________________*/

OSErr utl_ReviveFinder (FSSpec *fSpec)

{
	LaunchParamBlockRec	pBlock;		/* launch param block */
	OSErr						rCode;		/* result code */
	short						count;		/* counter */
	EventRecord				theEvent;	/* event record */
	
	pBlock.launchBlockID = extendedBlock;
	pBlock.launchEPBLength = extendedBlockLen;
	pBlock.launchFileFlags = 0;
	pBlock.launchControlFlags = launchContinue | launchNoFileFlags |
		launchDontSwitch;
	pBlock.launchAppSpec = fSpec;
	pBlock.launchAppParameters = nil;
	rCode = LaunchApplication(&pBlock);
	if (rCode) return rCode;
	count = 20;
	while (count--) GetNextEvent(everyEvent, &theEvent);
	return noErr;
}

/*______________________________________________________________________

	utl_Rom64 - Check to see if we have the old 64K ROM.
	
	Exit:		function result = true if 64K ROM.
_____________________________________________________________________*/

Boolean utl_Rom64 (void)

{
	return *(short*)ROM85 < 0;
}

/*______________________________________________________________________

	utl_SaveChangesAlert - Present Save Changes Alert.
	
	Entry:	tmpl = save changes message template.
				dlogID = resource id of save changes message dialog.
				filterProc = pointer to filter proc.
				p0,p1,p2,p3 = parameters to plug into template.
				
	Exit:		function result = item number of button selected.
				
	The save changes message dialog must have:
	item 1 = Save button.
	item 2 = Cancel button.
	item 3 = Don't Save button.
	item 4 = note icon.
	item 5 = static text field with content "^0".
	item 6 = user item used to outline the OK button.
	
	This function adjusts the height of the alert and the static text
	field so that they are just high enough to contain the message.
_____________________________________________________________________*/

short utl_SaveChangesAlert (Str255 tmpl, short dlogID, 
	ModalFilterProcPtr filterProc, Str255 p0, Str255 p1,
	Str255 p2, Str255 p3)

{
	return MagicAlert(tmpl, dlogID, filterProc, 
		p0, p1, p2, p3, 3, 5, 6, 1, 2);
}

/*______________________________________________________________________

	utl_SaveChangesAlertRez - Present Save Changes Alert.
	
	Entry:	rezID = resource id of STR# resource containing message
					template.
				index = index in STR# resource of message template.
				dlogID = resource id of message dialog.
				filterProc = pointer to filter proc.
				p0, p1, p2, p3 = parameters to plug into template.
				
	Exit:		function result = item number of button selected.
_____________________________________________________________________*/

short utl_SaveChangesAlertRez (short rezID, short index, short dlogID,
	ModalFilterProcPtr filterProc, 
	Str255 p0, Str255 p1, Str255 p2, Str255 p3)
	
{
	Str255			tmpl;			/* message template */
	
	GetIndString(tmpl, rezID, index);
	return utl_SaveChangesAlert(tmpl, dlogID, filterProc, p0, p1, p2, p3);
}

/*______________________________________________________________________

	utl_SaveWindowPos - Save Window Position.
	
	Entry:	theWindow = window pointer.
	
	Exit:		userState = user state rectangle of the window.
				zoomed = true if window zoomed.
				
	See HIN 6: "Before closing a movable window, check to see if its
	location or size have changed.  If so, save the new location and
	size.  If the window can be zoomed, save the user state and also 
	save whether or not the window is in the zoomed (standard) sate."
	
	We assume in this routine that the caller has already kept track
	of the fact that this window's location or size has changed, and that
	we do indeed need to save the location and size.
	
	This routine only works if the window's origin has not been offset.
_____________________________________________________________________*/

void utl_SaveWindowPos (WindowPtr theWindow, Rect *userState, Boolean *zoomed)

{
	GrafPtr		savedPort;		/* saved grafport */
	WindowPeek	w;					/* window pointer */
	Rect			curState;		/* current window rect, global coords */
	Rect			stdState;		/* standard (zoom) rect, global coords */
	Point			p;					/* scratch point */
	Rect			r;					/* scratch rect */
	
	GetPort(&savedPort);
	SetPort(theWindow);
	SetPt(&p, 0, 0);
	LocalToGlobal(&p);
	curState = theWindow->portRect;
	OffsetRect(&curState, p.h, p.v);
	w = (WindowPeek)theWindow;
	if (w->dataHandle && w->spareFlag) {
		/* This window supports zooming */
		/* Determine if window is zoomed.  The criteria is that both the
			top left and bottom right corners of the current window rectangle
			and the standard (zoom) rectangle must be within 7 pixels of each
			other.  This is the same algorithm as the one used by the standard
			system window definition function. */
		*zoomed = false;
		stdState = (**((WStateData**)w->dataHandle)).stdState;
		SetPt(&p, curState.left, curState.top);
		SetRect(&r, stdState.left, stdState.top, stdState.left, stdState.top);
		InsetRect(&r, -7, -7);
		if (PtInRect(p, &r)) {
			SetPt(&p, curState.right, curState.bottom);
			SetRect(&r, stdState.right, stdState.bottom, stdState.right,
				stdState.bottom);
			InsetRect(&r, -7, -7);
			*zoomed = PtInRect(p, &r);
		}
		if (*zoomed) {
			*userState = (**((WStateData**)w->dataHandle)).userState;
		} else {
			*userState = curState;
		}
	} else {
		/* This window does not support zooming. */
		*zoomed = false;
		*userState = curState;
	}
	SetPort(savedPort);
}

/*______________________________________________________________________

	utl_ScaleFontSize - Scale Font Size
	
	Entry:	theFontNum = font number.
				theFontSize = nominal font size.
				percent = percent change in size.
				laser = true if laserwriter.
	
	Exit:		function result = scaled font size.
				
	The nominal font size is multiplied by the percentage,
	then truncated.  For non-laserwriters it is then rounded down
	to the nearest font size which is available in that true size,
	without font manager scaling, or which can be generated by doubling
	an existing font size.
_____________________________________________________________________*/

short utl_ScaleFontSize (short theFontNum, short theFontSize, short percent,
	Boolean laser)
	
{
	short			nSize;				/* new size */
	short			x;						/* new test size */
	
	nSize = theFontSize * percent / 100;
	if (!laser) {
		x = nSize;
		while (x > 0) {
			if (RealFont(theFontNum, x)) break;
			if (!(x&1) && RealFont(theFontNum, x>>1)) break;
			x--;
		}
		if (x) nSize = x;
	}
	return nSize;
}

/*______________________________________________________________________

	utl_SetDialogControlValue - Set Dialog Control Value.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of control.
				val = new value for control.
_____________________________________________________________________*/

void utl_SetDialogControlValue (DialogPtr theDialog, short itemNo, short val)

{
	short		itemType;			/* item type */
	Handle	item;					/* handle to item */
	Rect		box;					/* item rectangle */

	GetDItem(theDialog, itemNo, &itemType, &item, &box);
	SetCtlValue((ControlHandle)item, val);
}

/*______________________________________________________________________

	utl_SetDialogNumericText - Set Dialog Numeric Text Item.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of statText or editText item.
				theNum = the number.
_____________________________________________________________________*/

void utl_SetDialogNumericText (DialogPtr theDialog, short itemNo, short theNum)

{
	Str255		theNumStr;		/* the number as a string */
	
	NumToString(theNum, theNumStr);
	utl_SetDialogText(theDialog, itemNo, theNumStr);
}

/*______________________________________________________________________

	utl_SetDialogPopupValue - Set Dialog Popup Control Value.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of statText or editText item.
				val = the new string value.
				defaultVal = the default numeric control value.
_____________________________________________________________________*/

void utl_SetDialogPopupValue (DialogPtr theDialog, short itemNo,
	Str255 val, short defaultVal)
	
{
	short				itemType;		/* item type */
	ControlHandle	theControl;		/* handle to control */
	Rect				box;				/* item rectangle */
	MenuHandle		theMenu;			/* handle to menu */
	short				numItems;		/* number of menu items */
	short				i;					/* loop index */
	Str255			menuItem;		/* menu item */
	
	GetDItem(theDialog, itemNo, &itemType, (Handle*)&theControl, &box);
	theMenu = (**((popupPrivateData**)(**theControl).contrlData)).mHandle;
	numItems = CountMItems(theMenu);
	for (i = 1; i <= numItems; i++) {
		GetItem(theMenu, i, menuItem);
		if (EqualString(menuItem, val, true, true)) {
			SetCtlValue(theControl, i);
			return;
		}
	}
	SetCtlValue(theControl, defaultVal);
}

/*______________________________________________________________________

	utl_SetDialogText - Set Dialog Item Text.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of statText or editText item.
				theText = the new text.
_____________________________________________________________________*/

void utl_SetDialogText (DialogPtr theDialog, short itemNo, Str255 theText)

{
	short		itemType;			/* item type */
	Handle	item;					/* handle to item */
	Rect		box;					/* item rectangle */

	GetDItem(theDialog, itemNo, &itemType, &item, &box);
	SetIText(item, theText);
}

/*______________________________________________________________________

	utl_SetDialogUserItem - Set Dialog User Item.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of statText or editText item.
				userItem = pointer to user item function.
_____________________________________________________________________*/

void utl_SetDialogUserItem (DialogPtr theDialog, short itemNo, Handle userItem)

{
	short		itemType;			/* item type */
	Handle	item;					/* handle to item */
	Rect		box;					/* item rectangle */

	GetDItem(theDialog, itemNo, &itemType, &item, &box);
	SetDItem(theDialog, itemNo, itemType, userItem, &box);
}

/*______________________________________________________________________

	utl_SpinCursor - Animate cursor.
	
	After calling InitSpinCursor to initialize an animated cursor, call
	SpinCursor periodically to make the cursor animate.
_____________________________________________________________________*/

void utl_SpinCursor (void)

{
	CursHandle		h;				/* handle to cursor */
	long				ticksNow;	/* current tick count */
	
	ticksNow = TickCount();
	if (ticksNow < LastTick + TickInterval) return;
	LastTick = ticksNow;
	CurCurs++;
	if (CurCurs >= NumCurs) CurCurs = 0;
	h = CursArray[CurCurs];
	SetCursor(*h);
}

/*______________________________________________________________________

	utl_StaggerWindow - Stagger a New Window
	
	Entry:	windRect = window portrect.
				initialOffset = intitial pixel offset of window from corner.
				offset = offset for subsequent staggered windows.
	
	Exit:		pos = window position.
	
	According to HIN 6, a new window should be positioned as follows:
	"The first document window should be positioned in the upper-left corner
	of the gray area of the main screen (the screen with the menu bar).
	Each additional independent window should be staggered from the upper-left
	corner of the screen that contains the largest portion of the
	frontmost window."  Also, "When a window is used or closed, its original
	position becomes available again.  The next window opened should use this
	position.  Similarly, if a window is moved onto a previously available
	position, that position becomes unavailable again."
	
	This routine implements these rules.  A position is considered to be
	"unavailable" if some other window is within (offset+1)/2 pixels of the
	position in both the vertical and horizontal directions.
	
	If all slots are occupied, the routine attempts to locate the first one
	which is occupied by only one other window.  If this attempt fails, it
	tries to locate one which is occupied by only two other windows, etc.
	Thus, if the screen is filled with staggered windows, subsequent windows
	will be staggered on top of the existing ones, forming a second "layer."
	If this layer fills up, a third layer is started, etc.
_____________________________________________________________________*/

void utl_StaggerWindow (Rect *windRect, short initialOffset, short offset, 
	Point *pos)

{
	GrafPtr			savedPort;		/* saved grafport */
	short				offsetDiv2;		/* offset/2 */
	short				windHeight;		/* window height */
	short				windWidth;		/* window width */
	WindowPtr		frontWind;		/* pointer to front window */
	GDHandle			gd;				/* GDevice */
	Rect				screenRect;		/* screen rectangle */
	Rect				junkRect;		/* window rectangle */
	Boolean			hasMB;			/* true if screen has menu bar */
	Point				initPos;			/* initial staggered window position */
	Point				curPos;			/* current staggered window position */
	WindowPtr		curWind;			/* pointer to current window */
	Point				windPos;			/* current window position */
	short				deltaH;			/* horizontal distance */
	short				deltaV;			/* vertical distance */
	short				layer;			/* layer number */
	short				nOccupied;		/* number windows occupying cur pos */
	Boolean			noPos;			/* true if no position is on screen */
	
	GetPort(&savedPort);
	offsetDiv2 = (offset+1)>>1;
	windHeight = windRect->bottom - windRect->top;
	windWidth = windRect->right - windRect->left;
	frontWind = FrontWindow();
	if (frontWind) {
		utl_GetWindGD(frontWind, &gd, &screenRect, &junkRect, &hasMB);
	} else {
		screenRect = qd.screenBits.bounds;
		hasMB = true;
	}
	if (hasMB) screenRect.top += utl_GetMBarHeight();
	SetPt(&initPos, screenRect.left + initialOffset + 1, 
		screenRect.top + initialOffset + titleBarHeight);
	layer = 1;
	while (true) {
		/* Test each layer number "layer", starting with 1 and incrementing by 1.
			Break out of the loop when we find a position curPos which
			is "occupied" by fewer than "layer" other windows. */
		curPos = initPos;
		noPos = true;
		while (true) {
			/* Test each possible position curPos.  Break out of the loop
				when we have exhaused the possible positions, or when we
				have located one which has < layer "occupants". */
			curWind = frontWind;
			if (curPos.v + windHeight >= screenRect.bottom ||
				curPos.h + windWidth >= screenRect.right) {
				break;
			}
			noPos = false;
			nOccupied = 0;
			while (curWind) {
				/* Scan the window list and count up how many of them "occupy"
					the current location curPos.  Break out of the loop when
					we reach the end of the list, or when the count is >=
					layer. */
				SetPt(&windPos, 0, 0);
				SetPort(curWind);
				LocalToGlobal(&windPos);
				deltaH = curPos.h - windPos.h;
				deltaV = curPos.v - windPos.v;
				if (deltaH < 0) deltaH = -deltaH;
				if (deltaV < 0) deltaV = -deltaV;
				if (deltaH <= offsetDiv2 && deltaV <= offsetDiv2) {
					nOccupied++;
					if (nOccupied >= layer) break;
				}
				curWind = (WindowPtr)(((WindowPeek)curWind)->nextWindow);
			}
			if (!curWind) break;
			curPos.h += offset;
			curPos.v += offset;
		}
		if (!curWind || noPos) break;
		layer++;
	}
	SetPort(savedPort);
	*pos = curPos;
}

/*_____________________________________________________________________

	utl_StandardAsciiChar - Check if Character is a Standard ASCII Printable
		Character.
		
	Entry:	c = character.
	
	Exit:		Function result = true if standard ASCII printable char.
_____________________________________________________________________*/

Boolean utl_StandardAsciiChar (char c)

{
	return (c >= 0x20) && (c <= 0x7e);
}

/*______________________________________________________________________

	utl_StopAlert - Present Stop Alert
	
	Entry:	alertID = resource id of alert.
				filterProc = pointer to filter proc.
				cancelItem = item number of cancel button, or 0 if
					none.
	
	Exit:		function result = item number
				
	This routine is identical to the Dialog Manager routine StopAlert,
	except that it centers the alert on the main window, and if requested,
	Command/Period or the Escape key is treated the same as a click on the 
	Cancel button.
_____________________________________________________________________*/

short utl_StopAlert (short alertID, ModalFilterProcPtr filterProc,
	short cancelItem)

{
	GrafPtr			savedPort;	/* save grafport */
	Handle			h;				/* handle to alert resource */
	short				result;		/* function result */
	
	GetPort(&savedPort);
	InitCursor();
	h = GetResource('ALRT', alertID);
	HLock(h);
	utl_CenterDlogRect(*(Rect**)h, false);
	Filter = filterProc;
	DefaultItem = ok;
	CancelItem = cancelItem;
	IBeamHandle = GetCursor(iBeamCursor);
	result = StopAlert(alertID, MetaFilter);
	HUnlock(h);
	SetPort(savedPort);
	return result;
}

/*______________________________________________________________________

	utl_SwitchDialogRadioButton - Switch Dialog Radio Button.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of radio button to set..
				first = item number of first radio button in group.
				last = item number of last radio button in group.
_____________________________________________________________________*/

void utl_SwitchDialogRadioButton (DialogPtr theDialog, short itemNo,
	short first, short last)

{
	short			i;			/* loop index */
	
	for (i = first; i <= last; i++)
		utl_SetDialogControlValue(theDialog, i, 0);
	utl_SetDialogControlValue(theDialog, itemNo, 1);
}

/*______________________________________________________________________

	utl_SysHasNotMgr - Check to See if System has Notification Manager.
	
	Exit:			function result = true if system has Notification
						Manager..
	
	The system version must be >= 6.0.
_____________________________________________________________________*/

Boolean utl_SysHasNotMgr (void)

{
	if (!GotSysEnviron) GetSysEnvirons();
	return TheWorld.systemVersion >= 0x0600;
}

/*______________________________________________________________________

	utl_SysHasPopUp - Check to See if System has Popup Menus.
	
	Exit:			function result = true if system has popup menus.
	
	The system version must be >= 4.1, the machine must have the 128 ROM or 
	later, and the PopUpMenuSelect trap must exist.
_____________________________________________________________________*/

Boolean utl_SysHasPopUp (void)

{
	if (!GotSysEnviron) GetSysEnvirons();
	if ((TheWorld.systemVersion < 0x0410) || (TheWorld.machineType == envMac)) {
		return false;
	} else {
		return NGetTrapAddress(_PopUpMenuSelect & 0x3ff, ToolTrap) !=
			NGetTrapAddress(_Unimplemented & 0x3ff, ToolTrap);
	}
}

/*______________________________________________________________________

	utl_TellMeTwiceAlert - Present Tell Me Twice Alert.
	
	Entry:	tmpl = tell me twice message template.
				dlogID = resource id of tell me twice message dialog.
				filterProc = pointer to filter proc.
				p0,p1,p2,p3 = parameters to plug into template.
				
	Exit:		function result = item number of button selected.
				
	The tell me twice dialog must have:
	item 1 = Doit button.
	item 2 = Cancel button.
	item 3 = stop icon.
	item 4 = static text field with content "^0".
	item 5 = user item used to outline the OK button.
	
	This function adjusts the height of the alert and the static text
	field so that they are just high enough to contain the message.
_____________________________________________________________________*/

short utl_TellMeTwiceAlert (Str255 tmpl, short dlogID, 
	ModalFilterProcPtr filterProc, Str255 p0, Str255 p1,
	Str255 p2, Str255 p3)

{
	return MagicAlert(tmpl, dlogID, filterProc, p0, p1, p2, p3, 2, 4, 5, 1, 2);
}

/*______________________________________________________________________

	utl_TellMeTwiceAlertRez - Present Tell Me Twice Alert.
	
	Entry:	rezID = resource id of STR# resource containing message
					template.
				index = index in STR# resource of message template.
				dlogID = resource id of dialog.
				filterProc = pointer to filter proc.
				p0, p1, p2, p3 = parameters to plug into template.
				
	Exit:		function result = item number of button selected.
_____________________________________________________________________*/

short utl_TellMeTwiceAlertRez (short rezID, short index, short dlogID,
	ModalFilterProcPtr filterProc, 
	Str255 p0, Str255 p1, Str255 p2, Str255 p3)
	
{
	Str255			tmpl;			/* message template */
	
	GetIndString(tmpl, rezID, index);
	return utl_TellMeTwiceAlert(tmpl, dlogID, filterProc, p0, p1, p2, p3);
}

/*______________________________________________________________________

	utl_ToggleDialogCheckBox - Toggle a dialog checkbox control.
	
	Entry:	theDialog = pointer to dialog record.
				itemNo = item number of statText or editText item.
				
	Exit:		function result = new control value (0=off, 1=on).
_____________________________________________________________________*/

short utl_ToggleDialogCheckBox (DialogPtr theDialog, short itemNo)

{
	short			newVal;		/* new value of checkbox control */

	newVal = 1 - utl_GetDialogControlValue(theDialog, itemNo);
	utl_SetDialogControlValue(theDialog, itemNo, newVal);
	return newVal;
}

/*______________________________________________________________________

	utl_VolIsMFS - Test for MFS volume.
	
	Entry:	vRefNum = volume reference number.
	
	Exit:		function result = true if volume is MFS.
_____________________________________________________________________*/

Boolean utl_VolIsMFS (short vRefNum)

{
	HParamBlockRec		vBlock;			/* vol info param block */
	
	vBlock.volumeParam.ioNamePtr = nil;
	vBlock.volumeParam.ioVolIndex = 0;
	vBlock.volumeParam.ioVRefNum = vRefNum;
	vBlock.volumeParam.ioVSigWord = 0xd2d7;    /* in case we don't have HFS */
	(void) PBHGetVInfo(&vBlock, false);
	return vBlock.volumeParam.ioVSigWord == 0xd2d7;
}

/*______________________________________________________________________

	utl_WaitNextEvent - Get Next Event.
	
	Entry:	eventMask = event mask.
				sleep = sleep interval.
				mouseRgn = mouse region.
	
	Exit:		theEvent = the next event.
				function result = true if event to be processed.
				
	This routine calls WaitNextEvent if the trap exists, otherwise it
	calls GetNextEvent.  
	
	If GetNextEvent is called, the sleep and mouseRgn parameters are 
	ignored.  SystemTask is also called.
				
	This routine also saves and restores the current grafport.  This is
	necessary to protect against some GetNextEvent trap patches which change
	the grafport without restoring it (e.g., the Flex screen saver).
_____________________________________________________________________*/

Boolean utl_WaitNextEvent (short eventMask, EventRecord *theEvent,
	long sleep, RgnHandle mouseRgn)

{
	GrafPtr			curPort;		/* pointer to current grafport */
	Boolean			result;		/* function result */
	static short	wne = 2;		/* 0 if WaitNextEvent does not exist
											1 if WaitNextEvent exists
											2 if we don't yet know (first call) */
	
	/* Find out whether the WaitNextEvent trap is implemented if this is 
		the first call. */
	
	if (wne == 2) {
		if (!GotSysEnviron) GetSysEnvirons();
		if (TheWorld.machineType < 0) {
			wne = false;
		} else {
			wne = (NGetTrapAddress(_WaitNextEvent & 0x3ff, ToolTrap) == 
				NGetTrapAddress(_Unimplemented & 0x3ff, ToolTrap)) ? 0 : 1;
		}
	}
	
	/* Save the port, call the trap, and restore the port. */
	
	GetPort(&curPort);
	if (wne) {
		result = WaitNextEvent(eventMask, theEvent, sleep, mouseRgn);
	} else {
		SystemTask();
		result = GetNextEvent(eventMask, theEvent);
	}
	SetPort(curPort);
	return result;
}
