/*_____________________________________________________________________

  	login.c - Login Movable Modal Dialog.
_____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "login.h"
#include "utl.h"
#include "glob.h"
#include "oop.h"
#include "wstm.h"

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static DialogPtr		Window;			/* ptr to dialog window */
static Str255			Pswd;				/* password */
static Rect				PopupRect;		/* rectangle for popup menu */
static short			CurSel;			/* currently selected popup menu item */

static oop_Dispatch dispatch = {
								login_DoPeriodic,
								oop_DoClick,
								login_DoKey,
								oop_DoUpdate,
								oop_DoActivate,
								oop_DoDeactivate,
								oop_DoGrow,
								oop_DoZoom,
								oop_DoClose,
								login_DoCommand
							};

/*_____________________________________________________________________

	login_DoPeriodic - Do Periodic Tasks.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void login_DoPeriodic (WindowPtr w)

{
	oop_DoPeriodic(w);
	if (utl_GetDialogEditFieldNum(w) == loginPswdField)
		glob_CheckPswdSel(w);
}

/*_____________________________________________________________________

	login_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void login_DoKey (WindowPtr w, char key, short modifiers)

{
	if (utl_GetDialogEditFieldNum(w) == loginPswdField) {
		if (!glob_FilterPswdChar(w, key, modifiers, Pswd)) return;
	} else {
		if (!glob_FilterAsciiChar(w, key, modifiers)) return;
	}
	oop_DoKey(w, key, modifiers);
}

/*_____________________________________________________________________

	login_DoCommand - Process a Command.
	
	Entry:	w = pointer to window record.
				theMenu = menu index.
				theItem = item index.
				
	Exit:		function result = true if command handled.
_____________________________________________________________________*/

Boolean login_DoCommand (WindowPtr w, short theMenu, short theItem)

{
	if (theMenu == editID) {
		if (utl_GetDialogEditFieldNum(w) == loginPswdField &&
			theItem != selectAllCmd) {
			glob_FilterPswdEditCmd(w, theItem, Pswd);
			return true;
		} else if (theItem == pasteCmd && !glob_FilterPaste()) {
			return true;
		}
	}
	return oop_DoCommand(w, theMenu, theItem);
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

	login_DoDialog - Do Login Dialog.
	
	Entry:	server = initial value for server field.
				user = initial value for user field.
				pswd = initial value for password field.
	
	Exit:		function result = true if canceled by user.
				server = server name.
				user = login alias or name.
				pswd = password.
_____________________________________________________________________*/

Boolean login_DoDialog (Str255 server, Str255 user, Str255 pswd)

{
	short			itemType;	/* item type */
	Handle		item;			/* handle to item */
	Rect			box;			/* item rectangle */
	short			itemHit;		/* item hit */
	short			menuWidth;	/* menu width */
	Str255		bullets;		/* string of bullets for initial password field */
	short			newSel;		/* new index of server in menu */
	Str255		newDomain;	/* new server domain name */

	ShowCursor();
	Window = wstm_Restore(true, loginDlogID, nil, &LoginState);
	utl_SetDialogText(Window, loginServField, server);
	utl_SetDialogText(Window, loginUserField, user);
	SelIText(Window, loginUserField, 0, 0x7fff);
	utl_CopyPString(Pswd, pswd);
	*bullets = *pswd;
	memset(bullets+1, '�', *pswd);
	utl_SetDialogText(Window, loginPswdField, bullets);
	GetDItem(Window, loginPopup, &itemType, &item, &PopupRect);
	if (SiteMenu) {
		CalcMenuSize(SiteMenu);
		menuWidth = (**SiteMenu).menuWidth;
		PopupRect.left = PopupRect.right - menuWidth;
		CurSel = glob_GetSiteIndex(DefaultServer);
	}
	SetDItem(Window, loginPopup, itemType, (Handle)DrawPopUp, &PopupRect);
	SetPort(Window);
	TextFont(0);
	TextSize(12);
	oop_NewDialog(Window, loginModal, nil, &dispatch, true, 
		loginLogin, loginCancel);
	ShowWindow(Window);
	while (true) {
		oop_ClearWindItemHit(Window);
		while (!(itemHit = oop_GetWindItemHit(Window)) && !Done) 
			oop_DoEvent(nil, everyEvent, 
				oop_InForeground() ? shortSleep : longSleep, nil);
		if (Done) itemHit = loginCancel;
		if (itemHit == loginCancel) break;
		if (itemHit == loginPopup && SiteMenu) {
			glob_PopupSiteMenu(&PopupRect, CurSel, &newSel, &newDomain);
			if (newSel == -1) continue;
			CurSel = newSel;
			utl_SetDialogText(Window, loginServField, newDomain);
			if (((DialogPeek)Window)->editField == loginServField-1)
				SelIText(Window, loginServField, 0, 0x7fff);
			InvalRect(&box);
			continue;
		}
		break;
	}
	if (itemHit == loginLogin) {
		utl_GetDialogText(Window, loginServField, server);
		utl_GetDialogText(Window, loginUserField, user);
		utl_CopyPString(pswd, Pswd);
	}
	wstm_Save(Window, &LoginState);
	oop_DoClose(Window);
	InitCursor();
	return itemHit == loginCancel;
}
