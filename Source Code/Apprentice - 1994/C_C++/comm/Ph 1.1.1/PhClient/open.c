/*_____________________________________________________________________

  	open.c - Open Ph Record Movable Modal Dialog.
_____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "open.h"
#include "utl.h"
#include "glob.h"
#include "oop.h"
#include "wstm.h"

/*_____________________________________________________________________

	Global Variables.
_____________________________________________________________________*/

static DialogPtr		Window;			/* ptr to dialog window */

static oop_Dispatch	dispatch = {
								oop_DoPeriodic,
								oop_DoClick,
								open_DoKey,
								oop_DoUpdate,
								oop_DoActivate,
								oop_DoDeactivate,
								oop_DoGrow,
								oop_DoZoom,
								oop_DoClose,
								open_DoCommand
							};

/*_____________________________________________________________________

	open_DoKey - Process a Key Down Event.
	
	Entry:	w = pointer to window record.
				key = ascii code of key.
				modifiers = modifiers from event record.
_____________________________________________________________________*/

void open_DoKey (WindowPtr w, char key, short modifiers)

{
	if (!glob_FilterAsciiChar(w, key, modifiers)) return;
	oop_DoKey(w, key, modifiers);
}

/*_____________________________________________________________________

	open_DoCommand - Process a Command.
	
	Entry:	w = pointer to window record.
				theMenu = menu index.
				theItem = item index.
_____________________________________________________________________*/

Boolean open_DoCommand (WindowPtr w, short theMenu, short theItem)

{
	if (theMenu == editID && theItem == pasteCmd &&
		!glob_FilterPaste()) return true;
	return oop_DoCommand(w, theMenu, theItem);
}

/*_____________________________________________________________________

	InitField - Initialize Dialog Field.
	
	Entry:	fNum = field number.
				val = field value.
_____________________________________________________________________*/

static void InitField (short fNum, Str255 val)

{
	short			itemType;			/* item type */
	Handle		item;					/* handle to item */
	Rect			box;					/* item rect */

	GetDItem(Window, fNum, &itemType, &item, &box);
	SetIText(item, val);
}

/*_____________________________________________________________________

	open_DoDialog - Do open Dialog.
	
	Entry:	user = initial value for user field.
	
	Exit:		function result = true if canceled by user.
				user = alias or name.
_____________________________________________________________________*/

Boolean open_DoDialog (Str255 user)

{
	short 		itemHit;		/* item hit */

	ShowCursor();
	Window = wstm_Restore(true, openDlogID, nil, &OpenState);
	utl_SetDialogText(Window, openUserField, user);
	SelIText(Window, openUserField, 0, 0x7fff);
	SetPort(Window);
	TextFont(0);
	TextSize(12);
	oop_NewDialog(Window, openModal, nil, &dispatch, true, openOK, openCancel);
	ShowWindow(Window);
	while (true) {
		oop_ClearWindItemHit(Window);
		while (!(itemHit = oop_GetWindItemHit(Window)) && !Done) 
			oop_DoEvent(nil, everyEvent, 
				oop_InForeground() ? shortSleep : longSleep, nil);
		if (Done) itemHit = openCancel;
		if (itemHit == openCancel) break;
		break;
	}
	if (itemHit == openOK) utl_GetDialogText(Window, openUserField, user);
	wstm_Save(Window, &OpenState);
	oop_DoClose(Window);
	InitCursor();
	return itemHit == openCancel;
}
