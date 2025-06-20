/*_____________________________________________________________________

  	new.c - New Ph Record Movable Modal Dialog.
_____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "new.h"
#include "utl.h"
#include "glob.h"
#include "oop.h"
#include "wstm.h"

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static DialogPtr		Window;			/* ptr to dialog window */
static Str255			Pswd1;			/* first password */
static Str255			Pswd2;			/* second password */

static oop_Dispatch dispatch = {
								new_DoPeriodic,
								oop_DoClick,
								new_DoKey,
								oop_DoUpdate,
								oop_DoActivate,
								oop_DoDeactivate,
								oop_DoGrow,
								oop_DoZoom,
								oop_DoClose,
								new_DoCommand
							};

/*_____________________________________________________________________

	new_DoPeriodic - Do Periodic Tasks.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void new_DoPeriodic (WindowPtr w)

{
	short			fNum;				/* edit field number */

	oop_DoPeriodic(w);
	fNum = utl_GetDialogEditFieldNum(w);
	if (fNum == newPswdField1 || fNum == newPswdField2)
		glob_CheckPswdSel(w);
}

/*_____________________________________________________________________

	new_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void new_DoKey (WindowPtr w, char key, short modifiers)

{
	short			fNum;				/* edit field number */
	char			*pswd;			/* pointer to Pswd1 or Pswd2 */

	fNum = utl_GetDialogEditFieldNum(w);
	if (fNum == newPswdField1 || fNum == newPswdField2) {
		pswd = fNum == newPswdField1 ? Pswd1 : Pswd2;
		if (!glob_FilterPswdChar(w, key, modifiers, pswd)) return;
	} else {
		if (!glob_FilterAsciiChar(w, key, modifiers)) return;
	}
	oop_DoKey(w, key, modifiers);
}

/*_____________________________________________________________________

	new_DoCommand - Process a Command.
	
	Entry:	w = pointer to window record.
				theMenu = menu index.
				theItem = item index.
				
	Exit:		function result = true if command handled.
_____________________________________________________________________*/

Boolean new_DoCommand (WindowPtr w, short theMenu, short theItem)

{
	short			fNum;				/* edit field number */
	char			*pswd;			/* pointer to Pswd1 or Pswd2 */

	fNum = utl_GetDialogEditFieldNum(w);
	if (fNum == newPswdField1 || fNum == newPswdField2) {
		pswd = fNum == newPswdField1 ? Pswd1 : Pswd2;
		if (theMenu == editID && theItem != selectAllCmd) {
			glob_FilterPswdEditCmd(w, theItem, pswd);
			return true;
		}
	} else if (theMenu == editID && theItem == pasteCmd &&
		!glob_FilterPaste()) {
		return true;
	}
	return oop_DoCommand(w, theMenu, theItem);
}

/*_____________________________________________________________________

	InitField - Initialize Dialog Field.
	
	Entry:	fNum = field number.
				val = field value.
				hilite = true to hilite (select) this field.
_____________________________________________________________________*/

static void InitField (short fNum, Str255 val, Boolean hilite)

{
	utl_SetDialogText(Window, fNum, val);
	if (hilite) SelIText(Window, fNum, 0, 0x7fff);
}

/*_____________________________________________________________________

	new_DoDialog - Do New Ph Record Dialog.
	
	Entry:	alias = initial value for alias field.
				name = initial value for name field.
				type = initial value for type field.
				pswd1 = initial value for first password field.
				pswd2 = initial value for second password field.
				whichField = field to hilite initially:
					0 = alias.
					1 = name.
					2 = type.
					3 = password.
	
	Exit:		function result = true if canceled by user.
				alias = alias.
				name = name.
				type = type.
				pswd1 = first password.
				pswd2 = second password.
_____________________________________________________________________*/

Boolean new_DoDialog (Str255 alias, Str255 name, Str255 type, Str255 pswd1,
	Str255 pswd2, short whichField)

{
	short			itemHit;		/* item hit */
	Str255		bullets;		/* string of bullets for initial password field */

	ShowCursor();
	Window = wstm_Restore(true, newDlogID, nil, &NewState);
	InitField(newAliasField, alias, whichField == 0);
	InitField(newNameField, name, whichField == 1);
	InitField(newTypeField, type, whichField == 2);
	utl_CopyPString(Pswd1, pswd1);
	utl_CopyPString(Pswd2, pswd2);
	*bullets = *pswd1;
	memset(bullets+1, '�', *pswd1);
	InitField(newPswdField1, bullets, whichField == 3);
	*bullets = *pswd2;
	memset(bullets+1, '�', *pswd2);
	InitField(newPswdField2, bullets, false);
	SetPort(Window);
	TextFont(0);
	TextSize(12);
	oop_NewDialog(Window, newModal, nil, &dispatch, true, 
		newOK, newCancel);
	ShowWindow(Window);
	while (true) {
		oop_ClearWindItemHit(Window);
		while (!(itemHit = oop_GetWindItemHit(Window)) && !Done) 
			oop_DoEvent(nil, everyEvent, 
				oop_InForeground() ? shortSleep : longSleep, nil);
		if (Done) itemHit = newCancel;
		if (itemHit == newCancel) break;
		break;
	}
	if (itemHit == newOK) {
		utl_GetDialogText(Window, newAliasField, alias);
		utl_GetDialogText(Window, newNameField, name);
		utl_GetDialogText(Window, newTypeField, type);
		utl_CopyPString(pswd1, Pswd1);
		utl_CopyPString(pswd2, Pswd2);
	}
	wstm_Save(Window, &NewState);
	oop_DoClose(Window);
	InitCursor();
	return itemHit == newCancel;
}
