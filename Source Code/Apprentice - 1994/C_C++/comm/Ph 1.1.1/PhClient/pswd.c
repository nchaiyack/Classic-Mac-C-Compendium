/*_____________________________________________________________________

  	pswd.c - Server Transaction Movable Modal Dialog.
_____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "pswd.h"
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

static oop_Dispatch	dispatch = {
								pswd_DoPeriodic,
								oop_DoClick,
								pswd_DoKey,
								oop_DoUpdate,
								oop_DoActivate,
								oop_DoDeactivate,
								oop_DoGrow,
								oop_DoZoom,
								oop_DoClose,
								pswd_DoCommand
							};

/*_____________________________________________________________________

	pswd_DoPeriodic - Do Periodic Tasks.
	
	Entry:	w = pointer to window record.
_____________________________________________________________________*/

void pswd_DoPeriodic (WindowPtr w)

{
	oop_DoPeriodic(w);
	glob_CheckPswdSel(w);
}

/*_____________________________________________________________________

	pswd_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void pswd_DoKey (WindowPtr w, char key, short modifiers)

{
	char					*pswd;		/* pointer to Pswd1 or Pswd2 */

	pswd = utl_GetDialogEditFieldNum(w) == pswdField1 ? Pswd1 : Pswd2;
	if (!glob_FilterPswdChar(w, key, modifiers, pswd)) return;
	oop_DoKey(w, key, modifiers);
}

/*_____________________________________________________________________

	pswd_DoCommand - Process a Command.
	
	Entry:	w = pointer to window record.
				theMenu = menu index.
				theItem = item index.
				
	Exit:		function result = true if command handled.
_____________________________________________________________________*/

Boolean pswd_DoCommand (WindowPtr w, short theMenu, short theItem)

{
	char					*pswd;		/* pointer to Pswd1 or Pswd2 */

	pswd = utl_GetDialogEditFieldNum(w) == pswdField1 ? Pswd1 : Pswd2;
	if (theMenu == editID && theItem != selectAllCmd) {
		glob_FilterPswdEditCmd(w, theItem, pswd);
		return true;
	}
	return oop_DoCommand(w, theMenu, theItem);
}

/*_____________________________________________________________________

	pswd_DoDialog - Do Change Password Dialog.
	
	Entry:	pswd1 = inital value for first password field.
				pswd2 = initial value for second password field.
	
	Exit:		function result = true if canceled by user.
				pswd1 = first password.
				pswd2 = second password.
_____________________________________________________________________*/

Boolean pswd_DoDialog (Str255 pswd1, Str255 pswd2)

{
	short			itemHit;		/* item hit */
	Str255		bullets;		/* string of bullets for initial password field */

	ShowCursor();
	Window = wstm_Restore(true, pswdDlogID, nil, &PswdState);
	utl_CopyPString(Pswd1, pswd1);
	utl_CopyPString(Pswd2, pswd2);
	*bullets = *pswd1;
	memset(bullets+1, '¥', *pswd1);
	utl_SetDialogText(Window, pswdField1, bullets);
	SelIText(Window, pswdField1, 0, 0x7fff);
	*bullets = *pswd2;
	memset(bullets+1, '¥', *pswd2);
	utl_SetDialogText(Window, pswdField2, bullets);
	SetPort(Window);
	TextFont(0);
	TextSize(12);
	oop_NewDialog(Window, pswdModal, nil, &dispatch, true,
		pswdOK, pswdCancel);
	ShowWindow(Window);
	while (true) {
		oop_ClearWindItemHit(Window);
		while (!(itemHit = oop_GetWindItemHit(Window)) && !Done) 
			oop_DoEvent(nil, everyEvent, 
				oop_InForeground() ? shortSleep : longSleep, nil);
		if (Done) itemHit = pswdCancel;
		if (itemHit == pswdCancel) break;
		break;
	}
	if (itemHit == pswdOK) {
		utl_CopyPString(pswd1, Pswd1);
		utl_CopyPString(pswd2, Pswd2);
	}
	wstm_Save(Window, &PswdState);
	oop_DoClose(Window);
	InitCursor();
	return itemHit == pswdCancel;
}
