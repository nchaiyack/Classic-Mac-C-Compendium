/*_____________________________________________________________________

  	abou.c - About Box.
____________________________________________________________________*/

#pragma load "precompile"
#include "rez.h"
#include "utl.h"
#include "abou.h"
#include "glob.h"
#include "edit.h"
#include "oop.h"

#pragma segment abou

/*_____________________________________________________________________

	DrawAboutText - Draw About Box Text.

	Entry:	theWindow = pointer to dialog window.
				itemNo = item number of useritem.	
_____________________________________________________________________*/

static pascal void DrawAboutText (WindowPtr theWindow, short itemNo)

{
	short				itemType;		/* item type */
	Handle			item;				/* handle to item */
	Rect				box;				/* item rectangle */
	short				base;				/* baseline */
	char				line[80];		/* line to be drawn */
	short				i;					/* loop index */

	GetDItem(theWindow, itemNo, &itemType, &item, &box);
	TextFont(FontNum);
	TextSize(fontSize);
	base = box.top + Ascent + Leading;
	i = 1;
	while (true) {
		GetIndString(line, aboutStrID, i);
		if (!line[0]) break;
		MoveTo(box.left, base);
		DrawString(line);
		base += LineHeight;
		i++;
	}
	TextFont(0);
	TextSize(12);
}

/*_____________________________________________________________________

	DrawStatusMsg - Draw Status Message.

	Entry:	theWindow = pointer to dialog window.
				itemNo = item number of useritem.	
_____________________________________________________________________*/

static pascal void DrawStatusMsg (WindowPtr theWindow, short itemNo)

{
	short					itemType;		/* item type */
	Handle				item;				/* handle to item */
	Rect					box;				/* item rectangle */
	Str255				tmpl;				/* status message template */
	Str255				msg;				/* status message */
	Str255				heroMsg;			/* hero message */
	Str255				server;			/* login server */
	Str255				alias;			/* login alias */
	EditLoginStatus	status;			/* edit login status */

	status = edit_GetStatus(server, alias);
	if (status == editNotLoggedIn) {
		GetIndString(msg, stringsID, notLoggedInMsg);
	} else {
		GetIndString(tmpl, stringsID, statusMsg);
		if (status == editHero) {
			GetIndString(heroMsg, stringsID, heroStatusMsg);
		} else {
			*heroMsg = 0;
		}
		utl_PlugParams(tmpl, msg, server, alias, heroMsg, nil);
	}
	GetDItem(theWindow, itemNo, &itemType, &item, &box);
	TextFont(FontNum);
	TextSize(fontSize);
	TextBox(msg+1, *msg, &box, teJustLeft);
	TextFont(0);
	TextSize(12);
}

/*_____________________________________________________________________

	DrawNothing - Draw Nothing.

	Entry:	theWindow = pointer to dialog window.
				itemNo = item number of useritem.	
_____________________________________________________________________*/

static pascal void DrawNothing (WindowPtr theWindow, short itemNo)

{
#pragma unused (theWindow, itemNo)
}

/*_____________________________________________________________________

	abou_DoCommand - Process About Command.

	Entry:	top = pointer to top window record.
				theMenu = menu number.
				theItem = menu item number.
_____________________________________________________________________*/

#pragma segment command

Boolean abou_DoCommand (WindowPtr top, short theMenu, short theItem)

{
#pragma unused (top)

	DialogPtr			theDialog;			/* pointer to dialog record */
	short					itemHit;				/* item number hit */
	
	if (theMenu == appleID && theItem == aboutCmd) {
		utl_ModalDialog(aboutID, oop_ModalUpdate, 0, 0, &itemHit, &theDialog,
			DrawStatusMsg, DrawAboutText, utl_FrameItem, DrawNothing);
		DisposDialog(theDialog);
		return true;
	} else {
		return false;
	}
}

#pragma segment abou

/*_____________________________________________________________________

	abou_Init - Initialize.
_____________________________________________________________________*/

#pragma segment init

void abou_Init (void)

{
	oop_RegisterCommandHandler(abou_DoCommand);
}

#pragma segment abou
