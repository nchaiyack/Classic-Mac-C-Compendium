/*----------------------------------------------------------------------------

	popup.c

	This module contains miscellaneous popup menu utility routines.
	
	Portions copyright � 1990, Apple Computer.
	Portions copyright � 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include <Packages.h>

#include "dlgutil.h"
#include "popup.h"



/* This is the private data for a popup menu control using CDEF 63.
   It's documented in IM6:Compatibility, and in NIM:Toolbox Essentials. */
   
typedef struct {
	MenuHandle mHandle;		/* Handle to menu */
	short mID;				/* Menu ID */
	char mPrivate;			/* Private data of varying length */
} popupPrivateData;



/*----------------------------------------------------------------------------
	GetPopupPString
	
	Gets the text of an item in a Popup Menu control, as a Pascal format
	string.
	
	Entry:	ctl = handle to popup menu control.
			item = item number to get, or kCurrentPopupItem for 
				the currently selected item.
				
	Exit:	str = the item text, as a Pascal format string.
----------------------------------------------------------------------------*/

void GetPopupPString (ControlHandle	ctl, short item, Str255	str)
{
	popupPrivateData **data;
	
	data = (popupPrivateData**)(**ctl).contrlData;
	if (item == kCurrentPopupItem) item = GetCtlValue(ctl);
	GetItem((**data).mHandle, item, str);
}



/*----------------------------------------------------------------------------
	GetPopupCString
	
	Gets the text of an item in a Popup Menu control, as a C format
	string.
	
	Entry:	ctl = handle to popup menu control.
			item = item number to get, or kCurrentPopupItem for 
				the currently selected item.
				
	Exit:	str = the item text, as a C format string.
----------------------------------------------------------------------------*/

void GetPopupCString (ControlHandle ctl, short item, char *str)
{
	GetPopupPString(ctl, item, (StringPtr)str);
	p2cstr((StringPtr)str);
}



/*----------------------------------------------------------------------------
	SetPopupItemStyle
	
	Sets the text style for one of the items in a popup menu control.
	
	Entry:	ctl = handle to popup menu control.
			item = item number to set, or kCurrentPopupItem for 
				the currently selected item.
			style = the new text style.
----------------------------------------------------------------------------*/

void SetPopupItemStyle (ControlHandle ctl, short item, short style)
{
	popupPrivateData **data;
	
	data = (popupPrivateData**)(**ctl).contrlData;
	if (item == kCurrentPopupItem) item = GetCtlValue(ctl);
	SetItemStyle((**data).mHandle, item, style);
}



/*----------------------------------------------------------------------------
	AddPopupItem
	
	Adds an item to a popup menu control.
	
	Entry:	ctl = handle to popup menu control.
			after = item number after which the new item should be
				inserted, or 0 to insert the new item and the beginning
				of the menu.
			str = text or the new item, in Pascal format.
----------------------------------------------------------------------------*/

void AddPopupItem (ControlHandle ctl, short after, Str255 str)
{
	popupPrivateData **data;
	short curValue;

	data = (popupPrivateData**)(**ctl).contrlData;
	InsMenuItem((**data).mHandle, str, after);
	curValue = GetCtlValue(ctl);
	if (after < curValue) SetCtlValue(ctl, curValue+1);
}



/*----------------------------------------------------------------------------
	DelPopupItem
	
	Deletes an item from a popup menu control.
	
	Entry:	ctl = handle to popup menu control.
			item = item number to delete, or kCurrentPopupItem to delete 
				the currently selected item.
----------------------------------------------------------------------------*/

void DelPopupItem (ControlHandle ctl, short item)
{
	popupPrivateData **data;
	short curValue;

	data = (popupPrivateData**)(**ctl).contrlData;
	curValue = GetCtlValue(ctl);
	if (item == kCurrentPopupItem) item = curValue;
	DelMenuItem((**data).mHandle, item);
	if (item < curValue) SetCtlValue(ctl, curValue-1);
}


/*----------------------------------------------------------------------------
	SetPopupValue
	
	Sets the value of a popup menu control to the item matching a string.
	
	Entry:	ctl = handle to popup menu control.
			str = Pascal format string.
			isNumber = true if string and menu items are numbers.

	Exit:	function result = new control value, or 0 if no matching
				item.
----------------------------------------------------------------------------*/

short SetPopupValue (ControlHandle ctl, Str255 str, Boolean	isNumber)
{
	popupPrivateData **data;
	long checkVal, itemVal;
	MenuHandle menu;
	short numItems, item;
	Str255 tempStr;

	data = (popupPrivateData**)(**ctl).contrlData;
	if (isNumber) StringToNum(str, &checkVal);
	menu = (**data).mHandle;

	numItems = CountMItems(menu);
	for (item = 1; item <= numItems; item++) {
		GetItem(menu, item, tempStr);
		if (isNumber) {
			StringToNum(tempStr, &itemVal);
			if (checkVal == itemVal) {
				SetCtlValue(ctl, item);
				return item;
			}
		} else if (EqualString(str, tempStr, false, true)) {
			SetCtlValue(ctl, item);
			return item;
		}
	}
	return 0;
}

/*----------------------------------------------------------------------------
	TrackPopup
	
	Tracks a click in a typein popup menu control.
	
	Entry:	ctl = handle to popup menu control.
			where = location of mouse click, in local coords.
			checkItem = text of the item that should appear checked
				(the contents of the "partner" typein textedit field),
				in Pascal format.
			isNumber = true if menu items are numbers.
			
	Exit:	function result = new control value, or 0 if no change.

	If the text does not match a menu item, a new item is added
	at the beginning, followed by a separator line. These two extra
	items are deleted before the function returns. 
----------------------------------------------------------------------------*/

short TrackPopup (ControlHandle	ctl, Point where, Str255 checkItem, Boolean	isNumber)
{
	popupPrivateData **data;
	MenuHandle menu;
	long checkVal, itemVal;
	short numItems, i, itemType, itemsAdded, part, newValue, oldValue;
	Boolean	found;
	Str255 tempStr;

	data = (popupPrivateData **) (**ctl).contrlData;
	menu = (**data).mHandle;
	itemsAdded = 0;
	if (checkItem && *checkItem) {
		oldValue = SetPopupValue(ctl, checkItem, isNumber);
		if (oldValue == 0) {
			if (isNumber) {
				StringToNum(checkItem, &checkVal);
				NumToString(checkVal, tempStr);
				InsMenuItem(menu, tempStr, 0);
			} else {
				InsMenuItem(menu, checkItem, 0);
			}
			InsMenuItem(menu, "\p(-", 1);
			oldValue = 1;
			itemsAdded = 2;
			SetCtlValue(ctl, oldValue);
		}
	} else {
		oldValue = 0;
	}

	part = TrackControl(ctl, where, (ProcPtr)-1);
	newValue = GetCtlValue(ctl);

	if (part != 0 && oldValue != newValue) {
		newValue = newValue - itemsAdded;
	} else {
		newValue = 0;
	}
	if (itemsAdded) {
		for (i = 0; i < itemsAdded; i++) {
			DelMenuItem(menu, 1);
		}
		SetCtlValue(ctl, newValue);
	}
	return newValue;
}
