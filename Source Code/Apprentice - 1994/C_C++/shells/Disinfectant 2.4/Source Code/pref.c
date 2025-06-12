/*______________________________________________________________________

	pref.c - Preferences Window Manager.
	
	Copyright © 1988, 1989, 1990 Northwestern University.  Permission is granted
	to use this code in your own projects, provided you give credit to both
	John Norstad and Northwestern University in your about box or document.
_____________________________________________________________________*/


#pragma load "precompile"
#include "utl.h"
#include "rez.h"
#include "glob.h"
#include "wstm.h"
#include "pref.h"
#include "help.h"
#include "misc.h"

#pragma segment pref

/*______________________________________________________________________

	Global Variables.
_____________________________________________________________________*/


static WindowPtr			PrefWindow = nil;		/* ptr to prefs window */
static DialogRecord		PrefWRecord;			/* prefs window record */
static WindowObject		PrefWindObject;		/* pref window object */
static MenuHandle			RepMenu = nil;			/* handle to rep creator popup menu */
static MenuHandle			DocMenu = nil;			/* handle to doc creator popup menu */
	
/*______________________________________________________________________

	Activate - Process an Activate Event.
_____________________________________________________________________*/


static void Activate (void)

{
	TEActivate(((DialogPeek)PrefWindow)->textH);
}
	
/*______________________________________________________________________

	Deactivate - Process a Deactivate Event.
_____________________________________________________________________*/


static void Deactivate (void)

{
	TEDeactivate(((DialogPeek)PrefWindow)->textH);
	misc_SetCursor();
}

/*______________________________________________________________________

	Help - Process Mouse Down Event in Help Mode.
	
	Entry:		where = mouse down location, local coords.
_____________________________________________________________________*/


static void Help (Point where)

{
	short				itemType;				/* dialog item type */
	Handle			itemHandle;				/* dialog item handle */
	Rect				box;						/* dialog item rectangle */
		
	GetDItem(PrefWindow, prefsHorRule1, &itemType, &itemHandle, &box);
	if (where.v < box.top) {
		help_Open(tagPrefBeep);
		return;
	};
	GetDItem(PrefWindow, prefsHorRule2, &itemType, &itemHandle, &box);
	if (where.v < box.top) {
		help_Open(tagPrefStation);
		return;
	};
	GetDItem(PrefWindow, prefsHorRule3, &itemType, &itemHandle, &box);
	if (where.v < box.top) {
		help_Open(tagPrefSave);
		return;
	};
	help_Open(tagPrefNotif);
}

/*______________________________________________________________________

	Close - Close the Window
_____________________________________________________________________*/


static void Close (void)

{
	Prefs.prefState.moved = PrefWindObject.moved;
	wstm_Save(PrefWindow, &Prefs.prefState);
	CloseDialog(PrefWindow);
	PrefWindow = nil;
}
	
/*______________________________________________________________________

	Edit - Process Edit Command.
	
	Entry:	cmd = which Edit command.
_____________________________________________________________________*/


static void Edit (EditCmd cmd)

{
	short				item;						/* dialog item number */
	TEHandle			textH;					/* handle to TextEdit record */
	short				selSize;					/* size of selected part of item */
	short				textLen;					/* text length */
	long				scrapLen;				/* size of scrap */
	Handle			scrapHandle;			/* handle to scrap */
	char				scrapChar;				/* scrap character */
			
	switch (cmd) {
		case cutCmd:
			DlgCut(PrefWindow);
			ZeroScrap();
			TEToScrap();
			break;
		case copyCmd:
			DlgCopy(PrefWindow);
			ZeroScrap();
			TEToScrap();
			break;
		case pasteCmd:
			TEFromScrap();
			item = ((DialogPeek)PrefWindow)->editField + 1;
			textH = ((DialogPeek)PrefWindow)->textH;
			selSize = (**textH).selEnd - (**textH).selStart;
			textLen = (**textH).teLength;
			scrapLen = TEGetScrapLen();
			if ( item == prefsBeep) {
				/* Number of beeps field - beep and return if there would 
					be more than two digits in the field after the paste. */
				if (textLen + scrapLen - selSize > 2) {
					SysBeep(beepDuration);
					return;
				};
				/* Beep and return if the scrap contains a non-digit. */
				scrapHandle = TEScrapHandle();
				while (scrapLen--) {
					scrapChar = *(*scrapHandle + scrapLen);
					if (scrapChar < '0' || scrapChar > '9') {
						SysBeep(beepDuration);
						return;
					};
				};
			} else {
				/* File creator type field - beep and return if there would
					be more than four characters in the field after the paste. */
				if (textLen + scrapLen - selSize > 4) {
					SysBeep(beepDuration);
					return;
				};
			};
			DlgPaste(PrefWindow);
			break;
		case clearCmd:
			DlgDelete(PrefWindow);
			break;
	};
}	
	
/*______________________________________________________________________

	Adjust - Adjust Menus.
_____________________________________________________________________*/


static void Adjust (void)

{
	MenuHandle				fileM;					/* handle to file menu */
	MenuHandle				editM;					/* handle to edit menu */
	MenuHandle				scanM;					/* handle to scan menu */
	
	fileM = GetMHandle(fileMID);
	editM = GetMHandle(editMID);
	scanM = GetMHandle(scanMID);
	EnableItem(fileM, closeCommand);
	DisableItem(fileM, saveAsCommand);
	DisableItem(fileM, pageSetupCommand);
	DisableItem(fileM, printCommand);
	DisableItem(fileM, printOneCommand);
	if (Scanning) {
		DisableItem(scanM, 0);
	} else {
		EnableItem(scanM, 0);
	};
	DisableItem(editM, undoCommand);
	EnableItem(editM, cutCommand);
	EnableItem(editM, copyCommand);
	EnableItem(editM, pasteCommand);
	EnableItem(editM, clearCommand);
	EnableItem(editM, 0);
}

/*______________________________________________________________________

	Periodic - Handle Periodic Tasks.
_____________________________________________________________________*/


static void Periodic (void)

{
	Point				mouseLoc;			/* mouse location */
	short				itemType;			/* dialog item type */
	Handle			itemHandle;			/* dialog item handle */
	Rect				beepBox;				/* beep count textedit item rectangle */
	Rect				repTypeBox;			/* report type textedit item rectangle */
	Rect				docTypeBox;			/* doc type textedit item rectangle */
	
	if (FrontWindow() != PrefWindow || HelpMode) return;
	SetPort(PrefWindow);
	GetMouse(&mouseLoc);
	GetDItem(PrefWindow, prefsBeep, &itemType, &itemHandle, &beepBox);
	GetDItem(PrefWindow, prefsRepType, &itemType, &itemHandle, &repTypeBox);
	GetDItem(PrefWindow, prefsDocType, &itemType, &itemHandle, &docTypeBox);
	if (PtInRect(mouseLoc, &beepBox) || PtInRect(mouseLoc, &repTypeBox) ||
		PtInRect(mouseLoc, &docTypeBox)) {
		SetCursor(*IBeamCurs);
	} else {
		InitCursor();
	};
}

/*______________________________________________________________________

	DialogPre - Preprocess Dialog Event
	
	Entry:		key = key pressed.
	
	Exit:			function result = true if event should be processed,
						false if it should be discarded.
_____________________________________________________________________*/


static Boolean DialogPre (short key)

{
	short				item;						/* dialog item number */
	TEHandle			textH;					/* handle to TextEdit record */
	short				selSize;					/* size of selected part of item */
	short				textLen;					/* text length */
		
	if (key == deleteKey || key == tabKey ||
		key == leftArrow || key == rightArrow) return true;
	item = ((DialogPeek)PrefWindow)->editField + 1;
	textH = ((DialogPeek)PrefWindow)->textH;
	selSize = (**textH).selEnd - (**textH).selStart;
	textLen = (**textH).teLength;
	if (item == prefsBeep) {
		/* Number of beeps field - beep and return false if the key
			is not a digit, or if there would be more than two digits in
			the field after the key is processed. */
		if (key < '0' || key > '9' || textLen + 1 - selSize > 2) {
			SysBeep(beepDuration);
			return false;
		};
	} else {
		/* File creator type field - beep and return false if there would
			be more than four characters in the field after the key is
			processed. */
		if (textLen + 1 - selSize > 4) {
			SysBeep(beepDuration);
			return false;
		};
	};
	return true;
}

/*______________________________________________________________________

	GetPopInfo - Get Pop Up Menu Info.
	
	Entry:	popItem = item number of popup menu user item.
				creType = creator type.
	
	Exit:		theMenu = handle to menu.
				numItems = number of items in menu.
				creList = handle to creator type list.
				creIndex = index in menu of application name.
_____________________________________________________________________*/


static void GetPopInfo (short popItem, OSType creType,
	MenuHandle *theMenu, short *numItems,
	OSType ***creList, short *creIndex)
	
{
	short		inx;				/* index in menu */

	if (popItem == prefsRepPop) {
		if (!RepMenu) RepMenu = GetMenu(prefsRepMenuID);
		*creList = (OSType**)GetResource('CREA', prefsRepCreaID);
		*theMenu = RepMenu;
	} else {
		if (!DocMenu) DocMenu = GetMenu(prefsDocMenuID);
		*creList = (OSType**)GetResource('CREA', prefsDocCreaID);
		*theMenu = DocMenu;
	};
	*numItems = CountMItems(*theMenu);
	for (inx = 0; inx < *numItems; inx++) {
		if (creType == *(**creList + inx)) break;
	};
	if (inx < *numItems) inx++;
	*creIndex = inx;
}

/*______________________________________________________________________

	PopUp - Pop Up Menu.
	
	Entry:	popItem = item number of popup menu user item.
				labelItem = item number of label static text item.
				editItem = item number of edittext item.
				menuID = resource id of popup menu.
				creType = pointer to Prefs creator type field.
				otherType = pointer to Prefs other creator type field.
_____________________________________________________________________*/


static void PopUp (short popItem, short labelItem, short editItem, short menuID,
	OSType *creType, OSType *otherType)

{
	MenuHandle		theMenu;				/* handle to menu */
	short				itemType;			/* dialog item type */
	Handle			itemHandle;			/* dialog item handle */
	Rect				labelBox;			/* label rectangle */
	Rect				popBox;				/* popup menu rectangle */
	Rect				typeBox;				/* textedit rectangle */
	Point				popPoint;			/* location of popup menu, global coords */
	long				result;				/* PopUpMenuSelect result */
	OSType			**creList;			/* handle to list of creator types */
	short				numItems;			/* number of items in list */
	short				oldIndex;			/* index in list of old selected item */
	short				newIndex;			/* index in list of new selected item */
	char				itemText[5];		/* creator type */
	
	/* Return if the system does not support popup menus. */
	
	if (!utl_SysHasPopUp()) return;
	
	/* Get popup info. */
	
	GetPopInfo(popItem, *creType, &theMenu, &numItems, &creList, &oldIndex);
	
	/* Check the currently selected item, and invert the label rectangle. */
	
	CheckItem(theMenu, oldIndex, true);
	GetDItem(PrefWindow, labelItem, &itemType, &itemHandle, &labelBox);
	InvertRect(&labelBox);
	
	/* Get the location of the popup menu and convert it to global coords. */
	
	GetDItem(PrefWindow, popItem, &itemType, &itemHandle, &popBox);
	popPoint = *(Point*)&popBox;
	LocalToGlobal(&popPoint);
	
	/* Present the popup menu. */
	
	InsertMenu(theMenu, -1);
	result = PopUpMenuSelect(theMenu, popPoint.v, popPoint.h, oldIndex);
	DeleteMenu(menuID);
	
	/* Uncheck the old selected item, and uninvert the lable rectangle. */
	
	CheckItem(theMenu, oldIndex, false);
	InvertRect(&labelBox);
	
	/* Process the new selected item. */
	
	if ((result >> 16) & 0xffff) {
		GetDItem(PrefWindow, editItem, &itemType, &itemHandle, &typeBox);
		newIndex = (result & 0xffff);
		if (oldIndex == numItems) {
			GetIText(itemHandle, itemText);
			*otherType = '    ';
			memcpy(otherType, itemText+1, *itemText);
		};
		if (newIndex == numItems) {
			*creType = *otherType;
		} else {
			*creType = *(*creList + newIndex - 1);
		};
		memcpy(itemText+1, creType, 4);
		*itemText = 4;
		SetIText(itemHandle, itemText);
		InvalRect(&popBox);
	};
}

/*______________________________________________________________________

	DialogPost - Postprocess Dialog Event
	
	Entry:		item = dialog item number.
_____________________________________________________________________*/


static void DialogPost (short item)

{
	short				itemType;				/* dialog item type */
	Handle			itemHandle;				/* dialog item handle */
	Rect				box;						/* dialog item rectangle */
	char				itemText[5];			/* dialog item text */
	long				beepCount;				/* beep count */
		
	switch (item) {
		case prefsBeep:
			GetDItem(PrefWindow, prefsBeep, &itemType, &itemHandle, &box);
			GetIText(itemHandle, itemText);
			StringToNum(itemText, &beepCount);
			Prefs.beepCount = beepCount;
			break;
		case prefsStation:
			Prefs.scanningStation = !Prefs.scanningStation;
			GetDItem(PrefWindow, prefsStation, &itemType, &itemHandle, &box);
			SetCtlValue((ControlHandle)itemHandle, Prefs.scanningStation);
			GetDItem(PrefWindow, prefsScan, &itemType, &itemHandle, &box);
			HiliteControl((ControlHandle)itemHandle, 
				Prefs.scanningStation ? 0 : 255);
			GetDItem(PrefWindow, prefsDisinfect, &itemType, &itemHandle, &box);
			HiliteControl((ControlHandle)itemHandle, 
				Prefs.scanningStation ? 0 : 255);
			break;
		case prefsScan:
		case prefsDisinfect:
			GetDItem(PrefWindow, prefsScan + Prefs.scanningStationOp,
				&itemType, &itemHandle, &box);
			SetCtlValue((ControlHandle)itemHandle, 0);
			Prefs.scanningStationOp = item - prefsScan;
			GetDItem(PrefWindow, item, &itemType, &itemHandle, &box);
			SetCtlValue((ControlHandle)itemHandle, 1);
			break;
		case prefsRepPop:
			PopUp(prefsRepPop, prefsRepLabel, prefsRepType, prefsRepMenuID,
				&Prefs.repCreator, &Prefs.repOtherCre);
			break;
		case prefsDocPop:
			PopUp(prefsDocPop, prefsDocLabel, prefsDocType, prefsDocMenuID,
				&Prefs.docCreator, &Prefs.docOtherCre);			
			break;
		case prefsRepType:
			GetDItem(PrefWindow, prefsRepType, &itemType, &itemHandle, &box);
			GetIText(itemHandle, itemText);
			Prefs.repCreator = '    ';
			memcpy(&Prefs.repCreator, itemText+1, *itemText);
			GetDItem(PrefWindow, prefsRepPop, &itemType, &itemHandle, &box);
			InvalRect(&box);
			break;
		case prefsDocType:
			GetDItem(PrefWindow, prefsDocType, &itemType, &itemHandle, &box);
			GetIText(itemHandle, itemText);
			Prefs.docCreator = '    ';
			memcpy(&Prefs.docCreator, itemText+1, *itemText);
			GetDItem(PrefWindow, prefsDocPop, &itemType, &itemHandle, &box);
			InvalRect(&box);
			break;
		case prefsDiamond:
		case prefsIcon:
		case prefsAlert:
			GetDItem(PrefWindow, prefsDiamond + Prefs.notifOption,
				&itemType, &itemHandle, &box);
			SetCtlValue((ControlHandle)itemHandle, 0);
			Prefs.notifOption = item - prefsDiamond;
			GetDItem(PrefWindow, item, &itemType, &itemHandle, &box);
			SetCtlValue((ControlHandle)itemHandle, 1);
			break;
	};
}

/*______________________________________________________________________

	DrawPopUp - Draw PopUp User Item.
	
	Entry:		theWindow = pointer to dialog window.
					itemNo = item number.
_____________________________________________________________________*/


static pascal void DrawPopUp (WindowPtr theWindow, short itemNo)

{
	MenuHandle	theMenu;				/* handle to menu */
	short			itemType;			/* item type */
	Handle		itemHandle;			/* item handle */
	Rect			box;					/* item rectangle */
	OSType		**creList;			/* handle to list of creator types */
	short			numItems;			/* number of items in list */
	short			creIndex;			/* index in list of selected item */
	Str255		appName;				/* application name */
	PolyHandle	triangle;			/* handle to triangle polygon */
	
	/* Get the currently selected application name. */
	
	GetPopInfo(itemNo, 
		itemNo == prefsRepPop ? Prefs.repCreator : Prefs.docCreator, 
		&theMenu, &numItems, &creList, &creIndex);
	GetItem(theMenu, creIndex, appName);
	
	/* Draw the currently selected application name. */
	
	GetDItem(theWindow, itemNo, &itemType, &itemHandle, &box);
	box.left += 4;
	TextBox(appName+1, *appName, &box, teJustLeft);
	box.left -= 4;

	/* Frame the rectangle and draw the drop shadow and triangle. */

	if (utl_SysHasPopUp()) {
		InsetRect(&box, -1, -1);
		FrameRect(&box);
		MoveTo(box.right, box.top+2);
		LineTo(box.right, box.bottom);
		LineTo(box.left+2, box.bottom);
		triangle = OpenPoly();
		MoveTo(box.right - 14, (box.top + box.bottom - 5)>>1);
		Line(10, 0);
		Line(-5, 5);
		Line(-5, -5);
		ClosePoly();
		PaintPoly(triangle);
		KillPoly(triangle);
	};
}

/*______________________________________________________________________

	DimNotif - Dim Notificiation Section User Item.
	
	Entry:		theWindow = pointer to dialog window.
					itemNo = item number.
					
	If the system does not support the Notification Manager, this user
	item dims the entire section.
_____________________________________________________________________*/


static pascal void DimNotif (WindowPtr theWindow, short itemNo)

{
#pragma unused (itemNo)

	short			itemType;			/* item type */
	Handle		itemHandle;			/* item handle */
	Rect			box;					/* item rectangle */
	
	if (!utl_SysHasNotMgr()) {
		PenMode(patBic);
		PenPat(qd.gray);
		GetDItem(theWindow, prefsNotPict, &itemType, &itemHandle, &box);
		PaintRect(&box);
		GetDItem(theWindow, prefsNotTitle1, &itemType, &itemHandle, &box);
		PaintRect(&box);
		GetDItem(theWindow, prefsNotTitle2, &itemType, &itemHandle, &box);
		PaintRect(&box);
		GetDItem(theWindow, prefsNotTitle3, &itemType, &itemHandle, &box);
		PaintRect(&box);
		PenMode(patCopy);
		PenPat(qd.black);
	};
}
	
/*______________________________________________________________________

	pref_Open - Open Pref Window.
_____________________________________________________________________*/


void pref_Open (void)

{
	short				i;					/* loop index */
	short				itemType;		/* dialog item type */
	Handle			itemHandle;				/* dialog item handle */
	Rect				box;				/* dialog item rectangle */
	char				itemText[5];	/* dialog item text */
	
	/* If the window is already open, activate it. */
	
	if (PrefWindow) {
		SelectWindow(PrefWindow);
		return;
	};
	
	/* Get the pref window and restore its state. */
	
	PrefWindow = wstm_Restore(true, prefsWindID, (Ptr)&PrefWRecord,
		&Prefs.prefState);
	SetPort(PrefWindow);
	
	/* Initialize the window object. */
	
	((WindowPeek)PrefWindow)->refCon = (long)&PrefWindObject;
	PrefWindObject.windKind = prefWind;
	PrefWindObject.moved = Prefs.prefState.moved;
	PrefWindObject.update = nil;
	PrefWindObject.activate = Activate;
	PrefWindObject.deactivate = Deactivate;
	PrefWindObject.resume = nil;
	PrefWindObject.suspend = nil;
	PrefWindObject.click = nil;
	PrefWindObject.help = Help;
	PrefWindObject.grow = nil;
	PrefWindObject.zoom = nil;
	PrefWindObject.key = nil;
	PrefWindObject.close = Close;
	PrefWindObject.disk = nil;
	PrefWindObject.save = nil;
	PrefWindObject.pageSetup = nil;
	PrefWindObject.print = nil;
	PrefWindObject.edit = Edit;
	PrefWindObject.adjust = Adjust;
	PrefWindObject.periodic = Periodic;
	PrefWindObject.dialogPre = DialogPre;
	PrefWindObject.dialogPost = DialogPost;
	
	/* Set the pointers to the user item handlers. */
	
	for (i = prefsFirstRule; i <= prefsLastRule; i++) {
		GetDItem(PrefWindow, i, &itemType, &itemHandle, &box);
		SetDItem(PrefWindow, i, itemType, (Handle)utl_FrameItem, &box);
	};
	GetDItem(PrefWindow, prefsRepPop, &itemType, &itemHandle, &box);
	SetDItem(PrefWindow, prefsRepPop, itemType, (Handle)DrawPopUp, &box);
	GetDItem(PrefWindow, prefsDocPop, &itemType, &itemHandle, &box);
	SetDItem(PrefWindow, prefsDocPop, itemType, (Handle)DrawPopUp, &box);
	GetDItem(PrefWindow, prefsDim, &itemType, &itemHandle, &box);
	SetDItem(PrefWindow, prefsDim, itemType, (Handle)DimNotif, &box);
	
	/* Initialize the control values and hilite states. */
	
	GetDItem(PrefWindow, prefsStation, &itemType, &itemHandle, &box);
	SetCtlValue((ControlHandle)itemHandle, Prefs.scanningStation);
	if (utl_SysHasNotMgr()) {
		GetDItem(PrefWindow, prefsDiamond + Prefs.notifOption, 
			&itemType, &itemHandle, &box);
		SetCtlValue((ControlHandle)itemHandle, 1);
	} else {
		for (i = prefsDiamond; i <= prefsAlert; i++) {
			GetDItem(PrefWindow, i, &itemType, &itemHandle, &box);
			HiliteControl((ControlHandle)itemHandle, 255);
		};
	};
	GetDItem(PrefWindow, prefsScan, &itemType, &itemHandle, &box);
	SetCtlValue((ControlHandle)itemHandle, 
		Prefs.scanningStationOp == checkOp);
	if (!Prefs.scanningStation) HiliteControl((ControlHandle)itemHandle, 255);
	GetDItem(PrefWindow, prefsDisinfect, &itemType, &itemHandle, &box);
	SetCtlValue((ControlHandle)itemHandle, 
		false);
	if (!Prefs.scanningStation) HiliteControl((ControlHandle)itemHandle, 255);
	
	/* Initialize the textedit fields. */
	
	GetDItem(PrefWindow, prefsRepType, &itemType, &itemHandle, &box);
	memcpy(itemText+1, &Prefs.repCreator, 4);
	*itemText = 4;
	while (*(itemText + *itemText) == ' ') (*itemText)--;
	SetIText(itemHandle, itemText);
	GetDItem(PrefWindow, prefsDocType, &itemType, &itemHandle, &box);
	memcpy(itemText+1, &Prefs.docCreator, 4);
	*itemText = 4;
	SetIText(itemHandle, itemText);
	GetDItem(PrefWindow, prefsBeep, &itemType, &itemHandle, &box);
	NumToString(Prefs.beepCount, itemText);
	SetIText(itemHandle, itemText);
	SelIText(PrefWindow, prefsBeep, 0, 32767);
	
	/* Show the window. */
	
	utl_LockControls(PrefWindow);
	ShowWindow(PrefWindow);
}