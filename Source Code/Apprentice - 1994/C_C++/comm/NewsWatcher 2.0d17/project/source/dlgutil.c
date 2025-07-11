/*----------------------------------------------------------------------------

	dlgutil.c

	This module contains miscellaneous dialog utility routines.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "MacTCPCommonTypes.h"
#include <Packages.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>

#include "glob.h"
#include "dlgutil.h"
#include "util.h"
#include "activate.h"
#include "draw.h"
#include "popup.h"
#include "menus.h"


#define kNewsWatcherSmallIconID		128		/* Small program icon */

#define kErrDlg				130			/* Error message dialog */

#define kServerErrDlg		131			/* News server error message dialog */

#define kNewsServerErrorMessageStaticTextItem		3
#define kNewsServerErrorMessageGrayBorderUserItem	4
#define kNewsServerErrorMessageTheMessageItem		5
#define kNewsServerErrorMessageBozoTextUserItem 	6
#define kNewsServerErrorMessageBozoText				129

#define kMailFTPErrDlg		139			/* Mail & FTP server error message dialog */

#define kMailFTPErrorMessageGrayBorderUserItem		4
#define kMailFTPErrorMessageTheMessageItem			5

#define kUnexpectedErrDlg	143			/* Unexpected error dialog */



static Str255 gServerErrorMessageStaticText;
static Str255 gServerErrorMessageTheMessage;



typedef struct TDialogItemInfo {
	Boolean numeric;				/* true if numeric textedit field */
	Boolean returnIsLegal;			/* true if Return is legal in textedit field */
	short popupTypeinItem;			/* item number of partner typein field for popup item,
									   or 0 if none */
	char keyEquivalent;				/* command key equivalent for button, or 0 if none */
	short maxLength;				/* max length of textedit field */
	char *password;					/* pointer to password if password field, or nil */
	Boolean readOnly;				/* true if read only text field with scroll bar */
	TEHandle readOnlyTE;			/* textedit handle for read only text field */
	ControlHandle vScroll;			/* handle to vertical scroll bar control if read only
									   text field, or nil if none */
} TDialogItemInfo;

typedef struct TDialogInfo {
	Boolean returnIsOK;				/* true if return is equivalent to OK button */
	Boolean hasCancelButton;		/* true if dialog has Cancel button */
	CustomCopyCommandHandler customCopyCommandHandler;	/* pointer to custom Copy command
														   handler, or nil if none */
	TDialogItemInfo	itemInfo[];		/* array of item information */
} TDialogInfo;




/*----------------------------------------------------------------------------
	DlgGetCheck 
	
	Gets the state of a checkbox in a dialog.
	
	Entry:	dlg = pointer to dialog.
			item = item number of checkbox control.
			
	Exit:	function result = true if checked, false if unchecked.
----------------------------------------------------------------------------*/

Boolean DlgGetCheck (DialogPtr dlg, short item)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	return (GetCtlValue((ControlHandle)itemHandle) == 1);
}



/*----------------------------------------------------------------------------
	DlgSetCheck 
	
	Sets the state of a checkbox in a dialog.
	
	Entry:	dlg = pointer to dialog.
			item = item number of checkbox control.
			value = true to check, false to uncheck.
----------------------------------------------------------------------------*/

void DlgSetCheck (DialogPtr dlg, short item, Boolean value)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	SetCtlValue((ControlHandle)itemHandle,value);
}



/*----------------------------------------------------------------------------
	DlgToggleCheck 
	
	Toggles the state of a checkbox in a dialog.
	
	Entry:	dlg = pointer to dialog.
			item = item number of checkbox control.

----------------------------------------------------------------------------*/

void DlgToggleCheck (DialogPtr dlg, short item)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	short value;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	value = (GetCtlValue((ControlHandle)itemHandle) == 1) ? 0 : 1;
	SetCtlValue((ControlHandle)itemHandle,value);
}



/*----------------------------------------------------------------------------
	DlgGetControl
	
	Gets the control handle for a dialog item.
	
	Entry:	dlg = pointer to dialog.
			item = item number of control item.
			
	Exit:	function result = handle to control.
----------------------------------------------------------------------------*/

ControlHandle DlgGetControl (DialogPtr dlg, short item)
{
	Handle itemHandle;
	short itemType;
	Rect box;

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	return (ControlHandle)itemHandle;
}



/*----------------------------------------------------------------------------
	DlgGetCtlValue 
	
	Gets the value of a control in a dialog.
	
	Entry:	dlg = pointer to dialog.
			item = item number of  control.
			
	Exit:	function result = value of control.
----------------------------------------------------------------------------*/

short DlgGetCtlValue (DialogPtr dlg, short item)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	return GetCtlValue((ControlHandle)itemHandle);
}



/*----------------------------------------------------------------------------
	DlgSetCtlValue 
	
	Sets the value of a control in a dialog.
	
	Entry:	dlg = pointer to dialog.
			item = item number of checkbox control.
			value = new control value.
----------------------------------------------------------------------------*/

void DlgSetCtlValue (DialogPtr dlg, short item, short value)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	SetCtlValue((ControlHandle)itemHandle, value);
}



/*----------------------------------------------------------------------------
	DlgGetNumber 
	
	Gets the value of a numeric dialog item.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
			
	Exit:	function result = numeric value of item.
----------------------------------------------------------------------------*/

short DlgGetNumber (DialogPtr dlg, short item)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	Str255 valStr;
	long value;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	GetIText(itemHandle, valStr);
	StringToNum(valStr, &value);
	return (short)value;
}



/*----------------------------------------------------------------------------
	DlgSetNumber 
	
	Sets the value of a numeric dialog item.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
			value = new value for item.
----------------------------------------------------------------------------*/

void DlgSetNumber (DialogPtr dlg, short item, short value)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	Str255 valStr;
	
	NumToString(value, valStr);
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	SetIText(itemHandle, valStr);
}



/*----------------------------------------------------------------------------
	DlgGetCString 
	
	Gets the value of a string dialog item.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
			
	Exit:	value = the string (C format).
----------------------------------------------------------------------------*/

void DlgGetCString (DialogPtr dlg, short item, char *value)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	GetIText(itemHandle, (StringPtr)value);
	p2cstr((StringPtr)value);
}



/*----------------------------------------------------------------------------
	DlgSetCString 
	
	Sets the value of a string dialog item.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
			value = new string value (C format).
----------------------------------------------------------------------------*/

void DlgSetCString (DialogPtr dlg, short item, char *value)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	c2pstr(value);
	SetIText(itemHandle, (StringPtr)value);
	p2cstr((StringPtr)value);
}



/*----------------------------------------------------------------------------
	DlgGetPString
	
	Gets the value of a string dialog item.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
			
	Exit:	value = the string (Pascal format).
----------------------------------------------------------------------------*/

void DlgGetPString (DialogPtr dlg, short item, StringPtr value)
{
	Handle itemHandle;
	short itemType;
	Rect box;

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	GetIText(itemHandle, value);
}



/*----------------------------------------------------------------------------
	DlgSetPString
	
	Sets the value of a string dialog item.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
			value = new string value (Pascal format).
----------------------------------------------------------------------------*/

void DlgSetPString (DialogPtr dlg, short item, StringPtr value)
{
	Handle itemHandle;
	short itemType;
	Rect box;

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	SetIText(itemHandle, value);
}



/*----------------------------------------------------------------------------
	DlgEnableItem
	
	Enables or disables an item in a dialog box. Checks to see if the item 
	is a control and calls HiliteControl if so. Also redraws the outline 
	around the default button if it's what's being enabled or disabled.
	
	Entry:	dlg = pointer to dialog.
			item = item number of checkbox control.
			enabled = new enabled state.
----------------------------------------------------------------------------*/

void DlgEnableItem (DialogPtr dlg, short item, Boolean enabled)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	Boolean	oldEnable;

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	oldEnable = (itemType & itemDisable) == 0;
	itemType = enabled ? itemType & (~itemDisable) : itemType | itemDisable;
	SetDItem(dlg, item, itemType, itemHandle, &box);
	
	SetPort(dlg);
	if ((itemType & ctrlItem) && oldEnable != enabled) {
		if (item == ok) {
			InsetRect(&box, -4, -4);
			InvalRect(&box);
			EraseRect(&box);
		}
		HiliteControl((ControlHandle)itemHandle, enabled ? 0 : 255);
	}
}



/*----------------------------------------------------------------------------
	DlgSetUserItem
	
	Sets the procedure for a user item in a dialog.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
			proc = pointer to user item proc.
----------------------------------------------------------------------------*/

void DlgSetUserItem (DialogPtr dlg, short item, UserItemProcPtr proc)
{
	Handle itemHandle;
	short itemType;
	Rect box;

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	SetDItem(dlg, item, itemType, (Handle)proc, &box);
}



/*----------------------------------------------------------------------------
	DlgSetPict
	
	Set a dialog picture.
	
	Entry:	dlg = pointer to dialog.
			item = item number of picture item.
			pict = handle to picture.
----------------------------------------------------------------------------*/


void DlgSetPict (DialogPtr dlg, short item, PicHandle pict)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	
	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	SetDItem(dlg, item, itemType, (Handle)pict, &box);
}



/*----------------------------------------------------------------------------
	DlgEraseItems
	
	Erase and invalidate the area occupied by the specified items in a dialog.
	
	Entry:	dlg = pointer to dialog.
			first = first item number to erase.
			last = last item number to erase.
----------------------------------------------------------------------------*/

void DlgEraseItems (DialogPtr dlg, short first, short last)
{
	Rect itemRect;
	Handle itemHandle;
	short itemType, i;
	
	SetPort(dlg);
	for (i = first; i <= last; i++) {
		GetDItem(dlg, i, &itemType, &itemHandle, &itemRect);
		if ((itemType & editText) ||
			i == ok && (itemType & ctrlItem) && (itemType & 0x03) == btnCtrl) {
			InsetRect(&itemRect, -4, -4);	/* Erase the border too */
		}
		EraseRect(&itemRect);
		InvalRect(&itemRect);
	}
}



/*----------------------------------------------------------------------------
	DlgGrayBorderItem
	
	A user item procedure to draw a dotted gray border around itself.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
----------------------------------------------------------------------------*/

pascal void DlgGrayBorderItem(DialogPtr dlg, short item)
{
	Handle itemHandle;
	short itemType;
	Rect box;
	PenState savePen;

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	GetPenState(&savePen);
	PenNormal();
	PenPat(qd.gray);
	FrameRect(&box);
	SetPenState(&savePen);
}



/*----------------------------------------------------------------------------
	DlgFlashButton
 
	Flashes a button.
	
	Entry:	dlg = pointer to dialog.
			item = item number of button to flash.
----------------------------------------------------------------------------*/

void DlgFlashButton (DialogPtr dlg, short item)
{
	short itemType;
	ControlHandle theItem;
	Rect box;
	long myticks;
	
	if (item > 0) {
		GetDItem(dlg, item, &itemType, (Handle*)&theItem, &box);
		HiliteControl(theItem, 1);
		Delay(8, &myticks);
		HiliteControl(theItem, 0);
	}
}



/*----------------------------------------------------------------------------
	SetDialogCustomCopyHandler
 
	Sets a custom Copy command handler for a dialog.
	
	Entry:	dlg = pointer to dialog.
			theHandler = pointer to custom Copy command handler.
----------------------------------------------------------------------------*/

void SetDialogCustomCopyHandler (DialogPtr dlg, CustomCopyCommandHandler theHandler)
{
	TDialogInfo **info;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).customCopyCommandHandler = theHandler;
}



/*----------------------------------------------------------------------------
	SetItemNumeric
 
	Sets the "numeric" item info.
	
	Entry:	dlg = pointer to dialog.
			item = item number of textedit field.
			numeric = true if field is numeric (can only contain digits).
----------------------------------------------------------------------------*/

void SetItemNumeric (DialogPtr dlg, short item, Boolean numeric)
{
	TDialogInfo **info;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).itemInfo[item-1].numeric = numeric;
}



/*----------------------------------------------------------------------------
	SetItemReturnIsLegal
 
	Sets the "returnIsLegal" item info.
	
	Entry:	dlg = pointer to dialog.
			item = item number of textedit field.
			returnIsLegal = true if the Return character is legal.
----------------------------------------------------------------------------*/

void SetItemReturnIsLegal (DialogPtr dlg, short item, Boolean returnIsLegal)
{
	TDialogInfo **info;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).itemInfo[item-1].returnIsLegal = returnIsLegal;
}



/*----------------------------------------------------------------------------
	SetItemPopupTypeinItem
 
	Sets the "popupTypeinItem" item info.
	
	Entry:	dlg = pointer to dialog.
			item = item number of popup menu control.
			popupTypeInItem = item number of corresponding typein textedit
				field, or 0 if none.
----------------------------------------------------------------------------*/

void SetItemPopupTypeinItem (DialogPtr dlg, short item, short popupTypeinItem)
{
	TDialogInfo **info;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).itemInfo[item-1].popupTypeinItem = popupTypeinItem;
}



/*----------------------------------------------------------------------------
	SetItemKeyEquivalent
 
	Sets the "keyEquivalent" item info.
	
	Entry:	dlg = pointer to dialog.
			item = item number of push button control.
			keyEquivalent = keyboard equivalent for control, or 0 if none.
----------------------------------------------------------------------------*/

void SetItemKeyEquivalent (DialogPtr dlg, short item, char keyEquivalent)
{
	TDialogInfo **info;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).itemInfo[item-1].keyEquivalent = keyEquivalent;
}



/*----------------------------------------------------------------------------
	SetItemMaxLength
 
	Sets the "max length" item info.
	
	Entry:	dlg = pointer to dialog.
			item = item number of textedit field.
			maxLength = max length of field.
----------------------------------------------------------------------------*/

void SetItemMaxLength (DialogPtr dlg, short item, short maxLength)
{
	TDialogInfo **info;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).itemInfo[item-1].maxLength = maxLength;
}



/*----------------------------------------------------------------------------
	SetItemPassword
 
	Sets the "password" item info.
	
	Entry:	dlg = pointer to dialog.
			item = item number of textedit field.
			password = pointer to password string, or nil if none.
----------------------------------------------------------------------------*/

void SetItemPassword (DialogPtr dlg, short item, char *password)
{
	TDialogInfo **info;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).itemInfo[item-1].password = password;
}



/*----------------------------------------------------------------------------
	ReadOnlyUserItem
	
	The user item procedure to draw the contents of a read only text field.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
----------------------------------------------------------------------------*/

static pascal void ReadOnlyUserItem(DialogPtr dlg, short item)
{
	TDialogInfo **info;
	Handle itemHandle;
	short itemType;
	Rect box;
	PenState savePen;
	
	SetPort(dlg);
	
	info = (TDialogInfo**)GetWRefCon(dlg);

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	GetPenState(&savePen);
	PenNormal();
	PenPat(qd.black);
	FrameRect(&box);
	SetPenState(&savePen);
	
	TEUpdate(&box, (**info).itemInfo[item-1].readOnlyTE);
}



/*----------------------------------------------------------------------------
	SetItemReadOnly
 
	Sets the "read only" item info.
	
	Entry:	dlg = pointer to dialog.
			item = item number of textedit field.
			theText = handle to contents of textedit field.
			fontNum = font number for textedit field.
			fontSize = font size for textedit field.
----------------------------------------------------------------------------*/

void SetItemReadOnly (DialogPtr dlg, short item, Handle theText, short fontNum, short fontSize)
{
	TDialogInfo **info;
	short itemType;
	Handle itemHandle;
	Rect box;
	TEHandle hTE;
	short saveFont, saveSize;
	Boolean needScrollBar;
	FontInfo fontInfo;
	short lineHeight, numLinesInBox;
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	(**info).itemInfo[item-1].readOnly = true;
	DlgSetUserItem(dlg, item, ReadOnlyUserItem);

	GetDItem(dlg, item, &itemType, &itemHandle, &box);

	SetPort(dlg);
	saveFont = dlg->txFont;
	saveSize = dlg->txSize;
	TextFont(fontNum);
	TextSize(fontSize);
	GetFontInfo(&fontInfo);
	lineHeight = fontInfo.ascent+fontInfo.descent+fontInfo.leading;
	numLinesInBox = (box.bottom - box.top - 2*kTextMargin) / lineHeight;
	box.bottom = box.top + lineHeight * numLinesInBox + 2*kTextMargin;
	SetDItem(dlg, item, itemType, itemHandle, &box);
	
	InsetRect(&box, kTextMargin, kTextMargin);
	(**info).itemInfo[item-1].readOnlyTE = hTE = TENew(&box, &box);
	HLock(theText);
	TESetText(*theText, GetHandleSize(theText), hTE);
	HUnlock(theText);
	(**info).itemInfo[item-1].vScroll = nil;
	
	needScrollBar = (**hTE).nLines > numLinesInBox;
	
	if (needScrollBar) {
		TEDispose(hTE);
		box.right -= 16;
		(**info).itemInfo[item-1].readOnlyTE = hTE = TENew(&box, &box);
		HLock(theText);
		TESetText(*theText, GetHandleSize(theText), hTE);
		HUnlock(theText);
		box.right += 16;
		InsetRect(&box, -kTextMargin, -kTextMargin);
		box.left = box.right - 16;
		(**info).itemInfo[item-1].vScroll = NewControl(dlg, &box, "\p", true, 0, 0, 
			(**hTE).nLines - numLinesInBox, scrollBarProc, item);
	}
	
	TextFont(saveFont);
	TextSize(saveSize);
}



/*----------------------------------------------------------------------------
	PrepDialog 
	
	Prepares for presenting a dialog:
----------------------------------------------------------------------------*/

void PrepDialog (void)
{
	WindowPtr wind;
	NMRec nRec;

	CloseStatusWindow();
	if (gInBackground) {
		nRec.qType = nmType;
		nRec.nmMark = 1;
		nRec.nmIcon = GetResource('SICN', kNewsWatcherSmallIconID);
		nRec.nmSound = nil;
		nRec.nmStr = nil;
		nRec.nmResp = nil;
		NMInstall(&nRec);
		while (gInBackground) GiveTime();
		NMRemove(&nRec);
	}
	wind = FrontWindow();
	if (IsAppWindow(wind)) {
		HandleActivate(wind, false);
		HandleUpdate(wind);
	}
	SetCursor(&qd.arrow);
}



/*----------------------------------------------------------------------------
	MyGetNewDialog 
	
	Gets a new dialog and intializes the dialog information.
	
	Entry:	id = resource id of DLOG resource.
	
	Exit:	function result = pointer to dialog record
----------------------------------------------------------------------------*/

DialogPtr MyGetNewDialog (short id)
{
	DialogPtr dlg;
	TDialogInfo **info;
	short numItems, i;
	TDialogItemInfo itemInfo;

	PrepDialog();
	dlg = GetNewDialog(id, nil, (WindowPtr)-1);
	numItems = CountDITL(dlg);;
	info = (TDialogInfo**)MyNewHandle(sizeof(TDialogInfo) + numItems*sizeof(TDialogItemInfo));
	(**info).returnIsOK = true;
	(**info).hasCancelButton = false;
	(**info).customCopyCommandHandler = nil;
	itemInfo.numeric = false;
	itemInfo.returnIsLegal = false;
	itemInfo.popupTypeinItem = 0;
	itemInfo.keyEquivalent = 0;
	itemInfo.maxLength = 0x7fff;
	itemInfo.password = nil;
	itemInfo.readOnly = false;
	for (i = 0; i < numItems; i++) (**info).itemInfo[i] = itemInfo;
	SetWRefCon(dlg, (long)info);
	SetMenusTo(false, 0, 0, 0, 0, 0, 0);
	HiliteMenu(0);
	return dlg;
}



/*----------------------------------------------------------------------------
	MyDisposDialog 
	
	Disposes a dialog.
	
	Entry:	dlg = pointer to dialog.
----------------------------------------------------------------------------*/

void MyDisposDialog (DialogPtr dlg)
{
	TDialogInfo **info;
	short itemNumber, numItems;
	TDialogItemInfo *pItemInfo;

	info = (TDialogInfo**)GetWRefCon(dlg);
	numItems = CountDITL(dlg);
	HLock((Handle)info);
	for (itemNumber = 1, pItemInfo = (**info).itemInfo; 
		itemNumber <= numItems; 
		itemNumber++, pItemInfo++) 
	{
		if (pItemInfo->readOnly) TEDispose(pItemInfo->readOnlyTE);
	}
	MyDisposHandle((Handle)info);
	DisposDialog(dlg);
}



/*----------------------------------------------------------------------------
	MyShortenDITL
	
	Shortens a dialog's item list.
	
	Entry:	dlg = pointer to dialog.
			numberItems = number of items to remove from the dialog item list.
----------------------------------------------------------------------------*/

void MyShortenDITL (DialogPtr dlg, short numberItems)
{
	TDialogInfo **info;
	short numItems;
	
	ShortenDITL(dlg, numberItems);
	info = (TDialogInfo**)GetWRefCon(dlg);
	numItems = CountDITL(dlg);
	MySetHandleSize((Handle)info, 
		sizeof(TDialogInfo) + numItems*sizeof(TDialogItemInfo));
}



/*----------------------------------------------------------------------------
	MyAppendDITL 
	
	Adds items to a dialog's item list.
	
	Entry:	dlg = pointer to dialog.
			theDITL = handle to item list to append.
			theMethod = the manner in which the new items should be displayed,
				as in the Dialog Manger AppendDITL procedure.
----------------------------------------------------------------------------*/

void MyAppendDITL (DialogPtr dlg, Handle theDITL, DITLMethod theMethod)
{
	TDialogInfo **info;
	TDialogItemInfo itemInfo;
	short oldNumItems, numItems, i;

	oldNumItems = CountDITL(dlg);
	AppendDITL(dlg, theDITL, theMethod);
	info = (TDialogInfo**)GetWRefCon(dlg);
	numItems = CountDITL(dlg);
	MySetHandleSize((Handle)info, 
		sizeof(TDialogInfo) + numItems*sizeof(TDialogItemInfo));
	itemInfo.numeric = false;
	itemInfo.returnIsLegal = false;
	itemInfo.popupTypeinItem = 0;
	itemInfo.keyEquivalent = 0;
	itemInfo.maxLength = 0x7fff;
	itemInfo.password = nil;
	itemInfo.readOnly = false;
	for (i = oldNumItems; i < numItems; i++) (**info).itemInfo[i] = itemInfo;
}



/*----------------------------------------------------------------------------
	ScrollAction 
	
	Action procedure for vertical scroll bars in dialog read only text fields.
	
	Entry:	dlg = pointer to dialog.
----------------------------------------------------------------------------*/

static pascal void ScrollAction (ControlHandle vScroll, short part)
{
	TDialogInfo **info;
	short item;
	TDialogItemInfo *pItemInfo;
	TEHandle hTE;
	short scrollAmt;
	short lineHeight, pageSize;
	short oldCtlValue, newCtlValue;
	short min, max;
	
	info = (TDialogInfo**)GetWRefCon((**vScroll).contrlOwner);
	item = GetCRefCon(vScroll);
	pItemInfo = (**info).itemInfo + item - 1;
	hTE = pItemInfo->readOnlyTE;
	lineHeight = (**hTE).lineHeight;
	pageSize = ((**hTE).viewRect.bottom - (**hTE).viewRect.top) / lineHeight - 1;
	min = GetCtlMin(vScroll);
	max = GetCtlMax(vScroll);
	switch (part) {
		case inUpButton:
			scrollAmt = -1;
			break;
		case inDownButton:
			scrollAmt = 1;
			break;
		case inPageUp:
			scrollAmt = -pageSize;
			break;
		case inPageDown:
			scrollAmt = pageSize;
			break;
	}
	oldCtlValue = GetCtlValue(vScroll);
	newCtlValue = oldCtlValue + scrollAmt;
	if (newCtlValue < min) newCtlValue = min;
	if (newCtlValue > max) newCtlValue = max;
	if (newCtlValue == oldCtlValue) return;
	SetCtlValue(vScroll, newCtlValue);
	TEScroll(0, (oldCtlValue - newCtlValue) * lineHeight, hTE);
}



/*----------------------------------------------------------------------------
	DialogFilter 
	
	A humongous universal dialog filter which does all of the following 
	gnarly stuff:
	
	Changes the cursor to an ibeam over textedit fields.
	Handles the Return and Enter keyboard equivalents for the default button.
	Handles the Command-Period and Escape keyboard equivalents for the 
		Cancel button.
	Properly flashes buttons when keyboard equivalents are used.
	Properly handles update events for all windows.
	Takes care of outlining the default button.
	Permits the use of the Clear key in addition to the Delete key for 
		deleting text.
	Handles the forward delete key.
	Properly adjusts the Edit menu and handles Edit menu commands.
	Handles caller-defined custom Copy command handler for the entire dialog.
	Handles caller-defined command key equivalents for push buttons.
	Handles caller-defined numeric fields (non-digits are ignored).
	Handles caller-defined definitions of which fields permit the Return key.
	Handles caller-defined password fields (bullets displayed for security).
	Handles caller-defined typein popup menu controls (popup menus associated
		with a typein textedit field for specifying values not in the menu).
	Handles caller-defined max lengths for textedit fields.
	Handles caller-defined read only textedit fields with scroll bars.
		
	Entry:	dlg = pointer to dialog.
			*theEvent = event record.
			refcon field of dialog record = handle to TDialogInfo record.
			
	Exit:	function result = true if event handled.
			*itemHit = item number of item hit.
----------------------------------------------------------------------------*/

pascal Boolean DialogFilter (DialogPtr dlg, EventRecord *theEvent, short *itemHit)
{
	char keyPressed, charPressed;
	short itemNumber;
	short numItems;
	short itemType;
	Handle itemHandle;
	Rect box;
	Boolean setCursorToIBeam;
	Point where;
	WindowPtr theWindow;
	PenState savePen;
	TEHandle hTE;
	TDialogInfo **info;
	TDialogItemInfo *pItemInfo;
	short editField;
	short part;
	short popupTypeinItem;
	Boolean numeric, returnIsLegal;
	short maxLength;
	ControlHandle popupCtl;
	Str255 str;
	Boolean commandKey;
	DialogPeek dPeek;
	short selStart, selEnd;
	char *password;
	short pwLen;
	unsigned long editMenuFlags;
	long menuSelection;
	Boolean isCommand = false;
	Boolean commandKeyIsButtonEquivalent = false;
	char *p, *pEnd;
	Boolean pasteOK = true;
	short nBullets;
	short theCommand;
	CustomCopyCommandHandler customCopyCommandHandler;
	Boolean readOnly;
	ControlHandle vScroll, control;
	short viewTop, destTop, oldScroll, scrollValue, height, newScroll, scrollDiff;
	Boolean setItemHit = false;
	
	SetPort(dlg);
	
	info = (TDialogInfo**)GetWRefCon(dlg);
	dPeek = (DialogPeek)dlg;
	numItems = **(short**)(dPeek->items) + 1;
	editField = dPeek->editField + 1;
	if (editField > 0) {
		hTE = dPeek->textH;
		selStart = (**hTE).selStart;
		selEnd = (**hTE).selEnd;
		pItemInfo = (**info).itemInfo + editField - 1;
		numeric = pItemInfo->numeric;
		returnIsLegal = pItemInfo->returnIsLegal;
		maxLength = pItemInfo->maxLength;
		password = pItemInfo->password;
		if (password != nil) pwLen = strlen(password);
	}
	
	GetMouse(&where);
	setCursorToIBeam = false;
	for (itemNumber = 1; itemNumber <= numItems; itemNumber++) {
		GetDItem(dlg, itemNumber, &itemType, &itemHandle, &box);
		if ((itemType & 0x7f) == editText && PtInRect(where, &box)) {
			setCursorToIBeam = true;
			break;
		}
	}
	SetCursor(setCursorToIBeam ? &gIBeamCurs : &qd.arrow);
	
	if ((**info).customCopyCommandHandler != nil) {
		editMenuFlags = kEntireMenuMask | kCopyMask;
		SetMenusTo(false, 0, editMenuFlags, 0, 0, 0, 0);
	} else if (editField > 0) {
		editMenuFlags = 0;
		if ((**hTE).teLength > 0) editMenuFlags |= kSelectAllMask;
		if (selStart < selEnd) editMenuFlags |= kCutMask | kCopyMask | kClearMask;
		TEFromScrap();
		if (TEScrpLength > 0) {
			if ((**hTE).teLength - (selEnd - selStart) + TEScrpLength <= maxLength) {
				p = *TEScrpHandle;
				pEnd = p + TEScrpLength;
				while (p < pEnd) {
					if (*p < ' ' && (*p != returnKey || !returnIsLegal)) {
						pasteOK = false;
						break;
					}
					if (numeric && (*p < '0' || *p > '9')) {
						pasteOK = false;
						break;
					}
					p++;
				}		
				if (pasteOK) editMenuFlags |= kPasteMask;
			}
		}
		if (editMenuFlags != 0) editMenuFlags |= kEntireMenuMask;
		SetMenusTo(false, 0, editMenuFlags, 0, 0, 0, 0);
	}

	if ((theEvent->what == keyDown)	|| (theEvent->what == autoKey)) {
	
		charPressed = theEvent->message & charCodeMask;
		keyPressed = (theEvent->message & keyCodeMask) >> 8;
		commandKey = (theEvent->modifiers & cmdKey) != 0;
		if (!commandKey && ((charPressed == returnKey && (**info).returnIsOK) || 
			(charPressed == enterKey)))	
		{
			GetDItem(dlg, ok, &itemType, &itemHandle, &box);
			if (!(itemType & itemDisable)) {
				*itemHit = ok;
				DlgFlashButton(dlg, ok);
				return true;
			} else {
				theEvent->what = nullEvent;
			}
		} else if ((**info).hasCancelButton && 
			((!commandKey && keyPressed == escapeKeyCode) ||
			(commandKey && charPressed == '.'))) 
		{
			GetDItem(dlg, cancel, &itemType, &itemHandle, &box);
			if (!(itemType & itemDisable)) {
				*itemHit = cancel;
				DlgFlashButton(dlg, cancel);
				return true;
			} else {
				theEvent->what = nullEvent;
			}
		} else if (commandKey) {
			HLock((Handle)info);
			for (itemNumber = 1, pItemInfo = (**info).itemInfo; 
				itemNumber <= numItems; 
				itemNumber++, pItemInfo++) 
			{
				if (tolower(charPressed) == tolower(pItemInfo->keyEquivalent)) {
					commandKeyIsButtonEquivalent = true;
					GetDItem(dlg, itemNumber, &itemType, &itemHandle, &box);
					if (!(itemType & itemDisable)) {
						*itemHit = itemNumber;
						DlgFlashButton(dlg, itemNumber);
						HUnlock((Handle)info);
						return true;
					} else {
						theEvent->what = nullEvent;
					}
				}
			}
			HUnlock((Handle)info);
			if (!commandKeyIsButtonEquivalent) {
				menuSelection = MenuKey(charPressed);
				isCommand = true;
			}
		}
		if (editField != 0 && theEvent->what != nullEvent && !commandKey) {
			if (keyPressed == clearKeyCode) theEvent->message = charPressed = deleteKey;
			switch (charPressed) {
				case tabKey:
				case leftArrow:
				case rightArrow:
				case upArrow:
				case downArrow:
					break;
				case deleteKey:
					if (password != nil) {
						if (selStart == selEnd && selStart > 0) selStart--;
						BlockMove(password + selEnd, password + selStart, pwLen - selEnd + 1);
					}
					break;
				case returnKey:
					if (!returnIsLegal) {
						theEvent->what = nullEvent;
						SysBeep(1);
					}
					break;
				case forwardDelKey:
					if (selStart == selEnd && selEnd < (**hTE).teLength) {
						selEnd++;
						(**hTE).selEnd = selEnd;
					}
					TEDelete(hTE);
					if (password != nil) 
						BlockMove(password + selEnd, password + selStart, pwLen - selEnd + 1);
					*itemHit = editField;
					return true;
					break;
				default:
					if (charPressed < ' ' ||
						numeric && !(charPressed >= '0' && charPressed <= '9') ||
						(**hTE).teLength - (selEnd - selStart) + 1 > maxLength)
					{
						theEvent->what = nullEvent;
						SysBeep(1);
					} else if (password != nil) {
						BlockMove(password + selEnd, password + selStart + 1, pwLen - selEnd + 1);
						*(password + selStart) = charPressed;
						theEvent->message = '�';
					}
					break;
			}
		}
	} else if (theEvent->what == updateEvt) {
		theWindow = (WindowPtr)theEvent->message;
		if (theWindow == dlg) {
			GetDItem(dlg, ok, &itemType, &itemHandle, &box);
			GetPenState(&savePen);
			PenSize(3,3);
			if (itemType & itemDisable) PenPat(qd.gray);
			InsetRect(&box, -4, -4);
			FrameRoundRect(&box, 16, 16);
			SetPenState(&savePen);
		} else {
			HandleUpdate(theWindow);
			SetPort(dlg);
		}
	} else if (theEvent->what == mouseDown) {
		where = theEvent->where;
		part = FindWindow(where, &theWindow);
		if (part == inMenuBar) {
			menuSelection = MenuSelect(where);
			isCommand = true;
		} else if (part == inContent && theWindow == dlg) {
			GlobalToLocal(&where);
			itemNumber = FindDItem(dlg, where) + 1;
			if (itemNumber > 0) {
				pItemInfo = (**info).itemInfo + itemNumber - 1;
				popupTypeinItem = pItemInfo->popupTypeinItem;
				readOnly = pItemInfo->readOnly;
				hTE = pItemInfo->readOnlyTE;
				vScroll = pItemInfo->vScroll;
				if (popupTypeinItem != 0) {
					SelIText(dlg, popupTypeinItem, 0, 0x7fff);
					DlgGetPString(dlg, popupTypeinItem, str);
					popupCtl = DlgGetControl(dlg, itemNumber);
					numeric = (**info).itemInfo[popupTypeinItem-1].numeric;
					if (TrackPopup(popupCtl, where, str, numeric)) {
						*itemHit = itemNumber;
						return true;
					} else {
						theEvent->what = nullEvent;
					}
				} else if (readOnly && vScroll != nil) {
					if ((part = FindControl(where, dlg, &control)) != 0 && control == vScroll) {
						if (part == inThumb) {
							TrackControl(vScroll, where, nil);
							viewTop = (**hTE).viewRect.top;
							destTop = (**hTE).destRect.top;
							oldScroll = viewTop - destTop;
							scrollValue = GetCtlValue(vScroll);
							height = (**hTE).lineHeight;
							newScroll = scrollValue * height;
							scrollDiff = oldScroll - newScroll;
							if (scrollDiff != 0) TEScroll(0, scrollDiff, hTE);
						} else {
							TrackControl(vScroll, where, (ProcPtr)ScrollAction);
						}
					}
				}
			}
		}
	}
	if (isCommand) {
		if (HiWord(menuSelection) == kEditMenu) {
			theCommand = LoWord(menuSelection);
			switch (theCommand) {
				case kCutItem:
				case kCopyItem:
					if (theCommand == kCopyItem) {
						customCopyCommandHandler = (**info).customCopyCommandHandler;
						if (customCopyCommandHandler != nil) {
							(*customCopyCommandHandler)(dlg);
							break;
						}
					}
					if (password == nil) {
						if (theCommand == kCutItem) {
							TECut(hTE);
						} else {
							TECopy(hTE);
						}
						ZeroScrap();
						TEToScrap();
					} else {
						ZeroScrap();
						PutScrap(selEnd - selStart, 'TEXT', password + selStart);
						TEFromScrap();
						if (theCommand == kCutItem) {
							TEDelete(hTE);
							BlockMove(password + selEnd, password + selStart, pwLen - selEnd + 1);
						}
					}
					setItemHit = theCommand == kCutItem;
					break;
				case kPasteItem:
					TEFromScrap();
					if (password == nil) {
						TEPaste(hTE);
					} else {
						BlockMove(password + selEnd,
							password + selStart + TEScrpLength,
							pwLen - selEnd + 1);
						BlockMove(*TEScrpHandle, 
							password + selStart,
							TEScrpLength);
						nBullets = TEScrpLength;
						TEDelete(hTE);
						while (nBullets--) TEInsert("�", 1, hTE);
					}
					setItemHit = true;
					break;
				case kClearItem:
					TEDelete(hTE);
					setItemHit = true;
					break;
				case kSelectAllItem:
					TESetSelect(0, 0x7fff, hTE);
					break;
			}
		}
		HiliteMenu(0);
		if (setItemHit) {
			*itemHit = editField;
			return true;
		}
	}
	return false;
}



/*----------------------------------------------------------------------------
	MyModalDialog 
 
	Presents a modal dialog.
	
	Entry:	filterProc = pointer to filter proc.
			hasCancelButton = true if dialog has a cancel button.
			returnIsOK = true if Return key is equivalent to ok button.
			
	Exit:	itemHit = item number hit.
----------------------------------------------------------------------------*/

void MyModalDialog (ModalFilterProcPtr filterProc, short *itemHit,
	Boolean hasCancelButton, Boolean returnIsOK)
{
	TDialogInfo **info;

	info = (TDialogInfo**)GetWRefCon(FrontWindow());
	(**info).returnIsOK = returnIsOK;
	(**info).hasCancelButton = hasCancelButton;
	TEFromScrap();
	ModalDialog(filterProc, itemHit);
	ZeroScrap();
	TEToScrap();
}



/*----------------------------------------------------------------------------
	ErrorMessage 
	
	Issues an error message alert.
	
	Entry:	msg = error message.
----------------------------------------------------------------------------*/

void ErrorMessage (char *msg)
{
	DialogPtr dlg;
	short item;

	c2pstr(msg);
	ParamText((StringPtr)msg, "\p", "\p", "\p");
	p2cstr((StringPtr)msg);
	dlg = MyGetNewDialog(kErrDlg);
	SysBeep(1);
	MyModalDialog(DialogFilter, &item, false, true);
	MyDisposDialog(dlg);
}



/*----------------------------------------------------------------------------
	UnexpectedErrorMessage 
	
	Issues an unexpected error alert.
	
	Entry:	err = error number.
----------------------------------------------------------------------------*/

void UnexpectedErrorMessage (OSErr err)
{
	Str255 errNumStr;
	DialogPtr dlg;
	short item;

	if (err == -1 || err == noErr) return;
	if (err == connectionClosing || err == connectionDoesntExist ||
		err == connectionTerminated) 
	{
		ErrorMessage("Lost connection to server.");
		return;
	}
	NumToString(err, errNumStr);
	ParamText(errNumStr, "\p", "\p", "\p");
	dlg = MyGetNewDialog(kUnexpectedErrDlg);
	SysBeep(1);
	MyModalDialog(DialogFilter, &item, false, true);
	MyDisposDialog(dlg);
}



/*----------------------------------------------------------------------------
	DrawNewsServerErrorMessageBozoText
	
	A user item procedure to draw the server error message bozo text.
	
	Entry:	dlg = pointer to dialog.
			item = item number.
----------------------------------------------------------------------------*/

static pascal void DrawNewsServerErrorMessageBozoText (DialogPtr dlg, short item)
{
	short itemType;
	Rect box;
	Handle itemHandle;
	Handle text;
	short saveFont, saveSize;
	GrafPtr curPort;

	GetDItem(dlg, item, &itemType, &itemHandle, &box);
	text = GetResource('TEXT', kNewsServerErrorMessageBozoText);
	GetPort(&curPort);
	saveFont = curPort->txFont;
	saveSize = curPort->txSize;
	TextFont(applFont);
	TextSize(9);
	HLock(text);
	TextBox(*text, GetHandleSize(text), &box, teFlushDefault);
	HUnlock(text);
	TextFont(saveFont);
	TextSize(saveSize);
}



/*----------------------------------------------------------------------------
	NewsServerErrorMessageCopyHandler
	
	Handles the Edit menu Copy command for the news server error message dialog.
		
	Entry:	dlg = pointer to dialog.
----------------------------------------------------------------------------*/

static void NewsServerErrorMessageCopyHandler (DialogPtr dlg)
{
	char str[516];
	Handle txtHandle, rsrcHandle;

	sprintf(str, "%#s\r\r%#s\r\r", gServerErrorMessageStaticText, 
		gServerErrorMessageTheMessage);
	PtrToHand(str, &txtHandle, strlen(str));
	rsrcHandle = GetResource('TEXT', kNewsServerErrorMessageBozoText);
	HLock(rsrcHandle);
	HandAndHand(rsrcHandle, txtHandle);
	HUnlock(rsrcHandle);
	ZeroScrap();
	HLock(txtHandle);
	PutScrap(GetHandleSize(txtHandle), 'TEXT', *txtHandle);
	MyDisposHandle(txtHandle);
	TEFromScrap();
}



/*----------------------------------------------------------------------------
  NewsServerErrorMessage 
  
  Issues a news server error message.
  
  Entry:	serverCommand = name of the command NewsWatcher tried to send
  				to the news server.
  			serverResponse = error message as returned by the news server, terminated
  				by CR or LF.
----------------------------------------------------------------------------*/

void NewsServerErrorMessage (char *serverCommand, char *serverResponse)
{
	DialogPtr dlg;
	short i, item;

	dlg = MyGetNewDialog(kServerErrDlg);
	SysBeep(1);
	for (i = 0; i < 255 && serverResponse[i] != CR && serverResponse[i] != LF; i++);
	BlockMove(serverResponse, gServerErrorMessageTheMessage+1, i);
	*gServerErrorMessageTheMessage = i;
	if (serverCommand == nil) {
		strcpy((char*)gServerErrorMessageStaticText,
			"A news server error occurred when NewsWatcher tried to connect to the server.");
	} else {
		sprintf((char*)gServerErrorMessageStaticText,
			"A news server error occurred in response to a NewsWatcher %s command.",
			serverCommand);
	}
	c2pstr((char*)gServerErrorMessageStaticText);
	DlgSetPString(dlg, kNewsServerErrorMessageStaticTextItem, gServerErrorMessageStaticText); 
	DlgSetPString(dlg, kNewsServerErrorMessageTheMessageItem, gServerErrorMessageTheMessage); 
	DlgSetUserItem(dlg, kNewsServerErrorMessageGrayBorderUserItem, DlgGrayBorderItem);	
	DlgSetUserItem(dlg, kNewsServerErrorMessageBozoTextUserItem, DrawNewsServerErrorMessageBozoText);
	SetDialogCustomCopyHandler(dlg, NewsServerErrorMessageCopyHandler);
	MyModalDialog(DialogFilter, &item, false, true);
	MyDisposDialog(dlg);
}



/*----------------------------------------------------------------------------
  MailOrFTPServerErrorMessage 
  
  Issues a mail or FTP server error message.
  
  Entry:	serverResponse = error message as returned by the server, terminated
  				by CR or LF.
----------------------------------------------------------------------------*/

void MailOrFTPServerErrorMessage (char *serverResponse)
{
	DialogPtr dlg;
	short i, item;
	Str255 msg;

	dlg = MyGetNewDialog(kMailFTPErrDlg);
	SysBeep(1);
	for (i = 0; i < 255 && serverResponse[i] != CR && serverResponse[i] != LF; i++);
	BlockMove(serverResponse, msg+1, i);
	*msg = i;
	DlgSetPString(dlg, kMailFTPErrorMessageTheMessageItem, msg); 
	DlgSetUserItem(dlg, kMailFTPErrorMessageGrayBorderUserItem, DlgGrayBorderItem);	
	MyModalDialog(DialogFilter, &item, false, true);
	MyDisposDialog(dlg);
}
