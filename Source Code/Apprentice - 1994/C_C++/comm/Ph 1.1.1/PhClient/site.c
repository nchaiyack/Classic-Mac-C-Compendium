/*_____________________________________________________________________

  	site.c - Server Site Movable Modal Dialog.
	
	This movable modal dialog is used to select both the default server
	site and the help server site.
_____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "site.h"
#include "utl.h"
#include "glob.h"
#include "oop.h"
#include "wstm.h"

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static DialogPtr		Window;			/* ptr to dialog window */
static Rect				PopupRect;		/* rectangle for popup menu */
static short			CurSel;			/* currently selected popup menu item */

static oop_Dispatch dispatch = {
								oop_DoPeriodic,
								oop_DoClick,
								site_DoKey,
								oop_DoUpdate,
								oop_DoActivate,
								oop_DoDeactivate,
								oop_DoGrow,
								oop_DoZoom,
								oop_DoClose,
								oop_DoCommand
							};

/*_____________________________________________________________________

	site_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void site_DoKey (WindowPtr w, char key, short modifiers)

{
	if (!glob_FilterAsciiChar(w, key, modifiers)) return;
	oop_DoKey(w, key, modifiers);
}

/*_____________________________________________________________________

	DrawPopUp - Draw Popup Menu.
	
	Entry:	theWindow = pointer to dialog window.
				itemNo = item number of popup menu user item.
_____________________________________________________________________*/

static pascal void DrawPopUp (WindowPtr theWindow, short itemNo)

{
#pragma unused (theWindow, itemNo)

	if (!SiteMenu) return;
	glob_DrawSitePopup(&PopupRect, CurSel);
}

/*_____________________________________________________________________

	DoOne - Do One Server Dialog.
	
	Entry:	server = initial value for server domain name field.
				label1 = index in stringsID STR# resource of message to use
					if site list exists.
				label2 = index in stringsID STR# resource of message to use
					if site list does not exist.
	
	Exit:		function result = true if dialog canceled.
				server = new default server domain name.
_____________________________________________________________________*/

Boolean DoOne (Str255 server, short label1, short label2)

{
	short			itemType;	/* item type */
	Handle		item;			/* handle to item */
	Rect			box;			/* item rectangle */
	Str255		label;		/* dialog label static text */
	short			itemHit;		/* item hit */
	short			menuWidth;	/* menu width */
	short			newSel;		/* new index of server in menu */
	Str255		newDomain;	/* new server domain name */

	ShowCursor();
	Window = wstm_Restore(true, siteDlogID, nil, &SiteState);
	utl_SetDialogText(Window, siteServField, server);
	SelIText(Window, siteServField, 0, 0x7fff);
	GetIndString(label, stringsID, SiteMenu ? label1 : label2);
	utl_SetDialogText(Window, siteLabel, label);
	GetDItem(Window, sitePopup, &itemType, &item, &PopupRect);
	if (SiteMenu) {
		CalcMenuSize(SiteMenu);
		menuWidth = (**SiteMenu).menuWidth;
		PopupRect.left = (Window->portRect.right - menuWidth) >> 1;
		PopupRect.right = PopupRect.left + menuWidth;
		CurSel = glob_GetSiteIndex(server);
	}
	SetDItem(Window, sitePopup, itemType, (Handle)DrawPopUp, &PopupRect);
	SetPort(Window);
	TextFont(0);
	TextSize(12);
	oop_NewDialog(Window, siteModal, nil, &dispatch, true, siteOK, siteCancel);
	ShowWindow(Window);
	while (true) {
		oop_ClearWindItemHit(Window);
		while (!(itemHit = oop_GetWindItemHit(Window)) && !Done) 
			oop_DoEvent(nil, everyEvent, 
				oop_InForeground() ? shortSleep : longSleep, nil);
		if (Done) itemHit = siteCancel;
		if (itemHit == siteCancel) break;
		if (itemHit == sitePopup && SiteMenu) {
			glob_PopupSiteMenu(&PopupRect, CurSel, &newSel, &newDomain);
			if (newSel == -1) continue;
			CurSel = newSel;
			utl_SetDialogText(Window, siteServField, newDomain);
			SelIText(Window, siteServField, 0, 0x7fff);
			InvalRect(&box);
			continue;
		}
		break;
	}
	if (itemHit == siteOK) utl_GetDialogText(Window, siteServField, server);
	wstm_Save(Window, &SiteState);
	oop_DoClose(Window);
	InitCursor();
	return itemHit == siteCancel;
}

/*_____________________________________________________________________

	site_DoDialog - Do Default Server Dialog.
	
	Entry:	server = initial value for server domain name field.
				label1 = index in stringsID STR# resource of message to use
					if site list exists.
				label2 = index in stringsID STR# resource of message to use
					if site list does not exist.
	
	Exit:		function result = true if dialog canceled.
				server = new default server domain name.
_____________________________________________________________________*/

Boolean site_DoDialog (Str255 server, short label1, short label2)

{
	while (true) {
		if (DoOne(server, label1, label2)) return true;
		if (!*server) {
			glob_Error(servErrors, msgNoServer, nil);
			continue;
		}
		break;
	}
	return false;
}
