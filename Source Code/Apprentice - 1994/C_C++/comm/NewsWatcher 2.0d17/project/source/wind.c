/*----------------------------------------------------------------------------

	wind.c

	This module handles the commands in the Windows menu.
	
	Portions copyright © 1990, Apple Computer.
	Portions copyright © 1993, Northwestern University.

----------------------------------------------------------------------------*/

#include "glob.h"
#include "full.h"
#include "menus.h"
#include "resize.h"
#include "util.h"
#include "wind.h"


/*	SetWindowNeedsZooming sets the "standard state" of a window to
	(0,0,0,0). This forces the next ToggleZoom call to zoom the window 
	out. This function should be called whenever the window contents change.
*/

void SetWindowNeedsZooming (WindowPtr wind)
{
	WStateData **stateHndl;
	
	stateHndl = (WStateData**)((WindowPeek)wind)->dataHandle;
	SetRect(&(**stateHndl).stdState, 0, 0, 0, 0);
}


/*	ToggleZoom toggles the window zoomed state of the active window.
	The window will only be grown as big as it needs to be to contain
	all of the data.  Also, whichever monitor contains the majority of
	the window will be the destination for the window.
*/

void ToggleZoom (WindowPtr wind)
{
	WStateData **stateHndl;
	Rect windRect;
	GrafPtr savePort;
	
	GetPort(&savePort);
	SetPort(wind);
	windRect = wind->portRect;
	LocalToGlobal((Point*)&windRect.top);
	LocalToGlobal((Point*)&windRect.bottom);
	SetPort(savePort);
	stateHndl = (WStateData **) ((WindowPeek)wind)->dataHandle;
	if (EqualRect(&(**stateHndl).stdState,&windRect)) {
		DoZoom(wind,inZoomIn);
	} else {
		DoZoom(wind,inZoomOut);
	}
}


/*	ShowHideGroups hides/shows the main groups window.
*/

void ShowHideGroups (void)
{
	MenuHandle windMenu;

	windMenu = GetMHandle(kWindMenu);
	if (gPrefs.groupWindowVisible) {
		HideWindow(gFullGroupWindow);
		RemoveWindMenu(gFullGroupWindow);
		SetItem(windMenu, kShowHideFullItem, kShowText);
	}
	else {
		if (gMustDoZoomOnShowFullGroupList)
			if (!DoZoom(gFullGroupWindow, inZoomOut)) return;
		gMustDoZoomOnShowFullGroupList = false;
		ShowWindow(gFullGroupWindow);
		AddWindMenu(gFullGroupWindow);
		SetItem(windMenu, kShowHideFullItem, kHideText);
		SelectWindow(gFullGroupWindow);
	}
	gPrefs.groupWindowVisible = !gPrefs.groupWindowVisible;
}


/*	RemoveWindMenu removes a window's title from the windows menu when the
	window is closed, etc…
*/

void RemoveWindMenu (WindowPtr wind)
{
	short item;
	Str255 name,itemString;
	MenuHandle windMenu;
	
	if (!wind) return;
	windMenu = GetMHandle(kWindMenu);
	GetWTitle(wind, name);
	for (item = kFirstWindOffset; item <= CountMItems(windMenu); item++) {
		GetItem(windMenu,item,itemString);
		if (EqualString(name, itemString, true, true)) {
			DelMenuItem(windMenu, item);
			return;
		}
	}
}


/*	AddWindMenu adds a window's title to the windows menu.
*/

void AddWindMenu (WindowPtr wind)
{
	Str255 name;
	MenuHandle windMenu;
	short len;
	
	if (!wind) return;
	windMenu = GetMHandle(kWindMenu);
	GetWTitle(wind, name);
	len = *name;
	if (len > 0 && name[1] == '-') {
		/* if first char of window title is '-' prepend a space to avoid
		  menu divider line. */
		if (len < 255) len++;
		BlockMove(name+1, name+2, len-1);
		*name = len;
		name[1] = ' ';
	}
	InsMenuItem(windMenu, "\pnew item", kFirstWindOffset-1);
	SetItem(windMenu, kFirstWindOffset, name);
}


/*	SelectWindMenu is called when a user selects an item from the windows
	menu.  The selected window is brought to the front.
*/

void SelectWindMenu (short item)
{
	Str255 name,itemString;
	WindowPtr wind;
	
	GetItem(GetMHandle(kWindMenu), item, itemString);
	for (wind = FrontWindow(); 
		wind != nil; 
		wind = (WindowPtr)((WindowPeek)wind)->nextWindow)
	{
		GetWTitle(wind, name);
		if (EqualString(name, itemString, true, true)) {
			SelectWindow(wind);
			return;
		}
	}
}
